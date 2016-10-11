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

#define MOM_FORMAL_PREFIX "mofor_"
#define MOM_RESULT_PREFIX "mores_"
#define MOM_LOCAL_PREFIX "moloc_"
#define MOM_LABEL_PREFIX "molab_"

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
  double mo_cemsta_timelimit;   /* elapsed time limit */
  FILE *mo_cemsta_fil;          /* the emitted FILE handle */
  mo_cemitpayl_ty *mo_cemsta_payl;      /* the payload */
  mo_value_t mo_cemsta_objset;  /* the set of objects */
  mo_hashsetpayl_ty *mo_cemsta_hsetctypes;      /* the hashset of declared c_type-s - and signatures */
  mo_hashsetpayl_ty *mo_cemsta_hsetincludes;    /* the hashset of includes */
  // each global object, routine, ... has also a global role
  mo_assovaldatapayl_ty *mo_cemsta_assocmodulrole;
  mo_objref_t mo_cemsta_curfun; /* the current function */
  // each variable, expression, instruction, block has some role describing more of it
  mo_assovaldatapayl_ty *mo_cemsta_assoclocalrole;      /* associate objects to some role in the current function */
  mo_hashsetpayl_ty *mo_cemsta_hsetjumpedblocks;        /* the hashset of jumped-into blocks */
  mo_listpayl_ty *mo_cemsta_endtodolist;        /* list of things to do at end */
  mo_value_t mo_cemsta_errstr;  /* the error string */
  jmp_buf mo_cemsta_jmpbuf;     /* for errors */
};

// the slots of role object for formals
enum momcemit_rolformal_en
{
  MOMROLFORMIX_ROLE,
  MOMROLFORMIX_CTYPE,
  MOMROLFORMIX_FORMALRANK,
  MOMROLFORMIX__LASTFORMAL
};

// the slots of role object for result & variables & gobal
enum momcemit_rolvar_en
{
  MOMROLVARIX_ROLE = MOMROLFORMIX_ROLE,
  MOMROLVARIX_CTYPE = MOMROLFORMIX_CTYPE,
  MOMROLVARIX_IN,
  MOMROLVARIX__LASTVAR
};

// the slots of role object for blocks (& macros)
enum momcemit_rolblock_en
{
  MOMROLBLOCKIX_ROLE = MOMROLFORMIX_ROLE,
  /// either INSTRS for blocks or EXPANSION for macros but not both
  MOMROLBLOCKIX_INSTRS = MOMROLFORMIX_CTYPE,    // for blocks
  MOMROLBLOCKIX_EXPANSION = MOMROLBLOCKIX_INSTRS,       // for macros
  MOMROLBLOCKIX_IN,
  MOMROLBLOCKIX_LOCALS,
  MOMROLBLOCKIX__LASTVAR
};

// the slots of role object for inlined or global functions
enum momcemit_rolfunc_en
{
  MOMROLFUNCIX_ROLE = MOMROLFORMIX_ROLE,
  MOMROLFUNCIX_SIGNATURE = MOMROLFORMIX_CTYPE,
  MOMROLFUNCIX__LASTFUNC
};
// the slots of role object for enumerator values
enum momcemit_rolenumval_en
{
  MOMROLENUMVIX_ROLE = MOMROLFORMIX_ROLE,
  MOMROLENUMVIX_ENUMTYPE = MOMROLFORMIX_CTYPE,
  MOMROLENUMVIX_ENUMVAL,
  MOMROLENUMVIX__LASTENUMV
};

// the slots of role object for assignment instructions
enum momcemit_rolassigninstr_en
{
  MOMROLASSIGNIX_ROLE = MOMROLFORMIX_ROLE,      // MOM_PREDEF(assign)
  MOMROLASSIGNIX_TOTYPE,
  MOMROLASSIGNIX_TOREFERENCE,
  MOMROLASSIGNIX_FROMTYPE,
  MOMROLASSIGNIX_FROMEXPR,
  MOMROLASSIGNIX__LASTASSIGN
};
// the slots of role object for chunk instructions
enum momcemit_rolchunkinstr_en
{
  MOMROLCHUNKIX_ROLE = MOMROLFORMIX_ROLE,       // MOM_PREDEF(chunk)
  MOMROLCHUNKIX_VERBATIM,
  MOMROLCHUNKIX_REFERENCE,
  MOMROLCHUNKIX_EXPRESSION,
  MOMROLCHUNKIX_BLOCK,
  MOMROLCHUNKIX_LABEL,
  MOMROLCHUNKIX__LASTCHUNK
};

// the slots of role object for jump instructions
enum momcemit_roljumpinstr_en
{
  MOMROLJUMPIX_ROLE = MOMROLFORMIX_ROLE,        // MOM_PREDEF(to)
  MOMROLJUMPIX_TO,              // target block
  MOMROLJUMPIX__LAST
};

// the slots of role object for cond instructions
enum momcemit_rolcondinstr_en
{
  MOMROLCONDIX_ROLE = MOMROLFORMIX_ROLE,        // MOM_PREDEF(conditional)
  MOMROLCONDIX_CONDITIONS,      // tuple of conditions
  MOMROLCONDIX__LAST
};
// the slots of role object for conditions
enum momcemit_rolcondition_en
{
  MOMROLCONDITIONIX_ROLE = MOMROLFORMIX_ROLE,   // MOM_PREDEF(when)
  MOMROLCONDITIONIX_WHEN,
  MOMROLCONDITIONIX_BODY,
  MOMROLCONDITIONIX__LAST
};


// the slots of role object for call instructions
enum momcemit_rolcallinstr_en
{
  MOMROLCALLIX_ROLE = MOMROLFORMIX_ROLE,        // MOM_PREDEF(call)
  MOMROLCALLIX_CALL,
  MOMROLCALLIX_SIGNATURE,
  MOMROLCALLIX__LAST
};

// the slots of role object for object case
enum momcemit_rolobjcase_en
{
  MOMROLOBJCASIX_ROLE = MOMROLFORMIX_ROLE,      // MOM_PREDEF(object)
  MOMROLOBJCASIX_INSTR,         /* the containing case instr */
  MOMROLOBJCASIX_OBJECT,        /* the object or set */
  MOMROLOBJCASIX_BLOCK,
  MOMROLOBJCASIX__LAST
};

// the slots of role object for number case
enum momcemit_rolnumcase_en
{
  MOMROLNUMCASIX_ROLE = MOMROLFORMIX_ROLE,      // MOM_PREDEF(int)
  MOMROLNUMCASIX_INSTR,         /* the containing case instr */
  MOMROLNUMCASIX_NUMBER,        /* the object or set */
  MOMROLNUMCASIX_BLOCK,
  MOMROLNUMCASIX__LAST
};

// the slots of role object for case instructions
enum momcemit_rolcaseinstr_en
{
  MOMROLCASEIX_ROLE = MOMROLFORMIX_ROLE,        // MOM_PREDEF(case)
  MOMROLCASEIX_CASE,            // the selecting expr
  MOMROLCASEIX_CTYPE,           // its ctype
  MOMROLCASEIX_MAP,             /* the object with inthmap or assoval of cases */
  MOMROLCASEIX_BLOCKSET,        /* the object with hashset of blocks */
  MOMROLCASEIX__LAST
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


// practically useful with MOM_CEMITFAILURE to transmit & keep (for
// GC) some values related to the failure...
struct mom_cemitlocalstate_st *
mom_cemit_add_data (struct mom_cemitlocalstate_st *csta, ...)
__attribute ((sentinel));
#define MOM_CEMIT_ADD_DATA(Csta,...) \
  mom_cemit_add_data((Csta), ##__VA_ARGS__, NULL)


     struct mom_cemitlocalstate_st *mom_cemit_add_data (struct
                                                        mom_cemitlocalstate_st
                                                        *csta, ...)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC,
                    "cemit_add_data: bad csta@%p", csta);
  mo_cemitpayl_ty *cemp = csta->mo_cemsta_payl;
  MOM_ASSERTPRINTF (cemp && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC
                    && cemp->mo_cemit_locstate == csta,
                    "cemit_add_data: bad payl@%p in csta@%p", cemp, csta);
  mo_objref_t cemitobr = csta->mo_cemsta_objcemit;
  MOM_ASSERTPRINTF (mo_dyncast_objref (cemitobr),
                    "cemit_add_data: bad cemitobr");
  va_list args;
  va_start (args, csta);
  mo_value_t curval = NULL;
  do
    {
      curval = va_arg (args, mo_value_t);
      if (curval == MOM_EMPTY_SLOT)
        {
          mo_objref_comp_append (cemitobr, NULL);
          continue;
        }
      else if (mo_kind_of_value (curval) != mo_KNONE)
        mo_objref_comp_append (cemitobr, curval);
    }
  while (curval != NULL);
  va_end (args);
  return csta;
}                               /* end of mom_cemit_add_data */

bool
mom_cemit_ctype_is_scalar (struct mom_cemitlocalstate_st * csta,
                           mo_objref_t ctypob)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC,
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
      case CASE_GLOBALCTYPE_MOM (int64_t):      // momglob_int64_t
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
  if (mom_elapsed_real_time () > csta->mo_cemsta_timelimit)
    MOM_CEMITFAILURE (csta,
                      "cemit_write_ctype_for: %s timed out, depth %d, for %s",
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
  if (mom_elapsed_real_time () > csta->mo_cemsta_timelimit)
    MOM_CEMITFAILURE (csta, "cemit_define_fields: %s timed out, depth %d",
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
      if (depth == 0)
        mom_cemit_todo_put_attr (csta, fieldobr, MOM_PREDEF (in), typobr);
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
  if (mom_elapsed_real_time () > csta->mo_cemsta_timelimit)
    MOM_CEMITFAILURE (csta,
                      "cemit_define_enumerators: %s timed out, depth %d",
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
  csta->mo_cemsta_assocmodulrole =
    mo_assoval_reserve (csta->mo_cemsta_assocmodulrole, 4 * nbenums / 3 + 1);
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
          mo_objref_t rolob =
            mo_dyncast_objref (mo_assoval_get
                               (csta->mo_cemsta_assocmodulrole, curenumobr));
          if (rolob)
            MOM_CEMITFAILURE (csta,
                              "cemit_define_enumerators: %s with enumerator#%d %s already with role %s",
                              mo_objref_pnamestr (typobr), eix,
                              mo_objref_pnamestr (curenumobr),
                              mo_objref_pnamestr (rolob));
          rolob = mo_make_object ();
          rolob->mo_ob_class = MOM_PREDEF (c_role_class);
          mo_objref_comp_resize (rolob, MOMROLENUMVIX__LASTENUMV);
          mo_objref_put_comp (rolob, MOMROLENUMVIX_ROLE,
                              MOM_PREDEF (enumerators));
          mo_objref_put_comp (rolob, MOMROLENUMVIX_ENUMTYPE, typobr);
          mo_objref_put_comp (rolob, MOMROLENUMVIX_ENUMVAL,
                              mo_int_to_value (curval));
          csta->mo_cemsta_assocmodulrole =
            mo_assoval_put (csta->mo_cemsta_assocmodulrole, curenumobr,
                            rolob);
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
          mom_cemit_todo_put_attr (csta, curenumobr, MOM_PREDEF (in), typobr);
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
      if (mom_elapsed_real_time () > csta->mo_cemsta_timelimit)
        MOM_CEMITFAILURE (csta,
                          "cemit_ctypes: %s declaration timed out, index %d",
                          mo_objref_pnamestr (ctypob), tix);
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
      if (mom_elapsed_real_time () > csta->mo_cemsta_timelimit)
        MOM_CEMITFAILURE (csta,
                          "cemit_ctypes: %s definition timed out, index %d",
                          mo_objref_pnamestr (ctypob), tix);
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
        mo_objref_t rolob =
          mo_dyncast_objref (mo_assoval_get
                             (csta->mo_cemsta_assocmodulrole, ob));
        if (rolob)
          MOM_CEMITFAILURE (csta,
                            "cemit_object_declare: inlined function object %s has already role %s in module",
                            mo_objref_pnamestr (ob),
                            mo_objref_pnamestr (rolob));
        rolob = mo_make_object ();
        rolob->mo_ob_class = MOM_PREDEF (c_role_class);
        mo_objref_comp_resize (rolob, MOMROLFUNCIX__LASTFUNC);
        mo_objref_put_comp (rolob, MOMROLFUNCIX_ROLE, MOM_PREDEF (code));
        mo_objref_put_comp (rolob, MOMROLFUNCIX_SIGNATURE, signob);
        csta->mo_cemsta_assocmodulrole =
          mo_assoval_put (csta->mo_cemsta_assocmodulrole, ob, rolob);
        mom_cemit_printf (csta,
                          "static inline mo_%s_ty " MOM_FUNC_PREFIX "%s;\n",
                          mo_objref_shortnamestr (signob), shortnamob);
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
        mo_objref_t rolob =
          mo_dyncast_objref (mo_assoval_get
                             (csta->mo_cemsta_assocmodulrole, ob));
        if (rolob)
          MOM_CEMITFAILURE (csta,
                            "cemit_object_declare: global function object %s has already role %s in module",
                            mo_objref_pnamestr (ob),
                            mo_objref_pnamestr (rolob));
        rolob = mo_make_object ();
        rolob->mo_ob_class = MOM_PREDEF (c_role_class);
        mo_objref_comp_resize (rolob, MOMROLFUNCIX__LASTFUNC);
        mo_objref_put_comp (rolob, MOMROLFUNCIX_ROLE, MOM_PREDEF (code));
        mo_objref_put_comp (rolob, MOMROLFUNCIX_SIGNATURE, signob);
        csta->mo_cemsta_assocmodulrole =
          mo_assoval_put (csta->mo_cemsta_assocmodulrole, ob, rolob);
        if (obnamev)
          mom_cemit_printf (csta,
                            "extern /*idroutine*/ mo_%s_ty " MOM_FUNC_PREFIX
                            "%s;\n", mo_objref_shortnamestr (signob),
                            obid + 1);
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
        mo_objref_t rolob =
          mo_dyncast_objref (mo_assoval_get
                             (csta->mo_cemsta_assocmodulrole, ob));
        if (rolob)
          MOM_CEMITFAILURE (csta,
                            "cemit_object_declare: global data object %s has already role %s in module",
                            mo_objref_pnamestr (ob),
                            mo_objref_pnamestr (rolob));
        rolob = mo_make_object ();
        rolob->mo_ob_class = MOM_PREDEF (c_role_class);
        mo_objref_comp_resize (rolob, MOMROLVARIX__LASTVAR);
        mo_objref_put_comp (rolob, MOMROLVARIX_ROLE, MOM_PREDEF (data));
        mo_objref_put_comp (rolob, MOMROLVARIX_CTYPE, ctypob);
        csta->mo_cemsta_assocmodulrole =
          mo_assoval_put (csta->mo_cemsta_assocmodulrole, ob, rolob);
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
        mo_objref_t rolob =
          mo_dyncast_objref (mo_assoval_get
                             (csta->mo_cemsta_assocmodulrole, ob));
        if (rolob)
          MOM_CEMITFAILURE (csta,
                            "cemit_object_declare: global data object %s has already role %s in module",
                            mo_objref_pnamestr (ob),
                            mo_objref_pnamestr (rolob));
        rolob = mo_make_object ();
        rolob->mo_ob_class = MOM_PREDEF (c_role_class);
        mo_objref_comp_resize (rolob, MOMROLVARIX__LASTVAR);
        mo_objref_put_comp (rolob, MOMROLVARIX_ROLE, MOM_PREDEF (data));
        mo_objref_put_comp (rolob, MOMROLVARIX_CTYPE, ctypob);
        csta->mo_cemsta_assocmodulrole =
          mo_assoval_put (csta->mo_cemsta_assocmodulrole, ob, rolob);
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
  csta->mo_cemsta_assocmodulrole =
    mo_assoval_reserve (NULL, 10 + 3 * (nbextern + nbcode + nbdata) / 2);
  //
  if (nbextern > 0)
    {
      mom_cemit_printf (csta, "\n// %d extern declarations:\n", nbextern);
      for (unsigned ix = 0; ix < nbextern; ix++)
        {
          if (mom_elapsed_real_time () > csta->mo_cemsta_timelimit)
            MOM_CEMITFAILURE (csta,
                              "cemit_declarations: extern %s timed out, index %d",
                              mo_objref_pnamestr (mo_sequence_nth
                                                  (externv, ix)), ix);
          mom_cemit_object_declare (csta, mo_sequence_nth (externv, ix),
                                    true);
        }
    }
  else
    mom_cemit_printf (csta, "\n// no extern declarations\n");
  //
  if (nbdata > 0)
    {
      mom_cemit_printf (csta, "\n// %d data declarations:\n", nbdata);
      for (unsigned ix = 0; ix < nbdata; ix++)
        {
          if (mom_elapsed_real_time () > csta->mo_cemsta_timelimit)
            MOM_CEMITFAILURE (csta,
                              "cemit_declarations: data %s timed out, index %d",
                              mo_objref_pnamestr (mo_sequence_nth
                                                  (datav, ix)), ix);
          mom_cemit_object_declare (csta, mo_sequence_nth (datav, ix), true);
        }
    }
  else
    mom_cemit_printf (csta, "\n// no data declarations\n");
  //
  if (nbcode > 0)
    {
      mom_cemit_printf (csta, "\n// %d code declarations:\n", nbcode);
      for (unsigned ix = 0; ix < nbcode; ix++)
        {
          if (mom_elapsed_real_time () > csta->mo_cemsta_timelimit)
            MOM_CEMITFAILURE (csta,
                              "cemit_declarations: code %s timed out, index %d",
                              mo_objref_pnamestr (mo_sequence_nth
                                                  (codev, ix)), ix);
          mom_cemit_object_declare (csta, mo_sequence_nth (codev, ix), false);
        }
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
      if (mom_elapsed_real_time () > csta->mo_cemsta_timelimit)
        MOM_CEMITFAILURE (csta,
                          "cemit_data_definitions: %s timed out, index %d",
                          mo_objref_pnamestr (curdob), dix);
      char curobid[MOM_CSTRIDSIZ];
      memset (curobid, 0, sizeof (curobid));
      mo_objref_idstr (curobid, curdob);
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

// A block may contains locals and has a sequence of instructions
// (some of which may be sub-blocks); it translates in C to a block in
// braces {....}
void
mom_cemit_scan_block (struct mom_cemitlocalstate_st *csta,
                      mo_objref_t blockob, mo_objref_t fromob, int depth);

// An instruction may contain references (i.e. l-values in C) and
// expressions (i.e. r-values in C) or is some block; it translates in
// C to a statement
void
mom_cemit_scan_instr (struct mom_cemitlocalstate_st *csta,
                      mo_objref_t instrob, mo_objref_t fromob, int depth);

// a reference or l-value can be assigned to. It has some c-type. It
// translates in C to some l-value. The scan returns its c-type
mo_objref_t
mom_cemit_scan_reference (struct mom_cemitlocalstate_st *csta,
                          mo_objref_t refob, mo_objref_t fromob, int depth);

// an expression or r-value can be computed. It has some c-type. It translates in C to some r-value.
mo_objref_t
mom_cemit_scan_expression (struct mom_cemitlocalstate_st *csta,
                           mo_value_t expv, mo_objref_t fromob, int depth);

// a member access
mo_objref_t
mom_cemit_scan_member_access (struct mom_cemitlocalstate_st *csta,
                              mo_objref_t accob, mo_objref_t fromob,
                              int depth);

// a macro expression or reference
mo_objref_t
mom_cemit_scan_macro_expr (struct mom_cemitlocalstate_st *csta,
                           mo_objref_t macob, mo_objref_t fromob, int depth,
                           bool isref);

// a chunk expression or reference
mo_objref_t
mom_cemit_scan_chunk_expr (struct mom_cemitlocalstate_st *csta,
                           mo_objref_t chkob, mo_objref_t fromob, int depth,
                           bool isref);

// a cast expression 
mo_objref_t
mom_cemit_scan_cast_expr (struct mom_cemitlocalstate_st *csta,
                          mo_objref_t castob, mo_objref_t fromob, int depth);

static inline bool
mom_cemit_is_ctype (struct mom_cemitlocalstate_st *csta, mo_objref_t ctypob)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil != NULL,
                    "cemit_is_ctype: bad csta@%p", csta);
  mo_cemitpayl_ty *cemp = csta->mo_cemsta_payl;
  MOM_ASSERTPRINTF (cemp && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC
                    && cemp->mo_cemit_locstate == csta,
                    "cemit_is_ctype: bad payl@%p in csta@%p", cemp, csta);
  if (!mo_dyncast_objref (ctypob))
    return false;
  mo_objref_t classob = ctypob->mo_ob_class;
  if (!mo_dyncast_objref (classob))
    return false;
  if (mo_hashset_contains (csta->mo_cemsta_hsetctypes, ctypob))
    return true;
  if (classob == momglob_array_ctype_class
      || classob == momglob_pointer_ctype_class
      || classob == MOM_PREDEF (basic_ctype_class)
      || classob == MOM_PREDEF (enum_ctype_class)
      || classob == MOM_PREDEF (struct_ctype_class)
      || classob == MOM_PREDEF (struct_pointer_ctype_class)
      || classob == MOM_PREDEF (union_ctype_class))
    return true;
  return false;
}                               /* end mom_cemit_is_ctype */

// We sometimes need to compare two C-types (e.g. for some kind of
// assignment left := right - or argument passing, etc...) and get
// their common supertype
mo_objref_t
mom_cemit_compare_ctypes (struct mom_cemitlocalstate_st * csta,
                          mo_objref_t leftctypob,
                          mo_objref_t rightctypob, mo_objref_t fromob);

void
mom_cemit_write_formals (struct mom_cemitlocalstate_st *csta,
                         mo_value_t formaltup)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil != NULL,
                    "cemit_write_formals: bad csta@%p", csta);
  mo_cemitpayl_ty *cemp = csta->mo_cemsta_payl;
  MOM_ASSERTPRINTF (cemp && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC
                    && cemp->mo_cemit_locstate == csta,
                    "cemit_write_formals: bad payl@%p in csta@%p",
                    cemp, csta);
  if (!formaltup)
    {
      mom_cemit_printf (csta, " (/*no formals*/ void)");
      return;
    }
  MOM_ASSERTPRINTF (mo_dyncast_tuple (formaltup),
                    "cemit_write_formals: bad formaltup %s",
                    mo_value_pnamestr (formaltup));
  int nbformals = mo_tuple_size (formaltup);
  if (nbformals == 0)
    {
      mom_cemit_printf (csta, " (void)");
      return;
    }
  mom_cemit_printf (csta, " (");
  for (int fix = 0; fix < nbformals; fix++)
    {
      mo_objref_t curformalob = mo_tuple_nth (formaltup, fix);
      mo_objref_t curfortypob =
        mo_dyncast_objref (mo_objref_get_attr
                           (curformalob, MOM_PREDEF (c_type)));
      MOM_ASSERTPRINTF (mom_cemit_is_ctype (csta, curfortypob),
                        "cemit_write_formals: bad curfortypob %s for formal %s",
                        mo_objref_pnamestr (curfortypob),
                        mo_objref_pnamestr (curformalob));
      const char *forstr = mom_gc_printf (MOM_FORMAL_PREFIX "%s",
                                          mo_objref_shortnamestr
                                          (curformalob));
      if (fix > 0)
        mom_cemit_printf (csta, ", ");
      mom_cemit_write_ctype_for (csta, curfortypob, forstr, 0);
    }
  mom_cemit_printf (csta, ")");
}                               /* end of mom_cemit_write_formals */


