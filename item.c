// file item.c - managing items

/**   Copyright (C)  2015, 2016  Basile Starynkevitch and later the FSF
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

#define MAXLEN_SUFFIXEDITEM_MOM 256
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
  struct mom_item_st *rad_nakeditem;    /* the item without any prefix, i.e. zero hid & lid */
  struct mom_item_st **rad_items;       /* hashtable of rad_size */
};


/// actually, it might be better to have each element of the array be
/// individually allocated, with its own mutex...

static struct radix_mom_st **radix_arr_mom;     // sorted by alphabetic name
static unsigned radix_siz_mom;  /* allocated size of radix_arr_mom */
static unsigned radix_cnt_mom;  /* used count of radix_arr_mom */


bool
mom_valid_name_radix_len (const char *str, int len)
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
mom_find_name_radix (const char *str)
{
  struct mom_itemname_tu *tun = NULL;
  if (!str || str == MOM_EMPTY_SLOT || !str[0])
    return NULL;
  if (!mom_valid_name_radix (str))
    return NULL;
  pthread_mutex_lock (&radix_mtx_mom);
  assert (radix_cnt_mom <= radix_siz_mom);
  if (radix_cnt_mom == 0)
    goto end;
  int lo = 0, hi = (int) radix_cnt_mom;
  while (lo + 5 < hi)
    {
      int md = (lo + hi) / 2;
      struct mom_itemname_tu *curad = radix_arr_mom[md]->rad_name;
      assert (curad != NULL);
      assert (curad->itname_rank == (unsigned) md);
      int c = strcmp (str, curad->itname_string.cstr);
      if (c == 0)
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
      assert (ix == 0
              || strcmp (radix_arr_mom[ix - 1]->rad_name->itname_string.cstr,
                         curad->itname_string.cstr) < 0);
      assert (curad->itname_rank == (unsigned) ix);
      if (!strcmp (curad->itname_string.cstr, str))
        {
          tun = curad;
          goto end;
        };
    }
end:
  pthread_mutex_unlock (&radix_mtx_mom);
  return tun;
}                               /* end of mom_find_name_radix  */


/// to be called from gdb
void
mom_debugprint_radixtable (void)
{
  printf ("radix_cnt_mom=%d radix_siz_mom=%d\n",
          radix_cnt_mom, radix_siz_mom);
  for (int ix = 0; ix < (int) radix_cnt_mom; ix++)
    {
      if (radix_arr_mom[ix])
        printf ("radix_arr_mom[%d]: %s (@%p)",
                ix, radix_arr_mom[ix]->rad_name->itname_string.cstr,
                (void *) radix_arr_mom[ix]);
      else
        printf ("radix_arr_mom[%d] **NULL**", ix);
      if (radix_arr_mom[ix]->rad_name->itname_rank != (unsigned) ix)
        printf (" !!ix:%d != itname_rank:%d!!", ix,
                radix_arr_mom[ix]->rad_name->itname_rank);
      putchar ('\n');
    }
  for (int ix = 1; ix < (int) radix_cnt_mom; ix++)
    {
      if (!radix_arr_mom[ix - 1] || !radix_arr_mom[ix])
        continue;
      if (strcmp (radix_arr_mom[ix - 1]->rad_name->itname_string.cstr,
                  radix_arr_mom[ix]->rad_name->itname_string.cstr) >= 0)
        {
          MOM_WARNPRINTF ("missorted radix ([%d] %s :: [%d] %s)", ix - 1,
                          radix_arr_mom[ix - 1]->rad_name->itname_string.cstr,
                          ix,
                          radix_arr_mom[ix]->rad_name->itname_string.cstr);
          assert (ix > 0);
        }
    }
}                               /* end mom_debugprint_radixtable */




static long makeradixcounter_mom;


struct both_name_and_radix_mom_st
{
  struct mom_itemname_tu *btn_name;
  struct radix_mom_st *btn_radix;
};

struct both_name_and_radix_mom_st
put_name_radix_mom (int ix, const char *str)
{
  assert (ix >= 0 && ix < (int) radix_siz_mom);
  assert (str != NULL);
  int len = (int) strlen (str);
  assert (radix_arr_mom[ix] == NULL);
  struct mom_itemname_tu *newnam =
    mom_gc_alloc_atomic (((sizeof (struct mom_itemname_tu) + len +
                           2) | 3) + 1);
  newnam->itname_rank = ix;
  newnam->itname_string.va_itype = MOMITY_BOXSTRING;
  newnam->itname_string.va_hsiz = len >> 8;
  newnam->itname_string.va_lsiz = len & 0xffff;
  newnam->itname_string.hva_hash = mom_cstring_hash_len (str, len);
  strncpy (newnam->itname_string.cstr, str, len);
  newnam->itname_string.cstr[len] = (char) 0;
  struct radix_mom_st *newrad = mom_gc_alloc (sizeof (struct radix_mom_st));
  const unsigned itmsiz = 7;
  newrad->rad_name = newnam;
  newrad->rad_size = itmsiz;
  pthread_mutex_init (&newrad->rad_mtx, NULL);
  newrad->rad_items = mom_gc_alloc (itmsiz * sizeof (struct mom_item_st *));
  newrad->rad_size = itmsiz;
  newrad->rad_count = 0;
  radix_arr_mom[ix] = newrad;
  makeradixcounter_mom++;
  MOM_DEBUGPRINTF (item,
                   "put_name_radix ix=%d newnam@%p '%s', newrad@%p makeradixcounter=%ld",
                   ix, newnam, newnam->itname_string.cstr, newrad,
                   makeradixcounter_mom);
  return (struct both_name_and_radix_mom_st)
  {
  newnam, newrad};
}                               /* end of put_name_radix_mom */




const struct mom_itemname_tu *
mom_make_name_radix (const char *str)
{
  int tix = -1;
  struct mom_itemname_tu *tun = NULL;
  if (!str || !str[0])
    return NULL;
  int len = (int) strlen (str);
  if (len >= 256)
    MOM_FATAPRINTF ("too big length %d for name radix %.*s", len, len, str);
  if (!mom_valid_name_radix (str))
    return NULL;
  pthread_mutex_lock (&radix_mtx_mom);
  MOM_DEBUGPRINTF (item, "mom_make_name_radix '%.*s' #%ld", len, str,
                   makeradixcounter_mom);
  assert (makeradixcounter_mom >= 0);
#ifndef NDEBUG
  if (MOM_IS_DEBUGGING (load) || MOM_IS_DEBUGGING (item))
    {
      for (int ix = 1; ix < (int) radix_cnt_mom; ix++)
        assert (strcmp (radix_arr_mom[ix - 1]->rad_name->itname_string.cstr,
                        radix_arr_mom[ix]->rad_name->itname_string.cstr) < 0);
    };
#endif /*NDEBUG*/
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
          const unsigned radsiz = 15;
          radix_arr_mom = mom_gc_alloc (radsiz * sizeof (void *));
          radix_arr_mom[0] = NULL;
          radix_cnt_mom = 1;
          radix_siz_mom = radsiz;
          struct both_name_and_radix_mom_st both =
            put_name_radix_mom (0, str);
          tun = both.btn_name;
          tix = 0;
          MOM_DEBUGPRINTF (item, "make_name_radix first hash %u '%s'",
                           both.btn_name->itname_string.hva_hash,
                           both.btn_name->itname_string.cstr);
          goto end;
        }
    };
