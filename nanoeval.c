// file nanoeval.c

/**   Copyright (C)  2016  Basile Starynkevitch and later the FSF
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



#define NANOEVAL_MAXDEPTH_MOM 256

static struct mom_item_st *
nanoeval_freshenv_mom (struct mom_item_st *parenvitm, unsigned sizhint,
                       struct mom_item_st *protoitm)
{
  assert (mom_itype (parenvitm) == MOMITY_ITEM);
  assert (mom_itype (protoitm) == MOMITY_ITEM);
  struct mom_item_st *newenvitm = mom_clone_item (protoitm);
  mom_item_lock (newenvitm);
  mom_unsync_item_put_phys_attr (newenvitm, MOM_PREDEFITM (parent),
                                 parenvitm);
  newenvitm->itm_payload = (void *)
    mom_hashmap_reserve (NULL, mom_prime_above (sizhint + 1));
  mom_item_unlock (newenvitm);
  MOM_DEBUGPRINTF (run, "nanoeval_freshenv parenvitm=%s newenvitm=%s",
                   mom_item_cstring (parenvitm),
                   mom_item_cstring (newenvitm));
  return newenvitm;
}                               /* end of nanoeval_freshenv_mom */

static void
nanoeval_bind_mom (struct mom_item_st *envitm, struct mom_item_st *varitm,
                   const void *val)
{
  assert (mom_itype (envitm) == MOMITY_ITEM);
  assert (mom_itype (varitm) == MOMITY_ITEM);
  mom_item_lock (envitm);
  envitm->itm_payload =
    (void *) mom_hashmap_put ((void *) envitm->itm_payload, varitm, val);
  mom_item_unlock (envitm);
  MOM_DEBUGPRINTF (run, "nanoeval_bind envitm=%s varitm=%s val=%s",
                   mom_item_cstring (envitm), mom_item_cstring (varitm),
                   mom_value_cstring (val));
}                               /* end of nanoeval_bind_mom */

static void *
nanoeval_displaynode_mom (struct mom_nanoeval_st *nev,
                          struct mom_item_st *envitm,
                          const struct mom_boxnode_st *nod, int depth)
{
  MOM_DEBUGPRINTF (run,
                   "nanoeval_displaynode start envitm=%s nod=%s depth#%d",
                   mom_item_cstring (envitm),
                   mom_value_cstring ((struct mom_hashedvalue_st *) nod),
                   depth);
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  assert (nod && nod->va_itype == MOMITY_NODE);
  unsigned arity = mom_size (nod);
  struct mom_hashset_st *hset = mom_hashset_reserve (NULL, 4 * arity / 3 + 6);
  for (unsigned ix = 0; ix < arity; ix++)
    {
      const struct mom_hashedvalue_st *curexprv = nod->nod_sons[ix];
      MOM_DEBUGPRINTF (run,
                       "nanoeval_displaynode ix=%d depth=%d curexprv=%s",
                       ix, depth, mom_value_cstring (curexprv));
      const void *curdispv = mom_nanoeval (nev, envitm, curexprv, depth + 1);
      MOM_DEBUGPRINTF (run,
                       "nanoeval_displaynode ix=%d depth=%d curdispv=%s",
                       ix, depth, mom_value_cstring (curdispv));
      struct mom_item_st *dispitm = NULL;
      const struct mom_boxset_st *dispset = NULL;
      if ((dispset = mom_dyncast_set (curdispv)) != NULL)
        {
          unsigned nbdisp = mom_raw_size (dispset);
          for (unsigned elix = 0; elix < nbdisp; elix++)
            {
              dispitm = dispset->seqitem[elix];
              assert (dispitm && dispitm->va_itype == MOMITY_ITEM);
              hset = mom_hashset_insert (hset, dispitm);
              MOM_DEBUGPRINTF (run,
                               "nanoeval_displaynode ix=%d elix=%d dispitm=%s",
                               ix, elix, mom_item_cstring (dispitm));
            }
        }
      else if ((dispitm = mom_dyncast_item (curdispv)) != NULL)
        {
          hset = mom_hashset_insert (hset, dispitm);
          MOM_DEBUGPRINTF (run,
                           "nanoeval_displaynode ix=%d dispitm=%s",
                           ix, mom_item_cstring (dispitm));
        }
      else if ((dispitm = mom_dyncast_item (curexprv)) != NULL)
        {
          hset = mom_hashset_insert (hset, dispitm);
          MOM_DEBUGPRINTF (run,
                           "nanoeval_displaynode ix=%d expr dispitm=%s",
                           ix, mom_item_cstring (dispitm));
        }
      else
        {
          MOM_WARNPRINTF
            ("display expression#%d %s has unexpected value %s", ix,
             mom_value_cstring (curexprv), mom_value_cstring (curdispv));
          continue;
        }
    }
  const struct mom_hashedvalue_st *oldispitemv =
    mom_unsync_item_get_phys_attr (nev->nanev_thistatitm,
                                   MOM_PREDEFITM (item));
  const struct mom_boxset_st *oldispset = mom_dyncast_set (oldispitemv);
  MOM_DEBUGPRINTF (run, "nanoeval_displaynode oldispitemv=%s",
                   mom_value_cstring (oldispitemv));
  unsigned oldsiz = mom_size (oldispset);
  for (unsigned oix = 0; oix < oldsiz; oix++)
    {
      struct mom_item_st *olditm = oldispset->seqitem[oix];
      assert (olditm && olditm->va_itype == MOMITY_ITEM);
      hset = mom_hashset_insert (hset, olditm);
    }
  const struct mom_boxset_st *dispset = mom_hashset_to_boxset (hset);
  MOM_DEBUGPRINTF (run, "nanoeval_displaynode new dispset=%s",
                   mom_value_cstring ((struct mom_hashedvalue_st *) dispset));
  mom_unsync_item_put_phys_attr (nev->nanev_thistatitm,
                                 MOM_PREDEFITM (item),
                                 (struct mom_hashedvalue_st *) dispset);
  return (void *) dispset;
}                               /* end of nanoeval_displaynode_mom */


static void *
nanoeval_getnode_mom (struct mom_nanoeval_st *nev, struct mom_item_st *envitm,
                      const struct mom_boxnode_st *nod, int depth)
{
  const void *resv = NULL;
  const struct mom_seqitems_st *seq = NULL;
  const struct mom_boxnode_st *firstnod = NULL;
  MOM_DEBUGPRINTF (run, "nanoeval_getnode start envitm=%s nod=%s depth#%d",
                   mom_item_cstring (envitm),
                   mom_value_cstring ((struct mom_hashedvalue_st *) nod),
                   depth);
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  assert (nod && nod->va_itype == MOMITY_NODE);
  unsigned arity = mom_size (nod);
  MOM_DEBUGPRINTF (run, "nanoeval_getnode node %s in envitm %s",
                   mom_value_cstring ((const struct mom_hashedvalue_st
                                       *) nod), mom_item_cstring (envitm));
  if (arity != 2 && arity != 3)
    NANOEVAL_FAILURE_MOM (nev, nod,
                          mom_boxnode_make_va (MOM_PREDEFITM (arity), 1,
                                               mom_boxint_make (arity)));
  const struct mom_hashedvalue_st *firstargv = nod->nod_sons[0];
  MOM_DEBUGPRINTF (run, "nanoeval_getnode first son %s depth %d",
                   mom_value_cstring (firstargv), depth);
  const void *firstv = mom_nanoeval (nev, envitm, firstargv, depth + 1);
  MOM_DEBUGPRINTF (run, "nanoeval_getnode first son value %s depth %d",
                   mom_value_cstring (firstv), depth);
  if (!firstv && mom_itype (firstargv) == MOMITY_ITEM)
    firstv = firstargv;
  const struct mom_hashedvalue_st *secondargv = nod->nod_sons[1];
  MOM_DEBUGPRINTF (run, "nanoeval_getnode second son %s depth %d",
                   mom_value_cstring (secondargv), depth);
  const void *secondv = mom_nanoeval (nev, envitm, secondargv, depth + 1);
  MOM_DEBUGPRINTF (run, "nanoeval_getnode first son value %s depth %d",
                   mom_value_cstring (secondv), depth);
  if (!secondv && mom_itype (secondargv) == MOMITY_ITEM)
    secondv = secondargv;
  if (mom_itype (firstargv) == MOMITY_ITEM)
    {
      struct mom_item_st *firstitm = mom_dyncast_item (firstargv);
      mom_item_lock (firstitm);
      if (mom_itype (secondargv) == MOMITY_ITEM)
        {
          struct mom_item_st *seconditm = mom_dyncast_item (secondargv);
          resv = mom_unsync_item_get_phys_attr (firstitm, seconditm);
          MOM_DEBUGPRINTF (run,
                           "nanoeval_getnode firstitm=%s seconditm=%s resv=%s",
                           mom_item_cstring (firstitm),
                           mom_item_cstring (seconditm),
                           mom_value_cstring (resv));
        }
      else if (mom_itype (secondargv) == MOMITY_BOXINT)
        {
          intptr_t rk = mom_boxint_val_def (secondargv, 0);
          resv = mom_vectvaldata_nth (firstitm->itm_pcomp, rk);
          MOM_DEBUGPRINTF (run, "nanoeval_getnode firstitm=%s rk=%ld resv=%s",
                           mom_item_cstring (firstitm), (long) rk,
                           mom_value_cstring (resv));
        }
      mom_item_unlock (firstitm);
    }
  else if ((seq = mom_dyncast_seqitems (firstargv)) != NULL
           && mom_itype (secondargv) == MOMITY_BOXINT)
    {
      intptr_t rk = mom_boxint_val_def (secondargv, 0);
      resv = mom_seqitems_nth (seq, rk);
      MOM_DEBUGPRINTF (run, "nanoeval_getnode firstseq %s rk=%ld resv=%s",
                       mom_value_cstring (firstargv), (long) rk,
                       mom_value_cstring (resv));
    }
  else if ((firstnod = mom_dyncast_node (firstargv)) != NULL
           && mom_itype (secondargv) == MOMITY_BOXINT)
    {
      intptr_t rk = mom_boxint_val_def (secondargv, 0);
      resv = mom_boxnode_nth (firstnod, rk);
      MOM_DEBUGPRINTF (run, "nanoeval_getnode firstnod %s rk=%ld resv=%s",
                       mom_value_cstring (firstargv), (long) rk,
                       mom_value_cstring (resv));
    }
  MOM_DEBUGPRINTF (run, "nanoeval_getnode resv %s depth %d",
                   mom_value_cstring (resv), depth);
  if ((!resv || resv == MOM_EMPTY_SLOT) && arity == 3)
    {
      const struct mom_hashedvalue_st *elseargv = nod->nod_sons[2];
      MOM_DEBUGPRINTF (run, "nanoeval_getnode else son %s depth %d",
                       mom_value_cstring (elseargv), depth);
      const void *elsev = mom_nanoeval (nev, envitm, elseargv, depth + 1);
      MOM_DEBUGPRINTF (run, "nanoeval_getnode else son value %s depth %d",
                       mom_value_cstring (elsev), depth);
      resv = elsev;
    }
  return (void *) resv;
}                               /* end of nanoeval_getnode_mom */


