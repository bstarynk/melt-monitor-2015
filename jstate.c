// file jstate.c - persistency support in JSON & Sqlite

/**   Copyright (C) 2016  Basile Starynkevitch and later the FSF
    MONIMELT is a monitor for MELT - see http://gcc-melt.org/
    This file is part of GCC.
  
    GCC is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3, or (at your option)
    any later version.
  
    GCC is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with GCC; see the file COPYING3.   If not see
    <http://www.gnu.org/licenses/>.
**/

#include "meltmoni.h"


/*** IMPORTANT NOTICE: the list of tables created by function
 * mo_dump_initialize_sqlite_database should be kept in sync with the
 * monimelt-dump-state.sh script (look for lines with ".mode insert"
 * inside that script)
 ***/

typedef struct mo_dumper_st mo_dumper_ty;
typedef struct mo_loader_st mo_loader_ty;
void mo_dump_initialize_sqlite_database (mo_dumper_ty *);


#define MOM_LOADER_MAGIC  0x179128bd
struct mo_loader_st             // stack allocated
{
  unsigned mo_ld_magic;         /* always MOM_LOADER_MAGIC */
  double mo_ld_startelapsedtime;
  double mo_ld_startcputime;
  unsigned mo_ld_nbitems;
  mo_value_t mo_ld_hsetitems;
  sqlite3 *mo_ld_db;
};



enum mom_dumpstate_en
{ MOMDUMP_NONE, MOMDUMP_SCAN, MOMDUMP_EMIT };
#define MOM_DUMPER_MAGIC  0x372bb699
struct mo_dumper_st             // stack allocated
{
  unsigned mo_du_magic;         /* always MOM_DUMPER_MAGIC */
  enum mom_dumpstate_en mo_du_state;
  sqlite3 *mo_du_db;
  // SQLstmt: INSERT INTO t_params (par_name, par_value)
  sqlite3_stmt *mo_du_stmt_params;
  /** SQLstmt: INSERT INTO t_object (ob_id, ob_mtime, ob_classid,
                                  ob_paylkid, ob_paylcont, ob_jsoncont)
  **/
  sqlite3_stmt *mo_du_stmt_objects;
  // SQLstmt: INSERT INTO t_names (nam_str, nam_oid)
  sqlite3_stmt *mo_du_stmt_names;
  //
  const char *mo_du_dirv;
  mo_value_t mo_du_tempsufv;
  mo_hashsetpayl_ty *mo_du_objset;      /* the set of reachable & emittable objects */
  mo_listpayl_ty *mo_du_scanlist;       /* the todo list for scanning */
  mo_vectvaldatapayl_ty *mo_du_vectfilepath;    /* vector of dumped file paths */
};
bool
mo_dump_scanning (mo_dumper_ty * du)
{
  if (!du || du == MOM_EMPTY_SLOT)
    return false;
  MOM_ASSERTPRINTF (du->mo_du_magic == MOM_DUMPER_MAGIC, "bad dumper du@%p",
                    du);
  return du->mo_du_state == MOMDUMP_SCAN;
}                               /* end mo_dump_scanning */

bool
mo_dump_emitting (mo_dumper_ty * du)
{
  if (!du || du == MOM_EMPTY_SLOT)
    return false;
  MOM_ASSERTPRINTF (du->mo_du_magic == MOM_DUMPER_MAGIC, "bad dumper du@%p",
                    du);
  return du->mo_du_state == MOMDUMP_EMIT;
}                               /* end mo_dump_emitting */



// for SQLITE_CONFIG_LOG
void
mo_dump_errorlog (void *pdata MOM_UNUSED, int errcode, const char *msg)
{
  MOM_WARNPRINTF ("Sqlite3 Error: errcode#%d msg=%s", errcode, msg);
}

void
mo_dump_param (mo_dumper_ty * du, const char *pname, const char *pval)
{
  int rc = 0;
  enum paramindex_en
  { MOMPARAMIX__NONE, MOMPARAMIX_NAME, MOMPARAMIX_VAL };
  MOM_ASSERTPRINTF (du && du->mo_du_magic == MOM_DUMPER_MAGIC
                    && du->mo_du_state == MOMDUMP_EMIT
                    && du->mo_du_stmt_params, "invalid dumper");
  MOM_ASSERTPRINTF (pname != NULL && pname != MOM_EMPTY_SLOT
                    && isascii (pname[0]), "bad pname");
  MOM_ASSERTPRINTF (pval != NULL && pval != MOM_EMPTY_SLOT, "bad pval");
  rc =
    sqlite3_bind_text (du->mo_du_stmt_params, MOMPARAMIX_NAME, pname, -1,
                       SQLITE_STATIC);
  if (rc)
    MOM_FATAPRINTF
      ("failed to bind par_name %s to param insert Sqlite3 statment (%s)",
       pname, sqlite3_errstr (rc));
  rc =
    sqlite3_bind_text (du->mo_du_stmt_params, MOMPARAMIX_VAL, pval, -1,
                       SQLITE_STATIC);
  if (rc)
    MOM_FATAPRINTF
      ("failed to bind par_value %s to param insert Sqlite3 statment (%s)",
       pval, sqlite3_errstr (rc));
  rc = sqlite3_step (du->mo_du_stmt_params);
  if (rc != SQLITE_DONE)
    MOM_FATAPRINTF
      ("failed to step insert Sqlite3 statment for pname %s (%s)", pname,
       sqlite3_errstr (rc));
  rc = sqlite3_reset (du->mo_du_stmt_params);
  if (rc != SQLITE_OK)
    MOM_FATAPRINTF
      ("failed to reset insert Sqlite3 statment for pname %s (%s)", pname,
       sqlite3_errstr (rc));

}                               /* end of mo_dump_param */

