// file value.c - managing values

/**   Copyright (C)  2015 - 2016  Basile Starynkevitch and later the FSF
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

void *
mom_gc_alloc_at (size_t sz, const char *fil, int lin)
{
  void *ptr = NULL;
#ifdef GC_DEBUG
  ptr = GC_debug_malloc (sz, fil, lin);
#else
  ptr = GC_MALLOC (sz);
#endif /* GC_DEBUG */
  memset (ptr, 0, sz);
  MOM_DEBUGPRINTF_AT (fil, lin, garbcoll, "mom_gc_alloc(%ld) -> %p-%p",
                      (long) sz, ptr, (char *) ptr + sz);
  return ptr;
}                               /* end mom_gc_alloc_at */


void *
mom_gc_alloc_scalar_at (size_t sz, const char *fil, int lin)
{
  void *ptr = NULL;
#ifdef GC_DEBUG
  ptr = GC_debug_malloc_atomic (sz, fil, lin);
#else
  ptr = GC_MALLOC_ATOMIC (sz);
#endif /* GC_DEBUG */
  memset (ptr, 0, sz);
  MOM_DEBUGPRINTF_AT (fil, lin, garbcoll, "mom_gc_alloc_scalar(%zd) -> %p-%p",
                      sz, ptr, (char *) ptr + sz);
  return ptr;
}                               /* end mom_gc_alloc_scalar_at */


void *
mom_gc_calloc_at (size_t nmemb, size_t sizel, const char *fil, int lin)
{
  void *ptr = NULL;
  size_t totsz = 0;
  if (nmemb > INT32_MAX / 3 || sizel > INT32_MAX / 3
      || (totsz = nmemb * sizel) > INT32_MAX)
    MOM_FATAPRINTF_AT (fil, lin,
                       "too big nmemb=%zd or sizel=%zd for mom_gc_calloc",
                       nmemb, sizel);
#ifdef GC_DEBUG
  ptr = GC_debug_malloc (totsz, fil, lin);
#else
  ptr = GC_MALLOC (totsz);
#endif /* GC_DEBUG */
  memset (ptr, 0, totsz);
  MOM_DEBUGPRINTF_AT (fil, lin, garbcoll,
                      "mom_gc_calloc(nmemb=%zd,sizel=%zd;totsz=%zd) -> %p-%p",
                      nmemb, sizel, totsz, ptr, (char *) ptr + totsz);
  return ptr;
}                               /* end mom_gc_calloc_at */


void *
mom_gc_alloc_uncollectable_at (size_t sz, const char *fil, int lin)
{
  void *ptr = NULL;
#ifdef GC_DEBUG
  ptr = GC_debug_malloc_uncollectable (sz, fil, lin);
#else
  ptr = GC_MALLOC_UNCOLLECTABLE (sz);
#endif /* GC_DEBUG */
  memset (ptr, 0, sz);
  MOM_DEBUGPRINTF_AT (fil, lin, garbcoll,
                      "mom_gc_alloc_uncollectable(%zd) -> %p-%p", sz, ptr,
                      (char *) ptr + sz);
  return ptr;
}                               /* end mom_gc_alloc_uncollectable_at */

char *
mom_gc_strdup_at (const char *str, const char *fil, int lin)
{
  void *ptr = NULL;
#ifdef GC_DEBUG
  ptr = GC_debug_strdup (str, fil, lin);
#else
  ptr = GC_STRDUP (str);
#endif /* GC_DEBUG */
  MOM_DEBUGPRINTF_AT (fil, lin, garbcoll,
                      "mom_gc_strdup('%s'@%p) -> %p-%p", str, str, ptr,
                      (char *) ptr + strlen (str));
  return ptr;
}                               /* end mom_gc_alloc_uncollectable_at */

const char *
mom_itype_str (const void *p)
{
  if (!p)
    return "*nil*";
  else if (p == MOM_EMPTY_SLOT)
    return "*emptyslot*";
  if ((intptr_t) p % 2 != 0)
    return "*tagINT*";
  else
    {
      unsigned ty = ((const struct mom_anyvalue_st *) p)->va_itype;
      switch (ty)
        {
        case MOMITY_NONE:
          return "NONE";
        case MOMITY_INT:
          return "INT";
        case MOMITY_BOXDOUBLE:
          return "BOXDOUBLE";
        case MOMITY_BOXSTRING:
          return "BOXSTRING";
        case MOMITY_ITEM:
          return "ITEM";
        case MOMITY_TUPLE:
          return "TUPLE";
        case MOMITY_SET:
          return "SET";
        case MOMITY_NODE:
          return "NODE";
        case MOMITY_ASSOVALDATA:
          return "Assocvaldata";
        case MOMITY_VECTVALDATA:
          return "Vectvaldata";
        case MOMITY_QUEUE:
          return "Queue";
        case MOMITY_HASHSET:
          return "Hashset";
        case MOMITY_HASHMAP:
          return "Hashmap";
        case MOMITY_HASHASSOC:
          return "Hashassoc";
        case MOMITY_LOADER:
          return "Loader";
        case MOMITY_DUMPER:
          return "Dumper";
        case MOMITY_JSON:
          return "Json";
        case MOMITY_WEBEXCH:
          return "Webexch";
        case MOMITY_WEBSESSION:
          return "Websession";
        case MOMITY_TASKLET:
          return "Tasklet";
        }
      char tybuf[32];
      memset (tybuf, 0, sizeof (tybuf));
      snprintf (tybuf, sizeof (tybuf), "?Ityp#%u?", ty);
      return mom_gc_strdup (tybuf);
    }
}                               /* end of mom_itype_str */

const void *
mom_int_make (intptr_t i)
{
  if ((i >= 0 && i < INTPTR_MAX / 4) || (i < 0 && i > -INTPTR_MAX / 4))
    {
      return (const void *) (((intptr_t) i << 1) | 1);
    }
  struct mom_boxint_st *bi =
    mom_gc_alloc_scalar (sizeof (struct mom_boxint_st));
  bi->va_itype = MOMITY_INT;
  bi->hva_hash = mom_int_hash (i);
  bi->boxi_int = i;
  return bi;
}

const struct mom_boxstring_st *
mom_boxstring_make (const char *s)
{
  if (!s || s == MOM_EMPTY_SLOT)
    return NULL;
  unsigned ln = strlen (s);
  if (ln >= MOM_SIZE_MAX)
    MOM_FATAPRINTF ("too long %d boxed string %.60s", ln, s);
  struct mom_boxstring_st *bs =
    mom_gc_alloc_scalar (sizeof (struct mom_boxstring_st) + ln + 1);
  bs->va_itype = MOMITY_BOXSTRING;
  bs->va_size = ln;
  bs->hva_hash = mom_cstring_hash_len (s, ln);
  memcpy (bs->boxs_cstr, s, ln);
  return bs;
}

const struct mom_boxstring_st *
mom_boxstring_make_len (const char *s, int len)
{
  if (!s || s == MOM_EMPTY_SLOT)
    return NULL;
  if (len < 0)
    len = strlen (s);
  if (len >= MOM_SIZE_MAX)
    MOM_FATAPRINTF ("too long %d boxed string %.60s", len, s);
  for (int ix = 0; ix < len; ix++)
    if (s[ix] == 0)
      {
        len = ix;
        break;
      };
  struct mom_boxstring_st *bs =
    mom_gc_alloc_scalar (sizeof (struct mom_boxstring_st) + (len) + 1);
  bs->va_itype = MOMITY_BOXSTRING;
  bs->va_size = len;
  bs->hva_hash = mom_cstring_hash_len (s, len);
  memcpy (bs->boxs_cstr, s, len);
  return bs;
}

const struct mom_boxstring_st *
mom_boxstring_printf (const char *fmt, ...)
{
  struct mom_boxstring_st *bsv = NULL;
  char buf[96];
  memset (buf, 0, sizeof (buf));
  va_list args;
  va_start (args, fmt);
  int ln = vsnprintf (buf, sizeof (buf), fmt, args);
  va_end (args);
  if (MOM_UNLIKELY (ln < 0))
    MOM_FATAPRINTF ("mom_boxstring_printf fmt=%s failure", fmt);
  if (ln < (int) sizeof (buf) - 1)
    {
      bsv = (struct mom_boxstring_st *) mom_boxstring_make (buf);
    }
  else if (MOM_UNLIKELY (ln >= MOM_SIZE_MAX))
    MOM_FATAPRINTF ("too long %d boxstring_printf for fmt %s", ln, fmt);
  else
    {
      bsv = mom_gc_alloc_scalar (sizeof (struct mom_boxstring_st) + ln + 1);
      bsv->va_itype = MOMITY_BOXSTRING;
      bsv->va_size = ln;
      va_start (args, fmt);
      if (MOM_UNLIKELY (ln != vsnprintf (bsv->boxs_cstr, ln + 1, fmt, args)))
        MOM_FATAPRINTF ("mom_boxstring_printf big ln=%d fmt=%s failure",
                        ln, fmt);
      va_end (args);
      bsv->hva_hash = mom_cstring_hash_len (bsv->boxs_cstr, ln);
    }
  return bsv;
}                               /* end of mom_boxstring_printf */


momhash_t
mom_double_hash (double d)
{
  if (isnan (d))
    return 3000229;
  int e = 0;
  double x = frexp (d, &e);
  momhash_t h = ((momhash_t) (x / (M_PI * M_LN2 * DBL_EPSILON))) ^ e;
  if (!h)
    {
      h = e;
      if (!h)
        h = (x > 0.0) ? 1689767 : (x < 0.0) ? 2000281 : 13;
    }
  return h;
}


const struct mom_boxdouble_st *
mom_boxdouble_make (double x)
{
  struct mom_boxdouble_st *bd =
    mom_gc_alloc_scalar (sizeof (struct mom_boxdouble_st));
  bd->va_itype = MOMITY_BOXDOUBLE;
  bd->hva_hash = mom_double_hash (x);
  bd->boxd_dbl = x;
  return bd;
}



////////////////
#define SEQITEM_EMPTY_HASH_MOM(Ty) (127 * (Ty))
static const struct mom_boxset_st empty_boxset_mom = {
  .va_itype = MOMITY_SET,
  .va_size = 0,
  .hva_hash = SEQITEM_EMPTY_HASH_MOM (MOMITY_SET)
};