static const void *
nanoeval_condnode_mom (struct mom_nanoeval_st *nev,
                       struct mom_item_st *envitm,
                       const struct mom_boxnode_st *nod, int depth)
{
  const void *resv = NULL;
  MOM_DEBUGPRINTF (run, "nanoeval_condnode start envitm=%s nod=%s depth#%d",
                   mom_item_cstring (envitm),
                   mom_value_cstring ((struct mom_hashedvalue_st *) nod),
                   depth);
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  assert (nod && nod->va_itype == MOMITY_NODE);
  unsigned arity = mom_size (nod);
  MOM_DEBUGPRINTF (run, "nanoeval_condnode node %s in envitm %s",
                   mom_value_cstring ((const struct mom_hashedvalue_st
                                       *) nod), mom_item_cstring (envitm));
  for (int ix = 0; ix < (int) arity; ix++)
    {
      const struct mom_hashedvalue_st *curcondv = nod->nod_sons[ix];
      const struct mom_boxnode_st *condnod = mom_dyncast_node (curcondv);
      if (!condnod || condnod->nod_connitm != MOM_PREDEFITM (when))
        {
          resv = mom_nanoeval (nev, envitm, curcondv, depth + 1);
          break;
        }
      unsigned condarity = mom_size (condnod);
      if (!condarity)
        break;
      const struct mom_hashedvalue_st *curcondexpv = condnod->nod_sons[0];
      MOM_DEBUGPRINTF (run, "nanoeval_condnode ix#%d curcondexpv %s depth %d",
                       ix, mom_value_cstring (curcondexpv), depth);
      const void *curcondtest =
        mom_nanoeval (nev, envitm, curcondexpv, depth + 1);
      MOM_DEBUGPRINTF (run, "nanoeval_condnode ix#%d curcondtest %s depth %d",
                       ix, mom_value_cstring (curcondtest), depth);
      if (!curcondtest)
        continue;
      resv = (void *) curcondtest;
      for (int expix = 1; expix < (int) condarity; expix++)
        {
          const struct mom_hashedvalue_st *curexpv = condnod->nod_sons[expix];
          MOM_DEBUGPRINTF (run,
                           "nanoeval_condnode ix#%d expix#%d curexpv %s depth %d",
                           ix, expix, mom_value_cstring (curexpv), depth);
          resv = mom_nanoeval (nev, envitm, curexpv, depth + 1);
          MOM_DEBUGPRINTF (run, "nanoeval_condnode expix#%d res %s", expix,
                           mom_value_cstring (resv));
        }
      return resv;
    }
  return resv;
}                               /* end of nanoeval_condnode_mom */


static const void *
nanoeval_ornode_mom (struct mom_nanoeval_st *nev, struct mom_item_st *envitm,
                     const struct mom_boxnode_st *nod, int depth)
{
  const void *resv = NULL;
  MOM_DEBUGPRINTF (run, "nanoeval_ornode start envitm=%s nod=%s depth#%d",
                   mom_item_cstring (envitm),
                   mom_value_cstring ((struct mom_hashedvalue_st *) nod),
                   depth);
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  assert (nod && nod->va_itype == MOMITY_NODE);
  unsigned arity = mom_size (nod);
  MOM_DEBUGPRINTF (run, "nanoeval_ornode node %s in envitm %s",
                   mom_value_cstring ((const struct mom_hashedvalue_st
                                       *) nod), mom_item_cstring (envitm));
  resv = MOM_PREDEFITM (truth);
  for (int ix = 0; ix < (int) arity; ix++)
    {
      const struct mom_hashedvalue_st *curexpv = nod->nod_sons[ix];
      MOM_DEBUGPRINTF (run, "nanoeval_ornode ix#%d curexpv %s depth %d", ix,
                       mom_value_cstring (curexpv), depth);
      resv = mom_nanoeval (nev, envitm, curexpv, depth + 1);
      MOM_DEBUGPRINTF (run, "nanoeval_ornode ix#%d resv %s", ix,
                       mom_value_cstring (resv));
      if (resv)
        break;
    };
  return resv;
}                               /* end of nanoeval_ornode_mom */


static const void *
nanoeval_andnode_mom (struct mom_nanoeval_st *nev, struct mom_item_st *envitm,
                      const struct mom_boxnode_st *nod, int depth)
{
  const void *resv = NULL;
  MOM_DEBUGPRINTF (run, "nanoeval_andnode start envitm=%s nod=%s depth#%d",
                   mom_item_cstring (envitm),
                   mom_value_cstring ((struct mom_hashedvalue_st *) nod),
                   depth);
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  assert (nod && nod->va_itype == MOMITY_NODE);
  unsigned arity = mom_size (nod);
  MOM_DEBUGPRINTF (run, "nanoeval_andnode node %s in envitm %s",
                   mom_value_cstring ((const struct mom_hashedvalue_st
                                       *) nod), mom_item_cstring (envitm));
  for (int ix = 0; ix < (int) arity; ix++)
    {
      const struct mom_hashedvalue_st *curexpv = nod->nod_sons[ix];
      MOM_DEBUGPRINTF (run, "nanoeval_andnode ix#%d curexpv %s depth %d", ix,
                       mom_value_cstring (curexpv), depth);
      resv = mom_nanoeval (nev, envitm, curexpv, depth + 1);
      MOM_DEBUGPRINTF (run, "nanoeval_andnode ix#%d resv %s", ix,
                       mom_value_cstring (resv));
      if (!resv)
        break;
    };
  return resv;
}                               /* end of nanoeval_andnode_mom */

static const void *
nanoeval_verbatimnode_mom (struct mom_nanoeval_st *nev,
                           struct mom_item_st *envitm,
                           const struct mom_boxnode_st *nod, int depth)
{
  MOM_DEBUGPRINTF (run,
                   "nanoeval_verbatimnode start envitm=%s nod=%s depth#%d",
                   mom_item_cstring (envitm),
                   mom_value_cstring ((struct mom_hashedvalue_st *) nod),
                   depth);
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  assert (nod && nod->va_itype == MOMITY_NODE);
  unsigned arity = mom_size (nod);
  MOM_DEBUGPRINTF (run, "nanoeval_verbatimnode node %s in envitm %s",
                   mom_value_cstring ((const struct mom_hashedvalue_st
                                       *) nod), mom_item_cstring (envitm));
  if (arity != 1)
    NANOEVAL_FAILURE_MOM (nev, nod,
                          mom_boxnode_make_va (MOM_PREDEFITM (arity), 1,
                                               mom_boxint_make (arity)));
  return nod->nod_sons[0];
}


static const void *
nanoeval_whilenode_mom (struct mom_nanoeval_st *nev,
                        struct mom_item_st *envitm,
                        const struct mom_boxnode_st *nod, int depth)
{
  const void *resv = NULL;
  MOM_DEBUGPRINTF (run, "nanoeval_whilenode start envitm=%s nod=%s depth#%d",
                   mom_item_cstring (envitm),
                   mom_value_cstring ((struct mom_hashedvalue_st *) nod),
                   depth);
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  assert (nod && nod->va_itype == MOMITY_NODE);
  unsigned arity = mom_size (nod);;
  if (arity == 0)
    NANOEVAL_FAILURE_MOM (nev, nod,
                          mom_boxnode_make_va (MOM_PREDEFITM (arity), 1,
                                               mom_boxint_make (arity)));
  const struct mom_hashedvalue_st *testexpv = nod->nod_sons[0];
  const struct mom_hashedvalue_st *testv = NULL;
  do
    {
      MOM_DEBUGPRINTF (run, "nanoeval_whilenode before test depth=%d", depth);
      testv = mom_nanoeval (nev, envitm, testexpv, depth + 1);
      MOM_DEBUGPRINTF (run, "nanoeval_whilenode testv=%s depth=%d",
                       mom_value_cstring (testv), depth);
      if (testv == NULL)
        break;
      for (int ix = 1; ix < (int) arity; ix++)
        {
          const struct mom_hashedvalue_st *expv = nod->nod_sons[ix];
          resv = mom_nanoeval (nev, envitm, expv, depth + 1);
        }
    }
  while (testv != NULL);
  MOM_DEBUGPRINTF (run, "nanoeval_whilenode final resv=%s depth=%d",
                   mom_value_cstring (resv), depth);
  return resv;
}                               /* end nanoeval_whilenode_mom */



