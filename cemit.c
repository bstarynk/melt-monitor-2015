// file cemit.c - primitive C code emission

/**   Copyright (C)  2016  Basile Starynkevitch and later the FSF
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


/// mention momglob_int for emission of _mom_global.h
#define MOM_CEMIT_MAGIC 0x56c59cc3      /*1455791299 cemit_magic */
#define MOMFIELDS_cemitpayl					\
  MOMFIELDS_countedpayl;					\
  uint32_t mo_cemit_nmagic; /* always MOM_CEMIT_MAGIC */	\
  /* the suffix is often '.c', but without underscores */      	\
  char mo_cemit_suffix[4];					\
/* the prefix could be 'modules.dir/momg_' */			\
  char mo_cemit_prefix[40];					\
  /* a pointer to a local struct during emission */		\
  struct mom_cemitlocalstate_st* mo_cemit_locstate;      	\
/* the source module */						\
  mo_objref_t mo_cemit_modobj

struct mo_cemitpayl_st
{
  MOMFIELDS_cemitpayl;
};

// an internal stack-allocated structure
#define MOM_CEMITSTATE_MAGIC 0x2fd11731 /* 802232113 cemitstate_magic */
struct mom_cemitlocalstate_st
{
  uint32_t mo_cemsta_nmagic;    /* always MOM_CEMITSTATE_MAGIC */
  char mo_cemsta_tempsuffix[46];        /* a temporary suffix */
  uint8_t mo_cemsta_indentation;
  mo_objref_t mo_cemsta_objcemit;       /* the objref with the cemit payload  */
  char mo_cemsta_modid[MOM_CSTRIDSIZ];
  FILE *mo_cemsta_fil;          /* the emitted FILE handle */
  mo_cemitpayl_ty *mo_cemsta_payl;      /* the payload */
  mo_hashsetpayl_ty *mo_cemsta_hsetctypes;      /* the hashset of declared c_type-s - and signatures */
  mo_hashsetpayl_ty *mo_cemsta_hsetincludes;    /* the hashset of includes */
  mo_listpayl_ty *mo_cemsta_endtodolist;        /* list of things to do at end */
  mo_value_t mo_cemsta_errstr;  /* the error string */
  jmp_buf mo_cemsta_jmpbuf;     /* for errors */
};

/// maximal size of emitted C file
#define MOM_CEMIT_MAX_FSIZE (32<<20)    /* 32 megabytes */
/// maximal recursion depth
#define MOM_CEMIT_MAX_DEPTH 100
void
mo_objref_cleanup_cemit (mo_objref_t obr)
{                               // called only from mo_objref_really_clear_payload
  MOM_ASSERTPRINTF (mo_dyncast_objref (obr)
                    && obr->mo_ob_paylkind == MOM_PREDEF (payload_c_emit)
                    && obr->mo_ob_payldata,
                    "bad obr@%p to cleanup_cemit", obr);
  mo_cemitpayl_ty *cemp = obr->mo_ob_payldata;
  MOM_ASSERTPRINTF (cemp != NULL && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC,
                    "bad cemp@%p", cemp);
  struct mom_cemitlocalstate_st *csta = cemp->mo_cemit_locstate;
  if (csta)
    {
      MOM_ASSERTPRINTF (csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                        && csta->mo_cemsta_objcemit == obr,
                        "bad csta@%p for obr %s", csta,
                        mo_objref_pnamestr (obr));

    }
}                               /* end of mo_objref_cleanup_cemit */


void
mo_objref_put_cemit_payload (mo_objref_t obr, mo_objref_t obmodul)
{
  if (!mo_dyncast_objref (obr))
    return;
  if (!mo_dyncast_objref (obmodul))
    return;
  if (obmodul->mo_ob_class != MOM_PREDEF (module_class))
    {
      MOM_WARNPRINTF
        ("put_cemit_payload: wrong module object %s for c_emit %s",
         mo_objref_pnamestr (obmodul), mo_objref_pnamestr (obr));
      return;
    }
  mo_cemitpayl_ty *cemp = mom_gc_alloc (sizeof (mo_cemitpayl_ty));
  cemp->mo_va_kind = mo_PCEMIT;
  cemp->mo_va_index = 0;
  cemp->mo_va_hash = (momrand_genrand_int31 () & 0xfffffff) + 2;
  cemp->mo_sva_size = 0;
  cemp->mo_cpl_count = 0;
  cemp->mo_cemit_nmagic = MOM_CEMIT_MAGIC;
  strcpy (cemp->mo_cemit_suffix, ".c");
  cemp->mo_cemit_suffix[sizeof (cemp->mo_cemit_suffix) - 1] = 0;
  strncpy (cemp->mo_cemit_prefix, MOM_MODULES_DIR "/" MOM_MODULE_INFIX,
           sizeof (cemp->mo_cemit_prefix) - 1);
  cemp->mo_cemit_prefix[sizeof (cemp->mo_cemit_prefix) - 1] = 0;
  cemp->mo_cemit_locstate = NULL;
  cemp->mo_cemit_modobj = obmodul;
  mo_objref_clear_payload (obr);
  obr->mo_ob_paylkind = MOM_PREDEF (payload_c_emit);
  obr->mo_ob_payldata = cemp;
}                               /* end mo_objref_put_cemit_payload */


mo_cemitpayl_ty *
mo_dyncastpayl_cemit (const void *p)
{
  if (!mo_valid_pointer_value (p))
    return NULL;
  mo_cemitpayl_ty *cemp = (mo_cemitpayl_ty *) p;
  if (cemp->mo_va_kind != mo_PCEMIT)
    return NULL;
  if (cemp->mo_cemit_nmagic != MOM_CEMIT_MAGIC)
    MOM_FATAPRINTF ("corrupted cemit @%p", cemp);
  return cemp;
}                               /* end mo_dyncastpayl_cemit */

bool
mo_objref_has_valid_cemit_payload (mo_objref_t obr)
{
  if (!mo_dyncast_objref (obr))
    return false;
  if (obr->mo_ob_paylkind != MOM_PREDEF (payload_c_emit))
    return false;
  mo_cemitpayl_ty *cemp = mo_dyncastpayl_cemit (obr->mo_ob_payldata);
  if (!cemp)
    return false;
  return true;
}                               /* end of mo_objref_has_valid_cemit_payload */

mo_cemitpayl_ty *
mo_objref_get_cemit (mo_objref_t obr)
{
  if (!mo_dyncast_objref (obr))
    return NULL;
  if (obr->mo_ob_paylkind != MOM_PREDEF (payload_c_emit))
    return NULL;
  mo_cemitpayl_ty *cemp = mo_dyncastpayl_cemit (obr->mo_ob_payldata);
  return cemp;
}                               /* end mo_objref_get_cemit */

