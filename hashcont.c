// file hashcont.c - hashed containers: set of objects, integer hash maps

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

// return the index where the obr could be found or put or else <0
static int
mom_hset_index (mo_hashsetpayl_ty * hset, mo_objref_t obr)
{
  MOM_ASSERTPRINTF (mo_dyncastpayl_hashset (hset) != NULL, "bad hset@%p",
                    hset);
  MOM_ASSERTPRINTF (mo_dyncast_objref (obr) != NULL, "bad obr@%p", obr);
  uint32_t sz = hset->mo_sva_size;
  MOM_ASSERTPRINTF (sz > 2, "bad sz=%u", sz);
  momhash_t h = mo_objref_hash (obr);
  MOM_ASSERTPRINTF (h != 0, "zero h");
  unsigned startix = h % sz;
  int pos = -1;
  for (unsigned ix = startix; ix < sz; ix++)
    {
      mo_objref_t obcur = hset->mo_hset_arr[ix];
      if (obcur == obr)
        return (int) ix;
      else if (obcur == MOM_EMPTY_SLOT)
        {
          if (pos < 0)
            pos = (int) ix;
          continue;
        }
      else if (obcur == NULL)
        {
          if (pos < 0)
            pos = (int) ix;
          return pos;
        }
    }
  for (unsigned ix = 0; ix < startix; ix++)
    {
      mo_objref_t obcur = hset->mo_hset_arr[ix];
      if (obcur == obr)
        return (int) ix;
      else if (obcur == MOM_EMPTY_SLOT)
        {
          if (pos < 0)
            pos = (int) ix;
          continue;
        }
      else if (obcur == NULL)
        {
          if (pos < 0)
            pos = (int) ix;
          return pos;
        }
    }
  return pos;
}                               /* end of mom_hset_index */

bool
mo_hashset_contains (mo_hashsetpayl_ty * hset, mo_objref_t obr)
{
  hset = mo_dyncastpayl_hashset (hset);
  if (!hset)
    return false;
  obr = mo_dyncast_objref (obr);
  if (!obr)
    return false;
  int pos = mom_hset_index (hset, obr);
  if (pos < 0)
    return false;
  return hset->mo_hset_arr[pos] == obr;
}                               /* end mo_hashset_contains */

mo_hashsetpayl_ty *
mo_hashset_reserve (mo_hashsetpayl_ty * hset, unsigned gap)
{
  hset = mo_dyncastpayl_hashset (hset);
  if (!hset)
    {
      if (gap > MOM_SIZE_MAX)
        MOM_FATAPRINTF ("hashset_reserve: too big initial gap %u", gap);
      unsigned sz = mom_prime_above (5 * gap / 4 + gap / 32 + 4);
      hset =
        mom_gc_alloc (sizeof (mo_hashsetpayl_ty) + sz * sizeof (mo_objref_t));
      hset->mo_va_kind = mo_PHASHSET;
      hset->mo_va_hash = (momrand_genrand_int31 () & 0xfffffff) + 2;
      hset->mo_sva_size = sz;
      hset->mo_cpl_count = 0;
      return hset;
    }
  unsigned sz = hset->mo_sva_size;
  unsigned cnt = hset->mo_cpl_count;
  if (4 * (cnt + gap) < 3 * sz && (sz < 60 || 3 * (cnt + gap) > sz))
    return hset;
  unsigned oldsz = sz;
  unsigned oldcnt = cnt;
  mo_hashsetpayl_ty *oldhset = hset;
  if (cnt < 200)
    sz = mom_prime_above (5 * (cnt + gap) / 4 + (cnt + gap) / 32 + 5);
  else
    sz = mom_prime_above (5 * (cnt + gap) / 4 + (cnt + gap) / 8 + 16);
  hset =
    mom_gc_alloc (sizeof (mo_hashsetpayl_ty) + sz * sizeof (mo_objref_t));
  hset->mo_va_kind = mo_PHASHSET;
  hset->mo_va_hash = (momrand_genrand_int31 () & 0xfffffff) + 2;
  hset->mo_sva_size = sz;
  hset->mo_cpl_count = 0;
  cnt = 0;
  for (unsigned oldix = 0; oldix < oldsz; oldix++)
    {
      mo_objref_t oldobr = oldhset->mo_hset_arr[oldix];
      if (!oldobr || oldobr == MOM_EMPTY_SLOT)
        continue;
      MOM_ASSERTPRINTF (mo_dyncast_objref (oldobr),
                        "bad oldobr at oldix=%u", oldix);
      int pos = mom_hset_index (hset, oldobr);
      MOM_ASSERTPRINTF (pos >= 0 && pos < (int) sz
                        && hset->mo_hset_arr[pos] == NULL,
                        "corrupted new hset pos=%d", pos);
      hset->mo_hset_arr[pos] = oldobr;
      cnt++;
    }
  MOM_ASSERTPRINTF (oldcnt == cnt, "oldcnt %u not same as cnt %u", oldcnt,
                    cnt);
  hset->mo_cpl_count = cnt;
  return hset;
}                               /* end mo_hashset_reserve */

