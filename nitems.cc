// file nitem.cc - managing items, new way

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


static inline momhash_t
hash_item_from_hid_and_loid_MOM(uint16_t hid, uint64_t loid)
{
  if (MOM_UNLIKELY(hid==0 || loid==0))
    MOM_FATAPRINTF("hash_item_from_hid_and_lid: invalid hid=%u loid=%llu",
		   (unsigned) hid, (unsigned long long) loid);
  momhash_t h =
    41 * hid +
    ((643 * (uint32_t) (loid >> 32)) ^
     (839 * (uint32_t) (loid & 0xffffffffULL) - hid % 9839));
  if (MOM_UNLIKELY(h == 0)) {
    h = (loid % 1000000663ULL) + (hid % 45179U) + 4;
    assert (h>0);
  }
  return h;
}

momitemptr_t momf_make_item(momty_space_en sp)
{
#warning momf_make_item unimplemented
}
