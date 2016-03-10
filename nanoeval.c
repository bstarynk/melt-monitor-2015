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

MOM_PRIVATE struct mom_item_st *
nanoeval_freshenv_mom (struct mom_nanoeval_st *nev,
                       struct mom_item_st *parenvitm, unsigned sizhint,
                       struct mom_item_st *protoitm)
{
  assert (mom_itype (parenvitm) == MOMITY_ITEM);
  assert (mom_itype (protoitm) == MOMITY_ITEM);
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  struct mom_item_st *newenvitm = mom_clone_item (protoitm);
  if (!nev->nanev_transient)
    mom_item_put_space (newenvitm, MOMSPA_GLOBAL);
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



void
mom_bind_nanoev (struct mom_nanoeval_st *nev,
                 struct mom_item_st *envitm, const struct mom_item_st *varitm,
                 const void *val)
{
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  MOM_DEBUGPRINTF (run, "nanoeval_bind envitm=%s varitm=%s start",
                   mom_item_cstring (envitm), mom_item_cstring (varitm));
  assert (mom_itype (envitm) == MOMITY_ITEM);
  assert (mom_itype (varitm) == MOMITY_ITEM);
  mom_item_lock (envitm);
  envitm->itm_payload =
    (void *) mom_hashmap_put ((void *) envitm->itm_payload, varitm, val);
  mom_item_unlock (envitm);
  MOM_DEBUGPRINTF (run, "nanoeval_bind envitm=%s varitm=%s val=%s",
                   mom_item_cstring (envitm), mom_item_cstring (varitm),
                   mom_value_cstring (val));
}                               /* end of mom_bind_nanoev */

MOM_PRIVATE const void *
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
  struct mom_item_st *thistatitm = nev->nanev_thistatitm;
  MOM_DEBUGPRINTF (run,
                   "nanoeval_displaynode thistatitm=%s",
                   mom_item_cstring (thistatitm));
  assert (thistatitm != NULL);
  const struct mom_boxset_st *oldispset = NULL;
  {

    mom_item_lock (thistatitm);
    const struct mom_hashedvalue_st *dispitemv =
      mom_unsync_item_get_phys_attr (thistatitm,
                                     MOM_PREDEFITM (item));
    oldispset = mom_dyncast_set (dispitemv);
    mom_item_unlock (thistatitm);
    MOM_DEBUGPRINTF (run,
                     "nanoeval_displaynode dispitemv=%s",
                     mom_value_cstring (dispitemv));
  }
  if (oldispset != NULL)
    {
      unsigned oldsz = mom_size (oldispset);
      hset = mom_hashset_reserve (hset, 4 * oldsz / 3 + 3 * arity / 2 + 5);
      for (unsigned ix = 0; ix < oldsz; ix++)
        {
          struct mom_item_st *olditm = oldispset->seqitem[ix];
          assert (olditm != NULL && olditm->va_itype == MOMITY_ITEM);
          MOM_DEBUGPRINTF (run,
                           "nanoeval_displaynode ix=%d olditm=%s",
                           ix, mom_item_cstring (olditm));
          hset = mom_hashset_insert (hset, olditm);
        }
    }
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
  oldispset = mom_dyncast_set (oldispitemv);
  unsigned oldsiz = mom_size (oldispset);
  MOM_DEBUGPRINTF (run, "nanoeval_displaynode oldispitemv=%s oldsiz=%d",
                   mom_value_cstring (oldispitemv), oldsiz);
  for (unsigned oix = 0; oix < oldsiz; oix++)
    {
      struct mom_item_st *olditm = oldispset->seqitem[oix];
      assert (olditm && olditm->va_itype == MOMITY_ITEM);
      MOM_DEBUGPRINTF (run, "nanoeval_displaynode olditm=%s",
                       mom_item_cstring (olditm));
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


MOM_PRIVATE const void *
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


MOM_PRIVATE const void *
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
          continue;
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


MOM_PRIVATE const void *
nanoeval_switchitemnode_mom (struct mom_nanoeval_st *nev,
                             struct mom_item_st *envitm,
                             const struct mom_boxnode_st *nod, int depth)
{
  const void *resv = NULL;
  MOM_DEBUGPRINTF (run,
                   "nanoeval_switchitemnode start envitm=%s nod=%s depth#%d",
                   mom_item_cstring (envitm),
                   mom_value_cstring ((struct mom_hashedvalue_st *) nod),
                   depth);
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  assert (nod && nod->va_itype == MOMITY_NODE);
  unsigned arity = mom_size (nod);
  if (arity == 0)
    NANOEVAL_FAILURE_MOM (nev, nod,
                          mom_boxnode_make_va (MOM_PREDEFITM (arity), 1,
                                               mom_boxint_make (arity)));
  const struct mom_hashedvalue_st *selexpv = nod->nod_sons[0];
  MOM_DEBUGPRINTF (run, "nanoeval_switchitemnode depth#%d selexpv=%s",
                   depth, mom_value_cstring (selexpv));
  const void *selv = mom_nanoeval (nev, envitm, selexpv, depth + 1);
  MOM_DEBUGPRINTF (run, "nanoeval_switchitemnode depth#%d selv=%s",
                   depth, mom_value_cstring (selv));
  const struct mom_item_st *selitm = mom_dyncast_item (selv);
  for (int ix = 1; ix < (int) arity; ix++)
    {
      const struct mom_hashedvalue_st *curcasev = nod->nod_sons[ix];
      const struct mom_boxnode_st *casenod = mom_dyncast_node (curcasev);
      if (!casenod || casenod->nod_connitm != MOM_PREDEFITM (when))
        {
          resv = mom_nanoeval (nev, envitm, curcasev, depth + 1);
          MOM_DEBUGPRINTF (run,
                           "nanoeval_switchitemnode depth#%d ix#%d resv=%s",
                           depth, ix, mom_value_cstring (resv));
          continue;
        }
      unsigned casearity = mom_size (casenod);
      if (!casearity)
        break;
      const struct mom_hashedvalue_st *curcasexpv = casenod->nod_sons[0];
      MOM_DEBUGPRINTF (run,
                       "nanoeval_switchitemnode ix#%d curcasexpv %s depth %d",
                       ix, mom_value_cstring (curcasexpv), depth);
      const struct mom_item_st *curcasitm = mom_dyncast_item (curcasexpv);
      if (!curcasitm)
        {
          const void *curcasv =
            mom_nanoeval (nev, envitm, curcasexpv, depth + 1);
          const struct mom_boxset_st *curset = mom_dyncast_set (curcasv);
          if (curset && mom_set_contains (curset, selitm))
            goto founditem;
          curcasitm = mom_dyncast_item (curcasv);
          if (!curcasitm)
            continue;
        }
      if (curcasitm == selitm)
      founditem:
        {
          MOM_DEBUGPRINTF (run,
                           "nanoeval_switchitemnode ix#%d found item %s depth %d",
                           ix, mom_item_cstring (selitm), depth);
          for (int casix = 1; casix < (int) casearity; casix++)
            {
              const struct mom_hashedvalue_st *subexpv =
                casenod->nod_sons[ix];
              MOM_DEBUGPRINTF (run,
                               "nanoeval_switchitemnode ix#%d casix#%d depth#%d subexpv %s",
                               ix, casix, depth, mom_value_cstring (subexpv));
              resv = mom_nanoeval (nev, envitm, subexpv, depth + 1);
              MOM_DEBUGPRINTF (run,
                               "nanoeval_switchitemnode ix#%d casix#%d depth#%d resv=%s",
                               ix, casix, depth, mom_value_cstring (resv));
            }
          break;
        }
      else
        continue;
    }
  MOM_DEBUGPRINTF (run, "nanoeval_switchitemnode depth#%d final resv=%s",
                   depth, mom_value_cstring (resv));
  return resv;
}                               /* end of nanoeval_switchitemnode_mom */



MOM_PRIVATE const void *
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


MOM_PRIVATE const void *
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

MOM_PRIVATE const void *
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



MOM_PRIVATE const void *
nanoeval_outputnode_mom (struct mom_nanoeval_st *nev,
                         struct mom_item_st *envitm,
                         const struct mom_boxnode_st *nod, int depth)
{
  MOM_DEBUGPRINTF (run, "nanoeval_outputnode start envitm=%s nod=%s depth#%d",
                   mom_item_cstring (envitm),
                   mom_value_cstring ((struct mom_hashedvalue_st *) nod),
                   depth);
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  assert (nod && nod->va_itype == MOMITY_NODE);
  unsigned arity = mom_size (nod);
  if (arity < 2)
    NANOEVAL_FAILURE_MOM (nev, nod,
                          mom_boxnode_make_va (MOM_PREDEFITM (arity), 1,
                                               mom_boxint_make (arity)));
  void *outexpv = nod->nod_sons[0];
  const void *outv = mom_nanoeval (nev, envitm, outexpv, depth + 1);
  MOM_DEBUGPRINTF (run, "nanoeval_outputnode depth#%d outv=%s", depth,
                   mom_value_cstring (outv));
  struct mom_item_st *outitm = mom_dyncast_item (outv);
  if (!outitm)
    NANOEVAL_FAILURE_MOM (nev, nod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                               outv));
  /// special debug output
  enum mom_debug_en userdebug = momdbg__none;
  struct mom_nanoeval_st dbgnev = *nev;
  struct mom_nanoeval_st *orignev = nev;
  {
#define NBDBGOUTPUT_MOM 31
#define CASE_DBGOUTPUT_MOM(Nam) momhashpredef_##Nam % NBDBGOUTPUT_MOM:	\
	  if (outitm == MOM_PREDEFITM(Nam)) goto foundcase_##Nam;	\
	  goto defaultcasedbg; foundcase_##Nam
    switch (outitm->hva_hash % NBDBGOUTPUT_MOM)
      {
      case CASE_DBGOUTPUT_MOM (dbg_blue):
        userdebug = momdbg_blue;
        break;
      case CASE_DBGOUTPUT_MOM (dbg_green):
        userdebug = momdbg_green;
        break;
      case CASE_DBGOUTPUT_MOM (dbg_red):
        userdebug = momdbg_red;
        break;
      case CASE_DBGOUTPUT_MOM (dbg_yellow):
        userdebug = momdbg_yellow;
        break;
      defaultcasedbg:
      default:
        break;
      }
#undef CASE_DBGOUTPUT_MOM
#undef NBDBGOUTPUT_MOM
  };
  MOM_DEBUGPRINTF (run, "nanoeval_outputnode depth#%d userdebug=%#x", depth,
                   userdebug);
  FILE *fil = NULL;
  char *bufdbg = NULL;
  size_t sizdbg = 0;
  volatile int dbgfailine = 0;
  if ((int) userdebug > 0)
    {
      if (!(mom_debugflags & (1 << userdebug)))
        {
          MOM_DEBUGPRINTF (run,
                           "nanoeval_outputnode depth#%d no userdebug#%#x for %s",
                           depth, (int) userdebug, mom_item_cstring (outitm));
          return NULL;
        }
      fil = open_memstream (&bufdbg, &sizdbg);
      MOM_DEBUGPRINTF (run,
                       "nanoeval_outputnode depth#%d for debug fil@%p",
                       depth, (void *) fil);
      if (!fil)
        MOM_FATAPRINTF ("nanoeval_outputnode failed to open debug for %s",
                        mom_item_cstring (outitm));
      nev = &dbgnev;
      dbgfailine = setjmp (dbgnev.nanev_jb);
      if (dbgfailine)
        {
          MOM_DEBUGPRINTF (run,
                           "nanoeval_outputnode failed at dbgfailine#%d",
                           dbgfailine);
          goto failed_debug;
        }
    }
  ////
  for (unsigned ix = 1; ix < arity; ix++)
    {
      const void *subexpv = nod->nod_sons[ix];
      unsigned subsiz = mom_size (subexpv);
      MOM_DEBUGPRINTF (run, "nanoeval_outputnode depth#%d ix#%d subexpv=%s",
                       depth, ix, mom_value_cstring (subexpv));
      struct mom_file_st *mf = NULL;
      if ((int) userdebug == 0)
        {
          mom_item_lock (outitm);
          fil = mom_file (outitm->itm_payload);
          if (fil)
            mf = (struct mom_file_st *) (outitm->itm_payload);
          mom_item_unlock (outitm);
        }
      MOM_DEBUGPRINTF (run,
                       "nanoeval_outputnode depth#%d  userdebug#%#x fil@%p ix#%d",
                       depth, userdebug, (void *) fil, ix);
      if (!fil)
        break;
      const struct mom_boxnode_st *subexpnod = mom_dyncast_node (subexpv);
      struct mom_item_st *subconitm = NULL;
      if (subexpnod && (subconitm = subexpnod->nod_connitm) != NULL
          && subsiz == 1)
        {
          const void *firstv = NULL;
          const void *firstargexpv = subexpnod->nod_sons[0];
#define NBMODOUTPUT_MOM 179
#define CASE_OUTPUT_MOM(Nam) momhashpredef_##Nam % NBMODOUTPUT_MOM:	\
	  if (subconitm == MOM_PREDEFITM(Nam)) goto foundcase_##Nam;	\
	  goto defaultcase; foundcase_##Nam
          switch (subconitm->hva_hash % NBMODOUTPUT_MOM)
            {
            case CASE_OUTPUT_MOM (out_decimal):
              {
                firstv = mom_nanoeval (nev, envitm, firstargexpv, depth + 1);
                if (mom_itype (firstv) == MOMITY_BOXINT)
                  {
                    intptr_t ival = mom_boxint_val_def (firstv, 0);
                    fprintf (fil, "%ld", (long) ival);
                  }
                continue;
              }
            case CASE_OUTPUT_MOM (out_hexa):
              {
                firstv = mom_nanoeval (nev, envitm, firstargexpv, depth + 1);
                if (mom_itype (firstv) == MOMITY_BOXINT)
                  {
                    intptr_t ival = mom_boxint_val_def (firstv, 0);
                    fprintf (fil, "%lx", (long) ival);
                  }
                continue;
              }
            case CASE_OUTPUT_MOM (out_html):
              {
                firstv = mom_nanoeval (nev, envitm, firstargexpv, depth + 1);
                if (mom_itype (firstv) == MOMITY_BOXSTRING)
                  mom_output_utf8_html (fil, mom_boxstring_cstr (firstv),
                                        mom_size (firstv), false);
                continue;
              }
            case CASE_OUTPUT_MOM (out_utf8enc):
              {
                firstv = mom_nanoeval (nev, envitm, firstargexpv, depth + 1);
                if (mom_itype (firstv) == MOMITY_BOXSTRING)
                  mom_output_utf8_encoded (fil, mom_boxstring_cstr (firstv),
                                           mom_size (firstv));
                continue;
              }
            case CASE_OUTPUT_MOM (out_filebuffer):
              {
                firstv = mom_nanoeval (nev, envitm, firstargexpv, depth + 1);
                if (mom_itype (firstv) == MOMITY_ITEM)
                  {
                    struct mom_item_st *firstitm = mom_dyncast_item (firstv);
                    mom_item_lock (firstitm);
                    if (mom_itype (firstitm->itm_payload) ==
                        MOMITY_FILEBUFFER)
                      mom_puts_filebuffer (fil,
                                           (struct mom_filebuffer_st *)
                                           firstitm->itm_payload,
                                           MOM_FILEBUFFER_KEEPOPEN);
                    mom_item_unlock (firstitm);
                  }
                continue;
              }
            case CASE_OUTPUT_MOM (out_apply):
              {
                firstv = mom_nanoeval (nev, envitm, firstargexpv, depth + 1);
                if (mom_itype (firstv) == MOMITY_NODE)
                  (void) mom_nanoapply (nev, envitm,
                                        (struct mom_boxnode_st *) firstv,
                                        subexpnod, 1, (const void **) &outitm,
                                        depth + 1);
                continue;
              };
            case CASE_OUTPUT_MOM (out_raw):
              {
                firstv = mom_nanoeval (nev, envitm, firstargexpv, depth + 1);
                switch (mom_itype (firstv))
                  {
                  case MOMITY_BOXINT:
                    mom_file_printf (fil, "%lld",
                                     (long long) mom_boxint_val_def (firstv,
                                                                     0));
                    break;
                  case MOMITY_BOXSTRING:
                    mom_file_puts (fil, mom_boxstring_cstr (firstv));
                    break;
                  case MOMITY_BOXDOUBLE:
                    mom_file_printf (fil, "%g",
                                     mom_boxdouble_val_def (firstv, 0.0));
                    break;
                  case MOMITY_ITEM:
                    mom_file_puts (fil,
                                   mom_item_cstring (mom_dyncast_item
                                                     (firstv)));
                    break;
                  default:;
                  }
                continue;
              }
            default:
            defaultcase:
              break;
            }
#undef NBMODOUTPUT_MOM
#undef CASE_OUTPUT_MOM
        }
      else if (subexpnod
               && subsiz == 3
               && ((subconitm = subexpnod->nod_connitm) ==
                   MOM_PREDEFITM (out_gplv3_notice)))
        {
          const void *prefixv = NULL;
          const void *prefixexpv = subexpnod->nod_sons[0];
          const void *suffixv = NULL;
          const void *suffixexpv = subexpnod->nod_sons[1];
          const void *filnamv = NULL;
          const void *filnamexpv = subexpnod->nod_sons[2];
          prefixv = mom_nanoeval (nev, envitm, prefixexpv, depth + 1);
          suffixv = mom_nanoeval (nev, envitm, suffixexpv, depth + 1);
          filnamv = mom_nanoeval (nev, envitm, filnamexpv, depth + 1);
          mom_output_gplv3_notice (fil,
                                   mom_boxstring_cstr (prefixv),
                                   mom_boxstring_cstr (suffixv),
                                   mom_boxstring_cstr (filnamv));
          continue;
        }
      else if (subexpnod
               && subsiz == 0
               && ((subconitm = subexpnod->nod_connitm) ==
                   MOM_PREDEFITM (out_flush)))
        {
          fflush (fil);
          continue;
        }
      const void *subval = mom_nanoeval (nev, envitm, subexpv, depth + 1);
      if (subval)
        mom_output_value (fil, NULL, mf ? mf->mom_findent : 0,
                          (const struct mom_hashedvalue_st *) subval);
    }                           /* end for */
  if (userdebug > 0)
  failed_debug:
    {
      fflush (fil);
      if (dbgfailine)
        {
          mom_debugprintf_at ("*nanoeval*!!", nod->nod_metarank,
                              userdebug,
                              " (failed from %s line %d failure %s expr %s) %s",
                              dbgnev.nanev_errfile,
                              dbgfailine,
                              mom_value_cstring (dbgnev.nanev_fail),
                              mom_value_cstring (dbgnev.nanev_expr), bufdbg);
        }
      else
        {
          mom_debugprintf_at ("*nanoeval*", nod->nod_metarank,
                              userdebug, "%s", bufdbg);
        }
      fclose (fil);
      free (bufdbg);
      if (dbgfailine)
        {
          MOM_DEBUGPRINTF (run,
                           "reraising debug output error from %s line %d failure %s expr %s",
                           dbgnev.nanev_errfile, dbgfailine,
                           mom_value_cstring (dbgnev.nanev_fail),
                           mom_value_cstring (dbgnev.nanev_expr));

          orignev->nanev_fail = dbgnev.nanev_fail;
          orignev->nanev_errfile = dbgnev.nanev_errfile;
          orignev->nanev_expr = dbgnev.nanev_expr;
          longjmp (orignev->nanev_jb, dbgfailine);
        }
    }
  return outitm;
}                               /* end nanoeval_outputnode_mom */


MOM_PRIVATE const void *
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


MOM_PRIVATE const void *
nanoeval_assignnode_mom (struct mom_nanoeval_st *nev,
                         struct mom_item_st *envitm,
                         const struct mom_boxnode_st *nod, int depth)
{
  const void *resv = NULL;
  struct mom_item_st *origenvitm = envitm;
  MOM_DEBUGPRINTF (run, "nanoeval_assignnode start envitm=%s nod=%s depth#%d",
                   mom_item_cstring (envitm),
                   mom_value_cstring ((struct mom_hashedvalue_st *) nod),
                   depth);
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  assert (nod && nod->va_itype == MOMITY_NODE);
  unsigned arity = mom_size (nod);
  if (arity < 2)
    NANOEVAL_FAILURE_MOM (nev, nod,
                          mom_boxnode_make_va (MOM_PREDEFITM (arity), 1,
                                               mom_boxint_make (arity)));
  struct mom_item_st *varitm = mom_dyncast_item (nod->nod_sons[0]);
  if (!varitm)
    NANOEVAL_FAILURE_MOM (nev, nod, MOM_PREDEFITM (undefined_result));
  for (unsigned ix = 1; ix < arity; ix++)
    {
      const void *subexpv = nod->nod_sons[ix];
      MOM_DEBUGPRINTF (run,
                       "nanoeval_assignnode node %s ix#%d subexpv=%s depth#%d",
                       mom_value_cstring ((const struct mom_hashedvalue_st *)
                                          nod), ix,
                       mom_value_cstring (subexpv), depth);
      resv = mom_nanoeval (nev, envitm, subexpv, depth + 1);
      MOM_DEBUGPRINTF (run, "nanoeval_assignnode resv=%s depth#%d",
                       mom_value_cstring (resv), depth);
    }
  bool found = false;
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
          const void *oldv =
            mom_hashmap_get ((struct mom_hashmap_st *) envitm->itm_payload,
                             varitm);
          if (oldv)
            {
              found = true;
              prevenvitm = NULL;
              envitm->itm_payload =
                (void *) mom_hashmap_put ((void *) envitm->itm_payload,
                                          varitm, resv);
              MOM_DEBUGPRINTF (run,
                               "nanoeval_assignnode update envitm=%s depth#%d",
                               mom_item_cstring (envitm), depth);
            }
        }
      mom_item_unlock (envitm);
      envitm = prevenvitm;
    }
  if (!found)
    {
      mom_bind_nanoev (nev, origenvitm, varitm, resv);
      MOM_DEBUGPRINTF (run,
                       "nanoeval_assignnode update origenvitm=%s depth#%d",
                       mom_item_cstring (envitm), depth);
    }
  return resv;
}                               /* end of nanoeval_assignnode_mom */



