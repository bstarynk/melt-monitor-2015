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


/// the choice of Jansson is debatable
/// see https://github.com/miloyip/nativejson-benchmark

#include "meltmoni.h"

#define MOM_FORMAT_VERSION_PARAM "monimelt_format_version"

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
  mo_value_t mo_ld_sqlpathv;
  mo_value_t mo_ld_sqlitepathv;
  sqlite3 *mo_ld_db;
  unsigned mo_ld_nbobjects;
  unsigned mo_ld_nbnamed;
  unsigned mo_ld_nbmodules;
  void **mo_ld_modynharr;       /* array of module dlopen handles */
  mo_objref_t *mo_ld_modobjarr; /* array of module objects */
  mo_assovaldatapayl_ty *mo_ld_modassonum;      /* association moduleobj->modulerank */
  mo_hashsetpayl_ty *mo_ld_hsetobjects;
  // SQLstmt: SELECT par_value FROM t_params WHERE par_name = ?;
  sqlite3_stmt *mo_ld_stmt_params;
};



enum mom_dumpstate_en
{ MOMDUMP_NONE, MOMDUMP_SCAN, MOMDUMP_EMIT };
#define MOM_DUMPER_MAGIC  0x372bb699
struct mo_dumper_st             // stack allocated
{
  unsigned mo_du_magic;         /* always MOM_DUMPER_MAGIC */
  enum mom_dumpstate_en mo_du_state;
  double mo_du_startelapsedtime;
  double mo_du_startcputime;
  sqlite3 *mo_du_db;
  // SQLstmt: INSERT INTO t_params (par_name, par_value)
  sqlite3_stmt *mo_du_stmt_params;
  /** SQLstmt: INSERT INTO t_objects (ob_id, ob_mtime, ob_jsoncont, ob_classid,
      ob_paylkid, ob_paylcont, ob_paylmod)
  **/
  sqlite3_stmt *mo_du_stmt_objects;
  // SQLstmt: INSERT INTO t_names (nam_str, nam_oid)
  sqlite3_stmt *mo_du_stmt_names;
  //
  const char *mo_du_dirv;
  mo_value_t mo_du_tempsufv;
  mo_hashsetpayl_ty *mo_du_objset;      /* the set of reachable & emittable objects */
  mo_hashsetpayl_ty *mo_du_moduleset;   /* the set of modules */
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
  int nok = sqlite3_open_v2 (mo_string_cstr (sqlpathbufv), &du->mo_du_db,
                             SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE
                             | SQLITE_OPEN_PRIVATECACHE | SQLITE_OPEN_NOMUTEX,
                             NULL);
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
                    "  ob_jsoncont TEXT NOT NULL,"
                    "  ob_classid VARCHAR(20) NOT NULL,"
                    "  ob_paylkid VARCHAR(20) NOT NULL,"
                    "  ob_paylcont TEXT NOT NULL,"
                    "  ob_paylmod VARCHAR(20) NOT NULL)", NULL, NULL,
                    &errmsg))
    MOM_FATAPRINTF ("Failed to create t_objects Sqlite table: %s", errmsg);
  //
  if ((errmsg = NULL),          //
      sqlite3_exec (du->mo_du_db,
                    "CREATE TABLE t_names"
                    " (nam_str PRIMARY KEY ASC NOT NULL UNIQUE,"
                    "  nam_oid VARCHAR(20) NOT NULL UNIQUE)",
                    NULL, NULL, &errmsg))
    MOM_FATAPRINTF ("Failed to create t_names Sqlite table: %s", errmsg);
  if ((errmsg = NULL),          //
      sqlite3_exec (du->mo_du_db,
                    "CREATE TABLE t_modules"
                    " (mod_oid VARCHAR(20) PRIMARY KEY ASC NOT NULL UNIQUE,"
                    "  mod_cflags TEXT NOT NULL, mod_ldflags TEXT NOT NULL)",
                    NULL, NULL, &errmsg))
    MOM_FATAPRINTF ("Failed to create t_modules Sqlite table: %s", errmsg);
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
                                " (ob_id, ob_mtime, ob_jsoncont, ob_classid,"
                                "  ob_paylkid, ob_paylcont, ob_paylmod)"
                                " VALUES (?, ?, ?, ?, ?, ?, ?)", -1,
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
  mo_dump_param (du, MOM_FORMAT_VERSION_PARAM, MOM_DUMP_VERSIONID);
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
  { MOMOBJIX__NONE, MOMOBJIX_ID, MOMOBJIX_MTIME, MOMOBJIX_JSONCONT,
    MOMOBJIX_CLASSID,
    MOMOBJIX_PAYLKID, MOMOBJIX_PAYLCONT, MOMOBJIX_PAYLMOD
  };
  MOM_ASSERTPRINTF (du && du->mo_du_magic == MOM_DUMPER_MAGIC
                    && du->mo_du_state == MOMDUMP_EMIT
                    && du->mo_du_stmt_objects, "invalid dumper");
  if (!mo_dyncast_objref (obr) || !mo_dump_is_emitted_objref (du, obr))
    return;
  char bufid[MOM_CSTRIDSIZ];
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
      char clabufid[MOM_CSTRIDSIZ];
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
      char pkbufid[MOM_CSTRIDSIZ];
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
      void *payldata = obr->mo_ob_payldata;
      // should be statically filled, or garbage collected GC_strdup-ed strings
      const char *paylcontstr = NULL;
      const char *paylkindstr = NULL;
      const char *paylmodustr = NULL;
      if (paylkindobr->mo_ob_class == MOM_PREDEF (signature_class))
        {
          Dl_info dif;
          memset (&dif, 0, sizeof (dif));
          if (dladdr (payldata, &dif) && payldata == dif.dli_saddr)
            {
              int snamelen = 0;
              if (dif.dli_sname
                  &&
                  (!strncmp
                   (dif.dli_sname, MOM_FUNC_PREFIX, strlen (MOM_FUNC_PREFIX))
                   || !strncmp (dif.dli_sname, MOM_CODE_PREFIX,
                                strlen (MOM_FUNC_PREFIX)))
                  && (snamelen = strlen (dif.dli_sname)) > MOM_CSTRIDLEN
                  && !strcmp (dif.dli_sname + snamelen - MOM_CSTRIDLEN, bufid)
                  && dif.dli_fname)
                {
                  char modulidstr[MOM_CSTRIDSIZ];
                  mo_objref_t modulobr = NULL;
                  mo_hid_t modulhid = 0;
                  mo_loid_t modulloid = 0;
                  paylcontstr = ".";
                  paylkindstr = pkbufid;
                  char *lastslash = strrchr (dif.dli_fname, '/');
                  int pos = -1;
                  memset (modulidstr, 0, sizeof (modulidstr));
                  if (lastslash
                      && lastslash >
                      dif.dli_fname + sizeof (MOM_MODULES_DIR) + 1
                      && strstr (dif.dli_fname, MOM_MODULES_DIR)
                      && sscanf (lastslash + 1,
                                 MOM_MODULE_INFIX MOM_CSTRIDSCANF
                                 MOM_MODULE_SUFFIX "%n", modulidstr, &pos)
                      && pos > (int) MOM_CSTRIDLEN
                      && mo_get_hi_lo_ids_from_cstring (&modulhid, &modulloid,
                                                        modulidstr)
                      && (modulobr =
                          mo_objref_find_hid_loid (modulhid,
                                                   modulloid)) != NULL)
                    paylmodustr = modulidstr;
                  else
                    paylmodustr = ".";
                  paylcontstr = "!";
                }
            }
        }
      else
        {
          json_t *js = NULL;
#define MOM_NBCASE_PAYLOAD 307
#define CASE_PAYLOAD_MOM(Ob) momphash_##Ob % MOM_NBCASE_PAYLOAD:	\
	  if (paylkindobr != MOM_PREDEF(Ob)) goto defaultpayloadcase;	\
	  goto labpayl_##Ob; labpayl_##Ob
          switch (mo_objref_hash (paylkindobr) % MOM_NBCASE_PAYLOAD)
            {
            case CASE_PAYLOAD_MOM (payload_assoval):
              js = mo_dump_json_of_assoval (du,
                                            (mo_assovaldatapayl_ty *)
                                            payldata);
              break;
            case CASE_PAYLOAD_MOM (payload_hashset):
              js = mo_dump_json_of_hashset (du,
                                            (mo_hashsetpayl_ty *) payldata);
              break;
            case CASE_PAYLOAD_MOM (payload_list):
              js = mo_dump_json_of_list (du, (mo_listpayl_ty *) payldata);
              break;
            case CASE_PAYLOAD_MOM (payload_vectval):
              js = mo_dump_json_of_vectval (du,
                                            (mo_vectvaldatapayl_ty *)
                                            payldata);
              break;
            case CASE_PAYLOAD_MOM (payload_value):
              js = mo_dump_json_of_value (du, (mo_value_t) payldata);
              break;
            case CASE_PAYLOAD_MOM (payload_file):
              js = NULL;
              break;
            case CASE_PAYLOAD_MOM (payload_gobject):
              js = NULL;
              break;
            case CASE_PAYLOAD_MOM (payload_buffer):
              {
                extern json_t *mo_dump_json_for_buffer_objref (mo_dumper_ty *,
                                                               mo_objref_t);
                js = mo_dump_json_for_buffer_objref (du, obr);
              }
              break;
            case CASE_PAYLOAD_MOM (payload_json):
              if (payldata)
                js = json_pack ("{so}", "json", (json_t *) payldata);
              break;
            default:
            defaultpayloadcase:
              break;
            }
#undef MOM_NBCASE_PAYLOAD
#undef CASE_PAYLOAD_MOM
          if (js != NULL)
            {
              size_t jsonsiz = 2048;
              char *jsonbuf = calloc (jsonsiz, 1);
              if (!jsonbuf)
                MOM_FATAPRINTF ("failed to allocate jsonbuf of %zd bytes",
                                jsonsiz);
              FILE *jsonfil = open_memstream (&jsonbuf, &jsonsiz);
              if (!jsonfil)
                MOM_FATAPRINTF ("failed to open_memstream json");
              fputc ('\n', jsonfil);
              if (json_dumpf (js, jsonfil, JSON_INDENT (1) | JSON_SORT_KEYS))
                MOM_FATAPRINTF
                  ("failed to json_dumpf for payload of object %s",
                   mo_object_pnamestr (obr));
              fputc ('\n', jsonfil);
              long jsonlen = ftell (jsonfil);
              fflush (jsonfil);
              if (jsonlen > 16 * (long) MOM_SIZE_MAX)
                MOM_FATAPRINTF ("too big json for payload of %s kind %s",
                                mo_object_pnamestr (obr),
                                mo_object_pnamestr (paylkindobr));
              paylcontstr = mom_gc_alloc_scalar (jsonlen + 1);
              memcpy ((char *) paylcontstr, jsonbuf, jsonlen);
              fclose (jsonfil);
              free (jsonbuf), jsonbuf = NULL;
            }
        }
      if (!paylcontstr || !paylkindstr || !paylmodustr)
        goto nopayload;
      rc =
        sqlite3_bind_text (du->mo_du_stmt_objects, MOMOBJIX_PAYLKID,
                           paylkindstr, -1, SQLITE_STATIC);
      if (rc)
        MOM_FATAPRINTF
          ("failed to bind ob_paylkid for t_objects insert Sqlite3 statment (%s)",
           sqlite3_errstr (rc));
      rc =
        sqlite3_bind_text (du->mo_du_stmt_objects, MOMOBJIX_PAYLCONT,
                           paylcontstr, -1, SQLITE_STATIC);
      if (rc)
        MOM_FATAPRINTF
          ("failed to bind ob_paylcont for t_objects insert Sqlite3 statment (%s)",
           sqlite3_errstr (rc));
      rc =
        sqlite3_bind_text (du->mo_du_stmt_objects, MOMOBJIX_PAYLMOD,
                           paylmodustr, -1, SQLITE_STATIC);
      if (rc)
        MOM_FATAPRINTF
          ("failed to bind ob_paylmod for t_objects insert Sqlite3 statment (%s)",
           sqlite3_errstr (rc));
    }
  else
  nopayload:
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
      rc =
        sqlite3_bind_text (du->mo_du_stmt_objects, MOMOBJIX_PAYLMOD, "", -1,
                           SQLITE_STATIC);
      if (rc)
        MOM_FATAPRINTF
          ("failed to empty-bind ob_paylmod for t_objects insert Sqlite3 statment (%s)",
           sqlite3_errstr (rc));
    }
  // now construct the JSON object for the content and bind ob_jsoncont
  mo_json_t jattrs = mo_dump_json_of_assoval (du, obr->mo_ob_attrs);
  mo_json_t jcomps = mo_dump_json_of_vectval (du, obr->mo_ob_comps);
  mo_json_t jcont = NULL;
  mo_value_t namev = mo_get_namev (obr);
  if (namev)
    {
      // the @name field is emitted for convenience, in the case
      // patching manually the SQL dump file is needed
      jcont = json_pack ("{sssoso}", "@name", mo_string_cstr (namev),
                         "attrs", jattrs, "comps", jcomps);
    }
  else
    {
      jcont = json_pack ("{soso}", "attrs", jattrs, "comps", jcomps);
    }
  size_t contsiz = 4096;
  char *contbuf = calloc (1, contsiz);
  if (MOM_UNLIKELY (!contbuf))
    MOM_FATAPRINTF ("failed to allocate memstream buffer of %zd", contsiz);
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
  static long scancnt;
  scancnt++;
  MOM_ASSERTPRINTF (du && du->mo_du_magic == MOM_DUMPER_MAGIC
                    && du->mo_du_state == MOMDUMP_SCAN, "bad dumper du@%p",
                    du);
  MOM_ASSERTPRINTF (mo_dyncast_objref (obr), "bad obr");
  if (mo_objref_space (obr) == mo_SPACE_NONE)
    return;
  mo_dump_scan_objref (du, obr->mo_ob_class);
  if (obr->mo_ob_attrs)
    mo_dump_scan_assoval (du, obr->mo_ob_attrs);
  if (obr->mo_ob_comps)
    mo_dump_scan_vectval (du, obr->mo_ob_comps);
  if (obr->mo_ob_class == MOM_PREDEF (module_class))
    du->mo_du_moduleset = mo_hashset_put (du->mo_du_moduleset, obr);
  mo_objref_t obrpayk = obr->mo_ob_paylkind;
  if (obrpayk != NULL)
    {
      const void *payldata = obr->mo_ob_payldata;
      MOM_ASSERTPRINTF (mo_dyncast_objref (obrpayk),
                        "bad obrpayk@%p for obr@%p=%s",
                        obrpayk, obr, mo_object_pnamestr (obr));
      mo_dump_scan_objref (du, obrpayk);
      if (obrpayk->mo_ob_class == MOM_PREDEF (signature_class))
        {
          Dl_info dif;
          memset (&dif, 0, sizeof (dif));
          if (dladdr (payldata, &dif))
            {
              mo_objref_t modulobr = NULL;
              mo_hid_t modulhid = 0;
              mo_loid_t modulloid = 0;
              char modulidstr[MOM_CSTRIDSIZ];
              memset (modulidstr, 0, sizeof (modulidstr));
              char funidstr[MOM_CSTRIDSIZ];
              memset (funidstr, 0, sizeof (funidstr));
              char obridstr[MOM_CSTRIDSIZ];
              memset (obridstr, 0, sizeof (obridstr));
              mo_cstring_from_hi_lo_ids (obridstr, obr->mo_ob_hid,
                                         obr->mo_ob_loid);
              int posmod = -1;
              int posfun = -1;
              // dli_sname is the symbol name; dli_fname is the file path
              if (dif.dli_sname && dif.dli_saddr == payldata && dif.dli_fname)
                {
                  MOM_WARNPRINTF
                    ("obr@%p=%s with payldata %p payk %s dli_sname %s dli_fname %s",
                     obr, mo_object_pnamestr (obr), payldata,
                     mo_object_pnamestr (obrpayk), dif.dli_sname,
                     dif.dli_fname);
                  if (((sscanf
                        (dif.dli_sname, MOM_CODE_PREFIX MOM_CSTRIDSCANF "%n",
                         funidstr, &posfun) >= 1 && posfun > 0
                        && dif.dli_sname[posfun] == '\0')
                       ||
                       (sscanf
                        (dif.dli_sname, MOM_FUNC_PREFIX MOM_CSTRIDSCANF "%n",
                         funidstr, &posfun) >= 1 && posfun > 0
                        && dif.dli_sname[posfun] == '\0'))
                      && !strcmp (funidstr, obridstr))
                    {
                      const char *lastslash = strchr (dif.dli_fname, '/');
                      if (lastslash
                          && lastslash >
                          dif.dli_fname + sizeof (MOM_MODULES_DIR)
                          && !strcmp (lastslash - sizeof (MOM_MODULES_DIR),
                                      MOM_MODULES_DIR)
                          && sscanf (lastslash + sizeof (MOM_MODULES_DIR),
                                     MOM_MODULE_INFIX MOM_CSTRIDSCANF "%n",
                                     modulidstr, &posmod) >= 1 && posmod > 0
                          && lastslash[sizeof (MOM_MODULES_DIR) + posmod] ==
                          (char) 0
                          && mo_get_hi_lo_ids_from_cstring (&modulhid,
                                                            &modulloid,
                                                            modulidstr)
                          && (modulobr =
                              mo_objref_find_hid_loid (modulhid,
                                                       modulloid)) != NULL
                          && modulobr->mo_ob_class ==
                          MOM_PREDEF (module_class))
                        {
                          mo_dump_scan_objref (du, modulobr);
                          du->mo_du_moduleset =
                            mo_hashset_put (du->mo_du_moduleset, modulobr);
                        }
                    }
                }
              else
                MOM_FATAPRINTF ("obr@%p=%s with unamed payldata@%p payk %s",
                                obr, mo_object_pnamestr (obr), payldata,
                                mo_object_pnamestr (obrpayk));
            }
          else
            MOM_FATAPRINTF
              ("dladdr failed for scan inside obr@%p=%s payload@%p/kd.%s",
               obr, mo_object_pnamestr (obr), payldata,
               mo_object_pnamestr (obrpayk));
        }
      else
        {
#define MOM_NBCASE_PAYLOAD 307
#define CASE_PAYLOAD_MOM(Ob) momphash_##Ob % MOM_NBCASE_PAYLOAD:	\
	  if (obrpayk != MOM_PREDEF(Ob)) goto defaultpayloadcase;	\
	  goto labpayl_##Ob; labpayl_##Ob
          switch (mo_objref_hash (obrpayk) % MOM_NBCASE_PAYLOAD)
            {
            case CASE_PAYLOAD_MOM (payload_assoval):
              mo_dump_scan_assoval (du, (mo_assovaldatapayl_ty *) payldata);
              break;
            case CASE_PAYLOAD_MOM (payload_hashset):
              mo_dump_scan_hashset (du, (mo_hashsetpayl_ty *) payldata);
              break;
            case CASE_PAYLOAD_MOM (payload_list):
              mo_dump_scan_list (du, (mo_listpayl_ty *) payldata);
              break;
            case CASE_PAYLOAD_MOM (payload_vectval):
              mo_dump_scan_vectval (du, (mo_vectvaldatapayl_ty *) payldata);
              break;
            case CASE_PAYLOAD_MOM (payload_value):
              mo_dump_scan_value (du, (mo_value_t) payldata);
              break;
            case CASE_PAYLOAD_MOM (payload_file):
              break;
            case CASE_PAYLOAD_MOM (payload_gobject):
              break;
            case CASE_PAYLOAD_MOM (payload_buffer):
              break;
            case CASE_PAYLOAD_MOM (payload_json):
              break;
            default:
            defaultpayloadcase:
              break;
            }
#undef MOM_NBCASE_PAYLOAD
#undef CASE_PAYLOAD_MOM
        }
    }
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

