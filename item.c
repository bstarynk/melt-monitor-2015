// file item.c - managing items

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

/// we temporarily store the radix in a sorted dynamic array
static pthread_mutex_t radix_mtx_mom = PTHREAD_MUTEX_INITIALIZER;
// radix_arr_mom is mom_gc_alloc-ed but each itemname is
// mom_gc_alloc_atomic-ed

struct radixitems_mom_st;

struct radix_mom_st
{
  struct mom_itemname_tu *rad_name;
  uint32_t rad_size;            /* allocated size */
  uint32_t rad_count;           /* used count */
  pthread_mutex_t rad_mtx;
  struct mom_item_st **rad_items;       /* of rdxi_size */
};


/// actually, it might be better to have each element of the array be
/// individually allocated, with its own mutex...

static struct radix_mom_st **radix_arr_mom;
static unsigned radix_siz_mom;  /* allocated size of radix_arr_mom */
static unsigned radix_cnt_mom;  /* used count of radix_arr_mom */


bool
mom_valid_name_radix (const char *str, int len)
{
  if (!str)
    return false;
  if (len < 0)
    len = strlen (str);
  if (len <= 0)
    return false;
  if (!isalpha (str[0]))
    return false;
  const char *end = str + len;
  for (const char *pc = str; pc < end; pc++)
    {
      if (isalnum (*pc))
        continue;
      else if (*pc == '_')
        if (pc[-1] == '_')
          return false;
    }
  return true;
}                               /* end mom_valid_name_radix */



const struct mom_itemname_tu *
mom_find_name_radix (const char *str, int len)
{
  struct mom_itemname_tu *tun = NULL;
  if (!str || !str[0])
    return NULL;
  if (len < 0)
    len = strlen (str);
  if (!mom_valid_name_radix (str, len))
    return NULL;
  pthread_mutex_lock (&radix_mtx_mom);
  assert (radix_cnt_mom <= radix_siz_mom);
  if (radix_cnt_mom == 0)
    goto end;
  int lo = 0, hi = (int) radix_cnt_mom - 1;
  while (lo + 5 < hi)
    {
      int md = (lo + hi) / 2;
      struct mom_itemname_tu *curad = radix_arr_mom[md]->rad_name;
      assert (curad != NULL);
      assert (curad->itname_rank == (unsigned) md);
      int c = strncmp (str, curad->itname_string.cstr, len);
      if (c == 0 && curad->itname_string.cstr[len] == (char) 0)
        {
          tun = curad;
          goto end;
        };
      if (c <= 0)
        hi = md;
      else
        lo = md;
    };
  for (int ix = lo; ix < hi; ix++)
    {
      struct mom_itemname_tu *curad = radix_arr_mom[ix]->rad_name;
      assert (curad != NULL);
      assert (curad->itname_rank == (unsigned) ix);
      if (!strncmp (curad->itname_string.cstr, str, len)
          && curad->itname_string.cstr[len] == (char) 0)
        {
          tun = curad;
          goto end;
        };
    }
end:
  pthread_mutex_unlock (&radix_mtx_mom);
  return tun;
}                               /* end of mom_find_name_radix  */



