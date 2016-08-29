// file assoval.c - hashed association between objects & values

/**   Copyright (C) 2016  Basile Starynkevitch and later the FSF
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

#define MOM_ASSOVAL_SMALLSIZE 8 // below that size, seek sequentially
// return the index where the obr could be found or put or else <0
static int
mom_assoval_index (mo_assovaldatapayl_ty * asso, mo_objref_t obr)
{
  MOM_ASSERTPRINTF (mo_dyncastpayl_assoval (asso) != NULL, "bad asso@%p",
                    asso);
  MOM_ASSERTPRINTF (mo_dyncast_objref (obr) != NULL, "bad obr@%p", obr);
  uint32_t sz = ((mo_sizedvalue_ty *) asso)->mo_sva_size;
  MOM_ASSERTPRINTF (sz > 2, "bad sz=%u", sz);
  if (sz < MOM_ASSOVAL_SMALLSIZE)
    {
      for (unsigned ix = 0; ix < sz; ix++)
        {
          mo_objref_t obcur = asso->mo_seqent[ix].mo_asso_obr;
          if (obcur == obr)
            return ix;
          else if (obcur == NULL || obcur == MOM_EMPTY_SLOT)
            return ix;
        }
      return -1;
    }
  else
    {                           /* nonsmall sz */
      unsigned h = mo_objref_hash (obr);
      MOM_ASSERTPRINTF (h != 0, "bad h for obr@%p", obr);
      unsigned startix = h % sz;
      int pos = -1;
      for (unsigned ix = startix; ix < sz; ix++)
        {
          mo_objref_t obcur = asso->mo_seqent[ix].mo_asso_obr;
          if (obcur == obr)
            return ix;
          else if (obcur == MOM_EMPTY_SLOT)
            {
              if (pos < 0)
                pos = (int) ix;
              continue;
            }
          else if (!obcur)
            {
              if (pos < 0)
                pos = (int) ix;
              return pos;
            }
        }
      for (unsigned ix = 0; ix < startix; ix++)
        {
          mo_objref_t obcur = asso->mo_seqent[ix].mo_asso_obr;
          if (obcur == obr)
            return ix;
          else if (obcur == MOM_EMPTY_SLOT)
            {
              if (pos < 0)
                pos = (int) ix;
              continue;
            }
          else if (!obcur)
            {
              if (pos < 0)
                pos = (int) ix;
              return pos;
            }
        }
      return pos;
    }
}                               /* end mom_assoval_index */



mo_value_t
mo_assoval_get (mo_assovaldatapayl_ty * asso, mo_objref_t obr)
{
  if (mo_dyncastpayl_assoval (asso) == NULL)
    return NULL;
  if (mo_dyncast_objref (obr) == NULL)
    return NULL;
  int pos = mom_assoval_index (asso, obr);
  if (pos < 0)
    return NULL;
  MOM_ASSERTPRINTF (pos < (int) (((mo_sizedvalue_ty *) asso)->mo_sva_size),
                    "bad pos%d", pos);
  if (asso->mo_seqent[pos].mo_asso_obr != obr)
    return NULL;
  MOM_ASSERTPRINTF (asso->mo_seqent[pos].mo_asso_val != NULL,
                    "corrupted asso pos=%d", pos);
  return asso->mo_seqent[pos].mo_asso_val;
}