static const struct mom_boxtuple_st empty_boxtuple_mom = {
  .va_itype = MOMITY_TUPLE,
  .va_size = 0,
  .hva_hash = SEQITEM_EMPTY_HASH_MOM (MOMITY_TUPLE)
};

MOM_PRIVATE void
seqitem_hash_compute_mom (struct mom_seqitems_st *si)
{
  assert (si);
  assert (si->hva_hash == 0);
  assert (si->va_itype == MOMITY_TUPLE || si->va_itype == MOMITY_SET);
  unsigned l = mom_raw_size (si);
  momhash_t h = 17 * l + SEQITEM_EMPTY_HASH_MOM (si->va_itype);
  if (l > 0)
    {
      for (unsigned ix = 0; ix < l; ix++)
        {
          const struct mom_item_st *curitm = si->seqitem[ix];
          if (ix % 2 != 0)
            {
              if (curitm != NULL)
                {
                  assert (mom_itype (curitm) == MOMITY_ITEM);
                  h = (1667 * curitm->hva_hash) ^ (31 * h + ix);
                }
              else
                h = 1709 * h + ix;
            }
          else
            {
              if (curitm != NULL)
                {
                  assert (mom_itype (curitm) == MOMITY_ITEM);
                  h = (1783 * curitm->hva_hash) ^ (11 * h - 13 * ix);
                }
              else
                h = 139 * h + 31 * ix;
            }
        };
    };
  if (MOM_UNLIKELY (h == 0))
    h = (0xffff & l) + 30 + 3 * si->va_itype;
  si->hva_hash = h;
}                               /* end seqitem_hash_compute_mom  */


const struct mom_boxtuple_st *
mom_boxtuple_make_arr2 (unsigned siz1, const struct mom_item_st *const *arr1,
                        unsigned siz2, const struct mom_item_st *const *arr2)
{
  if (!arr1 || arr1 == MOM_EMPTY_SLOT)
    {
      siz1 = 0;
      arr1 = NULL;
    };
  if (!arr2 || arr2 == MOM_EMPTY_SLOT)
    {
      siz2 = 0;
      arr2 = NULL;
    };
  if (MOM_UNLIKELY (siz1 == 0 && siz2 == 0))
    return &empty_boxtuple_mom;
  unsigned tsiz = siz1 + siz2;
  if (siz1 > MOM_SIZE_MAX || siz2 > MOM_SIZE_MAX || tsiz >= MOM_SIZE_MAX)
    MOM_FATAPRINTF ("too big tuple from siz1=%d, siz2=%d", siz1, siz2);
  struct mom_boxtuple_st *tup =
    mom_gc_alloc (sizeof (struct mom_boxtuple_st) +
                  (tsiz > 0) ? ((tsiz - 1) * sizeof (void *)) : 0);
  tup->va_itype = MOMITY_TUPLE;
  tup->va_size = tsiz;
  for (unsigned ix = 0; ix < siz1; ix++)
    tup->seqitem[ix] = mom_dyncast_item (arr1[ix]);
  for (unsigned ix = 0; ix < siz2; ix++)
    tup->seqitem[siz1 + ix] = mom_dyncast_item (arr2[ix]);
  seqitem_hash_compute_mom ((struct mom_seqitems_st *) tup);
  return tup;
}                               /* end mom_boxtuple_make_arr2 */


const struct mom_boxtuple_st *
mom_boxtuple_make_arr (unsigned siz, const struct mom_item_st *const *arr)
{
  if (arr == MOM_EMPTY_SLOT)
    arr = NULL;
  if (!arr && siz)
    return NULL;
  if (siz >= MOM_SIZE_MAX)
    MOM_FATAPRINTF ("too big tuple %d", siz);
  if (MOM_UNLIKELY (!arr || siz == 0))
    return &empty_boxtuple_mom;
  struct mom_boxtuple_st *tup =
    mom_gc_alloc (sizeof (struct mom_boxtuple_st) +
                  ((siz > 0) ? ((siz - 1) * sizeof (void *)) : 0));
  tup->va_itype = MOMITY_TUPLE;
  tup->va_size = siz;
  assert (mom_raw_size (tup) == siz);
  for (unsigned ix = 0; ix < siz; ix++)
    tup->seqitem[ix] = (struct mom_item_st *) (arr[ix]);
  seqitem_hash_compute_mom ((struct mom_seqitems_st *) tup);
  return tup;
}                               /* end mom_boxtuple_make_arr */



const struct mom_boxtuple_st *
mom_boxtuple_make_va (unsigned siz, ...)
{
  va_list args;
  if (siz >= MOM_SIZE_MAX)
    MOM_FATAPRINTF ("too big tuple %d", siz);
  if (MOM_UNLIKELY (siz == 0))
    return &empty_boxtuple_mom;
  struct mom_boxtuple_st *tup =
    mom_gc_alloc (sizeof (struct mom_boxtuple_st) +
                  ((siz > 0) ? ((siz - 1) * sizeof (void *)) : 0));
  tup->va_itype = MOMITY_TUPLE;
  tup->va_size = siz;
  va_start (args, siz);
  for (unsigned ix = 0; ix < siz; ix++)
    tup->seqitem[ix] = mom_dyncast_item (va_arg (args, struct mom_item_st *));
  seqitem_hash_compute_mom ((struct mom_seqitems_st *) tup);
  return tup;
}                               /* end mom_boxtuple_make_va */


const struct mom_boxtuple_st *
mom_boxtuple_flatten_make_va (struct mom_item_st *itm, unsigned siz, ...)
{
  va_list args;
  if (siz >= 2 * MOM_SIZE_MAX / 3)
    MOM_FATAPRINTF ("too big flat-tuple %d", siz);
  if (mom_itype (itm) != MOMITY_ITEM)
    itm = NULL;
  if (MOM_UNLIKELY (siz == 0))
    return &empty_boxtuple_mom;
  unsigned asiz = mom_prime_above (4 * siz / 3 + 5);
  unsigned acnt = 0;
  struct mom_item_st **arr = mom_gc_alloc (asiz * sizeof (void *));
  va_start (args, siz);
  for (int aix = 0; aix < (int) siz; aix++)
    {
      if (MOM_UNLIKELY (acnt + 1 >= asiz))
        {
          unsigned newsiz = mom_prime_above (5 * asiz / 4 + siz / 8 + 9);
          struct mom_item_st **newarr =
            mom_gc_alloc (newsiz * sizeof (void *));
          memcpy (newarr, arr, acnt * sizeof (void *));
          GC_FREE (arr);
          arr = newarr;
          asiz = newsiz;
        }
      momvalue_t curarg = va_arg (args, momvalue_t);
      switch (mom_itype (curarg))
        {
        case MOMITY_NONE:
          curarg = NULL;
          // failthru
        case MOMITY_ITEM:
          arr[acnt++] = (struct mom_item_st *) curarg;
          break;
        case MOMITY_SET:
        case MOMITY_TUPLE:
          {
            unsigned cursiz = mom_raw_size (curarg);
            const struct mom_seqitems_st *curseq =
              (const struct mom_seqitems_st *) curarg;
            if (MOM_UNLIKELY (acnt + cursiz + 1 >= asiz))
              {
                unsigned newsiz =
                  mom_prime_above (5 * asiz / 4 + siz / 8 + cursiz + 9);
                struct mom_item_st **newarr =
                  mom_gc_alloc (newsiz * sizeof (void *));
                memcpy (newarr, arr, acnt * sizeof (void *));
                GC_FREE (arr);
                arr = newarr;
                asiz = newsiz;
              }
            memcpy (arr + acnt, curseq->seqitem, cursiz * sizeof (void *));
            acnt += cursiz;
          }
          break;
        case MOMITY_NODE:
          {
            unsigned cursiz = mom_raw_size (curarg);
            const struct mom_boxnode_st *curnod =
              (const struct mom_boxnode_st *) curnod;
            if (itm && curnod->nod_connitm != itm)
              break;
            if (MOM_UNLIKELY (acnt + cursiz + 1 >= asiz))
              {
                unsigned newsiz =
                  mom_prime_above (5 * asiz / 4 + siz / 8 + cursiz + 9);
                struct mom_item_st **newarr =
                  mom_gc_alloc (newsiz * sizeof (void *));
                memcpy (newarr, arr, acnt * sizeof (void *));
                GC_FREE (arr);
                arr = newarr;
                asiz = newsiz;
              }
            for (unsigned six = 0; six < cursiz; six++)
              {
                momvalue_t curson = curnod->nod_sons[six];
                if (!curson)
                  arr[acnt++] = NULL;
                else if (mom_itype (curson) == MOMITY_ITEM)
                  arr[acnt++] = (struct mom_item_st *) curson;
              }
          }
          break;
        default:
          break;
        }
    }
  va_end (args);
  const struct mom_boxtuple_st *restup =
    mom_boxtuple_make_arr (acnt, (const struct mom_item_st **) arr);
  GC_FREE (arr);
  return restup;
}                               /* end mom_boxtuple_flatten_make_va */



const struct mom_boxtuple_st *
mom_boxtuple_make_sentinel_va (struct mom_item_st *itm1, ...)
{
  va_list args;
  unsigned siz = 0;
  struct mom_item_st *itm = NULL;
  va_start (args, itm1);
  for (va_start (args, itm1), itm = itm1;
       itm != NULL; itm = va_arg (args, struct mom_item_st *))
      siz++;
  va_end (args);
  struct mom_item_st *smallarr[16] = { NULL };
  struct mom_item_st **arr = (siz < sizeof (smallarr) / sizeof (smallarr[0])) ? smallarr        //
    : mom_gc_alloc (sizeof (void *) * siz);
  va_start (args, itm1);
  for (unsigned ix = 0; ix < siz; ix++)
    {
      struct mom_item_st *curitm =
        mom_dyncast_item (va_arg (args, struct mom_item_st *));
      if (curitm && curitm != MOM_EMPTY_SLOT)
        arr[ix] = curitm;
      else
        arr[ix] = NULL;
    }
  va_end (args);
  return mom_boxtuple_make_arr (siz, (const struct mom_item_st **) arr);
}                               /* end of mom_boxtuple_make_sentinel_va */


