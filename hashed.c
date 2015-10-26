// file hashed.c - managing hashed stuff

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

#include "monimelt.h"

static int
hashset_index_mom (const struct mom_hashset_st *hset,
                   const struct mom_item_st *itm)
{
  assert (hset && hset->va_itype == MOMITY_HASHSET);
  assert (itm && itm->va_itype == MOMITY_ITEM);
  unsigned siz = mom_raw_size (hset);
  assert (hset->cda_count < siz);
  assert (siz > 2);
  momhash_t hitm = itm->hva_hash;
  unsigned startix = hitm % siz;
  int pos = -1;
  for (unsigned ix = startix; ix < siz; ix++)
    {
      const struct mom_item_st *curitm = hset->hset_items[ix];
      if (curitm == itm)
        return ix;
      else if (curitm == MOM_EMPTY_SLOT)
        {
          if (pos < 0)
            pos = ix;
          continue;
        }
      else if (curitm == NULL)
        {
          if (pos < 0)
            pos = ix;
          return pos;
        }
    }
  for (unsigned ix = 0; ix < startix; ix++)
    {
      const struct mom_item_st *curitm = hset->hset_items[ix];
      if (curitm == itm)
        return ix;
      else if (curitm == MOM_EMPTY_SLOT)
        {
          if (pos < 0)
            pos = ix;
          continue;
        }
      else if (curitm == NULL)
        {
          if (pos < 0)
            pos = ix;
          return pos;
        }
    }
  return -1;
}


struct mom_hashset_st *
mom_hashset_reserve (struct mom_hashset_st *hset, unsigned gap)
{
  if (!hset || hset == MOM_EMPTY_SLOT || hset->va_itype != MOMITY_HASHSET)
    {
      unsigned newsiz = mom_prime_above (6 * gap / 5 + 3);
      hset = mom_gc_alloc (sizeof (*hset) + newsiz * sizeof (void *));
      hset->va_itype = MOMITY_HASHSET;
      mom_put_size (hset, newsiz);
      hset->cda_count = 0;
      return hset;
    }
  unsigned cnt = hset->cda_count;
  unsigned siz = mom_raw_size (hset);
  assert (cnt <= siz);
  if (gap == 0)
    {
      // reorganize
      unsigned newsiz =
        mom_prime_above (5 * cnt / 4 + ((cnt > 100) ? (cnt / 32) : 1) + 3);
      unsigned oldsiz = siz;
      unsigned oldcnt = cnt;
      struct mom_hashset_st *oldhset = hset;
      struct mom_hashset_st *newhset =
        mom_gc_alloc (sizeof (*newhset) + newsiz * sizeof (void *));
      newhset->va_itype = MOMITY_HASHSET;
      mom_put_size (newhset, newsiz);
      for (unsigned ix = 0; ix < oldsiz; ix++)
        {
          struct mom_item_st *olditm = oldhset->hset_items[ix];
          if (!olditm || olditm == MOM_EMPTY_SLOT)
            continue;
          int pos = hashset_index_mom (newhset, olditm);
          assert (pos >= 0 && pos < (int) newsiz
                  && newhset->hset_items[pos] == NULL);
          newhset->hset_items[pos] = olditm;
          newhset->cda_count++;
        }
      assert (newhset->cda_count == oldcnt);
      siz = newsiz;
      hset = newhset;
    }
  else if (6 * (gap + cnt) >= 5 * siz)
    {
      unsigned newsiz =
        mom_prime_above (5 * (gap + cnt) / 4 +
                         (((gap + cnt) > 100) ? ((gap + cnt) / 32) : 1) + 3);
      unsigned oldsiz = siz;
      unsigned oldcnt = cnt;
      struct mom_hashset_st *oldhset = hset;
      struct mom_hashset_st *newhset =
        mom_gc_alloc (sizeof (*newhset) + newsiz * sizeof (void *));
      newhset->va_itype = MOMITY_HASHSET;
      mom_put_size (newhset, newsiz);
      for (unsigned ix = 0; ix < oldsiz; ix++)
        {
          struct mom_item_st *olditm = oldhset->hset_items[ix];
          if (!olditm || olditm == MOM_EMPTY_SLOT)
            continue;
          int pos = hashset_index_mom (newhset, olditm);
          assert (pos >= 0 && pos < (int) newsiz
                  && newhset->hset_items[pos] == NULL);
          newhset->hset_items[pos] = olditm;
          newhset->cda_count++;
        }
      assert (newhset->cda_count == oldcnt);
      return newhset;
    }
  return hset;
}