int
mom_predefsort_cmp (const void *p1, const void *p2)
{
  mo_objref_t ob1 = *(mo_objref_t *) p1;
  mo_objref_t ob2 = *(mo_objref_t *) p2;
  MOM_ASSERTPRINTF (mo_objref_space (ob1) == mo_SPACE_PREDEF, "bad ob1");
  MOM_ASSERTPRINTF (mo_objref_space (ob2) == mo_SPACE_PREDEF, "bad ob2");
  mo_value_t nam1 = mo_get_namev (ob1);
  mo_value_t nam2 = mo_get_namev (ob2);
  if (nam1 && nam2)
    {
      MOM_ASSERTPRINTF (mo_dyncast_string (nam1), "bad nam1");
      MOM_ASSERTPRINTF (mo_dyncast_string (nam2), "bad nam2");
      return strcmp (mo_string_cstr (nam1), mo_string_cstr (nam2));
    }
  else if (nam1)
    return 1;
  else if (nam2)
    return -1;
  else
    return mo_objref_cmp (ob1, ob2);
}                               /* end mom_predefsort_cmp */

void
mo_dump_emit_predefined (mo_dumper_ty * du, mo_value_t predset)
{
  MOM_ASSERTPRINTF (du && du->mo_du_magic == MOM_DUMPER_MAGIC
                    && du->mo_du_state == MOMDUMP_EMIT, "bad dumper du@%p",
                    du);
  int nbpredef = mo_set_size (predset);
  MOM_ASSERTPRINTF (nbpredef > 0, "empty predset");
  mo_objref_t *predarr =
    mom_gc_alloc (((nbpredef | 0xf) + 1) * sizeof (mo_objref_t));
  memcpy (predarr, ((mo_sequencevalue_ty *) predset)->mo_seqobj,
          nbpredef * sizeof (mo_objref_t));
  qsort (predarr, nbpredef, sizeof (mo_objref_t), mom_predefsort_cmp);
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
      mo_objref_t obp = predarr[ix];
      MOM_ASSERTPRINTF (mo_dyncast_objref (obp)
                        && mo_objref_space (obp) == mo_SPACE_PREDEF,
                        "bad obp");
      fputc ('\n', fp);
      mo_value_t namv = mo_get_namev (obp);
      char idstr[MOM_CSTRIDSIZ];
      memset (idstr, 0, sizeof (idstr));
      mo_cstring_from_hi_lo_ids (idstr, obp->mo_ob_hid, obp->mo_ob_loid);
      mo_value_t commv =
        mo_dyncast_string (mo_objref_get_attr (obp, MOM_PREDEF (comment)));
      if (commv)
        {
          const char *pc = mo_string_cstr (commv);
          int cnt = 0;
          fputs ("//+ ", fp);
          while (cnt < 80 && *pc && *pc != '\n')
            {
              const char *npc = g_utf8_next_char (pc);
              if (!npc)
                break;
              fwrite (pc, 1, npc - pc, fp);
              pc = npc;
              cnt++;
            }
          fputc ('\n', fp);
        }
      else
        {
          fputs ("//-\n", fp);
        }
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
  fputs ("\n\n", fp);
  for (int ix = 0; ix < nbpredef; ix++)
    {
      mo_objref_t obp = predarr[ix];
      mo_value_t namv = mo_get_namev (obp);
      char idstr[MOM_CSTRIDSIZ];
      memset (idstr, 0, sizeof (idstr));
      mo_cstring_from_hi_lo_ids (idstr, obp->mo_ob_hid, obp->mo_ob_loid);
      if (namv != NULL)
        {
          const char *namstr = mo_string_cstr (namv);
          MOM_ASSERTPRINTF (namstr != NULL
                            && isalpha (namstr[0]), "bad namv@%p", namv);
          fprintf (fp, "\n#undef moid_%s\n" "#define moid_%s %s\n", namstr,
                   namstr, idstr);
          fprintf (fp, "#undef monam%s\n" "#define monam%s %s\n", idstr,
                   idstr, namstr);
        }
    };
  fputs ("\n\n", fp);
  fputs ("#ifndef MOM_PREDEFINED_HASHES\n"
         "#define MOM_PREDEFINED_HASHES 1\n"
         "enum mom_predefined_hashes_en {\n", fp);
  for (int ix = 0; ix < nbpredef; ix++)
    {
      mo_objref_t obp = predarr[ix];
      mo_value_t namv = mo_get_namev (obp);
      char idstr[MOM_CSTRIDSIZ];
      memset (idstr, 0, sizeof (idstr));
      if (namv != NULL)
        {
          MOM_ASSERTPRINTF (mo_dyncast_string (namv), "bad namv");
          fprintf (fp, "  momphash_%s=%u,\n",
                   mo_string_cstr (namv), (unsigned) mo_objref_hash (obp));
        }
      else
        {
          mo_cstring_from_hi_lo_ids (idstr, obp->mo_ob_hid, obp->mo_ob_loid);
          fprintf (fp, "  momphash%s=%u,\n",
                   idstr, (unsigned) mo_objref_hash (obp));
        };
    };
  fputs ("}; // end mom_predefined_hashes_en\n", fp);
  fputs ("#endif /*MOM_PREDEFINED_HASHES */\n", fp);
  fputs ("\n\n", fp);
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
      char idbuf[MOM_CSTRIDSIZ];
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
                    && du->mo_du_state == MOMDUMP_EMIT, "bad dumper du@%p",
                    du);
  unsigned nbfil = mo_vectval_count (du->mo_du_vectfilepath);
  unsigned nbsamefiles = 0;
  for (unsigned ix = 0; ix < nbfil; ix++)
    {
      mo_value_t curbasev = mo_vectval_nth (du->mo_du_vectfilepath, ix);
      MOM_ASSERTPRINTF (mo_dyncast_string (curbasev), "bad curbasev");
      const char *curbastr = mo_string_cstr (curbasev);
      MOM_ASSERTPRINTF (curbastr
                        && (isalnum (curbastr[0])
                            || curbastr[0] == '_'), "bad curbastr %s",
                        curbastr);
      mo_value_t curpathv =     //
        mo_make_string_sprintf ("%s/%s",
                                mo_string_cstr (du->mo_du_dirv),
                                curbastr);
      mo_value_t tmpathv =      //
        mo_make_string_sprintf ("%s/%s%s",
                                mo_string_cstr (du->mo_du_dirv),
                                curbastr,
                                mo_string_cstr (du->mo_du_tempsufv));
      mo_value_t backupv =      //
        mo_make_string_sprintf ("%s/%s~",
                                mo_string_cstr (du->mo_du_dirv),
                                curbastr);
      bool samefilecont = false;
      curbastr = NULL;
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
mo_dump_symlink_needed_file (mo_dumper_ty * du, const char *filnam)
{
  MOM_ASSERTPRINTF (du && du->mo_du_magic == MOM_DUMPER_MAGIC
                    && du->mo_du_state == MOMDUMP_EMIT, "bad dumper du@%p",
                    du);
  MOM_ASSERTPRINTF (filnam && isalnum (filnam[0]), "bad filnam %s", filnam);
  mo_value_t srcpathv =         //
    mo_make_string_sprintf ("%s/%s",
                            monimelt_directory, filnam);
  mo_value_t sympathv =         //
    mo_make_string_sprintf ("%s/%s",
                            mo_string_cstr (du->mo_du_dirv), filnam);
  if (access (mo_string_cstr (sympathv), F_OK) && errno == ENOENT)
    {
      errno = 0;
      if (symlink (mo_string_cstr (srcpathv), mo_string_cstr (sympathv)))
        MOM_FATAPRINTF ("failed to symlink: %s -> %s",
                        mo_string_cstr (sympathv), mo_string_cstr (srcpathv));
      else
        MOM_INFORMPRINTF ("dump symlinked: %s -> %s",
                          mo_string_cstr (sympathv),
                          mo_string_cstr (srcpathv));
    }
}                               /* end of mo_dump_symlink_needed_file */

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
  for (const char *pc = dirname; *pc; pc++)
    if (!isprint (*pc) && !isalnum (*pc) && !strchr ("/.+-%,:_", *pc))
      MOM_FATAPRINTF ("forbidden character %c in dump directory %s", *pc,
                      dirname);
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
  dumper.mo_du_startelapsedtime = mom_elapsed_real_time ();
  dumper.mo_du_startcputime = mom_process_cpu_time ();
  dumper.mo_du_db = NULL;
  dumper.mo_du_dirv = mo_make_string_cstr (dirname);
  dumper.mo_du_tempsufv =
    mo_make_string_sprintf (".tmp_%lx_%lx_p%d%%",
                            momrand_genrand_int31 (),
                            momrand_genrand_int31 (), (int) getpid ());
  dumper.mo_du_objset = mo_hashset_reserve (NULL, 4 * nbpredef + 100);
  dumper.mo_du_moduleset = mo_hashset_reserve (NULL, 2 * nbpredef + 20);
  dumper.mo_du_scanlist = mo_list_make ();
  dumper.mo_du_vectfilepath = mo_vectval_reserve (NULL, 20 + nbpredef / 5);
  for (int ix = 0; ix < nbpredef; ix++)
    {
      mo_objref_t predobr = mo_set_nth (predefset, ix);
      MOM_ASSERTPRINTF (mo_dyncast_objref (predobr), "bad predobr ix=%d", ix);
      mo_dump_scan_objref (&dumper, predobr);
      MOM_ASSERTPRINTF (mo_objref_space (predobr) == mo_SPACE_PREDEF,
                        "non predef ix=%d predobr@%p", ix, predobr);
    };
  long nbobj = nbpredef;
  double lastbelltime = mom_elapsed_real_time ();
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
      if (MOM_UNLIKELY (nbobj % 1024 == 0))
        {
          dumper.mo_du_objset =
            mo_hashset_reserve (dumper.mo_du_objset, nbobj / 3 + 200);
          unsigned nbmodu = mo_hashset_count (dumper.mo_du_moduleset);
          dumper.mo_du_moduleset =
            mo_hashset_reserve (dumper.mo_du_moduleset, nbmodu / 5 + 20);
          if (mom_elapsed_real_time () > lastbelltime + 3.5)
            {
              MOM_INFORMPRINTF ("scanned %ld objects in %.3f sec...", nbobj,
                                mom_elapsed_real_time () -
                                dumper.mo_du_startelapsedtime);
              lastbelltime = mom_elapsed_real_time ();
            }
        };
    }
  double scanelapsedtime = mom_elapsed_real_time ();
  double scancputime = mom_process_cpu_time ();
  MOM_INFORMPRINTF ("dump scanned %ld objects\n"
                    ".. in %.4f (%.3f µs/ob) elapsed %.4f (%.3f µs/ob) cpu seconds",
                    nbobj,
                    (scanelapsedtime - dumper.mo_du_startelapsedtime),
                    1.0e6 * (scanelapsedtime -
                             dumper.mo_du_startelapsedtime) / nbobj,
                    (scancputime - dumper.mo_du_startcputime),
                    1.0e6 * (scancputime -
                             dumper.mo_du_startcputime) / nbobj);
  /// the emit loop
  dumper.mo_du_state = MOMDUMP_EMIT;
  mo_dump_initialize_sqlite_database (&dumper);
  mo_dump_emit_predefined (&dumper, predefset);
  mo_value_t elset = mo_hashset_elements_set (dumper.mo_du_objset);
  unsigned elsiz = mo_set_size (elset);
  char *errmsg = NULL;
  if ((errmsg = NULL),          //
      sqlite3_exec (dumper.mo_du_db,
                    "BEGIN TRANSACTION;", NULL, NULL, &errmsg))
    MOM_FATAPRINTF ("Failed to BEGIN Sqlite transaction: %s", errmsg);
  lastbelltime = mom_elapsed_real_time ();
  MOM_ASSERTPRINTF (elsiz >= (unsigned) nbpredef,
                    "bad elsiz %u nbpredef %u", elsiz, nbpredef);
  for (unsigned eix = 0; eix < elsiz; eix++)
    {
      mo_objref_t obr = mo_set_nth (elset, eix);
      MOM_ASSERTPRINTF (mo_dyncast_objref (obr), "bad obr@%p", obr);
      mo_dump_emit_object_content (&dumper, obr);
      if (MOM_UNLIKELY (eix % 65536 == 0))
        {
          if ((errmsg = NULL),  //
              sqlite3_exec (dumper.mo_du_db,
                            "END TRANSACTION;", NULL, NULL, &errmsg))
            MOM_FATAPRINTF ("Failed to END Sqlite transaction: %s", errmsg);
          if (mom_elapsed_real_time () > lastbelltime + 3.5)
            {
              MOM_INFORMPRINTF
                ("dumped %u objects in %.3f real %.3f cpu sec...", eix,
                 mom_elapsed_real_time () - dumper.mo_du_startelapsedtime,
                 mom_process_cpu_time () - dumper.mo_du_startcputime);
              lastbelltime = mom_elapsed_real_time ();
            }
          if ((errmsg = NULL),  //
              sqlite3_exec (dumper.mo_du_db,
                            "BEGIN TRANSACTION;", NULL, NULL, &errmsg))
            MOM_FATAPRINTF ("Failed to BEGIN Sqlite transaction: %s", errmsg);
        };
    }
  mo_dump_emit_names (&dumper);
  if ((errmsg = NULL),          //
      sqlite3_exec (dumper.mo_du_db, "END TRANSACTION;", NULL, NULL, &errmsg))
    MOM_FATAPRINTF ("Failed to END Sqlite transaction: %s", errmsg);
