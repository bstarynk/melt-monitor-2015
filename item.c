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

bool
mom_suffix_to_hi_lo (const char *buf, uint16_t *phi, uint64_t *plo)
{
  if (!buf || buf[0] != '_')
    return false;
  mom_uint128_t i = char14_to_num80_mom (buf);
  if (i)
    {
      if (phi)
        *phi = (uint16_t) (i >> 64);
      if (plo)
        *plo = (uint64_t) i;
      return true;
    }
  return false;
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
          nam->itname_string.va_itype = MOMITY_BOXSTRING;
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
          nam->itname_string.va_itype = MOMITY_BOXSTRING;
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


struct mom_item_st *
mom_find_item_from_string (const char *str, const char **pend)
{
  struct mom_item_st *itm = NULL;
  if (!str || str == MOM_EMPTY_SLOT || !isalpha (str[0]))
    return NULL;
  const char *endradix = str;
  while (*endradix)
    {
      if (!isalnum (*endradix) && *endradix != '_')
        break;
      if (*endradix == '_' && endradix[1] == '_')
        break;
      endradix++;
    };
  const char *end = NULL;
  uint16_t hid = 0;
  uint64_t lid = 0;
  const struct mom_itemname_tu *radix = NULL;
  radix = mom_find_name_radix (str, endradix - str);
  if (!radix)
    goto not_found;
  if (endradix && endradix[0] == '_' && endradix[1] == '_'
      && isdigit (endradix[2])
      && mom_suffix_to_hi_lo (endradix + 1, &hid, &lid) && hid > 0 && lid > 0)
    {
      end = endradix + 15;
      itm = mom_find_item_from_radix_id (radix, hid, lid);
      if (itm)
        {
          if (pend)
            *pend = end;
          MOM_DEBUGPRINTF (item,
                           "mom_find_item_from_string str=%.*s itm=%s (hid=%d lid=%lld)",
                           (int) (end - str), str, mom_item_cstring (itm),
                           hid, (long long) lid);
          return itm;
        }
      else
        goto not_found;
    }
  itm = mom_find_item_from_radix_id (radix, 0, 0);
  end = endradix;
  if (itm)
    {
      if (pend)
        *pend = end;
      MOM_DEBUGPRINTF (item, "mom_find_item_from_string str=%.*s itm=%s",
                       (int) (end - str), str, mom_item_cstring (itm));
      return itm;
    }
not_found:
  MOM_DEBUGPRINTF (item, "mom_find_item_from_string str=%.50s not found",
                   str);
  return NULL;
}                               /* end mom_find_item_from_string */


struct mom_item_st *
mom_make_item_from_string (const char *str, const char **pend)
{
  struct mom_item_st *itm = NULL;
  if (!str || str == MOM_EMPTY_SLOT || !isalpha (str[0]))
    return NULL;
  const char *endradix = str;
  while (*endradix)
    {
      if (!isalnum (*endradix) && *endradix != '_')
        break;
      if (*endradix == '_' && endradix[1] == '_')
        break;
      endradix++;
    };
  const char *end = NULL;
  uint16_t hid = 0;
  uint64_t lid = 0;
  const struct mom_itemname_tu *radix = NULL;
  radix = mom_make_name_radix (str, endradix - str);
  if (!radix)
    goto not_found;
  if (endradix && endradix[0] == '_' && endradix[1] == '_'
      && isdigit (endradix[2])
      && mom_suffix_to_hi_lo (endradix + 1, &hid, &lid) && hid > 0 && lid > 0)
    {
      end = endradix + 15;
      itm = mom_make_item_from_radix_id (radix, hid, lid);
      if (!itm)
        goto not_found;
      if (pend)
        *pend = end;
      MOM_DEBUGPRINTF (item,
                       "mom_make_item_from_string str=%.*s itm=%s (hid=%d lid=%lld)",
                       (int) (end - str), str, mom_item_cstring (itm),
                       hid, (long long) lid);
      return itm;
    }
  itm = mom_make_item_from_radix_id (radix, 0, 0);
  end = endradix;
  if (itm)
    {
      if (pend)
        *pend = end;
      MOM_DEBUGPRINTF (item, "mom_make_item_from_string str=%.*s itm=%s",
                       (int) (end - str), str, mom_item_cstring (itm));
      return itm;
    }
not_found:
  MOM_DEBUGPRINTF (item, "mom_make_item_from_string str=%.50s not found",
                   str);
  return NULL;
}                               /* end mom_make_item_from_string */


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
    newitm->va_itype = MOMITY_ITEM;
    newitm->hva_hash = hi;
    newitm->itm_radix = (struct mom_itemname_tu *) radix;
    time (&newitm->itm_mtime);
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
        quasitm->va_itype = MOMITY_ITEM;
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
    time (&quasitm->itm_mtime);
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
  assert (itm1->va_itype == MOMITY_ITEM);
  assert (itm2->va_itype == MOMITY_ITEM);
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

struct mom_hashedvalue_st *
mom_assovaldata_get (const struct mom_assovaldata_st *asso,
                     const struct mom_item_st *itmat)
{
  if (!asso || asso == MOM_EMPTY_SLOT || !itmat || itmat == MOM_EMPTY_SLOT)
    return NULL;
  assert (asso->va_itype == MOMITY_ASSOVALDATA);
  assert (itmat->va_itype == MOMITY_ITEM);
  unsigned cnt = asso->cda_count;
  assert (cnt <= mom_raw_size (asso));
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
  assert (asso->va_itype == MOMITY_ASSOVALDATA);
  assert (itmat->va_itype == MOMITY_ITEM);

  unsigned cnt = asso->cda_count;
  unsigned siz = mom_raw_size (asso);
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
          newasso->va_itype = MOMITY_ASSOVALDATA;
          mom_put_size (newasso, newsiz);
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
      newasso->va_itype = MOMITY_ASSOVALDATA;
      mom_put_size (newasso, newsiz);
      return newasso;
    }
  unsigned cnt = asso->cda_count;
  unsigned siz = mom_raw_size (asso);
  if (cnt + gap > MOM_SIZE_MAX)
    MOM_FATAPRINTF ("too large gap %u for count %u", gap, cnt);
  if (cnt + gap <= siz)
    return asso;
  unsigned newsiz = mom_prime_above (gap + cnt + cnt / 16 + 3);
  struct mom_assovaldata_st *newasso
    = mom_gc_alloc (sizeof (struct mom_assovaldata_st)
                    + newsiz * sizeof (struct mom_itementry_tu));
  newasso->va_itype = MOMITY_ASSOVALDATA;
  mom_put_size (newasso, newsiz);
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
      newasso->va_itype = MOMITY_ASSOVALDATA;
      mom_put_size (newasso, newsiz);
      newasso->ada_ents[0].ient_itm = (struct mom_item_st *) itmat;
      newasso->ada_ents[0].ient_val =
        (struct mom_hashedvalue_st *) (void *) data;
      newasso->cda_count = 1;
      return newasso;
    };
  assert (asso->va_itype == MOMITY_ASSOVALDATA);
  assert (itmat->va_itype == MOMITY_ITEM);
  unsigned cnt = asso->cda_count;
  unsigned siz = mom_raw_size (asso);
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
            (struct mom_hashedvalue_st *) (void *) data;
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
      newasso->va_itype = MOMITY_ASSOVALDATA;
      mom_put_size (newasso, newsiz);
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
                (struct mom_hashedvalue_st *) data;
              for (int j = ix + 1; j < (int) cnt; j++)
                newasso->ada_ents[j] = asso->ada_ents[j];
              newasso->cda_count = cnt;
              return newasso;
            }
        };
      if (inspos < 0)
        inspos = hi;
      newasso->ada_ents[inspos].ient_itm = (struct mom_item_st *) itmat;
      newasso->ada_ents[inspos].ient_val = (struct mom_hashedvalue_st *) data;
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
          asso->ada_ents[ix].ient_val = (struct mom_hashedvalue_st *) data;
          return asso;
        }
    };
  asso->ada_ents[cnt].ient_itm = (struct mom_item_st *) itmat;
  asso->ada_ents[cnt].ient_val = (struct mom_hashedvalue_st *) (void *) data;
  asso->cda_count = cnt + 1;
  return asso;
}                               /* end of mom_assovaldata_put */

