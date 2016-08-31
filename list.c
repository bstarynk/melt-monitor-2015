// file list.c - managing list payloads

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

mo_listpayl_ty *
mo_list_make (void)
{
  mo_listpayl_ty *lis = mom_gc_alloc (sizeof (mo_listpayl_ty));
  ((mo_hashedvalue_ty *) lis)->mo_va_kind = mo_PLIST;
  ((mo_hashedvalue_ty *) lis)->mo_va_hash =
    (momrand_genrand_int31 () & 0xfffffff) + 2;
  lis->mo_lip_first = NULL;
  lis->mo_lip_last = NULL;
  return lis;
}                               /* end mo_list_make */

void
mo_list_append (mo_listpayl_ty * lis, mo_value_t v)
{
  if (!mo_dyncastpayl_list (lis))
    return;
  if (mo_kind_of_value (v) == mo_KNONE)
    return;
#warning mo_list_append incomplete
  MOM_FATAPRINTF ("unimplemented mo_list_append");
}                               /* end mo_list_append */

void
mo_list_prepend (mo_listpayl_ty * lis, mo_value_t v)
{
  if (!mo_dyncastpayl_list (lis))
    return;
  if (mo_kind_of_value (v) == mo_KNONE)
    return;
#warning mo_list_prepend incomplete
  MOM_FATAPRINTF ("unimplemented mo_list_prepend");
}                               /* end mo_list_prepend */