void
mo_dump_initialize_sqlite_database (mo_dumper_ty * du)
{
  int rc = 0;
  MOM_ASSERTPRINTF (du && du->mo_du_magic == MOM_DUMPER_MAGIC
                    && du->mo_du_state == MOMDUMP_EMIT, "invalid dumper");
  du->mo_du_db = NULL;
  char *errmsg = NULL;
  /***** open the database *****/
  mo_value_t sqlpathbufv =      //
    mo_make_string_sprintf ("%s/%s.sqlite%s",
                            mo_string_cstr (du->mo_du_dirv),
                            monimelt_perstatebase,
                            mo_string_cstr (du->mo_du_tempsufv));
  int nok = sqlite3_open (mo_string_cstr (sqlpathbufv), &du->mo_du_db);
  if (nok != SQLITE_OK || !du->mo_du_db)
    MOM_FATAPRINTF ("failed to sqlite3_open %s (%s)",
                    mo_string_cstr (sqlpathbufv), sqlite3_errstr (nok));
  /***** create tables and indexes *****/
  // keep these table names in CREATE TABLE Sqlite statements in sync
  // with monimelt-dump-state.sh script
  //
  if ((errmsg = NULL),          //
      sqlite3_exec (du->mo_du_db,
                    "CREATE TABLE t_params"
                    " (par_name VARCHAR(35) PRIMARY KEY ASC NOT NULL UNIQUE,"
                    "  par_value TEXT NOT NULL)", NULL, NULL, &errmsg))
    MOM_FATAPRINTF ("Failed to create t_params Sqlite table: %s", errmsg);
  //
  if ((errmsg = NULL),          //
      sqlite3_exec (du->mo_du_db,
                    "CREATE TABLE t_objects"
                    " (ob_id VARCHAR(20) PRIMARY KEY ASC NOT NULL UNIQUE,"
                    "  ob_mtime DATETIME,"
                    "  ob_classid VARCHAR(20) NOT NULL,"
                    "  ob_paylkid VARCHAR(20) NOT NULL,"
                    "  ob_paylcont TEXT NOT NULL,"
                    "  ob_jsoncont TEXT NOT NULL)", NULL, NULL, &errmsg))
    MOM_FATAPRINTF ("Failed to create t_objects Sqlite table: %s", errmsg);
  //
  if ((errmsg = NULL),          //
      sqlite3_exec (du->mo_du_db,
                    "CREATE TABLE t_names"
                    " (nam_str PRIMARY KEY ASC NOT NULL UNIQUE,"
                    "  nam_oid VARCHAR(20) NOT NULL UNIQUE)",
                    NULL, NULL, &errmsg))
    MOM_FATAPRINTF ("Failed to create t_names Sqlite table: %s", errmsg);
  //
  if ((errmsg = NULL),          //
      sqlite3_exec (du->mo_du_db,
                    "CREATE UNIQUE INDEX x_namedid ON t_names (nam_oid)",
                    NULL, NULL, &errmsg))
    MOM_FATAPRINTF ("Failed to create x_namedid Sqlite index: %s", errmsg);
  /***** prepare statements *****/
  if ((rc = sqlite3_prepare_v2 (du->mo_du_db,
                                "INSERT INTO t_params (par_name, par_value) VALUES (?, ?)",
                                -1, &du->mo_du_stmt_params,
                                NULL)) != SQLITE_OK)
    MOM_FATAPRINTF ("Failed to prepare t_params Sqlite insertion: %s",
                    sqlite3_errstr (rc));
  if ((rc = sqlite3_prepare_v2 (du->mo_du_db,
                                "INSERT INTO t_objects"
                                " (ob_id, ob_mtime, ob_classid,"
                                "  ob_paylkid, ob_paylcont, ob_jsoncont)"
                                " VALUES (?, ?, ?, ?, ?, ?)", -1,
                                &du->mo_du_stmt_objects, NULL)) != SQLITE_OK)
    MOM_FATAPRINTF ("Failed to prepare t_objects Sqlite insertion: %s",
                    sqlite3_errstr (rc));
  if ((rc = sqlite3_prepare_v2 (du->mo_du_db,
                                "INSERT INTO t_names (nam_str, nam_oid) VALUES (?, ?)",
                                -1, &du->mo_du_stmt_names,
                                NULL)) != SQLITE_OK)
    MOM_FATAPRINTF ("Failed to prepare t_names Sqlite insertion: %s",
                    sqlite3_errstr (rc));
  /**** insert various parameters ****/
  mo_dump_param (du, "monimelt_format_version", MOM_DUMP_VERSIONID);
}                               /* end mo_dump_initialize_sqlite_database */


bool
mo_dump_is_emitted_objref (mo_dumper_ty * du, mo_objref_t obr)
{
  MOM_ASSERTPRINTF (du && du->mo_du_magic == MOM_DUMPER_MAGIC
                    && du->mo_du_state == MOMDUMP_EMIT, "bad dumper du@%p",
                    du);
  if (!mo_dyncast_objref (obr))
    return false;
  return mo_hashset_contains (du->mo_du_objset, obr);
}                               /* end mo_dump_is_emitted_objref */