void
mom_dumpscan_assovaldata (struct mom_dumper_st *du,
                          struct mom_assovaldata_st *asso)
{
  if (!asso || asso == MOM_EMPTY_SLOT)
    return;
  assert (du && du->va_itype == MOMITY_DUMPER);
  unsigned cnt = asso->cda_count;
  unsigned siz = mom_raw_size (asso);
  assert (cnt <= siz);
  for (int ix = 0; ix < (int) cnt; ix++)
    {
      if (!mom_dumpable_item (asso->ada_ents[ix].ient_itm))
        continue;
      mom_dumpscan_item (du, asso->ada_ents[ix].ient_itm);
      mom_dumpscan_value (du, asso->ada_ents[ix].ient_val);
    }
}

struct mom_vectvaldata_st *
mom_vectvaldata_reserve (struct mom_vectvaldata_st *vec, unsigned gap)
{
  if (vec == MOM_EMPTY_SLOT)
    vec = NULL;
  if (gap > MOM_SIZE_MAX)
    MOM_FATAPRINTF ("too big gap %u", gap);
  if (!vec || vec->va_itype != MOMITY_VECTVALDATA)
    {
      unsigned siz = mom_prime_above (gap + gap / 16 + 2);
      vec = mom_gc_alloc (sizeof (*vec) + siz * sizeof (void *));
      vec->va_itype = MOMITY_VECTVALDATA;
      mom_put_size (vec, siz);
      return vec;
    }
  unsigned cnt = vec->cda_count;
  unsigned siz = mom_raw_size (vec);
  if (cnt + gap >= siz)
    {
      if (cnt + gap >= MOM_SIZE_MAX)
        MOM_FATAPRINTF ("too big gap %u for count %u", gap, cnt);
      unsigned newsiz = mom_prime_above (cnt + gap + cnt / 16 + 4);
      struct mom_vectvaldata_st *newvec
        = mom_gc_alloc (sizeof (*vec) + newsiz * sizeof (void *));
      newvec->va_itype = MOMITY_VECTVALDATA;
      mom_put_size (newvec, newsiz);
      mempcpy (newvec->vecd_valarr, vec->vecd_valarr, cnt * sizeof (void *));
      newvec->cda_count = cnt;
      return newvec;
    }
  return vec;
}


