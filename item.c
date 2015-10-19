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

static pthread_mutexattr_t item_mtxattr_mom;

// radix_arr_mom is mom_gc_alloc-ed but each itemname is
// mom_gc_alloc_atomic-ed

struct radixitems_mom_st;

struct radix_mom_st
{
  struct mom_itemname_tu *rad_name;
  uint32_t rad_size;            /* allocated size */
  uint32_t rad_count;           /* used count */
  pthread_mutex_t rad_mtx;
  struct mom_item_st **rad_items;       /* of rad_size */
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


// we choose base 60, because with a 0-9 decimal digit then 13 extended
// digits in base 60 we can express a 80-bit number.  Notice that
// log(2**80/10)/log(60) is 12.98112
#define ID_DIGITS_MOM "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNPRSTUVWXYZ"
#define ID_BASE_MOM 60

static_assert (sizeof (ID_DIGITS_MOM) - 1 == ID_BASE_MOM,
               "invalid number of id digits");


static inline const char *
num80_to_char14_mom (mom_uint128_t num, char *buf)
{
  mom_uint128_t initnum = num;
  for (int ix = 13; ix > 0; ix--)
    {
      unsigned dig = num % ID_BASE_MOM;
      num = num / ID_BASE_MOM;
      buf[ix + 1] = ID_DIGITS_MOM[dig];
    }
  if (MOM_UNLIKELY (num > 9))
    MOM_FATAPRINTF ("bad num %d for initnum %16llx/%016llx", (int) num,
                    (unsigned long long) (initnum >> 64),
                    (unsigned long long) (initnum));
  assert (num <= 9);
  buf[1] = '0' + num;
  buf[0] = '_';
  return buf;
}

static inline mom_uint128_t
char14_to_num80_mom (const char *buf)
{
  mom_uint128_t num = 0;
  if (buf[0] != '_')
    return 0;
  if (buf[1] < '0' || buf[1] > '9')
    return 0;
  for (int ix = 1; ix <= 14; ix++)
    {
      char c = buf[ix];
      const char *p = strchr (ID_DIGITS_MOM, c);
      if (!p)
        return 0;
      num = (num * ID_BASE_MOM + (mom_uint128_t) (p - ID_DIGITS_MOM));
    }
  return num;
}


const char *
mom_hi_lo_suffix (char buf[static MOM_HI_LO_SUFFIX_LEN], uint16_t hi,
                  uint64_t lo)
{
  mom_uint128_t i = (((mom_uint128_t) hi) << 64) | lo;
  return num80_to_char14_mom (i, buf);
}


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
                             uint16_t hid, uint64_t loid)
{
  struct mom_item_st *itm = NULL;
  if (!radix)
    return NULL;
  struct radix_mom_st *curad = NULL;
  {
    pthread_mutex_lock (&radix_mtx_mom);
    assert (radix_cnt_mom <= radix_siz_mom);
    uint32_t radrk = radix->itname_rank;
    assert (radrk < radix_cnt_mom);
    curad = radix_arr_mom[radrk];
    assert (curad != NULL);
    pthread_mutex_unlock (&radix_mtx_mom);
  }
  pthread_mutex_lock (&curad->rad_mtx);
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
    struct mom_item_st *newitm = mom_gc_alloc (sizeof (struct mom_item_st));
    newitm->va_ltype = MOM_ITEM_LTYPE;
    newitm->hva_hash = hi;
    newitm->itm_radix = (struct mom_itemname_tu *) radix;
    pthread_mutex_init (&newitm->itm_mtx, &item_mtxattr_mom);
    newitm->itm_hid = hid;
    newitm->itm_lid = loid;
    GC_REGISTER_FINALIZER_IGNORE_SELF (newitm, mom_cleanup_item, NULL, NULL,
                                       NULL);
    itm = newitm;
  }
end:
  pthread_mutex_unlock (&curad->rad_mtx);
  return itm;
}                               /* end of mom_make_item_from_radix_id */