#ifndef NDEBUG
  if (MOM_IS_DEBUGGING (load) || MOM_IS_DEBUGGING (item))
    {
      assert (radix_cnt_mom <= radix_siz_mom);
      for (int ix = 1; ix < (int) radix_cnt_mom; ix++)
        {

          assert (radix_arr_mom[ix] != NULL);
          assert (radix_arr_mom[ix]->rad_name != NULL);
          assert (radix_arr_mom[ix]->rad_name->itname_rank == (unsigned) ix);
          assert (strcmp (radix_arr_mom[ix - 1]->rad_name->itname_string.cstr,
                          radix_arr_mom[ix]->rad_name->itname_string.cstr) <
                  0);
        }
    }
#endif /*NDEBUG*/
  int lo = 0, hi = (int) radix_cnt_mom;
  MOM_DEBUGPRINTF (item,
                   "make_name_radix beforeloop lo=%d hi=%d str='%s'",
                   lo, hi, str);
  while (lo + 5 < hi)
    {
      int md = (lo + hi) / 2;
      assert (radix_arr_mom[md]);
      struct mom_itemname_tu *curad = radix_arr_mom[md]->rad_name;
      assert (curad != NULL);
      MOM_DEBUGPRINTF (item,
                       "make_name_radix loop lo=%d hi=%d md=%d curadname '%s' str '%.*s'",
                       lo, hi, md, curad->itname_string.cstr, len, str);
      assert (curad->itname_rank == (unsigned) md);
      int c = strcmp (str, curad->itname_string.cstr);
      if (c == 0)
        {
          tun = curad;
          tix = md;
          goto end;
        }
      else if (c <= 0)
        hi = md;
      else
        lo = md;
    };
  MOM_DEBUGPRINTF (item,
                   "make_name_radix loop lo=%d hi=%d radix_cnt=%d str '%s'",
                   lo, hi, radix_cnt_mom, str);
  for (int ix = lo; ix < (int) radix_cnt_mom; ix++)
    {
      assert (radix_arr_mom[ix]);
      struct mom_itemname_tu *curad = radix_arr_mom[ix]->rad_name;
      assert (curad != NULL);
      assert (curad->itname_rank == (unsigned) ix);
      int c = strcmp (str, curad->itname_string.cstr);
      MOM_DEBUGPRINTF (item,
                       "make_name_radix loop ix=%d curad='%s' str='%s' c=%d",
                       ix, curad->itname_string.cstr, str, c);
      if (c == 0)
        {
          tun = curad;
          tix = ix;
          MOM_DEBUGPRINTF (item,
                           "make_name_radix found ix=%d curadname '%s'", ix,
                           curad->itname_string.cstr);
          goto end;
        }
      else if (c < 0)
        {
          // we need to insert just before curad
          MOM_DEBUGPRINTF (item,
                           "make_name_radix should insert before ix=%d curadname '%s' str '%s'",
                           ix, curad->itname_string.cstr, str);
          for (int j = radix_cnt_mom; j > ix; j--)
            {
              radix_arr_mom[j] = radix_arr_mom[j - 1];
              radix_arr_mom[j]->rad_name->itname_rank = j;
            };
          radix_arr_mom[ix] = NULL;
          struct both_name_and_radix_mom_st both
            = put_name_radix_mom (ix, str);
          tun = both.btn_name;
          radix_cnt_mom++;
          goto end;
        }
      else if (ix + 1 == (int) radix_cnt_mom)
        {
          // we need to append after end
          MOM_DEBUGPRINTF (item,
                           "make_name_radix should append str='%s' cnt=%d",
                           str, (int) radix_cnt_mom);
          struct both_name_and_radix_mom_st both
            = put_name_radix_mom (radix_cnt_mom, str);
          tun = both.btn_name;
          radix_cnt_mom++;
          goto end;
        }
      else
        {
          MOM_DEBUGPRINTF (item, "make_name_radix continue ix=%d", ix);
          continue;
        }
    }                           /* end for (ix=...) */
end:
  MOM_DEBUGPRINTF (item,
                   "make_name_radix final radix_cnt=%d str='%.*s' tix=%d",
                   radix_cnt_mom, len, str, tix);
#ifndef NDEBUG
  if (MOM_IS_DEBUGGING (load) || MOM_IS_DEBUGGING (item))
    {
      for (int ix = 1; ix < (int) radix_cnt_mom; ix++)
        assert (strcmp (radix_arr_mom[ix - 1]->rad_name->itname_string.cstr,
                        radix_arr_mom[ix]->rad_name->itname_string.cstr) < 0);
    };