void
mom_cemit_write_block (struct mom_cemitlocalstate_st *csta,
                       mo_objref_t blockob, mo_objref_t fromob, int depth);

void
mom_cemit_write_instr (struct mom_cemitlocalstate_st *csta,
                       mo_objref_t insob, mo_objref_t fromob, int depth);

void
mom_cemit_function_code (struct mom_cemitlocalstate_st *csta,
                         mo_objref_t funob)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil != NULL,
                    "cemit_function_code: bad csta@%p", csta);
  mo_cemitpayl_ty *cemp = csta->mo_cemsta_payl;
  MOM_ASSERTPRINTF (cemp && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC
                    && cemp->mo_cemit_locstate == csta,
                    "cemit_function_code: bad payl@%p in csta@%p",
                    cemp, csta);
  MOM_ASSERTPRINTF (mo_dyncast_objref (funob),
                    "cemit_function_code: bad funob");
  char funid[MOM_CSTRIDSIZ];
  memset (funid, 0, sizeof (funid));
  mo_objref_idstr (funid, funob);
  mo_value_t funamv = mo_objref_namev (funob);
  if (mom_elapsed_real_time () > csta->mo_cemsta_timelimit)
    MOM_CEMITFAILURE (csta,
                      "cemit_function_code: %s timed out",
                      mo_objref_pnamestr (funob));
  MOM_ASSERTPRINTF (funob->mo_ob_class == MOM_PREDEF (c_inlined_class)
                    || funob->mo_ob_class == MOM_PREDEF (c_routine_class),
                    "cemit_function_code: funob %s with bad class %s (not c_inlined_class or c_routine_class)",
                    mo_objref_pnamestr (funob),
                    mo_objref_pnamestr (funob->mo_ob_class));
  mo_objref_t signob =
    mo_dyncast_objref (mo_objref_get_attr (funob, MOM_PREDEF (signature)));
  MOM_ASSERTPRINTF (mo_dyncast_objref (signob),
                    "cemit_function_code: funob %s without signature",
                    mo_objref_pnamestr (funob));
  MOM_ASSERTPRINTF (csta->mo_cemsta_curfun == NULL,
                    "cemit_function_code: funob %s but already doing function %s",
                    mo_objref_pnamestr (funob),
                    mo_objref_pnamestr (csta->mo_cemsta_curfun));
  char signid[MOM_CSTRIDSIZ];
  memset (signid, 0, sizeof (signid));
  mo_objref_idstr (signid, signob);
  csta->mo_cemsta_curfun = funob;
  mo_value_t formaltup =
    mo_dyncast_tuple (mo_objref_get_attr (funob, MOM_PREDEF (formals)));
  unsigned nbformals = mo_tuple_size (formaltup);
  csta->mo_cemsta_assoclocalrole =
    mo_assoval_reserve (NULL, 50 + 3 * nbformals);
  csta->mo_cemsta_hsetjumpedblocks =
    mo_hashset_reserve (NULL, 16 + 2 * nbformals);
  mo_value_t formtyptup =
    mo_dyncast_tuple (mo_objref_get_attr
                      (signob, MOM_PREDEF (formals_ctypes)));
  if (nbformals != mo_tuple_size (formtyptup))
    MOM_CEMITFAILURE
      (csta,
       "cemit_function_code: formals %s are mismatching formals_ctypes %s (expecting %u)",
       mo_value_pnamestr (formaltup), mo_value_pnamestr (formtyptup),
       nbformals);
  for (unsigned foix = 0; foix < nbformals; foix++)
    {
      mo_objref_t curformalob = mo_tuple_nth (formaltup, foix);
      mo_objref_t curfortypob = mo_tuple_nth (formtyptup, foix);
      MOM_ASSERTPRINTF (mo_dyncast_objref (curformalob),
                        "bad curformalob foix#%d", foix);
      MOM_ASSERTPRINTF (mo_dyncast_objref (curfortypob),
                        "bad curfortypob foix#%d", foix);
      MOM_ASSERTPRINTF (mo_hashset_contains
                        (csta->mo_cemsta_hsetctypes, curfortypob),
                        "unknown type curfortypob foix#%d", foix);
      if (curformalob->mo_ob_class != MOM_PREDEF (c_variable_class))
        MOM_CEMITFAILURE
          (csta,
           "cemit_function_code: formal#%d %s has bad class %s (expecting c_variable_class)",
           foix,
           mo_objref_pnamestr (curformalob),
           mo_objref_pnamestr (curformalob->mo_ob_class));
      if (mo_dyncast_objref
          (mo_objref_get_attr (curformalob, MOM_PREDEF (c_type)))
          != curfortypob)
        MOM_CEMITFAILURE
          (csta, "cemit_function_code: formal#%d %s should have c_type %s",
           foix,
           mo_objref_pnamestr (curformalob),
           mo_objref_pnamestr (curfortypob));
      mo_objref_t forolob =
        mo_dyncast_objref (mo_assoval_get
                           (csta->mo_cemsta_assoclocalrole, curformalob));
      if (forolob)
        MOM_CEMITFAILURE
          (csta, "cemit_function_code: formal#%d %s already used for role %s",
           foix, mo_objref_pnamestr (curformalob),
           mo_objref_pnamestr (forolob));
      forolob = mo_make_object ();
      forolob->mo_ob_class = MOM_PREDEF (c_role_class);
      mo_objref_comp_resize (forolob, MOMROLFORMIX__LASTFORMAL);
      mo_objref_put_comp (forolob, MOMROLFORMIX_ROLE, MOM_PREDEF (formals));
      mo_objref_put_comp (forolob, MOMROLFORMIX_CTYPE, curfortypob);
      mo_objref_put_comp (forolob, MOMROLFORMIX_FORMALRANK,
                          mo_int_to_value (foix));
      csta->mo_cemsta_assoclocalrole =
        mo_assoval_put (csta->mo_cemsta_assoclocalrole, curformalob, forolob);
    }
  mo_objref_t resultob =
    mo_dyncast_objref (mo_objref_get_attr (funob, MOM_PREDEF (result)));
  mo_objref_t restypob =
    mo_dyncast_objref (mo_objref_get_attr
                       (signob, MOM_PREDEF (result_ctype)));
  if (resultob && (!restypob || restypob == momglob_void))
    MOM_CEMITFAILURE
      (csta,
       "cemit_function_code: result %s provided but signature %s gives void",
       mo_objref_pnamestr (resultob), mo_objref_pnamestr (signob));
  if (!resultob && restypob && restypob != momglob_void)
    MOM_CEMITFAILURE
      (csta,
       "cemit_function_code: no result provided but signature %s gives %s",
       mo_objref_pnamestr (signob), mo_objref_pnamestr (restypob));
  if (resultob
      &&
      mo_dyncast_objref (mo_objref_get_attr (resultob, MOM_PREDEF (c_type)))
      != restypob)
    MOM_CEMITFAILURE (csta,
                      "cemit_function_code: result %s should have c_type %s",
                      mo_objref_pnamestr (resultob),
                      mo_objref_pnamestr (restypob));
  if (resultob && resultob->mo_ob_class != MOM_PREDEF (c_variable_class))
    MOM_CEMITFAILURE
      (csta,
       "cemit_function_code: result %s has bad class %s (expecting c_variable_class)",
       mo_objref_pnamestr (resultob),
       mo_objref_pnamestr (resultob->mo_ob_class));
  mo_objref_t forolob =
    mo_dyncast_objref (mo_assoval_get
                       (csta->mo_cemsta_assoclocalrole, resultob));
  if (forolob)
    MOM_CEMITFAILURE
      (csta, "cemit_function_code: result %s already used for role %s",
       mo_objref_pnamestr (resultob), mo_objref_pnamestr (forolob));
  if (resultob)
    {
      forolob = mo_make_object ();
      forolob->mo_ob_class = MOM_PREDEF (c_role_class);
      mo_objref_comp_resize (forolob, MOMROLVARIX__LASTVAR);
      mo_objref_put_comp (forolob, MOMROLVARIX_ROLE, MOM_PREDEF (result));
      mo_objref_put_comp (forolob, MOMROLVARIX_CTYPE, restypob);
      csta->mo_cemsta_assoclocalrole =
        mo_assoval_put (csta->mo_cemsta_assoclocalrole, resultob, forolob);
    }
  mo_objref_t bodyob =
    mo_dyncast_objref (mo_objref_get_attr (funob, MOM_PREDEF (body)));
  if (!bodyob)
    MOM_CEMITFAILURE (csta, "cemit_function_code: no body in function %s",
                      mo_objref_pnamestr (funob));
  mom_cemit_scan_block (csta, bodyob, funob, 0);
  /// check that every jumped block has been scanned
  {
    mo_value_t jumpedsetv =
      mo_hashset_elements_set (csta->mo_cemsta_hsetjumpedblocks);
    int nbjumped = mo_set_size (jumpedsetv);
    for (int jix = 0; jix < nbjumped; jix++)
      {
        mo_objref_t jumpedob = mo_set_nth (jumpedsetv, jix);
        mo_objref_t jumprolob =
          mo_dyncast_objref (mo_assoval_get
                             (csta->mo_cemsta_assoclocalrole, jumpedob));
        if (!jumprolob || jumprolob->mo_ob_class != MOM_PREDEF (c_role_class)
            || mo_objref_get_comp (jumprolob,
                                   MOMROLBLOCKIX_ROLE) != MOM_PREDEF (body))
          MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA (csta, jumpedob, jumprolob),
                            "cemit_function_code: jumped block %s not scanned, or with bad role %s",
                            mo_objref_pnamestr (jumpedob),
                            mo_objref_pnamestr (jumprolob));
      }
  }
  const char *funstr = mom_gc_printf (MOM_FUNC_PREFIX "%s", funid + 1);
  if (funob->mo_ob_class == MOM_PREDEF (c_inlined_class))
    {
      if (funamv)
        {
          mom_cemit_printf (csta,
                            "\n#define " MOM_FUNC_PREFIX "%s " MOM_FUNC_PREFIX
                            "%s\n", mo_string_cstr (funamv), funid + 1);
          mom_cemit_printf (csta, "// inlined function %s (%s)\n",
                            mo_string_cstr (funamv), funid);
        }
      else
        mom_cemit_printf (csta, "// inlined function %s\n", funid);
      mom_cemit_printf (csta, "static inline\n");
    }
  else if (funob->mo_ob_class == MOM_PREDEF (c_routine_class))
    {
      mom_cemit_printf (csta, "// signature of function %s (%s)\n",
                        mo_string_cstr (funamv), funid);
      if (mo_objref_namev (signob))
        mom_cemit_printf (csta,
                          "const char " MOM_SIGNATURE_PREFIX
                          "%s[] = \"%s\"; //%s\n", funid + 1, signid,
                          mo_objref_pnamestr (signob));
      else
        mom_cemit_printf (csta,
                          "const char " MOM_SIGNATURE_PREFIX
                          "%s[] = \"%s\";\n", funid + 1, signid);
      if (funamv)
        {
          mom_cemit_printf (csta,
                            "// signature for global function %s (%s)\n",
                            mo_string_cstr (funamv), funid);
          mom_cemit_printf (csta,
                            "const char " MOM_SIGNATURE_PREFIX
                            "%s[] = \"%s\";\n", mo_string_cstr (funamv),
                            signid);
          const char *funamstr =
            mom_gc_printf (MOM_FUNC_PREFIX "%s", mo_string_cstr (funamv));
          mom_cemit_printf (csta, "mo%s_ty %s MOM_OPTIMIZEDFUN;\n", signid,
                            funamstr);
          mom_cemit_printf (csta, "\n// global function %s (%s)\n",
                            mo_string_cstr (funamv), funid);
        }
      else
        mom_cemit_printf (csta, "\n// global function %s\n", funid);
    }
  else
    MOM_CEMITFAILURE (csta, "cemit_function_code: bad function of class %s",
                      mo_objref_pnamestr (funob->mo_ob_class));
  if (!restypob || restypob == momglob_void)
    mom_cemit_printf (csta, "void\n%s", funstr);
  else
    mom_cemit_write_ctype_for (csta, restypob, funstr, 0);
  mom_cemit_write_formals (csta, formaltup);
  mom_cemit_printf (csta, "\n{ // start of function %s\n",
                    mo_objref_shortnamestr (funob));
  if (resultob)
    {
      const char *restr = mom_gc_printf (MOM_RESULT_PREFIX "%s",
                                         mo_objref_shortnamestr (resultob));
      char resid[MOM_CSTRIDSIZ];
      memset (resid, 0, sizeof (resid));
      mo_objref_idstr (resid, resultob);
      mom_cemit_printf (csta, " ");
      mom_cemit_write_ctype_for (csta, restypob, restr, 0);
      if (mom_cemit_ctype_is_scalar (csta, restypob))
        mom_cemit_printf (csta, " = 0; // scalar result %s\n", resid);
      else
        mom_cemit_printf (csta, " = {}; // aggregate result %s\n", resid);
    }
  mom_cemit_write_block (csta, bodyob, funob, 0);
  if (resultob)
    {
      const char *restr = mom_gc_printf (MOM_RESULT_PREFIX "%s",
                                         mo_objref_shortnamestr (resultob));
      char resid[MOM_CSTRIDSIZ];
      memset (resid, 0, sizeof (resid));
      mo_objref_idstr (resid, resultob);
      mom_cemit_printf (csta, " return %s;\n", restr);
    }
  if (funamv)
    {
      const char *funamstr
        = mom_gc_printf (MOM_FUNC_PREFIX "%s", mo_string_cstr (funamv));
      mom_cemit_printf (csta, "} // end of function %s (%s)\n\n",
                        mo_string_cstr (funamv), funid);
      mom_cemit_printf (csta, "#undef %s\n", funamstr);
      mom_cemit_printf (csta, "// synonym function %s (%s)\n",
                        mo_string_cstr (funamv), funid);
      if (funob->mo_ob_class == MOM_PREDEF (c_inlined_class))
        mom_cemit_printf (csta, "static inline\n");
      if (!restypob || restypob == momglob_void)
        mom_cemit_printf (csta, "void\n%s", funamstr);
      else
        mom_cemit_write_ctype_for (csta, restypob, funamstr, 0);
      mom_cemit_write_formals (csta, formaltup);
      mom_cemit_printf (csta, "\n{ // start of synonym function %s\n",
                        mo_objref_shortnamestr (funob));
      if (!restypob || restypob == momglob_void)
        mom_cemit_printf (csta, " " MOM_FUNC_PREFIX "%s (", funid + 1);
      else
        mom_cemit_printf (csta, " return " MOM_FUNC_PREFIX "%s (", funid + 1);
      for (unsigned foix = 0; foix < nbformals; foix++)
        {
          mo_objref_t curformalob = mo_tuple_nth (formaltup, foix);
          if (foix > 0)
            mom_cemit_printf (csta, ", ");
          mom_cemit_printf (csta, MOM_FORMAL_PREFIX "%s",
                            mo_objref_shortnamestr (curformalob));
        }
      mom_cemit_printf (csta, "); // end synonym body\n");
      mom_cemit_printf (csta, "} // end of synonym function %s\n",
                        mo_objref_shortnamestr (funob));
      mom_cemit_printf (csta,
                        "#define %s /*synonymdef*/ " MOM_FUNC_PREFIX "%s\n\n",
                        funamstr, funid + 1);
    }
  else
    mom_cemit_printf (csta, "} // end of function %s\n\n", funid);
  csta->mo_cemsta_curfun = NULL;
  csta->mo_cemsta_assoclocalrole = NULL;
  csta->mo_cemsta_hsetjumpedblocks = NULL;
}                               /* end of mom_cemit_function_code */



void
mom_cemit_scan_block (struct mom_cemitlocalstate_st *csta,
                      mo_objref_t blockob, mo_objref_t fromob, int depth)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil != NULL,
                    "cemit_scan_block: bad csta@%p", csta);
  mo_cemitpayl_ty *cemp = csta->mo_cemsta_payl;
  MOM_ASSERTPRINTF (cemp && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC
                    && cemp->mo_cemit_locstate == csta,
                    "cemit_scan_block: bad payl@%p in csta@%p", cemp, csta);
  MOM_ASSERTPRINTF (mo_dyncast_objref (blockob),
                    "cemit_scan_block: bad blockob at depth %d from %s",
                    depth, mo_objref_pnamestr (fromob));
  if (depth > MOM_CEMIT_MAX_DEPTH)
    MOM_CEMITFAILURE (csta, "cemit_scan_block: block %s too deep %d from %s",
                      mo_objref_pnamestr (blockob), depth,
                      mo_objref_pnamestr (fromob));
  if (mom_elapsed_real_time () > csta->mo_cemsta_timelimit)
    MOM_CEMITFAILURE (csta,
                      "cemit_scan_block: %s timed out, depth %d, from %s",
                      mo_objref_pnamestr (blockob), depth,
                      mo_objref_pnamestr (fromob));
  mo_objref_t rolob =
    mo_dyncast_objref (mo_assoval_get
                       (csta->mo_cemsta_assoclocalrole, blockob));
  if (rolob)
    MOM_CEMITFAILURE (csta,
                      "cemit_scan_block: %s has already a role %s, depth %d, from %s",
                      mo_objref_pnamestr (blockob),
                      mo_objref_pnamestr (rolob), depth,
                      mo_objref_pnamestr (fromob));
  if (blockob->mo_ob_class == MOM_PREDEF (macro_block_class))
    {
      mo_objref_t macrob =
        mo_dyncast_objref (mo_objref_get_attr (blockob, MOM_PREDEF (macro)));
      if (!macrob)
        MOM_CEMITFAILURE (csta,
                          "cemit_scan_block: macro block %s without macro, depth %d, from %s",
                          mo_objref_pnamestr (blockob), depth,
                          mo_objref_pnamestr (fromob));
      if (macrob->mo_ob_paylkind !=
          MOM_PREDEF (signature_two_objects_to_object)
          || !macrob->mo_ob_payldata)
        MOM_CEMITFAILURE (csta,
                          "cemit_scan_block: macro block %s with bad macro %s, depth %d, from %s",
                          mo_objref_pnamestr (blockob),
                          mo_objref_pnamestr (macrob), depth,
                          mo_objref_pnamestr (fromob));
      mo_objref_t objcemit = csta->mo_cemsta_objcemit;
      mo_signature_two_objects_to_object_sigt *funrout =
        macrob->mo_ob_payldata;
      MOM_ASSERTPRINTF ((void *) funrout != NULL
                        && (void *) funrout != MOM_EMPTY_SLOT,
                        "cemit_scan_block: macro block %s with macro %s bad funrout, depth %d, from %s",
                        mo_objref_pnamestr (blockob),
                        mo_objref_pnamestr (macrob), depth,
                        mo_objref_pnamestr (fromob));
      mo_objref_t resmacrob = (*funrout) (blockob, objcemit);
      if (!mo_dyncast_objref (resmacrob))
        MOM_CEMITFAILURE (csta,
                          "cemit_scan_block: macro block %s with failed macro %s, depth %d, from %s",
                          mo_objref_pnamestr (blockob),
                          mo_objref_pnamestr (macrob),
                          depth, mo_objref_pnamestr (fromob));
      rolob = mo_make_object ();
      rolob->mo_ob_class = MOM_PREDEF (c_role_class);
      mo_objref_comp_resize (rolob, MOMROLBLOCKIX__LASTVAR);
      mo_objref_put_comp (rolob, MOMROLBLOCKIX_ROLE, MOM_PREDEF (macro));
      mo_objref_put_comp (rolob, MOMROLBLOCKIX_EXPANSION, resmacrob);
      mo_objref_put_comp (rolob, MOMROLBLOCKIX_IN, fromob);
      csta->mo_cemsta_assoclocalrole =
        mo_assoval_put (csta->mo_cemsta_assoclocalrole, blockob, rolob);
      mom_cemit_scan_block (csta, resmacrob, fromob, depth + 1);
      return;
    }
  if (blockob->mo_ob_class != MOM_PREDEF (c_block_class))
    MOM_CEMITFAILURE (csta,
                      "cemit_scan_block: block %s has bad class %s (c_block_class expected)",
                      mo_objref_pnamestr (blockob),
                      mo_objref_pnamestr (blockob->mo_ob_class));
  rolob = mo_make_object ();
  rolob->mo_ob_class = MOM_PREDEF (c_role_class);
  mo_objref_comp_resize (rolob, MOMROLBLOCKIX__LASTVAR);
  mo_objref_put_comp (rolob, MOMROLBLOCKIX_ROLE, MOM_PREDEF (body));
  mo_objref_put_comp (rolob, MOMROLBLOCKIX_IN, fromob);
  csta->mo_cemsta_assoclocalrole =
    mo_assoval_put (csta->mo_cemsta_assoclocalrole, blockob, rolob);
  mo_value_t locseq =
    mo_dyncast_sequence (mo_objref_get_attr (blockob, MOM_PREDEF (locals)));
  int nblocals = mo_sequence_size (locseq);
  for (int lix = 0; lix < nblocals; lix++)
    {
      mo_objref_t curlocalob = mo_sequence_nth (locseq, lix);
      MOM_ASSERTPRINTF (mo_dyncast_objref (curlocalob),
                        "cemit_scan_block: bad local #%d", lix);
      if (curlocalob->mo_ob_class != MOM_PREDEF (c_variable_class))
        MOM_CEMITFAILURE
          (csta,
           "cemit_scan_block: local#%d %s in block %s has bad class %s (expecting c_variable_class)",
           lix,
           mo_objref_pnamestr (curlocalob),
           mo_objref_pnamestr (blockob),
           mo_objref_pnamestr (curlocalob->mo_ob_class));
      mo_objref_t curltypob =
        mo_dyncast_objref (mo_objref_get_attr
                           (curlocalob, MOM_PREDEF (c_type)));
      if (!mo_hashset_contains (csta->mo_cemsta_hsetctypes, curltypob))
        MOM_CEMITFAILURE
          (csta,
           "cemit_scan_block: local#%d %s in block %s has unknown c_type %s",
           lix,
           mo_objref_pnamestr (curlocalob),
           mo_objref_pnamestr (blockob), mo_objref_pnamestr (curltypob));
      mo_objref_t locrolob =
        mo_dyncast_objref (mo_assoval_get
                           (csta->mo_cemsta_assoclocalrole, curlocalob));
      if (locrolob)
        MOM_CEMITFAILURE
          (csta,
           "cemit_scan_block: local#%d %s in block %s already used for role %s",
           lix, mo_objref_pnamestr (curlocalob), mo_objref_pnamestr (blockob),
           mo_objref_pnamestr (locrolob));
      locrolob = mo_make_object ();
      locrolob->mo_ob_class = MOM_PREDEF (c_role_class);
      mo_objref_comp_resize (locrolob, MOMROLVARIX__LASTVAR);
      mo_objref_put_comp (locrolob, MOMROLVARIX_ROLE, MOM_PREDEF (locals));
      mo_objref_put_comp (locrolob, MOMROLVARIX_CTYPE, curltypob);
      mo_objref_put_comp (locrolob, MOMROLVARIX_IN, blockob);
      csta->mo_cemsta_assoclocalrole =
        mo_assoval_put (csta->mo_cemsta_assoclocalrole, curlocalob, locrolob);
    }
  mo_objref_put_comp (rolob, MOMROLBLOCKIX_LOCALS, locseq);
  int nbinstr = mo_objref_comp_count (blockob);
  csta->mo_cemsta_assoclocalrole =
    mo_assoval_reserve (csta->mo_cemsta_assoclocalrole, 4 * nbinstr + 7);
  {
    mo_sequencevalue_ty *seqins = mo_sequence_allocate (nbinstr);
    for (int ix = 0; ix < nbinstr; ix++)
      {
        mo_objref_t instrob =
          mo_dyncast_objref (mo_objref_get_comp (blockob, ix));
        if (!instrob)
          MOM_CEMITFAILURE
            (csta,
             "cemit_scan_block: no instr#%d in block %s",
             ix, mo_objref_pnamestr (blockob));
        mom_cemit_scan_instr (csta, instrob, blockob, depth + 1);
        seqins->mo_seqobj[ix] = instrob;
      };
    mo_value_t tupins = mo_make_tuple_closeq (seqins);
    mo_objref_put_comp (rolob, MOMROLBLOCKIX_INSTRS, tupins);
  }
  // "forget" the locals by overwiting the role
  // so the same local can't be used again...
  for (int lix = 0; lix < nblocals; lix++)
    {
      mo_objref_t curlocalob = mo_sequence_nth (locseq, lix);
      MOM_ASSERTPRINTF (mo_dyncast_objref (curlocalob),
                        "cemit_scan_block: bad local #%d", lix);
      mo_objref_t locrolob =
        mo_dyncast_objref (mo_assoval_get
                           (csta->mo_cemsta_assoclocalrole, curlocalob));
      MOM_ASSERTPRINTF (locrolob != NULL,
                        "cemit_scan_block: bad locolob lix#%d", lix);
      mo_objref_put_comp (locrolob, MOMROLVARIX_ROLE, NULL);
    };
}                               /* end of mom_cemit_scan_block */