void
mo_dump_emit_object_content (mo_dumper_ty * du, mo_objref_t obr)
{
  int rc = 0;
  enum paramindex_en
  { MOMOBJIX__NONE, MOMOBJIX_ID, MOMOBJIX_MTIME, MOMOBJIX_CLASSID,
    MOMOBJIX_PAYLKID, MOMOBJIX_PAYLCONT, MOMOBJIX_JSONCONT
  };
  MOM_ASSERTPRINTF (du && du->mo_du_magic == MOM_DUMPER_MAGIC
                    && du->mo_du_state == MOMDUMP_EMIT
                    && du->mo_du_stmt_objects, "invalid dumper");
  if (!mo_dyncast_objref (obr) || !mo_dump_is_emitted_objref (du, obr))
    return;
  char bufid[MOM_CSTRIDLEN + 6];
  memset (bufid, 0, sizeof (bufid));
  mo_cstring_from_hi_lo_ids (bufid, obr->mo_ob_hid, obr->mo_ob_loid);
  // bind the ob_id
  rc =
    sqlite3_bind_text (du->mo_du_stmt_objects, MOMOBJIX_ID, bufid, -1,
                       SQLITE_STATIC);
  if (rc)
    MOM_FATAPRINTF
      ("failed to bind ob_id for t_objects insert Sqlite3 statment (%s)",
       sqlite3_errstr (rc));
  // bind the ob_mtime
  rc =
    sqlite3_bind_double (du->mo_du_stmt_objects, MOMOBJIX_MTIME,
                         (double) obr->mo_ob_mtime);
  if (rc)
    MOM_FATAPRINTF
      ("failed to bind ob_mtime for t_objects insert Sqlite3 statment (%s)",
       sqlite3_errstr (rc));
  // bind the ob_classid if given & emittable
  mo_objref_t claobr = obr->mo_ob_class;
  if (mo_dyncast_objref (claobr) && mo_dump_is_emitted_objref (du, claobr))
    {
      char clabufid[MOM_CSTRIDLEN + 6];
      memset (clabufid, 0, sizeof (clabufid));
      mo_cstring_from_hi_lo_ids (clabufid, claobr->mo_ob_hid,
                                 claobr->mo_ob_loid);
      rc =
        sqlite3_bind_text (du->mo_du_stmt_objects, MOMOBJIX_CLASSID, clabufid,
                           -1, SQLITE_STATIC);
      if (rc)
        MOM_FATAPRINTF
          ("failed to bind ob_classid for t_objects insert Sqlite3 statment (%s)",
           sqlite3_errstr (rc));
    }
  else
    {
      rc =
        sqlite3_bind_text (du->mo_du_stmt_objects, MOMOBJIX_CLASSID, "", -1,
                           SQLITE_STATIC);
      if (rc)
        MOM_FATAPRINTF
          ("failed to empty-bind ob_classid for t_objects insert Sqlite3 statment (%s)",
           sqlite3_errstr (rc));
    }
  // bind the ob_paylkid if payload kind given & emittable
  mo_objref_t paylkindobr = obr->mo_ob_paylkind;
  if (mo_dyncast_objref (paylkindobr)
      && mo_dump_is_emitted_objref (du, paylkindobr))
    {
      char pkbufid[MOM_CSTRIDLEN + 6];
      memset (pkbufid, 0, sizeof (pkbufid));
      mo_cstring_from_hi_lo_ids (pkbufid, paylkindobr->mo_ob_hid,
                                 paylkindobr->mo_ob_loid);
      rc =
        sqlite3_bind_text (du->mo_du_stmt_objects, MOMOBJIX_PAYLKID, pkbufid,
                           -1, SQLITE_STATIC);
      if (rc)
        MOM_FATAPRINTF
          ("failed to bind ob_paylkid for t_objects insert Sqlite3 statment (%s)",
           sqlite3_errstr (rc));
      void *payload = obr->mo_ob_payload;
      // we should dump the payload, but how?
      MOM_WARNPRINTF
        ("unimplemented mo_dump_emit_object_content payload@%p for %s",
         payload, mo_object_pnamestr (obr));
#warning unimplemented mo_dump_emit_object_content payload
    }
  else
    {
      rc =
        sqlite3_bind_text (du->mo_du_stmt_objects, MOMOBJIX_PAYLKID, "", -1,
                           SQLITE_STATIC);
      if (rc)
        MOM_FATAPRINTF
          ("failed to empty-bind ob_paylkid for t_objects insert Sqlite3 statment (%s)",
           sqlite3_errstr (rc));
      rc =
        sqlite3_bind_text (du->mo_du_stmt_objects, MOMOBJIX_PAYLCONT, "", -1,
                           SQLITE_STATIC);
      if (rc)
        MOM_FATAPRINTF
          ("failed to empty-bind ob_paylcont for t_objects insert Sqlite3 statment (%s)",
           sqlite3_errstr (rc));
    }
  // now construct the JSON object for the content and bind ob_jsoncont
  mo_json_t jattrs = mo_dump_json_of_assoval (du, obr->mo_ob_attrs);
  mo_json_t jcomps = mo_dump_json_of_vectval (du, obr->mo_ob_comps);
  mo_json_t jcont = json_pack ("{soso}", "attrs", jattrs, "comps", jcomps);
  char *contbuf = NULL;
  size_t contsiz = 0;
  FILE *fmem = open_memstream (&contbuf, &contsiz);
  if (!fmem)
    MOM_FATAPRINTF ("failed to open memstream for content of object %s",
                    mo_object_pnamestr (obr));
  fputc ('\n', fmem);
  if (json_dumpf (jcont, fmem, JSON_INDENT (1) | JSON_SORT_KEYS))
    MOM_FATAPRINTF ("failed to json_dumpf for content of object %s",
                    mo_object_pnamestr (obr));
  fputc ('\n', fmem);
  fflush (fmem);
  rc =
    sqlite3_bind_text (du->mo_du_stmt_objects, MOMOBJIX_JSONCONT, contbuf,
                       contsiz, SQLITE_TRANSIENT);
  if (rc)
    MOM_FATAPRINTF
      ("failed to bind ob_jsoncont for t_objects insert Sqlite3 statment (%s)",
       sqlite3_errstr (rc));
  fclose (fmem);
  free (contbuf), contbuf = NULL;
  rc = sqlite3_step (du->mo_du_stmt_objects);
  if (rc != SQLITE_DONE)
    MOM_FATAPRINTF
      ("failed to step insert Sqlite3 statment for t_objects (%s)",
       sqlite3_errstr (rc));
  rc = sqlite3_reset (du->mo_du_stmt_objects);
  if (rc != SQLITE_OK)
    MOM_FATAPRINTF
      ("failed to reset insert Sqlite3 statment for t_objects (%s)",
       sqlite3_errstr (rc));
}                               /* end of mo_dump_emit_object_content */