MOM_PRIVATE void *
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
      if (!nev->nanev_transient)
        mom_item_put_space ((struct mom_item_st *) resv, MOMSPA_GLOBAL);
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
      if (!nev->nanev_transient)
        mom_item_put_space ((struct mom_item_st *) resv, MOMSPA_GLOBAL);
      MOM_DEBUGPRINTF (run, "nanoeval_itemnode cloned resv=%s",
                       mom_item_cstring (resv));
    }
  return resv;
}                               /* end nanoeval_itemnode_mom */


MOM_PRIVATE const void *
nanoeval_transienteval_mom (struct mom_nanoeval_st *nev,
                            bool transient,
                            struct mom_item_st *envitm,
                            const struct mom_boxnode_st *nod, int depth)
{
  struct mom_nanoeval_st newnev = { };
  MOM_DEBUGPRINTF (run,
                   "nanoeval_transienteval start %s envitm=%s nod=%s depth#%d",
                   transient ? "transient" : "persistent",
                   mom_item_cstring (envitm),
                   mom_value_cstring ((struct mom_hashedvalue_st *) nod),
                   depth);
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  assert (nod && nod->va_itype == MOMITY_NODE);
  newnev = *nev;
  int errlin = 0;
  if ((errlin = setjmp (newnev.nanev_jb)) > 0)
    {
      MOM_DEBUGPRINTF_AT (newnev.nanev_errfile, errlin, run,
                          "nanoeval_transienteval %s failure %s with %s rethrown",
                          transient ? "transient" : "persistent",
                          mom_value_cstring (newnev.nanev_fail),
                          mom_value_cstring (newnev.nanev_expr));
      nev->nanev_fail = newnev.nanev_fail;
      nev->nanev_expr = newnev.nanev_expr;
      nev->nanev_errfile = newnev.nanev_errfile;
      longjmp (nev->nanev_jb, errlin);
    }
  else
    {
      const void *res = NULL;
      assert (newnev.nanev_magic == NANOEVAL_MAGIC_MOM);
      newnev.nanev_transient = transient;
      unsigned siz = mom_raw_size (nod);
      for (unsigned ix = 0; ix < siz; ix++)
        {
          const void *expv = nod->nod_sons[ix];
          MOM_DEBUGPRINTF (run,
                           "nanoeval_transienteval %s ix#%d envitm=%s expv=%s depth#%d",
                           transient ? "transient" : "persistent", ix,
                           mom_item_cstring (envitm),
                           mom_value_cstring (expv), depth);

          res = mom_nanoeval (&newnev, envitm, expv, depth + 1);
          MOM_DEBUGPRINTF (run,
                           "nanoeval_transienteval %s ix#%d depth#%d res=%s",
                           transient ? "transient" : "persistent", ix, depth,
                           mom_value_cstring (res));
        }
      return res;
    }
}                               /* end nanoeval_transienteval_mom */