mo_hashsetpayl_ty *
mo_hashset_put (mo_hashsetpayl_ty * hset, mo_objref_t obr)
{
  hset = mo_dyncastpayl_hashset (hset);
  obr = mo_dyncast_objref (obr);
  if (!obr)
    return hset;
  if (!hset)
    {
      hset = mo_hashset_reserve (NULL, 4);
      int pos = mom_hset_index (hset, obr);
      MOM_ASSERTPRINTF (pos >= 0, "bad pos");
      hset->mo_hset_arr[pos] = obr;
      hset->mo_cpl_count = 1;
      return hset;
    }
  unsigned sz = hset->mo_sva_size;
  unsigned cnt = hset->mo_cpl_count;
  if (4 * cnt + 6 >= 3 * sz)
    {
      hset = mo_hashset_reserve (hset, cnt / 64 + 3);
      sz = hset->mo_sva_size;
      MOM_ASSERTPRINTF (cnt == hset->mo_cpl_count,
                        "count should not change cnt=%u", cnt);
    }
  int pos = mom_hset_index (hset, obr);
  MOM_ASSERTPRINTF (pos >= 0 && pos < (int) sz, "bad pos");
  if (hset->mo_hset_arr[pos] == obr)
    return hset;
  hset->mo_hset_arr[pos] = obr;
  hset->mo_cpl_count = cnt + 1;
  return hset;
}                               /* end mo_hashset_remove */

mo_hashsetpayl_ty *
mo_hashset_remove (mo_hashsetpayl_ty * hset, mo_objref_t obr)
{
  hset = mo_dyncastpayl_hashset (hset);
  obr = mo_dyncast_objref (obr);
  if (!obr)
    return hset;
  if (!hset)
    return NULL;
  unsigned sz = hset->mo_sva_size;
  unsigned cnt = hset->mo_cpl_count;
  int pos = mom_hset_index (hset, obr);
  if (pos < 0 || hset->mo_hset_arr[pos] != obr)
    return hset;
  MOM_ASSERTPRINTF (cnt > 0, "zero cnt");
  hset->mo_hset_arr[pos] = MOM_EMPTY_SLOT;
  hset->mo_cpl_count = cnt - 1;
  if (sz > 60 && 3 * cnt < sz)
    hset = mo_hashset_reserve (hset, 0);
  return hset;
}                               /* end mo_hashset_remove */


mo_value_t
mo_hashset_elements_set (mo_hashsetpayl_ty * hset)
{
  hset = mo_dyncastpayl_hashset (hset);
  if (!hset)
    return NULL;
  unsigned sz = hset->mo_sva_size;
  unsigned cnt = hset->mo_cpl_count;
  MOM_ASSERTPRINTF (sz > 2, "too low sz=%u", sz);
  unsigned nb = 0;
  MOM_ASSERTPRINTF (cnt <= sz && cnt <= MOM_SIZE_MAX,
                    "cnt %u above sz %u or too big", cnt, sz);
  if (cnt == 0)
    return mo_make_empty_set ();
  nb = 0;
  mo_sequencevalue_ty *sq = mo_sequence_allocate (cnt);
  for (unsigned ix = 0; ix < sz; ix++)
    {
      mo_objref_t obr = hset->mo_hset_arr[ix];
      if (!obr || obr == MOM_EMPTY_SLOT)
        continue;
      MOM_ASSERTPRINTF (mo_dyncast_objref (obr), "bad obr at ix=%d", ix);
      sq->mo_seqobj[nb++] = obr;
      MOM_ASSERTPRINTF (nb <= cnt, "too big nb=%u for cnt=%u", nb, cnt);
    }
  MOM_ASSERTPRINTF (nb == cnt, "cnt %u not same as nb %u", cnt, nb);
  return mo_make_set_closeq (sq);
}                               /* end mo_hashset_elements_set */