#define MOM_CEMITFAILURE_AT(Lin,Csta,Fmt,...) do {	\
    struct mom_cemitlocalstate_st*_csta_##Lin = (Csta);	\
    MOM_ASSERTPRINTF_AT					\
      (__FILE__,Lin,					\
       _csta_##Lin->mo_cemsta_nmagic			\
       == MOM_CEMITSTATE_MAGIC,				\
       "bad csta@%p for failure %s",			\
       _csta_##Lin, (Fmt));				\
    _csta_##Lin->mo_cemsta_errstr			\
      = mo_make_string_sprintf(Fmt, ##__VA_ARGS__);	\
    MOM_BACKTRACEPRINTF_AT				\
    (__FILE__,Lin,					\
     "%s*cemit failure:%s %s",				\
     MOM_TERMWARNCOLOR, MOM_TERMPLAIN,                  \
     mo_string_cstr(_csta_##Lin->mo_cemsta_errstr));	\
    longjmp(_csta_##Lin->mo_cemsta_jmpbuf, Lin);	\
} while(0)
#define MOM_CEMITFAILURE_AT_BIS(Lin,Csta,Fmt,...) \
  MOM_CEMITFAILURE_AT(Lin,Csta,Fmt,##__VA_ARGS__)
#define MOM_CEMITFAILURE(Csta,Fmt,...) \
  MOM_CEMITFAILURE_AT_BIS(__LINE__,Csta,Fmt,##__VA_ARGS__)

bool
mo_objref_active_cemit_payload (mo_objref_t obr)
{
  if (!mo_dyncast_objref (obr))
    return false;
  if (obr->mo_ob_paylkind != MOM_PREDEF (payload_c_emit))
    return false;
  mo_cemitpayl_ty *cemp = mo_dyncastpayl_cemit (obr->mo_ob_payldata);
  if (!cemp)
    return false;
  MOM_ASSERTPRINTF (cemp->mo_cemit_locstate == NULL
                    || cemp->mo_cemit_locstate->mo_cemsta_nmagic ==
                    MOM_CEMITSTATE_MAGIC, "bad locstate in cemp@%p", cemp);
  return cemp->mo_cemit_locstate != NULL;
}                               /* end of mo_objref_active_cemit_payload */


static inline struct mom_cemitlocalstate_st *
mom_cemit_locstate (mo_objref_t obr)
{
  if (!mo_dyncast_objref (obr))
    return NULL;
  if (obr->mo_ob_paylkind != MOM_PREDEF (payload_c_emit))
    return NULL;
  mo_cemitpayl_ty *cemp = mo_dyncastpayl_cemit (obr->mo_ob_payldata);
  if (!cemp)
    return NULL;
  return cemp->mo_cemit_locstate;
}                               /* end of mom_cemit_locstate */

const char *
mo_objref_cemit_detailstr (mo_objref_t obr)
{
  char *res = NULL;
  if (!mo_objref_has_valid_cemit_payload (obr))
    return NULL;
  mo_cemitpayl_ty *cemp = mo_dyncastpayl_cemit (obr->mo_ob_payldata);
  if (cemp->mo_cemit_locstate)
    {
      struct mom_cemitlocalstate_st *csta = cemp->mo_cemit_locstate;
      MOM_ASSERTPRINTF (csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC,
                        "bad csta@%p in cemp@%p", csta, cemp);
      char *buf = NULL;
      asprintf (&buf, "active cemit#%d: %s%s%s", fileno (csta->mo_cemsta_fil),
                cemp->mo_cemit_prefix,
                mo_objref_pnamestr (cemp->mo_cemit_modobj),
                cemp->mo_cemit_suffix);
      res = mom_gc_strdup (buf);
      free (buf);
    }
  else
    {
      char sbuf[48];
      char bufid[MOM_CSTRIDSIZ];
      memset (bufid, 0, sizeof (bufid));
      memset (sbuf, 0, sizeof (sbuf));
      snprintf (sbuf, sizeof (sbuf), "cemit/%s",
                mo_objref_idstr (bufid, cemp->mo_cemit_modobj));
      res = mom_gc_strdup (sbuf);
    }
  return res;
}                               /* end of mo_objref_cemit_detailstr */


void
mo_objref_cemit_set_suffix (mo_objref_t obrcem, const char *suffix)
{
  mo_cemitpayl_ty *cemp = mo_objref_get_cemit (obrcem);
  if (!cemp)
    return;
  if (!suffix || suffix == MOM_EMPTY_SLOT)
    return;
  if (strchr (suffix, '_'))
    {
      MOM_WARNPRINTF
        ("invalid suffix '%s' for obrcem=%s; underscores forbidden", suffix,
         mo_objref_pnamestr (obrcem));
      return;
    }
  if (cemp->mo_cemit_locstate)
    {
      MOM_WARNPRINTF
        ("for cemit object %s cannot set suffix %s since active %s",
         mo_objref_pnamestr (obrcem), suffix,
         mo_objref_cemit_detailstr (obrcem));
      return;
    }
  strncpy (cemp->mo_cemit_suffix, suffix, sizeof (cemp->mo_cemit_suffix) - 1);
}                               /* end of mo_objref_cemit_set_suffix */


void
mo_objref_cemit_set_prefix (mo_objref_t obrcem, const char *prefix)
{
  mo_cemitpayl_ty *cemp = mo_objref_get_cemit (obrcem);
  if (!cemp)
    return;
  if (!prefix || prefix == MOM_EMPTY_SLOT)
    return;
  bool goodprefix = false;
  for (const char *pc = prefix; *pc && goodprefix; pc++)
    {
      if (pc >= prefix + sizeof (cemp->mo_cemit_prefix) - 1)
        goodprefix = false;
      if (*pc == '.' && pc > prefix && pc[-1] == '.')
        goodprefix = false;
      if (*pc == '/' && pc > prefix && pc[-1] == '/')
        goodprefix = false;
      if (!isalnum (*pc) && *pc != '+' && *pc != '-' && *pc != '.'
          && *pc != '_' && *pc != '/')
        goodprefix = false;
    }
  if (!goodprefix)
    {
      MOM_WARNPRINTF ("for cemit object %s bad prefix '%s'",
                      mo_objref_pnamestr (obrcem), prefix);
      return;
    }
  if (cemp->mo_cemit_locstate)
    {
      MOM_WARNPRINTF
        ("for cemit object %s cannot set suffix %s since active %s",
         mo_objref_pnamestr (obrcem), prefix,
         mo_objref_cemit_detailstr (obrcem));
      return;
    }
  strncpy (cemp->mo_cemit_prefix, prefix, sizeof (cemp->mo_cemit_prefix) - 1);
}                               /* end mo_objref_cemit_set_prefix */



bool
mom_cemit_ctype_is_scalar (struct mom_cemitlocalstate_st *csta,
                           mo_objref_t ctypob)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil == NULL,
                    "cemit_ctype_is_scalar: bad csta@%p", csta);
  mo_cemitpayl_ty *cemp = csta->mo_cemsta_payl;
  MOM_ASSERTPRINTF (cemp && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC
                    && cemp->mo_cemit_locstate == csta,
                    "cemit_ctype_is_scalar: bad payl@%p in csta@%p", cemp,
                    csta);
  MOM_ASSERTPRINTF (mo_dyncast_objref (ctypob),
                    "cemit_ctype_is_scalar: no ctypob");
#define MOM_NBCASE_CTYPE 163
#define CASE_PREDEFCTYPE_MOM(Ob) momphash_##Ob % MOM_NBCASE_CTYPE:	\
  if (ctypob != MOM_PREDEF(Ob))						\
    goto defaultctypecase;						\
  goto labctype_##Ob;							\
  labctype_##Ob
#define CASE_GLOBALCTYPE_MOM(Ob) momghash_##Ob % MOM_NBCASE_CTYPE:	\
  if (ctypob != momglob_##Ob)						\
    goto defaultctypecase;						\
  goto labctype_##Ob;							\
  labctype_##Ob
  switch (mo_objref_hash (ctypob) % MOM_NBCASE_CTYPE)
    {
      case CASE_GLOBALCTYPE_MOM (int):  // momglob_int
        return true;
    case CASE_GLOBALCTYPE_MOM (bool):  // momglob_bool
      return true;
      case CASE_GLOBALCTYPE_MOM (char): // momglob_char
        return true;
      case CASE_GLOBALCTYPE_MOM (long): // momglob_long
        return true;
      case CASE_GLOBALCTYPE_MOM (double):       // momglob_double
        return true;
      case CASE_GLOBALCTYPE_MOM (intptr_t):     // momglob_intptr_t
        return true;
      case CASE_GLOBALCTYPE_MOM (int8_t):       // momglob_int8_t
        return true;
      case CASE_GLOBALCTYPE_MOM (int16_t):      // momglob_int16_t
        return true;
      case CASE_GLOBALCTYPE_MOM (int32_t):      // momglob_int32_t
        return true;
      case CASE_GLOBALCTYPE_MOM (uintptr_t):    // momglob_uintptr_t
        return true;
      case CASE_GLOBALCTYPE_MOM (uint8_t):      // momglob_uint8_t
        return true;
      case CASE_GLOBALCTYPE_MOM (uint16_t):     // momglob_uint16_t
        return true;
      case CASE_GLOBALCTYPE_MOM (uint32_t):     // momglob_uint32_t
        return true;
      case CASE_GLOBALCTYPE_MOM (uint64_t):     // momglob_uint64_t
        return true;
    defaultctypecase:
    default:
      break;
    }
#undef CASE_PREDEFCTYPE_MOM
#undef CASE_GLOBALCTYPE_MOM
#undef MOM_NBCASE_CTYPE
  if (ctypob->mo_ob_class == momglob_pointer_ctype_class)
    return true;
  if (ctypob->mo_ob_class == momglob_array_ctype_class)
    return false;
  if (ctypob->mo_ob_class == MOM_PREDEF (union_ctype_class))
    return false;
  if (ctypob->mo_ob_class == MOM_PREDEF (struct_ctype_class))
    return false;
  if (ctypob->mo_ob_class == MOM_PREDEF (enum_ctype_class))
    return true;
  if (ctypob->mo_ob_class == MOM_PREDEF (struct_pointer_ctype_class))
    return true;
  if (mo_objref_get_attr (ctypob, momglob_c_aggregate_initialization))
    return false;
  return true;
}                               /* end mom_cemit_ctype_is_scalar */


void mom_cemit_todo_put_attr (struct mom_cemitlocalstate_st *csta,
                              mo_objref_t obj, mo_objref_t attr,
                              mo_value_t val);


void mom_cemit_printf (struct mom_cemitlocalstate_st *csta, const char *fmt,
                       ...) __attribute__ ((format (printf, 2, 3)));
void
mom_cemit_printf (struct mom_cemitlocalstate_st *csta, const char *fmt, ...)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil != NULL,
                    "cemit_printf: bad csta@%p for fmt:%s", csta, fmt);
  FILE *fil = csta->mo_cemsta_fil;
  va_list args;
  va_start (args, fmt);
  vfprintf (fil, fmt, args);
  va_end (args);
  if (ftell (fil) > MOM_CEMIT_MAX_FSIZE)
    MOM_CEMITFAILURE (csta,
                      "cemit_printf: too big emission (%ld megabytes, %ld bytes)",
                      ftell (fil) >> 20, ftell (fil));
#ifndef NDEBUG
  if (ftell (fil) > MOM_CEMIT_MAX_FSIZE / 3)
    {
      static bool warned;
      if (!warned)
        {
          MOM_BACKTRACEPRINTF
            ("cemit_printf: very big emission (%ld megabytes, %ld bytes)",
             ftell (fil) >> 20, ftell (fil));
          warned = true;
        }
    }
#endif /*NDEBUG*/
}                               /* end mom_cemit_printf */

void
mom_cemit_vprintf (struct mom_cemitlocalstate_st *csta, const char *fmt,
                   va_list args)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil != NULL,
                    "cemit_vprintf: bad csta@%p for fmt:%s", csta, fmt);
  FILE *fil = csta->mo_cemsta_fil;
  if (ftell (fil) > MOM_CEMIT_MAX_FSIZE)
    MOM_CEMITFAILURE (csta,
                      "cemit_vprintf: too big emission (%ld megabytes, %ld bytes)",
                      ftell (fil) >> 20, ftell (fil));
#ifndef NDEBUG
  if (ftell (fil) > MOM_CEMIT_MAX_FSIZE / 3)
    {
      static bool warned;
      if (!warned)
        {
          MOM_BACKTRACEPRINTF
            ("cemit_vprintf: very big emission (%ld megabytes, %ld bytes)",
             ftell (fil) >> 20, ftell (fil));
          warned = true;
        }
    }
#endif /*NDEBUG*/
    vfprintf (fil, fmt, args);
}                               /* end mom_cemit_printf */

void
mom_objref_cemit_printf (mo_objref_t obrcem, const char *fmt, ...)
{
  mo_cemitpayl_ty *cemp = mo_objref_get_cemit (obrcem);
  if (!cemp || !cemp->mo_cemit_locstate)
    return;
  va_list args;
  va_start (args, fmt);
  mom_cemit_vprintf (cemp->mo_cemit_locstate, fmt, args);
  va_end (args);
}                               /* end of mom_objref_cemit_printf */


void
mom_cemit_newline (struct mom_cemitlocalstate_st *csta)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil != NULL,
                    "cemit_newline: bad csta@%p", csta);
  fputc ('\n', csta->mo_cemsta_fil);
  if (ftell (csta->mo_cemsta_fil) > MOM_CEMIT_MAX_FSIZE)
    MOM_CEMITFAILURE (csta,
                      "cemit_newline: too big emission (%ld megabytes, %ld bytes)",
                      ftell (csta->mo_cemsta_fil) >> 20,
                      ftell (csta->mo_cemsta_fil));
#ifndef NDEBUG
  if (ftell (csta->mo_cemsta_fil) > MOM_CEMIT_MAX_FSIZE / 3)
    {
      static bool warned;
      if (!warned)
        {
          MOM_BACKTRACEPRINTF
            ("cemit_newline: very big emission (%ld megabytes, %ld bytes)",
             ftell (csta->mo_cemsta_fil) >> 20, ftell (csta->mo_cemsta_fil));
          warned = true;
        }
    }
#endif /*NDEBUG*/
    for (int ix = (int)(csta->mo_cemsta_indentation % 16); ix > 0; ix--)
    fputc (' ', csta->mo_cemsta_fil);
}                               /* end of mom_cemit_newline */

void
mom_objref_cemit_newline (mo_objref_t obrcem)
{
  mo_cemitpayl_ty *cemp = mo_objref_get_cemit (obrcem);
  if (!cemp || !cemp->mo_cemit_locstate)
    return;
  mom_cemit_newline (cemp->mo_cemit_locstate);
}                               /* end of mom_objref_cemit_newline */

void
mom_cemit_open (struct mom_cemitlocalstate_st *csta)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil == NULL, "cemit_open: bad csta@%p",
                    csta);
  mo_cemitpayl_ty *cemp = csta->mo_cemsta_payl;
  MOM_ASSERTPRINTF (cemp && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC
                    && cemp->mo_cemit_locstate == csta,
                    "cemit_open: bad payl@%p in csta@%p", cemp, csta);
  snprintf (csta->mo_cemsta_tempsuffix, sizeof (csta->mo_cemsta_tempsuffix),
            "+r%x_%x_p%d.tmp~",
            (int) (momrand_genrand_int31 () & 0x3ffffff) + 1,
            (int) (momrand_genrand_int31 () & 0x3ffffff) + 1,
            (int) getpid ());
  char *pathbuf = NULL;
  asprintf (&pathbuf, "%s%s%s%s",
            cemp->mo_cemit_prefix,
            csta->mo_cemsta_modid, cemp->mo_cemit_suffix,
            csta->mo_cemsta_tempsuffix);
  if (!pathbuf)
    MOM_FATAPRINTF ("cemit_open: asprintf failed for module %s",
                    csta->mo_cemsta_modid);
  /* GNU extensions: e==Open the file with the O_CLOEXEC flag; 
     x==Open the file exclusively (like the O_EXCL flag of open(2)) */
  csta->mo_cemsta_fil = fopen (pathbuf, "wex");
  if (!csta->mo_cemsta_fil)
    MOM_FATAPRINTF ("cemit_open: fopen %s failed", pathbuf);
  free (pathbuf), pathbuf = NULL;
  char smallpath[128];
  memset (smallpath, 0, sizeof (smallpath));
  snprintf (smallpath, sizeof (smallpath), "%.30s%s%s",
            cemp->mo_cemit_prefix, csta->mo_cemsta_modid,
            cemp->mo_cemit_suffix);
  fprintf (csta->mo_cemsta_fil,
           "//// emitted C code file %s%s%s - DONT EDIT\n",
           cemp->mo_cemit_prefix, csta->mo_cemsta_modid,
           cemp->mo_cemit_suffix);
  mo_value_t modnamv = mo_objref_namev (cemp->mo_cemit_modobj);
  if (modnamv)
    fprintf (csta->mo_cemsta_fil, "/// for module %s (%s)\n",
             mo_string_cstr (modnamv), csta->mo_cemsta_modid);
  else
    fprintf (csta->mo_cemsta_fil, "/// for module %s\n",
             csta->mo_cemsta_modid);
  mom_output_gplv3_notice (csta->mo_cemsta_fil, "//! ", "", smallpath);
  if (!strcmp (cemp->mo_cemit_suffix, ".h"))
    fprintf (csta->mo_cemsta_fil, "#ifndef MOMHEADER%s\n"
             "#define MOMHEADER%s\n", csta->mo_cemsta_modid,
             csta->mo_cemsta_modid);
  fprintf (csta->mo_cemsta_fil,
           "\n#ifndef _GNU_SOURCE\n"
           "#define _GNU_SOURCE 1\n"
           "#endif /*_GNU_SOURCE for %s*/\n", csta->mo_cemsta_modid);
}                               /* end mom_cemit_open */


