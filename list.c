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
      MOM_ASSERTPRINTF(lis->mo_lip_first == NULL, "bad lis@%p", lis);
      tl = mom_gc_alloc (sizeof (mo_listelem_ty));
      tl->mo_lie_arr[0] = v;
      lis->mo_lip_first = lis->mo_lip_last = tl;
      return;
    };
  MOM_ASSERTPRINTF (tl->mo_lie_next == NULL, "wrong lis@%p", lis);
  mo_value_t keeparr[MOM_LISTCHUNK_LEN];
  memset (keeparr, 0, sizeof (keeparr));
  int nbel = 0;
  for (int ix = 0; ix < MOM_LISTCHUNK_LEN; ix++)
    {
      if (tl->mo_lie_arr[ix] != NULL)
        keeparr[nbel++] = tl->mo_lie_arr[ix];
    }
  MOM_ASSERTPRINTF (nbel > 0, "wrong lis@%p", lis);
  if (nbel < MOM_LISTCHUNK_LEN)
    {
      keeparr[nbel++] = v;
      memcpy (tl->mo_lie_arr, keeparr, sizeof (keeparr));
      return;
    };
  // tl, the last element, is full....
  mo_listelem_ty *newel = mom_gc_alloc (sizeof (mo_listelem_ty));
  newel->mo_lie_prev = tl;
  tl->mo_lie_next = newel;
  lis->mo_lip_last = newel;
  newel->mo_lie_arr[0] = v;
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
      MOM_ASSERTPRINTF (lis->mo_lip_last == NULL, "non-nil last lis@%p", lis);
      mo_listelem_ty *newel = mom_gc_alloc (sizeof (mo_listelem_ty));
      newel->mo_lie_arr[0] = v;
      lis->mo_lip_first = lis->mo_lip_last = newel;
      return;
    }
  MOM_ASSERTPRINTF (hd->mo_lie_prev == NULL, "non-first head lis@%p", lis);
  mo_value_t keeparr[MOM_LISTCHUNK_LEN];
  memset (keeparr, 0, sizeof (keeparr));
  int nbel = 0;
  for (int ix = 0; ix < MOM_LISTCHUNK_LEN; ix++)
    {
      if (hd->mo_lie_arr[ix] != NULL)
        keeparr[nbel++] = hd->mo_lie_arr[ix];
    }
  MOM_ASSERTPRINTF (nbel > 0, "wrong lis@%p", lis);
  if (nbel < MOM_LISTCHUNK_LEN)
    {
      hd->mo_lie_arr[0] = v;
      memcpy (hd->mo_lie_arr + 1, keeparr,
              (MOM_LISTCHUNK_LEN - 1) * sizeof (mo_objref_t));
      return;
    }
  // hd, the first element,  is full....
  mo_listelem_ty *newel = mom_gc_alloc (sizeof (mo_listelem_ty));
  newel->mo_lie_next = hd;
  hd->mo_lie_prev = newel;
  lis->mo_lip_first = newel;
  newel->mo_lie_arr[0] = v;
}                               /* end mo_list_prepend */


void
mo_list_pop_head (mo_listpayl_ty * lis)
{
  if (!mo_dyncastpayl_list (lis))
    return;
  mo_listelem_ty *hd = lis->mo_lip_first;
  if (!hd)
    return;
  MOM_ASSERTPRINTF (hd->mo_lie_prev == NULL, "non-first head lis@%p", lis);
  mo_value_t keeparr[MOM_LISTCHUNK_LEN];
  memset (keeparr, 0, sizeof (keeparr));
  int nbel = 0;
  for (int ix = 0; ix < MOM_LISTCHUNK_LEN; ix++)
    {
      if (hd->mo_lie_arr[ix] != NULL)
        keeparr[nbel++] = hd->mo_lie_arr[ix];
    }
  MOM_ASSERTPRINTF (nbel > 0, "wrong lis@%p", lis);
  if (nbel > 1)
    {
      memcpy (hd->mo_lie_arr, keeparr + 1,
              (MOM_LISTCHUNK_LEN - 1) * sizeof (mo_objref_t));
      hd->mo_lie_arr[MOM_LISTCHUNK_LEN - 1] = NULL;
      return;
    }
  if (lis->mo_lip_last == hd)
    {
      lis->mo_lip_first = NULL;
      lis->mo_lip_last = NULL;
    }
  else
    {
      mo_listelem_ty *nxhd = hd->mo_lie_next;
      MOM_ASSERTPRINTF (nxhd != NULL
                        && nxhd->mo_lie_prev == hd, "bad nxhd lis@%p", lis);
      lis->mo_lip_first = nxhd;
      nxhd->mo_lie_prev = NULL;
    }
  memset (hd, 0, sizeof (*hd)); // to be GC friendly
}                               /* end of mo_list_pop_head */


void
mo_list_pop_tail (mo_listpayl_ty * lis)
{
  if (!mo_dyncastpayl_list (lis))
    return;
  mo_listelem_ty *tl = lis->mo_lip_last;
  if (!tl)
    return;
  int nbel = 0;
  mo_value_t keeparr[MOM_LISTCHUNK_LEN];
  memset (keeparr, 0, sizeof (keeparr));
  for (int ix = 0; ix < MOM_LISTCHUNK_LEN; ix++)
    {
      if (tl->mo_lie_arr[ix] != NULL)
        keeparr[nbel++] = tl->mo_lie_arr[ix];
    }
  MOM_ASSERTPRINTF (nbel > 0, "wrong lis@%p", lis);
  if (nbel > 1)
    {
      memcpy (tl->mo_lie_arr, keeparr,
              (MOM_LISTCHUNK_LEN - 1) * sizeof (mo_objref_t));
      tl->mo_lie_arr[MOM_LISTCHUNK_LEN - 1] = NULL;
      return;
    }
  if (lis->mo_lip_first == tl)
    {
      lis->mo_lip_first = NULL;
      lis->mo_lip_last = NULL;
    }
  else
    {
      mo_listelem_ty *pvtl = tl->mo_lie_prev;
      MOM_ASSERTPRINTF (pvtl != NULL
                        && pvtl->mo_lie_next == tl, "bad lis@%p", lis);
      lis->mo_lip_last = pvtl;
      pvtl->mo_lie_next = NULL;
    }
  memset (tl, 0, sizeof (*tl)); // to be GC friendly
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