struct mom_hashset_st *
mom_hashset_insert (struct mom_hashset_st *hset, struct mom_item_st *itm)
{
  if (!itm || itm == MOM_EMPTY_SLOT || itm->va_itype != MOMITY_ITEM)
    return hset;
  if (!hset || hset == MOM_EMPTY_SLOT || hset->va_itype != MOMITY_HASHSET)
    {
      hset = mom_hashset_reserve (NULL, 5);
      int pos = hashset_index_mom (hset, itm);
      assert (pos >= 0 && pos < (int) mom_raw_size (hset));
      hset->hset_items[pos] = itm;
      hset->cda_count = 1;
      return hset;
    }
  unsigned cnt = hset->cda_count;
  unsigned siz = mom_raw_size (hset);
  assert (cnt <= siz);
  if (5 * cnt >= 4 * siz)
    {
      hset = mom_hashset_reserve (hset, 3 + cnt / 256);
      siz = mom_raw_size (hset);
    }
  int pos = hashset_index_mom (hset, itm);
  assert (pos >= 0 && pos < (int) siz);
  if (hset->hset_items[pos] != itm)
    {
      hset->hset_items[pos] = itm;
      hset->cda_count++;
    }
  return hset;
}


struct mom_hashset_st *
mom_hashset_remove (struct mom_hashset_st *hset, struct mom_item_st *itm)
{
  if (!hset || hset == MOM_EMPTY_SLOT || hset->va_itype != MOMITY_HASHSET)
    return NULL;
  if (!itm || itm == MOM_EMPTY_SLOT || itm->va_itype != MOMITY_ITEM)
    return hset;
  int pos = hashset_index_mom (hset, itm);
  if (pos >= 0 && hset->hset_items[pos] == itm)
    {
      hset->hset_items[pos] = MOM_EMPTY_SLOT;
      hset->cda_count--;
      unsigned siz = mom_raw_size (hset);
      if (siz > 12 && hset->cda_count * 2 + 3 < siz)
        hset = mom_hashset_reserve (hset, 0);
    }
  return hset;
}

bool
mom_hashset_contains (const struct mom_hashset_st * hset,
                      const struct mom_item_st * itm)
{
  if (!hset || hset == MOM_EMPTY_SLOT || hset->va_itype != MOMITY_HASHSET)
    return false;
  if (!itm || itm == MOM_EMPTY_SLOT || itm->va_itype != MOMITY_ITEM)
    return false;
  int pos = hashset_index_mom (hset, itm);
  if (pos >= 0 && hset->hset_items[pos] == itm)
    return true;
  return false;
}



const struct mom_boxset_st *
mom_hashset_to_boxset (const struct mom_hashset_st *hset)
{
  if (!hset || hset == MOM_EMPTY_SLOT || hset->va_itype != MOMITY_HASHSET)
    return NULL;
  unsigned cnt = hset->cda_count;
  unsigned siz = mom_raw_size (hset);
  assert (cnt <= siz);
  struct mom_item_st *smallarr[16] = { };
  const struct mom_item_st **arr =
    (cnt <
     sizeof (smallarr) /
     sizeof (smallarr[0])) ? smallarr : mom_gc_alloc ((cnt +
                                                       1) * sizeof (void *));
  unsigned card = 0;
  for (unsigned ix = 0; ix < siz; ix++)
    {
      struct mom_item_st *curitm = hset->hset_items[ix];
      if (!curitm || curitm == MOM_EMPTY_SLOT)
        continue;
      assert (card < cnt);
      arr[card++] = curitm;
    }
  assert (card == cnt);
  return mom_boxset_make_arr (card, arr);
}