void
mo_dump_scan_inside_object (mo_dumper_ty * du, mo_objref_t obr)
{
  MOM_ASSERTPRINTF (du && du->mo_du_magic == MOM_DUMPER_MAGIC
                    && du->mo_du_state == MOMDUMP_SCAN, "bad dumper du@%p",
                    du);
  MOM_ASSERTPRINTF (mo_dyncast_objref (obr), "bad obr");
  if (mo_objref_space (obr) == mo_SPACE_NONE)
    return;
  mo_dump_scan_objref (du, obr->mo_ob_class);
  mo_dump_scan_objref (du, obr->mo_ob_paylkind);
  MOM_WARNPRINTF ("unimplemented mo_dump_scan_inside_object for %s",
                  mo_object_pnamestr (obr));
#warning unimplemented mo_dump_scan_inside_object
}                               /* end of mo_dump_scan_inside_object */

FILE *
mo_dump_fopen (mo_dumper_ty * du, const char *path)
{
  MOM_ASSERTPRINTF (du && du->mo_du_magic == MOM_DUMPER_MAGIC
                    && du->mo_du_state == MOMDUMP_EMIT, "bad dumper du@%p",
                    du);
  MOM_ASSERTPRINTF (path && (isalnum (path[0]) || !strncmp (path, "_mom", 4)),
                    "bad path %s", path);
  mo_value_t pathbufv =         //
    mo_make_string_sprintf ("%s/%s%s",
                            mo_string_cstr (du->mo_du_dirv),
                            path,
                            mo_string_cstr (du->mo_du_tempsufv));
  FILE *f = fopen (mo_string_cstr (pathbufv), "wx");
  if (!f)
    MOM_FATAPRINTF ("fopen %s failed (%m)", mo_string_cstr (pathbufv));
  du->mo_du_vectfilepath =
    mo_vectval_append (du->mo_du_vectfilepath, mo_make_string_cstr (path));
  return f;
}                               /* end of mo_dump_fopen */

void
mo_dump_emit_predefined (mo_dumper_ty * du, mo_value_t predset)
{
  MOM_ASSERTPRINTF (du && du->mo_du_magic == MOM_DUMPER_MAGIC
                    && du->mo_du_state == MOMDUMP_EMIT, "bad dumper du@%p",
                    du);
  int nbpredef = mo_set_size (predset);
  MOM_ASSERTPRINTF (nbpredef > 0, "empty predset");
  FILE *fp = mo_dump_fopen (du, MOM_PREDEF_HEADER);
  mom_output_gplv3_notice (fp, "///", "", MOM_PREDEF_HEADER);
  fprintf (fp, "\n#ifndef MOM_HAS_PREDEFINED\n"
           "#error missing MOM_HAS_PREDEFINED\n" "#endif\n\n");
  fprintf (fp, "\n#undef MOM_NB_PREDEFINED\n"
           "#define MOM_NB_PREDEFINED %d\n", nbpredef);
  fprintf (fp, "\n\n//// MOM_HAS_PREDEFINED(Name,Idstr,Hid,Loid,Hash)\n");
  int nbanon = 0;
  int nbnamed = 0;
  for (int ix = 0; ix < nbpredef; ix++)
    {
      mo_objref_t obp = mo_set_nth (predset, ix);
      MOM_ASSERTPRINTF (mo_dyncast_objref (obp)
                        && mo_objref_space (obp) == mo_SPACE_PREDEF,
                        "bad obp");
      mo_value_t namv = mo_get_namev (obp);
      char idstr[MOM_CSTRIDLEN + 6];
      memset (idstr, 0, sizeof (idstr));
      mo_cstring_from_hi_lo_ids (idstr, obp->mo_ob_hid, obp->mo_ob_loid);
      if (namv != NULL)
        {
          nbnamed++;
          MOM_ASSERTPRINTF (mo_dyncast_string (namv), "bad namv");
          fprintf (fp, "MOM_HAS_PREDEFINED(%s,%s,%ld,%lld,%u)\n",
                   mo_string_cstr (namv), idstr,
                   (long) obp->mo_ob_hid, (long long) obp->mo_ob_loid,
                   (unsigned) mo_objref_hash (obp));
        }
      else
        {
          nbanon++;
          fprintf (fp, "MOM_HAS_PREDEFINED(%s,%s,%ld,%lld,%u)\n",
                   idstr, idstr,
                   (long) obp->mo_ob_hid, (long long) obp->mo_ob_loid,
                   (unsigned) mo_objref_hash (obp));
        }
    }
  MOM_ASSERTPRINTF (nbanon + nbnamed == nbpredef, "bad nbanon");
  fprintf (fp, "\n#undef MOM_NB_ANONYMOUS_PREDEFINED\n"
           "#define MOM_NB_ANONYMOUS_PREDEFINED %d\n", nbanon);
  fprintf (fp, "\n#undef MOM_NB_NAMED_PREDEFINED\n"
           "#define MOM_NB_NAMED_PREDEFINED %d\n", nbnamed);
  fprintf (fp, "\n\n#undef MOM_HAS_PREDEFINED\n");
  fprintf (fp, "// end of generated predefined file %s\n", MOM_PREDEF_HEADER);
  fclose (fp);
}                               /* end mo_dump_emit_predefined */