struct mom_item_st *
mom_clone_item_from_radix (const struct mom_itemname_tu *radix)
{
  struct mom_item_st *itm = NULL;
  if (!radix)
    return NULL;
  struct radix_mom_st *curad = NULL;
  {
    pthread_mutex_lock (&radix_mtx_mom);
    assert (radix_cnt_mom <= radix_siz_mom);
    uint32_t radrk = radix->itname_rank;
    assert (radrk < radix_cnt_mom);
    curad = radix_arr_mom[radrk];
    assert (curad != NULL);
    pthread_mutex_unlock (&radix_mtx_mom);
  }
  pthread_mutex_lock (&curad->rad_mtx);
  if (MOM_UNLIKELY (6 * curad->rad_count + 4 > 5 * curad->rad_size))
    {
      unsigned oldcnt = curad->rad_count;
      unsigned newsiz =
        mom_prime_above (4 * oldcnt / 3 +
                         ((oldcnt > 100) ? (oldcnt / 32) : 1) + 10);
      if (MOM_LIKELY (newsiz > curad->rad_size))
        {
          struct mom_item_st **oldarr = curad->rad_items;
          unsigned oldsiz = curad->rad_size;
          struct mom_item_st **newarr =
            mom_gc_alloc (newsiz * sizeof (void *));
          curad->rad_items = newarr;
          curad->rad_size = newsiz;
          curad->rad_count = 0;
          for (unsigned ix = 0; ix < oldsiz; ix++)
            {
              struct mom_item_st *olditm = oldarr[ix];
              if (!olditm || olditm == MOM_EMPTY_SLOT)
                continue;
              int pos = put_item_in_radix_rank_mom (curad, olditm);
              assert (pos >= 0 && pos < (int) newsiz);
              assert (newarr[pos] == NULL);
              newarr[pos] = olditm;
              curad->rad_count++;
            }
        }
    }
  {
    struct mom_item_st *quasitm = mom_gc_alloc (sizeof (struct mom_item_st));
    bool collided = false;
    int pos = -1;
    do
      {
        unsigned hid = 0;
        uint64_t lid = 0;
        collided = false;
        pos = -1;
        do
          {
            hid = mom_random_uint32 () & 0xffff;
            lid =
              (((uint64_t) (mom_random_uint32 ())) << 32) |
              (mom_random_uint32 ());
          }
        while (MOM_UNLIKELY (hid == 0 || lid == 0));
        quasitm->va_ltype = MOM_ITEM_LTYPE;
        quasitm->itm_radix = (struct mom_itemname_tu *) radix;
        quasitm->itm_hid = hid;
        quasitm->itm_lid = lid;
        quasitm->hva_hash = hash_item_from_radix_id_mom (radix, hid, lid);
        pos = put_item_in_radix_rank_mom (curad, quasitm);
        assert (pos >= 0 && pos < (int) curad->rad_size);
        if (curad->rad_items[pos] == NULL
            || curad->rad_items[pos] == MOM_EMPTY_SLOT)
          {
            curad->rad_items[pos] = quasitm;
            curad->rad_count++;
            collided = false;
          }
        else if (curad->rad_items[pos])
          collided = true;
      }
    while (MOM_UNLIKELY (collided));
    pthread_mutex_init (&quasitm->itm_mtx, &item_mtxattr_mom);
    GC_REGISTER_FINALIZER_IGNORE_SELF (quasitm, mom_cleanup_item, NULL, NULL,
                                       NULL);
    itm = quasitm;
  }
  pthread_mutex_unlock (&curad->rad_mtx);
  return itm;
}                               /* end of mom_clone_item_from_radix */


const char *
mom_item_cstring (const struct mom_item_st *itm)
{
  if (!itm || itm == MOM_EMPTY_SLOT)
    return "~";
  if (itm->itm_hid == 0 && itm->itm_lid == 0)
    return mom_item_radix_str (itm);
  else
    {
      char buf[256];
      char bufnum[MOM_HI_LO_SUFFIX_LEN];
      memset (buf, 0, sizeof (buf));
      if (MOM_UNLIKELY (snprintf (buf, sizeof (buf), "%s_%s",
                                  mom_item_radix_str (itm),
                                  mom_item_hi_lo_suffix (bufnum, itm))
                        >= (int) sizeof (buf)))
        MOM_FATAPRINTF ("too long item name %s", buf);
      return GC_STRDUP (buf);
    }
}