const struct mom_itemname_tu *
mom_make_name_radix (const char *str, int len)
{
  int tix = -1;
  struct mom_itemname_tu *tun = NULL;
  if (!str || !str[0])
    return NULL;
  if (len < 0)
    len = strlen (str);
  if (len >= 256)
    MOM_FATAPRINTF ("too big length %d for name radix %.*s", len, len, str);
  MOM_DEBUGPRINTF (item, "mom_make_name_radix %.*s", len, str);
  if (!mom_valid_name_radix (str, len))
    return NULL;
  pthread_mutex_lock (&radix_mtx_mom);
  assert (radix_cnt_mom <= radix_siz_mom);
  if (MOM_UNLIKELY (radix_cnt_mom + 2 >= radix_siz_mom))
    {
      unsigned newsiz = ((5 * radix_cnt_mom / 4 + 10) | 0xf) + 1;
      assert (newsiz > radix_siz_mom);
      struct radix_mom_st **newarr =
        mom_gc_alloc (newsiz * sizeof (struct radix_mom_st *));
      if (radix_cnt_mom > 0)
        {
          memcpy (newarr, radix_arr_mom,
                  sizeof (struct radix_mom_st *) * radix_cnt_mom);
          GC_FREE (radix_arr_mom);
        }
      radix_arr_mom = newarr;
      radix_siz_mom = newsiz;
      if (MOM_UNLIKELY (radix_cnt_mom == 0))
        {                       // create the first radix
          struct mom_itemname_tu *nam =
            mom_gc_alloc_atomic (((sizeof (struct mom_itemname_tu) + len +
                                   2) | 3) + 1);
          nam->itname_rank = 0;
          nam->itname_string.va_itype = MOM_BOXSTRING_ITYPE;
          nam->itname_string.va_hsiz = len >> 8;
          nam->itname_string.va_lsiz = len & 0xffff;
          nam->itname_string.hva_hash = mom_cstring_hash_len (str, len);
          strncpy (nam->itname_string.cstr, str, len);
          nam->itname_string.cstr[len] = (char) 0;
          struct radix_mom_st *newrad =
            mom_gc_alloc (sizeof (struct radix_mom_st));
          const unsigned itmsiz = 7;
          newrad->rad_name = nam;
          newrad->rad_size = itmsiz;
          pthread_mutex_init (&newrad->rad_mtx, NULL);
          newrad->rad_items =
            mom_gc_alloc (itmsiz * sizeof (struct mom_item_st *));
          newrad->rad_size = itmsiz;
          newrad->rad_count = 0;
          const unsigned radsiz = 15;
          radix_arr_mom = mom_gc_alloc (radsiz * sizeof (void *));
          radix_arr_mom[0] = newrad;
          radix_cnt_mom = 1;
          radix_siz_mom = radsiz;
          tun = nam;
          tix = 0;
          MOM_DEBUGPRINTF (item, "make first radix hash %u",
                           nam->itname_string.hva_hash);
          goto end;
        }
    };
  int lo = 0, hi = (int) radix_cnt_mom - 1;
  while (lo + 5 < hi)
    {
      int md = (lo + hi) / 2;
      assert (radix_arr_mom[md]);
      struct mom_itemname_tu *curad = radix_arr_mom[md]->rad_name;
      assert (curad != NULL);
      MOM_DEBUGPRINTF (item, "make radix loop lo=%d hi=%d md=%d curadname %s",
                       lo, hi, md, curad->itname_string.cstr);
      assert (curad->itname_rank == (unsigned) md);
      int c = strncmp (str, curad->itname_string.cstr, len);
      if (c == 0 && curad->itname_string.cstr[len] == (char) 0)
        {
          tun = curad;
          tix = md;
          goto end;
        };
      if (c <= 0)
        hi = md;
      else
        lo = md;
    };
  MOM_DEBUGPRINTF (item, "make radix loop lo=%d radix_cnt=%d", lo,
                   radix_cnt_mom);
  for (int ix = lo; ix < (int) radix_cnt_mom; ix++)
    {
      assert (radix_arr_mom[ix]);
      struct mom_itemname_tu *curad = radix_arr_mom[ix]->rad_name;
      struct mom_itemname_tu *nextrad = NULL;
      assert (curad != NULL);
      assert (curad->itname_rank == (unsigned) ix);
      int c = strncmp (str, curad->itname_string.cstr, len);
      MOM_DEBUGPRINTF (item, "make radix loop ix=%d curadname %s c=%d", ix,
                       curad->itname_string.cstr, c);
      if (c == 0 && curad->itname_string.cstr[len] == (char) 0)
        {
          tun = curad;
          tix = ix;
          MOM_DEBUGPRINTF (item, "make radix loop found ix=%d curadname %s",
                           ix, curad->itname_string.cstr);
          goto end;
        }
      else if (ix + 1 >= (int) radix_cnt_mom
               || (c <= 0
                   && ((nextrad = radix_arr_mom[ix + 1]->rad_name)
                       && strncmp (nextrad->itname_string.cstr, str,
                                   len) > 0)))
        {                       // insert at ix
          MOM_DEBUGPRINTF (item, "make radix loop inserting %s ix=%d next %s",
                           (c <= 0) ? "at" : "after",
                           ix,
                           nextrad ? nextrad->itname_string.cstr : "?none?");
          if (c > 0 && ix + 1 == (int) radix_cnt_mom)
            ix++;
          for (int j = radix_cnt_mom; j > ix; j--)
            {
              radix_arr_mom[j] = radix_arr_mom[j - 1];
              radix_arr_mom[j]->rad_name->itname_rank = j;
            };
          struct radix_mom_st *newrdx =
            mom_gc_alloc (sizeof (struct radix_mom_st));
          struct mom_itemname_tu *nam =
            mom_gc_alloc_atomic (((sizeof (struct mom_itemname_tu) + len +
                                   2) | 3) + 1);
          nam->itname_rank = ix;
          nam->itname_string.va_itype = MOM_BOXSTRING_ITYPE;
          nam->itname_string.va_hsiz = len >> 8;
          nam->itname_string.va_lsiz = len & 0xffff;
          nam->itname_string.hva_hash = mom_cstring_hash_len (str, len);
          strncpy (nam->itname_string.cstr, str, len);
          nam->itname_string.cstr[len] = (char) 0;
          MOM_DEBUGPRINTF (item,
                           "make radix loop insert ix=%d name %s hash %u", ix,
                           nam->itname_string.cstr,
                           nam->itname_string.hva_hash);
          newrdx->rad_name = nam;
          const unsigned itmsiz = 7;
          pthread_mutex_init (&newrdx->rad_mtx, NULL);
          struct mom_item_st **newritems =
            mom_gc_alloc (itmsiz * sizeof (void *));
          newrdx->rad_size = itmsiz;
          newrdx->rad_count = 0;
          newrdx->rad_items = newritems;
          radix_arr_mom[ix] = newrdx;
          tun = nam;
          tix = ix;
          radix_cnt_mom++;
          goto end;
        }
    }
end:
  MOM_DEBUGPRINTF (item, "make name radix final radix_cnt=%d", radix_cnt_mom);
  if (MOM_IS_DEBUGGING (item))
    for (int ix = 0; ix < (int) radix_cnt_mom; ix++)
      {
        MOM_DEBUGPRINTF (item, "make name radix [%d] @%p %s /%u", ix,
                         radix_arr_mom[ix],
                         radix_arr_mom[ix]->rad_name->itname_string.cstr,
                         radix_arr_mom[ix]->rad_name->itname_string.hva_hash);
        if (ix > 0)
          assert (strcmp
                  (radix_arr_mom[ix - 1]->rad_name->itname_string.cstr,
                   radix_arr_mom[ix]->rad_name->itname_string.cstr) < 0);
      }
  pthread_mutex_unlock (&radix_mtx_mom);
  MOM_DEBUGPRINTF (item, "mom_make_name_radix done %.*s tix %d", len, str,
                   tix);
  return tun;
}                               /* end of mom_make_name_radix */