void
mo_dump_scan_hashset (mo_dumper_ty * du, mo_hashsetpayl_ty * hset)
{
  MOM_ASSERTPRINTF (mo_dump_scanning (du), "bad du");
  hset = mo_dyncastpayl_hashset (hset);
  if (!hset)
    return;
  unsigned sz = hset->mo_sva_size;
  unsigned cnt = hset->mo_cpl_count;
  MOM_ASSERTPRINTF (sz > 2, "too low sz=%u", sz);
  MOM_ASSERTPRINTF (cnt <= sz, "too big cnt=%u sz=%u", cnt, sz);
  for (unsigned ix = 0; ix < sz; ix++)
    {
      mo_objref_t obr = hset->mo_hset_arr[ix];
      if (!obr || obr == MOM_EMPTY_SLOT)
        continue;
      MOM_ASSERTPRINTF (mo_dyncast_objref (obr), "bad obr at ix=%d", ix);
      mo_dump_scan_objref (du, obr);
    }
}                               /* end mo_dump_scan_hashset */



mo_json_t
mo_dump_json_of_hashset (mo_dumper_ty * du, mo_hashsetpayl_ty * hset)
{
  MOM_ASSERTPRINTF (mo_dump_emitting (du), "bad du");
  hset = mo_dyncastpayl_hashset (hset);
  if (!hset)
    return json_null ();
  mo_setvalue_ty *elemset = (mo_setvalue_ty *) mo_hashset_elements_set (hset);
  MOM_ASSERTPRINTF (mo_dyncast_set (elemset), "bad elemsetv");
  unsigned sz = mo_set_size (elemset);
  json_t *jarr = json_array ();
  for (unsigned ix = 0; ix < sz; ix++)
    {
      mo_objref_t elobr = elemset->mo_seqobj[ix];
      if (!mo_dump_is_emitted_objref (du, elobr))
        continue;
      json_t *jcomp = mo_dump_jsonid_of_objref (du, elobr);
      json_array_append_new (jarr, jcomp);
    }
  return json_pack ("{so}", "hashset", jarr);
}                               /* end mo_dump_json_of_hashset */



mo_hashsetpayl_ty *
mo_hashset_of_json (mo_json_t js)
{
  json_t *jarr = NULL;
  if (!json_is_object (js) || !(jarr = json_object_get (js, "hashset"))
      || !json_is_array (jarr))
    return NULL;
  unsigned sz = json_array_size (jarr);
  mo_hashsetpayl_ty *hset = mo_hashset_reserve (NULL, sz + sz / 3 + 4);
  for (unsigned ix = 0; ix < sz; ix++)
    {
      json_t *jelem = json_array_get (jarr, ix);
      if (!jelem)
        continue;
      mo_objref_t obr = mo_objref_of_jsonid (jelem);
      if (!obr)
        continue;
      hset = mo_hashset_put (hset, obr);
    }
  return hset;
}                               /* end mo_hashset_of_json */



/*****************************************************************
 * Hashed integer maps 
 *****************************************************************/


// return the index where the key could be found or put or else <0
static int
mom_inthmap_index (mo_inthmappayl_ty * ihmap, int64_t key)
{
  MOM_ASSERTPRINTF (mo_dyncastpayl_inthmap (ihmap) != NULL, "bad ihmap@%p",
                    ihmap);
  uint32_t sz = ihmap->mo_sva_size;
  MOM_ASSERTPRINTF (sz > 2, "bad sz=%u", sz);
  momhash_t h = mo_hash_int64 (key);
  MOM_ASSERTPRINTF (h != 0, "zero h");
  unsigned startix = h % sz;
  int pos = -1;
  for (unsigned ix = startix; ix < sz; ix++)
    {
      int64_t curkey = ihmap->mo_inthm_entarr[ix].mo_inthe_key;
      if (curkey == key)
        return (int) ix;
      mo_value_t curval = ihmap->mo_inthm_entarr[ix].mo_inthe_val;
      if (curval == MOM_EMPTY_SLOT)
        {
          if (pos < 0)
            pos = (int) ix;
          continue;
        }
      else if (curval == NULL)
        {
          if (pos < 0)
            pos = (int) ix;
          return pos;
        }
    }
  for (unsigned ix = 0; ix < startix; ix++)
    {
      int64_t curkey = ihmap->mo_inthm_entarr[ix].mo_inthe_key;
      if (curkey == key)
        return (int) ix;
      mo_value_t curval = ihmap->mo_inthm_entarr[ix].mo_inthe_val;
      if (curval == MOM_EMPTY_SLOT)
        {
          if (pos < 0)
            pos = (int) ix;
          continue;
        }
      else if (curval == NULL)
        {
          if (pos < 0)
            pos = (int) ix;
          return pos;
        }
    }
  return pos;
}                               /* end of mom_inthmap_index */