#warning mo_dump_state should emit modules
  MOM_WARNPRINTF ("mo_dump_state should emit the C code of %d modules",
                  mo_hashset_count (dumper.mo_du_moduleset));
  mo_dump_end_database (&dumper);
  mo_dump_rename_emitted_files (&dumper);
  mo_dump_symlink_needed_file (&dumper, "Makefile");
  mo_dump_symlink_needed_file (&dumper, MONIMELT_HEADER);
  for (const char *const *pfilnam = monimelt_csources;
       pfilnam && *pfilnam; pfilnam++)
    mo_dump_symlink_needed_file (&dumper, *pfilnam);
  for (const char *const *pfilnam = monimelt_shellsources;
       pfilnam && *pfilnam; pfilnam++)
    mo_dump_symlink_needed_file (&dumper, *pfilnam);
  mo_value_t sqlpathv =         //
    mo_make_string_sprintf ("%s/%s.sql",
                            mo_string_cstr (dumper.mo_du_dirv),
                            monimelt_perstatebase);
  errno = 0;
  mo_value_t sqlitepathv =      //
    mo_make_string_sprintf ("%s/%s.sqlite",
                            mo_string_cstr (dumper.mo_du_dirv),
                            monimelt_perstatebase);
  mo_value_t cmdv =             //
    mo_make_string_sprintf ("%s/%s %s %s",
                            mo_string_cstr (dumper.mo_du_dirv),
                            MOM_DUMP_SCRIPT,
                            mo_string_cstr (sqlitepathv),
                            mo_string_cstr (sqlpathv));

  MOM_INFORMPRINTF
    ("SQL file %s missing, dumping it...\n ... using: %s",
     mo_string_cstr (sqlpathv), mo_string_cstr (cmdv));
  fflush (NULL);
  int rc = system (mo_string_cstr (cmdv));
  if (rc)
    MOM_FATAPRINTF ("dump command %s failed %d", mo_string_cstr (cmdv), rc);
  MOM_INFORMPRINTF ("Sqlite base %s dumped into %s",
                    mo_string_cstr (sqlitepathv), mo_string_cstr (sqlpathv));
  double endelapsedtime = mom_elapsed_real_time ();
  double endcputime = mom_process_cpu_time ();
  char *realdirname = realpath (dirname, NULL);
  MOM_INFORMPRINTF ("dumped %u objects in %s directory\n"
                    ".. (real path %s ...)\n"
                    ".. in %.4f (%.3f µs/ob) elapsed %.4f (%.3f µs/ob) cpu seconds\n",
                    elsiz,
                    dirname, realdirname,
                    (endelapsedtime - dumper.mo_du_startelapsedtime),
                    1.0e6 * (endelapsedtime - dumper.mo_du_startelapsedtime)
                    / elsiz,
                    (endcputime - dumper.mo_du_startcputime),
                    1.0e6 * (endcputime - dumper.mo_du_startcputime) / elsiz);
  free (realdirname);
  memset (&dumper, 0, sizeof (dumper));
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