void
mom_cemit_scan_jump_instr (struct mom_cemitlocalstate_st *csta,
                           mo_objref_t instrob, mo_objref_t fromob, int depth)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil != NULL,
                    "cemit_scan_jump_instr: bad csta@%p", csta);
  mo_cemitpayl_ty *cemp = csta->mo_cemsta_payl;
  MOM_ASSERTPRINTF (cemp && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC
                    && cemp->mo_cemit_locstate == csta,
                    "cemit_scan_jump_instr: bad payl@%p in csta@%p", cemp,
                    csta);
  MOM_ASSERTPRINTF (mo_dyncast_objref (instrob)
                    && instrob->mo_ob_class ==
                    MOM_PREDEF (chunk_instruction_class),
                    "cemit_scan_jump_instr: chunk bad instrob at depth %d from %s",
                    depth, mo_objref_pnamestr (instrob));
  mo_objref_t rolob =
    mo_dyncast_objref (mo_assoval_get
                       (csta->mo_cemsta_assocmodulrole, instrob));
  if (rolob)
    MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA
                      (csta, instrob, rolob, mo_int_to_value (depth), fromob),
                      "cemit_scan_jump_instr: instr %s with existing role %s, depth %d, from %s",
                      mo_objref_pnamestr (instrob), mo_value_pnamestr (rolob),
                      depth, mo_objref_pnamestr (fromob));
  mo_objref_t toblockob =
    mo_dyncast_objref (mo_objref_get_attr (instrob, MOM_PREDEF (to)));
  if (!toblockob)
    MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA
                      (csta, instrob, mo_int_to_value (depth), fromob),
                      "cemit_scan_jump_instr: instr %s without `to`, depth %d, from %s",
                      mo_objref_pnamestr (instrob),
                      depth, mo_objref_pnamestr (fromob));
  if (toblockob->mo_ob_class != MOM_PREDEF (c_block_class))
    MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA
                      (csta, instrob, mo_int_to_value (depth), toblockob,
                       fromob),
                      "cemit_scan_jump_instr: instr %s with bad `to`:%s, depth %d, from %s",
                      mo_objref_pnamestr (instrob),
                      mo_objref_pnamestr (toblockob), depth,
                      mo_objref_pnamestr (fromob));
  rolob = mo_make_object ();
  rolob->mo_ob_class = MOM_PREDEF (c_role_class);
  mo_objref_comp_resize (rolob, MOMROLJUMPIX__LAST);
  mo_objref_put_comp (rolob, MOMROLJUMPIX_ROLE, MOM_PREDEF (to));
  mo_objref_put_comp (rolob, MOMROLJUMPIX_TO, toblockob);
  csta->mo_cemsta_assoclocalrole =
    mo_assoval_put (csta->mo_cemsta_assoclocalrole, instrob, rolob);
  csta->mo_cemsta_hsetjumpedblocks =
    mo_hashset_put (csta->mo_cemsta_hsetjumpedblocks, toblockob);
}                               /* end of mom_cemit_scan_jump_instr */



void
mom_cemit_scan_chunk_instr (struct mom_cemitlocalstate_st *csta,
                            mo_objref_t instrob, mo_objref_t fromob,
                            int depth)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil != NULL,
                    "cemit_scan_chunk_instr: bad csta@%p", csta);
  mo_cemitpayl_ty *cemp = csta->mo_cemsta_payl;
  MOM_ASSERTPRINTF (cemp && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC
                    && cemp->mo_cemit_locstate == csta,
                    "cemit_scan_chunk_instr: bad payl@%p in csta@%p", cemp,
                    csta);
  MOM_ASSERTPRINTF (mo_dyncast_objref (instrob)
                    && instrob->mo_ob_class ==
                    MOM_PREDEF (chunk_instruction_class),
                    "cemit_scan_chunk_instr: chunk bad instrob at depth %d from %s",
                    depth, mo_objref_pnamestr (instrob));
  mo_objref_t rolob =
    mo_dyncast_objref (mo_assoval_get
                       (csta->mo_cemsta_assocmodulrole, instrob));
  if (rolob)
    MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA
                      (csta, instrob, rolob, mo_int_to_value (depth), fromob),
                      "cemit_scan_chunk_instr: instr %s with existing role %s, depth %d, from %s",
                      mo_objref_pnamestr (instrob), mo_value_pnamestr (rolob),
                      depth, mo_objref_pnamestr (fromob));
  // the verbatim attribute gives the set of objects to be handled as-is
  mo_value_t verbatimv = mo_objref_get_attr (instrob, MOM_PREDEF (verbatim));
  if (verbatimv && !mo_dyncast_set (verbatimv))
    MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA
                      (csta, instrob, verbatimv, mo_int_to_value (depth),
                       fromob),
                      "cemit_scan_chunk_instr: instr %s with non-set verbatim %s, depth %d, from %s",
                      mo_objref_pnamestr (instrob),
                      mo_value_pnamestr (verbatimv), depth,
                      mo_objref_pnamestr (fromob));
  // the reference attribute gives the set of objects to be handled as reference-s
  mo_value_t referencev =
    mo_objref_get_attr (instrob, MOM_PREDEF (reference));
  if (referencev && !mo_dyncast_set (referencev))
    MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA
                      (csta, instrob, referencev, mo_int_to_value (depth),
                       fromob),
                      "cemit_scan_chunk_instr: instr %s with non-set reference %s, depth %d, from %s",
                      mo_objref_pnamestr (instrob),
                      mo_value_pnamestr (referencev), depth,
                      mo_objref_pnamestr (fromob));
  // the expression attribute gives the set of objects to be handled as expression-s
  mo_value_t expressionv =
    mo_objref_get_attr (instrob, MOM_PREDEF (expression));
  if (expressionv && !mo_dyncast_set (expressionv))
    MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA
                      (csta, instrob, expressionv, mo_int_to_value (depth),
                       fromob),
                      "cemit_scan_chunk_instr: instr %s with non-set expression %s, depth %d, from %s",
                      mo_objref_pnamestr (instrob),
                      mo_value_pnamestr (expressionv), depth,
                      mo_objref_pnamestr (fromob));
  // the block attribute gives the set of objects to be handled as expanded block-s
  mo_value_t blockv = mo_objref_get_attr (instrob, MOM_PREDEF (block));
  if (blockv && !mo_dyncast_set (blockv))
    MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA
                      (csta, instrob, blockv, mo_int_to_value (depth),
                       fromob),
                      "cemit_scan_chunk_instr: instr %s with non-set block %s, depth %d, from %s",
                      mo_objref_pnamestr (instrob),
                      mo_value_pnamestr (blockv), depth,
                      mo_objref_pnamestr (fromob));
  // the label attribute gives the set of objects to be handled as jumped block-s
  mo_value_t labelv = mo_objref_get_attr (instrob, MOM_PREDEF (label));
  if (labelv && !mo_dyncast_set (labelv))
    MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA
                      (csta, instrob, labelv, mo_int_to_value (depth),
                       fromob),
                      "cemit_scan_chunk_instr: instr %s with non-set label %s, depth %d, from %s",
                      mo_objref_pnamestr (instrob),
                      mo_value_pnamestr (labelv), depth,
                      mo_objref_pnamestr (fromob));
  unsigned nbcomp = mo_objref_comp_count (instrob);
  rolob = mo_make_object ();
  rolob->mo_ob_class = MOM_PREDEF (c_role_class);
  mo_objref_comp_resize (rolob, MOMROLCHUNKIX__LASTCHUNK);
  mo_objref_put_comp (rolob, MOMROLCHUNKIX_ROLE, MOM_PREDEF (chunk));
  mo_objref_put_comp (rolob, MOMROLCHUNKIX_VERBATIM, verbatimv);
  mo_objref_put_comp (rolob, MOMROLCHUNKIX_REFERENCE, referencev);
  mo_objref_put_comp (rolob, MOMROLCHUNKIX_EXPRESSION, expressionv);
  mo_objref_put_comp (rolob, MOMROLCHUNKIX_BLOCK, blockv);
  mo_objref_put_comp (rolob, MOMROLCHUNKIX_LABEL, labelv);
  MOM_INFORMPRINTF
    ("cemit_scan_chunk_instr: instrob %s verbatimv %s referencev %s expressionv %s rolob %s",
     mo_objref_pnamestr (instrob), mo_value_pnamestr (verbatimv),
     mo_value_pnamestr (referencev), mo_value_pnamestr (expressionv),
     mo_objref_pnamestr (rolob));
  csta->mo_cemsta_assoclocalrole =
    mo_assoval_put (csta->mo_cemsta_assoclocalrole, instrob, rolob);
  for (unsigned cix = 0; cix < nbcomp; cix++)
    {
      mo_value_t curcomp = mo_objref_get_comp (instrob, cix);
      enum mo_valkind_en curkind = mo_kind_of_value (curcomp);
      if (curkind == mo_KNONE || curkind == mo_KINT || curkind == mo_KSTRING)
        continue;
      if (curkind == mo_KTUPLE || curkind == mo_KSET)
        MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA
                          (csta, instrob, curcomp, mo_int_to_value (depth),
                           rolob, fromob),
                          "cemit_scan_chunk_instr: instr %s with unexpected"
                          " sequence comp#%d %s, depth %d, role %s, from %s",
                          mo_objref_pnamestr (instrob), cix,
                          mo_value_pnamestr (curcomp), depth,
                          mo_objref_pnamestr (rolob),
                          mo_objref_pnamestr (fromob));
      mo_objref_t compob = mo_dyncast_objref (curcomp);
      if (mo_set_contains (verbatimv, compob))
        {
          continue;
        }
      else if (mo_set_contains (referencev, compob))
        {
          mom_cemit_scan_reference (csta, compob, instrob, depth + 1);
          continue;
        }
      else if (mo_set_contains (blockv, compob))
        {
          mom_cemit_scan_block (csta, compob, instrob, depth + 1);
          continue;
        }
      else if (mo_set_contains (labelv, compob))
        {
          if (compob->mo_ob_class != MOM_PREDEF (c_block_class))
            MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA
                              (csta, instrob, curcomp,
                               mo_int_to_value (depth), fromob, rolob),
                              "cemit_scan_chunk_instr: instr %s with bad label comp#%u %s,"
                              " depth %d, from %s, role %s",
                              mo_objref_pnamestr (instrob), cix,
                              mo_value_pnamestr (curcomp), depth,
                              mo_objref_pnamestr (fromob),
                              mo_objref_pnamestr (rolob));
          csta->mo_cemsta_hsetjumpedblocks =
            mo_hashset_put (csta->mo_cemsta_hsetjumpedblocks, compob);
          continue;
        }
      else if (!compob || mo_set_contains (expressionv, compob))
        {
          mom_cemit_scan_expression (csta, curcomp, instrob, depth + 1);
          continue;
        }
      else
        {
          MOM_WARNPRINTF
            ("cemit_scan_chunk_instr: compob#%u %s unexpected in instrob %s",
             cix, mo_objref_pnamestr (compob), mo_objref_pnamestr (instrob));
          MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA
                            (csta, instrob, curcomp, mo_int_to_value (depth),
                             fromob, rolob),
                            "cemit_scan_chunk_instr: instr %s with unexpected comp#%u %s,"
                            " role %s, depth %d, from %s",
                            mo_objref_pnamestr (instrob), cix,
                            mo_value_pnamestr (curcomp),
                            mo_value_pnamestr (rolob), depth,
                            mo_objref_pnamestr (fromob));
        }
    }
}                               /* end mom_cemit_scan_chunk_instr */


void
mom_cemit_scan_condition (struct mom_cemitlocalstate_st *csta,
                          mo_objref_t condob, mo_objref_t fromob, int depth,
                          mo_vectvaldatapayl_ty ** pcondvec)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil != NULL,
                    "cemit_scan_condition: bad csta@%p", csta);
  mo_cemitpayl_ty *cemp = csta->mo_cemsta_payl;
  MOM_ASSERTPRINTF (cemp && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC
                    && cemp->mo_cemit_locstate == csta,
                    "cemit_scan_condition: bad payl@%p in csta@%p", cemp,
                    csta);
  if (!mo_dyncast_objref (condob)
      || condob->mo_ob_class != MOM_PREDEF (c_cond_class))
    MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA
                      (csta, condob, mo_int_to_value (depth), fromob),
                      "cemit_scan_condition: bad condob %s"
                      " depth %d, from %s", mo_objref_pnamestr (condob),
                      depth, mo_objref_pnamestr (fromob));
  mo_objref_t rolob =
    mo_dyncast_objref (mo_assoval_get
                       (csta->mo_cemsta_assoclocalrole, condob));
  if (rolob)
    MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA
                      (csta, condob, rolob, mo_int_to_value (depth), fromob),
                      "cemit_scan_condition: condob %s already with role %s"
                      " depth %d, from %s", mo_objref_pnamestr (condob),
                      mo_objref_pnamestr (rolob),
                      depth, mo_objref_pnamestr (fromob));
  mo_value_t whenv = mo_objref_get_attr (condob, MOM_PREDEF (when));
  mo_objref_t bodyob =
    mo_dyncast_objref (mo_objref_get_attr (condob, MOM_PREDEF (body)));
  if (!bodyob)
    MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA
                      (csta, condob, mo_int_to_value (depth), fromob),
                      "cemit_scan_condition: condob %s without `body`"
                      " depth %d, from %s", mo_objref_pnamestr (condob),
                      depth, mo_objref_pnamestr (fromob));
  rolob = mo_make_object ();
  rolob->mo_ob_class = MOM_PREDEF (c_role_class);
  mo_objref_comp_resize (rolob, MOMROLCONDITIONIX__LAST);
  mo_objref_put_comp (rolob, MOMROLCONDITIONIX_ROLE, MOM_PREDEF (when));
  mo_objref_put_comp (rolob, MOMROLCONDITIONIX_WHEN, whenv);
  mo_objref_put_comp (rolob, MOMROLCONDITIONIX_BODY, bodyob);
  csta->mo_cemsta_assoclocalrole =
    mo_assoval_put (csta->mo_cemsta_assoclocalrole, condob, rolob);
  mo_objref_t typwhenob =
    mom_cemit_scan_expression (csta, whenv, condob, depth + 1);
  if (!typwhenob || !mom_cemit_ctype_is_scalar (csta, typwhenob))
    MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA
                      (csta, condob, mo_int_to_value (depth),
                       whenv ? whenv : MOM_EMPTY_SLOT, fromob, typwhenob),
                      "cemit_scan_condition: condob %s with when %s of non-scalar type %s"
                      " depth %d, from %s", mo_objref_pnamestr (condob),
                      mo_value_pnamestr (whenv),
                      mo_objref_pnamestr (typwhenob), depth,
                      mo_objref_pnamestr (fromob));
  mom_cemit_scan_block (csta, bodyob, condob, depth + 1);
  *pcondvec = mo_vectval_append (*pcondvec, condob);
}                               /* end of mom_cemit_scan_condition */


void
mom_cemit_scan_cond_instr (struct mom_cemitlocalstate_st *csta,
                           mo_objref_t instrob, mo_objref_t fromob, int depth)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil != NULL,
                    "cemit_scan_cond_instr: bad csta@%p", csta);
  mo_cemitpayl_ty *cemp = csta->mo_cemsta_payl;
  MOM_ASSERTPRINTF (cemp && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC
                    && cemp->mo_cemit_locstate == csta,
                    "cemit_scan_cond_instr: bad payl@%p in csta@%p", cemp,
                    csta);
  MOM_ASSERTPRINTF (mo_dyncast_objref (instrob)
                    && instrob->mo_ob_class ==
                    MOM_PREDEF (conditional_instruction_class),
                    "cemit_scan_cond_instr: bad instrob at depth %d from %s",
                    depth, mo_objref_pnamestr (instrob));
  mo_objref_t rolob =
    mo_dyncast_objref (mo_assoval_get
                       (csta->mo_cemsta_assoclocalrole, instrob));
  if (rolob)
    MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA (csta, instrob, rolob, fromob),
                      "cemit_scan_cond_instr: instrob %s already has role %s at depth %d from %s",
                      mo_objref_pnamestr (instrob),
                      mo_objref_pnamestr (rolob), depth,
                      mo_objref_pnamestr (fromob));
  rolob = mo_make_object ();
  rolob->mo_ob_class = MOM_PREDEF (c_role_class);
  mo_objref_comp_resize (rolob, MOMROLCONDIX__LAST);
  mo_objref_put_comp (rolob, MOMROLCONDIX_ROLE, MOM_PREDEF (conditional));
  // slot MOMROLCONDIX_CONDITIONS will be filled later
  csta->mo_cemsta_assoclocalrole =
    mo_assoval_put (csta->mo_cemsta_assoclocalrole, instrob, rolob);

  // a cond instruction has a sequence of components all instances of c_cond_class
  // each condition has a `when` and `body` attribute..., or sequences of such instances
  int nbconds = mo_objref_comp_count (instrob);
  mo_vectvaldatapayl_ty *condvect =
    mo_vectval_reserve (NULL, 5 * nbconds / 4 + 2);
  for (int cdix = 0; cdix < nbconds; cdix++)
    {
      mo_value_t curcondv = mo_objref_get_comp (instrob, cdix);
      mo_objref_t curcondob = mo_dyncast_objref (curcondv);
      if (curcondob)
        mom_cemit_scan_condition (csta, curcondob, instrob, depth + 1,
                                  &condvect);
      else
        {
          mo_value_t curcondseq = mo_dyncast_sequence (curcondv);
          if (curcondseq)
            {
              unsigned lnseq = mo_sequence_size (curcondseq);
              condvect = mo_vectval_reserve (condvect, 4 * lnseq / 3 + 2);
              for (unsigned six = 0; six < lnseq; six++)
                mom_cemit_scan_condition (csta,
                                          mo_sequence_nth (curcondseq, six),
                                          instrob, depth + 1, &condvect);
            }
          else
            MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA
                              (csta, curcondv, mo_int_to_value (depth),
                               fromob),
                              "cemit_scan_cond_instr: cond instr %s with bad component#%u %s"
                              " depth %d, from %s",
                              mo_objref_pnamestr (instrob), cdix,
                              mo_value_pnamestr (curcondv), depth,
                              mo_objref_pnamestr (fromob));
        }
    }
  mo_objref_put_comp (rolob, MOMROLCONDIX_CONDITIONS,
                      mo_vectval_objects_tuple (condvect));
}                               /* end mom_cemit_scan_cond_instr */


