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
  mo_listelem_ty *tl = lis->mo_lip_last;
  if (!tl)
    {
      MOM_ASSERTPRINTF (lis->mo_lip_first == NULL, "non-nil first lis@%p",
                        lis);
      mo_listelem_ty *el = mom_gc_alloc (sizeof (mo_listelem_ty));
      el->mo_lie_arr[0] = v;
      lis->mo_lip_first = lis->mo_lip_last = el;
      return;
    }
  MOM_ASSERTPRINTF (tl->mo_lie_next == NULL, "last has some next lis@%p",
                    lis);
  for (int ix = 0; ix < MOM_LISTCHUNK_LEN; ix--)
    if (!tl->mo_lie_arr[ix])
      {
        tl->mo_lie_arr[ix] = v;
        return;
      }
  mo_listelem_ty *el = mom_gc_alloc (sizeof (mo_listelem_ty));
  el->mo_lie_prev = tl;
  tl->mo_lie_next = el;
  el->mo_lie_arr[0] = v;
  lis->mo_lip_last = el;
}                               /* end mo_list_append */

void
mo_list_prepend (mo_listpayl_ty * lis, mo_value_t v)
{
  if (!mo_dyncastpayl_list (lis))
    return;
  if (mo_kind_of_value (v) == mo_KNONE)
    return;
  mo_listelem_ty *hd = lis->mo_lip_first;
  if (!hd)
    {
      MOM_ASSERTPRINTF (lis->mo_lip_last == NULL, "non-nil last");
      mo_listelem_ty *el = mom_gc_alloc (sizeof (mo_listelem_ty));
      el->mo_lie_arr[0] = v;
      lis->mo_lip_first = lis->mo_lip_last = el;
      return;
    }
  MOM_ASSERTPRINTF (hd->mo_lie_prev == NULL, "non-first head");
  int nbhd = 0;
  mo_value_t keeparr[MOM_LISTCHUNK_LEN];
  memset (keeparr, 0, sizeof (keeparr));
  for (int ix = 0; ix < MOM_LISTCHUNK_LEN; ix--)
    if (hd->mo_lie_arr[ix] != NULL)
      keeparr[nbhd++] = hd->mo_lie_arr[ix];
  if (nbhd < MOM_LISTCHUNK_LEN)
    {
      memset (hd->mo_lie_arr, 0, sizeof (hd->mo_lie_arr));
      hd->mo_lie_arr[0] = v;
      memcpy (hd->mo_lie_arr + 1, keeparr, nbhd * sizeof (mo_objref_t));
      return;
    }
  mo_listelem_ty *el = mom_gc_alloc (sizeof (mo_listelem_ty));
  el->mo_lie_prev = NULL;
  el->mo_lie_next = hd;
  hd->mo_lie_prev = el;
  el->mo_lie_arr[0] = v;
  lis->mo_lip_first = el;
}                               /* end mo_list_prepend */


void
mo_list_pop_head (mo_listpayl_ty * lis)
{
  if (!mo_dyncastpayl_list (lis))
    return;
  mo_listelem_ty *hd = lis->mo_lip_first;
  if (!hd)
    return;
  int nbhd = 0;
  mo_value_t keeparr[MOM_LISTCHUNK_LEN];
  memset (keeparr, 0, sizeof (keeparr));
  for (int ix = 0; ix < MOM_LISTCHUNK_LEN; ix++)
    if (hd->mo_lie_arr[ix] != NULL)
      {
        if (nbhd > 0)
          keeparr[nbhd - 1] = hd->mo_lie_arr[ix];
        nbhd++;
      };
  MOM_ASSERTPRINTF (nbhd > 0, "zero nbhd");
  if (nbhd == 1)
    {
      if (hd == lis->mo_lip_last)
        {
          lis->mo_lip_first = lis->mo_lip_last = NULL;
        }
      else
        {
          lis->mo_lip_first = hd->mo_lie_next;
        }
    }
  else
    {
      memset (hd->mo_lie_arr, 0, sizeof (hd->mo_lie_arr));
      memcpy (hd->mo_lie_arr, keeparr, (nbhd - 1) * sizeof (mo_objref_t));
    }
}                               /* end of mo_list_pop_head */

void
mo_list_pop_tail (mo_listpayl_ty * lis)
{
  if (!mo_dyncastpayl_list (lis))
    return;
  mo_listelem_ty *tl = lis->mo_lip_last;
  if (!tl)
    return;
  int nbtl = 0;
  mo_value_t keeparr[MOM_LISTCHUNK_LEN];
  memset (keeparr, 0, sizeof (keeparr));
  for (int ix = 0; ix < MOM_LISTCHUNK_LEN; ix++)
    if (tl->mo_lie_arr[ix] != NULL)
      {
        if (nbtl > 0)
          keeparr[nbtl - 1] = tl->mo_lie_arr[ix];
        nbtl++;
      };
  MOM_ASSERTPRINTF (nbtl > 0, "zero nbtl");
  if (nbtl == 1)
    {
      if (tl == lis->mo_lip_first)
        {
          lis->mo_lip_first = lis->mo_lip_last = NULL;
        }
      else
        {
          lis->mo_lip_last = tl->mo_lie_prev;
        }
    }
  else
    {
      memset (tl->mo_lie_arr, 0, sizeof (tl->mo_lie_arr));
      memcpy (tl->mo_lie_arr, keeparr, (nbtl - 1) * sizeof (mo_objref_t));
    }
}                               /* end of mo_list_pop_tail */


mo_vectvaldatapayl_ty *
mo_list_to_vectvaldata (mo_listpayl_ty * lis)
{
  if (!mo_dyncastpayl_list (lis))
    return NULL;
  unsigned ln = mo_list_length (lis);
  mo_vectvaldatapayl_ty *vec = mo_vectval_reserve (NULL, ln);
  unsigned cnt = 0;
  for (mo_listelem_ty * el = lis->mo_lip_first; el != NULL;
       el = el->mo_lie_next)
    {
      for (int ix = 0; ix < MOM_LISTCHUNK_LEN; ix++)
        if (el->mo_lie_arr[ix])
          vec = mo_vectval_append (vec, el->mo_lie_arr[ix]);
      MOM_ASSERTPRINTF (cnt <= ln, "overflow cnt=%u", cnt);
    }
  return vec;
}                               /* end of mo_list_to_vectvaldata */

mo_value_t
mo_list_to_tuple (mo_listpayl_ty * lis)
{
  if (!mo_dyncastpayl_list (lis))
    return NULL;
  unsigned ln = mo_list_length (lis);
  mo_sequencevalue_ty *seq = mo_sequence_allocate (ln);
  unsigned cnt = 0;
  for (mo_listelem_ty * el = lis->mo_lip_first; el != NULL;
       el = el->mo_lie_next)
    {
      for (int ix = 0; ix < MOM_LISTCHUNK_LEN; ix++)
        {
          mo_objref_t obr = mo_dyncast_objref (el->mo_lie_arr[ix]);
          if (obr != NULL)
            seq->mo_seqobj[cnt++] = obr;
        }
      MOM_ASSERTPRINTF (cnt <= ln, "overflow cnt=%u", cnt);
    }
  return mo_make_tuple_closeq (seq);
}                               /* end of mo_list_to_tuple */

// eof list.c