// retrieve a load parameter as a string value or else NULL
mo_value_t
mo_loader_get_param_strv (mo_loader_ty * ld, const char *parname)
{
  enum
  { MOMGETPARAMIX__NONE, MOMGETPARAMIX_PVAL };
  enum
  { MOMGETPARAM_RESIX };
  int rc = 0;
  mo_value_t resv = NULL;
  MOM_ASSERTPRINTF (ld && ld->mo_ld_magic == MOM_LOADER_MAGIC, "bad ld");
  if (!parname || parname == MOM_EMPTY_SLOT || !isalpha (parname[0]))
    return NULL;
  rc =
    sqlite3_bind_text (ld->mo_ld_stmt_params, MOMGETPARAMIX_PVAL, parname, -1,
                       SQLITE_STATIC);
  if (rc)
    MOM_FATAPRINTF
      ("failed to bind par_name %s to param select Sqlite3 statment (%s)",
       parname, sqlite3_errstr (rc));
  rc = sqlite3_step (ld->mo_ld_stmt_params);
  if (rc == SQLITE_ROW)
    {
      resv =
        mo_make_string_cstr ((const char *)
                             sqlite3_column_text (ld->mo_ld_stmt_params,
                                                  MOMGETPARAM_RESIX));
    }
  rc = sqlite3_reset (ld->mo_ld_stmt_params);
  if (rc != SQLITE_OK)
    MOM_FATAPRINTF
      ("failed to reset select Sqlite3 statment for pname %s (%s)", parname,
       sqlite3_errstr (rc));
  return resv;
}                               /* end of mo_loader_get_param_strv */


