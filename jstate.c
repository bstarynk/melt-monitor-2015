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
 * mo_create_tables_for_dump should be kept in sync with the
 * monimelt-dump-state.sh script (look for lines with ".mode insert"
 * inside that script)
 ***/

typedef struct mo_dumper_st mo_dumper_ty;
typedef struct mo_loader_st mo_loader_ty;
void mo_create_tables_for_dump (mo_dumper_ty *);


#define MOM_LOADER_MAGIC  0x179128bd
struct mo_loader_st             // stack allocated
{
  unsigned mo_ld_magic;         /* always MOM_LOADER_MAGIC */
  double mo_ld_startelapsedtime;
  double mo_ld_startcputime;
  unsigned mo_ld_nbitems;
};



enum mom_dumpstate_en
{ MOMDUMP_NONE, MOMDUMP_SCAN, MOMDUMP_EMIT };
#define MOM_DUMPER_MAGIC  0x372bb699
struct mo_dumper_st             // stack allocated
{
  unsigned mo_du_magic;         /* always MOM_DUMPER_MAGIC */
  enum mom_dumpstate_en mo_du_state;
  sqlite3 *mo_du_db;
  const char *mo_du_dirpath;
  const char *mo_du_tempsuffix;
  mo_hashsetpayl_ty *mo_du_objset;      /* the set of reachable objects */
  mo_listpayl_ty *mo_du_scanlist;       /* the todo list for scanning */
};


void
mo_create_tables_for_dump (mo_dumper_ty * du)
{
  MOM_ASSERTPRINTF (du && du->mo_du_magic == MOM_DUMPER_MAGIC,
                    "invalid dumper");
  MOM_ASSERTPRINTF (du->mo_du_db, "bad database");
  // keep these table names in CREATE TABLE Sqlite statements in sync
  // with monimelt-dump-state.sh script
  char *errmsg = NULL;
  if (sqlite3_exec (du->mo_du_db,
                    "CREATE TABLE t_params"
                    " (par_name VARCHAR(35) PRIMARY KEY ASC NOT NULL UNIQUE,"
                    "  par_value TEXT NOT NULL)", NULL, NULL, &errmsg))
    MOM_FATAPRINTF ("Failed to create t_params Sqlite table: %s", errmsg);
  if (sqlite3_exec (du->mo_du_db,
                    "CREATE TABLE t_objects"
                    " (ob_id VARCHAR(20) PRIMARY KEY ASC NOT NULL UNIQUE,"
                    "  ob_mtime DATETIME,"
                    "  ob_jsoncont TEXT NOT NULL)", NULL, NULL, &errmsg))
    MOM_FATAPRINTF ("Failed to create t_objects Sqlite table: %s", errmsg);
}

void
mom_load_state (void)
{
}                               /* end mom_load_state */


void
mom_dump_state (const char *dirname)
{
  if (!dirname || dirname == MOM_EMPTY_SLOT)
    dirname = ".";
  mo_dumper_ty dumper;
  mo_value_t predefset = mo_predefined_objects_set ();
  int nbpredef = mo_set_size (predefset);
  if (nbpredef == 0 || nbpredef + 10 < MOM_NB_PREDEFINED / 2)
    MOM_FATAPRINTF ("too few remaining predef %d (previously %d)",
                    nbpredef, MOM_NB_PREDEFINED);
  memset (dumper, 0, sizeof (dumper));
  dumper.mo_du_magic = MOM_DUMPER_MAGIC;
  dumper.mo_du_state = MOMDUMP_SCAN;
  dumper.mo_du_db = NULL;
  dumper.mo_du_dirpath = GC_STRDUP (dirname);
  dumper.mo_du_tempsuffix =
    mo_make_string_sprintf (".tmp_%lx_%lx_p%d%%",
                            momrand_genrand_int31 (),
                            momrand_genrand_int31 (), (int) getpid ());
  dumper.mo_du_objset =
    mo_hashset_reserve (NULL, 4 * mo_set_size (predefset) + 100);
  dumper.mo_du_scanlist = mo_list_make ();
  for (int ix = 0; ix < nbpredef; ix++)
    mo_dump_scan_objref (&dumper, mo_set_nth (predefset, ix));
#warning mom_dump_state very incomplete
  MOM_WARNPRINTF ("mom_dump_state very incomplete");
}                               /* end mom_dump_state */


void
mo_dump_scan_value (mo_dumper_ty * du, mo_value_t v)
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
mo_dump_scan_objref (mo_dumper_ty * du, mo_objref_t obr)
{
  MOM_ASSERTPRINTF (du && du->mo_du_magic == MOM_DUMPER_MAGIC
                    && du->mo_du_state == MOMDUMP_SCAN, "bad dumper du@%p",
                    du);
  if (mo_dyncast_objref (obr) == NULL)
    return;
  if (mo_hashset_contains (du->mo_du_objset, obr))
    return;
  du->mo_du_objset = mo_hashset_put (du->mo_du_objset, obr);
  du->mo_du_scanlist = mo_list_append (du->mo_du_scanlist, obr);
}                               /* end mo_dump_scan_objref */


mo_json_t
mo_json_of_value (mo_value_t v)
{
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
          json_array_append_new (jarr,
                                 (json_t *)
                                 mo_jsonid_of_objref (((mo_sequencevalue_ty *)
                                                       v)->mo_seqobj[ix]));
        return json_pack ("{so}", (k == mo_KTUPLE) ? "tup" : "set", jarr);
      }
    case mo_KOBJECT:
      return json_pack ("{so}", "oid", mo_jsonid_of_objref ((mo_objref_t) v));
    };
  MOM_FATAPRINTF ("impossible value@%p k#%u to json", v, k);
}                               /* end mo_json_of_value */

mo_json_t
mo_jsonid_of_objref (mo_objref_t obr)
{
  if (!mo_dyncast_objref ((mo_value_t) obr))
    return json_null ();
  char idbuf[MOM_CSTRIDLEN + 2];
  memset (idbuf, 0, sizeof (idbuf));
  return
    json_string (mo_cstring_from_hi_lo_ids
                 (idbuf, obr->mo_ob_hid, obr->mo_ob_loid));
}                               /* end of mo_jsonid_of_objref */

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
      const json_t *jc = NULL;
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