static void *
nanoeval_itemnode_mom (struct mom_nanoeval_st *nev,
                       struct mom_item_st *envitm,
                       const struct mom_boxnode_st *nod, int depth)
{
  void *resv = NULL;
  MOM_DEBUGPRINTF (run, "nanoeval_itemnode start envitm=%s nod=%s depth#%d",
                   mom_item_cstring (envitm),
                   mom_value_cstring ((struct mom_hashedvalue_st *) nod),
                   depth);
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  assert (nod && nod->va_itype == MOMITY_NODE);
  unsigned arity = mom_size (nod);;
  if (arity != 1)
    NANOEVAL_FAILURE_MOM (nev, nod,
                          mom_boxnode_make_va (MOM_PREDEFITM (arity), 1,
                                               mom_boxint_make (arity)));
  const struct mom_hashedvalue_st *argitv = nod->nod_sons[0];
  struct mom_item_st *argitm = mom_dyncast_item (argitv);
  if (argitm != NULL)
    {
      resv = mom_clone_item (argitm);
      MOM_DEBUGPRINTF (run, "nanoeval_itemnode cloned resv=%s",
                       mom_item_cstring (resv));
    }
  else
    {
      const struct mom_hashedvalue_st *val =
        mom_nanoeval (nev, envitm, argitv, depth + 1);
      argitm = mom_dyncast_item (val);
      if (!argitm)
        NANOEVAL_FAILURE_MOM (nev, nod,
                              mom_boxnode_make_va (MOM_PREDEFITM (item), 1,
                                                   val));
      resv = mom_clone_item (argitm);
      MOM_DEBUGPRINTF (run, "nanoeval_itemnode cloned resv=%s",
                       mom_item_cstring (resv));
    }
  return resv;
}                               /* end nanoeval_itemnode_mom */


static const void *
nanoeval_funcnode_mom (struct mom_nanoeval_st *nev,
                       struct mom_item_st *envitm,
                       const struct mom_boxnode_st *nod, int depth)
{                               /// %func(<formals>,<body>....)
  MOM_DEBUGPRINTF (run, "nanoeval_funcnode start envitm=%s nod=%s depth#%d",
                   mom_item_cstring (envitm),
                   mom_value_cstring ((struct mom_hashedvalue_st *) nod),
                   depth);
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  assert (nod && nod->va_itype == MOMITY_NODE);
  unsigned arity = mom_size (nod);
  if (arity < 2)
    NANOEVAL_FAILURE_MOM (nev, nod,
                          mom_boxnode_make_va (MOM_PREDEFITM
                                               (arity), 1,
                                               mom_boxint_make (arity)));
  const struct mom_hashedvalue_st *formalsv = nod->nod_sons[0];
  const struct mom_boxtuple_st *formaltup = NULL;
  const struct mom_boxnode_st *formalnod = NULL;
  if ((formaltup = mom_dyncast_tuple (formalsv)) != NULL)
    {
      const struct mom_hashedvalue_st **arr =
        mom_gc_alloc ((arity + 3) * sizeof (void *));
      arr[0] = (const struct mom_hashedvalue_st *) formalsv;
      arr[1] = (const struct mom_hashedvalue_st *) envitm;
      for (unsigned ix = 1; ix < arity; ix++)
        arr[ix + 1] = nod->nod_sons[ix];
      const struct mom_boxnode_st *resnod =
        mom_boxnode_make (MOM_PREDEFITM (func), arity + 1, arr);
      MOM_DEBUGPRINTF (run,
                       "nanoeval_funcnode fixed arity:%d resnod %s depth#%d",
                       arity,
                       mom_value_cstring ((struct mom_hashedvalue_st *)
                                          resnod), depth);
      return resnod;
    }
  else if ((formalnod = mom_dyncast_node (formalsv)) != NULL)
    {
      unsigned formalnb = mom_raw_size (formalnod);
      for (unsigned j = 0; j < formalnb; j++)
        {
          if (mom_dyncast_item (formalnod->nod_sons[j]) == NULL)
            NANOEVAL_FAILURE_MOM (nev, nod,
                                  mom_boxnode_make_va (MOM_PREDEFITM
                                                       (arg), 1, formalsv));
        }
      const struct mom_hashedvalue_st **arr =
        mom_gc_alloc ((arity + 3) * sizeof (void *));
      if (formalnod->nod_connitm == MOM_PREDEFITM (tuple))
        arr[0] =                //
          (const struct mom_hashedvalue_st *)   //
          mom_boxtuple_make_arr (formalnb,
                                 (const struct mom_item_st **)
                                 formalnod->nod_sons);
      else
        arr[0] = (const struct mom_hashedvalue_st *) formalsv;
      arr[1] = (const struct mom_hashedvalue_st *) envitm;
      for (unsigned ix = 1; ix < arity; ix++)
        arr[ix + 1] = nod->nod_sons[ix];
      const struct mom_boxnode_st *resnod =
        mom_boxnode_make (MOM_PREDEFITM (func), arity + 1, arr);
      MOM_DEBUGPRINTF (run,
                       "nanoeval_funcnode arity:%d resnod %s depth#%d",
                       arity,
                       mom_value_cstring ((struct mom_hashedvalue_st *)
                                          resnod), depth);
      return resnod;
    }
  else
    NANOEVAL_FAILURE_MOM (nev, nod,
                          mom_boxnode_make_va (MOM_PREDEFITM
                                               (undefined_result), 1,
                                               formalsv));
}                               /* end nanoeval_funcnode_mom */


const void *
mom_nanoapply (struct mom_nanoeval_st *nev,
               struct mom_item_st *envitm,
               const struct mom_boxnode_st *nodfun,
               const struct mom_boxnode_st *nodexp,
               unsigned nbargs, const void **argv, int depth)
{
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  struct mom_item_st *opitm = mom_boxnode_conn (nodexp);
  MOM_DEBUGPRINTF (run,
                   "nanoapply envitm=%s nodfun=%s nodexp=%s nbargs#%d depth#%d",
                   mom_item_cstring (envitm),
                   mom_value_cstring ((struct mom_hashedvalue_st *) nodfun),
                   mom_value_cstring ((struct mom_hashedvalue_st *) nodexp),
                   nbargs, depth);
  if (mom_size (nodfun) < 3 || nodfun->nod_connitm != MOM_PREDEFITM (func))
    NANOEVAL_FAILURE_MOM (nev, nodexp,
                          mom_boxnode_make_va (MOM_PREDEFITM (func), 1,
                                               nodfun));
  void *formalsv = nodfun->nod_sons[0];
  struct mom_item_st *clenvitm = mom_dyncast_item (nodfun->nod_sons[1]);
  if (!clenvitm)
    NANOEVAL_FAILURE_MOM (nev, nodexp,
                          mom_boxnode_make_va (MOM_PREDEFITM
                                               (undefined_result), 1,
                                               nodfun));
  if (!opitm)
    NANOEVAL_FAILURE_MOM (nev, nodexp,
                          mom_boxnode_make_va (MOM_PREDEFITM (expression), 1,
                                               nodexp));
  struct mom_item_st *newenvitm =
    nanoeval_freshenv_mom (clenvitm, mom_size (formalsv) + 1, opitm);
  MOM_DEBUGPRINTF (run, "nanoapply newenvitm=%s depth#%d",
                   mom_item_cstring (newenvitm), depth);

  struct mom_boxtuple_st *formtup = mom_dyncast_tuple (formalsv);

  if (formtup)
    {
      unsigned nbform = mom_size (formtup);
    }
  MOM_FATAPRINTF
    ("unimplemented mom_nanoapply nodfun=%s, nodexp=%s, depth#%d",
     mom_value_cstring ((const struct mom_hashedvalue_st *) nodfun),
     mom_value_cstring ((const struct mom_hashedvalue_st *) nodexp), depth);
#warning mom_nanoapply unimplemented
}                               /* end of mom_nanoapply */



static unsigned
nbbindings_nano_mom (const struct mom_boxnode_st *nod)
{
  assert (nod && nod->va_itype == MOMITY_NODE);
  unsigned arity = mom_size (nod);
  for (unsigned ix = 0; ix < arity; ix++)
    {
      const void *curson = nod->nod_sons[ix];
      const struct mom_boxnode_st *sonod = mom_dyncast_node (curson);
      if (!sonod)
        return ix;
      if (sonod->nod_connitm != MOM_PREDEFITM (be))
        return ix;
      if (mom_raw_size (sonod) < 2)
        return ix;
      if (mom_itype (sonod->nod_sons[0]) != MOMITY_ITEM)
        return ix;
    }
  return arity;
}                               /* end of nbbindings_nano_mom */