void
mom_dumpscan_hashset (struct mom_dumper_st *du, struct mom_hashset_st *hset)
{
  if (!hset || hset == MOM_EMPTY_SLOT || hset->va_itype != MOMITY_HASHSET)
    return;
  unsigned cnt = hset->cda_count;
  unsigned siz = mom_raw_size (hset);
  assert (cnt <= siz);
  for (unsigned ix = 0; ix < siz; ix++)
    {
      struct mom_item_st *curitm = hset->hset_items[ix];
      if (!curitm || curitm == MOM_EMPTY_SLOT)
        continue;
      mom_dumpscan_item (du, curitm);
    }
}                               /* end of mom_dumpscan_hashset */

////////////////////////////////////////////////////////////////

/////// hashmap payload

static int
hashmap_index_mom (const struct mom_hashmap_st *hmap,
                   const struct mom_item_st *itm)
{
  assert (hmap && hmap->va_itype == MOMITY_HASHMAP);
  assert (itm && itm->va_itype == MOMITY_ITEM);
  unsigned siz = mom_raw_size (hmap);
  assert (hmap->cda_count < siz);
  assert (siz > 2);
  momhash_t hitm = itm->hva_hash;
  unsigned startix = hitm % siz;
  int pos = -1;
  for (unsigned ix = startix; ix < siz; ix++)
    {
      const struct mom_item_st *curitm = hmap->hmap_ents[ix].ient_itm;
      if (curitm == itm)
        return ix;
      else if (curitm == MOM_EMPTY_SLOT)
        {
          if (pos < 0)
            pos = ix;
          continue;
        }
      else if (curitm == NULL)
        {
          if (pos < 0)
            pos = ix;
          return pos;
        }
    }
  for (unsigned ix = 0; ix < startix; ix++)
    {
      const struct mom_item_st *curitm = hmap->hmap_ents[ix].ient_itm;
      if (curitm == itm)
        return ix;
      else if (curitm == MOM_EMPTY_SLOT)
        {
          if (pos < 0)
            pos = ix;
          continue;
        }
      else if (curitm == NULL)
        {
          if (pos < 0)
            pos = ix;
          return pos;
        }
    }
  return -1;
}                               /* end hashmap_index_mom */