mo_inthmappayl_ty *
mo_inthmap_reserve (mo_inthmappayl_ty * ihmap, unsigned gap)
{
  ihmap = mo_dyncastpayl_inthmap (ihmap);
  if (!ihmap)
    {
      if (gap > MOM_SIZE_MAX)
        MOM_FATAPRINTF ("inthmap_reserve: too big initial gap %u", gap);
      unsigned siz = mom_prime_above (5 * gap / 4 + gap / 32 + 10);
      ihmap =
        mom_gc_alloc (sizeof (mo_inthmappayl_ty) +
                      siz * sizeof (struct mo_inthentry_st));
      ihmap->mo_va_kind = mo_PINTHMAP;
      ihmap->mo_va_hash = (momrand_genrand_int31 () & 0xfffffff) + 2;
      ihmap->mo_sva_size = siz;
      ihmap->mo_cpl_count = 0;
      return ihmap;
    }
  else
    {
      unsigned cnt = ihmap->mo_cpl_count;
      unsigned siz = ihmap->mo_sva_size;
      if (4 * (cnt + gap) < 3 * siz && (siz < 60 || 3 * (cnt + gap) > siz))
        return ihmap;
      unsigned oldsiz = siz;
      unsigned oldcnt = cnt;
      mo_inthmappayl_ty *oldihmap = ihmap;
      if (cnt < 200)
        siz = mom_prime_above (5 * (cnt + gap) / 4 + (cnt + gap) / 32 + 5);
      else
        siz = mom_prime_above (5 * (cnt + gap) / 4 + (cnt + gap) / 8 + 16);
      ihmap =
        mom_gc_alloc (sizeof (mo_inthmappayl_ty) +
                      siz * sizeof (struct mo_inthentry_st));
      ihmap->mo_va_kind = mo_PINTHMAP;
      ihmap->mo_va_hash = (momrand_genrand_int31 () & 0xfffffff) + 2;
      ihmap->mo_sva_size = siz;
      ihmap->mo_cpl_count = 0;
      cnt = 0;
      for (unsigned oldix = 0; oldix < oldsiz; oldix++)
        {
          mo_value_t oldval = oldihmap->mo_inthm_entarr[oldix].mo_inthe_val;
          if (!oldval || oldval == MOM_EMPTY_SLOT)
            continue;
          int64_t oldkey = oldihmap->mo_inthm_entarr[oldix].mo_inthe_key;
          int pos = mom_inthmap_index (ihmap, oldkey);
          MOM_ASSERTPRINTF (pos >= 0 && pos < (int) siz
                            && ihmap->mo_inthm_entarr[pos].mo_inthe_val ==
                            NULL, "corrupted new inthmap pos=%d", pos);
          ihmap->mo_inthm_entarr[pos].mo_inthe_key = oldkey;
          ihmap->mo_inthm_entarr[pos].mo_inthe_val = oldval;
          cnt++;
        }
      MOM_ASSERTPRINTF (oldcnt == cnt, "oldcnt %u not same as cnt %u", oldcnt,
                        cnt);
      ihmap->mo_cpl_count = cnt;
      return ihmap;
    };
}                               /* end of mo_inthmap_reserve */