void
mom_cemit_scan_assign_instr (struct mom_cemitlocalstate_st *csta,
                             mo_objref_t instrob, mo_objref_t fromob,
                             int depth)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil != NULL,
                    "cemit_scan_assign_instr: bad csta@%p", csta);
  mo_cemitpayl_ty *cemp = csta->mo_cemsta_payl;
  MOM_ASSERTPRINTF (cemp && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC
                    && cemp->mo_cemit_locstate == csta,
                    "cemit_scan_assign_instr: bad payl@%p in csta@%p", cemp,
                    csta);
  MOM_ASSERTPRINTF (mo_dyncast_objref (instrob)
                    && instrob->mo_ob_class ==
                    MOM_PREDEF (assignment_instruction_class),
                    "cemit_scan_assign_instr: chunk bad instrob at depth %d from %s",
                    depth, mo_objref_pnamestr (instrob));
  mo_value_t fromv = mo_objref_get_attr (instrob, MOM_PREDEF (from));
  mo_objref_t tob =
    mo_dyncast_objref (mo_objref_get_attr (instrob, MOM_PREDEF (to)));
  if (!tob)
    MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA (csta, instrob, fromob),
                      "cemit_scan_assign_instr: instr %s without `to`, depth %d, from %s",
                      mo_objref_pnamestr (instrob),
                      depth, mo_objref_pnamestr (fromob));
  mo_objref_t rolob =
    mo_dyncast_objref (mo_assoval_get
                       (csta->mo_cemsta_assoclocalrole, instrob));
  if (rolob)
    MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA (csta, instrob, fromob, rolob),
                      "cemit_scan_assign_instr: instr %s already with role %s, depth %d, from %s",
                      mo_objref_pnamestr (instrob),
                      mo_objref_pnamestr (rolob),
                      depth, mo_objref_pnamestr (fromob));
  rolob = mo_make_object ();
  rolob->mo_ob_class = MOM_PREDEF (c_role_class);
  mo_objref_comp_resize (rolob, MOMROLASSIGNIX__LASTASSIGN);
  mo_objref_put_comp (rolob, MOMROLASSIGNIX_ROLE, MOM_PREDEF (assign));
  mo_objref_put_comp (rolob, MOMROLASSIGNIX_TOREFERENCE, tob);
  mo_objref_put_comp (rolob, MOMROLASSIGNIX_FROMEXPR, fromv);
  csta->mo_cemsta_assoclocalrole =
    mo_assoval_put (csta->mo_cemsta_assoclocalrole, instrob, rolob);
  mo_objref_t totypob =
    mom_cemit_scan_reference (csta, tob, instrob, depth + 1);
  mo_objref_put_comp (rolob, MOMROLASSIGNIX_TOTYPE, totypob);
  mo_objref_t fromtypob =
    mom_cemit_scan_expression (csta, fromv, instrob, depth + 1);
  mo_objref_put_comp (rolob, MOMROLASSIGNIX_FROMTYPE, fromtypob);
  (void) mom_cemit_compare_ctypes (csta, totypob, fromtypob, instrob);
}                               /* end mom_cemit_scan_assign_instr */


void
mom_cemit_scan_call_instr (struct mom_cemitlocalstate_st *csta,
                           mo_objref_t instrob, mo_objref_t fromob, int depth)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil != NULL,
                    "cemit_scan_call_instr: bad csta@%p", csta);
  mo_cemitpayl_ty *cemp = csta->mo_cemsta_payl;
  MOM_ASSERTPRINTF (cemp && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC
                    && cemp->mo_cemit_locstate == csta,
                    "cemit_scan_call_instr: bad payl@%p in csta@%p", cemp,
                    csta);
  MOM_ASSERTPRINTF (mo_dyncast_objref (instrob)
                    && instrob->mo_ob_class ==
                    MOM_PREDEF (call_instruction_class),
                    "cemit_scan_call_instr: chunk bad instrob at depth %d from %s",
                    depth, mo_objref_pnamestr (instrob));
  mo_objref_t callob =
    mo_dyncast_objref (mo_objref_get_attr (instrob, MOM_PREDEF (call)));
  if (!callob)
    MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA (csta, instrob, fromob),
                      "cemit_scan_call_instr: instr %s without `call`, depth %d, from %s",
                      mo_objref_pnamestr (instrob),
                      depth, mo_objref_pnamestr (fromob));
  mo_objref_t signob =
    mo_dyncast_objref (mo_objref_get_attr (instrob, MOM_PREDEF (signature)));
  if (!signob)
    {
      if (callob->mo_ob_class == MOM_PREDEF (c_inlined_class)
          || callob->mo_ob_class == MOM_PREDEF (c_routine_class))
        signob =
          mo_dyncast_objref (mo_objref_get_attr
                             (callob, MOM_PREDEF (signature)));
      if (!signob)
        MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA (csta, instrob, fromob),
                          "cemit_scan_call_instr: instr %s without `signature`, depth %d, from %s",
                          mo_objref_pnamestr (instrob),
                          depth, mo_objref_pnamestr (fromob));
    }
  mo_value_t formtup =
    mo_dyncast_tuple (mo_objref_get_attr
                      (signob, MOM_PREDEF (formals_ctypes)));
  if (signob->mo_ob_class != MOM_PREDEF (signature_class) || !formtup)
    MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA (csta, instrob, signob, fromob),
                      "cemit_scan_call_instr: instr %s with bad `signature` %s, depth %d, from %s",
                      mo_objref_pnamestr (instrob),
                      mo_objref_pnamestr (signob),
                      depth, mo_objref_pnamestr (fromob));
  mo_objref_t rolob =
    mo_dyncast_objref (mo_assoval_get
                       (csta->mo_cemsta_assoclocalrole, instrob));
  if (rolob)
    MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA (csta, instrob, fromob, rolob),
                      "cemit_scan_call_instr: instr %s already with role %s, depth %d, from %s",
                      mo_objref_pnamestr (instrob),
                      mo_objref_pnamestr (rolob),
                      depth, mo_objref_pnamestr (fromob));
  int instrcnt = mo_objref_comp_count (instrob);
  if (instrcnt != (int) mo_tuple_size (formtup))
    MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA (csta, instrob, signob, fromob),
                      "cemit_scan_call_instr: instr %s arity mismatch:"
                      " got %d expecting %d arguments in signature %s,"
                      " depth %d, from %s",
                      mo_objref_pnamestr (instrob),
                      instrcnt,
                      (int) mo_tuple_size (formtup),
                      mo_objref_pnamestr (signob),
                      depth, mo_objref_pnamestr (fromob));
  rolob = mo_make_object ();
  rolob->mo_ob_class = MOM_PREDEF (c_role_class);
  mo_objref_comp_resize (rolob, MOMROLCALLIX__LAST);
  mo_objref_put_comp (rolob, MOMROLCALLIX_ROLE, MOM_PREDEF (call));
  mo_objref_put_comp (rolob, MOMROLCALLIX_CALL, callob);
  mo_objref_put_comp (rolob, MOMROLCALLIX_SIGNATURE, signob);
  csta->mo_cemsta_assoclocalrole =
    mo_assoval_put (csta->mo_cemsta_assoclocalrole, instrob, rolob);
  if (callob->mo_ob_class != MOM_PREDEF (c_inlined_class)
      && callob->mo_ob_class != MOM_PREDEF (c_routine_class))
    {
      mo_objref_t typcallob =
        mom_cemit_scan_expression (csta, callob, instrob, depth + 1);
      bool wrongtypcall = true;
      if (typcallob)
        {
          if (typcallob->mo_ob_class == MOM_PREDEF (signature_class))
            wrongtypcall = (typcallob != signob);
          else if (typcallob->mo_ob_class == momglob_pointer_ctype_class)
            wrongtypcall = (mo_objref_get_comp (typcallob, 0) != signob);
        };
      if (wrongtypcall)
        MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA
                          (csta, instrob, signob, fromob, typcallob),
                          "cemit_scan_call_instr: instr %s indirect call mismatch,"
                          " callee type %s different of signature %s,"
                          " depth %d, from %s", mo_objref_pnamestr (instrob),
                          mo_objref_pnamestr (typcallob),
                          mo_objref_pnamestr (signob), depth,
                          mo_objref_pnamestr (fromob));
    }
  else
    {
      mo_objref_t callsignob
        =
        mo_dyncast_objref (mo_objref_get_attr
                           (callob, MOM_PREDEF (signature)));
      if (callsignob != signob)
        MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA
                          (csta, instrob, signob, fromob, callsignob),
                          "cemit_scan_call_instr: instr %s call mismatch,"
                          " callee signature %s different of signature %s,"
                          " depth %d, from %s", mo_objref_pnamestr (instrob),
                          mo_objref_pnamestr (callsignob),
                          mo_objref_pnamestr (signob), depth,
                          mo_objref_pnamestr (fromob));
    }
  for (int ix = 0; ix < instrcnt; ix++)
    {
      mo_value_t curarg = mo_objref_get_comp (instrob, ix);
      mo_objref_t formtypob = mo_tuple_nth (formtup, ix);
      mo_objref_t curtypob =
        mom_cemit_scan_expression (csta, curarg, instrob, depth + 1);
      if (!mom_cemit_compare_ctypes (csta, formtypob, curtypob, instrob))
        MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA
                          (csta, instrob, signob, fromob, formtypob,
                           curtypob),
                          "cemit_scan_call_instr: instr %s call arg#%d mistyped,"
                          " expecting %s got %s," " depth %d, from %s",
                          mo_objref_pnamestr (instrob), ix,
                          mo_objref_pnamestr (formtypob),
                          mo_objref_pnamestr (curtypob), depth,
                          mo_objref_pnamestr (fromob));
    }
}                               /* end mom_cemit_scan_call_instr */



#define MOM_CEMIT_MAX_OBJECT_CASE 1024
void
mom_cemit_scan_object_case (struct mom_cemitlocalstate_st *csta,
                            mo_objref_t objcasob, mo_objref_t insrolob,
                            mo_objref_t fromob, int depth)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil != NULL,
                    "cemit_scan_object_case: bad csta@%p", csta);
  mo_cemitpayl_ty *cemp = csta->mo_cemsta_payl;
  MOM_ASSERTPRINTF (cemp && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC
                    && cemp->mo_cemit_locstate == csta,
                    "cemit_scan_object_case: bad payl@%p in csta@%p", cemp,
                    csta);
  MOM_ASSERTPRINTF (mo_dyncast_objref (insrolob)
                    && insrolob->mo_ob_class == MOM_PREDEF (c_role_class)
                    && mo_objref_comp_count (insrolob) >= MOMROLCASEIX__LAST,
                    "cemit_scan_object_case: bad insrolob %s",
                    mo_objref_pnamestr (insrolob));
  mo_objref_t amapob =
    mo_dyncast_objref (mo_objref_get_comp (insrolob, MOMROLCASEIX_MAP));
  MOM_ASSERTPRINTF (mo_dyncast_objref (amapob)
                    && amapob->mo_ob_paylkind == MOM_PREDEF (payload_assoval),
                    "cemit_scan_object_case: bad amapob %s",
                    mo_objref_pnamestr (amapob));
  mo_assovaldatapayl_ty *amapassoc =
    mo_dyncastpayl_assoval (amapob->mo_ob_payldata);
  MOM_ASSERTPRINTF (amapassoc, "cemit_scan_object_case: bad amapassoc");
  if (mo_assoval_count (amapassoc) >= MOM_CEMIT_MAX_OBJECT_CASE)
    MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA (csta, objcasob, insrolob, fromob),
                      "cemit_scan_object_case: too many %u cases for objcasob %s insrolob %s,"
                      " fromob %s depth %d",
                      mo_assoval_count (amapassoc),
                      mo_objref_pnamestr (objcasob),
                      mo_objref_pnamestr (insrolob),
                      mo_objref_pnamestr (fromob), depth);
  if (!mo_dyncast_objref (objcasob)
      || objcasob->mo_ob_class != MOM_PREDEF (c_object_case_class))
    MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA (csta, objcasob, insrolob, fromob),
                      "cemit_scan_object_case: bad objcasob %s insrolob %s,"
                      " fromob %s depth %d", mo_objref_pnamestr (objcasob),
                      mo_objref_pnamestr (insrolob),
                      mo_objref_pnamestr (fromob), depth);
  mo_value_t whenv = mo_objref_get_attr (objcasob, MOM_PREDEF (when));
  mo_objref_t whenob = NULL;
  if (!(whenob = mo_dyncast_objref (whenv)) && mo_set_size (whenv) <= 0)
    MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA (csta, objcasob, insrolob, fromob),
                      "cemit_scan_object_case: objcasob %s has bad `when` %s (not object or non-empty set) insrolob %s,"
                      " fromob %s depth %d",
                      mo_objref_pnamestr (objcasob),
                      mo_value_pnamestr (whenv),
                      mo_objref_pnamestr (insrolob),
                      mo_objref_pnamestr (fromob), depth);
  mo_objref_t bodyob =
    mo_dyncast_objref (mo_objref_get_attr (objcasob, MOM_PREDEF (body)));
  mo_objref_t labelob =
    mo_dyncast_objref (mo_objref_get_attr (objcasob, MOM_PREDEF (label)));
  if (!bodyob && !labelob)
    MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA (csta, objcasob, insrolob, fromob),
                      "cemit_scan_object_case: objcasob %s has no `body` or `label` insrolob %s,"
                      " fromob %s depth %d",
                      mo_objref_pnamestr (objcasob),
                      mo_objref_pnamestr (insrolob),
                      mo_objref_pnamestr (fromob), depth);
  if (bodyob && labelob)
    MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA (csta, objcasob, insrolob, fromob),
                      "cemit_scan_object_case: objcasob %s has both `body` or `label` insrolob %s,"
                      " fromob %s depth %d",
                      mo_objref_pnamestr (objcasob),
                      mo_objref_pnamestr (insrolob),
                      mo_objref_pnamestr (fromob), depth);
  mo_objref_t rolcasob =
    mo_dyncast_objref (mo_assoval_get
                       (csta->mo_cemsta_assoclocalrole, objcasob));
  if (rolcasob)
    MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA
                      (csta, objcasob, rolcasob, insrolob, fromob),
                      "cemit_scan_object_case: objcasob %s has rolcasob %s insrolob %s,"
                      " fromob %s depth %d", mo_objref_pnamestr (objcasob),
                      mo_objref_pnamestr (rolcasob),
                      mo_objref_pnamestr (insrolob),
                      mo_objref_pnamestr (fromob), depth);
  rolcasob = mo_make_object ();
  rolcasob->mo_ob_class = MOM_PREDEF (c_role_class);
  mo_objref_comp_resize (rolcasob, MOMROLOBJCASIX__LAST);
  mo_objref_put_comp (rolcasob, MOMROLOBJCASIX_ROLE, MOM_PREDEF (object));
  mo_objref_put_comp (rolcasob, MOMROLOBJCASIX_INSTR, fromob);
  mo_objref_put_comp (rolcasob, MOMROLOBJCASIX_OBJECT, whenv);
  csta->mo_cemsta_assoclocalrole =
    mo_assoval_put (csta->mo_cemsta_assoclocalrole, objcasob, rolcasob);
  if (whenob)
    {
      if (mo_assoval_get (amapassoc, whenob))
        MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA
                          (csta, objcasob, rolcasob, insrolob, fromob),
                          "cemit_scan_object_case: objcasob %s for already handled %s insrolob %s,"
                          " fromob %s depth %d",
                          mo_objref_pnamestr (objcasob),
                          mo_objref_pnamestr (whenob),
                          mo_objref_pnamestr (insrolob),
                          mo_objref_pnamestr (fromob), depth);
      if (!mo_set_contains (csta->mo_cemsta_objset, whenob)
          || mo_objref_space (whenob) != mo_SPACE_PREDEF)
        MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA
                          (csta, objcasob, rolcasob, insrolob, fromob),
                          "cemit_scan_object_case: objcasob %s for nonconstant whenob %s insrolob %s,"
                          " fromob %s depth %d",
                          mo_objref_pnamestr (objcasob),
                          mo_objref_pnamestr (whenob),
                          mo_objref_pnamestr (insrolob),
                          mo_objref_pnamestr (fromob), depth);
      amapassoc = mo_assoval_put (amapassoc, whenob, objcasob);
      amapob->mo_ob_payldata = amapassoc;
    }
  else if (mo_dyncast_set (whenv))
    {
      unsigned nbwhen = mo_set_size (whenv);    // not zero
      for (unsigned wix = 0; wix < nbwhen; wix++)
        {
          mo_objref_t whenelemob = mo_set_nth (whenv, wix);
          if (mo_assoval_get (amapassoc, whenelemob))
            MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA
                              (csta, objcasob, rolcasob, insrolob, fromob),
                              "cemit_scan_object_case: objcasob %s for already handled %s insrolob %s,"
                              " fromob %s depth %d",
                              mo_objref_pnamestr (objcasob),
                              mo_objref_pnamestr (whenelemob),
                              mo_objref_pnamestr (insrolob),
                              mo_objref_pnamestr (fromob), depth);
          if (!mo_set_contains (csta->mo_cemsta_objset, whenelemob)
              || mo_objref_space (whenelemob) != mo_SPACE_PREDEF)
            MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA
                              (csta, objcasob, rolcasob, insrolob, fromob),
                              "cemit_scan_object_case: objcasob %s for nonconstant whenelemob %s insrolob %s,"
                              " fromob %s depth %d",
                              mo_objref_pnamestr (objcasob),
                              mo_objref_pnamestr (whenelemob),
                              mo_objref_pnamestr (insrolob),
                              mo_objref_pnamestr (fromob), depth);
          amapassoc = mo_assoval_put (amapassoc, whenelemob, objcasob);
          amapob->mo_ob_payldata = amapassoc;
        }
    }
  else                          // should never happen
    MOM_FATAPRINTF
      ("cemit_scan_object_case objcasob=%s with impossible whenv %s",
       mo_objref_pnamestr (objcasob), mo_value_pnamestr (whenv));
  if (bodyob)
    {
      mo_objref_t blocksetob =
        mo_dyncast_objref (mo_objref_get_comp
                           (insrolob, MOMROLCASEIX_BLOCKSET));
      mo_hashsetpayl_ty *blset = NULL;
      if (MOM_UNLIKELY
          (!blocksetob
           || blocksetob->mo_ob_paylkind != MOM_PREDEF (payload_hashset)
           || !(blset = mo_dyncastpayl_hashset (blocksetob->mo_ob_payldata))))
        MOM_FATAPRINTF
          ("cemit_scan_object_case insrolob %s with bad blockset",
           mo_objref_pnamestr (insrolob));
      blset = mo_hashset_put (blset, bodyob);
      blocksetob->mo_ob_payldata = blset;
      mom_cemit_scan_block (csta, bodyob, objcasob, depth + 1);
    }
  else if (labelob)
    {
      if (labelob->mo_ob_class != MOM_PREDEF (c_block_class))
        MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA
                          (csta, objcasob, rolcasob, insrolob, fromob),
                          "cemit_scan_object_case: objcasob %s with bad labelob %s insrolob %s,"
                          " fromob %s depth %d",
                          mo_objref_pnamestr (objcasob),
                          mo_objref_pnamestr (labelob),
                          mo_objref_pnamestr (insrolob),
                          mo_objref_pnamestr (fromob), depth);
      csta->mo_cemsta_hsetjumpedblocks =
        mo_hashset_put (csta->mo_cemsta_hsetjumpedblocks, labelob);
    }
  else                          // should never happen
    MOM_FATAPRINTF
      ("cemit_scan_object_case objcasob=%s without body or label",
       mo_objref_pnamestr (objcasob));
}                               /* end of mom_cemit_scan_object_case */


bool
mom_cemit_get_number (struct mom_cemitlocalstate_st *csta,
                      mo_value_t val, int64_t *pnum)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil != NULL,
                    "cemit_get_number: bad csta@%p", csta);
  mo_cemitpayl_ty *cemp = csta->mo_cemsta_payl;
  MOM_ASSERTPRINTF (cemp && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC
                    && cemp->mo_cemit_locstate == csta,
                    "cemit_get_number: bad payl@%p in csta@%p", cemp, csta);
  MOM_ASSERTPRINTF (pnum, "cemit_get_number: bad pnum");
  if (mo_value_is_int (val))
    {
      *pnum = mo_value_to_int (val, 0);
      return true;
    }
  mo_objref_t ob = mo_dyncast_objref (val);
  if (ob)
    {
      if (ob->mo_ob_class == MOM_PREDEF (enumerator_class))
        {
          mo_objref_t rolob =
            mo_dyncast_objref (mo_assoval_get
                               (csta->mo_cemsta_assocmodulrole, ob));
          if (rolob
              && mo_objref_comp_count (rolob) >= MOMROLENUMVIX__LASTENUMV)
            {
              mo_value_t rolv =
                mo_objref_get_comp (rolob, MOMROLENUMVIX_ENUMVAL);
              if (mo_value_is_int (rolv))
                {
                  *pnum = mo_value_to_int (rolv, 0);
                  return true;
                }
            }
        }
      return false;
    }
}                               /* end of mom_cemit_get_number */

