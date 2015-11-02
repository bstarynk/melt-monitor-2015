// file microedit.c

/**   Copyright (C)  2015  Basile Starynkevitch and later the FSF
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


extern mom_tasklet_sig_t momf_microedit;
const char momsig_microedit[] = "signature_tasklet";
void
momf_microedit (struct mom_item_st *tkitm)
{
  enum microedit_closoff_en
  {
    mec_wexitm,
    mec_protowebstate,
    mec_thiswebstate,
    mec__last
  };
  struct mom_item_st *wexitm = NULL;
  mom_item_lock (tkitm);
  const struct mom_boxnode_st *tknod =
    (struct mom_boxnode_st *) tkitm->itm_payload;
  MOM_DEBUGPRINTF (web, "momf_microedit start tkitm=%s tknod=%s",
                   mom_item_cstring (tkitm),
                   mom_value_cstring ((const struct mom_hashedvalue_st *)
                                      tknod));
  if (mom_itype (tknod) != MOMITY_NODE || mom_raw_size (tknod) < mec__last)
    {
      /// should not happen
      MOM_WARNPRINTF ("momf_microedit bad tknod %s",
                      mom_value_cstring ((const struct mom_hashedvalue_st *)
                                         tknod));
      goto end;
    }
  wexitm = mom_dyncast_item (tknod->nod_sons[mec_wexitm]);
  assert (wexitm && wexitm->va_itype == MOMITY_ITEM);
  mom_item_lock (wexitm);
  MOM_DEBUGPRINTF (web, "momf_microedit tkitm=%s wexitm=%s paylkind %s",
                   mom_item_cstring (tkitm), mom_item_cstring (wexitm),
                   mom_itype_str (wexitm->itm_payload));
  struct mom_webexch_st *wexch =
    (struct mom_webexch_st *) wexitm->itm_payload;
  assert (wexch && wexch->va_itype == MOMITY_WEBEXCH);
  MOM_DEBUGPRINTF (web,
                   "momf_microedit tkitm=%s wexch #%ld meth %s fupath %s sessitm %s",
                   mom_item_cstring (tkitm), wexch->webx_count,
                   mom_webmethod_name (wexch->webx_meth),
                   onion_request_get_fullpath (wexch->webx_requ),
                   mom_item_cstring (wexch->webx_sessitm));
end:
  if (wexitm)
    mom_item_unlock (wexitm);
  mom_item_unlock (tkitm);
}                               /* end of momf_microedit */
