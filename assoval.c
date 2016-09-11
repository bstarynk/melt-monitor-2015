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

#define MOM_ASSOVAL_SMALLTHRESHOLD 8    // below that size, seek sequentially

/* in commit 31d058a4f8ffe99d7... we have a bug with
   asso->mo_seqent[0].mo_asso_val containing (void*)2 */
#define MOM_BUGGYASSO(Asso) \
  ("buggyasso" != NULL && (Asso)->mo_asso_entarr[0].mo_asso_val==(mo_value_t)0x2)

static_assert (sizeof (mo_countedpayl_ty) <= sizeof (mo_assovaldatapayl_ty),
               "wrong size countedpayl vs assovaldatapayl");

// return the index where the obr could be found or put or else <0
static int
mom_assoval_index (mo_assovaldatapayl_ty * asso, mo_objref_t obr)
{
  MOM_ASSERTPRINTF (mo_dyncastpayl_assoval (asso) != NULL, "bad asso@%p",
                    asso);
  MOM_ASSERTPRINTF (mo_dyncast_objref (obr) != NULL, "bad obr@%p", obr);
  uint32_t sz = asso->mo_sva_size;
  MOM_ASSERTPRINTF (sz > 2, "bad sz=%u", sz);
  if (sz < MOM_ASSOVAL_SMALLTHRESHOLD)
    {
      for (unsigned ix = 0; ix < sz; ix++)
        {
          mo_objref_t obcur = asso->mo_asso_entarr[ix].mo_asso_obr;
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
          mo_objref_t obcur = asso->mo_asso_entarr[ix].mo_asso_obr;
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
          mo_objref_t obcur = asso->mo_asso_entarr[ix].mo_asso_obr;
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
  MOM_ASSERTPRINTF (!MOM_BUGGYASSO (asso), "buggy asso@%p", asso);
  int pos = mom_assoval_index (asso, obr);
  if (pos < 0)
    return NULL;
  MOM_ASSERTPRINTF (pos < (int) (asso->mo_sva_size), "bad pos%d", pos);
  if (asso->mo_asso_entarr[pos].mo_asso_obr != obr)
    return NULL;
  MOM_ASSERTPRINTF (asso->mo_asso_entarr[pos].mo_asso_val != NULL,
                    "corrupted asso pos=%d", pos);
  return asso->mo_asso_entarr[pos].mo_asso_val;
}




mo_assovaldatapayl_ty *
mo_assoval_put (mo_assovaldatapayl_ty * asso, mo_objref_t obr, mo_value_t va)
{
  if (va == NULL || va == MOM_EMPTY_SLOT)
    return mo_assoval_remove (asso, obr);
  if (mo_dyncast_objref (obr) == NULL)
    return mo_dyncastpayl_assoval (asso);
  asso = mo_dyncastpayl_assoval (asso);
  if (!asso)
    {
      unsigned sz = 3;
      MOM_ASSERTPRINTF (sz < MOM_ASSOVAL_SMALLTHRESHOLD,
                        "bad initial tiny sz=%u", sz);
      asso =
        mom_gc_alloc (sizeof (mo_assovaldatapayl_ty) +
                      sz * sizeof (struct mo_assoentry_st));
      asso->mo_va_kind = mo_PASSOVALDATA;
      asso->mo_va_hash = (momrand_genrand_int31 () & 0xfffffff) + 2;
      asso->mo_sva_size = sz;
      asso->mo_cpl_count = 1;
      asso->mo_asso_entarr[0].mo_asso_obr = obr;
      asso->mo_asso_entarr[0].mo_asso_val = va;
      MOM_ASSERTPRINTF (!MOM_BUGGYASSO (asso), "buggy asso@%p", asso);
      return asso;
    }                           // end if NULL asso
  unsigned sz = asso->mo_sva_size;
  MOM_ASSERTPRINTF (sz > 2, "too low sz=%u", sz);
  MOM_ASSERTPRINTF (!MOM_BUGGYASSO (asso), "buggy asso@%p", asso);
  unsigned cnt = asso->mo_cpl_count;
  MOM_ASSERTPRINTF (cnt <= sz, "cnt %u above sz %u", cnt, sz);
  if (sz < MOM_ASSOVAL_SMALLTHRESHOLD && cnt + 1 < sz)
    {
      int pos = mom_assoval_index (asso, obr);
      MOM_ASSERTPRINTF (pos >= 0
                        && pos < (int) sz, "wrong pos %d, sz=%u,cnt=%u", pos,
                        sz, cnt);
      if (asso->mo_asso_entarr[pos].mo_asso_obr == obr)
        asso->mo_asso_entarr[pos].mo_asso_val = va;
      else
        {
          asso->mo_asso_entarr[pos].mo_asso_obr = obr;
          asso->mo_asso_entarr[pos].mo_asso_val = va;
          asso->mo_cpl_count = cnt + 1;
        }
      MOM_ASSERTPRINTF (!MOM_BUGGYASSO (asso), "buggy asso@%p", asso);
      return asso;
    }                           // end if small and fits
  else if (4 * cnt + 3 < 3 * sz)
    {                           // 3/4-th full, don't need to grow
      MOM_ASSERTPRINTF (!MOM_BUGGYASSO (asso), "buggy asso@%p", asso);
      int pos = mom_assoval_index (asso, obr);
      MOM_ASSERTPRINTF (pos >= 0
                        && pos < (int) sz, "wrong pos %d, sz=%u,cnt=%u", pos,
                        sz, cnt);
      if (asso->mo_asso_entarr[pos].mo_asso_obr == obr)
        asso->mo_asso_entarr[pos].mo_asso_val = va;
      else
        {
          asso->mo_asso_entarr[pos].mo_asso_obr = obr;
          asso->mo_asso_entarr[pos].mo_asso_val = va;
          asso->mo_cpl_count = cnt + 1;
        }
      MOM_ASSERTPRINTF (!MOM_BUGGYASSO (asso), "buggy asso@%p", asso);
      return asso;
    }                           // no more than 3/4-th full
  else
    {                           // should grow
      unsigned oldcnt = cnt;
      unsigned oldsz = sz;
      unsigned gap = 2 + cnt / 64;
      if (cnt > 100)
        gap += cnt / 16 + cnt / 32 + 9;
      MOM_ASSERTPRINTF (!MOM_BUGGYASSO (asso), "buggy asso@%p", asso);
      asso = mo_assoval_reserve (asso, gap);
      MOM_ASSERTPRINTF (!MOM_BUGGYASSO (asso), "buggy asso@%p", asso);
      sz = asso->mo_sva_size;
      MOM_ASSERTPRINTF (oldsz < sz,
                        "oldsz=%u not less than sz=%u oldcnt=%u gap=%u",
                        oldsz, sz, oldcnt, gap);
      MOM_ASSERTPRINTF (sz > 2, "too low sz=%u (cnt=%u)", sz, cnt);
      cnt = asso->mo_cpl_count;
      MOM_ASSERTPRINTF (cnt < sz && cnt == oldcnt,
                        "cnt %u above sz %u or not same as oldcnt %u", cnt,
                        sz, oldcnt);
      int pos = mom_assoval_index (asso, obr);
      MOM_ASSERTPRINTF (pos >= 0
                        && pos < (int) sz, "wrong pos %d, sz=%u, cnt=%u", pos,
                        sz, cnt);
      if (asso->mo_asso_entarr[pos].mo_asso_obr == obr)
        asso->mo_asso_entarr[pos].mo_asso_val = va;
      else
        {
          asso->mo_asso_entarr[pos].mo_asso_obr = obr;
          asso->mo_asso_entarr[pos].mo_asso_val = va;
          asso->mo_cpl_count = cnt + 1;
        }
      MOM_ASSERTPRINTF (!MOM_BUGGYASSO (asso), "buggy asso@%p", asso);
      return asso;
    }
}                               /* end mo_assoval_put */



mo_assovaldatapayl_ty *
mo_assoval_remove (mo_assovaldatapayl_ty * asso, mo_objref_t obr)
{
  asso = mo_dyncastpayl_assoval (asso);
  if (!asso)
    return NULL;
  if (mo_dyncast_objref (obr) == NULL)
    return asso;
  unsigned sz = asso->mo_sva_size;
  MOM_ASSERTPRINTF (sz > 2, "too low sz=%u", sz);
  MOM_ASSERTPRINTF (!MOM_BUGGYASSO (asso), "buggy asso@%p", asso);
  int pos = mom_assoval_index (asso, obr);
  if (pos < 0)
    return asso;
  MOM_ASSERTPRINTF (pos >= 0 && pos < (int) sz, "wrong pos %d", pos);
  unsigned cnt = asso->mo_cpl_count;
  MOM_ASSERTPRINTF (cnt <= sz, "cnt %u above sz %u", cnt, sz);
  if (asso->mo_asso_entarr[pos].mo_asso_obr == obr)
    {
      MOM_ASSERTPRINTF (cnt > 0, "cnt was zero");
      asso->mo_asso_entarr[pos].mo_asso_obr = MOM_EMPTY_SLOT;
      asso->mo_asso_entarr[pos].mo_asso_val = NULL;
      asso->mo_cpl_count = cnt - 1;
      cnt--;
    };
  if (2 * cnt < sz && sz > MOM_ASSOVAL_SMALLTHRESHOLD)
    asso = mo_assoval_reserve (asso, 0);
  MOM_ASSERTPRINTF (!MOM_BUGGYASSO (asso), "buggy asso@%p", asso);
  return asso;
}                               /* end mo_assoval_remove */

mo_assovaldatapayl_ty *
mo_assoval_reserve (mo_assovaldatapayl_ty * asso, unsigned gap)
{
  asso = mo_dyncastpayl_assoval (asso);
  if (!asso)
    {
      unsigned sz = 0;
      if (gap <= 3)
        sz = 3;
      else if (gap <= 5)
        sz = 5;
      else if (gap <= 7)
        sz = 7;
      else
        sz = mom_prime_above (5 * gap / 4 + gap / 32 + 1);
      asso =
        mom_gc_alloc (sizeof (mo_assovaldatapayl_ty) +
                      sz * sizeof (struct mo_assoentry_st));
      asso->mo_va_kind = mo_PASSOVALDATA;
      asso->mo_va_hash = (momrand_genrand_int31 () & 0xfffffff) + 2;
      asso->mo_sva_size = sz;
      asso->mo_cpl_count = 0;
      MOM_ASSERTPRINTF (!MOM_BUGGYASSO (asso), "buggy asso@%p", asso);
      return asso;
    }
  MOM_ASSERTPRINTF (!MOM_BUGGYASSO (asso), "buggy asso@%p", asso);
  unsigned sz = asso->mo_sva_size;
  MOM_ASSERTPRINTF (sz > 2, "too low sz=%u", sz);
  unsigned cnt = asso->mo_cpl_count;
  MOM_ASSERTPRINTF (cnt <= sz, "cnt %u above sz %u", cnt, sz);
  unsigned newsz = 0;
  if (sz < MOM_ASSOVAL_SMALLTHRESHOLD && cnt + gap <= sz)
    return asso;
  else if (cnt + gap <= 3)
    newsz = 3;
  else if (cnt + gap <= 5)
    newsz = 5;
  else if (cnt + gap <= 7)
    newsz = 7;
  else
    newsz =                     //
      mom_prime_above (5 * (cnt + gap) / 4 + (cnt + gap) / 32 + gap / 32 + 1);
  if (newsz == sz)
    return asso;
  mo_assovaldatapayl_ty *newasso =
    mom_gc_alloc (sizeof (mo_assovaldatapayl_ty) +
                  newsz * sizeof (struct mo_assoentry_st));
  newasso->mo_va_kind = mo_PASSOVALDATA;
  newasso->mo_va_hash = (momrand_genrand_int31 () & 0xfffffff) + 2;
  newasso->mo_sva_size = newsz;
  newasso->mo_cpl_count = 0;
  unsigned newcount = 0;
  for (unsigned ix = 0; ix < sz; ix++)
    {
      mo_objref_t oldobr = asso->mo_asso_entarr[ix].mo_asso_obr;
      if (!oldobr || oldobr == MOM_EMPTY_SLOT)
        continue;
      MOM_ASSERTPRINTF (mo_dyncast_objref (oldobr) != NULL,
                        "corrupted ix=%u", ix);
      mo_value_t oldval = asso->mo_asso_entarr[ix].mo_asso_val;
      if (!oldval || oldval == MOM_EMPTY_SLOT)
        continue;
      int newpos = mom_assoval_index (newasso, oldobr);
      MOM_ASSERTPRINTF (newpos >= 0 && newpos < (int) newsz
                        && newasso->mo_asso_entarr[newpos].mo_asso_obr ==
                        NULL, "bad newpos=%d", newpos);
      newasso->mo_asso_entarr[newpos].mo_asso_obr = oldobr;
      newasso->mo_asso_entarr[newpos].mo_asso_val = oldval;
      newcount++;
    }
  MOM_ASSERTPRINTF (newcount == cnt,
                    "count corruption newcount=%u cnt=%u", newcount, cnt);
  newasso->mo_cpl_count = newcount;
  MOM_ASSERTPRINTF (!MOM_BUGGYASSO (newasso),
                    "buggy newasso@%p for asso@%p", newasso, asso);
  MOM_ASSERTPRINTF (!MOM_BUGGYASSO (asso), "buggy asso@%p with newasso@%p",
                    asso, newasso);
  return newasso;
}                               /* end of mo_assoval_reserve */


mo_value_t
mo_assoval_keys_set (mo_assovaldatapayl_ty * asso)
{
  asso = mo_dyncastpayl_assoval (asso);
  if (!asso)
    return NULL;
  MOM_ASSERTPRINTF (!MOM_BUGGYASSO (asso), "buggy asso@%p", asso);
  unsigned sz = asso->mo_sva_size;
  MOM_ASSERTPRINTF (sz > 2, "too low sz=%u", sz);
  unsigned cnt = asso->mo_cpl_count;
  MOM_ASSERTPRINTF (cnt <= sz, "cnt %u above sz %u", cnt, sz);
  if (cnt == 0)
    return mo_make_empty_set ();
  unsigned nb = 0;
  mo_sequencevalue_ty *sq = mo_sequence_allocate (cnt);
  for (unsigned ix = 0; ix < sz; ix++)
    {
      mo_objref_t oldobr = asso->mo_asso_entarr[ix].mo_asso_obr;
      if (!oldobr || oldobr == MOM_EMPTY_SLOT)
        continue;
      MOM_ASSERTPRINTF (mo_dyncast_objref (oldobr) != NULL,
                        "corrupted ix=%u", ix);
      mo_value_t oldval = asso->mo_asso_entarr[ix].mo_asso_val;
      if (!oldval || oldval == MOM_EMPTY_SLOT)
        continue;
      MOM_ASSERTPRINTF (nb < cnt, "too big nb=%u for cnt=%u", nb, cnt);
      sq->mo_seqobj[nb++] = oldobr;
    }
  MOM_ASSERTPRINTF (nb == cnt, "cnt %u not same as nb %u", cnt, nb);
  MOM_ASSERTPRINTF (!MOM_BUGGYASSO (asso), "buggy asso@%p", asso);
  return mo_make_set_closeq (sq);
}

void
mo_dump_scan_assoval (mo_dumper_ty * du, mo_assovaldatapayl_ty * asso)
{
  if (!mo_dyncastpayl_assoval (asso))
    return;
  MOM_ASSERTPRINTF (mo_dump_scanning (du), "bad du");
  unsigned sz = asso->mo_sva_size;
  MOM_ASSERTPRINTF (sz > 2, "too low sz=%u", sz);
  MOM_ASSERTPRINTF (!MOM_BUGGYASSO (asso), "buggy asso@%p", asso);
  unsigned cnt = asso->mo_cpl_count;
  MOM_ASSERTPRINTF (cnt <= sz, "cnt %u above sz %u", cnt, sz);
  if (cnt == 0)
    return;
  for (unsigned ix = 0; ix < sz; ix++)
    {
      mo_objref_t obr = asso->mo_asso_entarr[ix].mo_asso_obr;
      if (!obr || obr == MOM_EMPTY_SLOT)
        continue;
      MOM_ASSERTPRINTF (mo_dyncast_objref (obr) != NULL,
                        "corrupted ix=%u", ix);
      mo_value_t val = asso->mo_asso_entarr[ix].mo_asso_val;
      if (!val || val == MOM_EMPTY_SLOT)
        continue;
      mo_dump_scan_objref (du, obr);
      mo_dump_scan_value (du, val);
    }
  MOM_ASSERTPRINTF (!MOM_BUGGYASSO (asso), "buggy asso@%p", asso);
}                               /* end of mo_dump_scan_assoval */

mo_json_t
mo_dump_json_of_assoval (mo_dumper_ty * du, mo_assovaldatapayl_ty * asso)
{
  MOM_ASSERTPRINTF (mo_dump_emitting (du), "bad du");
  if (!mo_dyncastpayl_assoval (asso))
    return json_null ();
  MOM_ASSERTPRINTF (!MOM_BUGGYASSO (asso), "buggy asso@%p", asso);
  mo_value_t ksetv = mo_assoval_keys_set (asso);
  unsigned nbkeys = mo_set_size (ksetv);
  json_t *jarr = json_array ();
  for (unsigned ix = 0; ix < nbkeys; ix++)
    {
      mo_objref_t keyobr = mo_set_nth (ksetv, ix);
      if (!mo_dyncast_objref (keyobr)
          || !mo_dump_is_emitted_objref (du, keyobr))
        continue;
      mo_value_t valv = mo_assoval_get (asso, keyobr);
      mo_objref_t valobr = mo_dyncast_objref (valv);
      if (valobr && !mo_dump_is_emitted_objref (du, valobr))
        continue;
      json_array_append_new (jarr, json_pack ("{soso}",
                                              "at",
                                              mo_dump_jsonid_of_objref (du,
                                                                        keyobr),
                                              "va", mo_dump_json_of_value (du,
                                                                           valv)));
    }
  return json_pack ("{so}", "assoval", jarr);
}                               /* end of mo_dump_json_of_assoval */



mo_assovaldatapayl_ty *
mo_assoval_of_json (mo_json_t js)
{
  json_t *jarr = NULL;
  if (json_is_object (js) && (jarr = json_object_get (js, "assoval")) != NULL
      && json_is_array (jarr))
    {
      unsigned sz = json_array_size (jarr);
      mo_assovaldatapayl_ty *asso =
        mo_assoval_reserve (NULL, sz + sz / 16 + 1);
      for (unsigned ix = 0; ix < sz; ix++)
        {
          json_t *jpair = json_array_get (jarr, ix);
          if (!json_is_object (jpair))
            continue;
          json_t *jat = json_object_get (jpair, "at");
          if (!jat)
            continue;
          json_t *jva = json_object_get (jpair, "va");
          if (!jva)
            continue;
          mo_objref_t atobr = mo_objref_of_jsonid (jat);
          if (!atobr)
            continue;
          mo_value_t valv = mo_value_of_json (jva);
          if (!valv)
            continue;
          asso = mo_assoval_put (asso, atobr, valv);
        }
      MOM_ASSERTPRINTF (!MOM_BUGGYASSO (asso), "buggy asso@%p", asso);
      return asso;
    }
  return NULL;
}                               /* end of mo_assoval_of_json */

/// eof assoval.c