void
mo_dump_emit_names (mo_dumper_ty * du)
{
  int rc = 0;
  enum nameindex_en
  { MOMNAMIX__NONE, MOMNAMIX_STR, MOMNAMIX_OID };
  MOM_ASSERTPRINTF (du && du->mo_du_magic == MOM_DUMPER_MAGIC
                    && du->mo_du_state == MOMDUMP_EMIT, "bad dumper du@%p",
                    du);
  mo_value_t namobjsetv = mo_named_objects_set ();
  int nbnamed = mo_set_size (namobjsetv);
  for (int ix = 0; ix < nbnamed; ix++)
    {
      mo_objref_t namobr = mo_set_nth (namobjsetv, ix);
      MOM_ASSERTPRINTF (mo_dyncast_objref (namobr), "bad namobr ix=%d", ix);
      mo_value_t strnamv = mo_get_namev (namobr);
      MOM_ASSERTPRINTF (mo_dyncast_string (strnamv), "bad strnamv ix=%d", ix);
      char idbuf[MOM_CSTRIDLEN + 6];
      memset (idbuf, 0, sizeof (idbuf));
      mo_cstring_from_hi_lo_ids
        (idbuf, namobr->mo_ob_hid, namobr->mo_ob_loid);
      rc =
        sqlite3_bind_text (du->mo_du_stmt_names, MOMNAMIX_OID, idbuf, -1,
                           SQLITE_STATIC);
      if (rc)
        MOM_FATAPRINTF
          ("failed to bind nam_oid to t_names insert Sqlite3 statment (%s)",
           sqlite3_errstr (rc));
      rc =
        sqlite3_bind_text (du->mo_du_stmt_names, MOMNAMIX_STR,
                           mo_string_cstr (strnamv), -1, SQLITE_STATIC);
      if (rc)
        MOM_FATAPRINTF
          ("failed to bind nam_str to t_names insert Sqlite3 statment (%s)",
           sqlite3_errstr (rc));
      rc = sqlite3_step (du->mo_du_stmt_names);
      if (rc != SQLITE_DONE)
        MOM_FATAPRINTF
          ("failed to step insert Sqlite3 statment for t_names (%s)",
           sqlite3_errstr (rc));
      rc = sqlite3_reset (du->mo_du_stmt_names);
      if (rc != SQLITE_OK)
        MOM_FATAPRINTF
          ("failed to reset insert Sqlite3 statment for t_names (%s)",
           sqlite3_errstr (rc));
    }
  MOM_INFORMPRINTF ("dumped %d names", nbnamed);
}                               /* end mo_dump_emit_names */

void
mo_dump_end_database (mo_dumper_ty * du)
{
  MOM_ASSERTPRINTF (du && du->mo_du_magic == MOM_DUMPER_MAGIC
                    && du->mo_du_state == MOMDUMP_EMIT
                    && du->mo_du_db != NULL, "bad dumper du@%p", du);
  /// finalize all the prepared statements
  int rc = 0;
  rc = sqlite3_finalize (du->mo_du_stmt_params);
  du->mo_du_stmt_params = NULL;
  if (rc != SQLITE_OK)
    MOM_FATAPRINTF ("Failed to finalize t_params Sqlite insertion: %s",
                    sqlite3_errstr (rc));
  //
  rc = sqlite3_finalize (du->mo_du_stmt_objects);
  du->mo_du_stmt_objects = NULL;
  if (rc != SQLITE_OK)
    MOM_FATAPRINTF ("Failed to finalize t_objects Sqlite insertion: %s",
                    sqlite3_errstr (rc));
  //
  rc = sqlite3_finalize (du->mo_du_stmt_names);
  du->mo_du_stmt_objects = NULL;
  if (rc != SQLITE_OK)
    MOM_FATAPRINTF ("Failed to finalize t_names Sqlite insertion: %s",
                    sqlite3_errstr (rc));
  // the various paths
  mo_value_t sqltmpathbufv =    //
    mo_make_string_sprintf ("%s/%s.sqlite%s",
                            mo_string_cstr (du->mo_du_dirv),
                            monimelt_perstatebase,
                            mo_string_cstr (du->mo_du_tempsufv));
  mo_value_t sqlfupathbufv =    //
    mo_make_string_sprintf ("%s/%s.sqlite",
                            mo_string_cstr (du->mo_du_dirv),
                            monimelt_perstatebase);
  mo_value_t sqlbackuppathbufv =        //
    mo_make_string_sprintf ("%s/%s.sqlite%%",
                            mo_string_cstr (du->mo_du_dirv),
                            monimelt_perstatebase);
  /// close the database and rename files
  rc = sqlite3_close (du->mo_du_db);
  if (rc != SQLITE_OK)
    MOM_FATAPRINTF ("failed to close Sqlite3 database %s (%s)",
                    mo_string_cstr (sqltmpathbufv), sqlite3_errstr (rc));
  du->mo_du_db = NULL;
  if (!access (mo_string_cstr (sqlfupathbufv), F_OK))
    (void) rename (mo_string_cstr (sqlfupathbufv),
                   mo_string_cstr (sqlbackuppathbufv));
  errno = 0;
  if (rename (mo_string_cstr (sqltmpathbufv), mo_string_cstr (sqlfupathbufv)))
    MOM_FATAPRINTF ("failed to rename database %s -> %s",
                    mo_string_cstr (sqltmpathbufv),
                    mo_string_cstr (sqlfupathbufv));
  MOM_INFORMPRINTF ("dump closed Sqlite3 database %s",
                    mo_string_cstr (sqlfupathbufv));
}                               /* end mo_dump_end_database */