void
mom_cemit_close (struct mom_cemitlocalstate_st *csta)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil != NULL,
                    "cemit_close: bad csta@%p", csta);
  mo_cemitpayl_ty *cemp = csta->mo_cemsta_payl;
  MOM_ASSERTPRINTF (cemp && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC
                    && cemp->mo_cemit_locstate == csta,
                    "cemit_close: bad payl@%p in csta@%p", cemp, csta);
  fputs ("\n\n", csta->mo_cemsta_fil);
  if (!strcmp (cemp->mo_cemit_suffix, ".h"))
    fprintf (csta->mo_cemsta_fil, "#endif /*MOMHEADER%s*/\n",
             csta->mo_cemsta_modid);
  fprintf (csta->mo_cemsta_fil,
           "\n/*** end of emitted C file %s%s%s ***/\n",
           cemp->mo_cemit_prefix, csta->mo_cemsta_modid,
           cemp->mo_cemit_suffix);
  if (fclose (csta->mo_cemsta_fil))
    MOM_FATAPRINTF ("cemit_close: fclose failure for %s%s%s%s",
                    cemp->mo_cemit_prefix, csta->mo_cemsta_modid,
                    cemp->mo_cemit_suffix, csta->mo_cemsta_tempsuffix);
  csta->mo_cemsta_fil = NULL;
  char *newpathbuf = NULL;
  asprintf (&newpathbuf, "%s%s%s%s",
            cemp->mo_cemit_prefix,
            csta->mo_cemsta_modid, cemp->mo_cemit_suffix,
            csta->mo_cemsta_tempsuffix);
  if (MOM_UNLIKELY (!newpathbuf))
    MOM_FATAPRINTF ("cemit_close: asprintf newpathbuf failed for module %s",
                    csta->mo_cemsta_modid);
  char *oldpathbuf = NULL;
  asprintf (&oldpathbuf, "%s%s%s",
            cemp->mo_cemit_prefix,
            csta->mo_cemsta_modid, cemp->mo_cemit_suffix);
  if (!oldpathbuf)
    MOM_FATAPRINTF ("cemit_close: asprintf oldpathbuf failed for module %s",
                    csta->mo_cemsta_modid);
  MOM_INFORMPRINTF ("cemit_close: oldpathbuf='%s' newpathbuf='%s'",
                    oldpathbuf, newpathbuf);
  char *backuppathbuf = NULL;
  asprintf (&backuppathbuf, "%s%s%s%%",
            cemp->mo_cemit_prefix,
            csta->mo_cemsta_modid, cemp->mo_cemit_suffix);
  if (MOM_UNLIKELY (!backuppathbuf))
    MOM_FATAPRINTF
      ("cemit_close: asprintf backuppathbuf failed for module %s",
       csta->mo_cemsta_modid);
  FILE *newfil = fopen (newpathbuf, "r");
  if (MOM_UNLIKELY (!newfil))
    MOM_FATAPRINTF ("cemit_close: fopen %s read failed", newpathbuf);
  FILE *oldfil = fopen (oldpathbuf, "r");
  if (MOM_UNLIKELY (!oldfil))
    {
      if (errno != ENOENT)
        MOM_WARNPRINTF ("fopen %s strange failure", oldpathbuf);
      if (rename (newpathbuf, oldpathbuf))
        MOM_FATAPRINTF ("cemit_close: rename %s -> %s failure", newpathbuf,
                        oldpathbuf);
      fclose (newfil);
    }
  else
    {                           // test content of new & old file
      errno = 0;
      struct stat newstat = { };
      struct stat oldstat = { };
      if (fstat (fileno (oldfil), &oldstat))
        MOM_FATAPRINTF ("cemit_close: fstat old #%d %s failed",
                        fileno (oldfil), oldpathbuf);
      if (fstat (fileno (newfil), &newstat))
        MOM_FATAPRINTF ("cemit_close: fstat new #%d %s failed",
                        fileno (newfil), newpathbuf);
      bool samefilecontent = newstat.st_size == oldstat.st_size;
      while (samefilecontent)
        {
          int oldc = fgetc (oldfil);
          int newc = fgetc (newfil);
          if (oldc != newc)
            samefilecontent = false;
          else if (oldc == EOF)
            break;
        };
      fclose (oldfil), oldfil = NULL;
      fclose (newfil), newfil = NULL;
      if (samefilecontent)
        unlink (newpathbuf);
      else
        {
          if (MOM_UNLIKELY (rename (oldpathbuf, backuppathbuf)))
            MOM_FATAPRINTF ("cemit_close: backup rename %s -> %s failed",
                            oldpathbuf, backuppathbuf);
          if (MOM_UNLIKELY (rename (newpathbuf, oldpathbuf)))
            MOM_FATAPRINTF ("cemit_close: new rename %s -> %s failed",
                            newpathbuf, oldpathbuf);
        };
    }
  /// add a symbolic link for named modules
  mo_value_t namodv = mo_objref_namev (cemp->mo_cemit_modobj);
  MOM_INFORMPRINTF ("cemit_close: modobj %s namodv %s",
                    mo_objref_pnamestr (cemp->mo_cemit_modobj),
                    mo_value_pnamestr (namodv));
  if (namodv != NULL)
    {
      MOM_ASSERTPRINTF (mo_dyncast_string (namodv), "bad namodv for %s",
                        mo_objref_pnamestr (cemp->mo_cemit_modobj));
      char *oldbase = basename (oldpathbuf);
      char *oldunder = strrchr (oldbase, '_');
      MOM_INFORMPRINTF
        ("cemit_close: oldbase='%s' oldunder='%s' oldunder+CSTRIDLEN='%s'\n"
         "... oldpathbuf='%s'",
         oldbase, oldunder, oldunder + MOM_CSTRIDLEN, oldpathbuf);
      MOM_ASSERTPRINTF (oldbase != NULL
                        && oldbase >= oldpathbuf, "bad oldbase");
      char *oldbidp = strstr (oldbase, csta->mo_cemsta_modid);
      MOM_INFORMPRINTF ("cemit_close: oldbidp='%s'", oldbidp);
      MOM_ASSERTPRINTF (oldbidp != NULL
                        && strlen (oldbidp) >= MOM_CSTRIDLEN, "bad oldbidp");
      MOM_INFORMPRINTF
        ("cemit_close: begpat '%.*s'", (int) (oldbidp - oldpathbuf + 1),
         oldpathbuf);
      char *symlpathbuf = NULL;
      asprintf (&symlpathbuf, "%.*s%s%s", (int) (oldbidp - oldpathbuf + 1),
                oldpathbuf, mo_string_cstr (namodv), oldbidp + MOM_CSTRIDLEN);
      if (MOM_UNLIKELY (symlpathbuf == NULL))
        MOM_FATAPRINTF ("cemit_close: asprintf symlpathbuf failed for %s",
                        mo_objref_pnamestr (cemp->mo_cemit_modobj));
      errno = 0;
      MOM_INFORMPRINTF ("cemit_close symlpathbuf=%s oldbase=%s", symlpathbuf,
                        oldbase);
      if (access (symlpathbuf, R_OK) && errno == ENOENT)
        {
          if (symlink (oldbase, symlpathbuf))
            MOM_FATAPRINTF
              ("failed to symlink: %s -> %s for named module %s (%s)",
               symlpathbuf, oldbase, mo_string_cstr (namodv),
               csta->mo_cemsta_modid);
        }
      else
        MOM_WARNPRINTF
          ("cemit_close did not symlink oldbase=%s symlpathbuf=%s", oldbase,
           symlpathbuf);
    }
}                               /* end of mom_cemit_close */


void
mom_cemit_includes (struct mom_cemitlocalstate_st *csta)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil != NULL,
                    "cemit_includes: bad csta@%p", csta);
  mo_cemitpayl_ty *cemp = csta->mo_cemsta_payl;
  MOM_ASSERTPRINTF (cemp && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC
                    && cemp->mo_cemit_locstate == csta,
                    "cemit_includes: bad payl@%p in csta@%p", cemp, csta);
  mo_value_t cinclv =
    mo_objref_get_attr (cemp->mo_cemit_modobj, MOM_PREDEF (c_include));
  if (!cinclv)
    {
      mom_cemit_printf (csta, "\n// no includes\n");
      return;
    }
  if (!mo_dyncast_tuple (cinclv))
    MOM_CEMITFAILURE (csta, "bad c_include %s", mo_value_pnamestr (cinclv));
  unsigned nbincl = mo_tuple_size (cinclv);
  csta->mo_cemsta_hsetincludes =
    mo_hashset_reserve (NULL, 3 * nbincl / 2 + 10);
  mom_cemit_printf (csta, "\n// %u included headers:\n", nbincl);
  for (unsigned ix = 0; ix < nbincl; ix++)
    {
      mo_objref_t curinclob = mo_tuple_nth (cinclv, ix);
      if (!mo_dyncast_objref (curinclob))
        MOM_CEMITFAILURE (csta, "bad %d-th c_include %s", ix + 1,
                          mo_value_pnamestr (cinclv));
      mo_value_t inclstrv =
        mo_dyncast_string (mo_objref_get_attr
                           (curinclob, MOM_PREDEF (file_path)));
      const char *inclcstr = mo_string_cstr (inclstrv);
      if (!inclstrv || !inclcstr)
        MOM_CEMITFAILURE (csta, "included %s with bad `file_path`",
                          mo_objref_pnamestr (curinclob));
      for (const char *pi = inclcstr; *pi; pi++)
        if (!(isalnum (*pi) || *pi == '/' || *pi == '_' || *pi == '+'
              || *pi == '-' || *pi == '.'))
          MOM_CEMITFAILURE (csta, "included %s with invalid `file_path` %s",
                            mo_objref_pnamestr (curinclob), inclcstr);
      mom_cemit_printf (csta, "#include \"%s\"\n", inclcstr);
      csta->mo_cemsta_hsetincludes =
        mo_hashset_put (csta->mo_cemsta_hsetincludes, curinclob);
    }
  fputs ("\n\n", csta->mo_cemsta_fil);
  fflush (csta->mo_cemsta_fil);
}                               /* end mom_cemit_includes */

void
mom_cemit_write_ctype_for (struct mom_cemitlocalstate_st *csta,
                           mo_objref_t typobr, const char *forstr, int depth)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil != NULL,
                    "cemit_write_ctype_for: bad csta@%p", csta);
  if (!forstr || forstr == MOM_EMPTY_SLOT)
    forstr = "";
  mo_cemitpayl_ty *cemp = csta->mo_cemsta_payl;
  MOM_ASSERTPRINTF (cemp && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC
                    && cemp->mo_cemit_locstate == csta,
                    "cemit_write_ctype_for: bad payl@%p in csta@%p", cemp,
                    csta);
  MOM_ASSERTPRINTF (mo_dyncast_objref (typobr),
                    "cemit_write_ctype_for: bad typobr");
  if (depth > MOM_CEMIT_MAX_DEPTH)
    MOM_CEMITFAILURE (csta, "cemit_write_ctype_for: %s too deep %d, for %s",
                      mo_objref_pnamestr (typobr), depth, forstr);
  mo_objref_t inclobr =
    mo_dyncast_objref (mo_objref_get_attr (typobr, MOM_PREDEF (c_include)));
  if (inclobr && !mo_hashset_contains (csta->mo_cemsta_hsetincludes, inclobr))
    MOM_CEMITFAILURE (csta,
                      "cemit_write_ctype_for: %s has c_include %s which is not included",
                      mo_objref_pnamestr (typobr),
                      mo_objref_pnamestr (inclobr));

#define MOM_NBCASE_CTYPE 163
#define CASE_PREDEFCTYPE_MOM(Ob) momphash_##Ob % MOM_NBCASE_CTYPE:	\
  if (typobr != MOM_PREDEF(Ob))					\
    goto defaultctypecase;					\
  goto labctype_##Ob;						\
  labctype_##Ob