#endif /*NDEBUG*/
    ////
    if (MOM_IS_DEBUGGING (item))
    {
      for (int ix = 0; ix < (int) radix_cnt_mom; ix++)
        {
          MOM_DEBUGPRINTF (item, "make_name_radix [%d] @%p '%s' /%u", ix,
                           radix_arr_mom[ix],
                           radix_arr_mom[ix]->rad_name->itname_string.cstr,
                           radix_arr_mom[ix]->rad_name->
                           itname_string.hva_hash);
          if (ix > 0)
            assert (strcmp
                    (radix_arr_mom[ix - 1]->rad_name->itname_string.cstr,
                     radix_arr_mom[ix]->rad_name->itname_string.cstr) < 0);
          assert (radix_arr_mom[ix]->rad_name->itname_rank == (unsigned) ix);
        }
    }
  pthread_mutex_unlock (&radix_mtx_mom);
  MOM_DEBUGPRINTF (item,
                   "mom_make_name_radix done '%.*s' tix %d (makeradixcounter#%ld)",
                   len, str, tix, makeradixcounter_mom);
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
  MOM_DEBUGPRINTF (item, "find_item_from_string str=%s endradix+%d %s",
                   str, (int) (endradix - str), endradix);
  const char *end = NULL;
  uint16_t hid = 0;
  uint64_t lid = 0;
  const struct mom_itemname_tu *radix = NULL;
  radix = mom_find_name_radix_len (str, endradix - str);
  if (!radix)
    goto not_found;
  if (endradix && endradix[0] == '_' && endradix[1] == '_'
      && isdigit (endradix[2])
      && mom_suffix_to_hi_lo (endradix + 1, &hid, &lid) && hid > 0 && lid > 0)
    {
      end = endradix + 16;
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



const struct mom_boxset_st *
mom_set_items_prefixed (const char *str, int slen)
{
  const struct mom_boxset_st *res = NULL;
  if (!str || str == MOM_EMPTY_SLOT || !isalpha (str[0]))
    {
      MOM_DEBUGPRINTF (item, "mom_set_items_prefixed invalid str");
      return NULL;
    }
  if (slen < 0)
    slen = strlen (str);
  int postwo = -1;
  for (int ix = 0; ix < slen; ix++)
    {
      if (!isalnum (str[ix]) && str[ix] != '_')
        return NULL;
      if (str[ix] == '_' && postwo < 0 && ix > 0 && str[ix - 1] == '_')
        postwo = ix - 1;
    };
  MOM_DEBUGPRINTF (item, "mom_set_items_prefixed str=%.*s slen=%d postwo=%d",
                   slen, str, slen, postwo);
  if (postwo < 0)
    {                           // no __
      const struct mom_item_st **arr = NULL;
      const struct mom_item_st *smallarr[16] = { };
      int cnt = 0;
      memset (smallarr, 0, sizeof (smallarr));
      pthread_mutex_lock (&radix_mtx_mom);
      assert (radix_cnt_mom <= radix_siz_mom);
      if (MOM_UNLIKELY (radix_cnt_mom == 0))
        goto endradix;
      int lo = 0, hi = (int) radix_cnt_mom - 1;
      int nbloop = 0;
      for (;;)
        {
          MOM_DEBUGPRINTF (item,
                           "mom_set_items_prefixed str=%.*s lo=%d hi=%d nbloop#%d",
                           slen, str, lo, hi, nbloop);
          struct mom_itemname_tu *lorad = radix_arr_mom[lo]->rad_name;
          struct mom_itemname_tu *hirad = radix_arr_mom[hi]->rad_name;
          assert (lorad != NULL && lorad->itname_rank == (unsigned) lo);
          assert (hirad != NULL && hirad->itname_rank == (unsigned) hi);
          MOM_DEBUGPRINTF (item, "mom_set_items_prefixed loname %s hiname %s",
                           lorad->itname_string.cstr,
                           hirad->itname_string.cstr);
          if (!strncmp (lorad->itname_string.cstr, str, slen)
              && !strncmp (hirad->itname_string.cstr, str, slen))
            break;
          if (lo + 5 >= hi)
            break;
          nbloop++;
          assert (nbloop < 100);
          int md = (lo + hi) / 2;
          struct mom_itemname_tu *mdrad = radix_arr_mom[md]->rad_name;
          assert (mdrad != NULL);
          assert (mdrad->itname_rank == (unsigned) md);
          int c = strncmp (mdrad->itname_string.cstr, str, slen);
          MOM_DEBUGPRINTF (item,
                           "mom_set_items_prefixed str=%.*s md=%d mdname '%s' c=%d",
                           slen, str, md, mdrad->itname_string.cstr, c);
          if (c < 0)
            lo = md;
          else if (c > 0)
            hi = md;
          else if (c == 0)
            break;
        };
      int siz = hi - lo;
      MOM_DEBUGPRINTF (item,
                       "mom_set_items_prefixed final lo=%d hi=%d siz=%d", lo,
                       hi, siz);
      assert (siz >= 0 && siz <= (int) radix_cnt_mom);
      arr =                     //
        (siz < (int) (sizeof (smallarr) / sizeof (smallarr[0]))) ? smallarr
        : mom_gc_alloc (siz * sizeof (struct mom_item_st *));
      cnt = 0;
      for (int ix = lo; ix <= hi; ix++)
        {
          struct mom_itemname_tu *curad = radix_arr_mom[ix]->rad_name;
          assert (curad != NULL && curad->itname_rank == (unsigned) ix);
          MOM_DEBUGPRINTF (item,
                           "mom_set_items_prefixed str=%.*s ix=%d curadname %s",
                           slen, str, ix, curad->itname_string.cstr);
          if (!strncmp (curad->itname_string.cstr, str, slen))
            {
              const struct mom_item_st *curitm =
                radix_arr_mom[ix]->rad_nakeditem;
              MOM_DEBUGPRINTF (item,
                               "mom_set_items_prefixed ix=%d naked curitm %s",
                               ix, mom_item_cstring (curitm));
              if (curitm == NULL || curitm == MOM_EMPTY_SLOT)
                continue;
              assert (cnt < siz);
              MOM_DEBUGPRINTF (item, "mom_set_items_prefixed arr[%d] = %s",
                               cnt, mom_item_cstring (curitm));
              arr[cnt++] = curitm;
            }
        }
    endradix:
      pthread_mutex_unlock (&radix_mtx_mom);
      res = mom_boxset_make_arr (cnt, arr);
    }
  else
    {                           // we have __ at position postwo 
      const struct mom_itemname_tu *radix =     //
        mom_find_name_radix_len (str, postwo);
      if (radix)
        {
          struct mom_hashset_st *hset = NULL;
          struct radix_mom_st *curad = NULL;
          {
            pthread_mutex_lock (&radix_mtx_mom);
            curad = radix_arr_mom[radix->itname_rank];
            pthread_mutex_unlock (&radix_mtx_mom);
          }
          {
            char buf[MAXLEN_SUFFIXEDITEM_MOM];
            memset (buf, 0, sizeof (buf));
            pthread_mutex_lock (&curad->rad_mtx);
            hset =
              mom_hashset_reserve (NULL,
                                   9 + (curad->rad_count >> (slen - postwo)));
            unsigned rsiz = curad->rad_size;
            for (unsigned ix = 0; ix < rsiz; ix++)
              {
                struct mom_item_st *curitm = curad->rad_items[ix];
                if (!curitm || curitm == MOM_EMPTY_SLOT)
                  continue;
                char bufnum[MOM_HI_LO_SUFFIX_LEN];
                memset (bufnum, 0, sizeof (bufnum));
                memset (buf, 0, sizeof (buf));
                if (MOM_UNLIKELY (snprintf (buf, sizeof (buf), "%s_%s",
                                            radix->itname_string.cstr,
                                            mom_item_hi_lo_suffix (bufnum,
                                                                   curitm)) >=
                                  (int) sizeof (buf)))
                  MOM_FATAPRINTF ("too long item name %s", buf);
                if (!strncmp (buf, str, slen))
                  hset = mom_hashset_insert (hset, curitm);
              }
            pthread_mutex_unlock (&curad->rad_mtx);
          }
          res = mom_hashset_to_boxset (hset);
        }
    }
  MOM_DEBUGPRINTF (item, "mom_set_items_prefixed %.*s = %s",
                   slen, str,
                   mom_value_cstring ((const struct mom_hashedvalue_st *)
                                      res));
  return res;
}                               /* end of mom_set_items_prefixed */


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
  radix = mom_make_name_radix_len (str, endradix - str);
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
index_item_in_radix_mom (struct radix_mom_st *curad, struct mom_item_st *itm)
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
        {
          pos = ix;
          goto end;
        };
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
      // sometimes, itm is a pseudoitemzon from mom_make_item_from_radix_id
      // so we need to test equality of hash, radix, hid, lid
      else if (curitm->hva_hash == itm->hva_hash
               && curitm->itm_lid == itm->itm_lid
               && curitm->itm_radix == itm->itm_radix
               && curitm->itm_hid == itm->itm_hid)
        {
          pos = ix;
          goto end;
        };
    }
  for (unsigned ix = 0; ix < startix; ix++)
    {
      struct mom_item_st *curitm = curad->rad_items[ix];
      if (curitm == itm)
        {
          pos = ix;
          goto end;
        }
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
      // sometimes, itm is a pseudoitemzon from mom_make_item_from_radix_id
      // so we need to test equality of hash, radix, hid, lid
      else if (curitm->hva_hash == itm->hva_hash
               && curitm->itm_lid == itm->itm_lid
               && curitm->itm_radix == itm->itm_radix
               && curitm->itm_hid == itm->itm_hid)
        {
          pos = ix;
          goto end;
        };
    }