static const void *
nanoeval_letnode_mom (struct mom_nanoeval_st *nev,
                      struct mom_item_st *envitm,
                      const struct mom_boxnode_st *nod, int depth)
{                               /// %let(bindings... body....)
  void *res = NULL;
  MOM_DEBUGPRINTF (run, "nanoeval_letnode start envitm=%s nod=%s depth#%d",
                   mom_item_cstring (envitm),
                   mom_value_cstring ((struct mom_hashedvalue_st *) nod),
                   depth);
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  assert (nod && nod->va_itype == MOMITY_NODE);
  unsigned arity = mom_size (nod);
  unsigned nbbind = nbbindings_nano_mom (nod);
  struct mom_item_st *newenvitm =
    nanoeval_freshenv_mom (envitm, nbbind, nod->nod_connitm);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_letnode depth#%d arity=%d nbbind=%d newenvitm=%s",
                   depth, arity, nbbind, mom_item_cstring (newenvitm));
  assert (nbbind <= arity);
  for (unsigned ix = 0; ix < nbbind; ix++)
    {
      struct mom_boxnode_st *bindnod = mom_dyncast_node (nod->nod_sons[ix]);
      assert (bindnod != NULL);
      unsigned bindln = mom_size (bindnod);
      assert (bindnod->nod_connitm == MOM_PREDEFITM (be) && bindln >= 2);
      struct mom_item_st *varitm = mom_dyncast_item (bindnod->nod_sons[0]);
      assert (varitm != NULL && varitm->va_itype == MOMITY_ITEM);
      MOM_DEBUGPRINTF (run, "nanoeval_letnode depth#%d binding#%d varitm %s",
                       depth, ix, mom_item_cstring (varitm));
      const void *bvalv = NULL;
      for (unsigned bnix = 1; bnix < bindln; bnix++)
        {
          const void *bexpv = bindnod->nod_sons[bnix];
          MOM_DEBUGPRINTF (run,
                           "nanoeval_letnode depth#%d varitm %s bexpv %s",
                           depth, mom_item_cstring (varitm),
                           mom_value_cstring (bexpv));
          bvalv = mom_nanoeval (nev, newenvitm, bexpv, depth + 1);
          MOM_DEBUGPRINTF (run,
                           "nanoeval_letnode depth#%d varitm %s bvalv %s",
                           depth, mom_item_cstring (varitm),
                           mom_value_cstring (bvalv));
        }
      nanoeval_bind_mom (newenvitm, varitm, bvalv);
    }
  for (unsigned ix = nbbind; ix < arity; ix++)
    {
      void *subexpv = nod->nod_sons[ix];
      MOM_DEBUGPRINTF (run, "nanoeval_letnode depth#%d ix#%d subexpv %s",
                       depth, ix, mom_value_cstring (subexpv));
      void *subvalv = mom_nanoeval (nev, newenvitm, subexpv, depth + 1);
      MOM_DEBUGPRINTF (run, "nanoeval_letnode depth#%d ix#%d subvalv %s",
                       depth, ix, mom_value_cstring (subvalv));
      res = subvalv;
    }
  MOM_DEBUGPRINTF (run, "nanoeval_letnode depth#%d final res=%s", depth,
                   mom_value_cstring (res));
  return res;
}                               /* end nanoeval_letnode_mom */


static const void *
nanoeval_letrecnode_mom (struct mom_nanoeval_st *nev,
                         struct mom_item_st *envitm,
                         const struct mom_boxnode_st *nod, int depth)
{                               /// %letrec(bindings... body....)
  void *res = NULL;
  MOM_DEBUGPRINTF (run, "nanoeval_letrecnode start envitm=%s nod=%s depth#%d",
                   mom_item_cstring (envitm),
                   mom_value_cstring ((struct mom_hashedvalue_st *) nod),
                   depth);
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  assert (nod && nod->va_itype == MOMITY_NODE);
  unsigned arity = mom_size (nod);
  unsigned nbbind = nbbindings_nano_mom (nod);
  struct mom_item_st *newenvitm =
    nanoeval_freshenv_mom (envitm, nbbind, nod->nod_connitm);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_letrecnode depth#%d arity=%d nbbind=%d newenvitm=%s",
                   depth, arity, nbbind, mom_item_cstring (newenvitm));
  assert (nbbind <= arity);
  // first loop to temporarily bind the variables to themselves
  for (unsigned ix = 0; ix < nbbind; ix++)
    {
      struct mom_boxnode_st *bindnod = mom_dyncast_node (nod->nod_sons[ix]);
      assert (bindnod != NULL);
      unsigned bindln = mom_size (bindnod);
      assert (bindnod->nod_connitm == MOM_PREDEFITM (be) && bindln >= 2);
      struct mom_item_st *varitm = mom_dyncast_item (bindnod->nod_sons[0]);
      assert (varitm != NULL && varitm->va_itype == MOMITY_ITEM);
      MOM_DEBUGPRINTF (run,
                       "nanoeval_letrecnode depth#%d binding#%d first varitm %s",
                       depth, ix, mom_item_cstring (varitm));
      nanoeval_bind_mom (newenvitm, varitm, varitm);
    }
  for (unsigned ix = 0; ix < nbbind; ix++)
    {
      struct mom_boxnode_st *bindnod = mom_dyncast_node (nod->nod_sons[ix]);
      unsigned bindln = mom_size (bindnod);
      struct mom_item_st *varitm = mom_dyncast_item (bindnod->nod_sons[0]);
      MOM_DEBUGPRINTF (run,
                       "nanoeval_letrecnode depth#%d binding#%d varitm %s",
                       depth, ix, mom_item_cstring (varitm));
      const void *bvalv = NULL;
      for (unsigned bnix = 1; bnix < bindln; bnix++)
        {
          const void *bexpv = bindnod->nod_sons[bnix];
          MOM_DEBUGPRINTF (run,
                           "nanoeval_letrecnode depth#%d varitm %s bexpv %s",
                           depth, mom_item_cstring (varitm),
                           mom_value_cstring (bexpv));
          bvalv = mom_nanoeval (nev, newenvitm, bexpv, depth + 1);
          MOM_DEBUGPRINTF (run,
                           "nanoeval_letrecnode depth#%d varitm %s bvalv %s",
                           depth, mom_item_cstring (varitm),
                           mom_value_cstring (bvalv));
        }
      nanoeval_bind_mom (newenvitm, varitm, bvalv);
    }
  for (unsigned ix = nbbind; ix < arity; ix++)
    {
      void *subexpv = nod->nod_sons[ix];
      MOM_DEBUGPRINTF (run, "nanoeval_letrecnode depth#%d ix#%d subexpv %s",
                       depth, ix, mom_value_cstring (subexpv));
      void *subvalv = mom_nanoeval (nev, newenvitm, subexpv, depth + 1);
      MOM_DEBUGPRINTF (run, "nanoeval_letrecnode depth#%d ix#%d subvalv %s",
                       depth, ix, mom_value_cstring (subvalv));
      res = subvalv;
    }
  MOM_DEBUGPRINTF (run, "nanoeval_letrecnode depth#%d final res=%s", depth,
                   mom_value_cstring (res));
  return res;
}                               /* end nanoeval_letrecnode_mom */