#define CASE_GLOBALCTYPE_MOM(Ob) momghash_##Ob % MOM_NBCASE_CTYPE: \
  if (typobr != momglob_##Ob)					\
    goto defaultctypecase;					\
  goto labctype_##Ob;						\
  labctype_##Ob
  switch (mo_objref_hash (typobr) % MOM_NBCASE_CTYPE)
    {
      /// for global ctypes, we need to mention the global in full
      /// with its momglob_ prefix to keep the global scanner,
      /// e.g. mo_dump_csource_global_objects_set, happy
      case CASE_GLOBALCTYPE_MOM (int):  // momglob_int
        mom_cemit_printf (csta, "int %s", forstr);
      return;
      case CASE_GLOBALCTYPE_MOM (void): // momglob_void
        mom_cemit_printf (csta, "void %s", forstr);
      return;
    case CASE_GLOBALCTYPE_MOM (bool):  // momglob_bool
      mom_cemit_printf (csta, "bool %s", forstr);
      return;
      case CASE_GLOBALCTYPE_MOM (char): // momglob_char
        mom_cemit_printf (csta, "char %s", forstr);
      return;
      case CASE_GLOBALCTYPE_MOM (long): // momglob_long
        mom_cemit_printf (csta, "long %s", forstr);
      return;
      case CASE_GLOBALCTYPE_MOM (double):       // momglob_double
        mom_cemit_printf (csta, "double %s", forstr);
      return;
      case CASE_GLOBALCTYPE_MOM (intptr_t):     // momglob_intptr_t
        mom_cemit_printf (csta, "intptr_t %s", forstr);
      return;
      case CASE_GLOBALCTYPE_MOM (int8_t):       // momglob_int8_t
        mom_cemit_printf (csta, "int8_t %s", forstr);
      return;
      case CASE_GLOBALCTYPE_MOM (int16_t):      // momglob_int16_t
        mom_cemit_printf (csta, "int16_t %s", forstr);
      return;
      case CASE_GLOBALCTYPE_MOM (int32_t):      // momglob_int32_t
        mom_cemit_printf (csta, "int32_t %s", forstr);
      return;
      case CASE_GLOBALCTYPE_MOM (int64_t):      // momglob_int64_t
        mom_cemit_printf (csta, "int64_t %s", forstr);
      return;
      case CASE_GLOBALCTYPE_MOM (uintptr_t):    // momglob_uintptr_t
        mom_cemit_printf (csta, "uintptr_t %s", forstr);
      return;
      case CASE_GLOBALCTYPE_MOM (uint8_t):      // momglob_uint8_t
        mom_cemit_printf (csta, "uint8_t %s", forstr);
      return;
      case CASE_GLOBALCTYPE_MOM (uint16_t):     // momglob_uint16_t
        mom_cemit_printf (csta, "uint16_t %s", forstr);
      return;
      case CASE_GLOBALCTYPE_MOM (uint32_t):     // momglob_uint32_t
        mom_cemit_printf (csta, "uint32_t %s", forstr);
      return;
      case CASE_GLOBALCTYPE_MOM (uint64_t):     // momglob_uint64_t
        mom_cemit_printf (csta, "uint64_t %s", forstr);
      return;
    default:
    defaultctypecase:
      break;
    }
#undef MOM_NBCASE_CTYPE
#undef CASE_PREDEFCTYPE_MOM
#undef CASE_GLOBALCTYPE_MOM
  if (mo_hashset_contains (csta->mo_cemsta_hsetctypes, typobr))
    {
      char typobid[MOM_CSTRIDSIZ];
      memset (typobid, 0, sizeof (typobid));
      mo_objref_idstr (typobid, typobr);
      mo_value_t typobnamv = mo_objref_namev (typobr);
      if (typobnamv)
        mom_cemit_printf (csta, "mo_%s_ty %s", mo_string_cstr (typobnamv),
                          forstr);
      else
        mom_cemit_printf (csta, "mo%s_ty %s", typobid, forstr);
      return;
    }
  if (typobr->mo_ob_class == momglob_pointer_ctype_class)
    {
      mo_objref_t pointedtypobr =
        mo_dyncast_objref (mo_objref_get_comp (typobr, 0));
      if (!pointedtypobr)
        MOM_CEMITFAILURE (csta,
                          "write_ctype_for: pointer typobr %s without pointed-type (comp#0)",
                          mo_objref_pnamestr (typobr));
      const char *pointedforstr = mom_gc_printf (" *%s", forstr);
      mom_cemit_write_ctype_for (csta, pointedtypobr, pointedforstr,
                                 depth + 1);
      return;
    }
  else if (typobr->mo_ob_class == momglob_array_ctype_class)
    {
      mo_objref_t elementypobr =
        mo_dyncast_objref (mo_objref_get_comp (typobr, 0));
      mo_value_t dimv = mo_objref_get_comp (typobr, 1);
      const char *arrayforstr = NULL;
      if (!dimv)
        arrayforstr =
          mom_gc_printf (" %s[/*MOMFLEXIBLE %s*/]", forstr,
                         mo_objref_pnamestr (typobr));
      else if (mo_value_is_int (dimv))
        arrayforstr =
          mom_gc_printf (" %s[%ld]", forstr,
                         (long) mo_value_to_int (dimv, -1));
      else
        MOM_CEMITFAILURE (csta,
                          "write_ctype_for: array typobr %s with bad dimension %s",
                          mo_objref_pnamestr (typobr),
                          mo_value_pnamestr (dimv));
      mom_cemit_write_ctype_for (csta, elementypobr, arrayforstr, depth + 1);
      return;
    }
  MOM_CEMITFAILURE (csta, "write_ctype_for: typobr %s (of class %s) unknown",
                    mo_objref_pnamestr (typobr),
                    mo_objref_pnamestr (typobr->mo_ob_class));
}                               /* end mom_cemit_write_ctype_for */



void
mom_cemit_todo_last_at_end (struct mom_cemitlocalstate_st *csta,
                            mo_objref_t todobr)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil != NULL,
                    "cemit_todo_last_at_end: bad csta@%p", csta);
  mo_cemitpayl_ty *cemp = csta->mo_cemsta_payl;
  MOM_ASSERTPRINTF (cemp && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC
                    && cemp->mo_cemit_locstate == csta,
                    "cemit_todo_last_at_end: bad payl@%p in csta@%p", cemp,
                    csta);
  MOM_ASSERTPRINTF (mo_dyncast_objref (todobr),
                    "cemit_todo_last_at_end: bad todobr");
  mo_objref_t curoutobj = mo_dyncast_objref (mo_objref_get_comp (todobr, 0));
  if (!curoutobj
      || curoutobj->mo_ob_paylkind !=
      MOM_PREDEF (signature_two_objects_to_void)
      || !curoutobj->mo_ob_payldata)
    MOM_CEMITFAILURE (csta,
                      "cemit_todo_last_at_end: todobr %s has wrong curoutobj %s",
                      mo_objref_pnamestr (todobr),
                      mo_objref_pnamestr (curoutobj));
  MOM_ASSERTPRINTF (mo_dyncastpayl_list (csta->mo_cemsta_endtodolist),
                    "bad endtotolist");
  mo_list_append (csta->mo_cemsta_endtodolist, todobr);
}                               /* end of mom_cemit_todo_last_at_end */


void
mom_cemit_todo_first_at_end (struct mom_cemitlocalstate_st *csta,
                             mo_objref_t todobr)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil != NULL,
                    "cemit_todo_first_at_end: bad csta@%p", csta);
  mo_cemitpayl_ty *cemp = csta->mo_cemsta_payl;
  MOM_ASSERTPRINTF (cemp && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC
                    && cemp->mo_cemit_locstate == csta,
                    "cemit_todo_first_at_end: bad payl@%p in csta@%p", cemp,
                    csta);
  MOM_ASSERTPRINTF (mo_dyncast_objref (todobr),
                    "cemit_todo_first_at_end: bad todobr");
  mo_objref_t curoutobj = mo_dyncast_objref (mo_objref_get_comp (todobr, 0));
  if (!curoutobj
      || curoutobj->mo_ob_paylkind !=
      MOM_PREDEF (signature_two_objects_to_void)
      || !curoutobj->mo_ob_payldata)
    MOM_CEMITFAILURE (csta,
                      "cemit_todo_first_at_end: todobr %s has wrong curoutobj %s",
                      mo_objref_pnamestr (todobr),
                      mo_objref_pnamestr (curoutobj));
  MOM_ASSERTPRINTF (mo_dyncastpayl_list (csta->mo_cemsta_endtodolist),
                    "bad endtotolist");
  mo_list_prepend (csta->mo_cemsta_endtodolist, todobr);
}                               /* end of mom_cemit_todo_first_at_end */


void
mom_cemit_declare_ctype (struct mom_cemitlocalstate_st *csta,
                         mo_objref_t typobr)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil != NULL,
                    "cemit_declare_ctype: bad csta@%p", csta);
  mo_cemitpayl_ty *cemp = csta->mo_cemsta_payl;
  MOM_ASSERTPRINTF (cemp && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC
                    && cemp->mo_cemit_locstate == csta,
                    "cemit_declare_ctype: bad payl@%p in csta@%p", cemp,
                    csta);
  MOM_ASSERTPRINTF (mo_dyncast_objref (typobr),
                    "cemit_declare_ctype: bad typobr");
  if (mo_hashset_contains (csta->mo_cemsta_hsetctypes, typobr))
    MOM_CEMITFAILURE (csta, "declare_ctype: typobr %s already declared",
                      mo_objref_pnamestr (typobr));
  char typobid[MOM_CSTRIDSIZ];
  memset (typobid, 0, sizeof (typobid));
  mo_objref_idstr (typobid, typobr);
#define MOM_NBCASE_CTYPE 331
#define CASE_PREDEFCTYPE_MOM(Ob) momphash_##Ob % MOM_NBCASE_CTYPE:	\
  if (typobr->mo_ob_class != MOM_PREDEF(Ob))			\
    goto defaultctypecase;					\
  goto labctype_##Ob;						\
  labctype_##Ob
#define CASE_GLOBALCTYPE_MOM(Ob) momghash_##Ob % MOM_NBCASE_CTYPE: \
  if (typobr != momglob_##Ob)					\
    goto defaultctypecase;					\
  goto labctype_##Ob;						\
  labctype_##Ob
  switch (mo_objref_hash (typobr->mo_ob_class) % MOM_NBCASE_CTYPE)
    {
    case CASE_PREDEFCTYPE_MOM (basic_ctype_class):
      {
        mo_value_t ccodv = mo_objref_get_attr (typobr, MOM_PREDEF (c_code));
        if (!mo_dyncast_string (ccodv))
          MOM_CEMITFAILURE (csta,
                            "declare_ctype: bad c_code %s in basic ctype %s (%s)",
                            mo_value_pnamestr (ccodv),
                            mo_objref_pnamestr (typobr), typobid);
        mom_cemit_printf (csta, "typedef %s mo%s_ty;\n",
                          mo_string_cstr (ccodv), typobid);
      }
      break;
    case CASE_PREDEFCTYPE_MOM (struct_pointer_ctype_class):
      mom_cemit_printf (csta, "typedef struct mo%s_ptrst* mo%s_ty;\n",
                        typobid, typobid);
      break;
    case CASE_PREDEFCTYPE_MOM (struct_ctype_class):
      mom_cemit_printf (csta, "typedef struct mo%s_st mo%s_ty;\n",
                        typobid, typobid);
      break;
    case CASE_PREDEFCTYPE_MOM (union_ctype_class):
      mom_cemit_printf (csta, "typedef union mo%s_un mo%s_ty;\n",
                        typobid, typobid);
      break;
    case CASE_PREDEFCTYPE_MOM (enum_ctype_class):
      mom_cemit_printf (csta, "typedef enum mo%s_en mo%s_ty;\n",
                        typobid, typobid);
      break;
    case CASE_PREDEFCTYPE_MOM (signature_class):
      {
        mo_value_t formtytup =
          mo_dyncast_tuple (mo_objref_get_attr
                            (typobr, MOM_PREDEF (formals_ctypes)));
        MOM_INFORMPRINTF ("cemit_declare_ctype: typobr %s formtytup %s",
                          mo_objref_pnamestr (typobr),
                          mo_value_pnamestr (formtytup));
        if (!formtytup)
          MOM_CEMITFAILURE (csta,
                            "cemit_declare_ctype: bad formals_ctypes in signature type %s",
                            mo_objref_pnamestr (typobr));
        mo_objref_t restypobr =
          mo_dyncast_objref (mo_objref_get_attr
                             (typobr, MOM_PREDEF (result_ctype)));
        if (!restypobr)
          MOM_CEMITFAILURE (csta,
                            "cemit_declare_ctype: bad result_ctype in signature type %s",
                            mo_objref_pnamestr (typobr));
        if (!mo_hashset_contains (csta->mo_cemsta_hsetctypes, restypobr))
          MOM_CEMITFAILURE (csta,
                            "cemit_declare_ctype: result_ctype %s unknown in signature type %s",
                            mo_objref_pnamestr (restypobr),
                            mo_objref_pnamestr (typobr));
        unsigned nbformals = mo_tuple_size (formtytup);
        for (unsigned foix = 0; foix < nbformals; foix++)
          {
            mo_objref_t curformobr = mo_tuple_nth (formtytup, foix);
            if (!mo_dyncast_objref (curformobr)
                || !mo_hashset_contains (csta->mo_cemsta_hsetctypes,
                                         curformobr))
              MOM_CEMITFAILURE (csta,
                                "cemit_declare_ctype: formal#%d ctype %s unknown in signature type %s",
                                foix, mo_objref_pnamestr (curformobr),
                                mo_objref_pnamestr (typobr));
          };
        mom_cemit_printf (csta, "typedef /*signature*/ ");
        mom_cemit_write_ctype_for (csta, restypobr, "", 0);
        mom_cemit_printf (csta, " mo%s_ty (", typobid);
        for (unsigned foix = 0; foix < nbformals; foix++)
          {
            if (foix > 0)
              fputs (", ", csta->mo_cemsta_fil);
            mo_objref_t curformobr = mo_tuple_nth (formtytup, foix);
            mom_cemit_write_ctype_for (csta, curformobr, "", 0);
          };
        mom_cemit_printf (csta, "); // signature %s\n", typobid);
      }
      break;
    default:
    defaultctypecase:
      {
        mom_cemit_printf (csta, "typedef /*other*/ ");
        const char *forstr = mom_gc_printf (" mo%s_ty", typobid);
        mom_cemit_write_ctype_for (csta, typobr, forstr, 0);
        mom_cemit_printf (csta, ";\n");
      };
      break;
    }
#undef MOM_NBCASE_CTYPE
#undef CASE_PREDEFCTYPE_MOM
  mo_value_t typobnamv = mo_objref_namev (typobr);
  if (typobnamv)
    mom_cemit_printf (csta, "typedef mo%s_ty mo_%s_ty;\n",
                      typobid, mo_string_cstr (typobnamv));
  csta->mo_cemsta_hsetctypes =
    mo_hashset_put (csta->mo_cemsta_hsetctypes, typobr);
}                               /* end mom_cemit_declare_ctype */


