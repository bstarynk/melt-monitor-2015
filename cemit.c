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
/* the temporary suffix is often				\
   something like '+r0abcd_ef09_p324' */			\
  char mo_cemit_tempsuff[40];					\
/* the prefix could be 'modules.dir/momg_' */			\
  char mo_cemit_prefix[32];					\
/* the generated temporary file handle */			\
  FILE* mo_cemit_fil;						\
/* the source module */						\
  mo_objref_t mo_cemit_modobj

struct mo_cemitpayl_st
{
  MOMFIELDS_cemitpayl;
};

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
  if (cemp->mo_cemit_fil)
    fclose (cemp->mo_cemit_fil), cemp->mo_cemit_fil = NULL;
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
  snprintf (cemp->mo_cemit_tempsuff, sizeof (cemp->mo_cemit_tempsuff),
            "+r%x_%x_p%d",
            (int) (momrand_genrand_int31 () & 0x3ffffff) + 1,
            (int) (momrand_genrand_int31 () & 0x3ffffff) + 1,
            (int) getpid ());
  cemp->mo_cemit_fil = NULL;
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

bool
mo_objref_has_opened_cemit_payload (mo_objref_t obr)
{
  if (!mo_dyncast_objref (obr))
    return false;
  if (obr->mo_ob_paylkind != MOM_PREDEF (payload_c_emit))
    return false;
  mo_cemitpayl_ty *cemp = mo_dyncastpayl_cemit (obr->mo_ob_payldata);
  if (!cemp)
    return false;
  return cemp->mo_cemit_fil != NULL;
}                               /* end of mo_objref_has_opened_cemit_payload */

const char *
mo_objref_cemit_detailstr (mo_objref_t obr)
{
  char *res = NULL;
  if (!mo_objref_has_valid_cemit_payload (obr))
    return NULL;
  mo_cemitpayl_ty *cemp = mo_dyncastpayl_cemit (obr->mo_ob_payldata);
  if (cemp->mo_cemit_fil)
    {
      char *buf = NULL;
      asprintf (&buf, "cemit#%d: %s%s%s", fileno (cemp->mo_cemit_fil),
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

/*** end of file cemit.c ***/