static const void *
nanoeval_othernode_mom (struct mom_nanoeval_st *nev,
                        struct mom_item_st *envitm,
                        const struct mom_boxnode_st *nod, int depth)
{
  const void *resv = NULL;
  MOM_DEBUGPRINTF (run, "nanoeval_othernode start envitm=%s nod=%s depth#%d",
                   mom_item_cstring (envitm),
                   mom_value_cstring ((struct mom_hashedvalue_st *) nod),
                   depth);
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  assert (nod && nod->va_itype == MOMITY_NODE);
  unsigned arity = mom_size (nod);
  struct mom_item_st *connitm = nod->nod_connitm;
  const struct mom_hashedvalue_st *nanev = NULL;
  {
    mom_item_lock (connitm);
    nanev = mom_unsync_item_get_phys_attr (connitm, MOM_PREDEFITM (nanoeval));
    mom_item_unlock (connitm);
  }
  MOM_DEBUGPRINTF (run,
                   "nanoeval_othernode connitm %s with nanev %s depth#%d",
                   mom_item_cstring (connitm), mom_value_cstring (nanev),
                   depth);
  if (nanev && mom_itype (nanev) != MOMITY_NODE)
    MOM_WARNPRINTF
      ("nanoeval_othernode connitm %s with strange `nanoeval` %s -expecting a closure node- depth#%d",
       mom_item_cstring (connitm), mom_value_cstring (nanev), depth);
  const struct mom_boxnode_st *nanenod = mom_dyncast_node (nanev);
  if (nanenod)
    {
      struct mom_item_st *opitm = nanenod->nod_connitm;
      struct mom_item_st *opsigitm = NULL;
      void *opfun = NULL;
      assert (opitm && opitm->va_itype == MOMITY_ITEM);
      {
        mom_item_lock (opitm);
        opsigitm = opitm->itm_funsig;
        if (opsigitm)
          opfun = opitm->itm_funptr;
        mom_item_unlock (opitm);
      }
      MOM_DEBUGPRINTF (run,
                       "nanoeval_othernode connitm %s nanev %s opsigitm %s opfun@%p",
                       mom_item_cstring (connitm), mom_value_cstring (nanev),
                       mom_item_cstring (opsigitm), opfun);
      if (opsigitm)
        {
          assert (opfun != NULL);
#define NANOEVALSIG_MOM(Nam) momhashpredef_##Nam % 239: \
    if (opsigitm == MOM_PREDEFITM(Nam)) goto foundcase_##Nam; goto defaultcase; foundcase_##Nam
          switch (opsigitm->hva_hash % 239)
            {
            case NANOEVALSIG_MOM (signature_nanoeval0):
              {
                if (arity != 0)
                  NANOEVAL_FAILURE_MOM (nev, nod,
                                        mom_boxnode_make_va (MOM_PREDEFITM
                                                             (arity), 1,
                                                             mom_boxint_make
                                                             (arity)));
                mom_nanoeval0_sig_t *fun0 = (mom_nanoeval0_sig_t *) opfun;
                MOM_DEBUGPRINTF (run,
                                 "nanoeval_othernode opitm %s nanev %s optsigitm %s depth#%d",
                                 mom_item_cstring (opitm),
                                 mom_value_cstring (nanev),
                                 mom_item_cstring (opsigitm), depth);
                return (*fun0) (nev, envitm, depth, nod, nanenod);
              }
            case NANOEVALSIG_MOM (signature_nanoeval1):
              {
                if (arity != 1)
                  NANOEVAL_FAILURE_MOM (nev, nod,
                                        mom_boxnode_make_va (MOM_PREDEFITM
                                                             (arity), 1,
                                                             mom_boxint_make
                                                             (arity)));
                const struct mom_hashedvalue_st *exp0v = nod->nod_sons[0];
                MOM_DEBUGPRINTF (run,
                                 "nanoeval_othernode opitm %s nanev %s optsigitm %s, exp0v %s, depth#%d",
                                 mom_item_cstring (opitm),
                                 mom_value_cstring (nanev),
                                 mom_item_cstring (opsigitm),
                                 mom_value_cstring (exp0v), depth);
                const void *val0 =
                  mom_nanoeval (nev, envitm, exp0v, depth + 1);
                MOM_DEBUGPRINTF (run,
                                 "nanoeval_othernode nanev %s val0 %s depth#%d",
                                 mom_value_cstring (nanev),
                                 mom_value_cstring (val0), depth);
                mom_nanoeval1_sig_t *fun1 = (mom_nanoeval1_sig_t *) opfun;
                return (*fun1) (nev, envitm, depth, nod, nanenod, val0);
              }
            case NANOEVALSIG_MOM (signature_nanoeval2):
              {
                if (arity != 2)
                  NANOEVAL_FAILURE_MOM (nev, nod,
                                        mom_boxnode_make_va (MOM_PREDEFITM
                                                             (arity), 1,
                                                             mom_boxint_make
                                                             (arity)));
                const struct mom_hashedvalue_st *exp0v = nod->nod_sons[0];
                MOM_DEBUGPRINTF (run,
                                 "nanoeval_othernode opitm %s nanev %s optsigitm %s, exp0v %s, depth#%d",
                                 mom_item_cstring (opitm),
                                 mom_value_cstring (nanev),
                                 mom_item_cstring (opsigitm),
                                 mom_value_cstring (exp0v), depth);
                const void *val0 =
                  mom_nanoeval (nev, envitm, exp0v, depth + 1);
                MOM_DEBUGPRINTF (run,
                                 "nanoeval_othernode nanev %s val0 %s depth#%d",
                                 mom_value_cstring (nanev),
                                 mom_value_cstring (val0), depth);
                const struct mom_hashedvalue_st *exp1v = nod->nod_sons[1];
                MOM_DEBUGPRINTF (run,
                                 "nanoeval_othernode opitm %s nanev %s optsigitm %s, exp1v %s, depth#%d",
                                 mom_item_cstring (opitm),
                                 mom_value_cstring (nanev),
                                 mom_item_cstring (opsigitm),
                                 mom_value_cstring (exp1v), depth);
                const void *val1 =
                  mom_nanoeval (nev, envitm, exp1v, depth + 1);
                MOM_DEBUGPRINTF (run,
                                 "nanoeval_othernode nanev %s val1 %s depth#%d",
                                 mom_value_cstring (nanev),
                                 mom_value_cstring (val1), depth);
                mom_nanoeval2_sig_t *fun2 = (mom_nanoeval2_sig_t *) opfun;
                return (*fun2) (nev, envitm, depth, nod, nanenod, val0, val1);
              }
            case NANOEVALSIG_MOM (signature_nanoeval3):
              {
                if (arity != 3)
                  NANOEVAL_FAILURE_MOM (nev, nod,
                                        mom_boxnode_make_va (MOM_PREDEFITM
                                                             (arity), 1,
                                                             mom_boxint_make
                                                             (arity)));
                const struct mom_hashedvalue_st *exp0v = nod->nod_sons[0];
                MOM_DEBUGPRINTF (run,
                                 "nanoeval_othernode opitm %s nanev %s optsigitm %s, exp0v %s, depth#%d",
                                 mom_item_cstring (opitm),
                                 mom_value_cstring (nanev),
                                 mom_item_cstring (opsigitm),
                                 mom_value_cstring (exp0v), depth);
                const void *val0 =
                  mom_nanoeval (nev, envitm, exp0v, depth + 1);
                MOM_DEBUGPRINTF (run,
                                 "nanoeval_othernode nanev %s val0 %s depth#%d",
                                 mom_value_cstring (nanev),
                                 mom_value_cstring (val0), depth);
                const struct mom_hashedvalue_st *exp1v = nod->nod_sons[1];
                MOM_DEBUGPRINTF (run,
                                 "nanoeval_othernode opitm %s nanev %s optsigitm %s, exp1v %s, depth#%d",
                                 mom_item_cstring (opitm),
                                 mom_value_cstring (nanev),
                                 mom_item_cstring (opsigitm),
                                 mom_value_cstring (exp1v), depth);
                const void *val1 =
                  mom_nanoeval (nev, envitm, exp1v, depth + 1);
                MOM_DEBUGPRINTF (run,
                                 "nanoeval_othernode nanev %s val1 %s depth#%d",
                                 mom_value_cstring (nanev),
                                 mom_value_cstring (val1), depth);
                const struct mom_hashedvalue_st *exp2v = nod->nod_sons[2];
                MOM_DEBUGPRINTF (run,
                                 "nanoeval_othernode opitm %s nanev %s optsigitm %s, exp2v %s, depth#%d",
                                 mom_item_cstring (opitm),
                                 mom_value_cstring (nanev),
                                 mom_item_cstring (opsigitm),
                                 mom_value_cstring (exp2v), depth);
                const void *val2 =
                  mom_nanoeval (nev, envitm, exp2v, depth + 1);
                MOM_DEBUGPRINTF (run,
                                 "nanoeval_othernode nanev %s val2 %s depth#%d",
                                 mom_value_cstring (nanev),
                                 mom_value_cstring (val2), depth);
                mom_nanoeval3_sig_t *fun3 = (mom_nanoeval3_sig_t *) opfun;
                return (*fun3) (nev, envitm, depth, nanenod, nod, val0, val1,
                                val2);
              }
            case NANOEVALSIG_MOM (signature_nanoeval4):
              {
                if (arity != 4)
                  NANOEVAL_FAILURE_MOM (nev, nod,
                                        mom_boxnode_make_va (MOM_PREDEFITM
                                                             (arity), 1,
                                                             mom_boxint_make
                                                             (arity)));
                const struct mom_hashedvalue_st *exp0v = nod->nod_sons[0];
                MOM_DEBUGPRINTF (run,
                                 "nanoeval_othernode opitm %s nanev %s optsigitm %s, exp0v %s, depth#%d",
                                 mom_item_cstring (opitm),
                                 mom_value_cstring (nanev),
                                 mom_item_cstring (opsigitm),
                                 mom_value_cstring (exp0v), depth);
                const void *val0 =
                  mom_nanoeval (nev, envitm, exp0v, depth + 1);
                MOM_DEBUGPRINTF (run,
                                 "nanoeval_othernode nanev %s val0 %s depth#%d",
                                 mom_value_cstring (nanev),
                                 mom_value_cstring (val0), depth);
                const struct mom_hashedvalue_st *exp1v = nod->nod_sons[1];
                MOM_DEBUGPRINTF (run,
                                 "nanoeval_othernode opitm %s nanev %s optsigitm %s, exp1v %s, depth#%d",
                                 mom_item_cstring (opitm),
                                 mom_value_cstring (nanev),
                                 mom_item_cstring (opsigitm),
                                 mom_value_cstring (exp1v), depth);
                const void *val1 =
                  mom_nanoeval (nev, envitm, exp1v, depth + 1);
                MOM_DEBUGPRINTF (run,
                                 "nanoeval_othernode nanev %s val1 %s depth#%d",
                                 mom_value_cstring (nanev),
                                 mom_value_cstring (val1), depth);
                const struct mom_hashedvalue_st *exp2v = nod->nod_sons[2];
                MOM_DEBUGPRINTF (run,
                                 "nanoeval_othernode opitm %s nanev %s optsigitm %s, exp2v %s, depth#%d",
                                 mom_item_cstring (opitm),
                                 mom_value_cstring (nanev),
                                 mom_item_cstring (opsigitm),
                                 mom_value_cstring (exp2v), depth);
                const void *val2 =
                  mom_nanoeval (nev, envitm, exp2v, depth + 1);
                MOM_DEBUGPRINTF (run,
                                 "nanoeval_othernode nanev %s val2 %s depth#%d",
                                 mom_value_cstring (nanev),
                                 mom_value_cstring (val2), depth);
                const struct mom_hashedvalue_st *exp3v = nod->nod_sons[3];
                MOM_DEBUGPRINTF (run,
                                 "nanoeval_othernode opitm %s nanev %s optsigitm %s, exp3v %s, depth#%d",
                                 mom_item_cstring (opitm),
                                 mom_value_cstring (nanev),
                                 mom_item_cstring (opsigitm),
                                 mom_value_cstring (exp3v), depth);
                const void *val3 =
                  mom_nanoeval (nev, envitm, exp3v, depth + 1);
                MOM_DEBUGPRINTF (run,
                                 "nanoeval_othernode nanev %s val3 %s depth#%d",
                                 mom_value_cstring (nanev),
                                 mom_value_cstring (val3), depth);
                mom_nanoeval4_sig_t *fun4 = (mom_nanoeval4_sig_t *) opfun;
                return (*fun4) (nev, envitm, depth, nod, nanenod, val0, val1,
                                val2, val3);
              }
            case NANOEVALSIG_MOM (signature_nanoevalany):
              {
                void *smallarr[16] = { 0 };
                void **arrv =
                  (arity <
                   sizeof (smallarr) /
                   sizeof (smallarr[0])) ? smallarr : mom_gc_alloc ((arity +
                                                                     1) *
                                                                    sizeof
                                                                    (void *));
                MOM_DEBUGPRINTF (run,
                                 "nanoeval_othernode opitm %s nanev %s optsigitm %s arity=%d depth#%d",
                                 mom_item_cstring (opitm),
                                 mom_value_cstring (nanev),
                                 mom_item_cstring (opsigitm), arity, depth);
                for (int ix = 0; ix < (int) arity; ix++)
                  {
                    const struct mom_hashedvalue_st *curexpv =
                      nod->nod_sons[ix];
                    MOM_DEBUGPRINTF (run,
                                     "nanoeval_othernode opitm %s ix#%d curexpv=%s depth#%d",
                                     mom_item_cstring (opitm), ix,
                                     mom_value_cstring (curexpv), depth);
                    arrv[ix] = mom_nanoeval (nev, envitm, curexpv, depth + 1);
                    MOM_DEBUGPRINTF (run,
                                     "nanoeval_othernode opitm %s ix#%d curvalue %s",
                                     mom_item_cstring (opitm), ix,
                                     mom_value_cstring (arrv[ix]));
                  }
                mom_nanoevalany_sig_t *funany =
                  (mom_nanoevalany_sig_t *) opfun;
                return (*funany) (nev, envitm, depth, nod, nanenod, arity,
                                  arrv);
              }
            default:
            defaultcase:
              MOM_WARNPRINTF
                ("nanoeval_othernode connitm %s opitm %s with strange signature %s",
                 mom_item_cstring (connitm), mom_item_cstring (opitm),
                 mom_value_cstring (opsigitm));
              NANOEVAL_FAILURE_MOM (nev, nod,
                                    mom_boxnode_make_va (MOM_PREDEFITM
                                                         (signature), 1,
                                                         opitm));
              ;
            }
        }
      else
        {                       // opsigitm is NULL
          NANOEVAL_FAILURE_MOM
            (nev, nod,
             mom_boxnode_make_va (MOM_PREDEFITM (signature), 1, opitm));
        }
    }
  else
    {                           // nanenod is null, so nanev is not a node
      // and the nanoeval slot of the connective is missing or useless
      const void *connval = mom_nanoeval (nev, envitm, connitm, depth + 1);
      MOM_DEBUGPRINTF (run,
                       "nanoeval_othernode connitm %s connval %s depth#%d",
                       mom_item_cstring (connitm),
                       mom_value_cstring (connval), depth);
      const struct mom_boxnode_st *connvnod = mom_dyncast_node (connval);
      unsigned connsiz = 0;
      if (connvnod && connvnod->nod_connitm == MOM_PREDEFITM (func)
          && (connsiz = mom_raw_size (connvnod)) >= 2)
        {
          void **arr = mom_gc_alloc ((arity + 1) * sizeof (void *));
          for (unsigned aix = 0; aix < arity; aix++)
            {
              const void *curexp = nod->nod_sons[aix];
              MOM_DEBUGPRINTF (run,
                               "nanoeval_othernode connitm %s exp#%d %s depth#%d",
                               mom_item_cstring (connitm),
                               aix, mom_value_cstring (curexp), depth);
              const void *curval =
                mom_nanoeval (nev, envitm, curexp, depth + 1);
              arr[aix] = curval;
            }
          MOM_DEBUGPRINTF (run,
                           "nanoeval_othernode before apply connval %s depth#%d",
                           mom_value_cstring (connval), depth);
          const void *resapp =
            mom_nanoapply (nev, envitm, connvnod, nod, arity,
                           (const void **) arr, depth + 1);
          MOM_DEBUGPRINTF (run,
                           "nanoeval_othernode after apply connval %s resapp %s depth#%d",
                           mom_value_cstring (connval),
                           mom_value_cstring (resapp), depth);
          return resapp;
        };
    }
  NANOEVAL_FAILURE_MOM (nev, nod,
                        mom_boxnode_make_va (MOM_PREDEFITM (nanoeval), 1,
                                             connitm));
}                               /* end of nanoeval_othernode_mom */