end:
  assert (pos >= 0 && pos < (int) sz);
  MOM_DEBUGPRINTF (item,
                   "index_item_in_radix curad@%p rk#%d str=%s itm=%s pos=%d",
                   (void *) curad, curad->rad_name->itname_rank,
                   curad->rad_name->itname_string.cstr,
                   mom_item_cstring (itm), pos);
  return pos;
}                               /* end of index_item_in_radix_mom */



void
cleanup_item_payload_mom (struct mom_item_st *itm,
                          struct mom_anyvalue_st *payl)
{
  assert (itm != NULL && itm->va_itype == MOMITY_ITEM);
  switch (payl->va_itype)
    {
    case MOMITY_WEBEXCH:
      mom_webexch_payload_cleanup (itm, (struct mom_webexch_st *) payl);
      break;
    case MOMITY_WEBSESSION:
      mom_websession_payload_cleanup (itm, (struct mom_websession_st *) payl);
      break;
    }
}                               /* end of cleanup_item_payload_mom */


void
mom_cleanup_item (void *itmad, void *clad)
{
  assert (itmad != NULL);
  assert (clad == NULL);
  struct mom_item_st *itm = itmad;
  struct radix_mom_st *curad = NULL;
  MOM_DEBUGPRINTF (item, "mom_cleanup_item itm=%s", mom_item_cstring (itm));
  struct mom_anyvalue_st *payl = itm->itm_payload;
  if (payl && payl != MOM_EMPTY_SLOT)
    {
      itm->itm_payload = NULL;
      cleanup_item_payload_mom (itm, payl);
    }
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
  int pos = index_item_in_radix_mom (curad, itm);
  assert (pos >= 0 && pos < (int) curad->rad_size);
  assert (curad->rad_items);
  assert (curad->rad_items[pos] == itm);
  if (itm->itm_hid == 0 && itm->itm_lid == 0)
    {
      assert (curad->rad_nakeditem == itm);
      curad->rad_nakeditem = NULL;
    };
  curad->rad_items[pos] = MOM_EMPTY_SLOT;
  assert (curad->rad_count > 0 && curad->rad_count <= curad->rad_size);
  curad->rad_count--;
  pthread_mutex_destroy (&itm->itm_mtx);
  memset (itmad, 0, sizeof (struct mom_item_st));
  pthread_mutex_unlock (&curad->rad_mtx);
}                               /* end of mom_cleanup_item */




struct mom_item_st *
mom_make_item_from_radix_id (const struct mom_itemname_tu *radix,
                             uint16_t hid, uint64_t loid)
{
  struct mom_item_st *itm = NULL;
  if (!radix)
    return NULL;
  MOM_DEBUGPRINTF (item, "make_item_from_radix %s start hid %d loid %lld",
                   radix->itname_string.cstr, hid, (long long) loid);
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
              int pos = index_item_in_radix_mom (curad, olditm);
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
    pseudoitemzon.va_itype = MOMITY_ITEM;
    pseudoitemzon.hva_hash = hi;
    pseudoitemzon.itm_radix = (struct mom_itemname_tu *) radix;
    pseudoitemzon.itm_hid = hid;
    pseudoitemzon.itm_lid = loid;
    int pos = index_item_in_radix_mom (curad, &pseudoitemzon);
    assert (pos >= 0 && pos < (int) sz);
    if (curad->rad_items[pos] != NULL
        && curad->rad_items[pos] != MOM_EMPTY_SLOT)
      {
        itm = curad->rad_items[pos];
        MOM_DEBUGPRINTF (item,
                         "make_item_from_radix %s hid %d loid %lld pos %d found existing itm@%p %s",
                         radix->itname_string.cstr, hid, (long long) loid,
                         pos, (void *) itm, mom_item_cstring (itm));
        goto end;
      }
    struct mom_item_st *newitm = mom_gc_alloc (sizeof (struct mom_item_st));
    newitm->va_itype = MOMITY_ITEM;
    newitm->hva_hash = hi;
    newitm->itm_radix = (struct mom_itemname_tu *) radix;
    curad->rad_items[pos] = newitm;
    curad->rad_count++;
    time (&newitm->itm_mtime);
    pthread_mutex_init (&newitm->itm_mtx, &item_mtxattr_mom);
    newitm->itm_hid = hid;
    newitm->itm_lid = loid;
    if (!hid && !loid)
      {
        curad->rad_nakeditem = newitm;
        MOM_DEBUGPRINTF (item,
                         "make_item_from_radix %s nakeditm %s @%p",
                         radix->itname_string.cstr, mom_item_cstring (newitm),
                         newitm);
      }
    GC_REGISTER_FINALIZER_IGNORE_SELF (newitm, mom_cleanup_item, NULL, NULL,
                                       NULL);
    itm = newitm;
    MOM_DEBUGPRINTF (item,
                     "make_item_from_radix %s hid %d loid %lld new itm@%p %s",
                     radix->itname_string.cstr, hid, (long long) loid,
                     (void *) itm, mom_item_cstring (itm));
    goto end;
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
              int pos = index_item_in_radix_mom (curad, olditm);
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
        pos = index_item_in_radix_mom (curad, quasitm);
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
      char buf[MAXLEN_SUFFIXEDITEM_MOM];
      char bufnum[MOM_HI_LO_SUFFIX_LEN];
      memset (buf, 0, sizeof (buf));
      memset (bufnum, 0, sizeof (bufnum));
      if (MOM_UNLIKELY (snprintf (buf, sizeof (buf), "%s_%s",
                                  mom_item_radix_str (itm),
                                  mom_item_hi_lo_suffix (bufnum, itm))
                        >= (int) sizeof (buf)))
        MOM_FATAPRINTF ("too long item name %s", buf);
      return GC_STRDUP (buf);
    }
}                               /* end mom_item_cstring */



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