MOM_PRIVATE const void *
nanoeval_transientnode_mom (struct mom_nanoeval_st *nev,
                            struct mom_item_st *envitm,
                            const struct mom_boxnode_st *nod, int depth)
{
  return nanoeval_transienteval_mom (nev, true, envitm, nod, depth);
}                               // end nanoeval_transientnode_mom

MOM_PRIVATE const void *
nanoeval_persistentnode_mom (struct mom_nanoeval_st *nev,
                             struct mom_item_st *envitm,
                             const struct mom_boxnode_st *nod, int depth)
{
  return nanoeval_transienteval_mom (nev, false, envitm, nod, depth);
}                               // end nanoeval_persistentnode_mom


// like progn in Lisp or begin in Scheme
MOM_PRIVATE const void *
nanoeval_sequencenode_mom (struct mom_nanoeval_st *nev,
                           struct mom_item_st *envitm,
                           const struct mom_boxnode_st *nod, int depth)
{
  MOM_DEBUGPRINTF (run,
                   "nanoeval_sequencenode start envitm=%s nod=%s depth#%d",
                   mom_item_cstring (envitm),
                   mom_value_cstring ((struct mom_hashedvalue_st *) nod),
                   depth);
  unsigned ln = mom_size (nod);
  const void *res = NULL;
  for (unsigned ix = 0; ix < ln; ix++)
    {
      res = mom_nanoeval (nev, envitm, nod->nod_sons[ix], depth + 1);
    }
  MOM_DEBUGPRINTF (run, "nanoeval_sequencenode depth#%d final res=%s",
                   depth, mom_value_cstring (res));
  return res;
}                               // end nanoeval_sequencenode_mom

