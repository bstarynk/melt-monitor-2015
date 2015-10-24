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
  bi->va_itype = MOMITY_BOXINT;
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
  bs->va_itype = MOMITY_BOXSTRING;
  bs->va_hsiz = l >> 16;
  bs->va_lsiz = l & 0xffff;
  bs->hva_hash = mom_cstring_hash_len (s, l);
  memcpy (bs->cstr, s, l);
  return bs;
}


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
    mom_gc_alloc_atomic (sizeof (struct mom_boxdouble_st));
  bd->va_itype = MOMITY_BOXDOUBLE;
  bd->hva_hash = mom_double_hash (x);
  bd->boxd_dbl = x;
  return bd;
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
  tup->va_itype = MOMITY_TUPLE;
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
  tup->va_itype = MOMITY_TUPLE;
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
  tup->va_itype = MOMITY_TUPLE;
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
  set->va_itype = MOMITY_SET;
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
      newset->va_itype = MOMITY_SET;
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
  assert (nod->va_itype == MOMITY_NODE);
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
  nod->va_itype = MOMITY_NODE;
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
      qe->qu_elems[0] = (struct mom_anyvalue_st *) data;
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
      qe->qu_elems[0] = (struct mom_anyvalue_st *) data;
      qe->qu_next = qu->qu_first;
      qu->qu_first = qe;
    }
  else
    {
      struct mom_anyvalue_st *qdata[MOM_NB_QUELEM];
      memset (qdata, 0, sizeof (qdata));
      qdata[0] = (struct mom_anyvalue_st *) data;
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
      qe->qu_elems[0] = (struct mom_anyvalue_st *) data;
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
      qe->qu_elems[0] = (struct mom_anyvalue_st *) data;
      qlast->qu_next = qe;
      qu->qu_last = qe;
    }
  else
    {
      struct mom_anyvalue_st *qdata[MOM_NB_QUELEM];
      memset (qdata, 0, sizeof (qdata));
      qdata[0] = (struct mom_anyvalue_st *) data;
      int cnt = 0;
      for (unsigned ix = 0; ix < MOM_NB_QUELEM; ix++)
        {
          assert (cnt < MOM_NB_QUELEM);
          if (qlast->qu_elems[ix])
            qdata[cnt++] = qlast->qu_elems[ix];
        };
      assert (cnt < MOM_NB_QUELEM - 1);
      qdata[cnt++] = (struct mom_anyvalue_st *) data;
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
  struct mom_anyvalue_st *qdata[MOM_NB_QUELEM];
  memset (qdata, 0, sizeof (qdata));
  int cnt = 0;
  for (unsigned ix = 0; ix < MOM_NB_QUELEM; ix++)
    if (qfirst->qu_elems[ix])
      qdata[cnt++] = qfirst->qu_elems[ix];
  if (cnt <= 1)
    {
      assert (cnt == 1);
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




struct mom_boxnode_st *
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
  struct mom_anyvalue_st *smallarr[3 * MOM_NB_QUELEM];
  memset (smallarr, 0, sizeof (smallarr));
  struct mom_anyvalue_st **arr =
    (siz < (sizeof (smallarr) / sizeof (smallarr[0]))) ? smallarr
    : mom_gc_alloc (siz * sizeof (void *));
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
mom_dumpscan_value (struct mom_dumper_st *du,
                    const struct mom_anyvalue_st *val)
{
  if (!val || val == MOM_EMPTY_SLOT)
    return;
  assert (du && du->va_itype == MOMITY_DUMPER);
  switch (val->va_itype)
    {
    case MOMITY_BOXINT:
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
        const struct mom_boxnode_st *nod = (const struct mom_node_st *) val;
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