void
mo_dump_rename_emitted_files (mo_dumper_ty * du)
{
  MOM_ASSERTPRINTF (du && du->mo_du_magic == MOM_DUMPER_MAGIC
                    && du->mo_du_state == MOMDUMP_EMIT, "bad dumper du@%p", du);
  unsigned nbfil = mo_vectval_count (du->mo_du_vectfilepath);
  unsigned nbsamefiles = 0;
  for (unsigned ix = 0; ix < nbfil; ix++)
    {
      mo_value_t curpathv = mo_vectval_nth (du->mo_du_vectfilepath, ix);
      MOM_ASSERTPRINTF (mo_dyncast_string (curpathv), "bad curpathv");
      mo_value_t tmpathv =      //
        mo_make_string_sprintf ("%s%s",
                                mo_string_cstr (curpathv),
                                mo_string_cstr (du->mo_du_tempsufv));
      mo_value_t backupv =
        mo_make_string_sprintf ("%s%%", mo_string_cstr (curpathv));
      bool samefilecont = false;
      struct stat curstat;
      struct stat tmpstat;
      memset (&curstat, 0, sizeof (curstat));
      memset (&tmpstat, 0, sizeof (tmpstat));
      if (!stat (mo_string_cstr (curpathv), &curstat)
          && !stat (mo_string_cstr (tmpathv), &tmpstat)
          && curstat.st_size == tmpstat.st_size)
        {
          FILE *curf = fopen (mo_string_cstr (curpathv), "r");
          FILE *tmpf = fopen (mo_string_cstr (tmpathv), "r");
          if (curf != NULL && tmpf != NULL)
            {
              samefilecont = true;
              while (samefilecont)
                {
                  int curc = fgetc (curf);
                  int tmpc = fgetc (tmpf);
                  if (curc != tmpc)
                    samefilecont = false;
                  else if (curc == EOF)
                    break;
                }
            }
          if (curf)
            fclose (curf);
          if (tmpf)
            fclose (tmpf);
        };
      if (samefilecont)
        {
          if (unlink (mo_string_cstr (tmpathv)))
            MOM_FATAPRINTF ("failed to unlink %s (%m)",
                            mo_string_cstr (tmpathv));
          nbsamefiles++;
          MOM_INFORMPRINTF ("same file#%d/%d: %s", nbsamefiles, nbfil,
                            mo_string_cstr (curpathv));
        }
      else
        {
          (void) rename (mo_string_cstr (curpathv), mo_string_cstr (backupv));
          if (rename (mo_string_cstr (tmpathv), mo_string_cstr (curpathv)))
            MOM_FATAPRINTF ("failed to rename %s -> %s (%m)",
                            mo_string_cstr (tmpathv),
                            mo_string_cstr (curpathv));
          MOM_INFORMPRINTF ("generated file #%d: %s", nbfil,
                            mo_string_cstr (curpathv));
        }
    };
  MOM_INFORMPRINTF ("dump emitted %u files - with %u same as previously",
                    nbfil, nbsamefiles);
}                               /* end mo_dump_rename_emitted_files */