const struct mom_boxset_st *
mom_assovaldata_set_attrs (const struct mom_assovaldata_st *asso)
{
  if (!asso || asso == MOM_EMPTY_SLOT || asso->va_itype != MOMITY_ASSOVALDATA)
    return NULL;
  unsigned siz = mom_raw_size (asso);
  unsigned cnt = asso->cda_count;
  assert (cnt <= siz);
  const struct mom_item_st *smallarr[16] = { NULL };
  const struct mom_item_st **arr =
    (cnt < sizeof (smallarr) / sizeof (smallarr[0])) ? smallarr
    : mom_gc_alloc ((cnt + 1) * sizeof (void *));
  unsigned icnt = 0;
  for (unsigned ix = 0; ix < siz; ix++)
    {
      const struct mom_item_st *curitm = asso->ada_ents[ix].ient_itm;
      if (!curitm || curitm == MOM_EMPTY_SLOT)
        continue;
      assert (icnt < cnt);
      arr[icnt++] = curitm;
    }
  assert (icnt == cnt);
  return mom_boxset_make_arr (icnt, arr);
}                               /* end of mom_assovaldata_set_attrs */



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
          for (int j = (int)cnt; j > ix; j--)
            asso->ada_ents[j] = asso->ada_ents[j - 1];
          asso->ada_ents[ix].ient_itm = (struct mom_item_st *) itmat;
          asso->ada_ents[ix].ient_val = (struct mom_hashedvalue_st *) data;
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
  if (!asso || asso == MOM_EMPTY_SLOT || asso->va_itype != MOMITY_ASSOVALDATA)
    return;
  assert (du && du->va_itype == MOMITY_DUMPER);
  unsigned cnt = asso->cda_count;
  unsigned siz = mom_raw_size (asso);
  MOM_DEBUGPRINTF (dump, "dumpscan_assovaldata asso@%p cnt%d siz%d", asso,
                   cnt, siz);
  assert (cnt <= siz);
  for (int ix = 0; ix < (int) cnt; ix++)
    {
      if (!mom_dumpable_item (asso->ada_ents[ix].ient_itm))
        continue;
      mom_dumpscan_item (du, asso->ada_ents[ix].ient_itm);
      mom_dumpscan_value (du, asso->ada_ents[ix].ient_val);
    }
  MOM_DEBUGPRINTF (dump, "dumpscan_assovaldata asso@%p done", asso);
}


void
mom_unsync_item_put_phys_attr (struct mom_item_st *itm,
                               const struct mom_item_st *itmat,
                               const void *data)
{
  if (!itm || itm == MOM_EMPTY_SLOT || itm->va_itype != MOMITY_ITEM)
    return;
  if (!itmat || itmat == MOM_EMPTY_SLOT || itmat->va_itype != MOMITY_ITEM)
    return;
  struct mom_assovaldata_st *attrs =
    (struct mom_assovaldata_st *) mom_assovaldata_dyncast (itm->itm_pattr);
  attrs = mom_assovaldata_put (attrs, itmat, data);
  itm->itm_pattr = attrs;
}                               /* end of mom_unsync_item_put_phys_attr */


