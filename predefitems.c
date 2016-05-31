// file predefitems.c - defining predefined items

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

#define MOM_HAS_PREDEFINED(Nam,Hash)		\
  struct mom_item_st mompredef_##Nam = {	\
  .va_itype = MOMITY_ITEM,			\
  .hva_hash = Hash,				\
  .itm_hid = 0,					\
  .itm_lid = 0,					\
  };
#include "_mom_predef.h"


void
mom_initialize_predefined_items (void)
{
  static bool initialized;
  if (initialized)
    MOM_FATAPRINTF ("already initialized items");
  initialized = true;
#define MOM_HAS_PREDEFINED(Nam,Hash) \
  mom_initialize_a_predefined (&mompredef_##Nam, #Nam, Hash);
#include "_mom_predef.h"
}                               /* end mom_initialize_predefined_items */