#define MOM_CEMIT_MAX_NUMBER_CASE 2048
void
mom_cemit_scan_number_case (struct mom_cemitlocalstate_st *csta,
                            mo_objref_t numcasob, mo_objref_t insrolob,
                            mo_objref_t fromob, int depth)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil != NULL,
                    "cemit_scan_number_case: bad csta@%p", csta);
  mo_cemitpayl_ty *cemp = csta->mo_cemsta_payl;
  MOM_ASSERTPRINTF (cemp && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC
                    && cemp->mo_cemit_locstate == csta,
                    "cemit_scan_number_case: bad payl@%p in csta@%p", cemp,
                    csta);
  if (!mo_dyncast_objref (numcasob)
      || numcasob->mo_ob_class != MOM_PREDEF (c_number_case_class))
    MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA (csta, numcasob, insrolob, fromob),
                      "cemit_scan_number_case: bad numcasob %s insrolob %s,"
                      " fromob %s depth %d", mo_objref_pnamestr (numcasob),
                      mo_objref_pnamestr (insrolob),
                      mo_objref_pnamestr (fromob), depth);
  mo_objref_t bodyob =
    mo_dyncast_objref (mo_objref_get_attr (numcasob, MOM_PREDEF (body)));
  mo_objref_t labelob =
    mo_dyncast_objref (mo_objref_get_attr (numcasob, MOM_PREDEF (label)));
  if (!bodyob && !labelob)
    MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA (csta, numcasob, insrolob, fromob),
                      "cemit_scan_number_case: numcasob %s has no `body` or `label` insrolob %s,"
                      " fromob %s depth %d",
                      mo_objref_pnamestr (numcasob),
                      mo_objref_pnamestr (insrolob),
                      mo_objref_pnamestr (fromob), depth);
  if (bodyob && labelob)
    MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA (csta, numcasob, insrolob, fromob),
                      "cemit_scan_number_case: objcasob %s has both `body` or `label` insrolob %s,"
                      " fromob %s depth %d",
                      mo_objref_pnamestr (numcasob),
                      mo_objref_pnamestr (insrolob),
                      mo_objref_pnamestr (fromob), depth);
  mo_objref_t imapob =
    mo_dyncast_objref (mo_objref_get_comp (insrolob, MOMROLCASEIX_MAP));
  MOM_ASSERTPRINTF (mo_dyncast_objref (imapob)
                    && imapob->mo_ob_paylkind ==
                    MOM_PREDEF (payload_inthmap),
                    "cemit_scan_number_case: bad imapob %s",
                    mo_objref_pnamestr (imapob));
  mo_inthmappayl_ty *ihmap = mo_dyncastpayl_inthmap (imapob->mo_ob_payldata);
  MOM_ASSERTPRINTF (ihmap, "cemit_scan_number_case: bad ihmap in imapob=%s",
                    mo_objref_pnamestr (imapob));
  mo_value_t whenv = mo_objref_get_attr (numcasob, MOM_PREDEF (when));
  mo_objref_t rolcasob =
    mo_dyncast_objref (mo_assoval_get
                       (csta->mo_cemsta_assoclocalrole, numcasob));
  if (rolcasob)
    MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA
                      (csta, numcasob, rolcasob, insrolob, fromob),
                      "cemit_scan_number_case: numcasob %s has rolcasob %s insrolob %s,"
                      " fromob %s depth %d", mo_objref_pnamestr (numcasob),
                      mo_objref_pnamestr (rolcasob),
                      mo_objref_pnamestr (insrolob),
                      mo_objref_pnamestr (fromob), depth);
  rolcasob = mo_make_object ();
  rolcasob->mo_ob_class = MOM_PREDEF (c_role_class);
  mo_objref_comp_resize (rolcasob, MOMROLNUMCASIX__LAST);
  mo_objref_put_comp (rolcasob, MOMROLNUMCASIX_ROLE, momglob_int);
  mo_objref_put_comp (rolcasob, MOMROLNUMCASIX_INSTR, fromob);
  mo_objref_put_comp (rolcasob, MOMROLNUMCASIX_NUMBER, whenv);
  csta->mo_cemsta_assoclocalrole =
    mo_assoval_put (csta->mo_cemsta_assoclocalrole, numcasob, rolcasob);
  int64_t num = 0;
  int setwhensiz = 0;
  if (mom_cemit_get_number (csta, whenv, &num))
    {
      if (mo_inthmap_get (ihmap, num))
        MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA
                          (csta, numcasob, insrolob, fromob),
                          "cemit_scan_number_case: in numcasob %s duplicate num %lld insrolob %s,"
                          " fromob %s depth %d",
                          mo_objref_pnamestr (numcasob), (long long) num,
                          mo_objref_pnamestr (insrolob),
                          mo_objref_pnamestr (fromob), depth);
      ihmap = mo_inthmap_put (ihmap, num, numcasob);
    }
  else if ((setwhensiz = mo_set_size (whenv)) > 0)
    {
      for (int weix = 0; weix < setwhensiz; weix++)
        {
          mo_objref_t whelemob = mo_set_nth (whenv, weix);
          int64_t elemnum = 0;
          if (!mom_cemit_get_number (csta, whelemob, &elemnum))
            goto numbadwhen;
          if (mo_inthmap_get (ihmap, elemnum))
            MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA
                              (csta, numcasob, insrolob, fromob),
                              "cemit_scan_number_case: in numcasob %s duplicate num %lld insrolob %s,"
                              " fromob %s depth %d",
                              mo_objref_pnamestr (numcasob),
                              (long long) elemnum,
                              mo_objref_pnamestr (insrolob),
                              mo_objref_pnamestr (fromob), depth);
          ihmap = mo_inthmap_put (ihmap, elemnum, numcasob);
        }
    }
  else
  numbadwhen:
    MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA (csta, numcasob, insrolob, fromob),
                      "cemit_scan_number_case: in numcasob %s bad `when` %s insrolob %s,"
                      " fromob %s depth %d", mo_objref_pnamestr (numcasob),
                      mo_value_pnamestr (whenv),
                      mo_objref_pnamestr (insrolob),
                      mo_objref_pnamestr (fromob), depth);
  imapob->mo_ob_payldata = ihmap;
  if (mo_inthmap_count (ihmap) > MOM_CEMIT_MAX_NUMBER_CASE)
    MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA (csta, numcasob, insrolob, fromob),
                      "cemit_scan_number_case: too many %d cases after numcasob %s  `when` %s insrolob %s,"
                      " fromob %s depth %d",
                      mo_inthmap_count (ihmap),
                      mo_objref_pnamestr (numcasob),
                      mo_value_pnamestr (whenv),
                      mo_objref_pnamestr (insrolob),
                      mo_objref_pnamestr (fromob), depth);
  if (bodyob)
    {
      mo_objref_t blocksetob =
        mo_dyncast_objref (mo_objref_get_comp
                           (insrolob, MOMROLCASEIX_BLOCKSET));
      mo_hashsetpayl_ty *blset = NULL;
      if (MOM_UNLIKELY
          (!blocksetob
           || blocksetob->mo_ob_paylkind != MOM_PREDEF (payload_hashset)
           || !(blset = mo_dyncastpayl_hashset (blocksetob->mo_ob_payldata))))
        MOM_FATAPRINTF
          ("cemit_scan_number_case insrolob %s with bad blockset",
           mo_objref_pnamestr (insrolob));
      blset = mo_hashset_put (blset, bodyob);
      blocksetob->mo_ob_payldata = blset;
      mom_cemit_scan_block (csta, bodyob, numcasob, depth + 1);
    }
  else if (labelob)
    {
      if (labelob->mo_ob_class != MOM_PREDEF (c_block_class))
        MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA
                          (csta, numcasob, rolcasob, insrolob, fromob),
                          "cemit_scan_number_case: objcasob %s with bad labelob %s insrolob %s,"
                          " fromob %s depth %d",
                          mo_objref_pnamestr (numcasob),
                          mo_objref_pnamestr (labelob),
                          mo_objref_pnamestr (insrolob),
                          mo_objref_pnamestr (fromob), depth);
      csta->mo_cemsta_hsetjumpedblocks =
        mo_hashset_put (csta->mo_cemsta_hsetjumpedblocks, labelob);
    }
  else                          // should never happen
    MOM_FATAPRINTF
      ("cemit_scan_number_case numcasob=%s without body or label",
       mo_objref_pnamestr (numcasob));
}                               /* end of mom_cemit_scan_number_case */


void
mom_cemit_scan_case_instr (struct mom_cemitlocalstate_st *csta,
                           mo_objref_t instrob, mo_objref_t fromob, int depth)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil != NULL,
                    "cemit_scan_case_instr: bad csta@%p", csta);
  mo_cemitpayl_ty *cemp = csta->mo_cemsta_payl;
  MOM_ASSERTPRINTF (cemp && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC
                    && cemp->mo_cemit_locstate == csta,
                    "cemit_scan_case_instr: bad payl@%p in csta@%p", cemp,
                    csta);
  MOM_ASSERTPRINTF (mo_dyncast_objref (instrob)
                    && instrob->mo_ob_class ==
                    MOM_PREDEF (case_instruction_class),
                    "cemit_scan_case_instr: chunk bad instrob at depth %d from %s",
                    depth, mo_objref_pnamestr (instrob));
  int instrlen = mo_objref_comp_count (instrob);
  mo_objref_t rolob =
    mo_dyncast_objref (mo_assoval_get
                       (csta->mo_cemsta_assoclocalrole, instrob));
  if (rolob)
    MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA (csta, instrob, fromob, rolob),
                      "cemit_scan_case_instr: instr %s already with role %s, depth %d, from %s",
                      mo_objref_pnamestr (instrob),
                      mo_objref_pnamestr (rolob),
                      depth, mo_objref_pnamestr (fromob));
  rolob = mo_make_object ();
  rolob->mo_ob_class = MOM_PREDEF (c_role_class);
  mo_objref_comp_resize (rolob, MOMROLCASEIX__LAST);
  mo_objref_put_comp (rolob, MOMROLCASEIX_ROLE, MOM_PREDEF (case));
  csta->mo_cemsta_assoclocalrole =
    mo_assoval_put (csta->mo_cemsta_assoclocalrole, instrob, rolob);
  mo_value_t casexpv = mo_objref_get_attr (instrob, MOM_PREDEF (case));
  mo_objref_put_comp (rolob, MOMROLCASEIX_CASE, casexpv);
  mo_objref_t castypob =
    mom_cemit_scan_expression (csta, casexpv, instrob, 0);
  mo_objref_t instypob =
    mo_dyncast_objref (mo_objref_get_attr (instrob, MOM_PREDEF (c_type)));
  mo_objref_t typob = NULL;
  if (instypob)
    {
      typob = mom_cemit_compare_ctypes (csta, instypob, castypob, instrob);
      if (!typob)
        MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA
                          (csta, instrob, instypob, castypob, fromob),
                          "cemit_scan_case_instr: instr %s with incompatible types %s from case, %s in instr,"
                          " depth %d, from %s",
                          mo_objref_pnamestr (instrob),
                          mo_objref_pnamestr (castypob),
                          mo_objref_pnamestr (instypob), depth,
                          mo_objref_pnamestr (fromob));
    }
  else
    typob = castypob;
  if (!typob)
    MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA
                      (csta, instrob, instypob, castypob, fromob),
                      "cemit_scan_case_instr: instr %s without c_type,"
                      " depth %d, from %s",
                      mo_objref_pnamestr (instrob), depth,
                      mo_objref_pnamestr (fromob));

  mo_objref_put_comp (rolob, MOMROLCASEIX_CTYPE, typob);
  mo_objref_t blhsetob = mo_make_object ();
  mo_hashsetpayl_ty *blockhset = mo_hashset_reserve (NULL, 2 * instrlen + 5);
  blhsetob->mo_ob_paylkind = MOM_PREDEF (payload_hashset);
  blhsetob->mo_ob_payldata = blockhset;
  mo_objref_put_comp (rolob, MOMROLCASEIX_BLOCKSET, blhsetob);
  if (typob == momglob_object_ctype)
    {
      mo_objref_t amapob = mo_make_object ();
      mo_assovaldatapayl_ty *assoval =
        mo_assoval_reserve (NULL, instrlen * 2 + 5);
      amapob->mo_ob_paylkind = MOM_PREDEF (payload_assoval);
      amapob->mo_ob_payldata = assoval;
      mo_objref_put_comp (rolob, MOMROLCASEIX_MAP, amapob);
      for (unsigned ix = 0; ix < (unsigned) instrlen; ix++)
        {
          mom_cemit_scan_object_case (csta,
                                      mo_dyncast_objref (mo_objref_get_comp
                                                         (instrob, ix)),
                                      rolob, instrob, depth + 1);
        }
      assoval = amapob->mo_ob_payldata;
      if (mo_assoval_count (assoval) >= MOM_CEMIT_MAX_OBJECT_CASE)
        MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA
                          (csta, instrob, instypob, castypob, fromob),
                          "cemit_scan_case_instr: instr %s with too many %d objcases,"
                          " depth %d, from %s",
                          mo_objref_pnamestr (instrob),
                          mo_assoval_count (assoval), depth,
                          mo_objref_pnamestr (fromob));

    }
  else if (typob->mo_ob_class == MOM_PREDEF (enum_ctype_class)
           || typob == momglob_char || typob == momglob_long
           || typob == momglob_int || typob == momglob_intptr_t
           || typob == momglob_int8_t || typob == momglob_int16_t
           || typob == momglob_int32_t || typob == momglob_int64_t
           || typob == momglob_uintptr_t || typob == momglob_uint8_t
           || typob == momglob_uint16_t || typob == momglob_uint32_t
           || typob == momglob_uint64_t)
    {
      mo_objref_t imapob = mo_make_object ();
      mo_inthmappayl_ty *ihmap = mo_inthmap_reserve (NULL, instrlen * 3 + 8);
      imapob->mo_ob_paylkind = MOM_PREDEF (payload_inthmap);
      imapob->mo_ob_payldata = ihmap;
      mo_objref_put_comp (rolob, MOMROLCASEIX_MAP, imapob);
      for (unsigned ix = 0; ix < (unsigned) instrlen; ix++)
        {
          mom_cemit_scan_number_case (csta,
                                      mo_dyncast_objref (mo_objref_get_comp
                                                         (instrob, ix)),
                                      rolob, instrob, depth + 1);
        }
      ihmap = imapob->mo_ob_payldata;
      if (mo_inthmap_count (ihmap) >= MOM_CEMIT_MAX_NUMBER_CASE)
        MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA
                          (csta, instrob, instypob, castypob, fromob),
                          "cemit_scan_case_instr: instr %s with too many %d numcases,"
                          " depth %d, from %s",
                          mo_objref_pnamestr (instrob),
                          mo_inthmap_count (ihmap), depth,
                          mo_objref_pnamestr (fromob));
    }
  else
    MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA
                      (csta, instrob, typob, fromob),
                      "cemit_scan_case_instr: instr %s with bad c_type %s,"
                      " depth %d, from %s",
                      mo_objref_pnamestr (instrob),
                      mo_objref_pnamestr (typob),
                      depth, mo_objref_pnamestr (fromob));
}                               /* end mom_cemit_scan_case_instr */


void
mom_cemit_scan_instr (struct mom_cemitlocalstate_st *csta,
                      mo_objref_t instrob, mo_objref_t fromob, int depth)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil != NULL,
                    "cemit_scan_instr: bad csta@%p", csta);
  mo_cemitpayl_ty *cemp = csta->mo_cemsta_payl;
  MOM_ASSERTPRINTF (cemp && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC
                    && cemp->mo_cemit_locstate == csta,
                    "cemit_scan_instr: bad payl@%p in csta@%p", cemp, csta);
  MOM_ASSERTPRINTF (mo_dyncast_objref (instrob),
                    "cemit_scan_block: bad instrob at depth %d from %s",
                    depth, mo_objref_pnamestr (instrob));
  if (depth > MOM_CEMIT_MAX_DEPTH)
    MOM_CEMITFAILURE (csta,
                      "cemit_scan_instr: instr %s too deep %d from %s",
                      mo_objref_pnamestr (instrob), depth,
                      mo_objref_pnamestr (fromob));
  if (mom_elapsed_real_time () > csta->mo_cemsta_timelimit)
    MOM_CEMITFAILURE (csta,
                      "cemit_scan_instr: instr %s timed out, depth %d, from %s",
                      mo_objref_pnamestr (instrob), depth,
                      mo_objref_pnamestr (fromob));
  mo_objref_t insclassob = instrob->mo_ob_class;
  if (!insclassob)
    MOM_CEMITFAILURE (csta,
                      "cemit_scan_instr: instr %s without class, depth %d, from %s",
                      mo_objref_pnamestr (instrob), depth,
                      mo_objref_pnamestr (fromob));
#define MOM_NBCASE_INSCLASS 193
#define CASE_PREDEFINSCLASS_MOM(Ob) momphash_##Ob % MOM_NBCASE_INSCLASS: \
  if (insclassob != MOM_PREDEF(Ob))    					\
    goto defaultinsclasscase;						\
  goto labinsclass_##Ob;						\
  labinsclass_##Ob
  switch (mo_objref_hash (insclassob) % MOM_NBCASE_INSCLASS)
    {
    case CASE_PREDEFINSCLASS_MOM (c_block_class):
      {
        mom_cemit_scan_block (csta, instrob, fromob, depth + 1);
        return;
      }
    case CASE_PREDEFINSCLASS_MOM (macro_block_class):
      {
        mom_cemit_scan_block (csta, instrob, fromob, depth + 1);
        return;
      }
    case CASE_PREDEFINSCLASS_MOM (chunk_instruction_class):
      {
        mom_cemit_scan_chunk_instr (csta, instrob, fromob, depth);
        return;
      }
      break;
    case CASE_PREDEFINSCLASS_MOM (conditional_instruction_class):
      {
        mom_cemit_scan_cond_instr (csta, instrob, fromob, depth);
        return;
      }
      break;
    case CASE_PREDEFINSCLASS_MOM (assignment_instruction_class):
      {
        mom_cemit_scan_assign_instr (csta, instrob, fromob, depth);
        return;
      }
      break;
    case CASE_PREDEFINSCLASS_MOM (call_instruction_class):
      {
        mom_cemit_scan_call_instr (csta, instrob, fromob, depth);
        return;
      }
      break;
    case CASE_PREDEFINSCLASS_MOM (case_instruction_class):
      {
        mom_cemit_scan_case_instr (csta, instrob, fromob, depth);
        return;
      }
      break;
    case CASE_PREDEFINSCLASS_MOM (jump_instruction_class):
      {
        mom_cemit_scan_jump_instr (csta, instrob, fromob, depth);
        return;
      }
      break;
    defaultinsclasscase:
    default:
      MOM_CEMITFAILURE (csta,
                        "cemit_scan_instr: bad instr %s with %s class, depth %d, from %s",
                        mo_objref_pnamestr (instrob),
                        mo_objref_pnamestr (insclassob), depth,
                        mo_objref_pnamestr (fromob));
      break;
    }
}                               /* end of mom_cemit_scan_instr */

// a reference or l-value can be assigned to. It has some c-type. It
// translates in C to some l-value. The scan returns its c-type
mo_objref_t
mom_cemit_scan_reference (struct mom_cemitlocalstate_st *csta,
                          mo_objref_t refob, mo_objref_t fromob, int depth)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil != NULL,
                    "cemit_scan_reference: bad csta@%p", csta);
  mo_cemitpayl_ty *cemp = csta->mo_cemsta_payl;
  MOM_ASSERTPRINTF (cemp && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC
                    && cemp->mo_cemit_locstate == csta,
                    "cemit_scan_reference: bad payl@%p in csta@%p", cemp,
                    csta);
  if (!mo_dyncast_objref (refob))
    MOM_CEMITFAILURE (csta,
                      "cemit_scan_reference: no refob from %s depth %d",
                      mo_objref_pnamestr (fromob), depth);
  if (depth > MOM_CEMIT_MAX_DEPTH)
    MOM_CEMITFAILURE (csta,
                      "cemit_scan_reference: refob %s too deep %d from %s",
                      mo_objref_pnamestr (refob), depth,
                      mo_objref_pnamestr (fromob));
  if (mom_elapsed_real_time () > csta->mo_cemsta_timelimit)
    MOM_CEMITFAILURE (csta,
                      "cemit_scan_reference: refob %s timed out, depth %d, from %s",
                      mo_objref_pnamestr (refob), depth,
                      mo_objref_pnamestr (fromob));
  if (refob->mo_ob_class == MOM_PREDEF (macro_expression_class))
    return mom_cemit_scan_macro_expr (csta, refob, fromob, depth,       //
                                      /*isref: */ true);
  else if (refob->mo_ob_class == MOM_PREDEF (chunk_expression_class))
    return mom_cemit_scan_chunk_expr (csta, refob, fromob, depth,       //
                                      /*isref: */ true);
  else if (refob->mo_ob_class == MOM_PREDEF (c_variable_class))
    {
      // temporary
      mo_objref_t rolob =
        mo_dyncast_objref (mo_assoval_get
                           (csta->mo_cemsta_assoclocalrole, refob));
      if (!rolob)
        MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA
                          (csta, refob, mo_int_to_value (depth), fromob),
                          "cemit_scan_reference: refob %s c-var without role,  depth %d, from %s",
                          mo_objref_pnamestr (refob), depth,
                          mo_objref_pnamestr (fromob));
      if (rolob->mo_ob_class != MOM_PREDEF (c_role_class))
        MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA
                          (csta, refob, mo_int_to_value (depth), fromob,
                           rolob),
                          "cemit_scan_reference: refob %s c-var, depth %d, from %s, with bad rolob %s",
                          mo_objref_pnamestr (refob), depth,
                          mo_objref_pnamestr (fromob),
                          mo_objref_pnamestr (rolob));
      mo_objref_t kindrolob =
        mo_dyncast_objref (mo_objref_get_comp (rolob, MOMROLVARIX_ROLE));
      mo_objref_t typrolob =
        mo_dyncast_objref (mo_objref_get_comp (rolob, MOMROLVARIX_CTYPE));
      if (kindrolob == MOM_PREDEF (data) || kindrolob == MOM_PREDEF (result)
          || kindrolob == MOM_PREDEF (locals)
          || kindrolob == MOM_PREDEF (formals))
        {
          MOM_ASSERTPRINTF (mom_cemit_is_ctype (csta, typrolob),
                            "cemit_scan_reference: c-var refob %s of role %s has bad type %s",
                            mo_objref_pnamestr (refob),
                            mo_objref_pnamestr (rolob),
                            mo_objref_pnamestr (typrolob));
          return typrolob;
        }
      // this should probably never happen
      MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA
                        (csta, refob, mo_int_to_value (depth), fromob,
                         rolob),
                        "cemit_scan_reference: refob %s unexpected c-var, depth %d, from %s, rolob %s",
                        mo_objref_pnamestr (refob), depth,
                        mo_objref_pnamestr (fromob),
                        mo_objref_pnamestr (rolob));
    }
  else if (refob->mo_ob_class == MOM_PREDEF (member_access_class))
    return mom_cemit_scan_member_access (csta, refob, fromob, depth);
  else
    MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA
                      (csta, refob, mo_int_to_value (depth), fromob),
                      "cemit_scan_reference: refob %s of unexpected class %s, depth %d, from %s",
                      mo_objref_pnamestr (refob),
                      mo_objref_pnamestr (refob->mo_ob_class), depth,
                      mo_objref_pnamestr (fromob));
}                               /* end of mom_cemit_scan_reference */




