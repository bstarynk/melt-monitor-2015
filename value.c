// file value.c - managing values

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

const struct mom_boxint_st *
mom_boxint_make (intptr_t i)
{
  struct mom_boxint_st *bi =
    mom_gc_alloc_atomic (sizeof (struct mom_boxint_st));
  bi->va_ltype = MOM_BOXINT_LTYPE;
  bi->hva_hash = mom_int_hash (i);
  bi->boxi_int = i;
  return bi;
}

const struct mom_boxstring_st *
mom_boxstring_make (const char *s)
{
  if (!s || s == MOM_EMPTY_SLOT)
    return NULL;
  unsigned l = strlen (s);
  if (l >= MOM_SIZE_MAX)
    MOM_FATAPRINTF ("too long %d boxed string %.60s", l, s);
  struct mom_boxstring_st *bs =
    mom_gc_alloc_atomic (sizeof (struct mom_boxstring_st) + (l + 1));
  bs->va_itype = MOM_BOXSTRING_ITYPE;
  bs->va_hsiz = l >> 16;
  bs->va_lsiz = l & 0xffff;
  bs->hva_hash = mom_cstring_hash_len (s, l);
  memcpy (bs->cstr, s, l);
  return bs;
}

static void
seqitem_hash_compute_mom (struct mom_seqitems_st *si)
{
  assert (si);
  assert (si->hva_hash == 0);
  unsigned l = ((si->va_hsiz << 16) + si->va_lsiz);
  momhash_t h = 17 * l + 127 * si->va_itype;
  for (unsigned ix = 0; ix < l; ix++)
    {
      if (ix & 1)
        {
          if (si->seqitem[ix])
            h = (1667 * si->seqitem[ix]->hva_hash) ^ (31 * h + ix);
          else
            h = 1709 * h + ix;
        }
      else
        {
          if (si->seqitem[ix])
            h = (1783 * si->seqitem[ix]->hva_hash) ^ (11 * h - 13 * ix);
          else
            h = 139 * h + 5 * ix;
        }
    };
  if (!h)
    h = (0xffff & l) + 30 + 3 * si->va_itype;
  si->hva_hash = h;
}

const struct mom_boxtuple_st *
mom_boxtuple_make_arr2 (unsigned siz1, const struct mom_item_st **arr1,
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
  if (!arr1 && !arr2)
    return NULL;
  unsigned tsiz = siz1 + siz2;
  if (siz1 > MOM_SIZE_MAX || siz2 > MOM_SIZE_MAX || tsiz >= MOM_SIZE_MAX)
    MOM_FATAPRINTF ("too big tuple from siz1=%d, siz2=%d", siz1, siz2);
  struct mom_boxtuple_st *tup =
    mom_gc_alloc (sizeof (struct mom_boxtuple_st) + tsiz * sizeof (void *));
  tup->va_itype = MOM_TUPLE_ITYPE;
  tup->va_hsiz = tsiz >> 16;
  tup->va_lsiz = tsiz & 0xffff;
  for (unsigned ix = 0; ix < siz1; ix++)
    tup->seqitem[ix] = (struct mom_item_st *) (arr1[ix]);
  for (unsigned ix = 0; ix < siz2; ix++)
    tup->seqitem[siz1 + ix] = (struct mom_item_st *) (arr2[ix]);
  seqitem_hash_compute_mom ((struct mom_seqitems_st *) tup);
  return tup;
}                               /* end mom_boxtuple_make_arr2 */