void
mom_dump_state (const char *dirname)
{
  if (!dirname || dirname == MOM_EMPTY_SLOT || !dirname[0])
    dirname = ".";
  if ((isalnum (dirname[0]) || dirname[0] == '/')
      && access (dirname, F_OK) && errno == ENOENT)
    {
      if (mkdir (dirname, 0750))
        MOM_FATAPRINTF ("failed to mkdir dump directory %s (%m)", dirname);
    }
  {
    struct stat dirstat;
    memset (&dirstat, 0, sizeof (dirstat));
    if (stat (dirname, &dirstat) || !S_ISDIR (dirstat.st_mode)
        || !(S_IWUSR & dirstat.st_mode))
      MOM_FATAPRINTF ("bad dump directory %s (non-writable directory)",
                      dirname);
  }
  mo_dumper_ty dumper;
  mo_value_t predefset = mo_predefined_objects_set ();
  int nbpredef = mo_set_size (predefset);
  if (nbpredef == 0 || nbpredef + 10 < MOM_NB_PREDEFINED / 2)
    MOM_FATAPRINTF ("too few remaining predef %d (previously %d)",
                    nbpredef, MOM_NB_PREDEFINED);
  memset (&dumper, 0, sizeof (dumper));
  dumper.mo_du_magic = MOM_DUMPER_MAGIC;
  dumper.mo_du_state = MOMDUMP_SCAN;
  dumper.mo_du_db = NULL;
  dumper.mo_du_dirv = mo_make_string_cstr (dirname);
  dumper.mo_du_tempsufv =
    mo_make_string_sprintf (".tmp_%lx_%lx_p%d%%",
                            momrand_genrand_int31 (),
                            momrand_genrand_int31 (), (int) getpid ());
  dumper.mo_du_objset = mo_hashset_reserve (NULL, 4 * nbpredef + 100);
  dumper.mo_du_scanlist = mo_list_make ();
  dumper.mo_du_vectfilepath = mo_vectval_reserve (NULL, 20 + nbpredef / 5);
  for (int ix = 0; ix < nbpredef; ix++)
    mo_dump_scan_objref (&dumper, mo_set_nth (predefset, ix));
  long nbobj = 0;
  /// the scan loop
  errno = 0;
  while (mo_list_non_empty (dumper.mo_du_scanlist))
    {
      mo_objref_t obr =
        mo_dyncast_objref (mo_list_head (dumper.mo_du_scanlist));
      MOM_ASSERTPRINTF (obr != NULL, "nil obr");
      mo_list_pop_head (dumper.mo_du_scanlist);
      mo_dump_scan_inside_object (&dumper, obr);
      nbobj++;
    }
  /// the emit loop
  dumper.mo_du_state = MOMDUMP_EMIT;
  mo_dump_initialize_sqlite_database (&dumper);
  mo_dump_emit_predefined (&dumper, predefset);
  mo_value_t elset = mo_hashset_elements_set (dumper.mo_du_objset);
  unsigned elsiz = mo_set_size (elset);
  MOM_ASSERTPRINTF (elsiz >= (unsigned) nbpredef, "bad elsiz");
  MOM_ASSERTPRINTF (elsiz <= (unsigned) nbobj, "bad elsiz");
  for (unsigned eix = 0; eix < elsiz; eix++)
    {
      mo_objref_t obr = mo_set_nth (elset, eix);
      MOM_ASSERTPRINTF (mo_dyncast_objref (obr), "bad obr");
      mo_dump_emit_object_content (&dumper, obr);
    }
  mo_dump_emit_names (&dumper);
  mo_dump_end_database (&dumper);
  mo_dump_rename_emitted_files (&dumper);
#warning mom_dump_state very incomplete
  MOM_WARNPRINTF ("mom_dump_state very incomplete into %s", dirname);
}                               /* end mom_dump_state */


void
mo_dump_really_scan_value (mo_dumper_ty * du, mo_value_t v)
{
  MOM_ASSERTPRINTF (du && du->mo_du_magic == MOM_DUMPER_MAGIC
                    && du->mo_du_state == MOMDUMP_SCAN, "bad dumper du@%p",
                    du);
  enum mo_valkind_en kd = mo_kind_of_value (v);
  switch (kd)
    {
    case mo_KNONE:
    case mo_KINT:
    case mo_KSTRING:
      return;
    case mo_KTUPLE:
    case mo_KSET:
      {
        mo_sequencevalue_ty *seq = (mo_sequencevalue_ty *) v;
        unsigned sz = ((mo_sizedvalue_ty *) seq)->mo_sva_size;
        for (unsigned ix = 0; ix < sz; ix++)
          mo_dump_scan_objref (du, seq->mo_seqobj[ix]);
        return;
      }
    case mo_KOBJECT:
      mo_dump_scan_objref (du, (mo_objref_t) v);
      return;
    }
}                               /* end mo_dump_scan_value */

void
mo_dump_really_scan_objref (mo_dumper_ty * du, mo_objref_t obr)
{
  MOM_ASSERTPRINTF (du && du->mo_du_magic == MOM_DUMPER_MAGIC
                    && du->mo_du_state == MOMDUMP_SCAN, "bad dumper du@%p",
                    du);
  if (mo_dyncast_objref (obr) == NULL)
    return;
  if (mo_objref_space (obr) == mo_SPACE_NONE)
    return;
  if (mo_hashset_contains (du->mo_du_objset, obr))
    return;
  du->mo_du_objset = mo_hashset_put (du->mo_du_objset, obr);
  mo_list_append (du->mo_du_scanlist, obr);
}                               /* end mo_dump_scan_objref */


/**************************** LOADER ********************/
void
mom_load_state (void)
{
  /*** steps to consider:
    0. Check that _momstate.sql & _momstate.sqlite have a comparable age...
    1. Open the database, create the statements
    2. SELECT COUNT(*) FROM t_objects --> nbobjects
    3. SELECT COUNT(*) FROM t_named   --> nbnamedobjects
    4. Reserve hashed set hsetitems #nbobjects & reserve names #nbnamedobjects
    5. SELECT ob_id FROM t_objects  ===> create each object from its id, put it into hsetitems
    6. SELECT nam_oid, nam_str FROM t_names ===> name each named object
    7. SELECT ob_id, ob_mtime, ob_jsoncont FROM t_objects ===> fill the mtime & content of each object
    | more steps to fill the class & the payload, 
    | perhaps SELECT ob_id, ob_classid FROM t_objects WHERE ob_classid != ""
  ***/
  MOM_WARNPRINTF ("load state unimplemented");
#warning mom_load_state unimplemented
}                               /* end mom_load_state */


