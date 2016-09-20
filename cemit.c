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

#define MOM_CEMIT_MAGIC 0x56c59cc3      /*1455791299 cemit_magic */
#define MOMFIELDS_cemitpayl					\
  MOMFIELDS_countedpayl;					\
  uint32_t mo_cemit_nmagic; /* always MOM_CEMIT_MAGIC */	\
  /* the suffix is often '.c' */				\
  char mo_cemit_suffix[4];					\
/* the prefix could be 'modules.dir/momg_' */			\
  char mo_cemit_prefix[32];					\
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
  mo_hashsetpayl_ty *mo_cemsta_hsetctypes;      /* the hashset of tpyes */
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
  strcpy (cemp->mo_cemit_prefix, MOM_MODULES_DIR "/" MOM_MODULE_INFIX);
  cemp->mo_cemit_prefix[sizeof (cemp->mo_cemit_prefix) - 1] = 0;
  cemp->mo_cemit_locstate = NULL;
  cemp->mo_cemit_modobj = NULL;
  MOM_FATAPRINTF ("put_cemit_payload uncomplete cemp@%p", cemp);
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
     "cemit failure: %s",				\
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
            "+r%x_%x_p%d", (int) (momrand_genrand_int31 () & 0x3ffffff) + 1,
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
  char *backuppathbuf = NULL;
  asprintf (&oldpathbuf, "%s%s%s%%",
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
      if (rename (oldpathbuf, newpathbuf))
        MOM_FATAPRINTF ("cemit_close: rename %s -> %s failure", oldpathbuf,
                        newpathbuf);
      fclose (newfil);
      return;
    }
  struct stat newstat = { };
  struct stat oldstat = { };
  if (fstat (fileno (oldfil), &oldstat))
    MOM_FATAPRINTF ("cemit_close: fstat old #%d %s failed", fileno (oldfil),
                    oldpathbuf);
  if (fstat (fileno (newfil), &newstat))
    MOM_FATAPRINTF ("cemit_close: fstat new #%d %s failed", fileno (newfil),
                    newpathbuf);
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
        MOM_FATAPRINTF ("cemit_close: new rename %s -> %s failed", newpathbuf,
                        oldpathbuf);
    };
  /// add a symbolic link for named modules
  mo_value_t namodv = mo_objref_namev (cemp->mo_cemit_modobj);
  if (namodv != NULL)
    {
      MOM_ASSERTPRINTF (mo_dyncast_string (namodv), "bad namodv for %s",
                        mo_objref_pnamestr (cemp->mo_cemit_modobj));
      char *oldbase = basename (oldpathbuf);
      MOM_ASSERTPRINTF (oldbase != NULL
                        && oldbase >= oldpathbuf, "bad oldbase");
      char *oldbidp = strstr (oldbase, csta->mo_cemsta_modid);
      MOM_ASSERTPRINTF (oldbidp != NULL
                        && strlen (oldbidp) >= MOM_CSTRIDLEN, "bad oldbidp");
      char *symlpathbuf = NULL;
      asprintf (&symlpathbuf, "%*s%s%s", (int) (oldbidp - oldpathbuf),
                oldpathbuf, mo_string_cstr (namodv), oldbidp + MOM_CSTRIDLEN);
      if (MOM_UNLIKELY (symlpathbuf == NULL))
        MOM_FATAPRINTF ("cemit_close: asprintf symlpathbuf failed for %s",
                        mo_objref_pnamestr (cemp->mo_cemit_modobj));
      errno = 0;
      if (access (symlpathbuf, R_OK) && errno == ENOENT)
        {
          if (symlink (oldbase, symlpathbuf))
            MOM_FATAPRINTF
              ("failed to symlink: %s -> %s for named module %s (%s)",
               symlpathbuf, oldbase, mo_string_cstr (namodv),
               csta->mo_cemsta_modid);
        }
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
        if (!isalnum (*pi) || *pi == '/' || *pi == '_' || *pi == '+'
            || *pi == '-' || *pi == '.')
          MOM_CEMITFAILURE (csta, "included %s with invalid `file_path` %s",
                            mo_objref_pnamestr (curinclob), inclcstr);
      mom_cemit_printf (csta, "#include \"%s\"\n", inclcstr);
    }
  fputs ("\n\n", csta->mo_cemsta_fil);
  fflush (csta->mo_cemsta_fil);
}                               /* end mom_cemit_includes */

