// file miniedit.c

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

enum miniedit_closoff_en
{
  mic_wexitm,
  mic__last
};


struct mom_item_st *
mom_miniedit_nth_item (struct mom_item_st *taskitm, int off)
{
  struct mom_item_st *resitm = NULL;
  const struct mom_boxnode_st *nod = NULL;
  if (!taskitm || taskitm == MOM_EMPTY_SLOT
      || taskitm->va_itype != MOMITY_ITEM)
    return NULL;
  unsigned siz=0;
  mom_item_lock (taskitm);
  nod = mom_dyncast_node (taskitm->itm_payload);
  mom_item_unlock (taskitm);
  if (nod && nod->va_itype == MOMITY_NODE
      && nod->nod_connitm == MOM_PREDEFITM (miniedit)
      && (siz=mom_raw_size (nod)) >= mic__last) {
    int realoff = off;
    if (realoff<0) realoff += siz;
    if (realoff >= 0 && realoff < (int)siz)
      resitm = mom_dyncast_item (nod->nod_sons[realoff]);
  };
  MOM_DEBUGPRINTF (web, "miniedit_nth_item taskitm %s off#%d result itm %s",
                   mom_item_cstring (taskitm), off, mom_item_cstring (resitm));
  return resitm;
} /* end mom_miniedit_nth_item */


extern mom_tasklet_sig_t momf_miniedit;
const char momsig_miniedit[] = "signature_tasklet";
void
momf_miniedit (struct mom_item_st *tkitm)
{
  struct mom_item_st *wexitm = NULL;
  struct mom_item_st *thistatitm = NULL;
  struct mom_item_st *sessitm = NULL;
  struct mom_item_st *hsetitm = NULL;
  mom_item_lock (tkitm);
  const struct mom_boxnode_st *tknod =
    (struct mom_boxnode_st *) tkitm->itm_payload;
  MOM_DEBUGPRINTF (web,
                   "momf_miniedit start tkitm=%s tknod=%s",
                   mom_item_cstring (tkitm),
                   mom_value_cstring ((const struct
                                       mom_hashedvalue_st *) tknod));
  if (mom_itype (tknod) != MOMITY_NODE || mom_raw_size (tknod) < mic__last)
    {
      /// should not happen
      MOM_WARNPRINTF ("momf_miniedit bad tknod %s",
                      mom_value_cstring ((const struct
                                          mom_hashedvalue_st *) tknod));
      goto end;
    }
  wexitm = mom_miniedit_nth_item(tkitm, mic_wexitm);
  MOM_DEBUGPRINTF (web,
                   "momf_miniedit wexitm=%s", mom_item_cstring(wexitm));
  if (!wexitm) goto end;
  mom_item_lock (wexitm);
  struct mom_webexch_st *wexch =
    (struct mom_webexch_st *) wexitm->itm_payload;
  assert (wexch && wexch->va_itype == MOMITY_WEBEXCH);
  sessitm = wexch->webx_sessitm;
  MOM_DEBUGPRINTF (web,
                   "momf_miniedit sessitm=%s wexch #%ld meth %s fupath %s path %s",
		   mom_item_cstring(sessitm), 
                   wexch->webx_count,
                   mom_webmethod_name (wexch->webx_meth),
		   onion_request_get_fullpath(wexch->webx_requ),
		   onion_request_get_path(wexch->webx_requ));
  mom_item_lock (sessitm);  
end:
  if (sessitm)
    mom_item_unlock (sessitm);
  if (wexitm)
    mom_item_unlock (wexitm);
  mom_item_unlock (tkitm);
} /* end of momf_miniedit */
  




enum minieditgenstyle_closoff_en
{
  miedgsty_wexitm,
  miedgsty_stytup,
  miedgsty__last
};