MOM_PRIVATE int
compare_item_ptr_mom (const void *p1, const void *p2)
{
  return mom_item_cmp (*(const struct mom_item_st **) p1,
                       *(const struct mom_item_st **) p2);
}


const struct mom_boxset_st *
mom_boxset_make_arr2 (unsigned siz1, const struct mom_item_st **arr1,
                      unsigned siz2, const struct mom_item_st **arr2)
{
  if (!arr1 || arr1 == MOM_EMPTY_SLOT)
    {
      siz1 = 0;
      arr1 = NULL;
    };
  if (!arr2 || arr2 == MOM_EMPTY_SLOT)
    {
      siz2 = 0;
      arr2 = NULL;
    };
  if (MOM_UNLIKELY (siz1 == 0 && siz2 == 0))
    return &empty_boxset_mom;
  unsigned tsiz = siz1 + siz2;
  if (siz1 > MOM_SIZE_MAX || siz2 > MOM_SIZE_MAX || tsiz >= MOM_SIZE_MAX)
    MOM_FATAPRINTF ("too big set from siz1=%d, siz2=%d", siz1, siz2);
  struct mom_boxset_st *set =
    mom_gc_alloc (sizeof (struct mom_boxset_st) +
                  ((tsiz > 0) ? ((tsiz - 1) * sizeof (void *)) : 0));
  set->va_itype = MOMITY_SET;
  unsigned cnt = 0;
  for (unsigned ix = 0; ix < siz1; ix++)
    {
      struct mom_item_st *elitm = mom_dyncast_item (arr1[ix]);
      if (!elitm)
        continue;
      set->seqitem[cnt++] = elitm;
    }
  for (unsigned ix = 0; ix < siz2; ix++)
    {
      struct mom_item_st *elitm = mom_dyncast_item (arr2[ix]);
      if (!elitm)
        continue;
      set->seqitem[cnt++] = elitm;
    }
  if (cnt > 1)
    qsort (set->seqitem, cnt, sizeof (void *), compare_item_ptr_mom);
  for (unsigned ix = 1; ix < cnt; ix++)
    if (set->seqitem[ix - 1] == set->seqitem[ix])
      {
        memmove (set->seqitem + ix - 1, set->seqitem + ix,
                 (cnt - ix) * sizeof (void *));
        cnt--;
      };
  if (cnt != tsiz)
    {
      assert (cnt < tsiz);
      struct mom_boxset_st *oldset = set;
      struct mom_boxset_st *newset =
        mom_gc_alloc (sizeof (struct mom_boxset_st) +
                      ((cnt > 0) ? ((cnt - 1) * sizeof (void *)) : 0));
      newset->va_itype = MOMITY_SET;
      if (tsiz > 10)
        GC_FREE (oldset);
      set = newset;
    };
  set->va_size = cnt;
  seqitem_hash_compute_mom ((struct mom_seqitems_st *) set);
  return set;
}                               /* end of mom_boxset_make_arr2 */

const struct mom_boxset_st *
mom_boxset_make_va (unsigned siz, ...)
{
  va_list args;
  if (siz >= MOM_SIZE_MAX)
    MOM_FATAPRINTF ("too big set %d", siz);
  struct mom_item_st *smallarr[16] = { NULL };
  const unsigned smallsize = sizeof (smallarr) / sizeof (smallarr[0]);
  struct mom_item_st **arr = (siz < smallsize) ? smallarr       //
    : mom_gc_alloc ((siz + 1) * sizeof (struct mom_item_st *));
  va_start (args, siz);
  unsigned cnt = 0;
  for (unsigned ix = 0; ix < siz; ix++)
    {
      struct mom_item_st *curitm =
        mom_dyncast_item (va_arg (args, struct mom_item_st *));
      if (!curitm)
        continue;
      arr[cnt++] = curitm;
    };
  return mom_boxset_make_arr (cnt, (const struct mom_item_st **) arr);
}                               /* end mom_boxset_make_va */

const struct mom_boxset_st *
mom_boxset_make_sentinel_va (struct mom_item_st *itm1, ...)
{
  va_list args;
  unsigned siz = 0;
  struct mom_item_st *itm = NULL;
  va_start (args, itm1);
  for (va_start (args, itm1), itm = itm1;
       itm != NULL; itm = va_arg (args, struct mom_item_st *))
      siz++;
  va_end (args);
  struct mom_item_st *smallarr[16] = { NULL };
  struct mom_item_st **arr = (siz < sizeof (smallarr) / sizeof (smallarr[0]))   //
    ? smallarr : mom_gc_alloc (sizeof (void *) * (siz + 1));
  va_start (args, itm1);
  for (unsigned ix = 0; ix < siz; ix++)
    arr[ix] = va_arg (args, struct mom_item_st *);
  va_end (args);
  return mom_boxset_make_arr (siz, (const struct mom_item_st **) arr);
}

const struct mom_boxset_st *
mom_boxset_flatten_make_va (struct mom_item_st *itm, unsigned siz, ...)
{
  va_list args;
  if (siz >= 2 * MOM_SIZE_MAX / 3)
    MOM_FATAPRINTF ("too big flat-tuple %d", siz);
  if (mom_itype (itm) != MOMITY_ITEM)
    itm = NULL;
  if (MOM_UNLIKELY (siz == 0))
    return &empty_boxset_mom;
  unsigned asiz = mom_prime_above (4 * siz / 3 + 5);
  unsigned acnt = 0;
  struct mom_item_st **arr = mom_gc_alloc (asiz * sizeof (void *));
  va_start (args, siz);
  for (int aix = 0; aix < (int) siz; aix++)
    {
      if (MOM_UNLIKELY (acnt + 1 >= asiz))
        {
          unsigned newsiz = mom_prime_above (5 * asiz / 4 + siz / 8 + 9);
          struct mom_item_st **newarr =
            mom_gc_alloc (newsiz * sizeof (void *));
          memcpy (newarr, arr, acnt * sizeof (void *));
          GC_FREE (arr);
          arr = newarr;
          asiz = newsiz;
        }
      momvalue_t curarg = va_arg (args, momvalue_t);
      switch (mom_itype (curarg))
        {
        case MOMITY_ITEM:
          arr[acnt++] = (struct mom_item_st *) curarg;
          break;
        case MOMITY_SET:
        case MOMITY_TUPLE:
          {
            unsigned cursiz = mom_raw_size (curarg);
            const struct mom_seqitems_st *curseq =
              (const struct mom_seqitems_st *) curarg;
            if (MOM_UNLIKELY (acnt + cursiz + 1 >= asiz))
              {
                unsigned newsiz =
                  mom_prime_above (5 * asiz / 4 + siz / 8 + cursiz + 9);
                struct mom_item_st **newarr =
                  mom_gc_alloc (newsiz * sizeof (void *));
                memcpy (newarr, arr, acnt * sizeof (void *));
                GC_FREE (arr);
                arr = newarr;
                asiz = newsiz;
              }
            for (unsigned six = 0; six < cursiz; six++)
              {
                struct mom_item_st *curitm = curseq->seqitem[six];
                if (curitm)
                  arr[acnt++] = curitm;
              }
          }
          break;
        case MOMITY_NODE:
          {
            unsigned cursiz = mom_raw_size (curarg);
            const struct mom_boxnode_st *curnod =
              (const struct mom_boxnode_st *) curnod;
            if (itm && curnod->nod_connitm != itm)
              break;
            if (MOM_UNLIKELY (acnt + cursiz + 1 >= asiz))
              {
                unsigned newsiz =
                  mom_prime_above (5 * asiz / 4 + siz / 8 + cursiz + 9);
                struct mom_item_st **newarr =
                  mom_gc_alloc (newsiz * sizeof (void *));
                memcpy (newarr, arr, acnt * sizeof (void *));
                GC_FREE (arr);
                arr = newarr;
                asiz = newsiz;
              }
            for (unsigned six = 0; six < cursiz; six++)
              {
                momvalue_t curson = curnod->nod_sons[six];
                if (mom_itype (curson) == MOMITY_ITEM)
                  arr[acnt++] = (struct mom_item_st *) curson;
              }
          }
          break;
        default:
          break;
        }
    }
  va_end (args);
  const struct mom_boxset_st *setv =
    mom_boxset_make_arr (acnt, (const struct mom_item_st **) arr);
  GC_FREE (arr);
  return setv;
}                               /* end mom_boxset_flatten_make_va */



const struct mom_boxset_st *
mom_boxset_union (const struct mom_boxset_st *set1,
                  const struct mom_boxset_st *set2)
{
  unsigned card1 = 0, card2 = 0;
  if (!set1 || set1 == MOM_EMPTY_SLOT || set1->va_itype != MOMITY_SET
      || !(card1 = mom_raw_size (set1)))
    set1 = NULL;
  if (!set2 || set2 == MOM_EMPTY_SLOT || set2->va_itype != MOMITY_SET
      || !(card2 = mom_raw_size (set2)))
    set2 = NULL;
  if (MOM_UNLIKELY (card1 == 0 && card2 == 0))
    return &empty_boxset_mom;
  if (!set1)
    return set2;
  if (!set2)
    return set1;
  if (card1 > MOM_SIZE_MAX)
    MOM_FATAPRINTF ("too big set1@%p of size %u", set1, card1);
  if (card2 > MOM_SIZE_MAX)
    MOM_FATAPRINTF ("too big set2@%p of size %u", set2, card2);
  unsigned siz = card1 + card2 + 1;
  const struct mom_item_st **arr =
    mom_gc_alloc ((siz + 1) * sizeof (struct mom_item_st *));
  unsigned i1 = 0, i2 = 0;
  unsigned nbun = 0;
  while (i1 < card1 && i2 < card2)
    {
      const struct mom_item_st *itm1 = set1->seqitem[i1];
      const struct mom_item_st *itm2 = set2->seqitem[i2];
      assert (itm1 && itm1->va_itype == MOMITY_ITEM);
      assert (itm2 && itm2->va_itype == MOMITY_ITEM);
      assert (nbun < siz);
      int cmp = mom_item_cmp (itm1, itm2);
      if (cmp < 0)
        {
          arr[nbun++] = itm1;
          i1++;
        }
      else if (cmp > 0)
        {
          arr[nbun++] = itm2;
          i2++;
        }
      else
        {
          assert (itm1 == itm2);
          arr[nbun++] = itm1;
          i1++, i2++;
        }
    }
  if (i1 < card1)
    for (unsigned ix = i1; ix < card1; ix++)
      arr[nbun++] = set1->seqitem[ix];
  else if (i2 < card2)
    for (unsigned ix = i2; ix < card2; ix++)
      arr[nbun++] = set2->seqitem[ix];
  return mom_boxset_make_arr (nbun, arr);
}                               /* end of mom_boxset_union */