void
mo_loader_begin_database (mo_loader_ty * ld)
{
  int rc = 0;
  MOM_ASSERTPRINTF (ld && ld->mo_ld_magic == MOM_LOADER_MAGIC, "bad ld");
  struct stat statsql;
  memset (&statsql, 0, sizeof (statsql));
  struct stat statsqlite;
  memset (&statsqlite, 0, sizeof (statsqlite));
  MOM_ASSERTPRINTF (mo_dyncast_string (ld->mo_ld_sqlpathv), "bad sqlpathv");
  MOM_ASSERTPRINTF (mo_dyncast_string (ld->mo_ld_sqlitepathv),
                    "bad sqlitepathv");
  if (stat (mo_string_cstr (ld->mo_ld_sqlpathv), &statsql))
    MOM_FATAPRINTF ("failed to stat SQL file %s",
                    mo_string_cstr (ld->mo_ld_sqlpathv));
  if (stat (mo_string_cstr (ld->mo_ld_sqlitepathv), &statsqlite))
    MOM_FATAPRINTF ("failed to stat Ssqlite base %s",
                    mo_string_cstr (ld->mo_ld_sqlitepathv));
  if (!S_ISREG (statsql.st_mode))
    MOM_FATAPRINTF ("SQL file %s is not a plain file",
                    mo_string_cstr (ld->mo_ld_sqlpathv));
  if (!S_ISREG (statsqlite.st_mode))
    MOM_FATAPRINTF ("Sqlite base %s is not a plain file",
                    mo_string_cstr (ld->mo_ld_sqlitepathv));
  if (statsql.st_mtime > statsqlite.st_mtime)
    MOM_FATAPRINTF ("SQL file %s is younger (by %ld sec) that Sqlite base %s",
                    mo_string_cstr (ld->mo_ld_sqlpathv),
                    statsql.st_mtime - statsqlite.st_mtime,
                    mo_string_cstr (ld->mo_ld_sqlitepathv));
  rc = sqlite3_open_v2 (mo_string_cstr (ld->mo_ld_sqlitepathv),
                        &ld->mo_ld_db, SQLITE_OPEN_READONLY, NULL);
  if (rc != SQLITE_OK || !ld->mo_ld_db)
    MOM_FATAPRINTF ("failed to open loaded Sqlite base %s (%s)",
                    mo_string_cstr (ld->mo_ld_sqlitepathv),
                    sqlite3_errstr (rc));
  /* prepare statements */
  if ((rc = sqlite3_prepare_v2 (ld->mo_ld_db,
                                "SELECT par_value FROM t_params WHERE par_name = ?",
                                -1,
                                &ld->mo_ld_stmt_params, NULL)) != SQLITE_OK)
    MOM_FATAPRINTF
      ("Failed to prepare t_object Sqlite select par_value: %s",
       sqlite3_errstr (rc));
  /* check loader version */
  mo_value_t ldversv =
    mo_loader_get_param_strv (ld, MOM_FORMAT_VERSION_PARAM);
  if (!mo_dyncast_string (ldversv))
    MOM_FATAPRINTF ("missing %s in Sqlite loader base %s",
                    MOM_FORMAT_VERSION_PARAM,
                    mo_string_cstr (ld->mo_ld_sqlitepathv));
  if (strcmp (mo_string_cstr (ldversv), MOM_DUMP_VERSIONID))
    MOM_FATAPRINTF
      ("incompatible format version in Sqlite loader base %s - got %s expecting %s",
       mo_string_cstr (ld->mo_ld_sqlitepathv), mo_string_cstr (ldversv),
       MOM_DUMP_VERSIONID);
}                               /* end mo_loader_begin_database */

/* execute a simple SQL request (such as "SELECT COUNT(*) FROM
   t_objects") which gives a single integer, and return that integer;
   show a warning if no result is obtained */
long
mo_loader_exec_intreq (mo_loader_ty * ld, const char *reqs)
{
  long retnum = 0;
  int rc = 0;
  sqlite3_stmt *lstmt = NULL;
  bool gotres = false;
  MOM_ASSERTPRINTF (ld && ld->mo_ld_magic == MOM_LOADER_MAGIC, "bad ld");
  MOM_ASSERTPRINTF (reqs && reqs != MOM_EMPTY_SLOT
                    && isprint (reqs[0]), "bad reqs");
  if ((rc =
       sqlite3_prepare_v2 (ld->mo_ld_db, reqs, -1, &lstmt,
                           NULL)) != SQLITE_OK)
    MOM_FATAPRINTF ("Sqlite loader base %s with bad intreq %s (%s)",
                    mo_string_cstr (ld->mo_ld_sqlitepathv), reqs,
                    sqlite3_errstr (rc));
  rc = sqlite3_step (lstmt);
  if (rc == SQLITE_ROW)
    {
      int nbc = sqlite3_data_count (lstmt);
      if (nbc != 1)
        MOM_FATAPRINTF
          ("Sqlite loader base %s intreq %s dont give one column but %d",
           mo_string_cstr (ld->mo_ld_sqlitepathv), reqs, nbc);
      retnum = sqlite3_column_int64 (lstmt, 0);
      gotres = true;
      rc = sqlite3_step (lstmt);
    }
  if (rc != SQLITE_DONE)
    {
      MOM_FATAPRINTF
        ("Sqlite loader base %s intreq %s gives more than one result (%s)",
         mo_string_cstr (ld->mo_ld_sqlitepathv), reqs, sqlite3_errstr (rc));
    }
  rc = sqlite3_finalize (lstmt);
  lstmt = NULL;
  if (rc != SQLITE_OK)
    MOM_FATAPRINTF ("Sqlite loader base %s intreq %s failed to finalize (%s)",
                    mo_string_cstr (ld->mo_ld_sqlitepathv), reqs,
                    sqlite3_errstr (rc));
  if (!gotres)
    MOM_WARNPRINTF ("Sqlite loader base %s intreq %s did not give any result",
                    mo_string_cstr (ld->mo_ld_sqlitepathv), reqs);
  return retnum;
}                               /* end of mo_loader_exec_intreq */

void
mo_loader_create_objects (mo_loader_ty * ld)
{
  int rc = 0;
  MOM_ASSERTPRINTF (ld && ld->mo_ld_magic == MOM_LOADER_MAGIC, "bad ld");
  long nbobj = mo_loader_exec_intreq (ld, "SELECT COUNT(*) FROM t_objects");
  ld->mo_ld_nbobjects = nbobj;
  MOM_ASSERTPRINTF (nbobj >= MOM_NB_PREDEFINED, "too low nbobj=%ld", nbobj);
  ld->mo_ld_hsetobjects = mo_hashset_reserve (NULL, 5 * nbobj / 4 + 10);
  /* repeat: SELECT ob_id FROM t_objects */
  {
    sqlite3_stmt *oidstmt = NULL;
    enum
    { MOMRESIX_OID, MOMRESIX__LAST };
    if ((rc = sqlite3_prepare_v2 (ld->mo_ld_db,
                                  "SELECT ob_id FROM t_objects",
                                  -1, &oidstmt, NULL)) != SQLITE_OK)
      MOM_FATAPRINTF
        ("Sqlite loader base %s failed to prepare ob_id selection (%s)",
         mo_string_cstr (ld->mo_ld_sqlitepathv), sqlite3_errstr (rc));
    long obcnt = 0;
    while ((rc = sqlite3_step (oidstmt)) == SQLITE_ROW)
      {
        MOM_ASSERTPRINTF (sqlite3_data_count (oidstmt) == MOMRESIX__LAST
                          && sqlite3_column_type (oidstmt, MOMRESIX_OID)
                          == SQLITE_TEXT, "bad oidstmt step");
        const char *idstr =
          (const char *) sqlite3_column_text (oidstmt, MOMRESIX_OID);
        mo_hid_t hid = 0;
        mo_loid_t loid = 0;
        if (!mo_get_hi_lo_ids_from_cstring (&hid, &loid, idstr)
            || hid == 0 || loid == 0)
          MOM_FATAPRINTF ("Sqlite loader base %s with bad ob_id %s",
                          mo_string_cstr (ld->mo_ld_sqlitepathv), idstr);
        mo_objref_t obr = mo_objref_create_hid_loid (hid, loid);
        MOM_ASSERTPRINTF (obr != NULL, "failed to create from ob_id %s",
                          idstr);
        ld->mo_ld_hsetobjects = mo_hashset_put (ld->mo_ld_hsetobjects, obr);
        if (mo_objref_space (obr) == mo_SPACE_NONE)
          mo_objref_put_space (obr, mo_SPACE_GLOBAL);
        obcnt++;
      }                         /* end while rc... */
    if (rc != SQLITE_DONE)
      MOM_FATAPRINTF ("Sqlite loader base %s ob_id selection not done (%s)",
                      mo_string_cstr (ld->mo_ld_sqlitepathv),
                      sqlite3_errstr (rc));
    rc = sqlite3_finalize (oidstmt);
    oidstmt = NULL;
    if (rc != SQLITE_OK)
      MOM_FATAPRINTF
        ("Sqlite loader base %s ob_id selection unfinalized (%s)",
         mo_string_cstr (ld->mo_ld_sqlitepathv), sqlite3_errstr (rc));
    if (obcnt != nbobj)
      MOM_FATAPRINTF ("Sqlite loader base %s ob_id counted %ld got %ld...",
                      mo_string_cstr (ld->mo_ld_sqlitepathv), obcnt, nbobj);
  }                             /* Done: SELECT ob_id FROM t_object */
}                               /* end of mo_loader_create_objects */