int
mom_item_cmp (const struct mom_item_st *itm1, const struct mom_item_st *itm2)
{
  if (itm1 == itm2)
    return 0;
  if (!itm1)
    return -1;
  if (!itm2)
    return 1;
  assert (itm1->va_ltype == MOM_ITEM_LTYPE);
  assert (itm2->va_ltype == MOM_ITEM_LTYPE);
  const struct mom_itemname_tu *rad1 = itm1->itm_radix;
  const struct mom_itemname_tu *rad2 = itm2->itm_radix;
  if (rad1 == rad2)
    {
      if (itm1->itm_hid < itm2->itm_hid)
        return -1;
      else if (itm1->itm_hid > itm2->itm_hid)
        return 1;
      assert (itm1->itm_lid != itm2->itm_lid);
      if (itm1->itm_lid < itm2->itm_lid)
        return -1;
      else if (itm1->itm_lid > itm2->itm_lid)
        return 1;
      else
        MOM_FATAPRINTF ("itm1@%p & itm2@%p are same but non-identical", itm1,
                        itm2);
    }
  else
    {
      int c = 0;
      pthread_mutex_lock (&radix_mtx_mom);
      if (rad1->itname_rank < rad2->itname_rank)
        c = -1;
      else if (rad1->itname_rank > rad2->itname_rank)
        c = 1;
      else
        MOM_FATAPRINTF
          ("itm1@%p & itm2@%p have different radixes %p&%p with same rank %u",
           itm1, itm2, rad1, rad2, rad1->itname_rank);
      pthread_mutex_unlock (&radix_mtx_mom);
      return c;
    }
}                               /* end of mom_item_cmp */

struct mom_anyvalue_st *
mom_assovaldata_get (const struct mom_assovaldata_st *asso,
                     const struct mom_item_st *itmat)
{
  if (!asso || asso == MOM_EMPTY_SLOT || !itmat || itmat == MOM_EMPTY_SLOT)
    return NULL;
  assert (asso->va_ltype == MOM_ASSOVALDATA_LTYPE);
  assert (itmat->va_ltype == MOM_ITEM_LTYPE);
  unsigned cnt = asso->cda_count;
  assert (cnt <= asso->cda_size);
  if (!cnt)
    return NULL;
  int lo = 0, hi = (int) cnt, md = 0;
  while (lo + 5 < hi)
    {
      md = (lo + hi) / 2;
      int c = mom_item_cmp (itmat, asso->ada_ents[md].ient_itm);
      if (c < 0)
        hi = md;
      else if (c > 0)
        lo = md;
      else
        return asso->ada_ents[md].ient_val;
    }
  for (md = lo; md < hi; md++)
    if (itmat == asso->ada_ents[md].ient_itm)
      return asso->ada_ents[md].ient_val;
  return NULL;
}                               /* end of mom_assovaldata_get */



struct mom_assovaldata_st *
mom_assovaldata_remove (struct mom_assovaldata_st *asso,
                        const struct mom_item_st *itmat)
{
  if (!asso || asso == MOM_EMPTY_SLOT || !itmat || itmat == MOM_EMPTY_SLOT)
    return NULL;
  assert (asso->va_ltype == MOM_ASSOVALDATA_LTYPE);
  assert (itmat->va_ltype == MOM_ITEM_LTYPE);

  unsigned cnt = asso->cda_count;
  unsigned siz = asso->cda_size;
  assert (cnt <= siz);
  if (!cnt)
    return asso;
  int lo = 0, hi = (int) cnt, md = 0;
  while (lo + 5 < hi)
    {
      md = (lo + hi) / 2;
      int c = mom_item_cmp (itmat, asso->ada_ents[md].ient_itm);
      if (c < 0)
        hi = md;
      else if (c > 0)
        lo = md;
      else
        goto remove_at_md;
    }
  for (md = lo; md < hi; md++)
    if (itmat == asso->ada_ents[md].ient_itm)
      goto remove_at_md;
  return NULL;
remove_at_md:
  assert (itmat == asso->ada_ents[md].ient_itm);
  assert (cnt > 0);
  if (MOM_UNLIKELY (siz > 10 && cnt < siz / 3))
    {
      unsigned newsiz = mom_prime_above (4 * cnt / 3 + 3);
      if (newsiz < siz)
        {
          struct mom_assovaldata_st *newasso
            = mom_gc_alloc (sizeof (struct mom_assovaldata_st)
                            + newsiz * sizeof (struct mom_itementry_tu));
          newasso->va_ltype = MOM_ASSOVALDATA_LTYPE;
          newasso->cda_size = newsiz;
          for (int ix = 0; ix < md; ix++)
            newasso->ada_ents[ix] = asso->ada_ents[ix];
          for (int ix = md + 1; ix < hi; ix++)
            newasso->ada_ents[ix - 1] = asso->ada_ents[ix];
          newasso->cda_count = cnt - 1;
          return newasso;
        };
    }
  for (int ix = md + 1; ix < hi; ix++)
    asso->ada_ents[ix - 1] = asso->ada_ents[ix];
  asso->ada_ents[cnt - 1].ient_itm = NULL;
  asso->ada_ents[cnt - 1].ient_val = NULL;
  asso->cda_count = cnt - 1;
  return asso;
}