// like prog1 in Lisp; if only one subexpression, take its first component
MOM_PRIVATE const void *
nanoeval_firstnode_mom (struct mom_nanoeval_st *nev,
                        struct mom_item_st *envitm,
                        const struct mom_boxnode_st *nod, int depth)
{
  MOM_DEBUGPRINTF (run,
                   "nanoeval_sequencenode start envitm=%s nod=%s depth#%d",
                   mom_item_cstring (envitm),
                   mom_value_cstring ((struct mom_hashedvalue_st *) nod),
                   depth);
  unsigned ln = mom_size (nod);
  const void *res = NULL;
  if (ln == 1)
    {
      const void *subres =
        mom_nanoeval (nev, envitm, nod->nod_sons[0], depth + 1);
      switch (mom_itype (subres))
        {
        case MOMITY_TUPLE:
        case MOMITY_SET:
          res = mom_seqitems_nth (subres, 0);
          break;
        case MOMITY_NODE:
          res = mom_boxnode_nth (subres, 0);
          break;
        default:
          res = subres;
        }
    }
  else
    {
      for (unsigned ix = 0; ix < ln; ix++)
        {
          const void *subres =
            mom_nanoeval (nev, envitm, nod->nod_sons[ix], depth + 1);
          if (ix == 0)
            res = subres;
        }
    }
  MOM_DEBUGPRINTF (run, "nanoeval_sequencenode depth#%d final res=%s",
                   depth, mom_value_cstring (res));
  return res;
}                               // end nanoeval_firstnode_mom