const struct mom_boxset_st *
mom_boxset_intersection (const struct mom_boxset_st *set1,
                         const struct mom_boxset_st *set2)
{
  unsigned card1 = 0, card2 = 0;
  if (!set1 || set1 == MOM_EMPTY_SLOT || set1->va_itype != MOMITY_SET
      || !(card1 = mom_raw_size (set1)))
    return &empty_boxset_mom;
  if (!set2 || set2 == MOM_EMPTY_SLOT || set2->va_itype != MOMITY_SET
      || !(card2 = mom_raw_size (set2)))
    return &empty_boxset_mom;
  unsigned siz = ((card1 < card2) ? card1 : card2) + 1;
  const struct mom_item_st **arr =
    mom_gc_alloc (sizeof (struct mom_item_st *) * siz);
  unsigned i1 = 0, i2 = 0, nbin = 0;
  while (i1 < card1 && i2 < card2)
    {
      const struct mom_item_st *itm1 = set1->seqitem[i1];
      const struct mom_item_st *itm2 = set2->seqitem[i2];
      assert (itm1 && itm1->va_itype == MOMITY_ITEM);
      assert (itm2 && itm2->va_itype == MOMITY_ITEM);
      assert (nbin < siz);
      if (itm1 == itm2)
        goto same;
      int cmp = mom_item_cmp (itm1, itm2);
      if (cmp < 0)
        i1++;
      else if (cmp > 0)
        i2++;
      else
      same:
        {
          assert (itm1 == itm2);
          arr[nbin++] = itm1;
          i1++, i2++;
        }
    }
  return mom_boxset_make_arr (nbin, arr);
}                               /* end of mom_boxset_intersection */


const struct mom_boxset_st *
mom_boxset_difference (const struct mom_boxset_st *set1,
                       const struct mom_boxset_st *set2)
{
  unsigned card1 = 0, card2 = 0;
  if (!set1 || set1 == MOM_EMPTY_SLOT || set1->va_itype != MOMITY_SET
      || !(card1 = mom_raw_size (set1)))
    return &empty_boxset_mom;
  if (!set2 || set2 == MOM_EMPTY_SLOT || set2->va_itype != MOMITY_SET
      || !(card2 = mom_raw_size (set2)))
    return set1;
  unsigned siz = card1 + 1;
  const struct mom_item_st **arr =
    mom_gc_alloc (sizeof (struct mom_item_st *) * siz);
  unsigned i1 = 0, i2 = 0, nbdi = 0;
  while (i1 < card1 && i2 < card2)
    {
      const struct mom_item_st *itm1 = set1->seqitem[i1];
      const struct mom_item_st *itm2 = set2->seqitem[i2];
      assert (itm1 && itm1->va_itype == MOMITY_ITEM);
      assert (itm2 && itm2->va_itype == MOMITY_ITEM);
      assert (nbdi < siz);
      if (itm1 == itm2)
        goto same;
      int cmp = mom_item_cmp (itm1, itm2);
      if (cmp < 0)
        {
          i1++;
          arr[nbdi++] = itm1;
        }
      else if (cmp > 0)
        i2++;
      else
      same:
        {
          assert (itm1 == itm2);
          i1++, i2++;
        }
    }
  if (i1 < card1)
    {
      for (; i1 < card1; i1++)
        {
          const struct mom_item_st *itm1 = set1->seqitem[i1];
          assert (itm1 && itm1->va_itype == MOMITY_ITEM);
          assert (nbdi < siz);
          arr[nbdi++] = itm1;
        };
    }
  return mom_boxset_make_arr (nbdi, arr);
}                               /* end of mom_boxset_difference */


////////////////
MOM_PRIVATE void
update_node_hash_mom (struct mom_boxnode_st *nod)
{
  assert (nod);
  assert (nod->va_itype == MOMITY_NODE);
  assert (nod->hva_hash == 0);
  assert (nod->nod_connitm != NULL);
  unsigned siz = nod->va_size;
  momhash_t h = (23 * nod->nod_connitm->hva_hash) ^ (149 * siz);
  for (unsigned ix = 0; ix < siz; ix++)
    {
      const struct mom_hashedvalue_st *curson = nod->nod_sons[ix];
      if (!curson)
        continue;
      momhash_t hson = mom_hash (curson);
      if (ix % 2)
        h = (307 * h + 7 * ix) ^ (419 * hson);
      else
        h = (509 * h - 31 * ix) ^ (607 * hson);
    }
  if (!h)
    h = ((17 * nod->nod_connitm->hva_hash + 3 * siz) & 0xfffff) + 20;
  nod->hva_hash = h;
}

const struct mom_boxnode_st *
mom_boxnode_make_meta (const struct mom_item_st *connitm,
                       int size,
                       const momvalue_t *sons,
                       const struct mom_item_st *metaitm, intptr_t metarank)
{
  if (mom_itype (connitm) != MOMITY_ITEM)
    return NULL;
  if (!sons || sons == MOM_EMPTY_SLOT)
    size = 0;
  if (size < 0)
    size = 0;
  if (size >= MOM_SIZE_MAX)
    MOM_FATAPRINTF ("too big %d node of connective %s",
                    size, mom_item_cstring (connitm));
  struct mom_boxnode_st *nod =
    mom_gc_alloc (sizeof (*nod) +
                  ((size > 0) ? ((size - 1) * sizeof (void *)) : 0));
  nod->va_itype = MOMITY_NODE;
  nod->va_size = size;
  nod->nod_connitm = (struct mom_item_st *) connitm;
  nod->nod_metaitem = (struct mom_item_st *) metaitm;
  nod->nod_metarank = metarank;
  for (unsigned ix = 0; ix < (unsigned) size; ix++)
    {
      momvalue_t curson = sons[ix];
      if (curson == MOM_EMPTY_SLOT)
        curson = NULL;
      nod->nod_sons[ix] = curson;
    }
  update_node_hash_mom (nod);
  return nod;
}

const struct mom_boxnode_st *
mom_boxnode_meta_make_va (const struct mom_item_st *metaitm,
                          intptr_t metarank,
                          const struct mom_item_st *connitm, unsigned size,
                          ...)
{
  va_list args;
  if (mom_itype (connitm) != MOMITY_ITEM)
    return NULL;
  momvalue_t smallarr[8] = { NULL };
  if (size >= MOM_SIZE_MAX)
    MOM_FATAPRINTF ("too big %d node of connective %s",
                    size, mom_item_cstring (connitm));
  momvalue_t *arr
    = (size < (unsigned) (sizeof (smallarr) / sizeof (smallarr[0])))
    ? smallarr : mom_gc_alloc (size * sizeof (void *));
  va_start (args, size);
  for (unsigned ix = 0; ix < size; ix++)
    {
      momvalue_t curval = va_arg (args, momvalue_t);
      if (curval == MOM_EMPTY_SLOT)
        curval = NULL;
      arr[ix] = curval;
      MOM_DEBUGPRINTF (gencod, "arr[%d]: %s", ix, mom_value_cstring (curval));
    }
  va_end (args);
  return mom_boxnode_make_meta (connitm, size, arr, metaitm, metarank);
}                               /* end mom_boxnode_meta_make_va */


const struct mom_boxnode_st *
mom_boxnode_make_va (const struct mom_item_st *connitm,
                     unsigned size, /*momvalue_t-s */ ...)
{
  va_list args;
  if (mom_itype (connitm) != MOMITY_ITEM)
    return NULL;
  momvalue_t smallarr[8] = { NULL };
  if (size >= MOM_SIZE_MAX)
    MOM_FATAPRINTF ("too big %d node of connective %s",
                    size, mom_item_cstring (connitm));
  momvalue_t *arr
    = (size < (unsigned) (sizeof (smallarr) / sizeof (smallarr[0])))
    ? smallarr : mom_gc_alloc (size * sizeof (void *));
  va_start (args, size);
  for (unsigned ix = 0; ix < size; ix++)
    {
      momvalue_t curval = va_arg (args, momvalue_t);
      if (curval == MOM_EMPTY_SLOT)
        curval = NULL;
      arr[ix] = curval;
      assert (curval == NULL || mom_itype (curval) > 0);
    }
  va_end (args);
  return mom_boxnode_make (connitm, size, arr);
}                               /* end mom_boxnode_make_va */