void
mom_cemit_write_ctype (struct mom_cemitlocalstate_st *csta,
                       mo_objref_t typobr)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil != NULL,
                    "cemit_write_ctype: bad csta@%p", csta);
  mo_cemitpayl_ty *cemp = csta->mo_cemsta_payl;
  MOM_ASSERTPRINTF (cemp && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC
                    && cemp->mo_cemit_locstate == csta,
                    "cemit_write_ctype: bad payl@%p in csta@%p", cemp, csta);
  MOM_ASSERTPRINTF (mo_dyncast_objref (typobr),
                    "cemit_write_ctype: bad typobr");
  if (mo_objref_space (typobr) == mo_SPACE_PREDEF)
    {
#define MOM_NBCASE_CTYPE 59
#define CASE_CTYPE_MOM(Ob) momphash_##Ob % MOM_NBCASE_CTYPE:	\
  if (typobr != MOM_PREDEF(Ob))					\
    goto defaultctypecase;					\
  goto labctype_##Ob;						\
  labctype_##Ob
      switch (mo_objref_hash (typobr) % MOM_NBCASE_CTYPE)
        {
          case CASE_CTYPE_MOM (int):    //
            mom_cemit_printf (csta, "int ");
          return;
          case CASE_CTYPE_MOM (void):   //
            mom_cemit_printf (csta, "void ");
          return;
        case CASE_CTYPE_MOM (bool):    //
          mom_cemit_printf (csta, "bool ");
          return;
          case CASE_CTYPE_MOM (char):   //
            mom_cemit_printf (csta, "char ");
          return;
          case CASE_CTYPE_MOM (long):   //
            mom_cemit_printf (csta, "long ");
          return;
          case CASE_CTYPE_MOM (double): //
            mom_cemit_printf (csta, "double ");
          return;
        default:
        defaultctypecase:
          break;
        }
#undef MOM_NBCASE_CTYPE
#undef CASE_CTYPE_MOM
    }
  if (!mo_hashset_contains (csta->mo_cemsta_hsetctypes, typobr))
    MOM_CEMITFAILURE (csta, "write_ctype: typobr %s unknown",
                      mo_objref_pnamestr (typobr));
  char typobid[MOM_CSTRIDSIZ];
  memset (typobid, 0, sizeof (typobid));
  mo_objref_idstr (typobid, typobr);
  mo_value_t typobnamv = mo_objref_namev (typobr);
  if (typobnamv)
    mom_cemit_printf (csta, "mo_%s_ty ", mo_string_cstr (typobnamv));
  else
    mom_cemit_printf (csta, "mo%s_ty ", typobid);
}                               /* end mom_cemit_write_ctype */



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
#define CASE_CTYPE_MOM(Ob) momphash_##Ob % MOM_NBCASE_CTYPE:	\
  if (typobr->mo_ob_class != MOM_PREDEF(Ob))			\
    goto defaultctypecase;					\
  goto labctype_##Ob;						\
  labctype_##Ob
  switch (mo_objref_hash (typobr->mo_ob_class) % MOM_NBCASE_CTYPE)
    {
    case CASE_CTYPE_MOM (basic_ctype_class):
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
    case CASE_CTYPE_MOM (struct_pointer_ctype_class):
      mom_cemit_printf (csta, "typedef struct mo%s_ptrst* mo%s_ty;\n",
                        typobid, typobid);
      break;
    case CASE_CTYPE_MOM (struct_ctype_class):
      mom_cemit_printf (csta, "typedef struct mo%s_st mo%s_ty;\n",
                        typobid, typobid);
      break;
    case CASE_CTYPE_MOM (union_ctype_class):
      mom_cemit_printf (csta, "typedef union mo%s_un mo_%s_ty;\n",
                        typobid, typobid);
      break;
    case CASE_CTYPE_MOM (enum_ctype_class):
      mom_cemit_printf (csta, "typedef enum mo%s_en mo_%s_ty;\n",
                        typobid, typobid);
      break;
    case CASE_CTYPE_MOM (signature_class):
      {
        mo_value_t formtytup =
          mo_dyncast_tuple (mo_objref_get_attr
                            (typobr, MOM_PREDEF (formals_ctypes)));
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
        mom_cemit_printf (csta, "typedef ");
        mom_cemit_write_ctype (csta, restypobr);
        mom_cemit_printf (csta, " mo%s_ty (", typobid);
        for (unsigned foix = 0; foix < nbformals; foix++)
          {
            if (foix > 0)
              fputs (", ", csta->mo_cemsta_fil);
            mo_objref_t curformobr = mo_tuple_nth (formtytup, foix);
            mom_cemit_write_ctype (csta, curformobr);
          };
        mom_cemit_printf (csta, ");\n");
      }
      break;
    default:
    defaultctypecase:
      MOM_CEMITFAILURE (csta,
                        "cemit_declare_ctype: typobr %s has bad class %s",
                        mo_objref_pnamestr (typobr),
                        mo_objref_pnamestr (typobr->mo_ob_class));

      break;
    }
#undef MOM_NBCASE_CTYPE
#undef CASE_CTYPE_MOM
  mo_value_t typobnamv = mo_objref_namev (typobr);
  if (typobnamv)
    mom_cemit_printf (csta, "typedef mo%s_ty mo_%s_ty\n",
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

    }
}                               /* end mom_cemit_define_fields */