mo_value_t
mo_inthmap_get (mo_inthmappayl_ty * ihmap, int64_t key)
{
  if (!ihmap)
    return NULL;
  ihmap = mo_dyncastpayl_inthmap (ihmap);
  if (!ihmap)
    return NULL;
  unsigned siz = ihmap->mo_sva_size;
  int pos = mom_inthmap_index (ihmap, key);
  if (pos >= 0)
    {
      MOM_ASSERTPRINTF (pos >= 0 && pos < (int) siz, "bad pos");
      if (ihmap->mo_inthm_entarr[pos].mo_inthe_key == key)
        return ihmap->mo_inthm_entarr[pos].mo_inthe_val;
    }
  return NULL;
}                               /* end of mo_inthmap_get */



mo_inthmappayl_ty *
mo_inthmap_put (mo_inthmappayl_ty * ihmap, int64_t key, mo_value_t val)
{
  ihmap = mo_dyncastpayl_inthmap (ihmap);
  if (!val || val == MOM_EMPTY_SLOT)
    return mo_inthmap_remove (ihmap, key);
  if (!ihmap)
    {
      ihmap = mo_inthmap_reserve (NULL, 4);
      int pos = mom_inthmap_index (ihmap, key);
      MOM_ASSERTPRINTF (pos >= 0, "bad pos");
      ihmap->mo_inthm_entarr[pos].mo_inthe_key = key;
      ihmap->mo_inthm_entarr[pos].mo_inthe_val = val;
      ihmap->mo_cpl_count = 1;
      return ihmap;
    }
  unsigned cnt = ihmap->mo_cpl_count;
  unsigned siz = ihmap->mo_sva_size;
  if (4 * cnt + 6 >= 3 * siz)
    {
      ihmap = mo_inthmap_reserve (ihmap, cnt / 64 + 3);
      siz = ihmap->mo_sva_size;
      MOM_ASSERTPRINTF (cnt == ihmap->mo_cpl_count,
                        "count should not change cnt=%u", cnt);
    }
  int pos = mom_inthmap_index (ihmap, key);
  MOM_ASSERTPRINTF (pos >= 0 && pos < (int) siz, "bad pos");
  if (ihmap->mo_inthm_entarr[pos].mo_inthe_key == key)
    {
      mo_value_t oldval = ihmap->mo_inthm_entarr[pos].mo_inthe_val;
      if (oldval == NULL || oldval == MOM_EMPTY_SLOT)
        ihmap->mo_cpl_count = cnt + 1;
      ihmap->mo_inthm_entarr[pos].mo_inthe_val = val;
    }
  else
    {
      ihmap->mo_inthm_entarr[pos].mo_inthe_key = key;
      ihmap->mo_inthm_entarr[pos].mo_inthe_val = val;
      ihmap->mo_cpl_count = cnt + 1;
    }
  return ihmap;
}                               /* end of mo_inthmap_put */




mo_inthmappayl_ty *
mo_inthmap_remove (mo_inthmappayl_ty * ihmap, int64_t key)
{
  ihmap = mo_dyncastpayl_inthmap (ihmap);
  if (!ihmap)
    return NULL;
  int pos = mom_inthmap_index (ihmap, key);
  if (pos < 0)
    return ihmap;
  unsigned cnt = ihmap->mo_cpl_count;
  unsigned siz = ihmap->mo_sva_size;
  MOM_ASSERTPRINTF (pos < (int) siz, "inthmap_remove: bad pos %d for size %u",
                    pos, siz);
  if (ihmap->mo_inthm_entarr[pos].mo_inthe_key == key)
    {
      MOM_ASSERTPRINTF (cnt > 0 && cnt <= siz, "inthmap_remove: bad cnt");
      ihmap->mo_inthm_entarr[pos].mo_inthe_val = MOM_EMPTY_SLOT;
      ihmap->mo_cpl_count = cnt - 1;
      cnt--;
    }
  else
    return ihmap;
  if (siz > 30 && 3 * cnt < siz)
    ihmap = mo_inthmap_reserve (ihmap, 0);
  return ihmap;
}                               /* end of mo_inthmap_remove */