const struct mom_boxnode_st *
mom_boxnode_flatten_make_va (const struct mom_item_st *connitm,
                             struct mom_item_st *itm, unsigned siz, ...)
{
  va_list args;
  if (MOM_UNLIKELY (siz >= 2 * MOM_SIZE_MAX / 3))
    MOM_FATAPRINTF ("too big flat-node %d", siz);
  if (mom_itype (connitm) != MOMITY_ITEM)
    return NULL;
  if (mom_itype (itm) != MOMITY_ITEM)
    itm = NULL;
  if (MOM_UNLIKELY (siz == 0))
    return mom_boxnode_make_va (connitm, 0);
  unsigned asiz = mom_prime_above (4 * siz / 3 + 5);
  unsigned acnt = 0;
  momvalue_t *arr = mom_gc_alloc (asiz * sizeof (void *));
  va_start (args, siz);
  for (int aix = 0; aix < (int) siz; aix++)
    {
      if (MOM_UNLIKELY (acnt + 1 >= asiz))
        {
          unsigned newsiz = mom_prime_above ((5 * asiz) / 4 + siz / 8 + 9);
          momvalue_t *newarr = mom_gc_alloc (newsiz * sizeof (void *));
          memcpy (newarr, arr, acnt * sizeof (void *));
          GC_FREE (arr);
          arr = newarr;
          asiz = newsiz;
        };
      momvalue_t curarg = va_arg (args, momvalue_t);
      unsigned curtyp = mom_itype (curarg);
      if (curtyp == MOMITY_NONE)
        curarg = NULL;
      if (curtyp == MOMITY_NODE)
        {
          const struct mom_boxnode_st *curnod =
            (const struct mom_boxnode_st *) curarg;
          if (itm && curnod->nod_connitm == itm)
            {
              unsigned nodsiz = mom_raw_size (curnod);
              if (MOM_UNLIKELY (acnt + nodsiz + 1 >= asiz))
                {
                  unsigned newsiz =
                    mom_prime_above ((5 * asiz) / 4 + siz / 8 +
                                     (5 * nodsiz) / 4 + 9);
                  momvalue_t *newarr =
                    mom_gc_alloc (newsiz * sizeof (void *));
                  memcpy (newarr, arr, acnt * sizeof (void *));
                  GC_FREE (arr);
                  arr = newarr;
                  asiz = newsiz;
                };
              memcpy (arr + acnt, curnod->nod_sons,
                      nodsiz * sizeof (momvalue_t));
              acnt += nodsiz;
            }
          else
            arr[acnt++] = curarg;
        }
      else
        arr[acnt++] = curarg;
    }
  va_end (args);
  const struct mom_boxnode_st *resnod = mom_boxnode_make (connitm, acnt, arr);
  GC_FREE (arr);
  return resnod;
}                               /* end mom_boxnode_flatten_make_va */



const struct mom_boxnode_st *
mom_boxnode_meta_make_sentinel_va (const struct mom_item_st *metaitm,
                                   intptr_t metarank,
                                   const struct mom_item_st *conn, ...)
{
  va_list args;
  if (!conn || conn == MOM_EMPTY_SLOT)
    return NULL;
  unsigned siz = 0;
  va_start (args, conn);
  while (va_arg (args, struct mom_hashedvalue_st *) != NULL)
      siz++;
  va_end (args);
  if (siz >= MOM_SIZE_MAX)
    MOM_FATAPRINTF ("too big %d node of connective %s",
                    siz, mom_item_cstring (conn));
  momvalue_t smallarr[12] = { NULL };
  momvalue_t *arr
    = (siz < sizeof (smallarr) / sizeof (smallarr[0])) ? smallarr
    : mom_gc_alloc (siz * sizeof (void *));
  va_start (args, conn);
  for (unsigned ix = 0; ix < siz; ix++)
    {
      momvalue_t curval = va_arg (args, momvalue_t);
      if (curval == MOM_EMPTY_SLOT)
        curval = NULL;
      arr[ix] = curval;
    }
  va_end (args);
  return mom_boxnode_make_meta (conn, siz, arr, metaitm, metarank);
}                               /* end of mom_boxnode_meta_make_sentinel_va */


////////////////
void
mom_queue_prepend (struct mom_queue_st *qu, const void *data)
{
  if (!qu || qu == MOM_EMPTY_SLOT || qu->va_itype != MOMITY_QUEUE)
    return;
  if (!data || data == MOM_EMPTY_SLOT)
    return;
  struct mom_quelem_st *qfirst = qu->qu_first;
  if (!qfirst)
    {
      assert (!qu->qu_last);
      struct mom_quelem_st *qe = mom_gc_alloc (sizeof (*qe));
      qe->qu_elems[0] = (struct mom_hashedvalue_st *) data;
      qu->qu_first = qu->qu_last = qe;
      return;
    }
  assert (qfirst != NULL && qu->qu_last != NULL);
  bool full = true;
  for (unsigned ix = 0; ix < MOM_NB_QUELEM && full; ix++)
    if (!qfirst->qu_elems[ix])
      full = false;
  if (full)
    {
      struct mom_quelem_st *qe = mom_gc_alloc (sizeof (*qe));
      qe->qu_elems[0] = (struct mom_hashedvalue_st *) data;
      qe->qu_next = qu->qu_first;
      qu->qu_first = qe;
    }
  else
    {
      struct mom_hashedvalue_st *qdata[MOM_NB_QUELEM];
      memset (qdata, 0, sizeof (qdata));
      qdata[0] = (struct mom_hashedvalue_st *) data;
      int cnt = 1;
      for (unsigned ix = 0; ix < MOM_NB_QUELEM; ix++)
        {
          assert (cnt < MOM_NB_QUELEM);
          if (qfirst->qu_elems[ix])
            qdata[cnt++] = qfirst->qu_elems[ix];
        };
      memcpy (qfirst->qu_elems, qdata, sizeof (qdata));
    }
}                               /* end of mom_queue_prepend */


void
mom_queue_append (struct mom_queue_st *qu, const void *data)
{
  if (!qu || qu == MOM_EMPTY_SLOT || qu->va_itype != MOMITY_QUEUE)
    return;
  if (!data || data == MOM_EMPTY_SLOT)
    return;
  struct mom_quelem_st *qlast = qu->qu_last;
  if (!qlast)
    {
      assert (!qu->qu_first);
      struct mom_quelem_st *qe = mom_gc_alloc (sizeof (*qe));
      qe->qu_elems[0] = (struct mom_hashedvalue_st *) data;
      qu->qu_first = qu->qu_last = qe;
      return;
    }
  assert (qlast != NULL && qu->qu_first != NULL);
  assert (qlast->qu_next == NULL);
  bool full = true;
  for (unsigned ix = 0; ix < MOM_NB_QUELEM && full; ix++)
    if (!qlast->qu_elems[ix])
      full = false;
  if (full)
    {
      struct mom_quelem_st *qe = mom_gc_alloc (sizeof (*qe));
      qe->qu_elems[0] = (struct mom_hashedvalue_st *) data;
      qlast->qu_next = qe;
      qu->qu_last = qe;
    }
  else
    {
      struct mom_hashedvalue_st *qdata[MOM_NB_QUELEM];
      memset (qdata, 0, sizeof (qdata));
      qdata[0] = (struct mom_hashedvalue_st *) data;
      int cnt = 0;
      for (unsigned ix = 0; ix < MOM_NB_QUELEM; ix++)
        {
          assert (cnt < MOM_NB_QUELEM);
          if (qlast->qu_elems[ix])
            qdata[cnt++] = qlast->qu_elems[ix];
        };
      assert (cnt <= MOM_NB_QUELEM - 1);
      qdata[cnt++] = (struct mom_hashedvalue_st *) data;
      memcpy (qlast->qu_elems, qdata, sizeof (qdata));
    }
}

void
mom_queue_pop_front (struct mom_queue_st *qu)
{
  if (!qu || qu == MOM_EMPTY_SLOT || qu->va_itype != MOMITY_QUEUE)
    return;
  struct mom_quelem_st *qfirst = qu->qu_first;
  if (!qfirst)
    return;
  struct mom_hashedvalue_st *qdata[MOM_NB_QUELEM];
  memset (qdata, 0, sizeof (qdata));
  int cnt = 0;
  for (unsigned ix = 0; ix < MOM_NB_QUELEM; ix++)
    if (qfirst->qu_elems[ix])
      qdata[cnt++] = qfirst->qu_elems[ix];
  if (cnt <= 1)
    {
      if (qfirst == qu->qu_last)
        {
          qu->qu_first = qu->qu_last = NULL;
          return;
        }
      else
        qu->qu_first = qfirst->qu_next;
    }
  else
    {
      memcpy (qfirst->qu_elems, qdata + 1,
              (MOM_NB_QUELEM - 1) * sizeof (void *));
      qfirst->qu_elems[MOM_NB_QUELEM - 1] = NULL;
    }
}




const struct mom_boxnode_st *
mom_queue_node (const struct mom_queue_st *qu,
                const struct mom_item_st *connitm)
{
  if (!qu || qu == MOM_EMPTY_SLOT || qu->va_itype != MOMITY_QUEUE)
    return NULL;
  if (!connitm || connitm == MOM_EMPTY_SLOT
      || connitm->va_itype != MOMITY_ITEM)
    return NULL;
  int nblink = 0;
  for (struct mom_quelem_st * ql = qu->qu_first; ql != NULL; ql = ql->qu_next)
    nblink++;
  if (nblink >= MOM_SIZE_MAX / MOM_NB_QUELEM - 1)
    MOM_FATAPRINTF ("too many links %d in queue", nblink);
  unsigned siz = nblink * MOM_NB_QUELEM;
  momvalue_t smallarr[3 * MOM_NB_QUELEM];
  memset (smallarr, 0, sizeof (smallarr));
  momvalue_t *arr =
    (siz < (sizeof (smallarr) / sizeof (smallarr[0]))) ? smallarr
    : mom_gc_alloc (siz * sizeof (momvalue_t));
  unsigned cnt = 0;
  for (struct mom_quelem_st * ql = qu->qu_first; ql != NULL; ql = ql->qu_next)
    {
      for (unsigned ix = 0; ix < MOM_NB_QUELEM; ix++)
        if (ql->qu_elems[ix])
          arr[cnt++] = ql->qu_elems[ix];
      assert (cnt <= siz);
    }
  return mom_boxnode_make (connitm, cnt, arr);
}                               /* end of mom_queue_node */


void
mom_dumpscan_queue (struct mom_dumper_st *du, const struct mom_queue_st *qu)
{
  assert (du && du->va_itype == MOMITY_DUMPER);
  if (!qu || qu == MOM_EMPTY_SLOT || qu->va_itype != MOMITY_QUEUE)
    return;
  if (!qu->qu_first || qu->qu_first == MOM_EMPTY_SLOT)
    return;
  for (struct mom_quelem_st * ql = qu->qu_first; ql != NULL; ql = ql->qu_next)
    {
      for (unsigned ix = 0; ix < MOM_NB_QUELEM; ix++)
        {
          struct mom_hashedvalue_st *curval = ql->qu_elems[ix];
          if (!curval || curval == MOM_EMPTY_SLOT)
            continue;
          mom_dumpscan_value (du, curval);
        }
    }
}                               /* end of mom_dumpscan_queue */