const struct mom_boxtuple_st *
mom_boxtuple_make_arr (unsigned siz, const struct mom_item_st **arr)
{
  if (arr == MOM_EMPTY_SLOT)
    arr = NULL;
  if (!arr && siz)
    return NULL;
  if (siz >= MOM_SIZE_MAX)
    MOM_FATAPRINTF ("too big tuple %d", siz);
  struct mom_boxtuple_st *tup =
    mom_gc_alloc (sizeof (struct mom_boxtuple_st) + siz * sizeof (void *));
  tup->va_itype = MOM_TUPLE_ITYPE;
  tup->va_hsiz = siz >> 16;
  tup->va_lsiz = siz & 0xffff;
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
  struct mom_boxtuple_st *tup =
    mom_gc_alloc (sizeof (struct mom_boxtuple_st) + siz * sizeof (void *));
  tup->va_itype = MOM_TUPLE_ITYPE;
  tup->va_hsiz = siz >> 16;
  tup->va_lsiz = siz & 0xffff;
  va_start (args, siz);
  for (unsigned ix = 0; ix < siz; ix++)
    tup->seqitem[ix] = va_arg (args, struct mom_item_st *);
  seqitem_hash_compute_mom ((struct mom_seqitems_st *) tup);
  return tup;
}                               /* end mom_boxtuple_make_va */


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
  struct mom_item_st **arr =
    (siz <
     sizeof (smallarr) /
     sizeof (smallarr[0])) ? smallarr : mom_gc_alloc (sizeof (void *) * siz);
  va_start (args, itm1);
  for (unsigned ix = 0; ix < siz; ix++)
    {
      struct mom_item_st *curitm = va_arg (args, struct mom_item_st *);
      if (curitm && curitm != MOM_EMPTY_SLOT)
        arr[ix] = curitm;
      else
        arr[ix] = NULL;
    }
  va_end (args);
  return mom_boxtuple_make_arr (siz, (const struct mom_item_st **) arr);
}                               /* end of mom_boxtuple_make_sentinel_va */


static int
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
  if (!arr1 && !arr2)
    return NULL;
  unsigned tsiz = siz1 + siz2;
  if (siz1 > MOM_SIZE_MAX || siz2 > MOM_SIZE_MAX || tsiz >= MOM_SIZE_MAX)
    MOM_FATAPRINTF ("too big set from siz1=%d, siz2=%d", siz1, siz2);
  struct mom_boxset_st *set =
    mom_gc_alloc (sizeof (struct mom_boxset_st) + tsiz * sizeof (void *));
  set->va_itype = MOM_SET_ITYPE;
  unsigned cnt = 0;
  for (unsigned ix = 0; ix < siz1; ix++)
    {
      if (!arr1[ix] || arr1[ix] == MOM_EMPTY_SLOT)
        continue;
      set->seqitem[cnt++] = (struct mom_item_st *) (arr1[ix]);
    }
  for (unsigned ix = 0; ix < siz2; ix++)
    {
      if (!arr2[ix] || arr2[ix] == MOM_EMPTY_SLOT)
        set->seqitem[cnt++] = (struct mom_item_st *) (arr2[ix]);
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
        mom_gc_alloc (sizeof (struct mom_boxset_st) + cnt * sizeof (void *));
      newset->va_itype = MOM_SET_ITYPE;
      if (tsiz > 10)
        GC_FREE (oldset);
      set = newset;
    };
  set->va_hsiz = cnt >> 16;
  set->va_lsiz = cnt & 0xffff;
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
  struct mom_item_st **arr =
    siz <
    smallsize ? smallarr : mom_gc_alloc (siz * sizeof (struct mom_item_st *));
  va_start (args, siz);
  unsigned cnt = 0;
  for (unsigned ix = 0; ix < siz; ix++)
    {
      struct mom_item_st *curitm = va_arg (args, struct mom_item_st *);
      if (!curitm || curitm == MOM_EMPTY_SLOT)
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
  struct mom_item_st **arr
    =
    (siz <
     sizeof (smallarr) /
     sizeof (smallarr[0])) ? smallarr : mom_gc_alloc (sizeof (void *) * siz);
  va_start (args, itm1);
  for (unsigned ix = 0; ix < siz; ix++)
    arr[ix] = va_arg (args, struct mom_item_st *);
  va_end (args);
  return mom_boxset_make_arr (siz, (const struct mom_item_st **) arr);
}