unsigned
mo_inthmap_retrieve_raw_keys (mo_inthmappayl_ty * ihmap, int64_t *keyarr,
                              unsigned nb)
{
  ihmap = mo_dyncastpayl_inthmap (ihmap);
  if (!ihmap)
    return 0;
  if (!keyarr || keyarr == MOM_EMPTY_SLOT || nb == 0)
    return 0;
  unsigned siz = ihmap->mo_sva_size;
  unsigned cnt = 0;
  for (int ix = 0; ix < (int) siz; ix++)
    {
      mo_value_t curval = ihmap->mo_inthm_entarr[ix].mo_inthe_val;
      if (!curval || curval == MOM_EMPTY_SLOT)
        continue;
      if (cnt >= nb)
        return cnt;
      keyarr[cnt++] = ihmap->mo_inthm_entarr[ix].mo_inthe_key;
    }
  return cnt;
}                               /* end of mo_inthmap_retrieve_raw_keys */

void
mo_dump_scan_inthmap (mo_dumper_ty * du, mo_inthmappayl_ty * ihmap)
{
  MOM_ASSERTPRINTF (mo_dump_scanning (du), "bad du");
  ihmap = mo_dyncastpayl_inthmap (ihmap);
  if (!ihmap)
    return;
  unsigned siz = ihmap->mo_sva_size;
  for (int ix = 0; ix < (int) siz; ix++)
    {
      mo_value_t curval = ihmap->mo_inthm_entarr[ix].mo_inthe_val;
      if (!curval || curval == MOM_EMPTY_SLOT)
        continue;
      mo_dump_scan_value (du, curval);
    }
}                               /* end of mo_dump_scan_inthmap */

static int
mom_int64_cmp (const void *p1, const void *p2)
{
  int64_t x1 = *(const int64_t *) p1;
  int64_t x2 = *(const int64_t *) p2;
  if (x1 == x2)
    return 0;
  else if (x1 > x2)
    return 1;
  else
    return -1;
}                               /* end mom_int64_cmp */

mo_json_t
mo_dump_json_of_inthmap (mo_dumper_ty * du, mo_inthmappayl_ty * ihmap)
{
  MOM_ASSERTPRINTF (mo_dump_emitting (du), "bad du");
  ihmap = mo_dyncastpayl_inthmap (ihmap);
  if (!ihmap)
    return json_null ();
  unsigned cnt = ihmap->mo_cpl_count;
  int64_t *keyarr = mom_gc_alloc_scalar ((cnt + 1) * sizeof (int64_t));
  unsigned nbkey = mo_inthmap_retrieve_raw_keys (ihmap, keyarr, cnt + 1);
  MOM_ASSERTPRINTF (nbkey == cnt, "bad nbkey");
  if (nbkey > 1)
    qsort (keyarr, nbkey, sizeof (int64_t), mom_int64_cmp);
  json_t *jarr = json_array ();
  for (unsigned ix = 0; ix < nbkey; ix++)
    {
      int64_t curkey = keyarr[ix];
      mo_value_t curval = mo_inthmap_get (ihmap, curkey);
      MOM_ASSERTPRINTF (curval != NULL,
                        "dump_json_of_inthmap: missing key %lld",
                        (long long) curkey);
      json_t *jval = mo_dump_json_of_value (du, curval);
      if (jval && !json_is_null (jval))
        json_array_append_new (jarr,
                               json_pack ("{sIso}", "key",
                                          (json_int_t) curkey, "val", jval));
    }
  return json_pack ("{so}", "inthmap", jarr);
}                               /* end of mo_dump_json_of_inthmap */

mo_inthmappayl_ty *
mo_inthmap_of_json (mo_json_t js)
{
  json_t *jarr = NULL;
  if (json_is_object (js) && (jarr = json_object_get (js, "inthmap")) != NULL
      && json_is_array (jarr))
    {
      unsigned sz = json_array_size (jarr);
      mo_inthmappayl_ty *ihmap = mo_inthmap_reserve (NULL, sz + sz / 16 + 4);
      for (unsigned ix = 0; ix < sz; ix++)
        {
          json_t *jpair = json_array_get (jarr, ix);
          if (!json_is_object (jpair))
            continue;
          json_t *jkey = json_object_get (jpair, "key");
          if (!jkey || !json_is_integer (jkey))
            continue;
          json_t *jval = json_object_get (jpair, "val");
          if (!jval)
            continue;
          int64_t key = json_integer_value (jkey);
          mo_value_t curval = mo_value_of_json (jval);
          if (!curval)
            continue;
          ihmap = mo_inthmap_put (ihmap, key, curval);
        }
      return ihmap;
    }
  return NULL;
}                               /* end of mo_inthmap_of_json */

// eof hashcont.c