void
mom_dumpscan_value (struct mom_dumper_st *du,
                    const struct mom_hashedvalue_st *val)
{
  if (!val || val == MOM_EMPTY_SLOT)
    return;
  assert (du && du->va_itype == MOMITY_DUMPER);
  switch (mom_itype (val))
    {
    case MOMITY_INT:
    case MOMITY_BOXDOUBLE:
    case MOMITY_BOXSTRING:
      return;
    case MOMITY_TUPLE:
    case MOMITY_SET:
      {
        struct mom_item_st *const *itmarr
          = ((const struct mom_seqitems_st *) val)->seqitem;
        unsigned siz = mom_raw_size (val);
        for (unsigned ix = 0; ix < siz; ix++)
          if (itmarr[ix])
            mom_dumpscan_item (du, itmarr[ix]);
      }
      break;
    case MOMITY_NODE:
      {
        const struct mom_boxnode_st *nod =
          (const struct mom_boxnode_st *) val;
        unsigned siz = mom_raw_size (val);
        mom_dumpscan_item (du, nod->nod_connitm);
        if (nod->nod_metaitem)
          mom_dumpscan_item (du, nod->nod_metaitem);
        for (unsigned ix = 0; ix < siz; ix++)
          if (nod->nod_sons[ix])
            mom_dumpscan_value (du, nod->nod_sons[ix]);
      }
      break;
    case MOMITY_ITEM:
      mom_dumpscan_item (du, (struct mom_item_st *) val);
      break;
    default:
      MOM_FATAPRINTF ("invalid type#%d of value@%p", (int) (val->va_itype),
                      val);
    }
}



const char *
mom_double_to_cstr (double x, char *buf, size_t buflen)
{
  assert (buf != NULL && buflen > 10);
  if (isnan (x))
    {
      strncpy (buf, "+NAN", buflen);
      return buf;
    }
  else if (isinf (x))
    {
      if (x > 0.0)
        strncpy (buf, "+INF", buflen);
      else
        strncpy (buf, "-INF", buflen);
      return buf;
    }
  snprintf (buf, buflen, "%.2f", x);
  if (strchr (buf, '.') && atof (buf) == x)
    return buf;
  snprintf (buf, buflen, "%.5f", x);
  if (strchr (buf, '.') && atof (buf) == x)
    return buf;
  snprintf (buf, buflen, "%.7e", x);
  if (strchr (buf, '.') && atof (buf) == x)
    return buf;
  snprintf (buf, buflen, "%.9f", x);
  if (strchr (buf, '.') && atof (buf) == x)
    return buf;
  snprintf (buf, buflen, "%.15e", x);
  if (strchr (buf, '.') && atof (buf) == x)
    return buf;
  snprintf (buf, buflen, "%#g", x);
  if (strchr (buf, '.') && atof (buf) == x)
    return buf;
  snprintf (buf, buflen, "%#.19g", x);
  if (strchr (buf, '.') && atof (buf) == x)
    return buf;
  snprintf (buf, buflen, "%a", x);
  return buf;
}                               // end mom_double_to_cstr


void
mom_dumpemit_value (struct mom_dumper_st *du,
                    const struct mom_hashedvalue_st *val)
{
  if (!val || val == MOM_EMPTY_SLOT)
    return;
  assert (du && du->va_itype == MOMITY_DUMPER);
  assert (du->du_state == MOMDUMP_EMIT);
  FILE *femit = du->du_emitfile;
  assert (femit != NULL);
  switch (mom_itype (val))
    {
    case MOMITY_INT:
      fprintf (femit, "%lld_\n", (long long) mom_int_val_def (val, 0));
      return;
    case MOMITY_BOXDOUBLE:
      {
        char buf[48];
        memset (buf, 0, sizeof (buf));
        double x = ((struct mom_boxdouble_st *) val)->boxd_dbl;
        fprintf (femit, "%s_\n", mom_double_to_cstr (x, buf, sizeof (buf)));
      }
      return;
    case MOMITY_BOXSTRING:
      {
        const struct mom_boxstring_st *vstr
          = (const struct mom_boxstring_st *) val;
        fputc ('"', femit);
        mom_output_utf8_encoded (femit, vstr->boxs_cstr, mom_raw_size (val));
        fputs ("\"_\n", femit);
      }
      return;
    case MOMITY_TUPLE:
      {
        const struct mom_boxtuple_st *tup =
          (const struct mom_boxtuple_st *) val;
        unsigned siz = mom_raw_size (val);
        fputs ("(\n", femit);
        for (unsigned ix = 0; ix < siz; ix++)
          mom_dumpemit_refitem (du, tup->seqitem[ix]);
        fputs (")tuple\n", femit);
      }
      return;
    case MOMITY_SET:
      {
        const struct mom_boxset_st *set = (const struct mom_boxset_st *) val;
        unsigned siz = mom_raw_size (val);
        fputs ("(\n", femit);
        for (unsigned ix = 0; ix < siz; ix++)
          {
            const struct mom_item_st *elemitm = set->seqitem[ix];
            if (!mom_dumped_item (du, elemitm))
              continue;
            mom_dumpemit_refitem (du, elemitm);
          }
        fputs (")set\n", femit);
      }
      break;
    case MOMITY_NODE:
      {
        const struct mom_boxnode_st *nod =
          (const struct mom_boxnode_st *) val;
        if (!mom_dumped_item (du, nod->nod_connitm))
          {
            fputs ("~\n", femit);
            break;
          }
        unsigned siz = mom_raw_size (val);
        fputs ("(\n", femit);
        for (unsigned ix = 0; ix < siz; ix++)
          mom_dumpemit_value (du, nod->nod_sons[ix]);
        if (mom_dumped_item (du, nod->nod_metaitem) || nod->nod_metarank)
          {
            fprintf (femit, "%lld\n", (long long) nod->nod_metarank);
            mom_dumpemit_refitem (du, nod->nod_metaitem);
            mom_dumpemit_refitem (du, nod->nod_connitm);
            fputs (")nodemeta\n", femit);
          }
        else
          {
            mom_dumpemit_refitem (du, nod->nod_connitm);
            fputs (")node\n", femit);
          }
      }
      break;
    case MOMITY_ITEM:
      mom_dumpemit_refitem (du, (struct mom_item_st *) val);
      break;
    default:
      MOM_FATAPRINTF ("invalid type#%d of value@%p", (int) (val->va_itype),
                      val);
    }
}

#define MOM_MAXDEPTH_OUT 9
void
mom_output_value (FILE *fout, long *plastnl, int depth, const momvalue_t val)
{
  if (!fout || fout == MOM_EMPTY_SLOT)
    return;
  long lastnl = plastnl ? (*plastnl) : 0;
  if (!val || val == MOM_EMPTY_SLOT)
    {
      fputs ("~", fout);
      return;
    }
#define INDENTED_NEWLINE_MOM() do { fputc('\n', fout); lastnl = ftell(fout); for (int ix=depth % 16; ix>0; ix--) fputc(' ', fout); } while(0);
  unsigned valtype = mom_itype (val);
  switch (valtype)
    {
    case MOMITY_INT:
      fprintf (fout, "%lld", (long long) mom_int_val_def (val, 0));
      break;
    case MOMITY_BOXDOUBLE:
      {
        char dblbuf[48];
        memset (dblbuf, 0, sizeof (dblbuf));
        fputs (mom_double_to_cstr
               (((struct mom_boxdouble_st *) val)->boxd_dbl, dblbuf,
                sizeof (dblbuf)), fout);
      }
      break;
    case MOMITY_BOXSTRING:
      {
        fputc ('"', fout);
        mom_output_utf8_encoded (fout,
                                 ((struct mom_boxstring_st *) val)->boxs_cstr,
                                 mom_size (val));
        fputc ('"', fout);
      }
      break;
    case MOMITY_ITEM:
      fputs (mom_item_cstring ((struct mom_item_st *) val), fout);
      break;
    case MOMITY_SET:
    case MOMITY_TUPLE:
      if (valtype == MOMITY_SET)
        fputc ('{', fout);
      else
        fputc ('[', fout);
      {
        struct mom_seqitems_st *seq = (struct mom_seqitems_st *) val;
        unsigned siz = mom_size (seq);
        for (unsigned ix = 0; ix < siz; ix++)
          {
            if (ftell (fout) > lastnl + 64)
              {
                INDENTED_NEWLINE_MOM ();
              }
            if (ix > 0)
              fputc (' ', fout);
            if (ix > 0 && ix % 5 == 0)
              fputc (' ', fout);
            fputs (mom_item_cstring (seq->seqitem[ix]), fout);
          }
        if (valtype == MOMITY_SET)
          fputc ('}', fout);
        else
          fputc (']', fout);
        break;
    case MOMITY_NODE:
        {
          const struct mom_boxnode_st *nod
            = (const struct mom_boxnode_st *) val;
          fputs ("%", fout);
          fputs (mom_item_cstring (nod->nod_connitm), fout);
          if (nod->nod_metaitem || nod->nod_metarank)
            {
              fprintf (fout, "/%s:%ld", mom_item_cstring (nod->nod_metaitem),
                       (long) nod->nod_metarank);
            }
          if (depth > MOM_MAXDEPTH_OUT)
            fputs ("(...)", fout);
          else
            {
              fputc ('(', fout);
              unsigned siz = mom_size (nod);
              for (unsigned ix = 0; ix < siz; ix++)
                {
                  if (ftell (fout) > lastnl + 64)
                    {
                      INDENTED_NEWLINE_MOM ();
                    }
                  if (ix > 0)
                    fputc (' ', fout);
                  if (ix > 0 && ix % 5 == 0)
                    fputc (' ', fout);
                  mom_output_value (fout, &lastnl, depth + 1,
                                    nod->nod_sons[ix]);
                }
              fputc (')', fout);
            }
        }
        break;
    default:
        fprintf (fout, "<strange value@%p of type %d>", val, valtype);
      }
      fflush (fout);
    }
  if (plastnl)
    *plastnl = lastnl;
}                               /* end mom_output_value */