// remove a physical attribute from an item
void
mom_unsync_item_remove_phys_attr (struct mom_item_st *itm,
                                  const struct mom_item_st *itmat)
{
  if (!itm || itm == MOM_EMPTY_SLOT || itm->va_itype != MOMITY_ITEM)
    return;
  if (!itmat || itmat == MOM_EMPTY_SLOT || itmat->va_itype != MOMITY_ITEM)
    return;
  struct mom_assovaldata_st *attrs =
    (struct mom_assovaldata_st *) mom_assovaldata_dyncast (itm->itm_pattr);
  attrs = mom_assovaldata_remove (attrs, itmat);
  itm->itm_pattr = attrs;
}                               /* end of mom_unsync_item_remove_phys_attr */




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
  MOM_DEBUGPRINTF (item, "initialize_predefined start %s itm@%p", name,
                   (void *) itm);
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
  const struct mom_itemname_tu *radix = mom_make_name_radix (name);
  if (!radix)
    MOM_FATAPRINTF ("initialize_predefined failed to make radix %s", name);
  struct radix_mom_st *curad = NULL;
  unsigned radrk = 0;
  {
    pthread_mutex_lock (&radix_mtx_mom);
    assert (radix_cnt_mom <= radix_siz_mom);
    radrk = radix->itname_rank;
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
              int pos = index_item_in_radix_mom (curad, olditm);
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
  time (&itm->itm_mtime);
  int pos = index_item_in_radix_mom (curad, itm);
  MOM_DEBUGPRINTF (item, "predefined itm@%p '%s' pos %d",
                   (void *) itm, mom_item_cstring (itm), pos);
  assert (pos >= 0 && pos < (int) sz);
  assert (curad->rad_items[pos] == NULL);
  if (!twou)
    curad->rad_nakeditem = itm;
  curad->rad_items[pos] = itm;
  curad->rad_count++;
  pthread_mutex_unlock (&curad->rad_mtx);
  mom_item_put_space (itm, MOMSPA_PREDEF);
}                               /* end initialize_predefined_mom  */

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
  if (MOM_IS_DEBUGGING (item))
    {
      MOM_DEBUGPRINTF (item, "showing %d predefined items",
                       MOM_NB_PREDEFINED);
      mom_debugprint_radixtable ();
      for (int ix = 1; ix < (int) radix_cnt_mom; ix++)
        {
          assert (strcmp (radix_arr_mom[ix - 1]->rad_name->itname_string.cstr,
                          radix_arr_mom[ix]->rad_name->itname_string.cstr) <
                  0);
        }
      MOM_DEBUGPRINTF (item, "well sorted %d predefined (radix_cnt_mom=%d)",
                       MOM_NB_PREDEFINED, radix_cnt_mom);
    }
  else if (MOM_IS_DEBUGGING (load))
    {
      MOM_DEBUGPRINTF (load, "showing %d predefined before loading",
                       MOM_NB_PREDEFINED);
      mom_debugprint_radixtable ();
    }
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
  MOM_DEBUGPRINTF (dump, "dumpemit_item_content start itm %s @%p",
                   mom_item_cstring (itm), (void *) itm);
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
          MOM_DEBUGPRINTF (dump, "dumpemit_item_content dli_sname %s",
                           dinf.dli_sname);
        }
    }
  // emit the signature
  if (itm->itm_funsig && itm->itm_funsig != MOM_EMPTY_SLOT
      && mom_dumped_item (du, itm->itm_funsig))
    {
      MOM_DEBUGPRINTF (dump, "dumpemit_item_content funsig %s",
                       mom_item_cstring (itm->itm_funsig));
      mom_dumpemit_refitem (du, itm->itm_funsig);
      fputs ("^funsignature\n", femit);
    }
  /// emit the attributes
  if (itm->itm_pattr && itm->itm_pattr != MOM_EMPTY_SLOT)
    {
      const struct mom_assovaldata_st *asso = itm->itm_pattr;
      assert (asso->va_itype == MOMITY_ASSOVALDATA);
      MOM_DEBUGPRINTF (dump, "dumpemit_item_content asso count %d",
                       asso->cda_count);
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
      assert (comps->va_itype == MOMITY_VECTVALDATA);
      unsigned siz = mom_raw_size (comps);
      unsigned cnt = comps->cda_count;
      MOM_DEBUGPRINTF (dump, "dumpemit_item_content comps count %d", cnt);
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
      MOM_DEBUGPRINTF (dump, "dumpemit_item_content itm %s payloadtype %s",
                       mom_item_cstring (itm), mom_itype_str (payl));
      switch (payl->va_itype)
        {
        case MOMITY_BOXINT:
        case MOMITY_BOXDOUBLE:
        case MOMITY_BOXSTRING:
        case MOMITY_TUPLE:
        case MOMITY_SET:
        case MOMITY_NODE:
        case MOMITY_ITEM:
          MOM_DEBUGPRINTF (dump, "dumpemit_item_content value payload %s",
                           mom_value_cstring ((struct mom_hashedvalue_st *)
                                              payl));
          mom_dumpemit_value (du, (struct mom_hashedvalue_st *) payl);
          fputs ("^payload_val\n", femit);
          break;
        case MOMITY_ASSOVALDATA:
          fputs ("(\n", femit);
          MOM_DEBUGPRINTF (dump, "dumpemit_item_content itm %s assovaldata",
                           mom_item_cstring (itm));
          dumpemit_assovaldata_mom (du, (struct mom_assovaldata_st *) payl);
          fputs (")payload_assoval\n", femit);
          break;
        case MOMITY_VECTVALDATA:
          {
            MOM_DEBUGPRINTF (dump, "dumpemit_item_content itm %s vectvaldata",
                             mom_item_cstring (itm));
            struct mom_vectvaldata_st *comps =
              (struct mom_vectvaldata_st *) payl;
            unsigned siz = mom_raw_size (comps);
            unsigned cnt = comps->cda_count;
            assert (cnt <= siz);
            fputs ("(\n", femit);
            for (unsigned ix = 0; ix < cnt; ix++)
              mom_dumpemit_value (du, comps->vecd_valarr[ix]);
            fputs (")payload_vect\n", femit);
          }
          break;
        case MOMITY_QUEUE:
          {
            MOM_DEBUGPRINTF (dump, "dumpemit_item_content itm %s queue",
                             mom_item_cstring (itm));
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
            fputs (")payload_queue\n", femit);
          }
          break;
        case MOMITY_HASHSET:
          {
            struct mom_hashset_st *hset = (struct mom_hashset_st *) payl;
            const struct mom_boxset_st *bxset = mom_hashset_to_boxset (hset);
            MOM_DEBUGPRINTF (dump,
                             "dumpemit_item_content itm %s hashset bxset %s",
                             mom_item_cstring (itm),
                             mom_value_cstring ((struct mom_hashedvalue_st *)
                                                bxset));
            unsigned siz = mom_size (bxset);
            fputs ("(\n", femit);
            for (unsigned ix = 0; ix < siz; ix++)
              {
                struct mom_item_st *elemitm = bxset->seqitem[ix];
                if (!mom_dumped_item (du, elemitm))
                  continue;
                mom_dumpemit_refitem (du, elemitm);
              }
            fputs (")payload_hashset\n", femit);
          }
          break;
        case MOMITY_HASHMAP:
          MOM_DEBUGPRINTF (dump, "dumpemit_item_content itm %s hashmap",
                           mom_item_cstring (itm));
          mom_dumpemit_hashmap_payload (du, (struct mom_hashmap_st *) payl);
          break;
        case MOMITY_HASHASSOC:
          MOM_DEBUGPRINTF (dump, "dumpemit_item_content itm %s hashassoc",
                           mom_item_cstring (itm));
          mom_dumpemit_hashassoc_payload (du,
                                          (struct mom_hashassoc_st *) payl);
          break;
        case MOMITY_FILEBUFFER:
          MOM_DEBUGPRINTF (dump, "dumpemit_item_content itm %s filebuffer",
                           mom_item_cstring (itm));
          mom_dumpemit_filebuffer_payload (du,
                                           (struct mom_filebuffer_st *) payl);
          break;
        default:
          MOM_DEBUGPRINTF (dump,
                           "dumpemit_item_content itm %s other payload %s @%p",
                           mom_item_cstring (itm),
                           mom_itype_str (itm->itm_payload),
                           itm->itm_payload);
          break;
        }
    }
  else
    MOM_DEBUGPRINTF (dump, "dumpemit_item_content itm %s without payload",
                     mom_item_cstring (itm));
  MOM_DEBUGPRINTF (dump, "dumpemit_item_content done itm %s @%p",
                   mom_item_cstring (itm), (void *) itm);
}                               /* end of mom_dumpemit_item_content */

////////////////

extern mom_loader_caret_sig_t momf_ldc_mtime;
extern mom_loader_caret_sig_t momf_ldc_func;
extern mom_loader_caret_sig_t momf_ldc_funsignature;
extern mom_loader_caret_sig_t momf_ldc_altfunc;
extern mom_loader_caret_sig_t momf_ldc_payload_val;

const char momsig_ldc_mtime[] = "signature_loader_caret";
void
momf_ldc_mtime (struct mom_item_st *itm, struct mom_loader_st *ld)
{
  assert (itm && itm->va_itype == MOMITY_ITEM);
  assert (ld && ld->va_itype == MOMITY_LOADER);
  time_t t = mom_ldstate_int_def (mom_loader_top (ld, 0), 0);
  MOM_DEBUGPRINTF (load, "momf_ldc_mtime  itm=%s t=%ld",
                   mom_item_cstring (itm), (long) t);
  if (t)
    itm->itm_mtime = t;
  mom_loader_pop (ld, 1);
}                               /* end of momf_ldc_mtime */