MOM_PRIVATE const void *
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
    NANOEVAL_FAILURE_MOM (nev, nod, mom_boxnode_make_va (MOM_PREDEFITM (arity), //
                                                         1,
                                                         mom_boxint_make
                                                         (arity)));
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
        mom_boxnode_make (MOM_PREDEFITM (nano_closure), arity + 1, arr);
      MOM_DEBUGPRINTF (run,
                       "nanoeval_funcnode fixed arity:%d resnod %s\n... depth#%d",
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
            NANOEVAL_FAILURE_MOM (nev, nod, mom_boxnode_make_va (MOM_PREDEFITM (arg),   //
                                                                 1,
                                                                 formalsv));

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
      MOM_DEBUGPRINTF (run,
                       "nanoeval_funcnode depth#%d arr[0]=%s",
                       depth,
                       mom_value_cstring ((struct mom_hashedvalue_st *)
                                          arr[0]));
      for (unsigned ix = 1; ix < arity; ix++)
        arr[ix + 1] = nod->nod_sons[ix];
      const struct mom_boxnode_st *resnod =
        mom_boxnode_make (MOM_PREDEFITM (nano_closure), arity + 1, arr);
      MOM_DEBUGPRINTF (run,
                       "nanoeval_funcnode arity:%d resnod %s\n... depth#%d",
                       arity,
                       mom_value_cstring ((struct mom_hashedvalue_st *)
                                          resnod), depth);
      return resnod;
    }
  else
    {
      MOM_DEBUGPRINTF (run,
                       "nanoeval_funcnode depth#%d bad formalsv %s",
                       depth, mom_value_cstring ((void *) formalsv));
      NANOEVAL_FAILURE_MOM (nev, nod, mom_boxnode_make_va (MOM_PREDEFITM (undefined_result),    //
                                                           1, formalsv));
    }
}                               /* end nanoeval_funcnode_mom */