static inline momhash_t
hash_item_from_radix_id_mom (const struct mom_itemname_tu *radix,
                             uint16_t hid, uint64_t loid)
{
  if (!radix)
    return 0;
  momhash_t hr = radix->itname_string.hva_hash;
  momhash_t hi =
    17 * hid +
    ((443 * (uint32_t) (loid >> 32)) ^ (541 *
                                        (uint32_t) (loid & 0xffffffff)));
  momhash_t h = ((601 * hr) ^ (647 * hi)) + ((hr - hi) & 0xffff);
  if (MOM_UNLIKELY (h == 0))
    h =
      11 * (hr & 0xffff) + 17 * (hi & 0xffff) + (hid % 1637) +
      (loid & 0xfffff) + 5;
  assert (h != 0);
  return h;
}


struct mom_item_st *
mom_find_item_from_radix_id (const struct mom_itemname_tu *radix,
                             uint16_t hid, uint64_t loid)
{
  struct mom_item_st *itm = NULL;
  if (!radix)
    return NULL;
  struct radix_mom_st *curad = NULL;
  uint32_t radrk = radix->itname_rank;
  {
    pthread_mutex_lock (&radix_mtx_mom);
    assert (radix_cnt_mom <= radix_siz_mom);
    assert (radrk < radix_cnt_mom);
    curad = radix_arr_mom[radrk];
    pthread_mutex_unlock (&radix_mtx_mom);
  }
  assert (curad != NULL);
  assert (curad->rad_name == radix);
  momhash_t hi = hash_item_from_radix_id_mom (radix, hid, loid);
  unsigned sz = curad->rad_size;
  assert (curad->rad_count < sz);
  pthread_mutex_lock (&curad->rad_mtx);
  unsigned startix = hi % sz;
  for (unsigned ix = startix; ix < sz; ix++)
    {
      struct mom_item_st *curitm = curad->rad_items[ix];
      if (curitm == MOM_EMPTY_SLOT)
        continue;
      if (!curitm)
        goto end;
      if (curitm->itm_radix == radix && curitm->itm_hid == hid
          && curitm->itm_lid == loid)
        {
          itm = curitm;
          goto end;
        }
    }
  for (unsigned ix = 0; ix < startix; ix++)
    {
      struct mom_item_st *curitm = curad->rad_items[ix];
      if (curitm == MOM_EMPTY_SLOT)
        continue;
      if (!curitm)
        goto end;
      if (curitm->itm_radix == radix && curitm->itm_hid == hid
          && curitm->itm_lid == loid)
        {
          itm = curitm;
          goto end;
        }
    }
end:
  pthread_mutex_unlock (&curad->rad_mtx);
  return itm;
}                               /* end of mom_find_item_from_radix_id */