void
mom_cemit_define_fields (struct mom_cemitlocalstate_st *csta,
                         mo_objref_t typobr, bool isunion, int depth)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil != NULL,
                    "cemit_define_fields: bad csta@%p", csta);
  mo_cemitpayl_ty *cemp = csta->mo_cemsta_payl;
  MOM_ASSERTPRINTF (cemp && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC
                    && cemp->mo_cemit_locstate == csta,
                    "cemit_define_fields: bad payl@%p in csta@%p", cemp,
                    csta);
  MOM_ASSERTPRINTF (mo_dyncast_objref (typobr),
                    "cemit_define_fields: bad typobr");
  if (isunion && typobr->mo_ob_class != MOM_PREDEF (union_ctype_class))
    MOM_CEMITFAILURE (csta, "cemit_define_fields: %s is not a union but %s",
                      mo_objref_pnamestr (typobr),
                      mo_objref_pnamestr (typobr->mo_ob_class));
  if (depth > MOM_CEMIT_MAX_DEPTH)
    MOM_CEMITFAILURE (csta, "cemit_define_fields: %s too deep %d",
                      mo_objref_pnamestr (typobr), depth);
  mo_objref_t extendobr =
    mo_dyncast_objref (mo_objref_get_attr (typobr, MOM_PREDEF (extend)));
  if (extendobr)
    {
      if (!mo_hashset_contains (csta->mo_cemsta_hsetctypes, extendobr))
        MOM_CEMITFAILURE (csta,
                          "cemit_define_fields: %s extended by unknown type %s",
                          mo_objref_pnamestr (typobr),
                          mo_objref_pnamestr (extendobr));
      mom_cemit_printf (csta, " // type %s extended by %s\n",
                        mo_objref_pnamestr (typobr),
                        mo_objref_pnamestr (extendobr));
      mom_cemit_define_fields (csta, extendobr, isunion, depth + 1);
    }
  mo_value_t fieldtup =
    mo_dyncast_tuple (mo_objref_get_attr (typobr, MOM_PREDEF (fields)));
  if (!fieldtup)
    MOM_CEMITFAILURE (csta, "cemit_define_fields: %s has bad fields",
                      mo_objref_pnamestr (typobr));
  unsigned nbfields = mo_tuple_size (fieldtup);
  mom_cemit_printf (csta, " // %d fields in %s\n",
                    nbfields, mo_objref_pnamestr (typobr));
  for (unsigned flix = 0; flix < nbfields; flix++)
    {
      mo_objref_t fieldobr = mo_tuple_nth (fieldtup, flix);
      if (!mo_dyncast_objref (fieldobr))
        MOM_CEMITFAILURE (csta, "cemit_define_fields: %s has no field#%d",
                          mo_objref_pnamestr (typobr), flix);
      if (fieldobr->mo_ob_class != MOM_PREDEF (c_field_class))
        MOM_CEMITFAILURE (csta,
                          "cemit_define_fields: in %s field#%d %s is not of c_field_class but %s",
                          mo_objref_pnamestr (typobr), flix,
                          mo_objref_pnamestr (fieldobr),
                          mo_objref_pnamestr (fieldobr->mo_ob_class));
      mo_objref_t ftypobr =
        mo_dyncast_objref (mo_objref_get_attr
                           (fieldobr, MOM_PREDEF (c_type)));
      if (!ftypobr)
        MOM_CEMITFAILURE (csta,
                          "cemit_define_fields: in %s field#%d %s has no c_type",
                          mo_objref_pnamestr (typobr), flix,
                          mo_objref_pnamestr (fieldobr));
      if (!mo_hashset_contains (csta->mo_cemsta_hsetctypes, ftypobr))
        {
          MOM_INFORMPRINTF
            ("cemit_define_fields: fieldobr=%s ftypobr=%s hsetctypes=%s",
             mo_objref_pnamestr (fieldobr), mo_objref_pnamestr (ftypobr),
             mo_value_pnamestr (mo_hashset_elements_set
                                (csta->mo_cemsta_hsetctypes)));
          MOM_CEMITFAILURE (csta,
                            "cemit_define_fields: in %s field#%d %s with unknown c_type %s",
                            mo_objref_pnamestr (typobr), flix,
                            mo_objref_pnamestr (fieldobr),
                            mo_objref_pnamestr (ftypobr));
        }
      fputc (' ', csta->mo_cemsta_fil);
      mo_value_t fieldnamv = mo_objref_namev (fieldobr);
      char fieldid[MOM_CSTRIDSIZ];
      memset (fieldid, 0, sizeof (fieldid));
      const char *fieldstr = NULL;
      mo_objref_idstr (fieldid, fieldobr);
      if (fieldnamv)
        fieldstr = mom_gc_printf (" mo_%s_fd ", mo_string_cstr (fieldnamv));
      else
        fieldstr = mom_gc_printf (" mo%s_fd", fieldid);
      mom_cemit_write_ctype_for (csta, ftypobr, fieldstr, 0);
      mom_cemit_printf (csta, ";  // %s\n", fieldid);
    }
}                               /* end mom_cemit_define_fields */


void
mom_cemit_define_enumerators (struct mom_cemitlocalstate_st *csta,
                              mo_objref_t typobr, mo_objref_t parobr,
                              int depth, long *pprev)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil != NULL,
                    "cemit_define_enumerators: bad csta@%p", csta);
  mo_cemitpayl_ty *cemp = csta->mo_cemsta_payl;
  MOM_ASSERTPRINTF (cemp && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC
                    && cemp->mo_cemit_locstate == csta,
                    "cemit_define_enumerators: bad payl@%p in csta@%p", cemp,
                    csta);
  MOM_ASSERTPRINTF (depth > 0 || typobr == parobr,
                    "bad depth %d in typobr %s", depth,
                    mo_objref_pnamestr (typobr));
  MOM_ASSERTPRINTF (mo_dyncast_objref (typobr),
                    "cemit_define_enumerators: bad typobr");
  MOM_ASSERTPRINTF (mo_dyncast_objref (parobr),
                    "cemit_define_enumerators: bad parobr");
  MOM_ASSERTPRINTF (pprev != NULL, "cemit_define_enumerators: no pprev");
  if (depth > MOM_CEMIT_MAX_DEPTH)
    MOM_CEMITFAILURE (csta, "cemit_define_enumerators: %s too deep %d",
                      mo_objref_pnamestr (typobr), depth);
  if (!mo_hashset_contains (csta->mo_cemsta_hsetctypes, typobr))
    MOM_CEMITFAILURE (csta,
                      "cemit_define_enumerators: %s is unknown type",
                      mo_objref_pnamestr (typobr));
  MOM_BACKTRACEPRINTF
    ("cemit_define_enumerators: typobr=%s parobr=%s depth %d *pprev@%p=%ld",
     mo_objref_pnamestr (typobr), mo_objref_pnamestr (parobr), depth, pprev,
     *pprev);
  mo_value_t enumstup =
    mo_dyncast_tuple (mo_objref_get_attr (typobr, MOM_PREDEF (enumerators)));
  if (!enumstup)
    MOM_CEMITFAILURE (csta,
                      "cemit_define_enumerators: %s without enumerators",
                      mo_objref_pnamestr (typobr));
  mo_objref_t extendobr =
    mo_dyncast_objref (mo_objref_get_attr (typobr, MOM_PREDEF (extend)));
  if (extendobr)
    {
      if (!mo_hashset_contains (csta->mo_cemsta_hsetctypes, extendobr))
        MOM_CEMITFAILURE (csta,
                          "cemit_define_enumerators: %s extended by unknown type %s",
                          mo_objref_pnamestr (typobr),
                          mo_objref_pnamestr (extendobr));
      mom_cemit_printf (csta, " // enumeration %s extended by %s\n",
                        mo_objref_pnamestr (typobr),
                        mo_objref_pnamestr (extendobr));
      mom_cemit_define_enumerators (csta, extendobr, parobr, depth + 1,
                                    pprev);
    }
  unsigned nbenums = mo_tuple_size (enumstup);
  mom_cemit_printf (csta, " // %d enumerators in %s, starting after %ld\n",
                    nbenums, mo_objref_pnamestr (typobr), (*pprev));

  for (unsigned eix = 0; eix < nbenums; eix++)
    {
      long curval = (*pprev) + 1;
      mo_objref_t curenumobr =
        mo_dyncast_objref (mo_tuple_nth (enumstup, eix));
      if (!curenumobr)
        MOM_CEMITFAILURE (csta,
                          "cemit_define_enumerators: %s with missing enumerator#%d",
                          mo_objref_pnamestr (typobr), eix);
      if (curenumobr->mo_ob_class != MOM_PREDEF (enumerator_class))
        MOM_CEMITFAILURE (csta,
                          "cemit_define_enumerators: %s with bad enumerator#%d %s "
                          "of class %s (enumerator_class expected)",
                          mo_objref_pnamestr (typobr), eix,
                          mo_objref_pnamestr (curenumobr),
                          mo_objref_pnamestr (curenumobr->mo_ob_class));
      char curenumid[MOM_CSTRIDSIZ];
      memset (curenumid, 0, sizeof (curenumid));
      mo_objref_idstr (curenumid, curenumobr);
      mo_value_t curvaluev =
        mo_objref_get_attr (curenumobr, MOM_PREDEF (value));
      curval = mo_value_to_int (curvaluev, curval);
      mom_cemit_todo_put_attr (csta, curenumobr, MOM_PREDEF (value),
                               mo_int_to_value (curval));
      MOM_INFORMPRINTF
        ("cemit_define_enumerators: typobr=%s curenumobr=%s curval=%ld curvaluev=%s",
         mo_objref_pnamestr (typobr), mo_objref_pnamestr (curenumobr), curval,
         mo_value_pnamestr (curvaluev));
      mo_value_t curenumnamv = mo_objref_namev (curenumobr);
      if (typobr == parobr && depth == 0)
        {
          if (curenumnamv)
            {
              mom_cemit_printf (csta, " mo_%s_ev=%ld,\n",
                                mo_string_cstr (curenumnamv), curval);
              mom_cemit_printf (csta, "#define mo%s_ev mo_%s_ev\n",
                                curenumid, mo_string_cstr (curenumnamv));
            }
          else
            {
              mom_cemit_printf (csta, " mo%s_ev=%ld,\n", curenumid, curval);
            }
        }
      else
        {
          mom_cemit_printf (csta, " mo_%s__x__%s__ev=%ld, //%s\n",
                            mo_objref_pnamestr (parobr),
                            mo_objref_pnamestr (curenumobr), curval,
                            curenumid);
        }
      *pprev = curval;
    }
}                               /* end of mom_cemit_define_enumerators */