/**************** JSON ****************/
mo_json_t
mo_dump_json_of_value (mo_dumper_ty * du, mo_value_t v)
{
  MOM_ASSERTPRINTF (mo_dump_emitting (du), "bad du");
  if (mo_value_is_int (v))
    return json_integer (mo_value_to_int (v, 0));
  else if (!mo_valid_pointer_value (v))
    return json_null ();
  enum mo_valkind_en k = mo_kind_of_value (v);
  switch (k)
    {
    case mo_KNONE:
    case mo_KINT:
      MOM_FATAPRINTF ("impossible kind of v@%p", v);
    case mo_KSTRING:
      return json_stringn (mo_string_cstr (v), mo_size_of_value (v));
    case mo_KTUPLE:
    case mo_KSET:
      {
        json_t *jarr = json_array ();
        unsigned sz = mo_size_of_value (v);
        for (unsigned ix = 0; ix < sz; ix++)
          {
            mo_objref_t obr = ((mo_sequencevalue_ty *) v)->mo_seqobj[ix];
            if (!mo_dyncast_objref (obr)
                || !mo_dump_is_emitted_objref (du, obr))
              continue;
            json_array_append_new (jarr,
                                   (json_t *) mo_dump_jsonid_of_objref (du,
                                                                        obr));
          }
        return json_pack ("{so}", (k == mo_KTUPLE) ? "tup" : "set", jarr);
      }
    case mo_KOBJECT:
      if (!mo_dump_is_emitted_objref (du, (mo_objref_t) v))
        return json_null ();
      return json_pack ("{so}", "oid",
                        mo_dump_jsonid_of_objref (du, (mo_objref_t) v));
    };
  MOM_FATAPRINTF ("impossible value@%p k#%u to json", v, k);
}                               /* end mo_dump_json_of_value */



mo_json_t
mo_dump_jsonid_of_objref (mo_dumper_ty * du, mo_objref_t obr)
{
  MOM_ASSERTPRINTF (mo_dump_emitting (du), "bad du");
  if (!mo_dyncast_objref ((mo_value_t) obr)
      || !mo_dump_is_emitted_objref (du, obr))
    return json_null ();
  char idbuf[MOM_CSTRIDLEN + 6];
  memset (idbuf, 0, sizeof (idbuf));
  return
    json_string (mo_cstring_from_hi_lo_ids
                 (idbuf, obr->mo_ob_hid, obr->mo_ob_loid));
}                               /* end of mo_dump_jsonid_of_objref */

mo_objref_t
mo_objref_of_jsonid (mo_json_t js)
{
  if (!js || json_is_null (js))
    return NULL;
  if (json_is_string (js))
    {
      mo_hid_t hid = 0;
      mo_loid_t loid = 0;
      if (!mo_get_hi_lo_ids_from_cstring
          (&hid, &loid, json_string_value (js)))
        {
          MOM_WARNPRINTF ("invalid jsonid %s", json_string_value (js));
          return NULL;
        };
      mo_objref_t orf = mo_objref_find_hid_loid (hid, loid);
      if (orf == NULL)
        {
          MOM_WARNPRINTF ("missing jsonid %s", json_string_value (js));
          return NULL;
        }
      return orf;
    }
  else if (json_is_object (js))
    {
      mo_json_t jid = json_object_get (js, "oid");
      if (!json_is_string (jid))
        {
          MOM_WARNPRINTF ("bad jsonid %s", json_dumps (js, JSON_SORT_KEYS));
          return NULL;
        }
      return mo_objref_of_jsonid (jid);
    }
  MOM_WARNPRINTF ("wrong jsonid %s", json_dumps (js, JSON_SORT_KEYS));
  return NULL;
}                               /* end mo_objref_of_jsonid */

mo_value_t
mo_value_of_json (mo_json_t js)
{
  if (json_is_null (js))
    return NULL;
  else if (json_is_integer (js))
    return mo_int_to_value (json_integer_value (js));
  else if (json_is_string (js))
    return mo_make_string_len (json_string_value (js),
                               json_string_length (js));
  else if (json_is_object (js))
    {
      json_t *jc = NULL;
      if ((jc = json_object_get (js, "oid")) != NULL && json_is_string (jc))
        {
          return mo_objref_of_jsonid (jc);
        }
      else if ((jc = json_object_get (js, "tup")) != NULL
               && json_is_array (jc))
        {
          unsigned sz = json_array_size (jc);
          mo_sequencevalue_ty *seq = mo_sequence_allocate (sz);
          for (unsigned ix = 0; ix < sz; ix++)
            {
              mo_objref_t obr = mo_objref_of_jsonid (json_array_get (jc, ix));
              seq->mo_seqobj[ix] = obr;
            }
          return mo_make_tuple_closeq (seq);
        }
      else if ((jc = json_object_get (js, "set")) != NULL
               && json_is_array (jc))
        {
          unsigned sz = json_array_size (jc);
          mo_sequencevalue_ty *seq = mo_sequence_allocate (sz);
          for (unsigned ix = 0; ix < sz; ix++)
            {
              mo_objref_t obr = mo_objref_of_jsonid (json_array_get (jc, ix));
              seq->mo_seqobj[ix] = obr;
            }
          return mo_make_set_closeq (seq);
        }
    }
  MOM_WARNPRINTF ("wrong json %s", json_dumps (js, JSON_SORT_KEYS));
  return NULL;
}                               /* end of mo_value_of_json */