struct mom_hashmap_st *
mom_hashmap_reserve (struct mom_hashmap_st *hmap, unsigned gap)
{
  if (!hmap || hmap == MOM_EMPTY_SLOT || hmap->va_itype != MOMITY_HASHMAP)
    {
      unsigned newsiz = mom_prime_above (6 * gap / 5 + 3);
      hmap =
        mom_gc_alloc (sizeof (*hmap) +
                      newsiz * sizeof (struct mom_itementry_tu));
      hmap->va_itype = MOMITY_HASHMAP;
      mom_put_size (hmap, newsiz);
      hmap->cda_count = 0;
      return hmap;
    }
  unsigned cnt = hmap->cda_count;
  unsigned siz = mom_raw_size (hmap);
  assert (cnt <= siz);
  if (gap == 0)
    {
      // reorganize
      unsigned newsiz =
        mom_prime_above (5 * cnt / 4 + ((cnt > 100) ? (cnt / 32) : 1) + 3);
      unsigned oldsiz = siz;
      unsigned oldcnt = cnt;
      struct mom_hashmap_st *oldhmap = hmap;
      struct mom_hashmap_st *newhmap =
        mom_gc_alloc (sizeof (*newhmap) +
                      newsiz * sizeof (struct mom_itementry_tu));
      newhmap->va_itype = MOMITY_HASHMAP;
      mom_put_size (newhmap, newsiz);
      for (unsigned ix = 0; ix < oldsiz; ix++)
        {
          const struct mom_item_st *curitm = hmap->hmap_ents[ix].ient_itm;
          if (!curitm || curitm == MOM_EMPTY_SLOT)
            continue;
          int pos = hashmap_index_mom (newhmap, curitm);
          assert (pos >= 0 && pos < (int) newsiz
                  && newhmap->hmap_ents[pos].ient_itm == NULL);
          newhmap->hmap_ents[pos] = oldhmap->hmap_ents[ix];
          newhmap->cda_count++;
        }
      assert (newhmap->cda_count == oldcnt);
      siz = newsiz;
      hmap = newhmap;
    }
  else if (6 * (gap + cnt) >= 5 * siz)
    {
      unsigned newsiz =
        mom_prime_above (5 * (gap + cnt) / 4 +
                         (((gap + cnt) > 100) ? ((gap + cnt) / 32) : 1) + 3);
      unsigned oldsiz = siz;
      unsigned oldcnt = cnt;
      struct mom_hashmap_st *oldhmap = hmap;
      struct mom_hashmap_st *newhmap =
        mom_gc_alloc (sizeof (*newhmap) +
                      newsiz * sizeof (struct mom_itementry_tu));
      newhmap->va_itype = MOMITY_HASHMAP;
      mom_put_size (newhmap, newsiz);
      for (unsigned ix = 0; ix < oldsiz; ix++)
        {
          const struct mom_item_st *curitm = hmap->hmap_ents[ix].ient_itm;
          if (!curitm || curitm == MOM_EMPTY_SLOT)
            continue;
          int pos = hashmap_index_mom (newhmap, curitm);
          assert (pos >= 0 && pos < (int) newsiz
                  && newhmap->hmap_ents[pos].ient_itm == NULL);
          newhmap->hmap_ents[pos] = oldhmap->hmap_ents[ix];
          newhmap->cda_count++;
        }
      assert (newhmap->cda_count == oldcnt);
      siz = newsiz;
      hmap = newhmap;
    }
  return hmap;
}                               /* end of mom_hashmap_reserve */

const struct mom_hashedvalue_st *
mom_hashmap_get (const struct mom_hashmap_st *hmap,
                 const struct mom_item_st *itm)
{
  if (!hmap || hmap == MOM_EMPTY_SLOT || hmap->va_itype != MOMITY_HASHMAP)
    return NULL;
  if (!itm || itm == MOM_EMPTY_SLOT || itm->va_itype != MOMITY_ITEM)
    return NULL;
  int pos = hashmap_index_mom (hmap, itm);
  if (pos < 0)
    return NULL;
  assert (pos < (int) mom_raw_size (hmap));
  if (hmap->hmap_ents[pos].ient_itm == itm)
    return hmap->hmap_ents[pos].ient_val;
  return NULL;
}


struct mom_hashmap_st *
mom_hashmap_put (struct mom_hashmap_st *hmap, const struct mom_item_st *itm,
                 const struct mom_hashedvalue_st *val)
{
  if (!hmap || hmap == MOM_EMPTY_SLOT || hmap->va_itype != MOMITY_HASHMAP)
    return NULL;
  if (!itm || itm == MOM_EMPTY_SLOT || itm->va_itype != MOMITY_ITEM)
    return hmap;
  if (!val || val == MOM_EMPTY_SLOT)
    return mom_hashmap_remove (hmap, itm);
  assert (val->va_itype > MOMITY_NONE && val->va_itype < MOMITY__LASTHASHED);
  unsigned siz = mom_raw_size (hmap);
  unsigned cnt = hmap->cda_count;
  if (6 * cnt + 2 >= 5 * siz)
    {
      hmap = mom_hashmap_reserve (hmap, cnt / 8 + 2);
      siz = mom_raw_size (hmap);
    }
  int pos = hashmap_index_mom (hmap, itm);
  assert (pos >= 0 && pos < (int) siz);
  if (hmap->hmap_ents[pos].ient_itm == itm)
    {
      hmap->hmap_ents[pos].ient_val = (struct mom_hashedvalue_st *) val;
      return hmap;
    }
  hmap->hmap_ents[pos].ient_itm = (struct mom_item_st *) itm;
  hmap->hmap_ents[pos].ient_val = (struct mom_hashedvalue_st *) val;
  hmap->cda_count = cnt + 1;
  return hmap;
}