void
mom_cemit_define_ctype (struct mom_cemitlocalstate_st *csta,
                        mo_objref_t typobr)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil != NULL,
                    "cemit_define_ctype: bad csta@%p", csta);
  mo_cemitpayl_ty *cemp = csta->mo_cemsta_payl;
  MOM_ASSERTPRINTF (cemp && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC
                    && cemp->mo_cemit_locstate == csta,
                    "cemit_define_ctype: bad payl@%p in csta@%p", cemp, csta);
  MOM_ASSERTPRINTF (mo_dyncast_objref (typobr),
                    "cemit_define_ctype: bad typobr");
  MOM_ASSERTPRINTF (mo_hashset_contains (csta->mo_cemsta_hsetctypes, typobr),
                    "cemit_define_ctype: unknown typobr %s",
                    mo_objref_pnamestr (typobr));
  char typobid[MOM_CSTRIDSIZ];
  memset (typobid, 0, sizeof (typobid));
  mo_objref_idstr (typobid, typobr);
  mo_value_t typnamv = mo_objref_namev (typobr);
#define MOM_NBCASE_CTYPE 331
#define CASE_PREDEFCTYPE_MOM(Ob) momphash_##Ob % MOM_NBCASE_CTYPE:	\
  if (typobr->mo_ob_class != MOM_PREDEF(Ob))			\
    goto defaultctypecase;					\
  goto labctype_##Ob;						\
  labctype_##Ob
#define CASE_GLOBALCTYPE_MOM(Ob) momghash_##Ob % MOM_NBCASE_CTYPE: \
  if (typobr != momglob_##Ob)					\
    goto defaultctypecase;					\
  goto labctype_##Ob;						\
  labctype_##Ob
  switch (mo_objref_hash (typobr->mo_ob_class) % MOM_NBCASE_CTYPE)
    {
    case CASE_PREDEFCTYPE_MOM (basic_ctype_class):
      break;
    case CASE_PREDEFCTYPE_MOM (struct_pointer_ctype_class):
      mom_cemit_printf (csta, "struct mo%s_ptrst {", typobid);
      if (typnamv)
        mom_cemit_printf (csta, "// %s\n", mo_string_cstr (typnamv));
      else
        fputc ('\n', csta->mo_cemsta_fil);
      mom_cemit_define_fields (csta, typobr, false, 0);
      mom_cemit_printf (csta, "}; // end struct mo%s_ptrst\n", typobid);
      break;
    case CASE_PREDEFCTYPE_MOM (struct_ctype_class):
      mom_cemit_printf (csta, "struct mo%s_st {", typobid);
      if (typnamv)
        mom_cemit_printf (csta, "// %s\n", mo_string_cstr (typnamv));
      else
        fputc ('\n', csta->mo_cemsta_fil);
      mom_cemit_define_fields (csta, typobr, false, 0);
      mom_cemit_printf (csta, "}; // end struct mo%s_st\n", typobid);
      break;
    case CASE_PREDEFCTYPE_MOM (union_ctype_class):
      mom_cemit_printf (csta, "union mo%s_un {", typobid);
      if (typnamv)
        mom_cemit_printf (csta, "// %s\n", mo_string_cstr (typnamv));
      else
        fputc ('\n', csta->mo_cemsta_fil);
      mom_cemit_define_fields (csta, typobr, true, 0);
      mom_cemit_printf (csta, "}; // end union mo%s_un\n", typobid);
      break;
    case CASE_PREDEFCTYPE_MOM (enum_ctype_class):
      {
        long prevnum = -1;
        mom_cemit_printf (csta, "enum mo%s_en {", typobid);
        if (typnamv)
          mom_cemit_printf (csta, "// %s\n", mo_string_cstr (typnamv));
        else
          fputc ('\n', csta->mo_cemsta_fil);
        mom_cemit_define_enumerators (csta, typobr, typobr, 0, &prevnum);
        mom_cemit_printf (csta, "}; // end enum mo%s_en\n", typobid);
      }
      break;
    case CASE_PREDEFCTYPE_MOM (signature_class):
      break;
    default:
    defaultctypecase:
      if (typnamv)
        mom_cemit_printf (csta,
                          "// type %s (%s) of class %s dont need a definition\n",
                          typobid, mo_objref_pnamestr (typobr),
                          mo_objref_pnamestr (typobr->mo_ob_class));
      else
        mom_cemit_printf (csta,
                          "// type %s of class %s dont need a definition\n",
                          typobid, mo_objref_pnamestr (typobr->mo_ob_class));
      if (!typobr->mo_ob_class)
        MOM_CEMITFAILURE (csta,
                          "cemit_define_ctype: typobr %s has no class",
                          mo_objref_pnamestr (typobr));
      break;
    }
#undef MOM_NBCASE_CTYPE
#undef CASE_PREDEFCTYPE_MOM
#undef CASE_GLOBALCTYPE_MOM
}                               /* end mom_cemit_define_ctype */

void
mom_cemit_ctypes (struct mom_cemitlocalstate_st *csta)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil != NULL,
                    "cemit_ctypes: bad csta@%p", csta);
  mo_cemitpayl_ty *cemp = csta->mo_cemsta_payl;
  MOM_ASSERTPRINTF (cemp && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC
                    && cemp->mo_cemit_locstate == csta,
                    "cemit_ctypes: bad payl@%p in csta@%p", cemp, csta);
  mo_value_t ctypv =
    mo_objref_get_attr (cemp->mo_cemit_modobj, MOM_PREDEF (c_type));
  if (!ctypv)
    {
      mom_cemit_printf (csta, "\n// no types\n");
      return;
    }
  if (!mo_dyncast_tuple (ctypv))
    MOM_CEMITFAILURE (csta, "bad c_type %s", mo_value_pnamestr (ctypv));
  unsigned nbctyp = mo_tuple_size (ctypv);
  mom_cemit_printf (csta, "\n// %d types ***\n", nbctyp);
  mom_cemit_printf (csta, "//// %d type declarations:\n", nbctyp);
  // first loop to emit typedefs
  for (unsigned tix = 0; tix < nbctyp; tix++)
    {
      mo_objref_t ctypob = mo_tuple_nth (ctypv, tix);
      char ctypid[MOM_CSTRIDSIZ];
      memset (ctypid, 0, sizeof (ctypid));
      mo_objref_idstr (ctypid, ctypob);
      MOM_ASSERTPRINTF (mo_dyncast_objref (ctypob), "bad ctypob tix#%d", tix);
      mo_value_t ctypnamob = mo_objref_namev (ctypob);
      if (ctypnamob != NULL)
        mom_cemit_printf (csta, "\n// type#%d declaration for %s (%s)\n",
                          tix, mo_string_cstr (ctypnamob), ctypid);
      else
        mom_cemit_printf (csta, "\n// type#%d declaration for %s\n",
                          tix, ctypid);
      mom_cemit_declare_ctype (csta, ctypob);
    }
  mom_cemit_printf (csta, "\n//// %d type definitions:\n", nbctyp);
  // second loop to emit enum, struct, union-s.... i.e. aggregate type definitions
  for (unsigned tix = 0; tix < nbctyp; tix++)
    {
      mo_objref_t ctypob = mo_tuple_nth (ctypv, tix);
      char ctypid[MOM_CSTRIDSIZ];
      memset (ctypid, 0, sizeof (ctypid));
      mo_objref_idstr (ctypid, ctypob);
      MOM_ASSERTPRINTF (mo_dyncast_objref (ctypob), "bad ctypob tix#%d", tix);
      mo_value_t ctypnamob = mo_objref_namev (ctypob);
      if (ctypnamob != NULL)
        mom_cemit_printf (csta, "\n// type definition#%d for %s (%s)\n",
                          tix, mo_string_cstr (ctypnamob), ctypid);
      else
        mom_cemit_printf (csta, "\n// type definition#%d for %s\n",
                          tix, ctypid);
      mom_cemit_define_ctype (csta, ctypob);
    }
  mom_cemit_printf (csta, "\n// end of %d types definitions ***\n\n", nbctyp);
}                               /* end of mom_cemit_ctypes */



void
mom_cemit_object_declare (struct mom_cemitlocalstate_st *csta, mo_objref_t ob,
                          bool checknoinline)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil != NULL,
                    "cemit_object_declare: bad csta@%p", csta);
  mo_cemitpayl_ty *cemp = csta->mo_cemsta_payl;
  MOM_ASSERTPRINTF (cemp && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC
                    && cemp->mo_cemit_locstate == csta,
                    "cemit_object_declare: bad payl@%p in csta@%p", cemp,
                    csta);
  MOM_ASSERTPRINTF (mo_dyncast_objref (ob), "cemit_object_declare: bad ob");
  char obid[MOM_CSTRIDSIZ];
  memset (obid, 0, sizeof (obid));
  mo_objref_idstr (obid, ob);
  mo_value_t obnamev = mo_objref_namev (ob);
  const char *shortnamob = mo_objref_shortnamestr (ob);
  mo_objref_t objclass = ob->mo_ob_class;
  if (!objclass)
    MOM_CEMITFAILURE (csta,
                      "cemit_object_declare: object %s to declare without class",
                      mo_objref_pnamestr (ob));

#define MOM_NBCLASS_OBJDECL 83
#define CASE_PREDEFCLASS_OBJDECL_MOM(Ob) momphash_##Ob % MOM_NBCLASS_OBJDECL: \
  if (objclass != MOM_PREDEF(Ob))					\
    goto defaultclasscase;						\
  goto labclass_##Ob;							\
  labclass_##Ob
  switch (mo_objref_hash (objclass) % MOM_NBCLASS_OBJDECL)
    {
    case CASE_PREDEFCLASS_OBJDECL_MOM (c_inlined_class):
      {
        if (checknoinline)
          MOM_CEMITFAILURE (csta,
                            "cemit_object_declare: unexpected inlined function object %s",
                            mo_objref_pnamestr (ob));
        mo_objref_t signob =
          mo_dyncast_objref (mo_objref_get_attr (ob, MOM_PREDEF (signature)));
        if (!signob)
          MOM_CEMITFAILURE (csta,
                            "cemit_object_declare: inlined function object %s without signature",
                            mo_objref_pnamestr (ob));
        if (!mo_hashset_contains (csta->mo_cemsta_hsetctypes, signob))
          MOM_CEMITFAILURE (csta,
                            "cemit_object_declare: inlined function object %s with unknown signature %s",
                            mo_objref_pnamestr (ob),
                            mo_objref_pnamestr (signob));
        mom_cemit_printf (csta,
                          "static inline mo_%s_ty " MOM_FUNC_PREFIX "%s;\n",
                          mo_objref_shortnamestr (signob), shortnamob);
        if (obnamev)
          mom_cemit_printf (csta,
                            "#define " MOM_FUNC_PREFIX "%s " MOM_FUNC_PREFIX
                            "%s\n", obid, mo_string_cstr (obnamev));
      }
      break;
    case CASE_PREDEFCLASS_OBJDECL_MOM (c_routine_class):
      {
        mo_objref_t signob =
          mo_dyncast_objref (mo_objref_get_attr (ob, MOM_PREDEF (signature)));
        if (!signob)
          MOM_CEMITFAILURE (csta,
                            "cemit_object_declare: global function object %s without signature",
                            mo_objref_pnamestr (ob));
        if (!mo_hashset_contains (csta->mo_cemsta_hsetctypes, signob))
          MOM_CEMITFAILURE (csta,
                            "cemit_object_declare: global function object %s with unknown signature %s",
                            mo_objref_pnamestr (ob),
                            mo_objref_pnamestr (signob));
        mom_cemit_printf (csta,
                          "extern /*routine*/ mo_%s_ty " MOM_FUNC_PREFIX
                          "%s;\n", mo_objref_shortnamestr (signob),
                          shortnamob);
      }
      break;
    case CASE_PREDEFCLASS_OBJDECL_MOM (global_c_data_class):
      {
        mo_objref_t ctypob =
          mo_dyncast_objref (mo_objref_get_attr (ob, MOM_PREDEF (c_type)));
        if (!ctypob)
          MOM_CEMITFAILURE (csta,
                            "cemit_object_declare: global data object %s without c_type",
                            mo_objref_pnamestr (ob));
        if (!mo_hashset_contains (csta->mo_cemsta_hsetctypes, ctypob))
          MOM_CEMITFAILURE (csta,
                            "cemit_object_declare: global data object %s with unknown c_type %s",
                            mo_objref_pnamestr (ob),
                            mo_objref_pnamestr (ctypob));
        mom_cemit_printf (csta, "extern /*data*/ mo_%s_ty mo_%s_data;\n",
                          mo_objref_shortnamestr (ctypob), shortnamob);
        if (obnamev)
          mom_cemit_printf (csta, "#define mo_%s_data mo_%s_data\n",
                            obid, mo_string_cstr (obnamev));
      }
      break;
    case CASE_PREDEFCLASS_OBJDECL_MOM (threadlocal_c_data_class):
      {
        mo_objref_t ctypob =
          mo_dyncast_objref (mo_objref_get_attr (ob, MOM_PREDEF (c_type)));
        if (!ctypob)
          MOM_CEMITFAILURE (csta,
                            "cemit_object_declare: threadlocal data object %s without c_type",
                            mo_objref_pnamestr (ob));
        if (!mo_hashset_contains (csta->mo_cemsta_hsetctypes, ctypob))
          MOM_CEMITFAILURE (csta,
                            "cemit_object_declare: threadlocal data object %s with unknown c_type %s",
                            mo_objref_pnamestr (ob),
                            mo_objref_pnamestr (ctypob));
        mom_cemit_printf (csta,
                          "extern /*threadlocal*/ mo_%s_ty mo_%s_data;\n",
                          mo_objref_shortnamestr (ctypob), shortnamob);
        if (obnamev)
          mom_cemit_printf (csta, "#define mo_%s_data mo_%s_data\n",
                            obid, mo_string_cstr (obnamev));
      }
      break;
    defaultclasscase:
    default:
      MOM_CEMITFAILURE (csta,
                        "cemit_object_declare: object %s to declare with bad %s class",
                        mo_objref_pnamestr (ob),
                        mo_objref_pnamestr (objclass));
    };
}                               /* end mom_cemit_object_declare */




