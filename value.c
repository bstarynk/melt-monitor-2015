// file value.c - managing values

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


const char *
mom_itype_str (const void *p)
{
  if (!p)
    return "*nil*";
  else if (p == MOM_EMPTY_SLOT)
    return "*emptyslot*";
  else
    {
      unsigned ty = ((const struct mom_anyvalue_st *) p)->va_itype;
      switch (ty)
        {
        case MOMITY_NONE:
          return "NONE";
        case MOMITY_BOXINT:
          return "BOXINT";
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
      return GC_STRDUP (tybuf);
    }
}                               /* end of mom_itype_str */

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
  struct mom_hashedvalue_st *smallarr[3 * MOM_NB_QUELEM];
  memset (smallarr, 0, sizeof (smallarr));
  const struct mom_hashedvalue_st **arr =
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
  else if isinf
    (x)
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
  switch (val->va_itype)
    {
    case MOMITY_BOXINT:
      fprintf (femit, "%lld_\n",
               (long long) ((struct mom_boxint_st *) val)->boxi_int);
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
        mom_output_utf8_encoded (femit, vstr->cstr, mom_raw_size (val));
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
mom_output_value (FILE *fout, long *plastnl,
                  int depth, const struct mom_hashedvalue_st *val)
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
  switch (val->va_itype)
    {
    case MOMITY_BOXINT:
      fprintf (fout, "%lld",
               (long long) ((struct mom_boxint_st *) val)->boxi_int);
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
                                 ((struct mom_boxstring_st *) val)->cstr,
                                 mom_size (val));
        fputc ('"', fout);
      }
      break;
    case MOMITY_ITEM:
      fputs (mom_item_cstring ((struct mom_item_st *) val), fout);
      break;
    case MOMITY_SET:
    case MOMITY_TUPLE:
      if (val->va_itype == MOMITY_SET)
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
            if (ix > 0 && ix % 5 == 0)
              fputc (' ', fout);
            if (ix > 0)
              fputc (' ', fout);
            fputs (mom_item_cstring (seq->seqitem[ix]), fout);
          }
        if (val->va_itype == MOMITY_SET)
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
              fprintf (fout, "/%s#%ld", mom_item_cstring (nod->nod_metaitem),
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
                  if (ix > 0 && ix % 5 == 0)
                    fputc (' ', fout);
                  if (ix > 0)
                    fputc (' ', fout);
                  mom_output_value (fout, &lastnl, depth + 1,
                                    nod->nod_sons[ix]);
                }
              fputc (')', fout);
            }
        }
        break;
    default:
        fprintf (fout, "<strange value@%p of type %d>", val, val->va_itype);
      }
      fflush (fout);
    }
  if (plastnl)
    *plastnl = lastnl;
#undef INDENTED_NEWLINE_MOM
}


const char *
mom_value_cstring (const struct mom_hashedvalue_st *val)
{
  if (!val || val == MOM_EMPTY_SLOT)
    return "~";
  if (val->va_itype == MOMITY_ITEM)
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
  char *res = GC_STRDUP (outbuf);
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
  if (val1->va_itype == 0 || val1->va_itype >= MOMITY__LASTHASHED)
    return false;
  if (val2->va_itype == 0 || val2->va_itype >= MOMITY__LASTHASHED)
    return false;
  if (val1->hva_hash != val2->hva_hash)
    return false;
  if (val1->va_itype != val2->va_itype)
    return false;
  switch (val1->va_itype)
    {
    case MOMITY_BOXINT:
      return ((const struct mom_boxint_st *) val1)->boxi_int ==
        ((const struct mom_boxint_st *) val2)->boxi_int;
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
      return !strcmp (((const struct mom_boxstring_st *) val1)->cstr,
                      ((const struct mom_boxstring_st *) val2)->cstr);
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
            if (nod1->nod_sons[ix] && nod2->nod_sons[ix]
                && nod1->nod_sons[ix]->hva_hash !=
                nod2->nod_sons[ix]->hva_hash)
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
  if (!val1 || val1->va_itype == 0 || val1->va_itype >= MOMITY__LASTHASHED)
    return -1;
  if (!val2 || val2->va_itype == 0 || val2->va_itype >= MOMITY__LASTHASHED)
    return 1;
  if (val1->va_itype < val2->va_itype)
    return -1;
  if (val1->va_itype > val2->va_itype)
    return 1;
  if (MOM_UNLIKELY
      (val1->hva_hash == val2->hva_hash
       && mom_hashedvalue_equal (val1, val2)))
    return 0;
  switch (val1->va_itype)
    {
    case MOMITY_BOXINT:
      {
        intptr_t i1 = ((const struct mom_boxint_st *) val1)->boxi_int;
        intptr_t i2 = ((const struct mom_boxint_st *) val2)->boxi_int;
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
        int c = strcmp (((const struct mom_boxstring_st *) val1)->cstr,
                        ((const struct mom_boxstring_st *) val2)->cstr);
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
  struct mom_hashedvalue_st *smallarr[16] = { };
  const struct mom_hashedvalue_st **arr =
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
#warning unimplemented momf_ldp_nodemeta
  MOM_FATAPRINTF ("unimplemented momf_ldp_nodemeta itm=%s",
                  mom_item_cstring (itm));
}                               /* end of momf_ldp_nodemeta */