static void
update_node_hash_mom (struct mom_boxnode_st *nod)
{
  assert (nod);
  assert (nod->va_itype == MOM_NODE_ITYPE);
  assert (nod->hva_hash == 0);
  assert (nod->nod_connitm != NULL);
  unsigned siz = (nod->va_hsiz << 16) | nod->va_lsiz;
  momhash_t h = (23 * nod->nod_connitm->hva_hash) ^ (149 * siz);
  for (unsigned ix = 0; ix < siz; ix++)
    {
      const struct mom_hashedvalue_st *curson = nod->nod_sons[ix];
      if (!curson)
        continue;
      if (ix % 2)
        h = (307 * h + 7 * ix) ^ (419 * curson->hva_hash);
      else
        h = (509 * h - 31 * ix) ^ (607 * curson->hva_hash);
    }
  if (!h)
    h = ((nod->nod_connitm->hva_hash + 3 * siz) & 0xffff) + 20;
  nod->hva_hash = h;
}

const struct mom_boxnode_st *
mom_boxnode_make_meta (const struct mom_item_st *conn,
                       unsigned size,
                       const struct mom_hashedvalue_st **sons,
                       const struct mom_item_st *metaitm, intptr_t metarank)
{
  if (!conn || conn == MOM_EMPTY_SLOT)
    return NULL;
  if (!sons || sons == MOM_EMPTY_SLOT)
    size = 0;
  if (size >= MOM_SIZE_MAX)
    MOM_FATAPRINTF ("too big %d node of connective %s",
                    size, mom_item_cstring (conn));
  struct mom_boxnode_st *nod =
    mom_gc_alloc (sizeof (*nod) + size * sizeof (void *));
  nod->va_itype = MOM_NODE_ITYPE;
  nod->va_hsiz = size >> 16;
  nod->va_lsiz = size & 0xffff;
  nod->nod_connitm = (struct mom_item_st *) conn;
  nod->nod_metaitem = (struct mom_item_st *) metaitm;
  nod->nod_metarank = metarank;
  for (unsigned ix = 0; ix < size; ix++)
    {
      const struct mom_hashedvalue_st *curson = sons[ix];
      if (curson == MOM_EMPTY_SLOT)
        curson = NULL;
      nod->nod_sons[ix] = (struct mom_hashedvalue_st *) curson;
    }
  update_node_hash_mom (nod);
  return nod;
}

const struct mom_boxnode_st *
mom_boxnode_meta_make_va (const struct mom_item_st *metaitm,
                          intptr_t metarank,
                          const struct mom_item_st *conn, unsigned size, ...)
{
  va_list args;
  if (!conn || conn == MOM_EMPTY_SLOT)
    return NULL;
  struct mom_hashedvalue_st *smallarr[16] = { NULL };
  if (size >= MOM_SIZE_MAX)
    MOM_FATAPRINTF ("too big %d node of connective %s",
                    size, mom_item_cstring (conn));
  struct mom_hashedvalue_st **arr
    = (size < sizeof (smallarr) / sizeof (smallarr[0])) ? smallarr
    : mom_gc_alloc (size * sizeof (void *));
  va_start (args, size);
  for (unsigned ix = 0; ix < size; ix++)
    {
      struct mom_hashedvalue_st *curval =
        va_arg (args, struct mom_hashedvalue_st *);
      if (curval == MOM_EMPTY_SLOT)
        curval = NULL;
      arr[ix] = curval;
    }
  va_end (args);
  return mom_boxnode_make_meta (conn, size,
                                (const struct mom_hashedvalue_st **) arr,
                                metaitm, metarank);
}


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
  struct mom_hashedvalue_st *smallarr[16] = { NULL };
  struct mom_hashedvalue_st **arr
    = (siz < sizeof (smallarr) / sizeof (smallarr[0])) ? smallarr
    : mom_gc_alloc (siz * sizeof (void *));
  va_start (args, conn);
  for (unsigned ix = 0; ix < siz; ix++)
    {
      struct mom_hashedvalue_st *curval =
        va_arg (args, struct mom_hashedvalue_st *);
      if (curval == MOM_EMPTY_SLOT)
        curval = NULL;
      arr[ix] = curval;
    }
  va_end (args);
  return mom_boxnode_make_meta (conn, siz,
                                (const struct mom_hashedvalue_st **) arr,
                                metaitm, metarank);
}