void
mom_cemit_declarations (struct mom_cemitlocalstate_st *csta)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil != NULL,
                    "cemit_declarations: bad csta@%p", csta);
  mo_cemitpayl_ty *cemp = csta->mo_cemsta_payl;
  MOM_ASSERTPRINTF (cemp && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC
                    && cemp->mo_cemit_locstate == csta,
                    "cemit_declarations: bad payl@%p in csta@%p", cemp, csta);
  //
  mo_value_t externv =
    mo_objref_get_attr (cemp->mo_cemit_modobj, MOM_PREDEF (extern));
  if (externv && !mo_dyncast_sequence (externv))
    MOM_CEMITFAILURE (csta, "bad extern %s", mo_value_pnamestr (externv));
  unsigned nbextern = mo_sequence_size (externv);
  //
  mo_value_t codev =
    mo_objref_get_attr (cemp->mo_cemit_modobj, MOM_PREDEF (code));
  if (codev && !mo_dyncast_sequence (codev))
    MOM_CEMITFAILURE (csta, "bad code %s", mo_value_pnamestr (codev));
  unsigned nbcode = mo_sequence_size (codev);
  //
  mo_value_t datav =
    mo_objref_get_attr (cemp->mo_cemit_modobj, MOM_PREDEF (data));
  if (datav && !mo_dyncast_sequence (datav))
    MOM_CEMITFAILURE (csta, "bad data %s", mo_value_pnamestr (datav));
  unsigned nbdata = mo_sequence_size (datav);
  //
  mom_cemit_printf (csta,
                    "\n\n// %d declarations (%d extern, %d code, %d data) *****\n",
                    nbextern + nbcode + nbdata, nbextern, nbcode, nbdata);
  //
  if (nbextern > 0)
    {
      mom_cemit_printf (csta, "\n// %d extern declarations:\n", nbextern);
      for (unsigned ix = 0; ix < nbextern; ix++)
        mom_cemit_object_declare (csta, mo_sequence_nth (externv, ix), true);
    }
  else
    mom_cemit_printf (csta, "\n// no extern declarations\n");
  //
  if (nbdata > 0)
    {
      mom_cemit_printf (csta, "\n// %d data declarations:\n", nbdata);
      for (unsigned ix = 0; ix < nbdata; ix++)
        mom_cemit_object_declare (csta, mo_sequence_nth (datav, ix), true);
    }
  else
    mom_cemit_printf (csta, "\n// no data declarations\n");
  //
  if (nbcode > 0)
    {
      mom_cemit_printf (csta, "\n// %d code declarations:\n", nbcode);
      for (unsigned ix = 0; ix < nbcode; ix++)
        mom_cemit_object_declare (csta, mo_sequence_nth (codev, ix), false);
    }
  else
    mom_cemit_printf (csta, "\n// no code declarations\n");
}                               /* end of mom_cemit_declarations */


void
mom_cemit_data_definitions (struct mom_cemitlocalstate_st *csta)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil != NULL,
                    "cemit_data_definitions: bad csta@%p", csta);
  mo_cemitpayl_ty *cemp = csta->mo_cemsta_payl;
  MOM_ASSERTPRINTF (cemp && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC
                    && cemp->mo_cemit_locstate == csta,
                    "cemit_data_definitions: bad payl@%p in csta@%p", cemp,
                    csta);
  mo_value_t datav =
    mo_objref_get_attr (cemp->mo_cemit_modobj, MOM_PREDEF (data));
  if (!datav)
    return;
  MOM_ASSERTPRINTF (!mo_dyncast_sequence (datav),
                    "cemit_data_definitions: bad data %s",
                    mo_value_pnamestr (datav));
  unsigned nbdata = mo_sequence_size (datav);
  mom_cemit_printf (csta, "\n\n// %d data definitions\n", nbdata);
  for (unsigned dix = 0; dix < nbdata; dix++)
    {
      mo_objref_t curdob = mo_sequence_nth (datav, dix);
      MOM_ASSERTPRINTF (mo_dyncast_objref (curdob),
                        "cemit_data_definitions: bad data #%d", dix);
      char curobid[MOM_CSTRIDSIZ];
      memset (curobid, 0, sizeof (curobid));
      mo_objref_idstr (curobid, curdob);
      mo_value_t curobnamev = mo_objref_namev (curdob);
      mo_objref_t objclass = curdob->mo_ob_class;
      if (objclass == MOM_PREDEF (global_c_data_class)
          || objclass == MOM_PREDEF (threadlocal_c_data_class))
        {
          mo_objref_t ctypob =
            mo_dyncast_objref (mo_objref_get_attr
                               (curdob, MOM_PREDEF (c_type)));
          MOM_ASSERTPRINTF (mo_hashset_contains
                            (csta->mo_cemsta_hsetctypes, ctypob),
                            "cemit_data_definitions: bad ctypob %s for curdob %s",
                            mo_objref_pnamestr (ctypob),
                            mo_objref_pnamestr (curdob));
          if (mom_cemit_ctype_is_scalar (csta, ctypob))
            mom_cemit_printf (csta, "mo_%s_ty mo_%s_data\n"
                              " = /*scalar*/0; // %s\n",
                              mo_objref_shortnamestr (ctypob),
                              mo_objref_shortnamestr (curdob), curobid);
          else
            mom_cemit_printf (csta, "mo_%s_ty mo_%s_data\n"
                              " = /*aggregate*/{}; // %s\n",
                              mo_objref_shortnamestr (ctypob),
                              mo_objref_shortnamestr (curdob), curobid);
        }
      else                      // should not happen
        MOM_FATAPRINTF ("cemit_data_definitions: bad curdob %s of class %s",
                        mo_objref_pnamestr (curdob),
                        mo_objref_pnamestr (objclass));
    }
  mom_cemit_printf (csta, "\n// end of %d data definitions\n\n", nbdata);
}                               /* end of mom_cemit_data_definitions */

void
mom_cemit_function_definitions (struct mom_cemitlocalstate_st *csta)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil != NULL,
                    "cemit_function_definitions: bad csta@%p", csta);
  mo_cemitpayl_ty *cemp = csta->mo_cemsta_payl;
  MOM_ASSERTPRINTF (cemp && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC
                    && cemp->mo_cemit_locstate == csta,
                    "cemit_function_definitions: bad payl@%p in csta@%p",
                    cemp, csta);
}                               /* end of mom_cemit_function_definitions */

void
mom_cemit_set (struct mom_cemitlocalstate_st *csta)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil != NULL,
                    "cemit_set: bad csta@%p", csta);
  mo_cemitpayl_ty *cemp = csta->mo_cemsta_payl;
  MOM_ASSERTPRINTF (cemp && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC
                    && cemp->mo_cemit_locstate == csta,
                    "cemit_set: bad payl@%p in csta@%p", cemp, csta);
  char modulid[MOM_CSTRIDSIZ];
  memset (modulid, 0, sizeof (modulid));
  mo_objref_idstr (modulid, cemp->mo_cemit_modobj);
  mo_value_t setv = mo_objref_get_attr (cemp->mo_cemit_modobj, momglob_set);
  if (!setv)
    {
      mom_cemit_printf (csta, "\n// no set of objects\n");
      return;
    }
  if (!mo_dyncast_set (setv))
    MOM_CEMITFAILURE (csta, "bad set %s", mo_value_pnamestr (setv));
  unsigned nbset = mo_set_size (setv);
  mom_cemit_printf (csta, "\n// set of %d objects\n", nbset);
  for (unsigned eix = 0; eix < nbset; eix++)
    {
      mo_objref_t elemob = mo_set_nth (setv, eix);
      MOM_ASSERTPRINTF (mo_dyncast_objref (elemob), "bad elemob#%d", eix);
      mo_value_t elemnamv = mo_objref_namev (elemob);
      char elemid[MOM_CSTRIDSIZ];
      memset (elemid, 0, sizeof (elemid));
      mo_objref_idstr (elemid, elemob);
      if (elemnamv)
        {
          mom_cemit_printf (csta,
                            "static mo_objref_t mo_%s_ob; // element %s\n",
                            mo_string_cstr (elemnamv), elemid);
          mom_cemit_printf (csta, "#define mo%s_ob mo_%s_ob\n", elemid,
                            mo_string_cstr (elemnamv));
        }
      else
        {
          mom_cemit_printf (csta,
                            "static mo_objref_t mo%s_ob; // element %s\n",
                            elemid, elemid);
        }
    }
  mom_cemit_printf (csta, "\n// initialization of %d objects\n", nbset);
  mom_cemit_printf (csta, "void " MOM_MODULEINIT_PREFIX "%s (void) {\n",
                    modulid);
  for (unsigned eix = 0; eix < nbset; eix++)
    {
      mo_objref_t elemob = mo_set_nth (setv, eix);
      char elemid[MOM_CSTRIDSIZ];
      memset (elemid, 0, sizeof (elemid));
      mo_objref_idstr (elemid, elemob);
      MOM_ASSERTPRINTF (mo_dyncast_objref (elemob), "bad elemob#%d", eix);
      mom_cemit_printf (csta, " if (!mo_%s_ob) // %s \n",
                        mo_objref_shortnamestr (elemob), elemid);
      mom_cemit_printf (csta,
                        "    mo_%s_ob =\n"
                        "      mo_objref_find_hid_loid(%lu,%llu); // %s\n",
                        mo_objref_shortnamestr (elemob),
                        (unsigned long) elemob->mo_ob_hid,
                        (unsigned long long) elemob->mo_ob_loid, elemid);
    }
  mom_cemit_printf (csta, "} /*end " MOM_MODULEINIT_PREFIX "%s */\n",
                    modulid);
  mo_value_t modulnamv = mo_objref_namev (cemp->mo_cemit_modobj);
  if (modulnamv)
    {
      mom_cemit_printf (csta, "void " MOM_MODULEINIT_PREFIX "%s (void) {\n",
                        mo_string_cstr (modulnamv));
      mom_cemit_printf (csta, "  " MOM_MODULEINIT_PREFIX "%s ();\n", modulid);
      mom_cemit_printf (csta, "} /*end " MOM_MODULEINIT_PREFIX "%s */\n",
                        mo_string_cstr (modulnamv));
    }
  mom_cemit_printf (csta,
                    "\n#ifdef MONIMELT_MODULE\n"
                    "void monimelt_module_init(void) __attribute__((constructor));\n");
  mom_cemit_printf (csta,
                    "void monimelt_module_init(void) {\n"
                    "   " MOM_MODULEINIT_PREFIX "%s ();\n"
                    "} /* end monimelt_module_init */\n"
                    "#endif /*MONIMELT_MODULE*/\n\n", modulid);
}                               /* end of mom_cemit_set */