void
mom_cemit_define_enumerators (struct mom_cemitlocalstate_st *csta,
                              mo_objref_t typobr, mo_objref_t parobr,
                              int depth)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil != NULL,
                    "cemit_define_enumerators: bad csta@%p", csta);
  mo_cemitpayl_ty *cemp = csta->mo_cemsta_payl;
  MOM_ASSERTPRINTF (cemp && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC
                    && cemp->mo_cemit_locstate == csta,
                    "cemit_define_enumerators: bad payl@%p in csta@%p", cemp,
                    csta);
  MOM_ASSERTPRINTF (mo_dyncast_objref (typobr),
                    "cemit_define_enumerators: bad typobr");
  MOM_ASSERTPRINTF (mo_dyncast_objref (parobr),
                    "cemit_define_enumerators: bad parobr");
  if (depth > MOM_CEMIT_MAX_DEPTH)
    MOM_CEMITFAILURE (csta, "cemit_define_enumerators: %s too deep %d",
                      mo_objref_pnamestr (typobr), depth);
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
#define CASE_CTYPE_MOM(Ob) momphash_##Ob % MOM_NBCASE_CTYPE:	\
  if (typobr->mo_ob_class != MOM_PREDEF(Ob))			\
    goto defaultctypecase;					\
  goto labctype_##Ob;						\
  labctype_##Ob
  switch (mo_objref_hash (typobr->mo_ob_class) % MOM_NBCASE_CTYPE)
    {
    case CASE_CTYPE_MOM (basic_ctype_class):
      break;
    case CASE_CTYPE_MOM (struct_pointer_ctype_class):
      mom_cemit_printf (csta, "struct mo%s_ptrst {", typobid);
      if (typnamv)
        mom_cemit_printf (csta, "// %s\n", mo_string_cstr (typnamv));
      else
        fputc ('\n', csta->mo_cemsta_fil);
      mom_cemit_define_fields (csta, typobr, false, 0);
      mom_cemit_printf (csta, "}; // end struct mo%s_ptrst\n", typobid);
      break;
    case CASE_CTYPE_MOM (struct_ctype_class):
      mom_cemit_printf (csta, "struct mo%s_st mo%s_ty {", typobid, typobid);
      if (typnamv)
        mom_cemit_printf (csta, "// %s\n", mo_string_cstr (typnamv));
      else
        fputc ('\n', csta->mo_cemsta_fil);
      mom_cemit_define_fields (csta, typobr, false, 0);
      mom_cemit_printf (csta, "}; // end struct mo%s_st\n", typobid);
      break;
    case CASE_CTYPE_MOM (union_ctype_class):
      mom_cemit_printf (csta, "union mo%s_un {", typobid);
      if (typnamv)
        mom_cemit_printf (csta, "// %s\n", mo_string_cstr (typnamv));
      else
        fputc ('\n', csta->mo_cemsta_fil);
      mom_cemit_define_fields (csta, typobr, true, 0);
      mom_cemit_printf (csta, "}; // end union mo%s_un\n", typobid);
      break;
    case CASE_CTYPE_MOM (enum_ctype_class):
      mom_cemit_printf (csta, "enum mo%s_en {", typobid);
      if (typnamv)
        mom_cemit_printf (csta, "// %s\n", mo_string_cstr (typnamv));
      else
        fputc ('\n', csta->mo_cemsta_fil);
      mom_cemit_define_enumerators (csta, typobr, typobr, 0);
      mom_cemit_printf (csta, "}; // end enum mo%s_en\n", typobid);
      break;
    case CASE_CTYPE_MOM (signature_class):
      break;
    default:
    defaultctypecase:
      MOM_CEMITFAILURE (csta,
                        "cemit_define_ctype: typobr %s has bad class %s",
                        mo_objref_pnamestr (typobr),
                        mo_objref_pnamestr (typobr->mo_ob_class));
      break;
    }
#undef MOM_NBCASE_CTYPE
#undef CASE_CTYPE_MOM
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
  mom_cemit_printf (csta, "\n// %d types definitions\n", nbctyp);
  // first loop to emit typedefs
  for (unsigned tix = 0; tix < nbctyp; tix++)
    {
      mo_objref_t ctypob = mo_tuple_nth (ctypv, tix);
      MOM_ASSERTPRINTF (mo_dyncast_objref (ctypob), "bad ctypob tix#%d", tix);
      mom_cemit_declare_ctype (csta, ctypob);
    }
  // second loop to emit enum, struct, union-s.... i.e. aggregate type definitions
  for (unsigned tix = 0; tix < nbctyp; tix++)
    {
      mo_objref_t ctypob = mo_tuple_nth (ctypv, tix);
      MOM_ASSERTPRINTF (mo_dyncast_objref (ctypob), "bad ctypob tix#%d", tix);
      mom_cemit_define_ctype (csta, ctypob);
    }
}                               /* end of mom_cemit_ctypes */


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
    };
  cemp->mo_cemit_locstate = &cemitstate;
  mom_cemit_open (&cemitstate);
  mom_cemit_includes (&cemitstate);
  mom_cemit_ctypes (&cemitstate);
#warning mo_objref_cemit_generate very incomplete
  MOM_WARNPRINTF ("cemit_close incomplete for %s",
                  cemitstate.mo_cemsta_modid);
  mom_cemit_close (&cemitstate);
  cemp->mo_cemit_locstate = NULL;
  MOM_INFORMPRINTF ("C code generated for module %s",
                    mo_objref_pnamestr (cemp->mo_cemit_modobj));
  return NULL;
}                               /* end of mo_objref_cemit_generate */

/*** end of file cemit.c ***/