void
mo_loader_link_modules (mo_loader_ty * ld)
{
  int rc = 0;
  MOM_ASSERTPRINTF (ld && ld->mo_ld_magic == MOM_LOADER_MAGIC, "bad ld");
  long nbmod = mo_loader_exec_intreq (ld, "SELECT COUNT(*) FROM t_modules");
  MOM_ASSERTPRINTF (nbmod >= 0
                    && nbmod < MOM_SIZE_MAX, "bad nbmod %ld", nbmod);
  ld->mo_ld_nbmodules = nbmod;
  ld->mo_ld_modynharr = mom_gc_alloc_scalar ((nbmod + 1) * sizeof (void *));
  ld->mo_ld_modobjarr = mom_gc_alloc ((nbmod + 1) * sizeof (mo_objref_t));
  ld->mo_ld_modassonum = mo_assoval_reserve (NULL, 4 * nbmod / 3 + 5);
  /* repeat: SELECT mod_oid FROM t_modules */
  sqlite3_stmt *modstmt = NULL;
  enum
  { MOMRESIX_OID, MOMRESIX__LAST };
  if ((rc = sqlite3_prepare_v2 (ld->mo_ld_db,
                                "SELECT mod_oid FROM t_modules",
                                -1, &modstmt, NULL)) != SQLITE_OK)
    MOM_FATAPRINTF
      ("Sqlite loader base %s failed to prepare mod_oid selection (%s)",
       mo_string_cstr (ld->mo_ld_sqlitepathv), sqlite3_errstr (rc));
  long modcnt = 0;
  while ((rc = sqlite3_step (modstmt)) == SQLITE_ROW)
    {
      MOM_ASSERTPRINTF (sqlite3_data_count (modstmt) == MOMRESIX__LAST
                        && sqlite3_column_type (modstmt, MOMRESIX_OID)
                        == SQLITE_TEXT, "bad module step");
      const char *modidstr =
        (const char *) sqlite3_column_text (modstmt, MOMRESIX_OID);
      mo_hid_t hid = 0;
      mo_loid_t loid = 0;
      if (!mo_get_hi_lo_ids_from_cstring (&hid, &loid, modidstr)
          || hid == 0 || loid == 0)
        MOM_FATAPRINTF ("Sqlite loader base %s with bad mod_oid %s",
                        mo_string_cstr (ld->mo_ld_sqlitepathv), modidstr);
      mo_objref_t modobr = mo_objref_find_hid_loid (hid, loid);
      if (modobr == NULL)
        MOM_FATAPRINTF ("Sqlite loader base %s mod_oid %s not found",
                        mo_string_cstr (ld->mo_ld_sqlitepathv), modidstr);
      char modulepathbuf[sizeof (MOM_MODULES_DIR) + MOM_CSTRIDSIZ + 12];
      memset (modulepathbuf, 0, sizeof (modulepathbuf));
      snprintf (modulepathbuf, sizeof (modulepathbuf), "%s/%s.so",
                MOM_MODULES_DIR, modidstr);
      char makecheckcmd[40 + sizeof (modulepathbuf)];
      snprintf (makecheckcmd, sizeof (makecheckcmd), "make -q %s",
                modulepathbuf);
      fflush (NULL);
      int mok = system (makecheckcmd);
      if (mok != 0)
        MOM_FATAPRINTF ("module '%s' should be remade (%s failed with %d)",
                        modidstr, makecheckcmd, mok);
      void *modlh = dlopen (modulepathbuf, RTLD_LAZY | RTLD_GLOBAL);
      if (!modlh)
        {
          MOM_WARNPRINTF ("Module '%s' failed to dlopen: %s ***@@@",
                          modidstr, dlerror ());
          continue;
        }
      ld->mo_ld_modynharr[modcnt] = modlh;
      ld->mo_ld_modobjarr[modcnt] = modobr;
      ld->mo_ld_modassonum = mo_assoval_put (ld->mo_ld_modassonum, modobr,
                                             mo_int_to_value (modcnt));
      modcnt++;
    }                           /* end while rc... */
  if (rc != SQLITE_DONE)
    MOM_FATAPRINTF ("Sqlite loader base %s mod_oid selection not done (%s)",
                    mo_string_cstr (ld->mo_ld_sqlitepathv),
                    sqlite3_errstr (rc));
  rc = sqlite3_finalize (modstmt);
  modstmt = NULL;
  if (rc != SQLITE_OK)
    MOM_FATAPRINTF
      ("Sqlite loader base %s mod_oid selection unfinalized (%s)",
       mo_string_cstr (ld->mo_ld_sqlitepathv), sqlite3_errstr (rc));
  if (modcnt < nbmod)
    MOM_WARNPRINTF ("Sqlite loader base %s mod_oid counted %ld got %ld...",
                    mo_string_cstr (ld->mo_ld_sqlitepathv), modcnt, nbmod);
}                               /* end mo_loader_link_modules */

void
mo_loader_name_objects (mo_loader_ty * ld)
{
  int rc = 0;
  MOM_ASSERTPRINTF (ld && ld->mo_ld_magic == MOM_LOADER_MAGIC, "bad ld");
  long nbnamed = mo_loader_exec_intreq (ld, "SELECT COUNT(*) FROM t_names");
  ld->mo_ld_nbnamed = nbnamed;
  MOM_ASSERTPRINTF (nbnamed <= ld->mo_ld_nbobjects,
                    "too big nbnamed %ld", nbnamed);
  mo_reserve_names (3 * nbnamed / 2 + MOM_NB_PREDEFINED);
  /* repeat: SELECT nam_oid, nam_str FROM t_names */
  sqlite3_stmt *namstmt = NULL;
  enum
  { MOMRESIX_OID, MOMRESIX_STR, MOMRESIX__LAST };
  if ((rc = sqlite3_prepare_v2 (ld->mo_ld_db,
                                "SELECT nam_oid, nam_str FROM t_names",
                                -1, &namstmt, NULL)) != SQLITE_OK)
    MOM_FATAPRINTF
      ("Sqlite loader base %s failed to prepare nam_oid,nam_str selection (%s)",
       mo_string_cstr (ld->mo_ld_sqlitepathv), sqlite3_errstr (rc));
  long namcnt = 0;
  while ((rc = sqlite3_step (namstmt)) == SQLITE_ROW)
    {
      MOM_ASSERTPRINTF (sqlite3_data_count (namstmt) == MOMRESIX__LAST
                        && sqlite3_column_type (namstmt, MOMRESIX_OID)
                        == SQLITE_TEXT
                        && sqlite3_column_type (namstmt, MOMRESIX_STR)
                        == SQLITE_TEXT, "bad name step");
      const char *oidstr =
        (const char *) sqlite3_column_text (namstmt, MOMRESIX_OID);
      const char *namstr =
        (const char *) sqlite3_column_text (namstmt, MOMRESIX_STR);
      mo_hid_t hid = 0;
      mo_loid_t loid = 0;
      if (!mo_get_hi_lo_ids_from_cstring (&hid, &loid, oidstr)
          || hid == 0 || loid == 0)
        MOM_FATAPRINTF ("Sqlite loader base %s with bad nam_oid %s",
                        mo_string_cstr (ld->mo_ld_sqlitepathv), oidstr);
      mo_objref_t obr = mo_objref_find_hid_loid (hid, loid);
      if (obr == NULL)
        MOM_FATAPRINTF ("Sqlite loader base %s nam_oid %s not found",
                        mo_string_cstr (ld->mo_ld_sqlitepathv), oidstr);
      if (mo_objref_space (obr) != mo_SPACE_PREDEF
          && !mo_register_name_string (obr, namstr))
        MOM_FATAPRINTF ("Sqlite loader base %s nam_oid %s naming %s failed",
                        mo_string_cstr (ld->mo_ld_sqlitepathv), oidstr,
                        namstr);
      namcnt++;
    }                           /* end while rc... */
  if (rc != SQLITE_DONE)
    MOM_FATAPRINTF ("Sqlite loader base %s nam_oid selection not done (%s)",
                    mo_string_cstr (ld->mo_ld_sqlitepathv),
                    sqlite3_errstr (rc));
  rc = sqlite3_finalize (namstmt);
  namstmt = NULL;
  if (rc != SQLITE_OK)
    MOM_FATAPRINTF
      ("Sqlite loader base %s nam_oid selection unfinalized (%s)",
       mo_string_cstr (ld->mo_ld_sqlitepathv), sqlite3_errstr (rc));
  if (namcnt != nbnamed)
    MOM_FATAPRINTF ("Sqlite loader base %s nam_oid counted %ld got %ld...",
                    mo_string_cstr (ld->mo_ld_sqlitepathv), namcnt, nbnamed);
  /* Done: SELECT nam_oid, nam_str FROM t_names */
}                               /* end mo_loader_name_objects */



void
mo_loader_fill_objects_contents (mo_loader_ty * ld)
{
  int rc = 0;
  MOM_ASSERTPRINTF (ld && ld->mo_ld_magic == MOM_LOADER_MAGIC, "bad ld");
  /* repeat: SELECT ob_id, ob_mtime, ob_jsoncont FROM t_objects */
  sqlite3_stmt *fillstmt = NULL;
  enum
  { MOMRESIX_OID, MOMRESIX_MTIME, MOMRESIX_JSCONT, MOMRESIX__LAST };
  if ((rc = sqlite3_prepare_v2 (ld->mo_ld_db,
                                "SELECT ob_id, ob_mtime, ob_jsoncont FROM t_objects",
                                -1, &fillstmt, NULL)) != SQLITE_OK)
    MOM_FATAPRINTF
      ("Sqlite loader base %s failed to prepare objectfill selection (%s)",
       mo_string_cstr (ld->mo_ld_sqlitepathv), sqlite3_errstr (rc));
  while ((rc = sqlite3_step (fillstmt)) == SQLITE_ROW)
    {
      MOM_ASSERTPRINTF (sqlite3_data_count (fillstmt) == MOMRESIX__LAST
                        && sqlite3_column_type (fillstmt, MOMRESIX_OID)
                        == SQLITE_TEXT
                        && (sqlite3_column_type (fillstmt, MOMRESIX_MTIME)
                            == SQLITE_INTEGER
                            || sqlite3_column_type (fillstmt,
                                                    MOMRESIX_MTIME) ==
                            SQLITE_FLOAT)
                        && sqlite3_column_type (fillstmt,
                                                MOMRESIX_JSCONT) ==
                        SQLITE_TEXT, "bad objfill step");
      const char *oidstr =
        (const char *) sqlite3_column_text (fillstmt, MOMRESIX_OID);
      double mtimf = sqlite3_column_double (fillstmt, MOMRESIX_MTIME);
      const char *jscontstr =
        (const char *) sqlite3_column_text (fillstmt, MOMRESIX_JSCONT);
      mo_hid_t hid = 0;
      mo_loid_t loid = 0;
      if (!mo_get_hi_lo_ids_from_cstring (&hid, &loid, oidstr)
          || hid == 0 || loid == 0)
        MOM_FATAPRINTF ("Sqlite loader base %s with bad ob_id  %s",
                        mo_string_cstr (ld->mo_ld_sqlitepathv), oidstr);
      mo_objref_t obr = mo_objref_find_hid_loid (hid, loid);
      if (obr == NULL)
        MOM_FATAPRINTF ("Sqlite loader base %s ob_id %s not found",
                        mo_string_cstr (ld->mo_ld_sqlitepathv), oidstr);
      json_error_t errj;
      memset (&errj, 0, sizeof (errj));
      json_t *jcont =
        json_loads (jscontstr, 0 /*perhaps JSON_REJECT_DUPLICATE */ ,
                    &errj);
      if (!jcont || !json_is_object (jcont))
        MOM_FATAPRINTF
          ("Sqlite loader base %s bad JSON content for ob_id %s (%s)",
           mo_string_cstr (ld->mo_ld_sqlitepathv), oidstr, errj.text);
      json_t *jattrs = json_object_get (jcont, "attrs");
      json_t *jcomps = json_object_get (jcont, "comps");
      obr->mo_ob_attrs = mo_assoval_of_json (jattrs);
      obr->mo_ob_comps = mo_vectval_of_json (jcomps);
      if (mtimf > 0.0)
        obr->mo_ob_mtime = (time_t) mtimf;
    }
  if (rc != SQLITE_DONE)
    MOM_FATAPRINTF ("Sqlite loader base %s content selection not done (%s)",
                    mo_string_cstr (ld->mo_ld_sqlitepathv),
                    sqlite3_errstr (rc));
  rc = sqlite3_finalize (fillstmt);
  fillstmt = NULL;
  if (rc != SQLITE_OK)
    MOM_FATAPRINTF
      ("Sqlite loader base %s objfill selection unfinalized (%s)",
       mo_string_cstr (ld->mo_ld_sqlitepathv), sqlite3_errstr (rc));
}                               /* end mo_loader_fill_objects_contents */