struct mom_vectvaldata_st *
mom_vectvaldata_resize (struct mom_vectvaldata_st *vec, unsigned count)
{
  if (!vec || vec == MOM_EMPTY_SLOT || vec->va_itype != MOMITY_VECTVALDATA)
    {
      unsigned newsiz = mom_prime_above (count + count / 8 + 2);
      struct mom_vectvaldata_st *newvec
        = mom_gc_alloc (sizeof (*vec) + newsiz * sizeof (void *));
      newvec->va_itype = MOMITY_VECTVALDATA;
      mom_put_size (newvec, newsiz);
      newvec->cda_count = count;
      return newvec;
    }
  unsigned oldcount = vec->cda_count;
  unsigned size = mom_raw_size (vec);
  assert (oldcount <= size);
  if (count > size)
    {
      unsigned newsiz = mom_prime_above (count + count / 8 + 2);
      struct mom_vectvaldata_st *newvec
        = mom_gc_alloc (sizeof (*vec) + newsiz * sizeof (void *));
      newvec->va_itype = MOMITY_VECTVALDATA;
      mom_put_size (newvec, newsiz);
      newvec->cda_count = count;
      memcpy (newvec->vecd_valarr, vec->vecd_valarr,
              oldcount * sizeof (void *));
      return newvec;
    }
  else if (count < size / 2 && size > 10)
    {
      unsigned newsiz = mom_prime_above (count + count / 8 + 2);
      if (newsiz != size)
        {
          struct mom_vectvaldata_st *newvec
            = mom_gc_alloc (sizeof (*vec) + newsiz * sizeof (void *));
          newvec->va_itype = MOMITY_VECTVALDATA;
          mom_put_size (newvec, newsiz);
          newvec->cda_count = count;
          memcpy (newvec->vecd_valarr, vec->vecd_valarr,
                  ((oldcount < count) ? oldcount : count) * sizeof (void *));
          return newvec;
        }
    }
  if (count == oldcount)
    return vec;
  if (count < oldcount)
    memset (vec->vecd_valarr + count, 0,
            (oldcount - count) * sizeof (void *));
  else
    memset (vec->vecd_valarr + oldcount, 0,
            (count - oldcount) * sizeof (void *));
  vec->cda_count = count;
  return vec;
}                               /* end of mom_vectvaldata_resize */