const char momsig_ldc_func[] = "signature_loader_caret";
void
momf_ldc_func (struct mom_item_st *itm, struct mom_loader_st *ld)
{
  assert (itm && itm->va_itype == MOMITY_ITEM);
  assert (ld && ld->va_itype == MOMITY_LOADER);
  MOM_DEBUGPRINTF (load, "momf_ldc_func itm=%s", mom_item_cstring (itm));
  char funambuf[256];
  memset (funambuf, 0, sizeof (funambuf));
  if (snprintf
      (funambuf, sizeof (funambuf), MOM_FUNC_PREFIX "%s",
       mom_item_cstring (itm)) < (int) sizeof (funambuf))
    {
      void *f = dlsym (mom_prog_dlhandle, funambuf);
      if (f)
        {
          itm->itm_funptr = f;
          MOM_DEBUGPRINTF (load, "momf_ldc_func itm=%s f@%p",
                           mom_item_cstring (itm), f);
        }
      else
        MOM_WARNPRINTF ("cannot find func %s : %s", funambuf, dlerror ());
    }
  else
    MOM_FATAPRINTF ("too long function name %s", funambuf);
  memset (funambuf, 0, sizeof (funambuf));
  if (snprintf
      (funambuf, sizeof (funambuf), MOM_SIGNATURE_PREFIX "%s",
       mom_item_cstring (itm)) < (int) sizeof (funambuf))
    {
      const char *signame = dlsym (mom_prog_dlhandle, funambuf);
      if (signame && isalpha (signame[0]))
        {
          MOM_DEBUGPRINTF (load, "momf_ldc_func itm=%s signame@%p='%s'",
                           mom_item_cstring (itm), (void *) signame, signame);
          const char *endsig = NULL;
          struct mom_item_st *sigitm =
            mom_find_item_from_string (signame, &endsig);
          if (sigitm && *endsig == (char) 0)
            {
              MOM_DEBUGPRINTF (load, "momf_ldc_func itm=%s sigitm=%s",
                               mom_item_cstring (itm),
                               mom_item_cstring (sigitm));
              if (!itm->itm_funsig)
                itm->itm_funsig = sigitm;
            }
        }
    }
  else
    MOM_FATAPRINTF ("too long signature name %s", funambuf);
}                               /* end of momf_ldc_func */


const char momsig_ldc_funsignature[] = "signature_loader_caret";
void
momf_ldc_funsignature (struct mom_item_st *itm, struct mom_loader_st *ld)
{
  assert (itm && itm->va_itype == MOMITY_ITEM);
  assert (ld && ld->va_itype == MOMITY_LOADER);
  struct mom_item_st *sigitm = mom_ldstate_dynitem (mom_loader_top (ld, 0));
  MOM_DEBUGPRINTF (load, "momf_ldc_funsignature itm=%s sigitm=%s",
                   mom_item_cstring (itm), mom_item_cstring (sigitm));
  if (sigitm)
    itm->itm_funsig = sigitm;
  mom_loader_pop (ld, 1);
}


extern mom_loader_paren_sig_t momf_ldp_attrs;
const char momsig_ldc_attrs[] = "signature_loader_paren";
void
momf_ldp_attrs (struct mom_item_st *itm,
                struct mom_loader_st *ld,
                struct mom_statelem_st *elemarr, unsigned elemsize)
{
  assert (ld != NULL && ld->va_itype == MOMITY_LOADER);
  MOM_DEBUGPRINTF (load, "momf_ldp_attrs itm=%s", mom_item_cstring (itm));
  if (elemsize % 2 != 0)
    MOM_FATAPRINTF ("momf_ldp_attrs itm=%s odd elemsize %d",
                    mom_item_cstring (itm), elemsize);
  itm->itm_pattr =
    mom_assovaldata_reserve (itm->itm_pattr, 4 * elemsize / 3 + 2);
  for (unsigned ix = 0; ix < elemsize; ix += 2)
    {
      struct mom_item_st *attitm = mom_ldstate_dynitem (elemarr[ix]);
      const struct mom_hashedvalue_st *attval =
        mom_ldstate_val (elemarr[ix + 1]);
      MOM_DEBUGPRINTF (load, "momf_ldp_attrs ix=%d attitm=%s attval=%s",
                       ix, mom_item_cstring (attitm),
                       mom_value_cstring (attval));
      itm->itm_pattr = mom_assovaldata_put (itm->itm_pattr, attitm, attval);
    }
}                               /* end of momf_ldp_attrs */


extern mom_loader_paren_sig_t momf_ldp_comps;
const char momsig_ldp_comps[] = "signature_loader_paren";
void
momf_ldp_comps (struct mom_item_st *itm,
                struct mom_loader_st *ld,
                struct mom_statelem_st *elemarr, unsigned elemsize)
{
  assert (ld != NULL && ld->va_itype == MOMITY_LOADER);
  MOM_DEBUGPRINTF (load, "momf_ldp_comps itm=%s elemsize=%d",
                   mom_item_cstring (itm), elemsize);
  itm->itm_pcomp = mom_vectvaldata_reserve (itm->itm_pcomp, elemsize);
  itm->itm_pcomp = mom_vectvaldata_resize (itm->itm_pcomp, elemsize);
  for (unsigned ix = 0; ix < elemsize; ix++)
    {
      const struct mom_hashedvalue_st *val = mom_ldstate_val (elemarr[ix]);
      MOM_DEBUGPRINTF (load, "momf_ldp_comps ix#%d val=%s", ix,
                       mom_value_cstring (val));
      mom_vectvaldata_put_nth (itm->itm_pcomp, (int) ix, val);
    }
}                               /* end of momf_ldp_comps */