void
mom_output_item_content (FILE *fout, long *plastnl, struct mom_item_st *itm)
{

  if (!fout || fout == MOM_EMPTY_SLOT)
    return;
  if (!itm || itm == MOM_EMPTY_SLOT || itm->va_itype != MOMITY_ITEM)
    {
      fputs ("**??**\n", fout);
      if (plastnl)
        *plastnl = ftell (fout);
      return;
    }
  fprintf (fout, "*** %s ***\n", mom_item_cstring (itm));
  mom_item_lock (itm);
  char timbuf[80];
  memset (timbuf, 0, sizeof (timbuf));
  struct tm tm = { };
  strftime (timbuf, sizeof (timbuf), "%c %Z",
            localtime_r (&itm->itm_mtime, &tm));
  fprintf (fout, "#mtim: %s\n", timbuf);
  unsigned isp = mom_item_space (itm);
  if (isp == MOMSPA_NONE)
    fputs ("#nospace\n", fout);
  else if (isp == MOMSPA_PREDEF)
    fputs ("#predef\n", fout);
  else if (isp == MOMSPA_GLOBAL)
    fputs ("#global\n", fout);
  else
    fprintf (fout, "#space#%u\n", isp);
  // output attributes
  struct mom_assovaldata_st *attrs = itm->itm_pattr;
  if (attrs && attrs != MOM_EMPTY_SLOT
      && attrs->va_itype == MOMITY_ASSOVALDATA)
    {
      const struct mom_boxset_st *attset = mom_assovaldata_set_attrs (attrs);
      unsigned nbattrs = mom_size (attset);
      assert (nbattrs == 0 || attset->va_itype == MOMITY_SET);
      fprintf (fout, "#%d attributes:\n", nbattrs);
      for (unsigned aix = 0; aix < nbattrs; aix++)
        {
          long curlastnl = ftell (fout);
          const struct mom_item_st *curatitm = attset->seqitem[aix];
          assert (curatitm && curatitm->va_itype == MOMITY_ITEM);
          const void *curatval = mom_assovaldata_get (attrs, curatitm);
          fprintf (fout, "*%s: ", mom_item_cstring (curatitm));
          mom_output_value (fout, &curlastnl, 1, curatval);
          fputc ('\n', fout);
        }
    }
  // output components
  struct mom_vectvaldata_st *comps = itm->itm_pcomp;
  if (comps && comps != MOM_EMPTY_SLOT
      && comps->va_itype == MOMITY_VECTVALDATA)
    {
      unsigned nbcomps = mom_vectvaldata_count (comps);
      fprintf (fout, "#%d components:\n", nbcomps);
      for (unsigned cix = 0; cix < nbcomps; cix++)
        {
          long curlastnl = ftell (fout);
          const struct mom_hashedvalue_st *curcomp =
            mom_vectvaldata_nth (comps, cix);
          if (nbcomps >= 10)
            fprintf (fout, "[%2d] ", cix);
          else
            fprintf (fout, "[%d] ", cix);
          mom_output_value (fout, &curlastnl, 1, curcomp);
          fputc ('\n', fout);
        }
    }
  // output payload
  if (itm->itm_payldata)
    mom_unsync_item_output_payload (fout, itm);
  mom_item_unlock (itm);
  fprintf (fout, "--%s--\n", mom_item_cstring (itm));
  if (plastnl)
    *plastnl = ftell (fout);
}                               /* end mom_output_item_content */

#undef INDENTED_NEWLINE_MOM

void
mom_print_value (const char *msg, const void *val)
{
  if (msg)
    puts (msg);
  long nl = ftell (stdout);
  mom_output_value (stdout, &nl, 0, val);
  putc ('\n', stdout);
  fflush (NULL);
}                               /* end mom_print_value */

void
mom_print_content (const char *msg, const void *v)
{
  if (msg)
    puts (msg);
  long nl = ftell (stdout);
  if (mom_itype (v) == MOMITY_ITEM)
    {
      mom_output_item_content (stdout, &nl, (struct mom_item_st *) v);
    }
  else
    {
      mom_output_value (stdout, &nl, 0, v);
    }
  putc ('\n', stdout);
  fflush (NULL);
}                               /* end mom_print_content */

const char *
mom_value_cstring (const void *val)
{
  if (!val || val == MOM_EMPTY_SLOT)
    return "~";
  if (mom_itype (val) == MOMITY_ITEM)
    return mom_item_cstring ((const struct mom_item_st *) val);
  size_t outsiz = 80;
  char *outbuf = malloc (outsiz);
  if (!outbuf)
    MOM_FATAPRINTF ("failed to malloc outbuf of %zd bytes : %m", outsiz);
  memset (outbuf, 0, outsiz);
  FILE *fout = open_memstream (&outbuf, &outsiz);
  long lastnl = 0;
  mom_output_value (fout, &lastnl, 0, val);
  fflush (fout);
  long siz = ftell (fout);
  outbuf[siz] = (char) 0;
  char *res = mom_gc_strdup (outbuf);
  free (outbuf), outbuf = NULL;
  return res;
}


////////////////////////////////////////////////////////////////

bool
mom_hashedvalue_equal (const struct mom_hashedvalue_st * val1,
                       const struct mom_hashedvalue_st * val2)
{
  if (val1 == MOM_EMPTY_SLOT)
    val1 = NULL;
  if (val2 == MOM_EMPTY_SLOT)
    val2 = NULL;
  if (val1 == val2)
    return true;
  if (!val1 || !val2)
    return false;
  unsigned typ1 = mom_itype (val1);
  unsigned typ2 = mom_itype (val2);
  if (typ1 == 0 || typ1 >= MOMITY__LASTHASHED)
    return false;
  if (typ2 == 0 || typ2 >= MOMITY__LASTHASHED)
    return false;
  if (typ1 != typ2)
    return false;
  if (mom_hash (val1) != mom_hash (val2))
    return false;
  switch (typ1)
    {
    case MOMITY_INT:
      return mom_int_val_def (val1, 0) == mom_int_val_def (val2, 0);
    case MOMITY_BOXDOUBLE:
      {
        double d1 = ((const struct mom_boxdouble_st *) val1)->boxd_dbl;
        double d2 = ((const struct mom_boxdouble_st *) val2)->boxd_dbl;
        if (d1 == d2)
          return true;
        if (isnan (d1) && isnan (d2))
          return true;
        return false;
      }
    case MOMITY_BOXSTRING:
      return !strcmp (((const struct mom_boxstring_st *) val1)->boxs_cstr,
                      ((const struct mom_boxstring_st *) val2)->boxs_cstr);
    case MOMITY_ITEM:
      return val1 == val2;
    case MOMITY_TUPLE:
    case MOMITY_SET:
      {
        const struct mom_seqitems_st *sq1 =
          (const struct mom_seqitems_st *) val1;
        const struct mom_seqitems_st *sq2 =
          (const struct mom_seqitems_st *) val2;
        unsigned siz1 = mom_raw_size (sq1);
        if (siz1 != mom_raw_size (sq2))
          return false;
        for (unsigned ix = 0; ix < siz1; ix++)
          if (sq1->seqitem[ix] != sq2->seqitem[ix])
            return false;
        return true;
      }
    case MOMITY_NODE:
      {
        const struct mom_boxnode_st *nod1 =
          (const struct mom_boxnode_st *) val1;
        const struct mom_boxnode_st *nod2 =
          (const struct mom_boxnode_st *) val2;
        if (nod1->nod_connitm != nod2->nod_connitm)
          return false;
        unsigned siz1 = mom_raw_size (nod1);
        if (siz1 != mom_raw_size (nod2))
          return false;
        for (unsigned ix = 0; ix < siz1; ix++)
          {
            if (nod1->nod_sons[ix] == nod2->nod_sons[ix])
              continue;
            if (mom_hash (nod1->nod_sons[ix]) !=
                mom_hash (nod2->nod_sons[ix]))
              return false;
          }
        for (unsigned ix = 0; ix < siz1; ix++)
          {
            if (nod1->nod_sons[ix] == nod2->nod_sons[ix])
              continue;
            if (!mom_hashedvalue_equal
                (nod1->nod_sons[ix], nod2->nod_sons[ix]))
              return false;
          }
        return true;
      }
    }
  return false;
}                               /* end of mom_hashedvalue_equal */