struct mom_assovaldata_st *
mom_assovaldata_reserve (struct mom_assovaldata_st *asso, unsigned gap)
{
  if (gap > MOM_SIZE_MAX)
    MOM_FATAPRINTF ("too large gap %u", gap);
  if (asso == MOM_EMPTY_SLOT)
    asso = NULL;
  if (!asso)
    {
      unsigned newsiz = mom_prime_above (gap + 3);
      struct mom_assovaldata_st *newasso
        = mom_gc_alloc (sizeof (struct mom_assovaldata_st)
                        + newsiz * sizeof (struct mom_itementry_tu));
      newasso->va_ltype = MOM_ASSOVALDATA_LTYPE;
      newasso->cda_size = newsiz;
      return newasso;
    }
  unsigned cnt = asso->cda_count;
  unsigned siz = asso->cda_size;
  if (cnt + gap > MOM_SIZE_MAX)
    MOM_FATAPRINTF ("too large gap %u for count %u", gap, cnt);
  if (cnt + gap <= siz)
    return asso;
  unsigned newsiz = mom_prime_above (gap + cnt + cnt / 16 + 3);
  struct mom_assovaldata_st *newasso
    = mom_gc_alloc (sizeof (struct mom_assovaldata_st)
                    + newsiz * sizeof (struct mom_itementry_tu));
  newasso->va_ltype = MOM_ASSOVALDATA_LTYPE;
  newasso->cda_size = newsiz;
  for (unsigned ix = 0; ix < cnt; ix++)
    newasso->ada_ents[ix] = asso->ada_ents[ix];
  newasso->cda_count = cnt;
  return newasso;

}                               /* end mom_assovaldata_reserve */