void
mom_dumpscan_vectvaldata (struct mom_dumper_st *du,
                          struct mom_vectvaldata_st *vec)
{
  assert (du && du->va_itype == MOMITY_DUMPER);
  if (!vec || vec == MOM_EMPTY_SLOT || vec->va_itype != MOMITY_VECTVALDATA)
    return;
  unsigned cnt = vec->cda_count;
  assert (cnt <= mom_raw_size (vec));
  for (int ix = 0; ix < (int) cnt; ix++)
    if (vec->vecd_valarr[ix])
      mom_dumpscan_value (du, vec->vecd_valarr[ix]);
}                               /* end of mom_dumpscan_vectvaldata */


struct mom_vectvaldata_st *
mom_vectvaldata_append (struct mom_vectvaldata_st *vec, const void *data)
{
  if (vec == MOM_EMPTY_SLOT)
    vec = NULL;
  if (data == MOM_EMPTY_SLOT)
    data = NULL;
  if (!vec || vec->va_itype != MOMITY_VECTVALDATA)
    {
      const unsigned nsiz = 5;
      vec = mom_gc_alloc (sizeof (*vec) + nsiz * sizeof (void *));
      vec->va_itype = MOMITY_VECTVALDATA;
      mom_put_size (vec, nsiz);
      vec->cda_count = 1;
      vec->vecd_valarr[0] = (struct mom_hashedvalue_st *) data;
      return vec;
    }
  unsigned siz = mom_raw_size (vec);
  unsigned cnt = vec->cda_count;
  assert (cnt <= siz);
  if (cnt >= siz)
    {
      assert (cnt == siz);
      unsigned newsiz =
        mom_prime_above (5 * cnt / 4 + ((cnt > 100) ? (cnt / 32) : 1) + 2);
      struct mom_vectvaldata_st *newvec =
        mom_gc_alloc (sizeof (*vec) + newsiz * sizeof (void *));
      newvec->va_itype = MOMITY_VECTVALDATA;
      mom_put_size (newvec, newsiz);
      newvec->cda_count = cnt;
      memcpy (newvec->vecd_valarr, vec->vecd_valarr, cnt * sizeof (void *));
      vec = newvec;
    }
  vec->vecd_valarr[cnt++] = (struct mom_hashedvalue_st *) data;
  vec->cda_count = cnt;
  return vec;
}                               /* end of mom_vectvaldata_append */





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
#define MOM_HAS_PREDEFINED(Nam,Hash)		\
  struct mom_item_st mompredef_##Nam = {	\
  .va_itype = MOMITY_ITEM,			\
  .hva_hash = Hash,				\
  .itm_hid = 0,					\
  .itm_lid = 0,					\
  };