int
mom_hashedvalue_cmp (const struct mom_hashedvalue_st *val1,
                     const struct mom_hashedvalue_st *val2)
{
  if (val1 == MOM_EMPTY_SLOT)
    val1 = NULL;
  if (val2 == MOM_EMPTY_SLOT)
    val2 = NULL;
  if (val1 == val2)
    return 0;
  if (!val1)
    return -1;
  if (!val2)
    return 1;
  unsigned typ1 = mom_itype (val1);
  unsigned typ2 = mom_itype (val2);
  if (typ1 == 0 || typ1 >= MOMITY__LASTHASHED)
    return -1;
  if (typ2 == 0 || typ2 >= MOMITY__LASTHASHED)
    return 1;
  if (typ1 < typ2)
    return -1;
  if (typ1 > typ2)
    return 1;
  if (MOM_UNLIKELY
      (mom_hash (val1) == mom_hash (val2)
       && mom_hashedvalue_equal (val1, val2)))
    return 0;
  switch (typ1)
    {
    case MOMITY_INT:
      {
        intptr_t i1 = mom_int_val_def (val1, 0);
        intptr_t i2 = mom_int_val_def (val2, 0);
        if (i1 < i2)
          return -1;
        else if (i1 > i2)
          return 1;
        return 0;
      }
    case MOMITY_BOXDOUBLE:
      {
        double d1 = ((const struct mom_boxdouble_st *) val1)->boxd_dbl;
        double d2 = ((const struct mom_boxdouble_st *) val2)->boxd_dbl;
        if (d1 == d2)
          return 0;
        if (isnan (d1) && isnan (d2))
          return 0;
        if (d1 < d2 || isnan (d1))
          return -1;
        if (d1 > d2 || isnan (d2))
          return 1;
        return 0;
      }
    case MOMITY_BOXSTRING:
      {
        int c = strcmp (((const struct mom_boxstring_st *) val1)->boxs_cstr,
                        ((const struct mom_boxstring_st *) val2)->boxs_cstr);
        if (c < 0)
          return -1;
        else if (c > 0)
          return 1;
        return 0;
      }
    case MOMITY_ITEM:
      return mom_item_cmp ((const struct mom_item_st *) val1,
                           (const struct mom_item_st *) val2);
    case MOMITY_SET:
    case MOMITY_TUPLE:
      {
        const struct mom_seqitems_st *sq1 =
          (const struct mom_seqitems_st *) val1;
        const struct mom_seqitems_st *sq2 =
          (const struct mom_seqitems_st *) val2;
        unsigned siz1 = mom_raw_size (sq1);
        unsigned siz2 = mom_raw_size (sq2);
        unsigned minsiz = (siz1 < siz2) ? siz1 : siz2;
        for (unsigned ix = 0; ix < minsiz; ix++)
          {
            const struct mom_item_st *itm1 = sq1->seqitem[ix];
            const struct mom_item_st *itm2 = sq2->seqitem[ix];
            if (itm1 == itm2)
              continue;
            return mom_item_cmp (itm1, itm2);
          }
        if (siz1 > siz2)
          return 1;
        else if (siz1 < siz2)
          return -1;
        return 0;
      }
    case MOMITY_NODE:
      {
        const struct mom_boxnode_st *nod1 =
          (const struct mom_boxnode_st *) val1;
        const struct mom_boxnode_st *nod2 =
          (const struct mom_boxnode_st *) val2;
        if (nod1->nod_connitm != nod2->nod_connitm)
          return mom_item_cmp (nod1->nod_connitm, nod2->nod_connitm);
        unsigned siz1 = mom_raw_size (nod1);
        unsigned siz2 = mom_raw_size (nod2);
        unsigned minsiz = (siz1 < siz2) ? siz1 : siz2;
        for (unsigned ix = 0; ix < minsiz; ix++)
          {
            const struct mom_hashedvalue_st *son1 = nod1->nod_sons[ix];
            const struct mom_hashedvalue_st *son2 = nod2->nod_sons[ix];
            if (son1 == son2)
              continue;
            if (!son1)
              return -1;
            if (!son2)
              return 1;
            if (MOM_UNLIKELY
                (son1->hva_hash == son2->hva_hash
                 && mom_hashedvalue_equal (son1, son2)))
              continue;
            return mom_hashedvalue_cmp (son1, son2);
          }
        if (siz1 > siz2)
          return 1;
        else if (siz1 < siz2)
          return -1;
        return 0;
      }
    }
  MOM_FATAPRINTF ("corrupted compare of val1@%p val2@%p", val1, val2);
}                               /* end of mom_hashedvalue_cmp */

////////////////////////////////////////////////////////////////

extern mom_loader_paren_sig_t momf_ldp_tuple;
extern mom_loader_paren_sig_t momf_ldp_set;
extern mom_loader_paren_sig_t momf_ldp_node;
extern mom_loader_paren_sig_t momf_ldp_nodemeta;

void
momf_ldp_tuple (struct mom_item_st *itm,
                struct mom_loader_st *ld,
                struct mom_statelem_st *elemarr, unsigned elemsize)
{
  MOM_DEBUGPRINTF (load, "momf_ldp_tuple start itm=%s elemsize=%u",
                   mom_item_cstring (itm), elemsize);
  assert (itm && itm->va_itype == MOMITY_ITEM);
  assert (ld && ld->va_itype == MOMITY_LOADER);
  struct mom_item_st *smallarr[16] = { };
  const struct mom_item_st **arr =
    (elemsize < sizeof (smallarr) / sizeof (smallarr[0])) ? smallarr :
    mom_gc_alloc ((elemsize + 1) * sizeof (struct mom_item_st *));
  for (unsigned ix = 0; ix < elemsize; ix++)
    {
      arr[ix] = mom_ldstate_dynitem (elemarr[ix]);
      MOM_DEBUGPRINTF (load, "momf_ldp_tuple arr[%d] = %s",
                       ix, mom_item_cstring (arr[ix]));
    }
  const struct mom_boxtuple_st *tup = mom_boxtuple_make_arr (elemsize, arr);
  mom_loader_push (ld, mom_ldstate_make_tuple (tup));
  ld->ld_kindcount[MOMITY_TUPLE]++;
  MOM_DEBUGPRINTF (load, "momf_ldp_tuple pushed #%d %s",
                   ld->ld_stacktop,
                   mom_value_cstring ((const struct mom_hashedvalue_st *)
                                      tup));
}                               /* end of momf_ldp_tuple */

void
momf_ldp_set (struct mom_item_st *itm,
              struct mom_loader_st *ld,
              struct mom_statelem_st *elemarr, unsigned elemsize)
{
  MOM_DEBUGPRINTF (load, "momf_ldp_set start itm=%s elemsize=%u",
                   mom_item_cstring (itm), elemsize);
  assert (itm && itm->va_itype == MOMITY_ITEM);
  assert (ld && ld->va_itype == MOMITY_LOADER);
  struct mom_item_st *smallarr[16] = { };
  const struct mom_item_st **arr =
    (elemsize < sizeof (smallarr) / sizeof (smallarr[0])) ? smallarr :
    mom_gc_alloc ((elemsize + 1) * sizeof (struct mom_item_st *));
  for (unsigned ix = 0; ix < elemsize; ix++)
    {
      arr[ix] = mom_ldstate_dynitem (elemarr[ix]);
      MOM_DEBUGPRINTF (load, "momf_ldp_set arr[%d] = %s",
                       ix, mom_item_cstring (arr[ix]));
    }
  const struct mom_boxset_st *set = mom_boxset_make_arr (elemsize, arr);
  mom_loader_push (ld, mom_ldstate_make_set (set));
  ld->ld_kindcount[MOMITY_SET]++;
  MOM_DEBUGPRINTF (load, "momf_ldp_set pushed #%d %s",
                   ld->ld_stacktop,
                   mom_value_cstring ((const struct mom_hashedvalue_st *)
                                      set));
}                               /* end of momf_ldp_set */

void
momf_ldp_node (struct mom_item_st *itm,
               struct mom_loader_st *ld,
               struct mom_statelem_st *elemarr, unsigned elemsize)
{
  MOM_DEBUGPRINTF (load, "momf_ldp_node start itm=%s elemsize=%u",
                   mom_item_cstring (itm), elemsize);
  if (elemsize == 0)
    MOM_FATAPRINTF ("momf_ldp_node itm=%s zero elemsize",
                    mom_item_cstring (itm));
  assert (itm && itm->va_itype == MOMITY_ITEM);
  assert (ld && ld->va_itype == MOMITY_LOADER);
  momvalue_t smallarr[16] = { };
  momvalue_t *arr =
    (elemsize < sizeof (smallarr) / sizeof (smallarr[0])) ? smallarr :
    mom_gc_alloc ((elemsize + 1) * sizeof (struct mom_hashedvalue_st *));
  for (unsigned ix = 0; ix < elemsize - 1; ix++)
    {
      arr[ix] =
        (const struct mom_hashedvalue_st *) mom_ldstate_val (elemarr[ix]);
      MOM_DEBUGPRINTF (load, "momf_ldp_node arr[%d] = %s", ix,
                       mom_value_cstring (arr[ix]));
    }
  struct mom_item_st *connitm = mom_ldstate_dynitem (elemarr[elemsize - 1]);
  MOM_DEBUGPRINTF (load, "momf_ldp_node connitm=%s",
                   mom_item_cstring (connitm));
  const struct mom_boxnode_st *nod =
    mom_boxnode_make (connitm, elemsize - 1, arr);
  mom_loader_push (ld, mom_ldstate_make_node (nod));
  ld->ld_kindcount[MOMITY_NODE]++;
  MOM_DEBUGPRINTF (load, "momf_ldp_node pushed #%d %s",
                   ld->ld_stacktop,
                   mom_value_cstring ((const struct mom_hashedvalue_st *)
                                      nod));
}                               /* end of momf_ldp_node */

void
momf_ldp_nodemeta (struct mom_item_st *itm,
                   struct mom_loader_st *ld,
                   struct mom_statelem_st *elemarr, unsigned elemsize)
{
  MOM_DEBUGPRINTF (load, "momf_ldp_nodemeta start itm=%s elemsize=%u",
                   mom_item_cstring (itm), elemsize);
  assert (itm && itm->va_itype == MOMITY_ITEM);
  assert (ld && ld->va_itype == MOMITY_LOADER);
  if (elemsize < 3)
    MOM_FATAPRINTF ("momf_ldp_nodemeta itm=%s bad elemsize %d",
                    mom_item_cstring (itm), elemsize);
  assert (itm && itm->va_itype == MOMITY_ITEM);
  assert (ld && ld->va_itype == MOMITY_LOADER);
  momvalue_t smallarr[16] = { };
  momvalue_t *arr =
    (elemsize < sizeof (smallarr) / sizeof (smallarr[0])) ? smallarr :
    mom_gc_alloc ((elemsize + 1) * sizeof (struct mom_hashedvalue_st *));
  for (unsigned ix = 0; ix < elemsize - 3; ix++)
    {
      arr[ix] = mom_ldstate_val (elemarr[ix]);
      MOM_DEBUGPRINTF (load, "momf_ldp_nodemeta arr[%d] = %s", ix,
                       mom_value_cstring (arr[ix]));
    }
  struct mom_item_st *connitm = mom_ldstate_dynitem (elemarr[elemsize - 1]);
  struct mom_item_st *metaitm = mom_ldstate_dynitem (elemarr[elemsize - 2]);;
  long metark = mom_ldstate_int_def (elemarr[elemsize - 3], -1);
  MOM_DEBUGPRINTF (load, "momf_ldp_nodemeta connitm=%s metaitm=%s metark=%ld",
                   mom_item_cstring (connitm), mom_item_cstring (metaitm),
                   metark);
  const struct mom_boxnode_st *nod =
    mom_boxnode_make_meta (connitm, elemsize - 3, arr, metaitm, metark);
  ld->ld_kindcount[MOMITY_NODE]++;
  mom_loader_push (ld, mom_ldstate_make_node (nod));
  MOM_DEBUGPRINTF (load, "momf_ldp_nodemeta pushed #%d %s",
                   ld->ld_stacktop,
                   mom_value_cstring ((const struct mom_hashedvalue_st *)
                                      nod));
}                               /* end of momf_ldp_nodemeta */

/* end of file value.c */