static const void *
nanoeval_node_mom (struct mom_nanoeval_st *nev, struct mom_item_st *envitm,
                   const struct mom_boxnode_st *nod, int depth)
{
  MOM_DEBUGPRINTF (run, "nanoeval_node start envitm=%s nod=%s depth#%d",
                   mom_item_cstring (envitm),
                   mom_value_cstring ((struct mom_hashedvalue_st *) nod),
                   depth);
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  assert (nod && nod->va_itype == MOMITY_NODE);
  struct mom_item_st *opitm = nod->nod_connitm;
  MOM_DEBUGPRINTF (run, "nanoeval_nod opitm=%s", mom_item_cstring (opitm));
  assert (opitm && opitm->va_itype == MOMITY_ITEM);
  switch (opitm->hva_hash % 373)
    {
#define OPITM_NANOEVALNODE_MOM(Nam) momhashpredef_##Nam % 373: \
    if (opitm == MOM_PREDEFITM(Nam)) goto foundcase_##Nam; goto defaultcase; foundcase_##Nam
      //////
    case OPITM_NANOEVALNODE_MOM (display):     ///// %display(<items...>)
      return nanoeval_displaynode_mom (nev, envitm, nod, depth);
    case OPITM_NANOEVALNODE_MOM (get): ///// %get(<item>,<itemattr>[,<default>]) or %get(<seq>,<rank>[,<default>])
      return nanoeval_getnode_mom (nev, envitm, nod, depth);
    case OPITM_NANOEVALNODE_MOM (cond):        ///// %cond(<when...>)
      return nanoeval_condnode_mom (nev, envitm, nod, depth);
    case OPITM_NANOEVALNODE_MOM (or):  ///// %or()
      return nanoeval_ornode_mom (nev, envitm, nod, depth);
    case OPITM_NANOEVALNODE_MOM (and): ///// %and()
      return nanoeval_andnode_mom (nev, envitm, nod, depth);
    case OPITM_NANOEVALNODE_MOM (verbatim):    ///// %verbatim(it)
      return nanoeval_verbatimnode_mom (nev, envitm, nod, depth);
    case OPITM_NANOEVALNODE_MOM (item):        ///// %item(it)
      return nanoeval_itemnode_mom (nev, envitm, nod, depth);
      case OPITM_NANOEVALNODE_MOM (while)
    :                          ///// %while(<cond>,...)
        return nanoeval_whilenode_mom (nev, envitm, nod, depth);
    case OPITM_NANOEVALNODE_MOM (func):        ///// %item(it)
      return nanoeval_funcnode_mom (nev, envitm, nod, depth);
    case OPITM_NANOEVALNODE_MOM (let): ///// %let(bindings... exprs...)
      return nanoeval_letnode_mom (nev, envitm, nod, depth);
    case OPITM_NANOEVALNODE_MOM (letrec):      ///// %letrec(bindings... exprs...)
      return nanoeval_letrecnode_mom (nev, envitm, nod, depth);
    defaultcase:
    default:
      return nanoeval_othernode_mom (nev, envitm, nod, depth);
    }
  MOM_FATAPRINTF ("unimplemented eval of node %s",
                  mom_value_cstring ((struct mom_hashedvalue_st *) nod));
}                               /* end of nanoeval_node_mom */



static const void *
nanoeval_item_mom (struct mom_nanoeval_st *nev, struct mom_item_st *envitm,
                   struct mom_item_st *itm, int depth)
{
  const void *res = NULL;
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  assert (itm && itm->va_itype == MOMITY_ITEM);
  assert (depth >= 0);
  while (envitm != NULL)
    {
      struct mom_item_st *prevenvitm = NULL;
      assert (envitm->va_itype == MOMITY_ITEM);
      mom_item_lock (envitm);
      prevenvitm =
        mom_dyncast_item (mom_unsync_item_get_phys_attr
                          (envitm, MOM_PREDEFITM (parent)));
      if (mom_itype (envitm->itm_payload) == MOMITY_HASHMAP)
        {
          res =
            mom_hashmap_get ((struct mom_hashmap_st *) envitm->itm_payload,
                             itm);
          if (res)
            prevenvitm = NULL;
        }
      mom_item_unlock (envitm);
      envitm = prevenvitm;
    }
  if (!res)
    res = itm;
  return res;
}                               /* end nanoeval_item_mom */


const void *
mom_nanoeval (struct mom_nanoeval_st *nev, struct mom_item_st *envitm,
              const void *exprv, int depth)
{
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  if (!exprv)
    return NULL;
  unsigned typexpr = mom_itype (exprv);
  long stepcount = nev->nanev_count;
  switch (typexpr)
    {
    case MOMITY_NODE:
      {
        if (stepcount >= nev->nanev_maxstep)
          NANOEVAL_FAILURE_MOM (nev, exprv,
                                mom_boxnode_make_va (MOM_PREDEFITM (step), 1,
                                                     mom_boxint_make
                                                     (stepcount)));
        nev->nanev_count = stepcount + 1;
        if (depth >= NANOEVAL_MAGIC_MOM)
          NANOEVAL_FAILURE_MOM (nev, exprv,
                                mom_boxnode_make_va (MOM_PREDEFITM (depth), 1,
                                                     mom_boxint_make
                                                     (depth)));
        return nanoeval_node_mom (nev, envitm,
                                  (const struct mom_boxnode_st *) exprv,
                                  depth);
      }
      break;
    case MOMITY_ITEM:
      {
        return nanoeval_item_mom (nev, envitm,
                                  (struct mom_item_st *) exprv, depth);
      }
      break;
    default:
      return exprv;
    }
}                               /* end of mom_nanoeval */