const void *
mom_nanoapply (struct mom_nanoeval_st *nev,
               struct mom_item_st *envitm,
               const struct mom_boxnode_st *nodfun,
               const struct mom_boxnode_st *nodexp,
               unsigned nbargs, const void **argv, int depth)
{
  const void *resv = NULL;
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  struct mom_item_st *opitm = mom_boxnode_conn (nodexp);
  MOM_DEBUGPRINTF (run,
                   "nanoapply envitm=%s nodfun=%s nodexp=%s nbargs#%d depth#%d",
                   mom_item_cstring (envitm),
                   mom_value_cstring ((struct mom_hashedvalue_st *) nodfun),
                   mom_value_cstring ((struct mom_hashedvalue_st *) nodexp),
                   nbargs, depth);
  unsigned funlen = mom_size (nodfun);
  if (funlen < 3 || nodfun->nod_connitm != MOM_PREDEFITM (nano_closure))
    NANOEVAL_FAILURE_MOM (nev, nodexp,
                          mom_boxnode_make_va (MOM_PREDEFITM (func), 1,
                                               nodfun));
  void *formalsv = nodfun->nod_sons[0];
  struct mom_item_st *clenvitm = mom_dyncast_item (nodfun->nod_sons[1]);
  if (!clenvitm)
    NANOEVAL_FAILURE_MOM (nev, nodexp, mom_boxnode_make_va (MOM_PREDEFITM (undefined_result),   //
                                                            1, nodfun));
  if (!opitm)
    NANOEVAL_FAILURE_MOM (nev, nodexp, mom_boxnode_make_va (MOM_PREDEFITM (expression), //
                                                            1, nodexp));
  struct mom_item_st *newenvitm =
    nanoeval_freshenv_mom (nev, clenvitm, mom_size (formalsv) + 1, opitm);
  MOM_DEBUGPRINTF (run, "nanoapply newenvitm=%s formalsv=%s depth#%d",
                   mom_item_cstring (newenvitm),
                   mom_value_cstring (formalsv), depth);
  const struct mom_boxtuple_st *formtup = NULL;
  const struct mom_boxnode_st *formnod = NULL;
  if ((formtup = mom_dyncast_tuple (formalsv)) != NULL)
    {
      unsigned nbform = mom_size (formtup);
      MOM_DEBUGPRINTF (run, "nanoapply fixed nbform=%d nbargs=%d", nbform,
                       nbargs);
      if (nbform != nbargs)
        NANOEVAL_FAILURE_MOM (nev, nodexp,
                              mom_boxnode_make_va (MOM_PREDEFITM (signature),
                                                   2, nodexp, formalsv));
      for (unsigned ix = 0; ix < nbform; ix++)
        mom_bind_nanoev (nev, newenvitm, formtup->seqitem[ix], argv[ix]);
    }
  else if ((formnod = mom_dyncast_node (formalsv)) != NULL)
    {
      unsigned nbform = mom_size (formnod);
      MOM_DEBUGPRINTF (run, "nanoapply variadic nbform=%d nbargs=%d", nbform,
                       nbargs);
      if (nbargs < nbform)
        NANOEVAL_FAILURE_MOM (nev, nodexp,
                              mom_boxnode_make_va (MOM_PREDEFITM (signature),
                                                   2, nodexp, formalsv));
      for (unsigned ix = 0; ix < nbform; ix++)
        {
          struct mom_item_st *curformitm =
            mom_dyncast_item (formnod->nod_sons[ix]);
          if (!curformitm)
            NANOEVAL_FAILURE_MOM (nev, nodexp,
                                  mom_boxnode_make_va (MOM_PREDEFITM (arg),
                                                       1, formalsv));
          mom_bind_nanoev (nev, newenvitm, curformitm, argv[ix]);
        }
      struct mom_item_st *connformitm = formnod->nod_connitm;
      if (nbargs > nbform)
        {
          const struct mom_boxnode_st *nodrest =
            mom_boxnode_make (connformitm, nbargs - nbform,
                              (const struct mom_hashedvalue_st **) (argv +
                                                                    nbform));
          mom_bind_nanoev (nev, newenvitm, connformitm, nodrest);
        }
      else
        mom_bind_nanoev (nev, newenvitm, connformitm, connformitm);
    }
  MOM_DEBUGPRINTF (run, "nanoapply newenvitm=%s nodfun=%s nodexp=%s depth#%d",
                   mom_item_cstring (newenvitm),
                   mom_value_cstring ((const struct mom_hashedvalue_st *)
                                      nodfun),
                   mom_value_cstring ((const struct mom_hashedvalue_st *)
                                      nodexp), depth);
  for (unsigned ix = 2; ix < funlen; ix++)
    {
      const void *subexpv = nodfun->nod_sons[ix];
      MOM_DEBUGPRINTF (run,
                       "nanoapply newenvitm=%s ix#%d subexpv=%s depth#%d",
                       mom_item_cstring (newenvitm), ix,
                       mom_value_cstring (subexpv), depth);
      resv = mom_nanoeval (nev, newenvitm, subexpv, depth + 1);
      MOM_DEBUGPRINTF (run, "nanoapply newenvitm=%s ix#%d resv=%s",
                       mom_item_cstring (newenvitm), ix,
                       mom_value_cstring (resv));
    }
  MOM_DEBUGPRINTF (run,
                   "nanoapply envitm=%s newenvitm=%s depth#%d final resv=%s",
                   mom_item_cstring (envitm), mom_item_cstring (newenvitm),
                   depth, mom_value_cstring (resv));
  return resv;
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



MOM_PRIVATE const void *
nanoeval_letnode_mom (struct mom_nanoeval_st *nev,
                      struct mom_item_st *envitm,
                      const struct mom_boxnode_st *nod, int depth)
{                               /// %let(bindings... body....)
  const void *res = NULL;
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
    nanoeval_freshenv_mom (nev, envitm, nbbind, nod->nod_connitm);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_letnode depth#%d arity=%d nbbind=%d newenvitm=%s",
                   depth, arity, nbbind, mom_item_cstring (newenvitm));
  assert (nbbind <= arity);
  for (unsigned ix = 0; ix < nbbind; ix++)
    {
      const struct mom_boxnode_st *bindnod =
        mom_dyncast_node (nod->nod_sons[ix]);
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
      mom_bind_nanoev (nev, newenvitm, varitm, bvalv);
    }
  for (unsigned ix = nbbind; ix < arity; ix++)
    {
      const void *subexpv = nod->nod_sons[ix];
      MOM_DEBUGPRINTF (run, "nanoeval_letnode depth#%d ix#%d subexpv %s",
                       depth, ix, mom_value_cstring (subexpv));
      const void *subvalv = mom_nanoeval (nev, newenvitm, subexpv, depth + 1);
      MOM_DEBUGPRINTF (run, "nanoeval_letnode depth#%d ix#%d subvalv %s",
                       depth, ix, mom_value_cstring (subvalv));
      res = subvalv;
    }
  MOM_DEBUGPRINTF (run, "nanoeval_letnode depth#%d final res=%s", depth,
                   mom_value_cstring (res));
  return res;
}                               /* end nanoeval_letnode_mom */