struct mom_hashmap_st *
mom_hashmap_remove (struct mom_hashmap_st *hmap,
                    const struct mom_item_st *itm)
{
  if (!hmap || hmap == MOM_EMPTY_SLOT || hmap->va_itype != MOMITY_HASHMAP)
    return NULL;
  if (!itm || itm == MOM_EMPTY_SLOT || itm->va_itype != MOMITY_ITEM)
    return hmap;
  unsigned siz = mom_raw_size (hmap);
  unsigned cnt = hmap->cda_count;
  if (3 * cnt < siz && siz > 20)
    {
      hmap = mom_hashmap_reserve (hmap, 0);
      siz = mom_raw_size (hmap);
    }
  int pos = hashmap_index_mom (hmap, itm);
  if (pos < 0)
    return hmap;
  if (hmap->hmap_ents[pos].ient_itm == itm)
    {
      hmap->hmap_ents[pos].ient_itm = MOM_EMPTY_SLOT;
      hmap->hmap_ents[pos].ient_val = NULL;
      hmap->cda_count = cnt - 1;
    }
  return hmap;
}

const struct mom_boxset_st *
mom_hashmap_keyset (const struct mom_hashmap_st *hmap)
{
  if (!hmap || hmap == MOM_EMPTY_SLOT || hmap->va_itype != MOMITY_HASHMAP)
    return NULL;
  unsigned siz = mom_raw_size (hmap);
  unsigned cnt = hmap->cda_count;
  struct mom_item_st *smallarr[20];
  memset (smallarr, 0, sizeof (smallarr));
  struct mom_item_st **arr
    = (cnt < sizeof (smallarr) / sizeof (smallarr[0])) ? smallarr
    : mom_gc_alloc (sizeof (struct momitem_st *) * (cnt + 1));
  unsigned itmcnt = 0;
  for (unsigned ix = 0; ix < siz; ix++)
    {
      struct mom_item_st *curitm = hmap->hmap_ents[ix].ient_itm;
      if (!curitm || curitm == MOM_EMPTY_SLOT)
        continue;
      assert (itmcnt < cnt);
      arr[itmcnt++] = curitm;
    }
  assert (itmcnt == cnt);
  return mom_boxset_make_arr (itmcnt, (const struct mom_item_st **) arr);
}

void
mom_dumpscan_hashmap (struct mom_dumper_st *du, struct mom_hashmap_st *hmap)
{
  if (!hmap || hmap == MOM_EMPTY_SLOT || hmap->va_itype != MOMITY_HASHMAP)
    return;
  assert (du && du != MOM_EMPTY_SLOT && du->va_itype == MOMITY_DUMPER);
  unsigned siz = mom_raw_size (hmap);
  for (unsigned ix = 0; ix < siz; ix++)
    {
      struct mom_item_st *curitm = hmap->hmap_ents[ix].ient_itm;
      if (!curitm || curitm == MOM_EMPTY_SLOT)
        continue;
      if (!mom_dumpable_item (curitm))
        continue;
      mom_dumpscan_item (du, curitm);
      mom_dumpscan_value (du, hmap->hmap_ents[ix].ient_val);
    }
}                               /* end of mom_dumpscan_hashmap */