////////////////
extern mom_loader_paren_sig_t momf_ldp_payload_assoval;
const char momsig_ldp_payload_assoval[] = "signature_loader_paren";
void
momf_ldp_payload_assoval (struct mom_item_st *itm,
                          struct mom_loader_st *ld,
                          struct mom_statelem_st *elemarr, unsigned elemsiz)
{
  assert (ld != NULL && ld->va_itype == MOMITY_LOADER);
  MOM_DEBUGPRINTF (load, "momf_ldp_payload_assoval itm=%s elemsiz=%d",
                   mom_item_cstring (itm), elemsiz);
  if (elemsiz % 2)
    MOM_FATAPRINTF ("momf_ldp_payload_assoval with odd size %d", elemsiz);
  struct mom_assovaldata_st *asso =
    mom_assovaldata_reserve (NULL, elemsiz / 2 + elemsiz / 16 + 1);
  for (unsigned ix = 0; ix < elemsiz; ix += 2)
    {
      struct mom_item_st *attitm = mom_ldstate_dynitem (elemarr[ix]);
      const struct mom_hashedvalue_st *attval =
        mom_ldstate_val (elemarr[ix + 1]);
      MOM_DEBUGPRINTF (load,
                       "momf_ldp_payload_assoval itm=%s ix#%d attitm=%s attval=%s",
                       mom_item_cstring (itm), ix, mom_item_cstring (attitm),
                       mom_value_cstring (attval));
      asso = mom_assovaldata_put (asso, attitm, attval);
    }
  itm->itm_payload = (struct mom_anyvalue_st *) asso;
}                               /* end of momf_ldp_payload_assoval */

////////////////
extern mom_loader_paren_sig_t momf_ldp_payload_vect;
const char momsig_ldp_payload_vect[] = "signature_loader_paren";
void
momf_ldp_payload_vect (struct mom_item_st *itm,
                       struct mom_loader_st *ld,
                       struct mom_statelem_st *elemarr, unsigned elemsize)
{
  assert (ld != NULL && ld->va_itype == MOMITY_LOADER);
  MOM_DEBUGPRINTF (load, "momf_ldp_payload_vect itm=%s elemsize=%d",
                   mom_item_cstring (itm), elemsize);
  MOM_FATAPRINTF ("unimplemented momf_ldp_payload_vect itm=%s",
                  mom_item_cstring (itm));
#warning unimplemented momf_ldp_payload_vect
}                               /* end of momf_ldp_payload_vect */



////////////////
extern mom_loader_paren_sig_t momf_ldp_payload_queue;
const char momsig_ldp_payload_queue[] = "signature_loader_paren";
void
momf_ldp_payload_queue (struct mom_item_st *itm,
                        struct mom_loader_st *ld,
                        struct mom_statelem_st *elemarr, unsigned elemsize)
{
  assert (ld != NULL && ld->va_itype == MOMITY_LOADER);
  MOM_DEBUGPRINTF (load, "momf_ldp_payload_queue itm=%s elemsize=%d",
                   mom_item_cstring (itm), elemsize);
  struct mom_queue_st *qu = mom_queue_make ();
  for (unsigned ix = 0; ix < elemsize; ix++)
    {
      const struct mom_hashedvalue_st *val = mom_ldstate_val (elemarr[ix]);
      MOM_DEBUGPRINTF (load, "momf_ldp_payload_queue [%d] %s",
                       ix, mom_value_cstring (val));
      mom_queue_append (qu, val);
    }
  itm->itm_payload = (struct mom_anyvalue_st *) qu;
}                               /* end of momf_ldp_payload_queue */


////////////////
extern mom_loader_paren_sig_t momf_ldp_payload_hashset;
const char momsig_ldp_payload_hashset[] = "signature_loader_paren";
void
momf_ldp_payload_hashset (struct mom_item_st *itm,
                          struct mom_loader_st *ld,
                          struct mom_statelem_st *elemarr, unsigned elemsize)
{
  assert (ld != NULL && ld->va_itype == MOMITY_LOADER);
  MOM_DEBUGPRINTF (load, "momf_ldp_payload_hashset itm=%s elemsize=%d",
                   mom_item_cstring (itm), elemsize);
  MOM_FATAPRINTF ("unimplemented momf_ldp_payload_hashset itm=%s",
                  mom_item_cstring (itm));
#warning unimplemented momf_ldp_payload_hashset
}                               /* end of momf_ldp_payload_hashset */



////////////////
extern mom_loader_paren_sig_t momf_ldp_payload_hashmap;
const char momsig_ldp_payload_hashmap[] = "signature_loader_paren";
void
momf_ldp_payload_hashmap (struct mom_item_st *itm,
                          struct mom_loader_st *ld,
                          struct mom_statelem_st *elemarr, unsigned elemsiz)
{
  assert (ld != NULL && ld->va_itype == MOMITY_LOADER);
  MOM_DEBUGPRINTF (load, "momf_ldp_payload_hashmap itm=%s elemsiz=%d",
                   mom_item_cstring (itm), elemsiz);
  if (elemsiz % 2)
    MOM_FATAPRINTF ("momf_ldp_payload_hashmap with odd size %d", elemsiz);
  struct mom_hashmap_st *hmap =
    mom_hashmap_reserve (NULL, elemsiz / 2 + elemsiz / 8 + elemsiz / 64 + 1);
  for (unsigned ix = 0; ix < elemsiz; ix += 2)
    {
      struct mom_item_st *attitm = mom_ldstate_dynitem (elemarr[ix]);
      const struct mom_hashedvalue_st *attval =
        mom_ldstate_val (elemarr[ix + 1]);
      MOM_DEBUGPRINTF (load,
                       "momf_ldp_payload_hashmap itm=%s ix#%d attitm=%s attval=%s",
                       mom_item_cstring (itm), ix, mom_item_cstring (attitm),
                       mom_value_cstring (attval));
      hmap = mom_hashmap_put (hmap, attitm, attval);
    }
  itm->itm_payload = (struct mom_anyvalue_st *) hmap;
}                               /* end of momf_ldp_payload_hashmap */



bool
mom_unsync_item_clear_payload (struct mom_item_st *itm)
{
  if (!itm || itm == MOM_EMPTY_SLOT || itm->va_itype != MOMITY_ITEM)
    return false;
  if (!itm->itm_payload)
    return true;
  unsigned typl = mom_itype (itm->itm_payload);
  switch (typl)
    {
    case MOMITY_WEBEXCH:
      /// don't even clear, return immediately
      MOM_WARNPRINTF ("avoid clearing webexch payload in item %s",
                      mom_item_cstring (itm));
      return false;
    case MOMITY_WEBSESSION:
      MOM_WARNPRINTF ("avoid clearing websession payload in item %s",
                      mom_item_cstring (itm));
      return false;
    case MOMITY_DUMPER:
      MOM_WARNPRINTF ("avoid clearing dumper payload in item %s",
                      mom_item_cstring (itm));
      return false;
    case MOMITY_LOADER:
      MOM_WARNPRINTF ("avoid clearing loader payload in item %s",
                      mom_item_cstring (itm));
      return false;
    case MOMITY_FILE:
    case MOMITY_FILEBUFFER:
      mom_file_close (itm->itm_payload);
    default:
      break;
    }
  itm->itm_payload = NULL;
  return true;
}                               /* end of mom_unsync_item_clear_payload */

/// eof item.c