MOM_PRIVATE const void *
nanoeval_letrecnode_mom (struct mom_nanoeval_st *nev,
                         struct mom_item_st *envitm,
                         const struct mom_boxnode_st *nod, int depth)
{                               /// %letrec(bindings... body....)
  const void *res = NULL;
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
    nanoeval_freshenv_mom (nev, envitm, nbbind, nod->nod_connitm);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_letrecnode depth#%d arity=%d nbbind=%d newenvitm=%s",
                   depth, arity, nbbind, mom_item_cstring (newenvitm));
  assert (nbbind <= arity);
  // first loop to temporarily bind the variables to themselves
  for (unsigned ix = 0; ix < nbbind; ix++)
    {
      const struct mom_boxnode_st *bindnod =
        mom_dyncast_node (nod->nod_sons[ix]);
      assert (bindnod != NULL);
      unsigned bindln = mom_size (bindnod);
      assert (bindnod->nod_connitm == MOM_PREDEFITM (be) && bindln >= 2);
      struct mom_item_st *varitm = mom_dyncast_item (bindnod->nod_sons[0]);
      assert (varitm != NULL && varitm->va_itype == MOMITY_ITEM);
      MOM_DEBUGPRINTF (run,
                       "nanoeval_letrecnode depth#%d binding#%d first varitm %s",
                       depth, ix, mom_item_cstring (varitm));
      mom_bind_nanoev (nev, newenvitm, varitm, varitm);
    }
  for (unsigned ix = 0; ix < nbbind; ix++)
    {
      const struct mom_boxnode_st *bindnod =
        mom_dyncast_node (nod->nod_sons[ix]);
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
      mom_bind_nanoev (nev, newenvitm, varitm, bvalv);
    }
  for (unsigned ix = nbbind; ix < arity; ix++)
    {
      const void *subexpv = nod->nod_sons[ix];
      MOM_DEBUGPRINTF (run, "nanoeval_letrecnode depth#%d ix#%d subexpv %s",
                       depth, ix, mom_value_cstring (subexpv));
      const void *subvalv = mom_nanoeval (nev, newenvitm, subexpv, depth + 1);
      MOM_DEBUGPRINTF (run, "nanoeval_letrecnode depth#%d ix#%d subvalv %s",
                       depth, ix, mom_value_cstring (subvalv));
      res = subvalv;
    }
  MOM_DEBUGPRINTF (run, "nanoeval_letrecnode depth#%d final res=%s", depth,
                   mom_value_cstring (res));
  return res;
}                               /* end nanoeval_letrecnode_mom */


/*** with one argument: %nanoeval(<expr>) evaluates the result of
 * <expr>.  
 *
 * With no arguments: %nanoeval() gives the current
 * environment.  
 * 
 * With several arguments: %nanoeval(<env>,<expr>...)  evaluate the
 * result of every <expr> in the <env>
 ***/
MOM_PRIVATE const void *
nanoeval_nanoevalnode_mom (struct mom_nanoeval_st *nev,
                           struct mom_item_st *envitm,
                           const struct mom_boxnode_st *nod, int depth)
{
  const void *resv = NULL;
  MOM_DEBUGPRINTF (run,
                   "nanoeval_nanoevalnode start envitm=%s nod=%s depth#%d",
                   mom_item_cstring (envitm),
                   mom_value_cstring ((struct mom_hashedvalue_st *) nod),
                   depth);
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  assert (nod && nod->va_itype == MOMITY_NODE);
  unsigned arity = mom_size (nod);
  if (arity == 0)
    {
      MOM_DEBUGPRINTF (run,
                       "nanoeval_nanoevalnode depth#%d gives the envitm=%s",
                       depth, mom_item_cstring (envitm));
      return envitm;
    }
  else if (arity == 1)
    {
      const void *evexpv =
        mom_nanoeval (nev, envitm, nod->nod_sons[0], depth + 1);
      MOM_DEBUGPRINTF (run, "nanoeval_nanoevalnode depth#%d re-evaluating %s",
                       depth, mom_value_cstring (evexpv));
      resv = mom_nanoeval (nev, envitm, evexpv, depth + 1);
      MOM_DEBUGPRINTF (run, "nanoeval_nanoevalnode depth#%d gives resv %s",
                       depth, mom_value_cstring (resv));
      return resv;
    }
  else
    {
      const void *envalv =
        mom_nanoeval (nev, envitm, nod->nod_sons[0], depth + 1);
      if (!envalv)
        envalv = envitm;
      struct mom_item_st *newenvitm = mom_dyncast_item (envalv);
      if (!newenvitm)
        NANOEVAL_FAILURE_MOM (nev, nod,
                              mom_boxnode_make_va (MOM_PREDEFITM (type_error),
                                                   1, envalv));
      for (unsigned ix = 1; ix < arity; ix++)
        {
          MOM_DEBUGPRINTF (run,
                           "nanoeval_nanoevalnode depth#%d primoevaluation#%d of %s in envitm %s",
                           depth, ix, mom_value_cstring (nod->nod_sons[ix]),
                           mom_item_cstring (envitm));
          const void *curexpv =
            mom_nanoeval (nev, envitm, nod->nod_sons[ix], depth + 1);
          MOM_DEBUGPRINTF (run,
                           "nanoeval_nanoevalnode depth#%d re-evaluating %s in newenvitm %s",
                           depth, mom_value_cstring (curexpv),
                           mom_item_cstring (newenvitm));
          resv = mom_nanoeval (nev, newenvitm, curexpv, depth + 1);
          MOM_DEBUGPRINTF (run,
                           "nanoeval_nanoevalnode depth#%d index#%d result %s",
                           depth, ix, mom_value_cstring (resv));
        }
      MOM_DEBUGPRINTF (run, "nanoeval_nanoevalnode depth#%d gives result %s",
                       depth, mom_value_cstring (resv));
      return resv;
    }
}                               /* end nanoeval_nanoevalnode_mom  */