void
mom_cemit_do_at_end (struct mom_cemitlocalstate_st *csta)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil != NULL,
                    "cemit_do_at_end: bad csta@%p", csta);
  mo_cemitpayl_ty *cemp = csta->mo_cemsta_payl;
  MOM_ASSERTPRINTF (cemp && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC
                    && cemp->mo_cemit_locstate == csta,
                    "cemit_do_at_end: bad payl@%p in csta@%p", cemp, csta);
  long nbdone = 0;
  mo_objref_t objcemit = csta->mo_cemsta_objcemit;
  MOM_ASSERTPRINTF (mo_dyncast_objref (objcemit),
                    "cemit_do_at_end: bad objcemit");
  while (mo_list_non_empty (csta->mo_cemsta_endtodolist))
    {
      mo_value_t curtodov = mo_list_head (csta->mo_cemsta_endtodolist);
      mo_list_pop_head (csta->mo_cemsta_endtodolist);
      nbdone++;
      mo_objref_t curtodobj = mo_dyncast_objref (curtodov);
      if (!curtodobj)
        MOM_CEMITFAILURE (csta, "cemit_do_at_end: bad todo#%ld %s",
                          nbdone, mo_value_pnamestr (curtodov));
      mo_objref_t curoutobj =
        mo_dyncast_objref (mo_objref_get_comp (curtodobj, 0));
      if (!curoutobj
          || curoutobj->mo_ob_paylkind !=
          MOM_PREDEF (signature_two_objects_to_void)
          || !curoutobj->mo_ob_payldata)
        {
          MOM_BACKTRACEPRINTF
            ("cemit_do_at_end: %s*bad curoutobj*%s %s in todo#%ld %s",
             MOM_TERMWARNCOLOR, MOM_TERMPLAIN, mo_objref_pnamestr (curtodobj),
             nbdone, mo_objref_pnamestr (curoutobj));
          continue;
        }
      mo_signature_two_objects_to_void_sigt *funrout =
        curoutobj->mo_ob_payldata;
      (*funrout) (curtodobj, objcemit);
    }
}                               /* end mom_cemit_do_at_end */


mo_value_t
mo_objref_cemit_generate (mo_objref_t obrcem)
{
  mo_cemitpayl_ty *cemp = mo_objref_get_cemit (obrcem);
  if (!cemp)
    return mo_make_string_sprintf ("bad cemit object %s",
                                   mo_objref_pnamestr (obrcem));
  if (cemp->mo_cemit_locstate != NULL)
    {
      MOM_WARNPRINTF
        ("cemit object %s is already active for module %s",
         mo_objref_pnamestr (obrcem),
         mo_objref_cemit_detailstr (cemp->mo_cemit_modobj));
      return
        mo_make_string_sprintf
        ("cemit object %s already active for module %s",
         mo_objref_pnamestr (obrcem),
         mo_objref_cemit_detailstr (cemp->mo_cemit_modobj));
    }
  struct mom_cemitlocalstate_st cemitstate = { };
  memset (&cemitstate, 0, sizeof (cemitstate));
  cemitstate.mo_cemsta_nmagic = MOM_CEMITSTATE_MAGIC;
  cemitstate.mo_cemsta_objcemit = obrcem;
  cemitstate.mo_cemsta_payl = cemp;
  cemitstate.mo_cemsta_hsetctypes =
    mo_hashset_reserve (NULL, 2 * (MOM_NB_GLOBAL + MOM_NB_PREDEFINED) + 30);
  cemitstate.mo_cemsta_endtodolist = mo_list_make ();
#define MOM_ADD_GLOBAL_CTYPE(Glob)			\
  cemitstate.mo_cemsta_hsetctypes =			\
    mo_hashset_put(cemitstate.mo_cemsta_hsetctypes,	\
		   (Glob))
  MOM_ADD_GLOBAL_CTYPE (momglob_int);
  MOM_ADD_GLOBAL_CTYPE (momglob_bool);
  MOM_ADD_GLOBAL_CTYPE (momglob_char);
  MOM_ADD_GLOBAL_CTYPE (momglob_double);
  MOM_ADD_GLOBAL_CTYPE (momglob_long);
  MOM_ADD_GLOBAL_CTYPE (momglob_void);
  MOM_ADD_GLOBAL_CTYPE (momglob_int16_t);
  MOM_ADD_GLOBAL_CTYPE (momglob_int32_t);
  MOM_ADD_GLOBAL_CTYPE (momglob_int64_t);
  MOM_ADD_GLOBAL_CTYPE (momglob_intptr_t);
  MOM_ADD_GLOBAL_CTYPE (momglob_uint16_t);
  MOM_ADD_GLOBAL_CTYPE (momglob_uint32_t);
  MOM_ADD_GLOBAL_CTYPE (momglob_uint64_t);
  MOM_ADD_GLOBAL_CTYPE (momglob_uintptr_t);
#undef MOM_ADD_GLOBAL_CTYPE
  mo_objref_idstr (cemitstate.mo_cemsta_modid, cemp->mo_cemit_modobj);
  int errlin = setjmp (cemitstate.mo_cemsta_jmpbuf);
  if (errlin)
    {
      MOM_ASSERTPRINTF (mo_dyncast_string (cemitstate.mo_cemsta_errstr),
                        "bad errstr in cemitstate@%p", &cemitstate);
      MOM_WARNPRINTF_AT (__FILE__, errlin,
                         "cemit_generate failure: %s (module %s, emitter %s)",
                         mo_string_cstr (cemitstate.mo_cemsta_errstr),
                         mo_objref_pnamestr (cemp->mo_cemit_modobj),
                         mo_objref_pnamestr (obrcem));
      if (cemitstate.mo_cemsta_fil)
        {
          fprintf (cemitstate.mo_cemsta_fil,
                   "\n\n\n/////@@@@@@@!!!!!! \n"
                   "#error @%d: %s (module %s, emitter %s)\n",
                   errlin,
                   mo_string_cstr (cemitstate.mo_cemsta_errstr),
                   mo_objref_pnamestr (cemp->mo_cemit_modobj),
                   mo_objref_pnamestr (obrcem));
          if (fclose (cemitstate.mo_cemsta_fil))
            MOM_FATAPRINTF
              ("cemit_generate: failed to close emitted file for module %s on error",
               mo_objref_pnamestr (cemp->mo_cemit_modobj));
          cemitstate.mo_cemsta_fil = NULL;
        }
      return cemitstate.mo_cemsta_errstr;
    }
  else
    {
      cemp->mo_cemit_locstate = &cemitstate;
      mom_cemit_open (&cemitstate);
      mom_cemit_includes (&cemitstate);
      mom_cemit_ctypes (&cemitstate);
      mom_cemit_declarations (&cemitstate);
      mom_cemit_set (&cemitstate);
      mom_cemit_data_definitions (&cemitstate);
      mom_cemit_function_definitions (&cemitstate);
      mom_cemit_do_at_end (&cemitstate);
#warning mo_objref_cemit_generate very incomplete
      MOM_WARNPRINTF ("cemit_generate incomplete for %s",
                      cemitstate.mo_cemsta_modid);
      mom_cemit_close (&cemitstate);
      cemp->mo_cemit_locstate = NULL;
      MOM_INFORMPRINTF ("C code generated for module %s",
                        mo_objref_pnamestr (cemp->mo_cemit_modobj));
    }
  return NULL;
}                               /* end of mo_objref_cemit_generate */


// momglob_put_attr_cemitact is needed

const char
MOM_PREFIXID (mosig_, put_attr_cemitact)[] = "signature_two_objects_to_void";

     extern mo_signature_two_objects_to_void_sigt
       MOM_PREFIXID (mofun_, put_attr_cemitact)
  __attribute__ ((optimize ("O2")));


     extern mo_signature_two_objects_to_void_sigt mofun_put_attr_cemitact;

     void
       MOM_PREFIXID (mofun_, put_attr_cemitact) (mo_objref_t todobj,
                                                 mo_objref_t cemitobj)
{
  mofun_put_attr_cemitact (todobj, cemitobj);
}

enum momcemit_putattrix_en
{
  MOTODIX_PUTATTR_ROUTINE,
  MOTODIX_PUTATTR_OBJECT,
  MOTODIX_PUTATTR_ATTR,
  MOTODIX_PUTATTR_VAL,
  MOTODIX_PUTATTR__LAST
};

void
mofun_put_attr_cemitact (mo_objref_t todobj, mo_objref_t cemitobj)
{
  if (!mo_dyncast_objref (cemitobj))
    MOM_FATAPRINTF ("put_attr_cemitact: no cemitobj for todobj=%s",
                    mo_objref_pnamestr (todobj));
  if (cemitobj->mo_ob_paylkind != MOM_PREDEF (payload_c_emit))
    MOM_FATAPRINTF
      ("put_attr_cemitact: bad cemitobj=%s (of payload kind %s) for todobj=%s",
       mo_objref_pnamestr (cemitobj),
       mo_objref_pnamestr (cemitobj->mo_ob_paylkind),
       mo_objref_pnamestr (todobj));
  mo_cemitpayl_ty *cemp = mo_objref_get_cemit (cemitobj);
  MOM_ASSERTPRINTF (cemp && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC,
                    "put_attr_cemitact: bad cemp in cemitobj %s",
                    mo_objref_pnamestr (cemitobj));
  struct mom_cemitlocalstate_st *csta = cemp->mo_cemit_locstate;
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil != NULL,
                    "put_attr_cemitact: bad csta@%p", csta);
  if (!mo_dyncast_objref (todobj))
    MOM_CEMITFAILURE (csta, "put_attr_cemitact: bad todobj");
  if (mo_objref_comp_count (todobj) != MOTODIX_PUTATTR__LAST)
    MOM_CEMITFAILURE (csta, "put_attr_cemitact: bad count %d of todobj %s",
                      mo_objref_comp_count (todobj),
                      mo_objref_pnamestr (todobj));
  MOM_ASSERTPRINTF (mo_objref_get_comp (todobj, MOTODIX_PUTATTR_ROUTINE) ==
                    momglob_put_attr_cemitact, "bad routine in todobj %s",
                    mo_objref_pnamestr (todobj));
  mo_objref_t obr =
    mo_dyncast_objref (mo_objref_get_comp (todobj, MOTODIX_PUTATTR_OBJECT));
  if (!obr)
    MOM_CEMITFAILURE (csta,
                      "put_attr_cemitact: bad object slot#%d in todobj %s",
                      MOTODIX_PUTATTR_OBJECT, mo_objref_pnamestr (todobj));
  mo_objref_t attrobr =
    mo_dyncast_objref (mo_objref_get_comp (todobj, MOTODIX_PUTATTR_ATTR));
  if (!attrobr)
    MOM_CEMITFAILURE (csta,
                      "put_attr_cemitact: bad attr slot#%d in todobj %s",
                      MOTODIX_PUTATTR_ATTR, mo_objref_pnamestr (todobj));
  mo_value_t val = mo_objref_get_comp (todobj, MOTODIX_PUTATTR_VAL);
  mo_objref_put_attr (obr, attrobr, val);
}                               /* end of mofun_put_attr_cemitact */



void
mom_cemit_todo_put_attr (struct mom_cemitlocalstate_st *csta,
                         mo_objref_t obj, mo_objref_t attrobr, mo_value_t val)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil != NULL,
                    "cemit_todo_put_attr: bad csta@%p", csta);
  mo_cemitpayl_ty *cemp = csta->mo_cemsta_payl;
  MOM_ASSERTPRINTF (cemp && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC
                    && cemp->mo_cemit_locstate == csta,
                    "cemit_todo_put_attr: bad payl@%p in csta@%p", cemp,
                    csta);
  if (!mo_dyncast_objref (obj))
    MOM_CEMITFAILURE (csta, "cemit_todo_put_attr: bad obj");
  if (!mo_dyncast_objref (attrobr))
    MOM_CEMITFAILURE (csta, "cemit_todo_put_attr: for obj %s bad attrobr",
                      mo_objref_pnamestr (obj));
  mo_objref_t todobj = mo_make_object ();
  mo_objref_comp_resize (todobj, MOTODIX_PUTATTR__LAST);
  mo_objref_put_comp (todobj, MOTODIX_PUTATTR_ROUTINE,
                      momglob_put_attr_cemitact);
  mo_objref_put_comp (todobj, MOTODIX_PUTATTR_OBJECT, obj);
  mo_objref_put_comp (todobj, MOTODIX_PUTATTR_ATTR, attrobr);
  mo_objref_put_comp (todobj, MOTODIX_PUTATTR_VAL, val);
  mom_cemit_todo_last_at_end (csta, todobj);
}                               /* end of mom_cemit_todo_put_attr */

/*** end of file cemit.c ***/