////////////////////////////////////////////////////////////////
static int
hashassoc_index_mom (const struct mom_hashassoc_st *hass,
                     const struct mom_hashedvalue_st *key)
{
  assert (hass && hass->va_itype == MOMITY_HASHASSOC);
  assert (key && key->va_itype < MOMITY__LASTHASHED);
  unsigned siz = mom_raw_size (hass);
  assert (hass->cda_count < siz);
  assert (siz > 2);
  momhash_t hkey = key->hva_hash;
  unsigned startix = hkey % siz;
  int pos = -1;
  for (unsigned ix = startix; ix < siz; ix++)
    {
      const struct mom_hashedvalue_st *curkey = hass->hass_ents[ix].hass_key;
      if (curkey == MOM_EMPTY_SLOT)
        {
          if (pos < 0)
            pos = ix;
          continue;
        }
      else if (curkey == NULL)
        {
          if (pos < 0)
            pos = ix;
          return pos;
        }
      else if (curkey->hva_hash != hkey)
        continue;
      else if (curkey == key || mom_hashedvalue_equal (curkey, key))
        {
          return ix;
        }
    }
  for (unsigned ix = 0; ix < startix; ix++)
    {
      const struct mom_hashedvalue_st *curkey = hass->hass_ents[ix].hass_key;
      if (curkey == MOM_EMPTY_SLOT)
        {
          if (pos < 0)
            pos = ix;
          continue;
        }
      else if (curkey == NULL)
        {
          if (pos < 0)
            pos = ix;
          return pos;
        }
      else if (curkey->hva_hash != hkey)
        continue;
      else if (curkey == key || mom_hashedvalue_equal (curkey, key))
        {
          return ix;
        }
    }
  return -1;
}                               /* end of hashassoc_index_mom */



struct mom_hashassoc_st *
mom_hashassoc_reserve (struct mom_hashassoc_st *hass, unsigned gap)
{
  if (!hass || hass == MOM_EMPTY_SLOT || hass->va_itype != MOMITY_HASHASSOC)
    {
      unsigned newsiz = mom_prime_above (6 * gap / 5 + 3);
      hass =
        mom_gc_alloc (sizeof (*hass) +
                      newsiz * sizeof (struct mom_itementry_tu));
      hass->va_itype = MOMITY_HASHASSOC;
      mom_put_size (hass, newsiz);
      hass->cda_count = 0;
      return hass;
    }
  unsigned cnt = hass->cda_count;
  unsigned siz = mom_raw_size (hass);
  assert (cnt <= siz);
  if (gap == 0)
    {
      // reorganize
      unsigned newsiz =
        mom_prime_above (5 * cnt / 4 + ((cnt > 100) ? (cnt / 32) : 1) + 3);
      unsigned oldsiz = siz;
      unsigned oldcnt = cnt;
      struct mom_hashassoc_st *oldhass = hass;
      struct mom_hashassoc_st *newhass =
        mom_gc_alloc (sizeof (*newhass) +
                      newsiz * sizeof (struct mom_hassocentry_tu));
      newhass->va_itype = MOMITY_HASHASSOC;
      mom_put_size (newhass, newsiz);
      for (unsigned ix = 0; ix < oldsiz; ix++)
        {
          const struct mom_hashedvalue_st *curkey =
            hass->hass_ents[ix].hass_key;
          if (!curkey || curkey == MOM_EMPTY_SLOT)
            continue;
          int pos = hashassoc_index_mom (newhass, curkey);
          assert (pos >= 0 && pos < (int) newsiz
                  && newhass->hass_ents[pos].hass_key == NULL);
          newhass->hass_ents[pos] = oldhass->hass_ents[ix];
          newhass->cda_count++;
        }
      assert (newhass->cda_count == oldcnt);
      siz = newsiz;
      hass = newhass;
    }
  else if (6 * (gap + cnt) >= 5 * siz)
    {
      unsigned newsiz =
        mom_prime_above (5 * (gap + cnt) / 4 +
                         (((gap + cnt) > 100) ? ((gap + cnt) / 32) : 1) + 3);
      unsigned oldsiz = siz;
      unsigned oldcnt = cnt;
      struct mom_hashassoc_st *oldhass = hass;
      struct mom_hashassoc_st *newhass =
        mom_gc_alloc (sizeof (*newhass) +
                      newsiz * sizeof (struct mom_itementry_tu));
      newhass->va_itype = MOMITY_HASHASSOC;
      mom_put_size (newhass, newsiz);
      for (unsigned ix = 0; ix < oldsiz; ix++)
        {
          const struct mom_hashedvalue_st *curkey =
            hass->hass_ents[ix].hass_key;
          if (!curkey || curkey == MOM_EMPTY_SLOT)
            continue;
          int pos = hashassoc_index_mom (newhass, curkey);
          assert (pos >= 0 && pos < (int) newsiz
                  && newhass->hass_ents[pos].hass_key == NULL);
          newhass->hass_ents[pos] = oldhass->hass_ents[ix];
          newhass->cda_count++;
        }
      assert (newhass->cda_count == oldcnt);
      siz = newsiz;
      hass = newhass;
    }
  return hass;
}                               /* end of mom_hashassoc_reserve */