MOM_PRIVATE const void *
nanoeval_othernode_mom (struct mom_nanoeval_st *nev,
                        struct mom_item_st *envitm,
                        const struct mom_boxnode_st *nod, int depth)
{
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
#define MODNANOVALSIG_MOM 239
#define NANOEVALSIG_MOM(Nam) momhashpredef_##Nam % MODNANOVALSIG_MOM: \
    if (opsigitm == MOM_PREDEFITM(Nam)) goto foundcase_##Nam; goto defaultcase; foundcase_##Nam
          switch (opsigitm->hva_hash % MODNANOVALSIG_MOM)
            {
            case NANOEVALSIG_MOM (signature_nanoeval0):
              {
                if (arity != 0)
                  NANOEVAL_FAILURE_MOM (nev, nod, mom_boxnode_make_va (MOM_PREDEFITM (arity),   //
                                                                       1,
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
                  NANOEVAL_FAILURE_MOM (nev, nod, mom_boxnode_make_va (MOM_PREDEFITM (arity),   //
                                                                       1,
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
                  NANOEVAL_FAILURE_MOM (nev, nod, mom_boxnode_make_va (MOM_PREDEFITM (arity),   //
                                                                       1,
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
                  NANOEVAL_FAILURE_MOM (nev, nod, mom_boxnode_make_va (MOM_PREDEFITM (arity),   //
                                                                       1,
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
                  NANOEVAL_FAILURE_MOM (nev, nod, mom_boxnode_make_va (MOM_PREDEFITM (arity),   //
                                                                       1,
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
                const void **arrv =
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
                 mom_item_cstring (opsigitm));
              NANOEVAL_FAILURE_MOM (nev, nod, mom_boxnode_make_va (MOM_PREDEFITM (signature),   //
                                                                   1, opitm));
            }
#undef MODNANOVALSIG_MOM
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
      if (connvnod && connvnod->nod_connitm == MOM_PREDEFITM (nano_closure)
          && (connsiz = mom_raw_size (connvnod)) >= 2)
        {
          const void **arr = mom_gc_alloc ((arity + 1) * sizeof (void *));
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




MOM_PRIVATE const void *
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
#define NANOEVAL_HASHMOD_MOM 239
  switch (opitm->hva_hash % NANOEVAL_HASHMOD_MOM)
    {
#define OPITM_NANOEVALNODE_MOM(Nam) momhashpredef_##Nam % NANOEVAL_HASHMOD_MOM: \
    if (opitm == MOM_PREDEFITM(Nam)) goto foundcase_##Nam; goto defaultcase; foundcase_##Nam
      //////
    case OPITM_NANOEVALNODE_MOM (display):     ///// %display(<items...>)
      return nanoeval_displaynode_mom (nev, envitm, nod, depth);
    case OPITM_NANOEVALNODE_MOM (get): ///// %get(<item>,<itemattr>[,<default>]) or %get(<seq>,<rank>[,<default>])
      return nanoeval_getnode_mom (nev, envitm, nod, depth);
    case OPITM_NANOEVALNODE_MOM (cond):        ///// %cond(<when...>)
      return nanoeval_condnode_mom (nev, envitm, nod, depth);
    case OPITM_NANOEVALNODE_MOM (switch_item): ///// %switch_item(<item-expr>,<when...>)
      return nanoeval_switchitemnode_mom (nev, envitm, nod, depth);
    case OPITM_NANOEVALNODE_MOM (assign):      ///// %assign(<var>,<expr>...)
      return nanoeval_assignnode_mom (nev, envitm, nod, depth);
    case OPITM_NANOEVALNODE_MOM (or):  ///// %or()
      return nanoeval_ornode_mom (nev, envitm, nod, depth);
    case OPITM_NANOEVALNODE_MOM (and): ///// %and()
      return nanoeval_andnode_mom (nev, envitm, nod, depth);
    case OPITM_NANOEVALNODE_MOM (verbatim):    ///// %verbatim(it)
      return nanoeval_verbatimnode_mom (nev, envitm, nod, depth);
    case OPITM_NANOEVALNODE_MOM (item):        ///// %item(it)
      return nanoeval_itemnode_mom (nev, envitm, nod, depth);
    case OPITM_NANOEVALNODE_MOM (transient):   ///// %transient(...)
      return nanoeval_transientnode_mom (nev, envitm, nod, depth);
    case OPITM_NANOEVALNODE_MOM (persistent):  ///// %persistent(...)
      return nanoeval_persistentnode_mom (nev, envitm, nod, depth);
    case OPITM_NANOEVALNODE_MOM (sequence):    ///// %sequence(...)
      return nanoeval_sequencenode_mom (nev, envitm, nod, depth);
    case OPITM_NANOEVALNODE_MOM (first):       ///// %first(...)
      return nanoeval_firstnode_mom (nev, envitm, nod, depth);
      case OPITM_NANOEVALNODE_MOM (while)
    :                          ///// %while(<cond>,...)
        return nanoeval_whilenode_mom (nev, envitm, nod, depth);
    case OPITM_NANOEVALNODE_MOM (func):        ///// %func(formals,body..)
      return nanoeval_funcnode_mom (nev, envitm, nod, depth);
    case OPITM_NANOEVALNODE_MOM (output):      //// %output(out,...)
      return nanoeval_outputnode_mom (nev, envitm, nod, depth);
    case OPITM_NANOEVALNODE_MOM (let): ///// %let(bindings... exprs...)
      return nanoeval_letnode_mom (nev, envitm, nod, depth);
    case OPITM_NANOEVALNODE_MOM (letrec):      ///// %letrec(bindings... exprs...)
      return nanoeval_letrecnode_mom (nev, envitm, nod, depth);
    case OPITM_NANOEVALNODE_MOM (nanoeval):    ///// can be
      ///// %nanoeval(<expr>)
      ///// or
      ///// %nanoeval(<env>,<expr>...)
      ///// or %nanoeval()
      ///// to get the
      ///// environment
      return nanoeval_nanoevalnode_mom (nev, envitm, nod, depth);
    case OPITM_NANOEVALNODE_MOM (nano_closure):
      return nod;
    defaultcase:
    default:
      return nanoeval_othernode_mom (nev, envitm, nod, depth);
    }
#undef NANOEVAL_HASHMOD_MOM
  MOM_FATAPRINTF ("unimplemented eval of node %s",
                  mom_value_cstring ((struct mom_hashedvalue_st *) nod));
}                               /* end of nanoeval_node_mom */



MOM_PRIVATE const void *
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





//////////////////////////////////////////// eof nanoeval.c