static int
put_item_in_radix_rank_mom (struct radix_mom_st *curad,
                            struct mom_item_st *itm)
{
  assert (curad);
  assert (itm);
  momhash_t hi = itm->hva_hash;
  unsigned sz = curad->rad_size;
  assert (curad->rad_count < sz);
  unsigned startix = hi % sz;
  int pos = -1;
  for (unsigned ix = startix; ix < sz; ix++)
    {
      struct mom_item_st *curitm = curad->rad_items[ix];
      if (curitm == itm)
        return ix;
      if (curitm == MOM_EMPTY_SLOT)
        {
          if (pos < 0)
            pos = (int) ix;
          continue;
        };
      if (!curitm)
        {
          if (pos < 0)
            pos = (int) ix;
          goto end;
        }
    }
  for (unsigned ix = 0; ix < startix; ix++)
    {
      struct mom_item_st *curitm = curad->rad_items[ix];
      if (curitm == itm)
        return ix;
      if (curitm == MOM_EMPTY_SLOT)
        {
          if (pos < 0)
            pos = (int) ix;
          continue;
        };
      if (!curitm)
        {
          if (pos < 0)
            pos = (int) ix;
          goto end;
        }
    }
end:
  assert (pos >= 0 && pos < (int) sz);
  return pos;
}                               /* end of put_item_in_radix_rank_mom */


void
mom_cleanup_item (void *itmad, void *clad)
{
  assert (itmad != NULL);
  assert (clad == NULL);
  struct mom_item_st *itm = itmad;
  unsigned sz = (itm->va_hsiz << 16) + itm->va_lsiz;
  struct radix_mom_st *curad = NULL;
  {
    pthread_mutex_lock (&radix_mtx_mom);
    assert (radix_cnt_mom <= radix_siz_mom);
    struct mom_itemname_tu *radix = itm->itm_radix;
    assert (radix != NULL);
    unsigned radrk = radix->itname_rank;
    assert (radrk <= radix_cnt_mom);
    assert (radix_cnt_mom <= radix_siz_mom);
    curad = radix_arr_mom[radrk];
    pthread_mutex_unlock (&radix_mtx_mom);
  }
  assert (curad != NULL);
  pthread_mutex_lock (&curad->rad_mtx);
  int pos = put_item_in_radix_rank_mom (curad, itm);
  assert (pos >= 0 && pos < (int) curad->rad_size);
  assert (curad->rad_items);
  assert (curad->rad_items[pos] == itm);
  curad->rad_items[pos] = MOM_EMPTY_SLOT;
  assert (curad->rad_count > 0 && curad->rad_count <= curad->rad_size);
  curad->rad_count--;
  pthread_mutex_destroy (&itm->itm_mtx);
  memset (itmad, 0, sizeof (struct mom_item_st) + sz * sizeof (void *));
  pthread_mutex_unlock (&curad->rad_mtx);
}                               /* end of mom_cleanup_item */