/// load the payload sitting in code, having a non-empty ob_paylmod
void
mo_loader_load_payload_code (mo_loader_ty * ld)
{
  int rc = 0;
  MOM_ASSERTPRINTF (ld && ld->mo_ld_magic == MOM_LOADER_MAGIC, "bad ld");
  /* repeat: SELECT ob_id, ob_paylkid, ob_paylcont, ob_paylmod FROM t_objects WHERE ob_paylmod IS NOT "" */
  sqlite3_stmt *omodstmt = NULL;
  enum
  { MOMRESIX_OID, MOMRESIX_PAYLKINDID, MOMRESIX_PAYLMOD, MOMRESIX__LAST };
  if ((rc = sqlite3_prepare_v2 (ld->mo_ld_db,
                                "SELECT ob_id, ob_paylkid, ob_paylmod"
                                " FROM t_objects"
                                " WHERE ob_paylmod IS NOT \"\" "
                                " AND ob_paylkid IS NOT \"\" ",
                                -1, &omodstmt, NULL)) != SQLITE_OK)
    MOM_FATAPRINTF
      ("Sqlite loader base %s failed to prepare codpayl selection (%s)",
       mo_string_cstr (ld->mo_ld_sqlitepathv), sqlite3_errstr (rc));
  while ((rc = sqlite3_step (omodstmt)) == SQLITE_ROW)
    {
      MOM_ASSERTPRINTF (sqlite3_data_count (omodstmt) == MOMRESIX__LAST
                        && sqlite3_column_type (omodstmt, MOMRESIX_OID)
                        == SQLITE_TEXT
                        && sqlite3_column_type (omodstmt, MOMRESIX_PAYLKINDID)
                        == SQLITE_TEXT
                        && sqlite3_column_type (omodstmt, MOMRESIX_PAYLMOD)
                        == SQLITE_TEXT, "bad omodstmt");
      const char *oidstr =
        (const char *) sqlite3_column_text (omodstmt, MOMRESIX_OID);
      mo_hid_t obhid = 0;
      mo_loid_t obloid = 0;
      if (!mo_get_hi_lo_ids_from_cstring (&obhid, &obloid, oidstr)
          || obhid == 0 || obloid == 0)
        MOM_FATAPRINTF ("Sqlite loader base %s with bad ob_id  %s",
                        mo_string_cstr (ld->mo_ld_sqlitepathv), oidstr);
      mo_objref_t obr = mo_objref_find_hid_loid (obhid, obloid);
      if (obr == NULL)
        MOM_FATAPRINTF ("Sqlite loader base %s ob_id %s not found",
                        mo_string_cstr (ld->mo_ld_sqlitepathv), oidstr);
      const char *kindidstr =
        (const char *) sqlite3_column_text (omodstmt, MOMRESIX_PAYLKINDID);
      mo_hid_t kindhid = 0;
      mo_loid_t kindloid = 0;
      if (!mo_get_hi_lo_ids_from_cstring (&kindhid, &kindloid, kindidstr)
          || kindhid == 0 || kindloid == 0)
        MOM_FATAPRINTF ("Sqlite loader base %s with bad ob_paylkid  %s",
                        mo_string_cstr (ld->mo_ld_sqlitepathv), kindidstr);
      mo_objref_t kindobr = mo_objref_find_hid_loid (kindhid, kindloid);
      if (kindobr == NULL)
        MOM_FATAPRINTF ("Sqlite loader base %s ob_paylkid %s not found",
                        mo_string_cstr (ld->mo_ld_sqlitepathv), kindidstr);
      mo_objref_put_signature_payload (obr, kindobr);
    }
  if (rc != SQLITE_DONE)
    MOM_FATAPRINTF ("Sqlite loader base %s objectmod selection not done (%s)",
                    mo_string_cstr (ld->mo_ld_sqlitepathv),
                    sqlite3_errstr (rc));
  rc = sqlite3_finalize (omodstmt);
  omodstmt = NULL;
  if (rc != SQLITE_OK)
    MOM_FATAPRINTF
      ("Sqlite loader base %s objectmod selection unfinalized (%s)",
       mo_string_cstr (ld->mo_ld_sqlitepathv), sqlite3_errstr (rc));
}                               /* end of mo_loader_load_payload_code */


void
mo_loader_load_payload_data (mo_loader_ty * ld)
{
  int rc = 0;
  MOM_ASSERTPRINTF (ld && ld->mo_ld_magic == MOM_LOADER_MAGIC, "bad ld");
  /** repeat: 
      SELECT ob_id, ob_paylkid, ob_paylcont FROM t_objects
      WHERE ob_paylkid IS NOT "" AND ob_paylmod IS ""
  **/
  sqlite3_stmt *lpaystmt = NULL;
  enum
  { MOMRESIX_OID, MOMRESIX_PAYLKINDID, MOMRESIX_PAYLCONT, MOMRESIX__LAST };
  if ((rc = sqlite3_prepare_v2 (ld->mo_ld_db,
                                "SELECT ob_id, ob_paylkid, ob_paylcont"
                                " FROM t_objects WHERE "
                                " ob_paylkid IS NOT \"\" "
                                " AND ob_paylmod IS \"\" ",
                                -1, &lpaystmt, NULL)) != SQLITE_OK)
    MOM_FATAPRINTF
      ("Sqlite loader base %s failed to prepare objectmod selection (%s)",
       mo_string_cstr (ld->mo_ld_sqlitepathv), sqlite3_errstr (rc));
  while ((rc = sqlite3_step (lpaystmt)) == SQLITE_ROW)
    {
      MOM_ASSERTPRINTF (sqlite3_data_count (lpaystmt) == MOMRESIX__LAST
                        && sqlite3_column_type (lpaystmt, MOMRESIX_OID)
                        == SQLITE_TEXT
                        && sqlite3_column_type (lpaystmt, MOMRESIX_PAYLKINDID)
                        == SQLITE_TEXT
                        && sqlite3_column_type (lpaystmt, MOMRESIX_PAYLCONT)
                        == SQLITE_TEXT, "bad lpaystmt");
      const char *obidstr =
        (const char *) sqlite3_column_text (lpaystmt, MOMRESIX_OID);
      const char *pkidstr =
        (const char *) sqlite3_column_text (lpaystmt, MOMRESIX_PAYLKINDID);
      const char *paylcontstr =
        (const char *) sqlite3_column_text (lpaystmt, MOMRESIX_PAYLCONT);
      mo_hid_t obhid = 0;
      mo_loid_t obloid = 0;
      if (!mo_get_hi_lo_ids_from_cstring (&obhid, &obloid, obidstr)
          || obhid == 0 || obloid == 0)
        MOM_FATAPRINTF ("Sqlite loader base %s with bad ob_id  %s",
                        mo_string_cstr (ld->mo_ld_sqlitepathv), obidstr);
      mo_objref_t obr = mo_objref_find_hid_loid (obhid, obloid);
      if (obr == NULL)
        MOM_FATAPRINTF ("Sqlite loader base %s ob_id %s not found",
                        mo_string_cstr (ld->mo_ld_sqlitepathv), obidstr);
      mo_hid_t pkhid = 0;
      mo_loid_t pkloid = 0;
      if (!mo_get_hi_lo_ids_from_cstring (&pkhid, &pkloid, pkidstr)
          || pkhid == 0 || pkloid == 0)
        MOM_FATAPRINTF ("Sqlite loader base %s with bad ob_paylkid  %s",
                        mo_string_cstr (ld->mo_ld_sqlitepathv), pkidstr);
      mo_objref_t pkobr = mo_objref_find_hid_loid (pkhid, pkloid);
      if (pkobr == NULL)
        MOM_FATAPRINTF ("Sqlite loader base %s ob_paylkid %s not found",
                        mo_string_cstr (ld->mo_ld_sqlitepathv), pkidstr);
      json_t *js = NULL;
      json_error_t jerr;
      memset (&jerr, 0, sizeof (jerr));
      errno = 0;
#define MOM_NBCASE_PAYLOAD 307
#define CASE_PAYLOAD_MOM(Ob) momphash_##Ob % MOM_NBCASE_PAYLOAD:	\
      if (pkobr != MOM_PREDEF(Ob)) goto defaultpayloadcase;		\
      goto labpayl_##Ob; labpayl_##Ob
      switch (mo_objref_hash (pkobr) % MOM_NBCASE_PAYLOAD)
        {
#define LOADJS_MOM(Js,PaylStr,Jerr)  do {				\
	    if (PaylStr)						\
	      Js = json_loads(PaylStr, JSON_DISABLE_EOF_CHECK, &Jerr);	\
	    if (!Js)							\
	      MOM_WARNPRINTF("Sqlite loader base %s:"			\
			     " bad payload JSON content for %s (%s"	\
			     ", l#%d, c#%d, p#%d): %.64s...",		\
			     mo_string_cstr (ld->mo_ld_sqlitepathv),	\
			     mo_object_pnamestr(obr),Jerr.text,		\
			     Jerr.line, Jerr.column,			\
			     Jerr.position,				\
			     PaylStr);					\
	  } while (0)
        case CASE_PAYLOAD_MOM (payload_assoval):
          {
            LOADJS_MOM (js, paylcontstr, jerr);
            if (js)
              {
                obr->mo_ob_paylkind = MOM_PREDEF (payload_assoval);
                obr->mo_ob_payldata = mo_assoval_of_json (js);
              }
          }
          break;
        case CASE_PAYLOAD_MOM (payload_vectval):
          {
            LOADJS_MOM (js, paylcontstr, jerr);
            if (js)
              {
                obr->mo_ob_paylkind = MOM_PREDEF (payload_vectval);
                obr->mo_ob_payldata = mo_vectval_of_json (js);
              }
          }
          break;
        case CASE_PAYLOAD_MOM (payload_hashset):
          {
            LOADJS_MOM (js, paylcontstr, jerr);
            if (js)
              {
                obr->mo_ob_paylkind = MOM_PREDEF (payload_hashset);
                obr->mo_ob_payldata = mo_hashset_of_json (js);
              }
          }
          break;
        case CASE_PAYLOAD_MOM (payload_list):
          {
            LOADJS_MOM (js, paylcontstr, jerr);
            if (js)
              {
                obr->mo_ob_paylkind = MOM_PREDEF (payload_list);
                obr->mo_ob_payldata = mo_list_of_json (js);
              }
          }
          break;
        case CASE_PAYLOAD_MOM (payload_value):
          {
            LOADJS_MOM (js, paylcontstr, jerr);
            if (js)
              {
                obr->mo_ob_paylkind = MOM_PREDEF (payload_value);
                obr->mo_ob_payldata = (void *) mo_value_of_json (js);
              }
          }
          break;
        case CASE_PAYLOAD_MOM (payload_buffer):
          {
            extern void
              mo_objref_set_buffer_from_json (mo_objref_t obr, json_t *js);
            LOADJS_MOM (js, paylcontstr, jerr);
            if (js)
              mo_objref_set_buffer_from_json (obr, js);
          }
          break;
        case CASE_PAYLOAD_MOM (payload_json):
          {
            json_t *jcont = NULL;
            LOADJS_MOM (js, paylcontstr, jerr);
            if (js && json_is_object (js)
                && (jcont = json_object_get (js, "json")))
              {
                obr->mo_ob_paylkind = MOM_PREDEF (payload_json);
                obr->mo_ob_payldata = (void *) jcont;
              }
          }
        default:
        defaultpayloadcase:
          break;
        }
#undef MOM_NBCASE_PAYLOAD
#undef CASE_PAYLOAD_MOM
#undef LOADJS_MOM
    }
  if (rc != SQLITE_DONE)
    MOM_FATAPRINTF ("Sqlite loader base %s payldata selection not done (%s)",
                    mo_string_cstr (ld->mo_ld_sqlitepathv),
                    sqlite3_errstr (rc));
  rc = sqlite3_finalize (lpaystmt);
  lpaystmt = NULL;
  if (rc != SQLITE_OK)
    MOM_FATAPRINTF
      ("Sqlite loader base %s payldata  selection unfinalized (%s)",
       mo_string_cstr (ld->mo_ld_sqlitepathv), sqlite3_errstr (rc));
}                               /* end mo_loader_load_payload_data */