const char momsig_nanoeval_type1[] = "signature_nanoeval1";
const void *
momf_nanoeval_type1 (struct mom_nanoeval_st *nev,
                     struct mom_item_st *envitm,
                     int depth,
                     const struct mom_boxnode_st *expnod,
                     const struct mom_boxnode_st *closnod, const void *arg0)
{
  MOM_DEBUGPRINTF (run,
                   "nanoeval_type1 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0));
  unsigned ty0 = mom_itype (arg0);
  switch (ty0)
    {
    case MOMITY_NONE:
      return MOM_PREDEFITM (truth);
    case MOMITY_BOXINT:
      return MOM_PREDEFITM (int);
    case MOMITY_BOXDOUBLE:
      return MOM_PREDEFITM (double);
    case MOMITY_ITEM:
      return MOM_PREDEFITM (item);
    case MOMITY_TUPLE:
      return MOM_PREDEFITM (tuple);
    case MOMITY_SET:
      return MOM_PREDEFITM (set);
    case MOMITY_NODE:
      return MOM_PREDEFITM (node);
    }
  MOM_FATAPRINTF ("nanoeval_type1 unexpected ty0#%d", ty0);
}                               /* end of momf_nanoeval_type1 */



const char momsig_nanoeval_identity1[] = "signature_nanoeval1";
const void *
momf_nanoeval_identity1 (struct mom_nanoeval_st *nev,
                         struct mom_item_st *envitm,
                         int depth,
                         const struct mom_boxnode_st *expnod,
                         const struct mom_boxnode_st *closnod,
                         const void *arg0)
{
  MOM_DEBUGPRINTF (run,
                   "nanoeval_identity1 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0));
  return arg0;
}                               /* end of momf_nanoeval_identity1 */


const char momsig_nanoeval_add2[] = "signature_nanoeval2";
const void *
momf_nanoeval_add2 (struct mom_nanoeval_st *nev,
                    struct mom_item_st *envitm,
                    int depth,
                    const struct mom_boxnode_st *expnod,
                    const struct mom_boxnode_st *closnod,
                    const void *arg0, const void *arg1)
{
  unsigned ty0 = mom_itype (arg0);
  unsigned ty1 = mom_itype (arg1);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_add2 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s arg1=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0), mom_value_cstring (arg1));
  if (ty0 == MOMITY_BOXINT && ty1 == MOMITY_BOXINT)
    {
      intptr_t i0 = mom_boxint_val_def (arg0, -1);
      intptr_t i1 = mom_boxint_val_def (arg1, -1);
      intptr_t ires = i0 + i1;
      MOM_DEBUGPRINTF (run, "nanoeval_add2 i0=%ld i1=%ld ires=%ld", (long) i0,
                       (long) i1, (long) ires);
      return mom_boxint_make (ires);
    }
  else if (ty0 == MOMITY_BOXDOUBLE && ty1 == MOMITY_BOXDOUBLE)
    {
      double d0 = mom_boxdouble_val_def (arg0, 0.0);
      double d1 = mom_boxdouble_val_def (arg1, 0.0);
      double dres = d0 + d1;
      MOM_DEBUGPRINTF (run, "nanoeval_add2 d0=%g d1=%g dres=%g",
                       d0, d1, dres);
      return mom_boxdouble_make (dres);
    }
  else if (ty0 == MOMITY_BOXSTRING && ty1 == MOMITY_BOXSTRING)
    {
      const char *s0 = mom_boxstring_cstr (arg0);
      const char *s1 = mom_boxstring_cstr (arg1);
      const struct mom_boxstring_st *bstres =
        mom_boxstring_printf ("%s%s", s0, s1);
      MOM_DEBUGPRINTF (run, "nanoeval_add2 s0=%s d1=%s bstres=%s",
                       mom_value_cstring ((struct mom_hashedvalue_st *) arg0),
                       mom_value_cstring ((struct mom_hashedvalue_st *) arg1),
                       mom_value_cstring ((struct mom_hashedvalue_st *)
                                          bstres));
      return bstres;
    }
  else if (ty0 == MOMITY_TUPLE && ty1 == MOMITY_TUPLE)
    {
      const struct mom_boxtuple_st *tu0 = mom_dyncast_tuple (arg0);
      const struct mom_boxtuple_st *tu1 = mom_dyncast_tuple (arg1);
      unsigned ln0 = mom_raw_size (tu0);
      unsigned ln1 = mom_raw_size (tu1);
      const struct mom_boxtuple_st *tures
        = mom_boxtuple_make_arr2 (ln0, tu0->seqitem, ln1, tu1->seqitem);
      MOM_DEBUGPRINTF (run, "nanoeval_add2 tu0=%s tu1=%s tures=%s",
                       mom_value_cstring ((struct mom_hashedvalue_st *) tu0),
                       mom_value_cstring ((struct mom_hashedvalue_st *) tu1),
                       mom_value_cstring ((struct mom_hashedvalue_st *)
                                          tures));
      return tures;
    }
  else if (ty0 == MOMITY_SET && ty1 == MOMITY_SET)
    {
      const struct mom_boxset_st *set0 = mom_dyncast_set (arg0);
      const struct mom_boxset_st *set1 = mom_dyncast_set (arg1);
      const struct mom_boxset_st *sres = mom_boxset_union (set0, set1);
      MOM_DEBUGPRINTF (run, "nanoeval_add2 set0=%s set1=%s sres=%s",
                       mom_value_cstring ((struct mom_hashedvalue_st *) set0),
                       mom_value_cstring ((struct mom_hashedvalue_st *) set1),
                       mom_value_cstring ((struct mom_hashedvalue_st *)
                                          sres));
      return sres;
    }
  else
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 2,
                                               arg0, arg1));
}                               /* end momf_nanoeval_add2 */




const char momsig_nanoeval_mult2[] = "signature_nanoeval2";
const void *
momf_nanoeval_mult2 (struct mom_nanoeval_st *nev,
                     struct mom_item_st *envitm,
                     int depth,
                     const struct mom_boxnode_st *expnod,
                     const struct mom_boxnode_st *closnod,
                     const void *arg0, const void *arg1)
{
  unsigned ty0 = mom_itype (arg0);
  unsigned ty1 = mom_itype (arg1);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_mult2 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s arg1=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0), mom_value_cstring (arg1));
  if (ty0 == MOMITY_BOXINT && ty1 == MOMITY_BOXINT)
    {
      intptr_t i0 = mom_boxint_val_def (arg0, -1);
      intptr_t i1 = mom_boxint_val_def (arg1, -1);
      intptr_t ires = i0 * i1;
      MOM_DEBUGPRINTF (run, "nanoeval_mult2 i0=%ld i1=%ld ires=%ld",
                       (long) i0, (long) i1, (long) ires);
      return mom_boxint_make (ires);
    }
  else if (ty0 == MOMITY_BOXDOUBLE && ty1 == MOMITY_BOXDOUBLE)
    {
      double d0 = mom_boxdouble_val_def (arg0, 0.0);
      double d1 = mom_boxdouble_val_def (arg1, 0.0);
      double dres = d0 * d1;
      MOM_DEBUGPRINTF (run, "nanoeval_mult2 d0=%g d1=%g dres=%g",
                       d0, d1, dres);
      return mom_boxdouble_make (dres);
    }
  else if (ty0 == MOMITY_SET && ty1 == MOMITY_SET)
    {
      const struct mom_boxset_st *set0 = mom_dyncast_set (arg0);
      const struct mom_boxset_st *set1 = mom_dyncast_set (arg1);
      unsigned ln0 = mom_raw_size (set0);
      unsigned ln1 = mom_raw_size (set1);
      const struct mom_boxtuple_st *sres =
        mom_boxset_intersection (set0, set1);
      MOM_DEBUGPRINTF (run, "nanoeval_mult2 set0=%s set1=%s sres=%s",
                       mom_value_cstring ((struct mom_hashedvalue_st *) set0),
                       mom_value_cstring ((struct mom_hashedvalue_st *) set1),
                       mom_value_cstring ((struct mom_hashedvalue_st *)
                                          sres));
      return sres;
    }
  else
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 2,
                                               arg0, arg1));
}                               /* end momf_nanoeval_mult2 */



const char momsig_nanoeval_diff2[] = "signature_nanoeval2";
const void *
momf_nanoeval_diff2 (struct mom_nanoeval_st *nev,
                     struct mom_item_st *envitm,
                     int depth,
                     const struct mom_boxnode_st *expnod,
                     const struct mom_boxnode_st *closnod,
                     const void *arg0, const void *arg1)
{
  unsigned ty0 = mom_itype (arg0);
  unsigned ty1 = mom_itype (arg1);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_diff2 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s arg1=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0), mom_value_cstring (arg1));
  if (ty0 == MOMITY_BOXINT && ty1 == MOMITY_BOXINT)
    {
      intptr_t i0 = mom_boxint_val_def (arg0, -1);
      intptr_t i1 = mom_boxint_val_def (arg1, -1);
      intptr_t ires = i0 - i1;
      MOM_DEBUGPRINTF (run, "nanoeval_diff2 i0=%ld i1=%ld ires=%ld",
                       (long) i0, (long) i1, (long) ires);
      return mom_boxint_make (ires);
    }
  else if (ty0 == MOMITY_BOXDOUBLE && ty1 == MOMITY_BOXDOUBLE)
    {
      double d0 = mom_boxdouble_val_def (arg0, 0.0);
      double d1 = mom_boxdouble_val_def (arg1, 0.0);
      double dres = d0 - d1;
      MOM_DEBUGPRINTF (run, "nanoeval_diff2 d0=%g d1=%g dres=%g",
                       d0, d1, dres);
      return mom_boxdouble_make (dres);
    }
  else if (ty0 == MOMITY_SET && ty1 == MOMITY_SET)
    {
      const struct mom_boxset_st *set0 = mom_dyncast_set (arg0);
      const struct mom_boxset_st *set1 = mom_dyncast_set (arg1);
      unsigned ln0 = mom_raw_size (set0);
      unsigned ln1 = mom_raw_size (set1);
      const struct mom_boxtuple_st *sres = mom_boxset_difference (set0, set1);
      MOM_DEBUGPRINTF (run, "nanoeval_diff2 set0=%s set1=%s sres=%s",
                       mom_value_cstring ((struct mom_hashedvalue_st *) set0),
                       mom_value_cstring ((struct mom_hashedvalue_st *) set1),
                       mom_value_cstring ((struct mom_hashedvalue_st *)
                                          sres));
      return sres;
    }
  else
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 2,
                                               arg0, arg1));
}                               /* end momf_nanoeval_diff2 */