// an expression or r-value can be computed. It has some c-type. It translates in C to some r-value.
mo_objref_t
mom_cemit_scan_expression (struct mom_cemitlocalstate_st *csta,
                           mo_value_t expv, mo_objref_t fromob, int depth)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil != NULL,
                    "cemit_scan_expression: bad csta@%p", csta);
  mo_cemitpayl_ty *cemp = csta->mo_cemsta_payl;
  MOM_ASSERTPRINTF (cemp && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC
                    && cemp->mo_cemit_locstate == csta,
                    "cemit_scan_expression: bad payl@%p in csta@%p", cemp,
                    csta);
  if (!expv)
    return MOM_PREDEF (null_ctype);
  if (depth > MOM_CEMIT_MAX_DEPTH)
    MOM_CEMITFAILURE (csta,
                      "cemit_scan_expression: expr %s too deep %d from %s",
                      mo_value_pnamestr (expv), depth,
                      mo_objref_pnamestr (fromob));
  if (mom_elapsed_real_time () > csta->mo_cemsta_timelimit)
    MOM_CEMITFAILURE (csta,
                      "cemit_scan_expression: expr %s timed out, depth %d, from %s",
                      mo_value_pnamestr (expv), depth,
                      mo_objref_pnamestr (fromob));
  enum mo_valkind_en kv = mo_kind_of_value (expv);
  switch (kv)
    {
    case mo_KNONE:
      MOM_CEMITFAILURE (csta,
                        "cemit_scan_expression: missing expression %s from %s",
                        mo_value_pnamestr (expv),
                        mo_objref_pnamestr (fromob));
    case mo_KINT:
      return momglob_int;
    case mo_KSTRING:
      return momglob_string;
    case mo_KTUPLE:
      goto setkindv;
    case mo_KSET:
    setkindv:
      MOM_CEMITFAILURE (csta,
                        "cemit_scan_expression: bad sequence expression %s from %s",
                        mo_value_pnamestr (expv),
                        mo_objref_pnamestr (fromob));
    case mo_KOBJECT:
      {
        mo_objref_t expob = mo_dyncast_objref (expv);
        MOM_ASSERTPRINTF (expob != NULL,
                          "cemit_scan_expression: bad expob from %s",
                          mo_objref_pnamestr (fromob));
        if (mo_set_contains (csta->mo_cemsta_objset, expob))
          return MOM_PREDEF (object);
        // handle locally bound objects
        {
          mo_objref_t locrolob =
            mo_dyncast_objref (mo_assoval_get
                               (csta->mo_cemsta_assoclocalrole, expob));
          if (locrolob)
            {
              MOM_ASSERTPRINTF (locrolob->mo_ob_class ==
                                MOM_PREDEF (c_role_class),
                                "cemit_scan_expression: bad locrolob %s for expob %s",
                                mo_objref_pnamestr (locrolob),
                                mo_objref_pnamestr (expob));
              mo_objref_t kindrolob =
                mo_dyncast_objref (mo_objref_get_comp
                                   (locrolob, MOMROLVARIX_ROLE));
              if (kindrolob == MOM_PREDEF (locals)
                  || kindrolob == MOM_PREDEF (formals)
                  || kindrolob == MOM_PREDEF (result))
                return
                  mo_dyncast_objref (mo_objref_get_comp
                                     (locrolob, MOMROLVARIX_CTYPE));
            }
        }
        // handle module bound objects
        {
          mo_objref_t modrolob =
            mo_dyncast_objref (mo_assoval_get
                               (csta->mo_cemsta_assocmodulrole, expob));
          if (modrolob)
            {
              MOM_ASSERTPRINTF (modrolob->mo_ob_class ==
                                MOM_PREDEF (c_role_class),
                                "cemit_scan_expression: bad modrolob %s for expob %s",
                                mo_objref_pnamestr (modrolob),
                                mo_objref_pnamestr (expob));
              mo_objref_t kindrolob =
                mo_dyncast_objref (mo_objref_get_comp
                                   (modrolob, MOMROLFUNCIX_ROLE));
              if (kindrolob == MOM_PREDEF (code))
                /// it is a signature
                return
                  mo_dyncast_objref (mo_objref_get_comp
                                     (modrolob, MOMROLFUNCIX_SIGNATURE));
              /// it is a ctype
              else if (kindrolob == MOM_PREDEF (data))
                return
                  mo_dyncast_objref (mo_objref_get_comp
                                     (modrolob, MOMROLVARIX_CTYPE));
              /// it is an enumerator
              else if (kindrolob == MOM_PREDEF (enumerators))
                return
                  mo_dyncast_objref (mo_objref_get_comp
                                     (modrolob, MOMROLENUMVIX_ENUMTYPE));
            }
        }
        // handle verbatim
        if (expob->mo_ob_class == MOM_PREDEF (verbatim_expression_class))
          {
            mo_objref_t verbob =
              mo_dyncast_objref (mo_objref_get_attr
                                 (expob, MOM_PREDEF (verbatim)));
            if (!verbob)
              MOM_CEMITFAILURE (csta,
                                "cemit_scan_expression: expr %s without verbatim, depth %d, from %s",
                                mo_objref_pnamestr (expob), depth,
                                mo_objref_pnamestr (fromob));
            if (!mo_set_contains (csta->mo_cemsta_objset, verbob)
                || mo_objref_space (verbob) != mo_SPACE_PREDEF)
              MOM_CEMITFAILURE (csta,
                                "cemit_scan_expression: expr %s with bad verbatim %s, depth %d, from %s",
                                mo_objref_pnamestr (expob),
                                mo_objref_pnamestr (verbob),
                                depth, mo_objref_pnamestr (fromob));
            return MOM_PREDEF (object);
          }
        else if (expob->mo_ob_class == MOM_PREDEF (member_access_class))
          return mom_cemit_scan_member_access (csta, expob, fromob, depth);
        else if (expob->mo_ob_class == MOM_PREDEF (macro_expression_class))
          return mom_cemit_scan_macro_expr (csta, expob, fromob, depth, //
                                            /*isref: */ false);
        else if (expob->mo_ob_class == MOM_PREDEF (chunk_expression_class))
          return mom_cemit_scan_chunk_expr (csta, expob, fromob, depth, //
                                            /*isref: */ false);
        else if (expob->mo_ob_class == MOM_PREDEF (cast_expression_class))
          return mom_cemit_scan_cast_expr (csta, expob, fromob, depth);
        else
          MOM_CEMITFAILURE (csta,
                            "cemit_scan_expression: bad expr %s, depth %d, from %s",
                            mo_objref_pnamestr (expob), depth,
                            mo_objref_pnamestr (fromob));
      }
    }
  MOM_CEMITFAILURE
    (csta, "cemit_scan_expression: unexpected expr %s, depth %d, from %s",
     mo_value_pnamestr (expv), depth, mo_objref_pnamestr (fromob));
}                               /* end of mom_cemit_scan_expression */


mo_objref_t
mom_cemit_scan_member_access (struct mom_cemitlocalstate_st *csta,
                              mo_objref_t accob, mo_objref_t fromob,
                              int depth)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil != NULL,
                    "cemit_scan_member_access: bad csta@%p", csta);
  mo_cemitpayl_ty *cemp = csta->mo_cemsta_payl;
  MOM_ASSERTPRINTF (cemp && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC
                    && cemp->mo_cemit_locstate == csta,
                    "cemit_scan_member_access: bad payl@%p in csta@%p",
                    cemp, csta);
  if (depth > MOM_CEMIT_MAX_DEPTH)
    MOM_CEMITFAILURE (csta,
                      "cemit_scan_member_access: expr %s too deep %d from %s",
                      mo_objref_pnamestr (accob), depth,
                      mo_objref_pnamestr (fromob));
  if (mom_elapsed_real_time () > csta->mo_cemsta_timelimit)
    MOM_CEMITFAILURE (csta,
                      "cemit_scan_member_access: expr %s timed out, depth %d, from %s",
                      mo_objref_pnamestr (accob), depth,
                      mo_objref_pnamestr (fromob));
  if (!mo_dyncast_objref(accob) || accob->mo_ob_class != MOM_PREDEF(member_access_class))
    MOM_CEMITFAILURE (csta,
                      "cemit_scan_member_access: expr %s of bad class %s, depth %d, from %s",
                      mo_objref_pnamestr (accob),
		      mo_dyncast_objref(accob)?mo_objref_pnamestr(accob->mo_ob_class):"??", depth,
                      mo_objref_pnamestr (fromob));
  mo_value_t fromexpv = mo_objref_get_attr(accob, MOM_PREDEF(from));
  mo_objref_t fieldob = mo_dyncast_objref(mo_objref_get_attr(accob, MOM_PREDEF(field)));
#warning mom_cemit_scan_member_access unimplemented
  MOM_FATAPRINTF ("mom_cemit_scan_member_access unimplemented accob=%s",
                  mo_objref_pnamestr (accob));
}                               /* end of mom_cemit_scan_member_access */



mo_objref_t
mom_cemit_scan_macro_expr (struct mom_cemitlocalstate_st *csta,
                           mo_objref_t macob, mo_objref_t fromob,
                           int depth, bool isref)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil != NULL,
                    "cemit_scan_macro_expr: bad csta@%p", csta);
  mo_cemitpayl_ty *cemp = csta->mo_cemsta_payl;
  MOM_ASSERTPRINTF (cemp && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC
                    && cemp->mo_cemit_locstate == csta,
                    "cemit_scan_macro_expr: bad payl@%p in csta@%p", cemp,
                    csta);
  if (depth > MOM_CEMIT_MAX_DEPTH)
    MOM_CEMITFAILURE (csta,
                      "cemit_scan_macro_expr: expr %s too deep %d from %s",
                      mo_objref_pnamestr (macob), depth,
                      mo_objref_pnamestr (fromob));
  if (mom_elapsed_real_time () > csta->mo_cemsta_timelimit)
    MOM_CEMITFAILURE (csta,
                      "cemit_scan_macro_expr: expr %s timed out, depth %d, from %s",
                      mo_objref_pnamestr (macob), depth,
                      mo_objref_pnamestr (fromob));
  MOM_ASSERTPRINTF (mo_dyncast_objref (macob)
                    && macob->mo_ob_class ==
                    MOM_PREDEF (macro_expression_class),
                    "cemit_scan_macro_expr: bad macob");
  mo_objref_t macrob =
    mo_dyncast_objref (mo_objref_get_attr (macob, MOM_PREDEF (macro)));
  if (!macrob)
    MOM_CEMITFAILURE (csta,
                      "cemit_scan_macro_expr: expr %s without macro, depth %d, from %s",
                      mo_objref_pnamestr (macob), depth,
                      mo_objref_pnamestr (fromob));
  if (macrob->mo_ob_paylkind !=
      MOM_PREDEF (signature_two_objects_to_value) || !macrob->mo_ob_payldata)
    MOM_CEMITFAILURE (csta,
                      "cemit_scan_macro_expr: expr %s with bad macro %s, depth %d, from %s",
                      mo_objref_pnamestr (macob),
                      mo_objref_pnamestr (macrob), depth,
                      mo_objref_pnamestr (fromob));
  mo_objref_t objcemit = csta->mo_cemsta_objcemit;
  mo_signature_two_objects_to_value_sigt *funrout = macrob->mo_ob_payldata;
  MOM_ASSERTPRINTF ((void *) funrout != NULL
                    && (void *) funrout != MOM_EMPTY_SLOT,
                    "cemit_scan_macro_expr: macro expr %s with macro %s bad funrout, depth %d, from %s",
                    mo_objref_pnamestr (macob),
                    mo_objref_pnamestr (macrob), depth,
                    mo_objref_pnamestr (fromob));
  mo_value_t resmacrv = (*funrout) (macob, objcemit);
  if (isref)
    {
      mo_objref_t resmacob = mo_dyncast_objref (resmacrv);
      if (!resmacob)
        MOM_CEMITFAILURE (csta,
                          "cemit_scan_macro_expr: reference %s macroexpanded to non-object %s, depth %d, from %s",
                          mo_objref_pnamestr (macob),
                          mo_value_pnamestr (resmacrv), depth,
                          mo_objref_pnamestr (fromob));
      return mom_cemit_scan_reference (csta, resmacob, macob, depth + 1);
    }
  else
    return mom_cemit_scan_expression (csta, resmacrv, macob, depth + 1);
}                               /* end of mom_cemit_scan_macro_expr */



// a cast expression 
mo_objref_t
mom_cemit_scan_cast_expr (struct mom_cemitlocalstate_st * csta,
                          mo_objref_t castob, mo_objref_t fromob, int depth)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil != NULL,
                    "cemit_scan_cast_expr: bad csta@%p", csta);
  mo_cemitpayl_ty *cemp = csta->mo_cemsta_payl;
  MOM_ASSERTPRINTF (cemp && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC
                    && cemp->mo_cemit_locstate == csta,
                    "cemit_scan_cast_expr: bad payl@%p in csta@%p", cemp,
                    csta);
  if (depth > MOM_CEMIT_MAX_DEPTH)
    MOM_CEMITFAILURE (csta,
                      "cemit_scan_cast_expr: expr %s too deep %d from %s",
                      mo_objref_pnamestr (castob), depth,
                      mo_objref_pnamestr (fromob));
  if (mom_elapsed_real_time () > csta->mo_cemsta_timelimit)
    MOM_CEMITFAILURE (csta,
                      "cemit_scan_cast_expr: expr %s timed out, depth %d, from %s",
                      mo_objref_pnamestr (castob), depth,
                      mo_objref_pnamestr (fromob));
  MOM_ASSERTPRINTF (mo_dyncast_objref (castob)
                    && castob->mo_ob_class ==
                    MOM_PREDEF (cast_expression_class),
                    "cemit_scan_cast_expr: bad castob %s",
                    mo_objref_pnamestr (castob));
  mo_value_t expv = mo_objref_get_attr (castob, MOM_PREDEF (expression));
  // there is no point of casting NIL, so...
  if (!expv)
    MOM_CEMITFAILURE (csta,
                      "cemit_scan_cast_expr: cast expr %s without sub-`expression`, depth %d, from %s",
                      mo_objref_pnamestr (castob), depth,
                      mo_objref_pnamestr (fromob));
  mo_objref_t ctypob =
    mo_dyncast_objref (mo_objref_get_attr (castob, MOM_PREDEF (c_type)));
  if (!ctypob)
    MOM_CEMITFAILURE (csta,
                      "cemit_scan_cast_expr: cast expr %s without `c_type`, depth %d, from %s",
                      mo_objref_pnamestr (castob), depth,
                      mo_objref_pnamestr (fromob));
  if (!mo_hashset_contains (csta->mo_cemsta_hsetctypes, ctypob))
    MOM_CEMITFAILURE (csta,
                      "cemit_scan_cast_expr: cast expr %s with unknown c_type %s, depth %d, from %s",
                      mo_objref_pnamestr (castob),
                      mo_objref_pnamestr (ctypob), depth,
                      mo_objref_pnamestr (fromob));
#warning mom_cemit_scan_cast_expr unimplemented
  MOM_FATAPRINTF ("cemit_scan_cast_expr: unimplemented castob=%s",
                  mo_objref_pnamestr (castob));
}                               /* end of mom_cemit_scan_cast_expr */


mo_objref_t
mom_cemit_scan_chunk_expr (struct mom_cemitlocalstate_st *csta,
                           mo_objref_t chkob, mo_objref_t fromob,
                           int depth, bool isref)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil != NULL,
                    "cemit_scan_chunk_expr: bad csta@%p", csta);
  mo_cemitpayl_ty *cemp = csta->mo_cemsta_payl;
  MOM_ASSERTPRINTF (cemp && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC
                    && cemp->mo_cemit_locstate == csta,
                    "cemit_scan_chunk_expr: bad payl@%p in csta@%p", cemp,
                    csta);
  if (depth > MOM_CEMIT_MAX_DEPTH)
    MOM_CEMITFAILURE (csta,
                      "cemit_scan_chunk_expr: expr %s too deep %d from %s",
                      mo_objref_pnamestr (chkob), depth,
                      mo_objref_pnamestr (fromob));
  if (mom_elapsed_real_time () > csta->mo_cemsta_timelimit)
    MOM_CEMITFAILURE (csta,
                      "cemit_scan_chunk_expr: expr %s timed out, depth %d, from %s",
                      mo_objref_pnamestr (chkob), depth,
                      mo_objref_pnamestr (fromob));
  MOM_ASSERTPRINTF (mo_dyncast_objref (chkob)
                    && chkob->mo_ob_class ==
                    MOM_PREDEF (chunk_expression_class),
                    "cemit_scan_chunk_expr: bad chkob %s",
                    mo_objref_pnamestr (chkob));
  mo_value_t verbatimv = mo_objref_get_attr (chkob, MOM_PREDEF (verbatim));
  if (verbatimv && !mo_dyncast_set (verbatimv))
    MOM_CEMITFAILURE (csta,
                      "cemit_scan_chunk_expr: expr %s with non-set verbatim %s, depth %d, from %s",
                      mo_objref_pnamestr (chkob),
                      mo_value_pnamestr (verbatimv), depth,
                      mo_objref_pnamestr (fromob));
  mo_objref_t refob =
    mo_dyncast_objref (mo_objref_get_attr (chkob, MOM_PREDEF (reference)));
  mo_objref_t valob =
    mo_dyncast_objref (mo_objref_get_attr (chkob, MOM_PREDEF (value)));
  if (isref && !refob)
    MOM_CEMITFAILURE (csta,
                      "cemit_scan_chunk_expr: reference chunk %s without `reference`, depth %d, from %s",
                      mo_objref_pnamestr (chkob), depth,
                      mo_objref_pnamestr (fromob));
  if (!isref && !refob && !valob)
    MOM_CEMITFAILURE (csta,
                      "cemit_scan_chunk_expr: chunk expression %s without `reference` or `value` ctype, depth %d, from %s",
                      mo_objref_pnamestr (chkob), depth,
                      mo_objref_pnamestr (fromob));
  unsigned nbcomp = mo_objref_comp_count (chkob);
  for (unsigned ix = 0; ix < nbcomp; ix++)
    {
      mo_value_t curcompv = mo_objref_get_comp (chkob, ix);
      mo_objref_t curcompob = mo_dyncast_objref (curcompv);
      if (curcompob && mo_set_contains (verbatimv, curcompob))
        continue;
      (void) mom_cemit_scan_expression (csta, curcompv, chkob, depth + 1);
    }
  if (isref)
    return refob;
  if (refob)
    return refob;
  if (valob && !isref)
    return valob;
  MOM_CEMITFAILURE (csta,
                    "cemit_scan_chunk_expr: chunk expression %s untyped, depth %d, from %s",
                    mo_objref_pnamestr (chkob), depth,
                    mo_objref_pnamestr (fromob));
}                               /* end of mom_cemit_scan_chunk_expr */