void
mo_loader_end_database (mo_loader_ty * ld)
{
  int rc = 0;
  MOM_ASSERTPRINTF (ld && ld->mo_ld_magic == MOM_LOADER_MAGIC, "bad ld");
  // finalize par_val FROM t_params...
  rc = sqlite3_finalize (ld->mo_ld_stmt_params);
  ld->mo_ld_stmt_params = NULL;
  if (rc != SQLITE_OK)
    MOM_FATAPRINTF ("Failed to finalise t_params Sqlite select par_val: %s",
                    sqlite3_errstr (rc));
  //
  rc = sqlite3_close (ld->mo_ld_db);
  if (rc != SQLITE_OK)
    MOM_FATAPRINTF ("failed to close loaded Sqlite3 database %s (%s)",
                    mo_string_cstr (ld->mo_ld_sqlitepathv),
                    sqlite3_errstr (rc));
}                               /* mo_loader_end_database */


void
mom_load_state (void)
{
  struct mo_loader_st loader;
  memset (&loader, 0, sizeof (loader));
  errno = 0;
  loader.mo_ld_magic = MOM_LOADER_MAGIC;
  loader.mo_ld_startelapsedtime = mom_elapsed_real_time ();
  loader.mo_ld_startcputime = mom_process_cpu_time ();
  loader.mo_ld_sqlpathv =
    mo_make_string_sprintf ("%s.sql", monimelt_perstatebase);
  loader.mo_ld_sqlitepathv =
    mo_make_string_sprintf ("%s.sqlite", monimelt_perstatebase);
  mo_loader_begin_database (&loader);
  mo_loader_create_objects (&loader);
  mo_loader_name_objects (&loader);
  mo_loader_link_modules (&loader);
  mo_loader_fill_objects_contents (&loader);
  /// we should first load the payload sitting in code, that is having
  /// a non-empty ob_paylmod:
  mo_loader_load_payload_code (&loader);
  /// the we should other payload
  mo_loader_load_payload_data (&loader);
  mo_loader_end_database (&loader);
  double endelapsedtime = mom_elapsed_real_time ();
  double endcputime = mom_process_cpu_time ();
  char cwdbuf[256];
  memset (cwdbuf, 0, sizeof (cwdbuf));
  if (!getcwd (cwdbuf, sizeof (cwdbuf)))
    strcpy (cwdbuf, "./");
  MOM_INFORMPRINTF ("loaded %u objects, %u named, %u modules\n"
                    "... (pid %d on %s in %s)\n"
                    "... in %.3f elapsed seconds (%.3f µs/obj), %.4f cpu seconds (%.3f µs/obj)\n",
                    loader.mo_ld_nbobjects, loader.mo_ld_nbnamed,
                    loader.mo_ld_nbmodules,
                    (int) getpid (), mom_hostname (), cwdbuf,
                    endelapsedtime - loader.mo_ld_startelapsedtime,
                    1.0e6 * (endelapsedtime -
                             loader.mo_ld_startelapsedtime) /
                    loader.mo_ld_nbobjects,
                    endcputime - loader.mo_ld_startcputime,
                    1.0e6 * (endcputime -
                             loader.mo_ld_startcputime) /
                    loader.mo_ld_nbobjects);
  memset (&loader, 0, sizeof (loader));
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
      {
        unsigned sz = mo_size_of_value (v);
        if (sz < 96)
          return json_stringn (mo_string_cstr (v), mo_size_of_value (v));
        else
          {
            json_t *jarr = json_array ();
            const char *cstr = mo_string_cstr (v);
            const char *ends = cstr + sz;
            const char *schk = cstr;
            const char *npc = NULL;
            for (const char *pc = cstr; pc < ends && *pc; pc = npc)
              {
                npc = g_utf8_next_char (pc);
                if (pc - schk >= 80 || pc + 1 >= ends
                    || (*pc == '\n' && pc - schk > 24))
                  {
                    if (*pc == '\n')
                      {
                        pc++;
                        npc = g_utf8_next_char (pc);
                      }
                    json_t *jchk = json_stringn (schk, pc - schk);
                    schk = pc;
                    json_array_append_new (jarr, jchk);
                  }
              }
            return json_pack ("{siso}", "ssize", sz, "string", jarr);
          }
      }
      break;
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
  char idbuf[MOM_CSTRIDSIZ];
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
      json_t *jl = NULL;
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
      else if ((jc = json_object_get (js, "string")) != NULL
               && json_is_array (jc)
               && (jl = json_object_get (js, "ssize")) != NULL
               && json_is_integer (jl))
        {
          unsigned sz = json_array_size (jc);
          long ln = json_integer_value (jl);
          if (ln < 0 || ln > MOM_SIZE_MAX)
            MOM_FATAPRINTF ("too long obj.string ssize=%ld", ln);
          char *buf = calloc (1, ln + 1);
          if (!buf)
            MOM_FATAPRINTF ("calloc failure of obj.string of ssize=%ld", ln);
          long pos = 0;
          for (unsigned ix = 0; ix < sz; ix++)
            {
              json_t *je = json_array_get (jc, ix);
              if (!json_is_string (je))
                continue;
              const char *strchk = json_string_value (je);
              size_t sizchk = json_string_length (je);
              size_t lenchk = 0;
              if (pos + (long) sizchk <= ln)
                lenchk = sizchk;
              else
                lenchk = ln - pos;
              memcpy (buf + pos, strchk, lenchk);
            }
          MOM_ASSERTPRINTF (pos <= ln, "bad final pos=%ld ln=%ld", pos,
                            (long) ln);
          mo_value_t vstr = mo_make_string_len (buf, ln);
          free (buf), buf = NULL;
          return vstr;
        }
    }
  MOM_WARNPRINTF ("wrong json %s", json_dumps (js, JSON_SORT_KEYS));
  return NULL;
}                               /* end of mo_value_of_json */


// end of file jstate.c