struct mom_assovaldata_st *
mom_assovaldata_put (struct mom_assovaldata_st *asso,
                     const struct mom_item_st *itmat, const void *data)
{
  if (asso == MOM_EMPTY_SLOT)
    asso = NULL;
  if (!itmat || itmat == MOM_EMPTY_SLOT)
    return asso;
  if (!data || data == MOM_EMPTY_SLOT)
    return mom_assovaldata_remove (asso, itmat);
  if (!asso)
    {
      const unsigned newsiz = 5;
      struct mom_assovaldata_st *newasso
        = mom_gc_alloc (sizeof (struct mom_assovaldata_st)
                        + newsiz * sizeof (struct mom_itementry_tu));
      newasso->va_ltype = MOM_ASSOVALDATA_LTYPE;
      newasso->cda_size = newsiz;
      newasso->ada_ents[0].ient_itm = (struct mom_item_st *) itmat;
      newasso->ada_ents[0].ient_val =
        (struct mom_anyvalue_st *) (void *) data;
      newasso->cda_count = 1;
      return newasso;
    };
  assert (asso->va_ltype == MOM_ASSOVALDATA_LTYPE);
  assert (itmat->va_ltype == MOM_ITEM_LTYPE);
  unsigned cnt = asso->cda_count;
  unsigned siz = asso->cda_size;
  assert (cnt <= siz);
  if (cnt >= MOM_SIZE_MAX)
    MOM_FATAPRINTF ("too big association %d", cnt);
  int lo = 0, hi = (int) cnt, md = 0;
  while (lo + 5 < hi)
    {
      md = (lo + hi) / 2;
      int c = mom_item_cmp (itmat, asso->ada_ents[md].ient_itm);
      if (c < 0)
        hi = md;
      else if (c > 0)
        lo = md;
      else
        {
          assert (itmat == asso->ada_ents[md].ient_itm);
          asso->ada_ents[md].ient_val =
            (struct mom_anyvalue_st *) (void *) data;
          return asso;
        }
    };
  if (MOM_UNLIKELY (cnt >= siz))
    {
      assert (cnt == siz);
      unsigned newsiz =
        mom_prime_above (5 * cnt / 4 + ((cnt > 32) ? (cnt / 32) : 1) + 2);
      assert (newsiz > siz);
      struct mom_assovaldata_st *newasso
        = mom_gc_alloc (sizeof (struct mom_assovaldata_st)
                        + newsiz * sizeof (struct mom_itementry_tu));
      newasso->va_ltype = MOM_ASSOVALDATA_LTYPE;
      newasso->cda_size = newsiz;
      for (int ix = 0; ix < lo; ix++)
        newasso->ada_ents[ix] = asso->ada_ents[ix];
      int inspos = -1;
      for (int ix = lo; ix < hi; ix++)
        {
          int c = mom_item_cmp (asso->ada_ents[ix].ient_itm, itmat);
          if (c < 0)
            newasso->ada_ents[ix] = asso->ada_ents[ix];
          else if (c > 0)
            {
              inspos = ix;
              break;
            }
          else
            {
              assert (asso->ada_ents[ix].ient_itm == itmat);
              newasso->ada_ents[ix].ient_itm = (struct mom_item_st *) itmat;
              newasso->ada_ents[ix].ient_val =
                (struct mom_anyvalue_st *) data;
              for (int j = ix + 1; j < (int) cnt; j++)
                newasso->ada_ents[j] = asso->ada_ents[j];
              newasso->cda_count = cnt;
              return newasso;
            }
        };
      if (inspos < 0)
        inspos = hi;
      newasso->ada_ents[inspos].ient_itm = (struct mom_item_st *) itmat;
      newasso->ada_ents[inspos].ient_val = (struct mom_anyvalue_st *) data;
      for (int ix = inspos; ix < (int) cnt; ix++)
        newasso->ada_ents[ix + 1] = asso->ada_ents[ix];
      newasso->cda_count = cnt + 1;
      return newasso;
    };
  for (int ix = lo; ix < (int) cnt; ix++)
    {
      int c = mom_item_cmp (asso->ada_ents[ix].ient_itm, itmat);
      if (c < 0)
        continue;
      else if (c > 0)
        {
          for (int j = cnt; j > ix; j++)
            asso->ada_ents[j] = asso->ada_ents[j - 1];
          asso->cda_count = cnt + 1;
          return asso;
        }
      else
        {
          assert (asso->ada_ents[ix].ient_itm == itmat);
          asso->ada_ents[ix].ient_itm = (struct mom_item_st *) itmat;
          asso->ada_ents[ix].ient_val = (struct mom_anyvalue_st *) data;
          return asso;
        }
    };
  asso->ada_ents[cnt].ient_itm = (struct mom_item_st *) itmat;
  asso->ada_ents[cnt].ient_val = (struct mom_anyvalue_st *) (void *) data;
  asso->cda_count = cnt + 1;
  return asso;
}                               /* end of mom_assovaldata_put */



struct mom_vectvaldata_st *
mom_vectvaldata_reserve (struct mom_vectvaldata_st *vec, unsigned gap)
{
  if (vec == MOM_EMPTY_SLOT)
    vec = NULL;
  if (gap > MOM_SIZE_MAX)
    MOM_FATAPRINTF ("too big gap %u", gap);
  if (!vec)
    {
      unsigned siz = mom_prime_above (gap + gap / 16 + 2);
      vec = mom_gc_alloc (sizeof (*vec) + siz * sizeof (void *));
      vec->va_ltype = MOM_VECTVALDATA_LTYPE;
      vec->cda_size = siz;
      return vec;
    }
  unsigned cnt = vec->cda_count;
  unsigned siz = vec->cda_size;
  if (cnt + gap >= siz)
    {
      if (cnt + gap >= MOM_SIZE_MAX)
        MOM_FATAPRINTF ("too big gap %u for count %u", gap, cnt);
      unsigned newsiz = mom_prime_above (cnt + gap + cnt / 16 + 4);
      struct mom_vectvaldata_st *newvec
        = mom_gc_alloc (sizeof (*vec) + newsiz * sizeof (void *));
      newvec->va_ltype = MOM_VECTVALDATA_LTYPE;
      newvec->cda_size = newsiz;
      mempcpy (newvec->vecd_valarr, vec->vecd_valarr, cnt * sizeof (void *));
      newvec->cda_count = cnt;
      return newvec;
    }
  return vec;
}


void
mom_initialize_items (void)
{
  static bool inited;
  if (inited)
    return;
  inited = true;
  pthread_mutexattr_init (&item_mtxattr_mom);
  pthread_mutexattr_settype (&item_mtxattr_mom, PTHREAD_MUTEX_RECURSIVE);
}