void
mom_cemit_write_block (struct mom_cemitlocalstate_st *csta,
                       mo_objref_t blockob, mo_objref_t fromob, int depth)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil != NULL,
                    "cemit_write_block: bad csta@%p", csta);
  mo_cemitpayl_ty *cemp = csta->mo_cemsta_payl;
  MOM_ASSERTPRINTF (cemp && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC
                    && cemp->mo_cemit_locstate == csta,
                    "cemit_write_block: bad payl@%p in csta@%p", cemp, csta);
  mo_objref_t rolob =
    mo_dyncast_objref (mo_assoval_get
                       (csta->mo_cemsta_assoclocalrole, blockob));
  if (!rolob || rolob->mo_ob_class != MOM_PREDEF (c_role_class)
      || mo_objref_get_comp (rolob, MOMROLBLOCKIX_ROLE) != MOM_PREDEF (body)
      || mo_objref_comp_count (rolob) < MOMROLBLOCKIX__LASTVAR)
    MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA
                      (csta, blockob, fromob, mo_int_to_value (depth),
                       rolob),
                      "mom_cemit_write_block blockob=%s fromob=%s depth=%d with bad rolob=%s",
                      mo_objref_pnamestr (blockob),
                      mo_objref_pnamestr (fromob), depth,
                      mo_objref_pnamestr (rolob));
  char blockid[MOM_CSTRIDSIZ];
  memset (blockid, 0, sizeof (blockid));
  mo_objref_idstr (blockid, blockob);
  mo_value_t localsv =
    mo_dyncast_sequence (mo_objref_get_comp (rolob, MOMROLBLOCKIX_LOCALS));
  int nbloc = mo_sequence_size (localsv);
  unsigned oldindent = csta->mo_cemsta_indentation;
  csta->mo_cemsta_indentation = depth;
  mom_cemit_newline (csta);
  if (mo_hashset_contains (csta->mo_cemsta_hsetjumpedblocks, blockob))
    mom_cemit_printf (csta,
                      MOM_LABEL_PREFIX
                      "%s: { // jumped block %s with %d locals", blockid,
                      mo_objref_pnamestr (blockob), nbloc);
  else
    mom_cemit_printf (csta, "{ // block %s with %d locals",
                      mo_objref_pnamestr (blockob), nbloc);
  csta->mo_cemsta_indentation = depth + 1;
  // emit local declarations
  for (int lix = 0; lix < nbloc; lix++)
    {
      mom_cemit_newline (csta);
      mo_objref_t curlocob = mo_sequence_nth (localsv, lix);
      char idcurloc[MOM_CSTRIDSIZ];
      memset (idcurloc, 0, sizeof (idcurloc));
      mo_objref_idstr (idcurloc, curlocob);
      mo_objref_t locrolob =
        mo_dyncast_objref (mo_assoval_get
                           (csta->mo_cemsta_assoclocalrole, curlocob));
      if (!locrolob || locrolob->mo_ob_class != MOM_PREDEF (c_role_class)
          || mo_objref_get_comp (locrolob,
                                 MOMROLVARIX_ROLE) != MOM_PREDEF (locals)
          || mo_objref_comp_count (locrolob) < MOMROLVARIX__LASTVAR)
        MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA
                          (csta, blockob, fromob, mo_int_to_value (depth),
                           curlocob, locrolob),
                          "mom_cemit_write_block blockob=%s fromob=%s depth=%d curlocob=%s with bad locrolob=%s",
                          mo_objref_pnamestr (blockob),
                          mo_objref_pnamestr (fromob), depth,
                          mo_objref_pnamestr (curlocob),
                          mo_objref_pnamestr (locrolob));
      mo_objref_t loctypob =
        mo_dyncast_objref (mo_objref_get_comp (locrolob, MOMROLVARIX_CTYPE));
      MOM_ASSERTPRINTF (mom_cemit_is_ctype (csta, loctypob),
                        "bad loctypob=%s in curlocob=%s for blockob=%s",
                        mo_objref_pnamestr (loctypob),
                        mo_objref_pnamestr (curlocob),
                        mo_objref_pnamestr (blockob));
      const char *locnamstr = mom_gc_printf (MOM_LOCAL_PREFIX "%s",
                                             mo_objref_shortnamestr
                                             (curlocob));
      mom_cemit_write_ctype_for (csta, loctypob, locnamstr, depth);
      if (mom_cemit_ctype_is_scalar (csta, loctypob))
        mom_cemit_printf (csta, " = /*scalar local*/0; // %s", idcurloc);
      else
        mom_cemit_printf (csta, " = /*aggregate local*/{}; // %s", idcurloc);
    }
  // emit instructions
  mo_value_t instrstup =
    mo_dyncast_tuple (mo_objref_get_comp (rolob, MOMROLBLOCKIX_INSTRS));
  int nbinstrs = mo_tuple_size (instrstup);
  mom_cemit_newline (csta);
  mom_cemit_printf (csta, "// %d instrs in block %s", nbinstrs,
                    mo_objref_pnamestr (blockob));
  for (int insix = 0; insix < nbinstrs; insix++)
    {
      mo_objref_t curinsob = mo_tuple_nth (instrstup, insix);
      mom_cemit_newline (csta);
      mom_cemit_write_instr (csta, curinsob, blockob, depth);
    }
  csta->mo_cemsta_indentation = depth;
  mom_cemit_newline (csta);
  mom_cemit_printf (csta, "} // end block %s", blockid);
  csta->mo_cemsta_indentation = oldindent;
  mom_cemit_newline (csta);
}                               /* end of mom_cemit_write_block */


void
mom_cemit_write_expression (struct mom_cemitlocalstate_st *csta,
                            mo_value_t expv, mo_objref_t fromob, int depth);

void
mom_cemit_write_reference (struct mom_cemitlocalstate_st *csta,
                           mo_objref_t refob, mo_objref_t fromob, int depth);

void
mom_cemit_write_assign_instr (struct mom_cemitlocalstate_st *csta,
                              mo_objref_t instrob, mo_objref_t fromob,
                              mo_objref_t rolinsob, int depth);

void
mom_cemit_write_chunk_instr (struct mom_cemitlocalstate_st *csta,
                             mo_objref_t instrob, mo_objref_t fromob,
                             mo_objref_t rolinsob, int depth);

void
mom_cemit_write_jump_instr (struct mom_cemitlocalstate_st *csta,
                            mo_objref_t instrob, mo_objref_t fromob,
                            mo_objref_t rolinsob, int depth);

void
mom_cemit_write_cond_instr (struct mom_cemitlocalstate_st *csta,
                            mo_objref_t instrob, mo_objref_t fromob,
                            mo_objref_t rolinsob, int depth);

void
mom_cemit_write_call_instr (struct mom_cemitlocalstate_st *csta,
                            mo_objref_t instrob, mo_objref_t fromob,
                            mo_objref_t rolinsob, int depth);

void
mom_cemit_write_instr (struct mom_cemitlocalstate_st *csta,
                       mo_objref_t instrob, mo_objref_t fromob, int depth)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil != NULL,
                    "cemit_write_instr: bad csta@%p", csta);
  mo_cemitpayl_ty *cemp = csta->mo_cemsta_payl;
  MOM_ASSERTPRINTF (cemp && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC
                    && cemp->mo_cemit_locstate == csta,
                    "cemit_write_instr: bad payl@%p in csta@%p", cemp, csta);
  mo_objref_t rolob =
    mo_dyncast_objref (mo_assoval_get
                       (csta->mo_cemsta_assoclocalrole, instrob));
  mo_objref_t rolkindob = NULL;
  if (!rolob || rolob->mo_ob_class != MOM_PREDEF (c_role_class)
      || !(rolkindob = mo_dyncast_objref (mo_objref_get_comp (rolob, 0))))
    MOM_CEMITFAILURE
      (MOM_CEMIT_ADD_DATA
       (csta, instrob, fromob, mo_int_to_value (depth), rolob),
       "mom_cemit_write_instr instrob=%s fromob=%s depth=%d with bad rolob=%s",
       mo_objref_pnamestr (instrob), mo_objref_pnamestr (fromob), depth,
       mo_objref_pnamestr (rolob));
#define MOM_NBCASE_INSTR 179
#define CASE_PREDEFINSTR_MOM(Ob)  momphash_##Ob % MOM_NBCASE_INSTR:	\
  if (rolkindob != MOM_PREDEF(Ob))					\
    goto defaultinstrcase;						\
  goto labinstr_##Ob;							\
  labinstr_##Ob
  switch (mo_objref_hash (rolkindob) % MOM_NBCASE_INSTR)
    {
    case CASE_PREDEFINSTR_MOM (assign):
      mom_cemit_write_assign_instr (csta, instrob, fromob, rolob, depth);
      return;
    case CASE_PREDEFINSTR_MOM (chunk):
      mom_cemit_write_chunk_instr (csta, instrob, fromob, rolob, depth);
      return;
    case CASE_PREDEFINSTR_MOM (to):
      mom_cemit_write_jump_instr (csta, instrob, fromob, rolob, depth);
      return;
    case CASE_PREDEFINSTR_MOM (conditional):
      mom_cemit_write_cond_instr (csta, instrob, fromob, rolob, depth);
      return;
    case CASE_PREDEFINSTR_MOM (call):
      mom_cemit_write_call_instr (csta, instrob, fromob, rolob, depth);
      return;
    defaultinstrcase:
    default:
      break;
    }
#undef MOM_NBCASE_INSTR
#undef CASE_PREDEFINSTR_MOM
#warning mom_cemit_write_instr unimplemented
  MOM_CEMITFAILURE
    (MOM_CEMIT_ADD_DATA
     (csta, instrob, fromob, mo_int_to_value (depth), rolob),
     "mom_cemit_write_instr unimplemented instrob=%s fromob=%s depth=%d rolob=%s rolkindob=%s",
     mo_objref_pnamestr (instrob), mo_objref_pnamestr (fromob), depth,
     mo_objref_pnamestr (rolob), mo_objref_pnamestr (rolkindob));
}                               /* end mom_cemit_write_instr */



void
mom_cemit_write_assign_instr (struct mom_cemitlocalstate_st *csta,
                              mo_objref_t instrob, mo_objref_t fromob,
                              mo_objref_t rolinsob, int depth)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil != NULL,
                    "cemit_write_assign_instr: bad csta@%p", csta);
  mo_cemitpayl_ty *cemp = csta->mo_cemsta_payl;
  MOM_ASSERTPRINTF (cemp && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC
                    && cemp->mo_cemit_locstate == csta,
                    "cemit_write_assign_instr: bad payl@%p in csta@%p",
                    cemp, csta);
  MOM_ASSERTPRINTF (mo_dyncast_objref (instrob)
                    && instrob->mo_ob_class ==
                    MOM_PREDEF (assignment_instruction_class),
                    "cemit_write_assign_instr: bad instrob");
  MOM_ASSERTPRINTF (mo_dyncast_objref (fromob),
                    "cemit_write_assign_instr: bad fromob");
  MOM_ASSERTPRINTF (mo_dyncast_objref (rolinsob)
                    && rolinsob->mo_ob_class == MOM_PREDEF (c_role_class)
                    && mo_objref_get_comp (rolinsob,
                                           MOMROLASSIGNIX_ROLE) ==
                    MOM_PREDEF (assign)
                    && mo_objref_comp_count (rolinsob) >=
                    MOMROLASSIGNIX__LASTASSIGN,
                    "cemit_write_assign_instr: bad rolinsob %s",
                    mo_objref_pnamestr (rolinsob));
  mo_objref_t totypob =
    mo_dyncast_objref (mo_objref_get_comp (rolinsob, MOMROLASSIGNIX_TOTYPE));
  mo_objref_t torefob =
    mo_dyncast_objref (mo_objref_get_comp
                       (rolinsob, MOMROLASSIGNIX_TOREFERENCE));
  mo_objref_t fromtypob =
    mo_dyncast_objref (mo_objref_get_comp
                       (rolinsob, MOMROLASSIGNIX_FROMTYPE));
  mo_value_t fromexpv =
    mo_dyncast_objref (mo_objref_get_comp
                       (rolinsob, MOMROLASSIGNIX_FROMEXPR));
  if (totypob == fromtypob)
    {
      mom_cemit_write_reference (csta, torefob, instrob, depth);
      mom_cemit_newline (csta);
      mom_cemit_printf (csta, " = ");
      mom_cemit_write_expression (csta, fromexpv, instrob, depth);
      mom_cemit_printf (csta, ";");
    }
  else
    {
      mom_cemit_write_reference (csta, torefob, instrob, depth);
      mom_cemit_newline (csta);
      mom_cemit_printf (csta, " = /*castassign*/ (");
      mom_cemit_write_ctype_for (csta, totypob, ")", depth + 1);
      mom_cemit_write_expression (csta, fromexpv, instrob, depth);
      mom_cemit_printf (csta, ";");
    }
}                               /* end mom_cemit_write_assign_instr */



void
mom_cemit_write_chunk_instr (struct mom_cemitlocalstate_st *csta,
                             mo_objref_t instrob, mo_objref_t fromob,
                             mo_objref_t rolinsob, int depth)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil != NULL,
                    "cemit_write_chunk_instr: bad csta@%p", csta);
  mo_cemitpayl_ty *cemp = csta->mo_cemsta_payl;
  MOM_ASSERTPRINTF (cemp && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC
                    && cemp->mo_cemit_locstate == csta,
                    "cemit_write_chunk_instr: bad payl@%p in csta@%p", cemp,
                    csta);
  MOM_ASSERTPRINTF (mo_dyncast_objref (instrob),
                    "cemit_write_chunk_instr: bad instrob");
  MOM_ASSERTPRINTF (mo_dyncast_objref (fromob),
                    "cemit_write_chunk_instr: bad fromob");
  MOM_ASSERTPRINTF (mo_dyncast_objref (rolinsob),
                    "cemit_write_chunk_instr: bad rolinsob");
  MOM_ASSERTPRINTF (rolinsob->mo_ob_class == MOM_PREDEF (c_role_class)
                    && mo_objref_comp_count (rolinsob) >=
                    MOMROLCHUNKIX__LASTCHUNK
                    && mo_objref_get_comp (rolinsob,
                                           MOMROLCHUNKIX_ROLE) ==
                    MOM_PREDEF (chunk),
                    "cemit_write_chunk_instr: wrong rolinsob %s",
                    mo_objref_pnamestr (rolinsob));
  mo_value_t verbatimv =
    mo_objref_get_comp (rolinsob, MOMROLCHUNKIX_VERBATIM);
  mo_value_t referencev =
    mo_objref_get_comp (rolinsob, MOMROLCHUNKIX_REFERENCE);
  mo_value_t expressionv =
    mo_objref_get_comp (rolinsob, MOMROLCHUNKIX_EXPRESSION);
  mo_value_t blockv = mo_objref_get_comp (rolinsob, MOMROLCHUNKIX_BLOCK);
  mo_value_t labelv = mo_objref_get_comp (rolinsob, MOMROLCHUNKIX_LABEL);
  int nbcomp = mo_objref_comp_count (instrob);
  char instrid[MOM_CSTRIDSIZ];
  memset (instrid, 0, sizeof (instrid));
  mo_objref_idstr (instrid, instrob);
  mom_cemit_printf (csta, "// +chunk %s", instrid);
  mom_cemit_newline (csta);
  for (int cix = 0; cix < nbcomp; cix++)
    {
      mo_value_t curcomp = mo_objref_get_comp (instrob, cix);
      enum mo_valkind_en curkind = mo_kind_of_value (curcomp);
      switch (curkind)
        {
        case mo_KNONE:
          continue;
        case mo_KINT:
          mom_cemit_printf (csta, "%lld",
                            (long long) mo_value_to_int (curcomp, 0));
          continue;
        case mo_KSTRING:
          mom_cemit_printf (csta, "%s", mo_string_cstr (curcomp));
          continue;
        case mo_KOBJECT:
          {
            mo_objref_t curob = mo_dyncast_objref (curcomp);
            char curid[MOM_CSTRIDSIZ];
            memset (curid, 0, sizeof (curid));
            if (mo_set_contains (verbatimv, curob))
              mom_cemit_printf (csta, "%s", mo_objref_shortnamestr (curob));
            else if (mo_set_contains (referencev, curob))
              mom_cemit_write_reference (csta, curob, instrob, depth + 1);
            else if (mo_set_contains (expressionv, curob))
              mom_cemit_write_expression (csta, curob, instrob, depth + 1);
            else if (mo_set_contains (blockv, curob))
              mom_cemit_write_block (csta, curob, instrob, depth + 1);
            else if (mo_set_contains (labelv, curob))
              mom_cemit_printf (csta, MOM_LABEL_PREFIX "%s",
                                mo_objref_idstr (curid, curob));
            else
              goto badcurcomp;
          }
          continue;
        badcurcomp:
        default:
          // should never happen, since scan_chunk_instr should have failed
          MOM_FATAPRINTF
            ("cemit_write_chunk_instr: instrob %s with impossible comp#%d %s",
             mo_objref_pnamestr (instrob), cix, mo_value_pnamestr (curcomp));
        }                       /* end switch */
    }
  mom_cemit_newline (csta);
  mom_cemit_printf (csta, "// -chunk %s", instrid);
  mom_cemit_newline (csta);
}                               /* end mom_cemit_write_chunk_instr */


void
mom_cemit_write_jump_instr (struct mom_cemitlocalstate_st *csta,
                            mo_objref_t instrob, mo_objref_t fromob,
                            mo_objref_t rolinsob, int depth)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil != NULL,
                    "cemit_write_jump_instr: bad csta@%p", csta);
  mo_cemitpayl_ty *cemp = csta->mo_cemsta_payl;
  MOM_ASSERTPRINTF (cemp && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC
                    && cemp->mo_cemit_locstate == csta,
                    "cemit_write_jump_instr: bad payl@%p in csta@%p", cemp,
                    csta);
  MOM_ASSERTPRINTF (mo_dyncast_objref (instrob)
                    && instrob->mo_ob_class ==
                    MOM_PREDEF (jump_instruction_class),
                    "cemit_write_jump_instr: bad instrob");
  MOM_ASSERTPRINTF (mo_dyncast_objref (fromob),
                    "cemit_write_jump_instr: bad fromob");
  MOM_ASSERTPRINTF (mo_dyncast_objref (rolinsob)
                    && rolinsob->mo_ob_class == MOM_PREDEF (c_role_class)
                    && mo_objref_get_comp (rolinsob,
                                           MOMROLJUMPIX_ROLE) ==
                    MOM_PREDEF (to)
                    && mo_objref_comp_count (rolinsob) >=
                    MOMROLJUMPIX__LAST,
                    "cemit_write_jump_instr: bad rolinsob %s",
                    mo_objref_pnamestr (rolinsob));
  mo_objref_t toblockob =
    mo_dyncast_objref (mo_objref_get_comp (rolinsob, MOMROLJUMPIX_TO));
  MOM_ASSERTPRINTF (toblockob
                    && toblockob->mo_ob_class == MOM_PREDEF (c_block_class),
                    "cemit_write_jump_instr: bad toblock");
  char toblid[MOM_CSTRIDSIZ];
  memset (toblid, 0, sizeof (toblid));
  mom_cemit_printf (csta, "/*jump %s*/ goto " MOM_LABEL_PREFIX "%s;",
                    mo_objref_pnamestr (instrob), mo_objref_idstr (toblid,
                                                                   toblockob));
}                               /* end of mom_cemit_write_jump_instr */

void
mom_cemit_write_cond_instr (struct mom_cemitlocalstate_st *csta,
                            mo_objref_t instrob, mo_objref_t fromob,
                            mo_objref_t rolinsob, int depth)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil != NULL,
                    "cemit_write_cond_instr: bad csta@%p", csta);
  mo_cemitpayl_ty *cemp = csta->mo_cemsta_payl;
  MOM_ASSERTPRINTF (cemp && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC
                    && cemp->mo_cemit_locstate == csta,
                    "cemit_write_cond_instr: bad payl@%p in csta@%p", cemp,
                    csta);
  MOM_ASSERTPRINTF (mo_dyncast_objref (instrob),
                    "cemit_write_cond_instr: bad instrob");
  MOM_ASSERTPRINTF (mo_dyncast_objref (fromob),
                    "cemit_write_cond_instr: bad fromob");
  MOM_ASSERTPRINTF (mo_dyncast_objref (rolinsob),
                    "cemit_write_cond_instr: bad rolinsob");
  MOM_ASSERTPRINTF (rolinsob->mo_ob_class == MOM_PREDEF (c_role_class)
                    && mo_objref_comp_count (rolinsob) >=
                    MOMROLCONDIX__LAST
                    && mo_objref_get_comp (rolinsob,
                                           MOMROLCONDIX_ROLE) ==
                    MOM_PREDEF (conditional),
                    "cemit_write_cond_instr: wrong rolinsob %s",
                    mo_objref_pnamestr (rolinsob));
  mo_value_t condtup
    =
    mo_dyncast_tuple (mo_objref_get_comp (rolinsob, MOMROLCONDIX_CONDITIONS));
  MOM_ASSERTPRINTF (condtup != NULL, "cemit_write_cond_instr: no condtup");
  char instrid[MOM_CSTRIDSIZ];
  memset (instrid, 0, sizeof (instrid));
  mo_objref_idstr (instrid, instrob);
  int nbcond = mo_tuple_size (condtup);
  mom_cemit_newline (csta);
  mom_cemit_printf (csta, "//+ condinstr %s with %d conditions", instrid,
                    nbcond);
  for (int cix = 0; cix < nbcond; cix++)
    {
      mo_objref_t curcondob = mo_tuple_nth (condtup, cix);
      MOM_ASSERTPRINTF (curcondob != NULL,
                        "cemit_write_cond_instr: no curcondob cix#%d", cix);
      char condid[MOM_CSTRIDSIZ];
      memset (condid, 0, sizeof (condid));
      mo_objref_idstr (condid, curcondob);
      mo_objref_t condrolob =
        mo_dyncast_objref (mo_assoval_get
                           (csta->mo_cemsta_assoclocalrole, curcondob));
      MOM_ASSERTPRINTF (condrolob != NULL,
                        "cemit_write_cond_instr: no condrolob cix#%d for curcondob %s",
                        cix, condid);
      mom_cemit_newline (csta);
      MOM_ASSERTPRINTF (mo_objref_get_comp
                        (condrolob,
                         MOMROLCONDITIONIX_ROLE) == MOM_PREDEF (when)
                        && mo_objref_comp_count (condrolob) >=
                        MOMROLCONDITIONIX__LAST,
                        "cemit_write_cond_instr: bad condrolob %s cix#%d for curcondob %s",
                        mo_objref_pnamestr (condrolob), cix, condid);
      mo_value_t condwhenv =
        mo_objref_get_comp (condrolob, MOMROLCONDITIONIX_WHEN);
      mo_objref_t condbodyob =
        mo_dyncast_objref (mo_objref_get_comp
                           (condrolob, MOMROLCONDITIONIX_BODY));
      if (cix <= 0)
        mom_cemit_printf (csta, "if /*first condition %s*/ (", condid);
      else
        mom_cemit_printf (csta, "else if /*condition#%d %s*/ (", cix, condid);
      mom_cemit_write_expression (csta, condwhenv, curcondob, depth + 1);
      mom_cemit_printf (csta, ") { /*cond#%d.body*/", cix);
      unsigned oldindent = csta->mo_cemsta_indentation;
      csta->mo_cemsta_indentation = depth + 1;
      mom_cemit_newline (csta);
      mom_cemit_write_block (csta, condbodyob, curcondob, depth + 1);
      csta->mo_cemsta_indentation = oldindent;
      mom_cemit_newline (csta);
      mom_cemit_printf (csta, "} /*end condition#%d %s*/", cix, condid);
    }
  mom_cemit_newline (csta);
  mom_cemit_printf (csta, "//- condinstr %s", instrid);
  mom_cemit_newline (csta);
}                               /* end mom_cemit_write_cond_instr */