struct mom_item_st *
mom_make_item_from_radix_id (const struct mom_itemname_tu *radix,
                             uint16_t hid, uint64_t loid, unsigned isize)
{
  struct mom_item_st *itm = NULL;
  if (!radix)
    return NULL;
  if (MOM_UNLIKELY (isize > MOM_ITEM_MAXFIELDS))
    MOM_FATAPRINTF ("too big item size %d", isize);
  pthread_mutex_lock (&radix_mtx_mom);
  assert (radix_cnt_mom <= radix_siz_mom);
  uint32_t radrk = radix->itname_rank;
  assert (radrk < radix_cnt_mom);
  struct radix_mom_st *curad = radix_arr_mom[radrk];
  assert (curad != NULL);
  momhash_t hi = hash_item_from_radix_id_mom (radix, hid, loid);
  unsigned sz = curad->rad_size;
  unsigned cnt = curad->rad_count;
  assert (cnt <= sz);
  if (MOM_UNLIKELY (4 * cnt + 3 >= 5 * sz))
    {
      unsigned newsiz =
        mom_prime_above (4 * cnt / 3 + 10 +
                         (((cnt > 100) ? (cnt / 32) : 3) + 1));
      if (newsiz > sz)
        {
          struct mom_item_st **oldarr = curad->rad_items;
          assert (oldarr != NULL);
          unsigned oldsiz = sz;
          unsigned oldcnt = cnt;
          struct mom_item_st **newarr =
            mom_gc_alloc (newsiz * sizeof (struct mom_item_st *));
          curad->rad_items = newarr;
          curad->rad_size = newsiz;
          curad->rad_count = 0;
          for (unsigned ix = 0; ix < oldsiz; ix++)
            {
              struct mom_item_st *olditm = oldarr[ix];
              if (!olditm || olditm == MOM_EMPTY_SLOT)
                continue;
              int pos = put_item_in_radix_rank_mom (curad, olditm);
              assert (pos >= 0 && pos < (int) newsiz
                      && curad->rad_items[pos] == NULL);
              curad->rad_items[pos] = olditm;
              curad->rad_count++;
            }
          assert (curad->rad_count == oldcnt);
          sz = newsiz;
        }
    };
  assert (curad->rad_items);
  {
    struct mom_item_st pseudoitemzon;
    memset (&pseudoitemzon, 0, sizeof (pseudoitemzon));
    pseudoitemzon.hva_hash = hi;
    pseudoitemzon.itm_radix = (struct mom_itemname_tu *) radix;
    pseudoitemzon.itm_hid = hid;
    pseudoitemzon.itm_lid = loid;
    int pos = put_item_in_radix_rank_mom (curad, &pseudoitemzon);
    assert (pos >= 0 && pos < (int) sz);
    if (curad->rad_items[pos] != NULL
        && curad->rad_items[pos] != MOM_EMPTY_SLOT)
      {
        itm = curad->rad_items[pos];
        goto end;
      }
    struct mom_item_st *newitm =
      mom_gc_alloc (sizeof (struct mom_item_st) * isize * sizeof (void *));
    newitm->va_itype = MOM_ITEM_ITYPE;
    newitm->va_hsiz = isize >> 16;
    newitm->va_lsiz = isize & 0xffff;
    newitm->hva_hash = hi;
    newitm->itm_radix = (struct mom_itemname_tu *) radix;
    pthread_mutex_init (&newitm->itm_mtx, NULL);
    newitm->itm_spacix = 0;
    newitm->itm_xtra = 0;
    newitm->itm_hid = hid;
    newitm->itm_lid = loid;
    newitm->itm_attrs = NULL;
    newitm->itm_comps = NULL;
    for (unsigned i = 0; i < isize; i++)
      newitm->itm_rest[i] = NULL;
    GC_REGISTER_FINALIZER_IGNORE_SELF (newitm, mom_cleanup_item, NULL, NULL,
                                       NULL);
    itm = newitm;
  }
end:
  pthread_mutex_unlock (&radix_mtx_mom);
  return itm;
}                               /* end of mom_make_item_from_radix_id */
