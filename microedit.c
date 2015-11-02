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
  mom_item_lock(tkitm);
  const struct mom_boxnode_st* tknod = (struct mom_boxnode_st*)tkitm->itm_payload;
  MOM_DEBUGPRINTF(web, "momf_microedit start tkitm=%s tknod=%s",
		  mom_item_cstring(tkitm),
		  mom_value_cstring((const struct mom_hashedvalue_st *)tknod));
  if (mom_itype(tknod) != MOMITY_NODE) {
    MOM_WARNPRINTF("momf_microedit bad tknod kind %s", mom_itype_str(tknod));
    goto end;
  }
 end:
  mom_item_unlock(tkitm);
}