void
mom_cemit_write_call_instr (struct mom_cemitlocalstate_st *csta,
                            mo_objref_t instrob, mo_objref_t fromob,
                            mo_objref_t rolinsob, int depth)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil != NULL,
                    "cemit_write_call_instr: bad csta@%p", csta);
  mo_cemitpayl_ty *cemp = csta->mo_cemsta_payl;
  MOM_ASSERTPRINTF (cemp && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC
                    && cemp->mo_cemit_locstate == csta,
                    "cemit_write_call_instr: bad payl@%p in csta@%p", cemp,
                    csta);
  MOM_ASSERTPRINTF (mo_dyncast_objref (instrob),
                    "cemit_write_call_instr: bad instrob");
  MOM_ASSERTPRINTF (mo_dyncast_objref (fromob),
                    "cemit_write_call_instr: bad fromob");
  MOM_ASSERTPRINTF (mo_dyncast_objref (rolinsob),
                    "cemit_write_call_instr: bad rolinsob");
  MOM_ASSERTPRINTF (rolinsob->mo_ob_class == MOM_PREDEF (c_role_class)
                    && mo_objref_comp_count (rolinsob) >=
                    MOMROLCALLIX__LAST
                    && mo_objref_get_comp (rolinsob,
                                           MOMROLCALLIX_ROLE) ==
                    MOM_PREDEF (call),
                    "cemit_write_call_instr: wrong rolinsob %s",
                    mo_objref_pnamestr (rolinsob));
  char instrid[MOM_CSTRIDSIZ];
  memset (instrid, 0, sizeof (instrid));
  mo_objref_idstr (instrid, instrob);
  mo_objref_t callob =
    mo_dyncast_objref (mo_objref_get_comp (rolinsob, MOMROLCALLIX_CALL));
  char callid[MOM_CSTRIDSIZ];
  memset (callid, 0, sizeof (callid));
  mo_objref_idstr (callid, callob);
  MOM_ASSERTPRINTF (callob, "cemit_write_call_instr: no callob");
  if (callob->mo_ob_class == MOM_PREDEF (c_routine_class)
      || callob->mo_ob_class == MOM_PREDEF (c_inlined_class))
    {
      mo_value_t callnam = mo_objref_namev (callob);
      if (callnam)
        mom_cemit_printf (csta,
                          "/*call %s*/ " MOM_FUNC_PREFIX "%s /*%s*/ (",
                          instrid, callid, mo_string_cstr (callnam));
      else
        mom_cemit_printf (csta, "/*call %s*/ " MOM_FUNC_PREFIX "%s (",
                          instrid, callid);
    }
  else
    {
      mom_cemit_printf (csta, "/*indirectcall %s*/ (", instrid);
      mom_cemit_write_expression (csta, callob, instrob, 0);
      mom_cemit_printf (csta, ")  (");
    }
  int instrlen = mo_objref_comp_count (instrob);
  for (int ix = 0; ix < instrlen; ix++)
    {
      if (ix > 0)
        mom_cemit_printf (csta, ", ");
      mom_cemit_write_expression (csta, mo_objref_get_comp (instrob, ix),
                                  instrob, 0);
    }
  mom_cemit_printf (csta, ");");
}                               /* end mom_cemit_write_call_instr */


void
mom_cemit_write_expression (struct mom_cemitlocalstate_st *csta,
                            mo_value_t expv, mo_objref_t fromob, int depth)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil != NULL,
                    "cemit_write_expression: bad csta@%p", csta);
  mo_cemitpayl_ty *cemp = csta->mo_cemsta_payl;
  MOM_ASSERTPRINTF (cemp && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC
                    && cemp->mo_cemit_locstate == csta,
                    "cemit_write_expression: bad payl@%p in csta@%p", cemp,
                    csta);
  if (!expv)
    {
      mom_cemit_printf (csta, " NULL");
      return;
    }
  enum mo_valkind_en kv = mo_kind_of_value (expv);
  switch (kv)
    {
    case mo_KNONE:
      MOM_CEMITFAILURE (csta,
                        "cemit_scan_expression: missing expression %s from %s",
                        mo_value_pnamestr (expv),
                        mo_objref_pnamestr (fromob));
    case mo_KINT:
      mom_cemit_printf (csta, " %lld",
                        (long long) mo_value_to_int (expv, -1));
      return;
    case mo_KSTRING:
      mom_cemit_printf (csta, " \"");
      mom_output_utf8_encoded (csta->mo_cemsta_fil, mo_string_cstr (expv),
                               mo_string_size (expv));
      mom_cemit_printf (csta, "\" ");
      return;
    case mo_KTUPLE:
      goto setkindv;
    case mo_KSET:
    setkindv:
      MOM_CEMITFAILURE (csta,
                        "cemit_write_expression: bad sequence expression %s from %s",
                        mo_value_pnamestr (expv),
                        mo_objref_pnamestr (fromob));
    case mo_KOBJECT:
      {
        mo_objref_t expob = mo_dyncast_objref (expv);
        MOM_ASSERTPRINTF (expob != NULL,
                          "cemit_write_expression: bad expob from %s",
                          mo_objref_pnamestr (fromob));
        mo_objref_t locrolob =
          mo_dyncast_objref (mo_assoval_get
                             (csta->mo_cemsta_assoclocalrole, expob));
        mo_objref_t modrolob = locrolob ? NULL :
          mo_dyncast_objref (mo_assoval_get
                             (csta->mo_cemsta_assocmodulrole, expob));
        if (locrolob)
          {
            MOM_ASSERTPRINTF (locrolob->mo_ob_class ==
                              MOM_PREDEF (c_role_class), "bad locrolob");
            mo_objref_t kindlocrob =
              mo_dyncast_objref (mo_objref_get_comp (locrolob, 0));
            if (kindlocrob == MOM_PREDEF (formals))
              {
                mom_cemit_printf (csta, MOM_FORMAL_PREFIX "%s",
                                  mo_objref_shortnamestr (expob));
                return;
              }
            else if (kindlocrob == MOM_PREDEF (result))
              {
                mom_cemit_printf (csta, MOM_RESULT_PREFIX "%s",
                                  mo_objref_shortnamestr (expob));
                return;
              }
            else if (kindlocrob == MOM_PREDEF (locals))
              {
                mom_cemit_printf (csta, MOM_LOCAL_PREFIX "%s",
                                  mo_objref_shortnamestr (expob));
                return;
              }
          }
#warning cemit_write_expression unimplemented
        MOM_CEMITFAILURE
          (MOM_CEMIT_ADD_DATA
           (csta, expob, mo_int_to_value (depth), fromob,
            locrolob ? locrolob : modrolob),
           "cemit_write_expression: unimplemented expression %s "
           "from %s depth %d locrolob %s modrolob %s",
           mo_value_pnamestr (expv), mo_objref_pnamestr (fromob), depth,
           mo_objref_pnamestr (locrolob), mo_objref_pnamestr (modrolob));
      }
    }
  MOM_CEMITFAILURE
    (MOM_CEMIT_ADD_DATA (csta, expv, mo_int_to_value (depth), fromob),
     "cemit_write_expression: unimplemented expression %s from %s depth %d",
     mo_value_pnamestr (expv), mo_objref_pnamestr (fromob), depth);
}                               /* end of mom_cemit_write_expression */


void
mom_cemit_write_reference (struct mom_cemitlocalstate_st *csta,
                           mo_objref_t refob, mo_objref_t fromob, int depth)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil != NULL,
                    "cemit_write_reference: bad csta@%p", csta);
  mo_cemitpayl_ty *cemp = csta->mo_cemsta_payl;
  MOM_ASSERTPRINTF (cemp && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC
                    && cemp->mo_cemit_locstate == csta,
                    "cemit_write_reference: bad payl@%p in csta@%p", cemp,
                    csta);
  mo_objref_t locrolob =
    mo_dyncast_objref (mo_assoval_get
                       (csta->mo_cemsta_assoclocalrole, refob));
  mo_objref_t modrolob =
    locrolob ? NULL : mo_dyncast_objref (mo_assoval_get
                                         (csta->mo_cemsta_assocmodulrole,
                                          refob));
  if (locrolob)
    {
      MOM_ASSERTPRINTF (locrolob->mo_ob_class == MOM_PREDEF (c_role_class),
                        "bad locrolob %s", mo_objref_pnamestr (locrolob));
      mo_objref_t kindrolob =
        mo_dyncast_objref (mo_objref_get_comp (locrolob, 0));
      if (kindrolob == MOM_PREDEF (result))
        {
          mom_cemit_printf (csta, MOM_RESULT_PREFIX "%s",
                            mo_objref_shortnamestr (refob));
          return;
        }
      else if (kindrolob == MOM_PREDEF (locals))
        {
          mom_cemit_printf (csta, MOM_LOCAL_PREFIX "%s",
                            mo_objref_shortnamestr (refob));
          return;
        }
    }
  MOM_CEMITFAILURE (MOM_CEMIT_ADD_DATA
                    (csta, refob, mo_int_to_value (depth), fromob,
                     locrolob ? locrolob : modrolob),
                    "cemit_write_reference: unimplemented reference %s from %s locrolob %s modrolob %s depth %d",
                    mo_objref_pnamestr (refob), mo_objref_pnamestr (fromob),
                    mo_objref_pnamestr (locrolob),
                    mo_objref_pnamestr (modrolob), depth);
}                               /* end of mom_cemit_write_reference */


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
  mo_value_t codev =
    mo_objref_get_attr (cemp->mo_cemit_modobj, MOM_PREDEF (code));
  if (!codev)
    {
      mom_cemit_printf (csta, "// no function definitions\n");
      return;
    }
  if (codev && !mo_dyncast_sequence (codev))
    MOM_CEMITFAILURE (csta, "bad code %s", mo_value_pnamestr (codev));
  unsigned nbcode = mo_sequence_size (codev);
  mom_cemit_printf (csta, "\n// %d function definitions\n", nbcode);
  for (unsigned cix = 0; cix < nbcode; cix++)
    {
      mo_objref_t curfunob = mo_sequence_nth (codev, cix);
      MOM_ASSERTPRINTF (mo_dyncast_objref (curfunob),
                        "bad function code#%d", cix);
      mo_value_t curfunamv = mo_objref_namev (curfunob);
      char curfunid[MOM_CSTRIDSIZ];
      memset (curfunid, 0, sizeof (curfunid));
      mo_objref_idstr (curfunid, curfunob);
      if (curfunamv)
        mom_cemit_printf (csta, "\n\n\n//// function#%d code for %s (%s)\n",
                          cix, mo_string_cstr (curfunamv), curfunid);
      else
        mom_cemit_printf (csta, "\n\n\n//// function#%d code for %s\n",
                          cix, curfunid);
      if (mom_elapsed_real_time () > csta->mo_cemsta_timelimit)
        MOM_CEMITFAILURE (csta,
                          "cemit_function_definitions: %s timed out, index %d",
                          mo_objref_pnamestr (curfunob), cix);
      mom_cemit_function_code (csta, curfunob);
      mom_cemit_printf (csta, "\n");
    }
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
      mom_cemit_printf (csta, "\n// no set of global objects\n");
      return;
    }
  if (!mo_dyncast_set (setv))
    MOM_CEMITFAILURE (csta, "bad set %s", mo_value_pnamestr (setv));
  csta->mo_cemsta_objset = setv;
  unsigned nbset = mo_set_size (setv);
  int nbpredef = 0;
  for (unsigned eix = 0; eix < nbset; eix++)
    {
      mo_objref_t elemob = mo_set_nth (setv, eix);
      MOM_ASSERTPRINTF (mo_dyncast_objref (elemob), "bad elemob#%d", eix);
      if (mo_objref_space (elemob) == mo_SPACE_PREDEF)
        {
          mo_value_t elemnamv = mo_objref_namev (elemob);
          char elemid[MOM_CSTRIDSIZ];
          memset (elemid, 0, sizeof (elemid));
          mo_objref_idstr (elemid, elemob);
          if (MOM_UNLIKELY (nbpredef == 0))
            {
              mom_cemit_printf (csta, "\n// global predefined objects\n");
            };
          nbpredef++;
          if (elemnamv)
            {
              mom_cemit_printf (csta,
                                "#define mo_%s_ob (&mompredef_%s) /*global MOM_PREDEF(%s) = %s*/\n",
                                mo_string_cstr (elemnamv),
                                mo_string_cstr (elemnamv),
                                mo_string_cstr (elemnamv), elemid);
              mom_cemit_printf (csta,
                                "#define mo_%s_ob mo_%s_ob /*MOM_PREDEF(%s)*/\n",
                                elemid + 1, mo_string_cstr (elemnamv),
                                mo_string_cstr (elemnamv));
            }
          else
            mom_cemit_printf (csta,
                              "#define mo_%s_ob (&mompredef_%s) /*anonglob MOM_PREDEF(%s)*/\n",
                              elemid + 1, elemid + 1, elemid);
        }
    }
  mom_cemit_printf (csta, "\n\n#ifdef MONIMELT_MODULE\n");
  mom_cemit_printf (csta, "\n// set of %d non-predefined global objects\n",
                    nbset - nbpredef);
  for (unsigned eix = 0; eix < nbset; eix++)
    {
      mo_objref_t elemob = mo_set_nth (setv, eix);
      MOM_ASSERTPRINTF (mo_dyncast_objref (elemob), "bad elemob#%d", eix);
      mo_value_t elemnamv = mo_objref_namev (elemob);
      char elemid[MOM_CSTRIDSIZ];
      memset (elemid, 0, sizeof (elemid));
      mo_objref_idstr (elemid, elemob);
      if (mo_objref_space (elemob) == mo_SPACE_PREDEF)
        continue;
      else
        {
          if (mo_objref_space (elemob) == mo_SPACE_NONE)
            MOM_CEMITFAILURE (csta, "global %s is transient",
                              mo_objref_pnamestr (elemob));
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
    }
  mom_cemit_printf (csta, "\n\n// initialization of %d objects\n", nbset);
  mom_cemit_printf (csta, "void " MOM_MODULEINIT_PREFIX "%s (void) {\n",
                    modulid + 1);
  for (unsigned eix = 0; eix < nbset; eix++)
    {
      mo_objref_t elemob = mo_set_nth (setv, eix);
      char elemid[MOM_CSTRIDSIZ];
      memset (elemid, 0, sizeof (elemid));
      mo_objref_idstr (elemid, elemob);
      MOM_ASSERTPRINTF (mo_dyncast_objref (elemob), "bad elemob#%d", eix);
      if (mo_objref_space (elemob) == mo_SPACE_PREDEF)
        {
          mom_cemit_printf (csta, "// global#%d:\n", eix);
          mom_cemit_printf (csta, "// global %s (%s) is predefined\n",
                            mo_objref_pnamestr (elemob), elemid);
          continue;
        }
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
                    modulid + 1);
  mo_value_t modulnamv = mo_objref_namev (cemp->mo_cemit_modobj);
  if (modulnamv)
    {
      mom_cemit_printf (csta, "void " MOM_MODULEINIT_PREFIX "%s (void) {\n",
                        mo_string_cstr (modulnamv));
      mom_cemit_printf (csta, "  " MOM_MODULEINIT_PREFIX "%s ();\n",
                        modulid + 1);
      mom_cemit_printf (csta, "} /*end " MOM_MODULEINIT_PREFIX "%s */\n",
                        mo_string_cstr (modulnamv));
    }
  mom_cemit_printf (csta,
                    "void monimelt_module_init(void) __attribute__((constructor));\n");
  mom_cemit_printf (csta,
                    "void monimelt_module_init(void) {\n"
                    "   " MOM_MODULEINIT_PREFIX "%s ();\n"
                    "} /* end monimelt_module_init */\n", modulid + 1);
  if ((int) nbset > nbpredef)
    mom_cemit_printf (csta, "#else /*!MONIMELT_MODULE*/\n"
                      "#warning %d global objects are not predefined"
                      " without MONIMELT_MODULE\n", nbset - nbpredef);
  mom_cemit_printf (csta, "#endif /*MONIMELT_MODULE*/\n\n");
  mom_cemit_printf (csta, "#ifndef MOM_NB_MODULE_GLOBAL\n"
                    "#define MOM_NB_MODULE_GLOBAL %d\n"
                    "#endif /*MOM_NB_MODULE_GLOBAL*/\n", nbset - nbpredef);
  mom_cemit_printf (csta, "#ifndef MOM_NB_MODULE_PREDEF\n"
                    "#define MOM_NB_MODULE_PREDEF %d\n"
                    "#endif /*MOM_NB_MODULE_PREDEF*/\n", nbpredef);
}                               /* end of mom_cemit_set */



// We sometimes need to compare two C-types (e.g. for some kind of
// assignment left := right - or argument passing, etc...) and get
// their common supertype
mo_objref_t
mom_cemit_compare_ctypes (struct mom_cemitlocalstate_st *csta,
                          mo_objref_t leftctypob,
                          mo_objref_t rightctypob, mo_objref_t fromob)
{
  MOM_ASSERTPRINTF (csta && csta->mo_cemsta_nmagic == MOM_CEMITSTATE_MAGIC
                    && csta->mo_cemsta_fil != NULL,
                    "cemit_compare_ctypes: bad csta@%p", csta);
  mo_cemitpayl_ty *cemp = csta->mo_cemsta_payl;
  MOM_ASSERTPRINTF (cemp && cemp->mo_cemit_nmagic == MOM_CEMIT_MAGIC
                    && cemp->mo_cemit_locstate == csta,
                    "cemit_compare_ctypes: bad payl@%p in csta@%p", cemp,
                    csta);
  if (!mom_cemit_is_ctype (csta, leftctypob))
    MOM_CEMITFAILURE (csta,
                      "cemit_compare_ctypes: bad leftctypob %s from %s",
                      mo_objref_pnamestr (leftctypob),
                      mo_objref_pnamestr (fromob));
  if (!mom_cemit_is_ctype (csta, rightctypob))
    MOM_CEMITFAILURE (csta,
                      "cemit_compare_ctypes: bad rightctypob %s from %s",
                      mo_objref_pnamestr (rightctypob),
                      mo_objref_pnamestr (fromob));
  if (leftctypob == rightctypob)
    return leftctypob;
  if (leftctypob == MOM_PREDEF (null_ctype))
    MOM_CEMITFAILURE (csta,
                      "cemit_compare_ctypes: null leftctype for rightctype %s from %s",
                      mo_objref_pnamestr (rightctypob),
                      mo_objref_pnamestr (fromob));
  if (rightctypob == MOM_PREDEF (null_ctype))
    {
      if (leftctypob == momglob_value_ctype
          || leftctypob == momglob_object_ctype
          || leftctypob->mo_ob_class == momglob_pointer_ctype_class
          || leftctypob->mo_ob_class ==
          MOM_PREDEF (struct_pointer_ctype_class))
        return leftctypob;
      else
        MOM_CEMITFAILURE (csta,
                          "cemit_compare_ctypes: null rightctype cant be assigned to %s from %s",
                          mo_objref_pnamestr (leftctypob),
                          mo_objref_pnamestr (fromob));
    }

#warning mom_cemit_compare_ctypes unimplemented
  MOM_FATAPRINTF
    ("mom_cemit_compare_ctypes unimplemented leftctypob=%s rightctypob=%s from=%s",
     mo_objref_pnamestr (leftctypob), mo_objref_pnamestr (rightctypob),
     mo_objref_pnamestr (fromob));
}                               /* end of mom_cemit_compare_ctypes */

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
             MOM_TERMWARNCOLOR, MOM_TERMPLAIN,
             mo_objref_pnamestr (curtodobj), nbdone,
             mo_objref_pnamestr (curoutobj));
          continue;
        }
      mo_signature_two_objects_to_void_sigt *funrout =
        curoutobj->mo_ob_payldata;
      (*funrout) (curtodobj, objcemit);
    }
}                               /* end mom_cemit_do_at_end */


#define MOM_CEMIT_GENERATE_TIMEOUT 2.5  /*elapsed seconds */
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
  cemitstate.mo_cemsta_timelimit =
    mom_elapsed_real_time () + MOM_CEMIT_GENERATE_TIMEOUT;
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
      if (cemitstate.mo_cemsta_curfun)
        MOM_WARNPRINTF_AT (__FILE__, errlin,
                           "cemit_generate failure: %s (module %s, emitter %s, function %s)",
                           mo_string_cstr (cemitstate.mo_cemsta_errstr),
                           mo_objref_pnamestr (cemp->mo_cemit_modobj),
                           mo_objref_pnamestr (obrcem),
                           mo_objref_pnamestr (cemitstate.mo_cemsta_curfun));
      else
        MOM_WARNPRINTF_AT (__FILE__, errlin,
                           "cemit_generate failure: %s (module %s, emitter %s)",
                           mo_string_cstr (cemitstate.mo_cemsta_errstr),
                           mo_objref_pnamestr (cemp->mo_cemit_modobj),
                           mo_objref_pnamestr (obrcem));
      if (cemitstate.mo_cemsta_fil)
        {
          if (cemitstate.mo_cemsta_curfun)
            fprintf (cemitstate.mo_cemsta_fil,
                     "\n\n\n/////@@@@@@@!!!!!! \n"
                     "#error @%d: %s (module %s, emitter %s, function %s)\n",
                     errlin,
                     mo_string_cstr (cemitstate.mo_cemsta_errstr),
                     mo_objref_pnamestr (cemp->mo_cemit_modobj),
                     mo_objref_pnamestr (obrcem),
                     mo_objref_pnamestr (cemitstate.mo_cemsta_curfun));
          else
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
      mo_value_t errv = cemitstate.mo_cemsta_errstr;
      memset (&cemitstate, 0, sizeof (cemitstate));
      cemp->mo_cemit_locstate = NULL;
      return errv;
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
      mom_cemit_close (&cemitstate);
      memset (&cemitstate, 0, sizeof (cemitstate));
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

     void MOM_PREFIXID (mofun_, put_attr_cemitact) (mo_objref_t todobj,
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