#include "_mom_predef.h"


static struct mom_hashset_st *predef_hset_mom;
static pthread_mutex_t predef_mtx_mom = PTHREAD_MUTEX_INITIALIZER;

static void
initialize_predefined_mom (struct mom_item_st *itm, const char *name,
                           momhash_t hash)
{
  assert (itm->va_itype == MOMITY_ITEM && itm->hva_hash == hash);
  MOM_DEBUGPRINTF (item, "initialize_predefined %s", name);
  const char *twou = strstr (name, "__");
  uint16_t hid = 0;
  uint64_t lid = 0;
  if (twou)
    {
#warning initialize_predefined unimplemented for cloned
      MOM_FATAPRINTF ("initialize_predefined unimplemented for cloned %s",
                      name);
    }
  else
    {
      hid = 0;
      lid = 0;
    };
  const struct mom_itemname_tu *radix = mom_make_name_radix (name, -1);
  if (!radix)
    MOM_FATAPRINTF ("initialize_predefined failed to make radix %s", name);
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
  momhash_t hi = hash_item_from_radix_id_mom (radix, hid, lid);
  assert (hi == hash);
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
  itm->va_itype = MOMITY_ITEM;
  itm->itm_radix = (struct mom_itemname_tu *) radix;
  itm->itm_hid = hid;
  itm->itm_lid = lid;
  int pos = put_item_in_radix_rank_mom (curad, itm);
  assert (pos >= 0 && pos < (int) sz);
  assert (curad->rad_items[pos] == NULL);
  curad->rad_items[pos] = itm;
  pthread_mutex_unlock (&curad->rad_mtx);
  mom_item_put_space (itm, MOMSPA_PREDEF);
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
  predef_hset_mom =
    mom_hashset_reserve (NULL,
                         (4 * MOM_NB_PREDEFINED / 3 +
                          MOM_NB_PREDEFINED / 32) + 20);
#define MOM_HAS_PREDEFINED(Nam,Hash) \
  initialize_predefined_mom(&mompredef_##Nam, #Nam, Hash);
#include "_mom_predef.h"
  MOM_DEBUGPRINTF (item, "done predefined %d", MOM_NB_PREDEFINED);
}

void
mom_item_put_space (struct mom_item_st *itm, enum mom_space_en sp)
{
  if (!itm || itm == MOM_EMPTY_SLOT || itm->va_itype != MOMITY_ITEM)
    return;
  pthread_mutex_lock (&itm->itm_mtx);
  if (itm->va_ixv == (unsigned) sp)
    goto end;
  enum mom_space_en oldsp = itm->va_ixv;
  itm->va_ixv = (uint16_t) sp;
  if (sp != MOMSPA_PREDEF && oldsp != MOMSPA_PREDEF)
    goto end;
  pthread_mutex_lock (&predef_mtx_mom);
  if (oldsp == MOMSPA_PREDEF)
    {                           // remove a predefined
      predef_hset_mom = mom_hashset_remove (predef_hset_mom, itm);
    }
  else
    {                           // add a predefined
      predef_hset_mom = mom_hashset_insert (predef_hset_mom, itm);
    }
  pthread_mutex_unlock (&predef_mtx_mom);
end:
  pthread_mutex_unlock (&itm->itm_mtx);
}                               /* end of mom_item_put_space */


const struct mom_boxset_st *
mom_predefined_items_boxset (void)
{
  const struct mom_boxset_st *set = NULL;
  pthread_mutex_lock (&predef_mtx_mom);
  set = mom_hashset_to_boxset (predef_hset_mom);
  pthread_mutex_unlock (&predef_mtx_mom);
  return set;
}                               /* end of mom_predefined_items_boxset */


void
mom_dumpemit_refitem (struct mom_dumper_st *du, const struct mom_item_st *itm)
{
  assert (du && du->va_itype == MOMITY_DUMPER);
  assert (du->du_state == MOMDUMP_EMIT);
  FILE *femit = du->du_emitfile;
  assert (femit != NULL);
  if (itm == NULL || itm == MOM_EMPTY_SLOT || !mom_dumped_item (du, itm))
    fputs ("~\n", femit);
  else
    fprintf (femit, "%s\n", mom_item_cstring (itm));
  return;
}                               /* end of mom_dumpemit_refitem */

static void
dumpemit_assovaldata_mom (struct mom_dumper_st *du,
                          const struct mom_assovaldata_st *asso)
{
  assert (du && du->va_itype == MOMITY_DUMPER);
  assert (du->du_state == MOMDUMP_EMIT);
  FILE *femit = du->du_emitfile;
  assert (femit != NULL);
  assert (asso && asso != MOM_EMPTY_SLOT
          && asso->va_itype == MOMITY_ASSOVALDATA);
  unsigned sizattr = mom_size (asso);
  unsigned cntattr = asso->cda_count;
  if (sizattr > 0 && cntattr > 0)
    {
      assert (cntattr <= sizattr);
      for (unsigned ix = 0; ix < cntattr; ix++)
        {
          const struct mom_item_st *itmat = asso->ada_ents[ix].ient_itm;
          if (!itmat || itmat == MOM_EMPTY_SLOT
              || !mom_dumped_item (du, itmat))
            continue;
          const struct mom_hashedvalue_st *valat =
            asso->ada_ents[ix].ient_val;
          if (!valat || valat == MOM_EMPTY_SLOT
              || !mom_dumped_value (du, valat))
            continue;
          mom_dumpemit_refitem (du, itmat);
          mom_dumpemit_value (du, asso->ada_ents[ix].ient_val);
        };
    }
}

void
mom_dumpemit_item_content (struct mom_dumper_st *du,
                           const struct mom_item_st *itm)
{
  assert (du && du->va_itype == MOMITY_DUMPER);
  assert (du->du_state == MOMDUMP_EMIT);
  FILE *femit = du->du_emitfile;
  assert (femit != NULL);
  /// emit the mtime
  if (itm->itm_mtime)
    {
      fprintf (femit, "%ld\n" "^mtime\n", (long) itm->itm_mtime);
    }
  /// emit the funptr
  if (itm->itm_funptr && itm->itm_funptr != MOM_EMPTY_SLOT)
    {
      Dl_info dinf;
      memset (&dinf, 0, sizeof (dinf));
      if (dladdr (itm->itm_funptr, &dinf)
          && dinf.dli_saddr == itm->itm_funptr
          && !strncmp (dinf.dli_sname, MOM_FUNC_PREFIX,
                       strlen (MOM_FUNC_PREFIX)))
        {
          if (strcmp
              (dinf.dli_sname + strlen (MOM_FUNC_PREFIX),
               mom_item_cstring (itm)))
            fprintf (femit, "%s\n" "^altfunc",
                     dinf.dli_sname + strlen (MOM_FUNC_PREFIX));
          else
            fputs ("^func\n", femit);
        }
    }
  /// emit the attributes
  if (itm->itm_pattr && itm->itm_pattr != MOM_EMPTY_SLOT)
    {
      const struct mom_assovaldata_st *asso = itm->itm_pattr;
      if (asso->cda_count > 0)
        {
          fputs ("(\n", femit);
          dumpemit_assovaldata_mom (du, asso);
          fputs (")attrs\n", femit);
        }
    }
  //// should dump the vector
  if (itm->itm_pcomp && itm->itm_pcomp != MOM_EMPTY_SLOT)
    {
      struct mom_vectvaldata_st *comps = itm->itm_pcomp;
      unsigned siz = mom_raw_size (comps);
      unsigned cnt = comps->cda_count;
      assert (cnt <= siz);
      fputs ("(\n", femit);
      for (unsigned ix = 0; ix < cnt; ix++)
        mom_dumpemit_value (du, comps->vecd_valarr[ix]);
      fputs (")comps\n", femit);
    }
  //// should dump the payload
  if (itm->itm_payload && itm->itm_payload != MOM_EMPTY_SLOT)
    {
      struct mom_anyvalue_st *payl = itm->itm_payload;
      switch (payl->va_itype)
        {
        case MOMITY_BOXINT:
        case MOMITY_BOXDOUBLE:
        case MOMITY_BOXSTRING:
        case MOMITY_TUPLE:
        case MOMITY_SET:
        case MOMITY_NODE:
        case MOMITY_ITEM:
          mom_dumpemit_value (du, (struct mom_hashedvalue_st *) payl);
          fputs ("^payloadval\n", femit);
          break;
        case MOMITY_ASSOVALDATA:
          fputs ("(\n", femit);
          dumpemit_assovaldata_mom (du, (struct mom_assovaldata_st *) payl);
          fputs (")payloadassoval\n", femit);
          break;
        case MOMITY_VECTVALDATA:
          {
            struct mom_vectvaldata_st *comps =
              (struct mom_vectvaldata_st *) payl;
            unsigned siz = mom_raw_size (comps);
            unsigned cnt = comps->cda_count;
            assert (cnt <= siz);
            fputs ("(\n", femit);
            for (unsigned ix = 0; ix < cnt; ix++)
              mom_dumpemit_value (du, comps->vecd_valarr[ix]);
            fputs (")payloadvect\n", femit);
          }
          break;
        case MOMITY_QUEUE:
          {
            struct mom_queue_st *qu = (struct mom_queue_st *) payl;
            fputs ("(\n", femit);
            if (qu->qu_first && qu->qu_first != MOM_EMPTY_SLOT)
              {
                for (struct mom_quelem_st * ql = qu->qu_first; ql != NULL;
                     ql = ql->qu_next)
                  {
                    for (unsigned ix = 0; ix < MOM_NB_QUELEM; ix++)
                      {
                        struct mom_hashedvalue_st *curval = ql->qu_elems[ix];
                        if (!curval || curval == MOM_EMPTY_SLOT)
                          continue;
                        mom_dumpemit_value (du, curval);
                      }
                  }
              }
            fputs (")payloadqueue\n", femit);
          }
          break;
        case MOMITY_HASHSET:
          {
            struct mom_hashset_st *hset = (struct mom_hashset_st *) payl;
            const struct mom_boxset_st *bxset = mom_hashset_to_boxset (hset);
            unsigned siz = mom_size (bxset);
            fputs ("(\n", femit);
            for (unsigned ix = 0; ix < siz; ix++)
              {
                struct mom_item_st *elemitm = bxset->seqitem[ix];
                if (!mom_dumped_item (du, elemitm))
                  continue;
                mom_dumpemit_refitem (du, elemitm);
              }
            fputs (")payloadhashset\n", femit);
          }
          break;
        case MOMITY_HASHMAP:
          {
            struct mom_hashmap_st *hmap = (struct mom_hashmap_st *) payl;
            const struct mom_boxset_st *kset = mom_hashmap_keyset (hmap);
            unsigned siz = mom_size (kset);
            fputs ("(\n", femit);
            for (unsigned ix = 0; ix < siz; ix++)
              {
                struct mom_item_st *keyitm = kset->seqitem[ix];
                if (!mom_dumped_item (du, keyitm))
                  continue;
                const struct mom_hashedvalue_st *val
                  = mom_hashmap_get (hmap, keyitm);
                if (!val || mom_dumped_value (du, val))
                  continue;
                mom_dumpemit_refitem (du, keyitm);
                mom_dumpemit_value (du, val);
              }
            fputs (")payloadhashmap\n", femit);
          }
          break;
        default:
          break;
        }
    }
}                               /* end of mom_dumpemit_item_content */