extern mom_tasklet_sig_t momf_miniedit_genstyle;
const char momsig_miniedit_genstyle[] = "signature_tasklet";
void
momf_miniedit_genstyle (struct mom_item_st *tkitm)
{
  struct mom_item_st *wexitm = NULL;
  struct mom_item_st *thistatitm = NULL;
  struct mom_item_st *sessitm = NULL;
  mom_item_lock (tkitm);
  const struct mom_boxnode_st *tknod =
    (struct mom_boxnode_st *) tkitm->itm_payload;
  MOM_DEBUGPRINTF (web,
                   "momf_miniedit_genstyle start tkitm=%s tknod=%s",
                   mom_item_cstring (tkitm),
                   mom_value_cstring ((const struct
                                       mom_hashedvalue_st *) tknod));
  if (!tknod || tknod->va_itype != MOMITY_NODE || mom_size(tknod)<miedgsty__last)
    MOM_FATAPRINTF("minedit_genstyle tkitm %s has bad tknod %s",
                   mom_item_cstring (tkitm),
                   mom_value_cstring ((const struct
                                       mom_hashedvalue_st *) tknod));
  wexitm = mom_dyncast_item(tknod->nod_sons[miedgsty_wexitm]);
  mom_item_lock(wexitm);
  MOM_DEBUGPRINTF (web,
                   "minedit_genstyle wexitm=%s", mom_item_cstring(wexitm));
  if (!wexitm) goto end;
  mom_item_lock (wexitm);
  struct mom_webexch_st *wexch =
    (struct mom_webexch_st *) wexitm->itm_payload;
  assert (wexch && wexch->va_itype == MOMITY_WEBEXCH);
  sessitm = wexch->webx_sessitm;
  MOM_DEBUGPRINTF (web,
                   "miniedit_genstyle sessitm=%s wexch #%ld meth %s fupath %s path %s",
		   mom_item_cstring(sessitm), 
                   wexch->webx_count,
                   mom_webmethod_name (wexch->webx_meth),
		   onion_request_get_fullpath(wexch->webx_requ),
		   onion_request_get_path(wexch->webx_requ));
  mom_item_lock (sessitm);
  struct mom_tuple_st*tupsty = mom_dyncast_tuple(tknod->nod_sons[miedgsty_stytup]);
  if (!tupsty)
    MOM_FATAPRINTF("minedit_genstyle tkitm %s has bad tuple of styles@#%d in tknod %s",
		   mom_item_cstring (tkitm),
                   (int)miedgsty_stytup,
		   mom_value_cstring ((const struct mom_hashedvalue_st *) tknod));
  mom_output_gplv3_notice (wexch->webx_outfil, "/*", "*/", "miniedit/_genstyle.css");
#warning should output each individual style
end:
  if (sessitm)
    mom_item_unlock (sessitm);
  if (wexitm)
    mom_item_unlock (wexitm);
  mom_item_unlock (tkitm);
} /* end of momf_miniedit_genstyle */
  


enum minieditgenscript_closoff_en
{
  miedgscr_wexitm,
  miedgscr__last
};

extern mom_tasklet_sig_t momf_miniedit_genscript;
const char momsig_miniedit_genscript[] = "signature_tasklet";
void
momf_miniedit_genscript (struct mom_item_st *tkitm)
{
  struct mom_item_st *wexitm = NULL;
  struct mom_item_st *thistatitm = NULL;
  struct mom_item_st *sessitm = NULL;
  struct mom_item_st *hsetitm = NULL;
  mom_item_lock (tkitm);
  const struct mom_boxnode_st *tknod =
    (struct mom_boxnode_st *) tkitm->itm_payload;
  MOM_DEBUGPRINTF (web,
                   "momf_miniedit_genscript start tkitm=%s tknod=%s",
                   mom_item_cstring (tkitm),
                   mom_value_cstring ((const struct
                                       mom_hashedvalue_st *) tknod)); 
  if (!tknod || tknod->va_itype != MOMITY_NODE || mom_size(tknod)<miedgscr__last)
    MOM_FATAPRINTF("minedit_genscript tkitm %s has bad tknod %s",
                   mom_item_cstring (tkitm),
                   mom_value_cstring ((const struct
                                       mom_hashedvalue_st *) tknod));
  wexitm = mom_dyncast_item(tknod->nod_sons[miedgsty_wexitm]);
  mom_item_lock(wexitm);
end:
  if (sessitm)
    mom_item_unlock (sessitm);
  if (wexitm)
    mom_item_unlock (wexitm);
  mom_item_unlock (tkitm);
} /* end of momf_miniedit_genscript */
  