const struct mom_hashedvalue_st *
mom_hashassoc_get (const struct mom_hashassoc_st *hass,
                   const struct mom_hashedvalue_st *key)
{
  if (!hass || hass == MOM_EMPTY_SLOT || hass->va_itype != MOMITY_HASHASSOC)
    return NULL;
  if (!key || key == MOM_EMPTY_SLOT || key->va_itype >= MOMITY__LASTHASHED)
    return NULL;
  int pos = hashassoc_index_mom (hass, key);
  assert (pos < (int) mom_raw_size (hass));
  if (pos < 0 || hass->hass_ents[pos].hass_key == NULL
      || hass->hass_ents[pos].hass_key == MOM_EMPTY_SLOT)
    return NULL;
  return hass->hass_ents[pos].hass_val;
}                               /* end of mom_hashassoc_get */



struct mom_hashassoc_st *
mom_hashassoc_put (struct mom_hashassoc_st *hass,
                   const struct mom_hashedvalue_st *key,
                   const struct mom_hashedvalue_st *val)
{
  if (!hass || hass == MOM_EMPTY_SLOT || hass->va_itype != MOMITY_HASHASSOC)
    return NULL;
  if (!key || key == MOM_EMPTY_SLOT || key->va_itype >= MOMITY__LASTHASHED)
    return hass;
  if (!val || val == MOM_EMPTY_SLOT)
    return mom_hashassoc_remove (hass, key);
  if (val->va_itype >= MOMITY__LASTHASHED)
    return hass;
  unsigned siz = mom_raw_size (hass);
  unsigned cnt = hass->cda_count;
  if (6 * cnt + 2 >= 5 * siz)
    {
      hass = mom_hashassoc_reserve (hass, cnt / 8 + 2);
      siz = mom_raw_size (hass);
    }
  int pos = hashassoc_index_mom (hass, key);
  assert (pos >= 0 && pos < (int) siz);
  if (hass->hass_ents[pos].hass_key == NULL
      || hass->hass_ents[pos].hass_key == MOM_EMPTY_SLOT)
    {
      hass->hass_ents[pos].hass_key = key;
      hass->hass_ents[pos].hass_val = val;
      hass->cda_count = cnt + 1;
      return hass;
    }
  else
    {
      hass->hass_ents[pos].hass_val = val;
      return hass;
    }
}                               /* end of mom_hashassoc_put */

struct mom_hashassoc_st *
mom_hashassoc_remove (struct mom_hashassoc_st *hass,
                      const struct mom_hashedvalue_st *key)
{
  if (!hass || hass == MOM_EMPTY_SLOT || hass->va_itype != MOMITY_HASHASSOC)
    return NULL;
  if (!key || key == MOM_EMPTY_SLOT || key->va_itype >= MOMITY__LASTHASHED)
    return hass;
  unsigned siz = mom_raw_size (hass);
  unsigned cnt = hass->cda_count;
  if (siz > 30 && 3 * cnt < siz)
    {
      hass = mom_hashassoc_reserve (hass, 2);
      siz = mom_raw_size (hass);
    }
  if (cnt == 0)
    return hass;
  int pos = hashassoc_index_mom (hass, key);
  assert (pos >= 0 && pos < (int) siz);
  if (hass->hass_ents[pos].hass_key == NULL
      || hass->hass_ents[pos].hass_key == MOM_EMPTY_SLOT)
    return hass;
  hass->hass_ents[pos].hass_key = MOM_EMPTY_SLOT;
  hass->hass_ents[pos].hass_val = NULL;
  hass->cda_count = cnt - 1;
  return hass;
}                               /* end of mom_hashassoc_remove */