const char momsig_nanoeval_div2[] = "signature_nanoeval2";
const void *
momf_nanoeval_div2 (struct mom_nanoeval_st *nev,
                    struct mom_item_st *envitm,
                    int depth,
                    const struct mom_boxnode_st *expnod,
                    const struct mom_boxnode_st *closnod,
                    const void *arg0, const void *arg1)
{
  unsigned ty0 = mom_itype (arg0);
  unsigned ty1 = mom_itype (arg1);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_div2 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s arg1=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0), mom_value_cstring (arg1));
  if (ty0 == MOMITY_BOXINT && ty1 == MOMITY_BOXINT)
    {
      intptr_t i0 = mom_boxint_val_def (arg0, -1);
      intptr_t i1 = mom_boxint_val_def (arg1, -1);
      if (i1 == 0)
        {
          MOM_DEBUGPRINTF (run,
                           "nanoeval_div2 i0=%ld zerodivide undef expnod=%s",
                           i0,
                           mom_value_cstring ((struct mom_hashedvalue_st *)
                                              expnod));
          NANOEVAL_FAILURE_MOM (nev, expnod,
                                MOM_PREDEFITM (undefined_result));
        }
      intptr_t ires = i0 / i1;
      MOM_DEBUGPRINTF (run, "nanoeval_div2 i0=%ld i1=%ld ires=%ld",
                       (long) i0, (long) i1, (long) ires);
      return mom_boxint_make (ires);
    }
  else if (ty0 == MOMITY_BOXDOUBLE && ty1 == MOMITY_BOXDOUBLE)
    {
      double d0 = mom_boxdouble_val_def (arg0, 0.0);
      double d1 = mom_boxdouble_val_def (arg1, 0.0);
      double dres = d0 / d1;
      MOM_DEBUGPRINTF (run, "nanoeval_div2 d0=%g d1=%g dres=%g",
                       d0, d1, dres);
      return mom_boxdouble_make (dres);
    }
  else
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 2,
                                               arg0, arg1));
}                               /* end momf_nanoeval_div2 */


const char momsig_nanoeval_rem2[] = "signature_nanoeval2";
const void *
momf_nanoeval_rem2 (struct mom_nanoeval_st *nev,
                    struct mom_item_st *envitm,
                    int depth,
                    const struct mom_boxnode_st *expnod,
                    const struct mom_boxnode_st *closnod,
                    const void *arg0, const void *arg1)
{
  unsigned ty0 = mom_itype (arg0);
  unsigned ty1 = mom_itype (arg1);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_rem2 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s arg1=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0), mom_value_cstring (arg1));
  if (ty0 == MOMITY_BOXINT && ty1 == MOMITY_BOXINT)
    {
      intptr_t i0 = mom_boxint_val_def (arg0, -1);
      intptr_t i1 = mom_boxint_val_def (arg1, -1);
      if (i1 == 0)
        {
          MOM_DEBUGPRINTF (run,
                           "nanoeval_rem2 i0=%ld zerodivide undef expnod=%s",
                           i0,
                           mom_value_cstring ((struct mom_hashedvalue_st *)
                                              expnod));
          NANOEVAL_FAILURE_MOM (nev, expnod,
                                MOM_PREDEFITM (undefined_result));
        }
      intptr_t ires = i0 % i1;
      MOM_DEBUGPRINTF (run, "nanoeval_rem2 i0=%ld i1=%ld ires=%ld",
                       (long) i0, (long) i1, (long) ires);
      return mom_boxint_make (ires);
    }
  else
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 2,
                                               arg0, arg1));
}                               /* end momf_nanoeval_rem2 */



const char momsig_nanoeval_put3[] = "signature_nanoeval3";
const void *
momf_nanoeval_put3 (struct mom_nanoeval_st *nev,
                    struct mom_item_st *envitm,
                    int depth,
                    const struct mom_boxnode_st *expnod,
                    const struct mom_boxnode_st *closnod,
                    const void *arg0, const void *arg1, const void *arg2)
{
  bool ok = false;
  unsigned ty0 = mom_itype (arg0);
  unsigned ty1 = mom_itype (arg1);
  unsigned ty2 = mom_itype (arg2);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_put3 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s arg1=%s arg1=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0), mom_value_cstring (arg1),
                   mom_value_cstring (arg2));
  if (ty0 != MOMITY_ITEM)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 3,
                                               arg0, arg1, arg2));
  struct mom_item_st *itm0 = arg0;
  mom_item_lock (itm0);
  if (ty1 == MOMITY_ITEM)
    {
      struct mom_item_st *itm1 = arg1;
      mom_unsync_item_put_phys_attr (itm0, itm1, arg2);
      ok = true;
    }
  else if (ty1 == MOMITY_BOXINT && itm0->itm_pcomp != NULL)
    {
      int rk = mom_boxint_val_def (arg1, INT_MAX);
      unsigned ln = mom_vectvaldata_count (itm0->itm_pcomp);
      if (rk < 0)
        rk += ln;
      if (rk >= 0 && rk < ln)
        {
          mom_vectvaldata_put_nth (itm0->itm_pcomp, rk, arg2);
          ok = true;
        }
    }
  mom_item_unlock (itm0);
  if (!ok)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 3,
                                               arg0, arg1, arg2));

  return NULL;
}                               /* end momf_nanoeval_put3 */

struct mom_hashset_st *
add2hset_mom (struct mom_hashset_st *hset, const void *val)
{
  switch (mom_itype (val))
    {
    case MOMITY_NONE:
    case MOMITY_BOXINT:
    case MOMITY_BOXDOUBLE:
    case MOMITY_BOXSTRING:
      return hset;
    case MOMITY_ITEM:
      return mom_hashset_insert (hset, (struct mom_item_st *) val);
    case MOMITY_SET:
    case MOMITY_TUPLE:
      {
        const struct mom_seqitems_st *seq = (val);
        unsigned ln = mom_raw_size (seq);
        for (unsigned ix = 0; ix < ln; ix++)
          {
            if (seq->seqitem[ix] == NULL)
              continue;
            hset = mom_hashset_insert (hset, seq->seqitem[ix]);
          }
        return hset;
      }
    }
}                               /* end of add2hset_mom */

const char momsig_nanoeval_setany[] = "signature_nanoevalany";
const void *
momf_nanoeval_setany (struct mom_nanoeval_st *nev,
                      struct mom_item_st *envitm,
                      int depth,
                      const struct mom_boxnode_st *expnod,
                      const struct mom_boxnode_st *closnod,
                      unsigned nbval, const void **valarr)
{
  struct mom_hashset_st *hset =
    mom_hashset_reserve (NULL, mom_prime_above (4 * nbval / 3 + 5));

  MOM_DEBUGPRINTF (run,
                   "nanoeval_setany start envitm=%s depth=%d expnod=%s closnod=%s nbval=%d",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   nbval);
  for (int ix = 0; ix < (int) nbval; ix++)
    {
      MOM_DEBUGPRINTF (run, "nanoeval_setany valarr[%d]=%s", ix,
                       mom_value_cstring (valarr[ix]));
      hset = add2hset_mom (hset, valarr[ix]);
    }
  const struct mom_boxset_st *setv = mom_hashset_to_boxset (hset);
  MOM_DEBUGPRINTF (run, "nanoeval_setany depth=%d result setv=%s",
                   depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) setv));
  return setv;
}                               /* end of momf_nanoeval_set */


static void
add2queue_mom (struct mom_queue_st *qu, const void *val)
{
  switch (mom_itype (val))
    {
    case MOMITY_NONE:
    case MOMITY_BOXINT:
    case MOMITY_BOXDOUBLE:
    case MOMITY_BOXSTRING:
    case MOMITY_NODE:
      return;
    case MOMITY_ITEM:
      mom_queue_append (qu, val);
      return;
    case MOMITY_TUPLE:
    case MOMITY_SET:
      {
        const struct mom_seqitems_st *seq = (val);
        unsigned ln = mom_raw_size (seq);
        for (unsigned ix = 0; ix < ln; ix++)
          {
            if (seq->seqitem[ix] == NULL)
              continue;
            mom_queue_append (qu, seq->seqitem[ix]);
          }
      }
    }
}                               /* end of add2queue_mom */

const char momsig_nanoeval_tupleany[] = "signature_nanoevalany";
const void *
momf_nanoeval_tupleany (struct mom_nanoeval_st *nev,
                        struct mom_item_st *envitm,
                        int depth,
                        const struct mom_boxnode_st *expnod,
                        const struct mom_boxnode_st *closnod,
                        unsigned nbval, const void **valarr)
{
  struct mom_queue_st qu = { 0 };
  mom_queue_init (&qu);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_tupleany start envitm=%s depth=%d expnod=%s closnod=%s nbval=%d",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   nbval);
  for (int ix = 0; ix < (int) nbval; ix++)
    {
      MOM_DEBUGPRINTF (run, "nanoeval_tupleany depth=%d valarr[%d]=%s", depth,
                       ix, mom_value_cstring (valarr[ix]));
      add2queue_mom (&qu, valarr[ix]);
    }
  const struct mom_boxnode_st *nodv =
    mom_queue_node (&qu, MOM_PREDEFITM (tuple));
  memset (&qu, 0, sizeof (struct mom_queue_st));
  assert (mom_itype (nodv) == MOMITY_NODE);
  unsigned ln = mom_size (nodv);
  for (unsigned ix = 0; ix < ln; ix++)
    {
      assert (mom_itype (nodv->nod_sons[ix]) == MOMITY_ITEM);
    };
  const struct mom_boxtuple_st *tupres  //
    = mom_boxtuple_make_arr (ln, (const struct mom_item_st * const *)
                             nodv->nod_sons);
  MOM_DEBUGPRINTF (run, "nanoeval_tupleany depth=%d nodv=%s tupres=%s", depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) nodv),
                   mom_value_cstring ((struct mom_hashedvalue_st *) tupres));
  return tupres;
}                               /* end of momf_nanoeval_tupleany */

//////////////////////////////////////////// eof nanoeval.c
