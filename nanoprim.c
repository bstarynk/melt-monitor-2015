// file nanoprim.c

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

const char momsig_nanoeval_type1[] = "signature_nanoeval1";
const void *
momf_nanoeval_type1 (struct mom_nanoeval_st *nev,
                     struct mom_item_st *envitm,
                     int depth,
                     const struct mom_boxnode_st *expnod,
                     const struct mom_boxnode_st *closnod, const void *arg0)
{

  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
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

const char momsig_nanoeval_size1[] = "signature_nanoeval1";
const void *
momf_nanoeval_size1 (struct mom_nanoeval_st *nev,
                     struct mom_item_st *envitm,
                     int depth,
                     const struct mom_boxnode_st *expnod,
                     const struct mom_boxnode_st *closnod, const void *arg0)
{

  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_size1 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0));
  return mom_boxint_make (mom_size (arg0));
}                               /* end momf_nanoeval_size1 */


const char momsig_nanoeval_identity1[] = "signature_nanoeval1";
const void *
momf_nanoeval_identity1 (struct mom_nanoeval_st *nev,
                         struct mom_item_st *envitm,
                         int depth,
                         const struct mom_boxnode_st *expnod,
                         const struct mom_boxnode_st *closnod,
                         const void *arg0)
{
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
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
      const struct mom_boxtuple_st *tures =     //
        mom_boxtuple_make_arr2 (ln0,
                                (const struct mom_item_st **) (tu0->seqitem),   //
                                ln1,
                                (const struct mom_item_st **) (tu1->seqitem));
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
      const struct mom_boxset_st *sres = mom_boxset_intersection (set0, set1);
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
      const struct mom_boxset_st *sres = mom_boxset_difference (set0, set1);
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
  struct mom_item_st *itm0 = (struct mom_item_st *) arg0;
  mom_item_lock (itm0);
  if (ty1 == MOMITY_ITEM)
    {
      struct mom_item_st *itm1 = (struct mom_item_st *) arg1;
      mom_unsync_item_put_phys_attr (itm0, itm1, arg2);
      ok = true;
    }
  else if (ty1 == MOMITY_BOXINT && itm0->itm_pcomp != NULL)
    {
      int rk = mom_boxint_val_def (arg1, INT_MAX);
      unsigned ln = mom_vectvaldata_count (itm0->itm_pcomp);
      if (rk < 0)
        rk += ln;
      if (rk >= 0 && rk < (int) ln)
        {
          mom_vectvaldata_put_nth (itm0->itm_pcomp, rk, arg2);
          ok = true;
        }
    }
  time (&itm0->itm_mtime);
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
  return hset;
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
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
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
}                               /* end of momf_nanoeval_setany */



const char momsig_nanoeval_nodeany[] = "signature_nanoevalany";
const void *
momf_nanoeval_nodeany (struct mom_nanoeval_st *nev,
                       struct mom_item_st *envitm,
                       int depth,
                       const struct mom_boxnode_st *expnod,
                       const struct mom_boxnode_st *closnod,
                       unsigned nbval, const void **valarr)
{
  MOM_DEBUGPRINTF (run,
                   "nanoeval_nodeany start envitm=%s depth=%d expnod=%s closnod=%s nbval=%d",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   nbval);
  for (int ix = 0; ix < (int) nbval; ix++)
    {
      MOM_DEBUGPRINTF (run, "nanoeval_nodeany valarr[%d]=%s", ix,
                       mom_value_cstring (valarr[ix]));
    }
  if (nbval == 0)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (arity), 1,
                                               mom_boxint_make (nbval)));
  struct mom_item_st *connitm = mom_dyncast_item (valarr[0]);
  if (!connitm)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                               valarr[0]));
  const struct mom_boxnode_st *resnod = mom_boxnode_make (connitm, nbval - 1,
                                                          (const struct
                                                           mom_hashedvalue_st
                                                           **) (valarr + 1));
  MOM_DEBUGPRINTF (run, "nanoeval_nodeany depth#%d resnod=%s", depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) resnod));
  return resnod;

}                               /* end of momf_nanoeval_nodeany */

const char momsig_nanoeval_flattennodeany[] = "signature_nanoevalany";
const void *
momf_nanoeval_flattennodeany (struct mom_nanoeval_st *nev,
                              struct mom_item_st *envitm,
                              int depth,
                              const struct mom_boxnode_st *expnod,
                              const struct mom_boxnode_st *closnod,
                              unsigned nbval, const void **valarr)
{
  const struct mom_boxnode_st *resnod = NULL;
  MOM_DEBUGPRINTF (run,
                   "nanoeval_flattennodeany start envitm=%s depth=%d expnod=%s closnod=%s nbval=%d",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   nbval);
  for (int ix = 0; ix < (int) nbval; ix++)
    {
      MOM_DEBUGPRINTF (run, "nanoeval_flattennodeany valarr[%d]=%s", ix,
                       mom_value_cstring (valarr[ix]));
    }
  if (nbval < 2)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (arity), 1,
                                               mom_boxint_make (nbval)));
  struct mom_item_st *connitm = mom_dyncast_item (valarr[0]);
  if (nbval == 2 && connitm)
    {                           // special case for two arguments
      const struct mom_seqitems_st *seq = mom_dyncast_seqitems (valarr[1]);
      if (seq)
        {
          MOM_DEBUGPRINTF (run, "nanoeval_flattennodeany seq=%s",
                           mom_value_cstring ((const struct mom_hashedvalue_st
                                               *) seq));
          unsigned len = mom_size (seq);
          resnod =
            mom_boxnode_make (connitm, len,
                              ((const struct mom_hashedvalue_st **)
                               seq->seqitem));
          goto end;
        }
    }

  struct mom_item_st *flatitm = mom_dyncast_item (valarr[1]);
  if (!connitm || !flatitm)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 2,
                                               valarr[0], valarr[1]));
  unsigned len = 0;
  for (unsigned ix = 2; ix < nbval; ix++)
    {
      const struct mom_boxnode_st *curnod = mom_dyncast_node (valarr[ix]);
      if (curnod && curnod->nod_connitm == flatitm)
        len += mom_size (curnod);
      else
        len++;
    }
  const void *smallarr[16] = { NULL };
  const void **arr =
    (len <
     sizeof (smallarr) / sizeof (void *))? smallarr : mom_gc_alloc ((len +
                                                                     1) *
                                                                    sizeof
                                                                    (void *));
  unsigned cnt = 0;
  for (unsigned ix = 2; ix < nbval; ix++)
    {
      assert (cnt < len);
      const struct mom_boxnode_st *curnod = mom_dyncast_node (valarr[ix]);
      if (curnod && curnod->nod_connitm == flatitm)
        {
          unsigned nodsiz = mom_size (curnod);
          assert (cnt + nodsiz < len);
          for (unsigned j = 0; j < nodsiz; j++)
            arr[cnt++] = curnod->nod_sons[j];
        }
      else
        arr[cnt++] = valarr[ix];
    }
  assert (cnt == len);
  resnod =
    mom_boxnode_make (connitm, len, (const struct mom_hashedvalue_st **) arr);
end:
  MOM_DEBUGPRINTF (run, "nanoeval_flattennodeany depth#%d resnod=%s",
                   depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) resnod));
  return resnod;
}                               /* end of momf_nanoeval_flattennodeany */



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
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
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




////////////////////////////////////////////////////////////////
//// assovaldata payload item support
const char momsig_nanoeval_payl_assovaldata1[] = "signature_nanoeval1";
const void *
momf_nanoeval_payl_assovaldata1 (struct mom_nanoeval_st *nev,
                                 struct mom_item_st *envitm,
                                 int depth,
                                 const struct mom_boxnode_st *expnod,
                                 const struct mom_boxnode_st *closnod,
                                 const void *arg0)
{
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_payl_assovaldata1 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0));
  struct mom_item_st *itm = mom_dyncast_item (arg0);
  if (!itm)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                               arg0));
  bool ok = false;
  mom_item_lock (itm);
  ok = mom_unsync_item_clear_payload (itm);
  if (ok)
    itm->itm_payload = (void *) mom_assovaldata_reserve (NULL, 5);
  time (&itm->itm_mtime);
  mom_item_unlock (itm);
  if (ok)
    return itm;
  NANOEVAL_FAILURE_MOM (nev, expnod,
                        mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                             arg0));
}                               // end of nanoeval_payl_assovaldata1 



const char momsig_nanoeval_payl_assovaldata_set_attrs1[] =
  "signature_nanoeval1";
const void *
momf_nanoeval_payl_assovaldata_set_attrs1 (struct mom_nanoeval_st *nev,
                                           struct mom_item_st *envitm,
                                           int depth,
                                           const struct mom_boxnode_st
                                           *expnod,
                                           const struct mom_boxnode_st
                                           *closnod, const void *arg0)
{
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_payl_assovaldata_set_attrs1 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0));
  struct mom_item_st *itm = mom_dyncast_item (arg0);
  if (!itm)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                               arg0));
  bool ok = false;
  const void *res = NULL;
  mom_item_lock (itm);
  ok = mom_itype (itm->itm_payload) == MOMITY_ASSOVALDATA;
  if (ok)
    res =
      mom_assovaldata_set_attrs ((const struct mom_assovaldata_st *)
                                 itm->itm_payload);
  mom_item_unlock (itm);
  if (ok)
    return res;
  NANOEVAL_FAILURE_MOM (nev, expnod,
                        mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                             arg0));
}                               // end of nanoeval_payl_assovaldata_set_attrs1 


const char momsig_nanoeval_payl_assovaldata_get2[] = "signature_nanoeval2";
const void *
momf_nanoeval_payl_assovaldata_get2 (struct mom_nanoeval_st *nev,
                                     struct mom_item_st *envitm,
                                     int depth,
                                     const struct mom_boxnode_st *expnod,
                                     const struct mom_boxnode_st *closnod,
                                     const void *arg0, const void *arg1)
{
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_payl_assovaldata_get2 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s arg1=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0), mom_value_cstring (arg1));
  struct mom_item_st *itm = mom_dyncast_item (arg0);
  struct mom_item_st *itmat = mom_dyncast_item (arg1);
  if (!itm || !itmat)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 2,
                                               arg0, arg1));
  bool ok = false;
  void *res = NULL;
  mom_item_lock (itm);
  ok = mom_itype (itm->itm_payload) == MOMITY_ASSOVALDATA;
  if (ok)
    res =
      mom_assovaldata_get ((const struct mom_assovaldata_st *)
                           itm->itm_payload, itmat);
  mom_item_unlock (itm);
  if (ok)
    return res;
  NANOEVAL_FAILURE_MOM (nev, expnod,
                        mom_boxnode_make_va (MOM_PREDEFITM (type_error), 2,
                                             arg0, arg1));
}                               // end of momf_nanoeval_payl_assovaldata_get2


const char momsig_nanoeval_payl_assovaldata_remove2[] = "signature_nanoeval2";
const void *
momf_nanoeval_payl_assovaldata_remove2 (struct mom_nanoeval_st *nev,
                                        struct mom_item_st *envitm,
                                        int depth,
                                        const struct mom_boxnode_st *expnod,
                                        const struct mom_boxnode_st *closnod,
                                        const void *arg0, const void *arg1)
{
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_payl_assovaldata_remove2 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s arg1=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0), mom_value_cstring (arg1));
  struct mom_item_st *itm = mom_dyncast_item (arg0);
  struct mom_item_st *itmat = mom_dyncast_item (arg1);
  const struct mom_boxset_st *setat = mom_dyncast_set (arg1);
  if (!itm || (!itmat && !setat))
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 2,
                                               arg0, arg1));
  bool ok = false;
  mom_item_lock (itm);
  ok = mom_itype (itm->itm_payload) == MOMITY_ASSOVALDATA;
  if (ok)
    {
      if (itmat)
        itm->itm_payload =
          (void *) mom_assovaldata_remove ((struct mom_assovaldata_st *)
                                           itm->itm_payload, itmat);
      else if (setat)
        {
          struct mom_assovaldata_st *ass =
            (struct mom_assovaldata_st *) itm->itm_payload;
          unsigned siz = mom_size (setat);
          for (unsigned ix = 0; ix < siz; ix++)
            ass = mom_assovaldata_remove (ass, setat->seqitem[ix]);
          itm->itm_payload = (void *) ass;
        }
      time (&itm->itm_mtime);
    }
  mom_item_unlock (itm);
  if (ok)
    return itm;
  NANOEVAL_FAILURE_MOM (nev, expnod,
                        mom_boxnode_make_va (MOM_PREDEFITM (type_error), 2,
                                             arg0, arg1));
}                               // end of momf_nanoeval_payl_assovaldata_remove2

const char momsig_nanoeval_payl_assovaldata_putany[] =
  "signature_nanoevalany";
const void *
momf_nanoeval_payl_assovaldata_putany (struct mom_nanoeval_st *nev,
                                       struct mom_item_st *envitm,
                                       int depth,
                                       const struct mom_boxnode_st *expnod,
                                       const struct mom_boxnode_st *closnod,
                                       unsigned nbval, const void **valarr)
{
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_payl_assovaldata_putany start envitm=%s depth=%d expnod=%s closnod=%s nbval=%d",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   nbval);
  if (nbval % 2 == 0)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (arity), 1,
                                               mom_boxint_make (nbval)));
  struct mom_item_st *itm = mom_dyncast_item (valarr[0]);
  if (!itm)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                               valarr[0]));
  for (unsigned ix = 1; ix < nbval; ix += 2)
    {
      struct mom_item_st *itmat = mom_dyncast_item (valarr[ix]);
      if (!itmat)
        NANOEVAL_FAILURE_MOM (nev, expnod,
                              mom_boxnode_make_va (MOM_PREDEFITM (type_error),
                                                   2, mom_boxint_make (ix),
                                                   valarr[ix]));
    }
  bool ok = false;
  mom_item_lock (itm);
  ok = mom_itype (itm->itm_payload) == MOMITY_ASSOVALDATA;
  if (ok)
    {
      struct mom_assovaldata_st *ass =
        (struct mom_assovaldata_st *) itm->itm_payload;
      ass = mom_assovaldata_reserve (ass, nbval / 2 + nbval / 8 + 1);
      for (unsigned ix = 1; ix < nbval; ix += 2)
        {
          struct mom_item_st *itmat = mom_dyncast_item (valarr[ix]);
          assert (ix + 1 < nbval);
          const void *curv = valarr[ix + 1];
          assert (itmat != NULL);
          ass = mom_assovaldata_put (ass, itmat, curv);
        }
      itm->itm_payload = (void *) ass;
      time (&itm->itm_mtime);
    }
  mom_item_unlock (itm);
  if (ok)
    return itm;
  NANOEVAL_FAILURE_MOM (nev, expnod,
                        mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                             valarr[0]));
}                               /* end of momf_nanoeval_payl_assovaldata_putany  */


////////////////////////////////////////////////////////////////
//// vectvaldata payload item support
const char momsig_nanoeval_payl_vectvaldata1[] = "signature_nanoeval1";
const void *
momf_nanoeval_payl_vectvaldata1 (struct mom_nanoeval_st *nev,
                                 struct mom_item_st *envitm,
                                 int depth,
                                 const struct mom_boxnode_st *expnod,
                                 const struct mom_boxnode_st *closnod,
                                 const void *arg0)
{
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_payl_vectvaldata1 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0));
  struct mom_item_st *itm = mom_dyncast_item (arg0);
  if (!itm)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                               arg0));
  bool ok = false;
  mom_item_lock (itm);
  ok = mom_unsync_item_clear_payload (itm);
  if (ok)
    {
      itm->itm_payload = (void *) mom_vectvaldata_reserve (NULL, 5);
      time (&itm->itm_mtime);
    }
  mom_item_unlock (itm);
  if (ok)
    return itm;
  NANOEVAL_FAILURE_MOM (nev, expnod,
                        mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                             arg0));
}                               // end of nanoeval_payl_vectvaldata1 


const char momsig_nanoeval_payl_vectvaldata_reserve2[] =
  "signature_nanoeval2";
const void *
momf_nanoeval_payl_vectvaldata_reserve2 (struct mom_nanoeval_st *nev,
                                         struct mom_item_st *envitm,
                                         int depth,
                                         const struct mom_boxnode_st *expnod,
                                         const struct mom_boxnode_st *closnod,
                                         const void *arg0, const void *arg1)
{
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_payl_vectvaldata_reserve2 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s arg1=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0), mom_value_cstring (arg1));
  struct mom_item_st *itm = mom_dyncast_item (arg0);
  if (!itm)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                               arg0));
  intptr_t n = mom_boxint_val_def (arg1, -1);
  bool ok = false;
  mom_item_lock (itm);
  ok = n >= 0 && mom_itype (itm->itm_payload) == MOMITY_VECTVALDATA;
  if (ok)
    {
      itm->itm_payload =        //
        (void *) mom_vectvaldata_reserve ((struct mom_vectvaldata_st *)
                                          itm->itm_payload, n);
      time (&itm->itm_mtime);
    }
  mom_item_unlock (itm);
  if (ok)
    return itm;
  NANOEVAL_FAILURE_MOM (nev, expnod,
                        mom_boxnode_make_va (MOM_PREDEFITM (type_error), 2,
                                             arg0, arg1));
}                               // end of nanoeval_payl_vectvaldata_reserve2


const char momsig_nanoeval_payl_vectvaldata_nth2[] = "signature_nanoeval2";
const void *
momf_nanoeval_payl_vectvaldata_nth2 (struct mom_nanoeval_st *nev,
                                     struct mom_item_st *envitm,
                                     int depth,
                                     const struct mom_boxnode_st *expnod,
                                     const struct mom_boxnode_st *closnod,
                                     const void *arg0, const void *arg1)
{
  void *res = NULL;
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_payl_vectvaldata_nth2 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s arg1=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0), mom_value_cstring (arg1));
  struct mom_item_st *itm = mom_dyncast_item (arg0);
  if (!itm || mom_itype (arg1) != MOMITY_BOXINT)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 2,
                                               arg0, arg1));
  intptr_t n = mom_boxint_val_def (arg1, -1);
  bool ok = false;
  mom_item_lock (itm);
  ok = mom_itype (itm->itm_payload) == MOMITY_VECTVALDATA;
  if (ok)
    res =
      mom_vectvaldata_nth ((struct mom_vectvaldata_st *) itm->itm_payload, n);
  mom_item_unlock (itm);
  if (ok)
    return res;
  NANOEVAL_FAILURE_MOM (nev, expnod,
                        mom_boxnode_make_va (MOM_PREDEFITM (type_error), 2,
                                             arg0, arg1));
}                               // end of nanoeval_payl_vectvaldata_nth2



const char momsig_nanoeval_payl_vectvaldata_put_nth3[] =
  "signature_nanoeval3";
const void *
momf_nanoeval_payl_vectvaldata_put_nth3 (struct mom_nanoeval_st *nev,
                                         struct mom_item_st *envitm,
                                         int depth,
                                         const struct mom_boxnode_st *expnod,
                                         const struct mom_boxnode_st *closnod,
                                         const void *arg0,
                                         const void *arg1, const void *arg2)
{
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_payl_vectvaldata_put_nth3 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s arg1=%s arg2=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0),
                   mom_value_cstring (arg1), mom_value_cstring (arg2));
  struct mom_item_st *itm = mom_dyncast_item (arg0);
  if (!itm || mom_itype (arg1) != MOMITY_BOXINT)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 2,
                                               arg0, arg1));
  intptr_t n = mom_boxint_val_def (arg1, -1);
  bool ok = false;
  mom_item_lock (itm);
  ok = mom_itype (itm->itm_payload) == MOMITY_VECTVALDATA;
  if (ok)
    {
      mom_vectvaldata_put_nth ((struct mom_vectvaldata_st *) itm->itm_payload,
                               n, arg2);
      time (&itm->itm_mtime);
    }
  mom_item_unlock (itm);
  if (ok)
    return itm;
  NANOEVAL_FAILURE_MOM (nev, expnod,
                        mom_boxnode_make_va (MOM_PREDEFITM (type_error), 2,
                                             arg0, arg1));
}                               // end of nanoeval_payl_vectvaldata_put_nth3


const char momsig_nanoeval_payl_vectvaldata_node2[] = "signature_nanoeval2";
const void *
momf_nanoeval_payl_vectvaldata_node2 (struct mom_nanoeval_st *nev,
                                      struct mom_item_st *envitm,
                                      int depth,
                                      const struct mom_boxnode_st *expnod,
                                      const struct mom_boxnode_st *closnod,
                                      const void *arg0, const void *arg1)
{
  const void *res = NULL;
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_payl_vectvaldata_node2 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s arg1=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0), mom_value_cstring (arg1));
  struct mom_item_st *itm = mom_dyncast_item (arg0);
  struct mom_item_st *itmconn = mom_dyncast_item (arg1);
  if (!itm || !itmconn)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 2,
                                               arg0, arg1));
  bool ok = false;
  mom_item_lock (itm);
  ok = mom_itype (itm->itm_payload) == MOMITY_VECTVALDATA;
  if (ok)
    {
      const struct mom_vectvaldata_st *vec =
        (const struct mom_vectvaldata_st *) itm->itm_payload;
      res =
        mom_boxnode_make (itmconn, mom_vectvaldata_count (vec),
                          (const struct mom_hashedvalue_st
                           **) (mom_vectvaldata_valvect (vec)));
    }
  mom_item_unlock (itm);
  if (ok)
    return res;
  NANOEVAL_FAILURE_MOM (nev, expnod,
                        mom_boxnode_make_va (MOM_PREDEFITM (type_error), 2,
                                             arg0, arg1));
}                               // end of nanoeval_payl_vectvaldata_node2


const char momsig_nanoeval_payl_vectvaldata_count1[] = "signature_nanoeval1";
const void *
momf_nanoeval_payl_vectvaldata_count1 (struct mom_nanoeval_st *nev,
                                       struct mom_item_st *envitm,
                                       int depth,
                                       const struct mom_boxnode_st *expnod,
                                       const struct mom_boxnode_st *closnod,
                                       const void *arg0)
{
  const void *res = NULL;
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_payl_vectvaldata_count1 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0));
  struct mom_item_st *itm = mom_dyncast_item (arg0);
  if (!itm)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                               arg0));
  bool ok = false;
  mom_item_lock (itm);
  ok = mom_itype (itm->itm_payload) == MOMITY_VECTVALDATA;
  if (ok)
    res =
      mom_boxint_make (mom_vectvaldata_count
                       ((const struct mom_vectvaldata_st *)
                        itm->itm_payload));
  mom_item_unlock (itm);
  if (ok)
    return res;
  NANOEVAL_FAILURE_MOM (nev, expnod,
                        mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                             arg0));
}                               // end of nanoeval_payl_vectvaldata_count1 


const char momsig_nanoeval_payl_vectvaldata_resize2[] = "signature_nanoeval2";
const void *
momf_nanoeval_payl_vectvaldata_resize2 (struct mom_nanoeval_st *nev,
                                        struct mom_item_st *envitm,
                                        int depth,
                                        const struct mom_boxnode_st *expnod,
                                        const struct mom_boxnode_st *closnod,
                                        const void *arg0, const void *arg1)
{
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_payl_vectvaldata_resize2 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s arg1=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0), mom_value_cstring (arg1));
  struct mom_item_st *itm = mom_dyncast_item (arg0);
  if (!itm)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                               arg0));
  intptr_t n = mom_boxint_val_def (arg1, -1);
  bool ok = false;
  mom_item_lock (itm);
  ok = n >= 0 && mom_itype (itm->itm_payload) == MOMITY_VECTVALDATA;
  if (ok)
    {
      itm->itm_payload =        //
        (void *) mom_vectvaldata_resize ((struct mom_vectvaldata_st *)
                                         itm->itm_payload, n);
      time (&itm->itm_mtime);
    }
  mom_item_unlock (itm);
  if (ok)
    return itm;
  NANOEVAL_FAILURE_MOM (nev, expnod,
                        mom_boxnode_make_va (MOM_PREDEFITM (type_error), 2,
                                             arg0, arg1));
}                               // end of nanoeval_payl_vectvaldata_resize2


const char momsig_nanoeval_payl_vectvaldata_appendany[] =
  "signature_nanoevalany";
const void *
momf_nanoeval_payl_vectvaldata_appendany (struct mom_nanoeval_st *nev,
                                          struct mom_item_st *envitm,
                                          int depth,
                                          const struct mom_boxnode_st *expnod,
                                          const struct mom_boxnode_st
                                          *closnod, unsigned nbval,
                                          const void **valarr)
{
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_payl_vectvaldata_appendany start envitm=%s depth=%d expnod=%s closnod=%s nbval=%d",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   nbval);
  if (nbval == 0)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (arity), 1,
                                               mom_boxint_make (nbval)));
  struct mom_item_st *itm = mom_dyncast_item (valarr[0]);
  if (!itm)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                               valarr[0]));
  bool ok = false;
  mom_item_lock (itm);
  ok = mom_itype (itm->itm_payload) == MOMITY_VECTVALDATA;
  if (ok)
    {
      struct mom_vectvaldata_st *vec =
        (struct mom_vectvaldata_st *) itm->itm_payload;
      vec = mom_vectvaldata_reserve (vec, nbval);
      for (unsigned ix = 1; ix < nbval; ix++)
        vec = mom_vectvaldata_append (vec, valarr[ix]);
      time (&itm->itm_mtime);
    }
  mom_item_unlock (itm);
  if (ok)
    return itm;
  NANOEVAL_FAILURE_MOM (nev, expnod,
                        mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                             valarr[0]));
}                               // end of momf_nanoeval_payl_vectvaldata_appendany 

const char momsig_nanoeval_payl_vectvaldata_append_content2[] =
  "signature_nanoeval2";
const void *
momf_nanoeval_payl_vectvaldata_append_content2 (struct mom_nanoeval_st *nev,
                                                struct mom_item_st *envitm,
                                                int depth,
                                                const struct mom_boxnode_st
                                                *expnod,
                                                const struct mom_boxnode_st
                                                *closnod, const void *arg0,
                                                const void *arg1)
{
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_payl_vectvaldata_append_content2 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s arg1=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0), mom_value_cstring (arg1));
  struct mom_item_st *itm = mom_dyncast_item (arg0);
  if (!itm)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 2,
                                               arg0, arg1));
  unsigned tyarg1 = mom_itype (arg1);
  bool ok = false;
  mom_item_lock (itm);
  ok = mom_itype (itm->itm_payload) == MOMITY_VECTVALDATA;
  if (ok)
    {
      struct mom_vectvaldata_st *vec =
        (struct mom_vectvaldata_st *) (itm->itm_payload);
      switch (tyarg1)
        {
        case MOMITY_TUPLE:
        case MOMITY_SET:
          {
            unsigned siz = mom_size (arg1);
            const struct mom_seqitems_st *seq =
              (const struct mom_seqitems_st *) arg1;
            vec = mom_vectvaldata_reserve (vec, siz);
            for (unsigned ix = 0; ix < siz; ix++)
              vec = mom_vectvaldata_append (vec, seq->seqitem[ix]);
          }
          break;
        case MOMITY_NODE:
          {
            unsigned siz = mom_size (arg1);
            const struct mom_boxnode_st *nod =
              (const struct mom_boxnode_st *) arg1;
            vec = mom_vectvaldata_reserve (vec, siz);
            for (unsigned ix = 0; ix < siz; ix++)
              vec = mom_vectvaldata_append (vec, nod->nod_sons[ix]);
          }
          break;
        case MOMITY_ITEM:
          {
            struct mom_item_st *secitm = mom_dyncast_item (arg1);
            mom_item_lock (secitm);
            ok = mom_itype (secitm->itm_payload) == MOMITY_VECTVALDATA;
            if (ok)
              {
                struct mom_vectvaldata_st *secvec =
                  (struct mom_vectvaldata_st *) (secitm->itm_payload);
                unsigned seccnt = mom_vectvaldata_count (secvec);
                for (unsigned ix = 0; ix < seccnt; ix++)
                  vec =
                    mom_vectvaldata_append (vec,
                                            mom_vectvaldata_nth (secvec, ix));
              }
            mom_item_unlock (secitm);
          }
          break;
        default:
          ok = false;
          break;
        }
      time (&itm->itm_mtime);
    }
  mom_item_unlock (itm);
  if (ok)
    return itm;
  NANOEVAL_FAILURE_MOM (nev, expnod,
                        mom_boxnode_make_va (MOM_PREDEFITM (type_error), 2,
                                             arg0, arg1));
}                               // end of nanoeval_payl_vectvaldata_append_content2


////////////////////////////////////////////////////////////////
//// hashset payload item support
const char momsig_nanoeval_payl_hashset1[] = "signature_nanoeval1";
const void *
momf_nanoeval_payl_hashset1 (struct mom_nanoeval_st *nev,
                             struct mom_item_st *envitm,
                             int depth,
                             const struct mom_boxnode_st *expnod,
                             const struct mom_boxnode_st *closnod,
                             const void *arg0)
{
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_payl_hashset1 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0));
  struct mom_item_st *itm = mom_dyncast_item (arg0);
  if (!itm)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                               arg0));
  bool ok = false;
  mom_item_lock (itm);
  ok = mom_unsync_item_clear_payload (itm);
  if (ok)
    {
      itm->itm_payload = (void *) mom_hashset_reserve (NULL, 5);
      time (&itm->itm_mtime);
    }
  mom_item_unlock (itm);
  if (ok)
    return itm;
  NANOEVAL_FAILURE_MOM (nev, expnod,
                        mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                             arg0));
}                               // end of nanoeval_payl_hashset1 

const char momsig_nanoeval_payl_hashset_reserve2[] = "signature_nanoeval2";
const void *
momf_nanoeval_payl_hashset_reserve2 (struct mom_nanoeval_st *nev,
                                     struct mom_item_st *envitm,
                                     int depth,
                                     const struct mom_boxnode_st *expnod,
                                     const struct mom_boxnode_st *closnod,
                                     const void *arg0, const void *arg1)
{
  MOM_DEBUGPRINTF (run,
                   "nanoeval_payl_hashset_reserve2 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s arg1=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0), mom_value_cstring (arg1));
  struct mom_item_st *itm = mom_dyncast_item (arg0);
  if (!itm)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                               arg0));
  intptr_t n = mom_boxint_val_def (arg1, -1);
  bool ok = false;
  mom_item_lock (itm);
  ok = n >= 0 && mom_itype (itm->itm_payload) == MOMITY_HASHSET;
  if (ok)
    {
      itm->itm_payload =        //
        (void *) mom_hashset_reserve ((struct mom_hashset_st *)
                                      itm->itm_payload, n);
      time (&itm->itm_mtime);
    }
  mom_item_unlock (itm);
  if (ok)
    return itm;
  NANOEVAL_FAILURE_MOM (nev, expnod,
                        mom_boxnode_make_va (MOM_PREDEFITM (type_error), 2,
                                             arg0, arg1));
}                               /* end of momf_nanoeval_payl_hashset_reserve2 */



const char momsig_nanoeval_payl_hashset_insert_content2[] =
  "signature_nanoeval2";
const void *momf_nanoeval_payl_hashset_insert_content2
  (struct mom_nanoeval_st *nev,
   struct mom_item_st *envitm,
   int depth,
   const struct mom_boxnode_st *expnod,
   const struct mom_boxnode_st *closnod, const void *arg0, const void *arg1)
{
  MOM_DEBUGPRINTF (run,
                   "nanoeval_payl_hashset_insert_content2 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s arg1=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0), mom_value_cstring (arg1));
  struct mom_item_st *itm = mom_dyncast_item (arg0);
  if (!itm)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                               arg0));
  unsigned ty1 = mom_itype (arg1);
  bool ok = false;
  mom_item_lock (itm);
  ok = mom_itype (itm->itm_payload) == MOMITY_HASHSET;
  if (ok)
    {
      struct mom_hashset_st *hs = (struct mom_hashset_st *) itm->itm_payload;
      switch (ty1)
        {
        case MOMITY_SET:
        case MOMITY_TUPLE:
          {
            const struct mom_seqitems_st *seq = arg1;
            unsigned ln = mom_size (seq);
            if (ln > 0)
              hs = mom_hashset_reserve (hs, ln);
            for (unsigned ix = 0; ix < ln; ix++)
              hs = mom_hashset_insert (hs, seq->seqitem[ix]);
            itm->itm_payload = (void *) hs;
          }
          break;
        case MOMITY_ITEM:
          {
            struct mom_item_st *secitm = (struct mom_item_st *) arg1;
            mom_item_lock (secitm);
            ok = mom_itype (secitm->itm_payload) == MOMITY_HASHSET;
            if (ok)
              {
                const struct mom_boxset_st *set2 =
                  mom_hashset_to_boxset ((const struct mom_hashset_st *)
                                         secitm->itm_payload);
                unsigned ln2 = mom_size (set2);
                for (unsigned ix = 0; ix < ln2; ix++)
                  hs = mom_hashset_insert (hs, set2->seqitem[ix]);
              }
            mom_item_unlock (secitm);
          }
          break;
        default:
          ok = false;
          break;
        }
      time (&itm->itm_mtime);
    }
  mom_item_unlock (itm);
  if (ok)
    return itm;
  NANOEVAL_FAILURE_MOM (nev, expnod,
                        mom_boxnode_make_va (MOM_PREDEFITM (type_error), 2,
                                             arg0, arg1));

}                               /* end of momf_nanoeval_payl_hashset_insert_content2 */



const char momsig_nanoeval_payl_hashset_insertany[] = "signature_nanoevalany";
const void *
momf_nanoeval_payl_hashset_insertany (struct mom_nanoeval_st *nev,
                                      struct mom_item_st *envitm,
                                      int depth,
                                      const struct mom_boxnode_st *expnod,
                                      const struct mom_boxnode_st *closnod,
                                      unsigned nbval, const void **valarr)
{
  MOM_DEBUGPRINTF (run,
                   "nanoeval_payl_hashset_insertany start envitm=%s depth=%d expnod=%s closnod=%s nbval=%d",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   nbval);
  if (nbval == 0)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (arity), 1,
                                               mom_boxint_make (nbval)));
  struct mom_item_st *itm = mom_dyncast_item (valarr[0]);
  if (!itm)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                               valarr[0]));
  bool ok = false;
  mom_item_lock (itm);
  ok = mom_itype (itm->itm_payload) == MOMITY_HASHSET;
  if (ok)
    {
      struct mom_hashset_st *hs = (struct mom_hashset_st *) itm->itm_payload;
      hs = mom_hashset_reserve (hs, 1 + 4 * nbval / 3);
      for (unsigned ix = 1; ix < nbval; ix++)
        {
          const void *curarg = valarr[ix];
          unsigned curty = mom_itype (curarg);
          switch (curty)
            {
            case MOMITY_SET:
            case MOMITY_TUPLE:
              {
                const struct mom_seqitems_st *seq = curarg;
                unsigned sz = mom_raw_size (seq);
                for (unsigned ix = 0; ix < sz; ix++)
                  hs = mom_hashset_insert (hs, seq->seqitem[ix]);
              }
              break;
            case MOMITY_ITEM:
              hs = mom_hashset_insert (hs, (struct mom_item_st *) curarg);
              break;
            default:
              continue;
            }
        }
      itm->itm_payload = (void *) hs;
      time (&itm->itm_mtime);
    }
  mom_item_unlock (itm);
  if (ok)
    return itm;
  else
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                               valarr[0]));

}                               /* end of momf_nanoeval_payl_hashset_insertany */


const char momsig_nanoeval_payl_hashset_removeany[] = "signature_nanoevalany";
const void *
momf_nanoeval_payl_hashset_removeany (struct mom_nanoeval_st *nev,
                                      struct mom_item_st *envitm,
                                      int depth,
                                      const struct mom_boxnode_st *expnod,
                                      const struct mom_boxnode_st *closnod,
                                      unsigned nbval, const void **valarr)
{
  MOM_DEBUGPRINTF (run,
                   "nanoeval_payl_hashset_insertany start envitm=%s depth=%d expnod=%s closnod=%s nbval=%d",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   nbval);
  if (nbval == 0)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (arity), 1,
                                               mom_boxint_make (nbval)));
  struct mom_item_st *itm = mom_dyncast_item (valarr[0]);
  if (!itm)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                               valarr[0]));
  bool ok = false;
  mom_item_lock (itm);
  ok = mom_itype (itm->itm_payload) == MOMITY_HASHSET;
  if (ok)
    {
      int nbrm = 0;
      struct mom_hashset_st *hs = (struct mom_hashset_st *) itm->itm_payload;
      for (unsigned ix = 1; ix < nbval; ix++)
        {
          const void *curarg = valarr[ix];
          unsigned curty = mom_itype (curarg);
          switch (curty)
            {
            case MOMITY_SET:
            case MOMITY_TUPLE:
              {
                const struct mom_seqitems_st *seq = curarg;
                unsigned sz = mom_raw_size (seq);
                for (unsigned ix = 0; ix < sz; ix++)
                  {
                    hs = mom_hashset_remove (hs, seq->seqitem[ix]);
                    nbrm++;
                  }
              }
              break;
            case MOMITY_ITEM:
              hs = mom_hashset_remove (hs, (struct mom_item_st *) curarg);
              nbrm++;
              break;
            default:
              continue;
            }
        }
      if (nbrm > (int) mom_hashset_count (hs) / 5)
        hs = mom_hashset_reserve (hs, 0);
      itm->itm_payload = (void *) hs;
      time (&itm->itm_mtime);
    }
  mom_item_unlock (itm);
  if (ok)
    return itm;
  else
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                               valarr[0]));

}                               /* end of momf_nanoeval_payl_hashset_removeany */


const char momsig_nanoeval_payl_hashset_remove_content2[] =
  "signature_nanoeval2";
const void *momf_nanoeval_payl_hashset_remove_content2
  (struct mom_nanoeval_st *nev,
   struct mom_item_st *envitm,
   int depth,
   const struct mom_boxnode_st *expnod,
   const struct mom_boxnode_st *closnod, const void *arg0, const void *arg1)
{
  MOM_DEBUGPRINTF (run,
                   "nanoeval_payl_hashset_remove_content2 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s arg1=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0), mom_value_cstring (arg1));
  struct mom_item_st *itm = mom_dyncast_item (arg0);
  if (!itm)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                               arg0));
  bool ok = false;
  mom_item_lock (itm);
  ok = mom_itype (itm->itm_payload) == MOMITY_HASHSET;
  if (ok)
    {
      int nbrm = 0;
      struct mom_hashset_st *hs = (struct mom_hashset_st *) itm->itm_payload;
      switch (mom_itype (arg1))
        {
        case MOMITY_SET:
        case MOMITY_TUPLE:
          {
            const struct mom_seqitems_st *seq = arg1;
            unsigned sz = mom_raw_size (seq);
            for (unsigned ix = 0; ix < sz; ix++)
              {
                hs = mom_hashset_remove (hs, seq->seqitem[ix]);
                nbrm++;
              }
          }
          break;
        case MOMITY_ITEM:
          {
            struct mom_item_st *secitm = (struct mom_item_st *) arg1;
            mom_item_lock (secitm);
            ok = mom_itype (secitm->itm_payload) == MOMITY_HASHSET;
            if (ok)
              {
                const struct mom_boxset_st *set2 =      //
                  mom_hashset_to_boxset ((struct mom_hashset_st *)
                                         secitm->itm_payload);
                unsigned ln2 = mom_size (set2);
                for (unsigned ix = 0; ix < ln2; ix++)
                  {
                    hs = mom_hashset_remove (hs, set2->seqitem[ix]);
                    nbrm++;
                  }
              }
            mom_item_unlock (secitm);
          }
          break;
        default:
          ok = false;
        }
      if (nbrm > (int) mom_size (hs) / 4)
        hs = mom_hashset_reserve (hs, 0);
      itm->itm_payload = (void *) hs;
      if (nbrm > 0)
        time (&itm->itm_mtime);
    }
  mom_item_unlock (itm);
  if (ok)
    return itm;
  else
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 2,
                                               arg0, arg1));
}                               /* end of momf_nanoeval_payl_hashset_remove_content2 */


const char momsig_nanoeval_payl_hashset_contains2[] = "signature_nanoeval2";
const void *
momf_nanoeval_payl_hashset_contains2 (struct mom_nanoeval_st *nev,
                                      struct mom_item_st *envitm,
                                      int depth,
                                      const struct mom_boxnode_st *expnod,
                                      const struct mom_boxnode_st *closnod,
                                      const void *arg0, const void *arg1)
{
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_payl_hashset_contains2 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s arg1=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0), mom_value_cstring (arg1));
  struct mom_item_st *itm = mom_dyncast_item (arg0);
  struct mom_item_st *elitm = mom_dyncast_item (arg1);
  if (!itm || !elitm)
    return NULL;
  bool ok = false;
  mom_item_lock (itm);
  ok = mom_itype (itm->itm_payload) == MOMITY_HASHSET;
  if (ok)
    {
      struct mom_hashset_st *hs = (struct mom_hashset_st *) itm->itm_payload;
      ok = mom_hashset_contains (hs, elitm);
    }
  mom_item_unlock (itm);
  return ok ? MOM_PREDEFITM (truth) : NULL;
}                               /* end of momf_nanoeval_payl_hashset_contains2 */


const char momsig_nanoeval_payl_hashset_to_set[] = "signature_nanoeval1";
const void *
momf_nanoeval_payl_hashset_to_set1 (struct mom_nanoeval_st *nev,
                                    struct mom_item_st *envitm,
                                    int depth,
                                    const struct mom_boxnode_st *expnod,
                                    const struct mom_boxnode_st *closnod,
                                    const void *arg0)
{
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_payl_hashset_to_set start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0));
  const void *res = NULL;
  struct mom_item_st *itm = mom_dyncast_item (arg0);
  if (!itm)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                               arg0));
  bool ok = false;
  mom_item_lock (itm);
  ok = mom_itype (itm->itm_payload) == MOMITY_HASHSET;
  if (ok)
    {
      struct mom_hashset_st *hs = (struct mom_hashset_st *) itm->itm_payload;
      res = mom_hashset_to_boxset (hs);
    }
  mom_item_unlock (itm);
  if (ok)
    return res;
  else
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                               arg0));
}                               /* end of momf_nanoeval_payl_hashset_to_set2 */



////////////////////////////////////////////////////////////////
//// hashmap payload item support
const char momsig_nanoeval_payl_hashmap1[] = "signature_nanoeval1";
const void *
momf_nanoeval_payl_hashmap1 (struct mom_nanoeval_st *nev,
                             struct mom_item_st *envitm,
                             int depth,
                             const struct mom_boxnode_st *expnod,
                             const struct mom_boxnode_st *closnod,
                             const void *arg0)
{
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_payl_hashmap1 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0));
  struct mom_item_st *itm = mom_dyncast_item (arg0);
  if (!itm)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                               arg0));
  bool ok = false;
  mom_item_lock (itm);
  ok = mom_unsync_item_clear_payload (itm);
  if (ok)
    itm->itm_payload = (void *) mom_hashmap_reserve (NULL, 5);
  mom_item_unlock (itm);
  if (ok)
    return itm;
  NANOEVAL_FAILURE_MOM (nev, expnod,
                        mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                             arg0));
}                               // end of nanoeval_payl_hashmap1 



const char momsig_nanoeval_payl_hashmap_get2[] = "signature_nanoeval2";
const void *
momf_nanoeval_payl_hashmap_get2 (struct mom_nanoeval_st *nev,
                                 struct mom_item_st *envitm,
                                 int depth,
                                 const struct mom_boxnode_st *expnod,
                                 const struct mom_boxnode_st *closnod,
                                 const void *arg0, const void *arg1)
{
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_payl_hashmap_get2 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s arg1=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0), mom_value_cstring (arg1));
  struct mom_item_st *itm = mom_dyncast_item (arg0);
  struct mom_item_st *atitm = mom_dyncast_item (arg1);
  bool ok = itm != NULL && atitm != NULL;
  if (!ok)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 2,
                                               arg0, arg1));
  const void *res = NULL;
  mom_item_lock (itm);
  ok = mom_itype (itm->itm_payload) == MOMITY_HASHMAP;
  if (ok)
    {
      struct mom_hashmap_st *hm = (struct mom_hashmap_st *) itm->itm_payload;
      res = mom_hashmap_get (hm, atitm);
    }
  mom_item_unlock (itm);
  if (ok)
    return res;
  else
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 2,
                                               arg0, arg1));

}                               /* end of momf_nanoeval_payl_hashmap_get2 */



const char momsig_nanoeval_payl_hashmap_putany[] = "signature_nanoevalany";
const void *
momf_nanoeval_payl_hashmap_putany (struct mom_nanoeval_st *nev,
                                   struct mom_item_st *envitm,
                                   int depth,
                                   const struct mom_boxnode_st *expnod,
                                   const struct mom_boxnode_st *closnod,
                                   unsigned nbval, const void **valarr)
{
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_payl_hashmap_put3 start envitm=%s depth=%d expnod=%s closnod=%s nbval#%d",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   nbval);
  if (nbval % 2 == 0)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (arity), 1,
                                               mom_boxint_make (nbval)));
  struct mom_item_st *itm = mom_dyncast_item (valarr[0]);
  if (!itm)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                               valarr[0]));
  bool ok = false;
  mom_item_lock (itm);
  ok = mom_itype (itm->itm_payload) == MOMITY_HASHMAP;
  for (unsigned ix = 1; ix < nbval && ok; ix += 2)
    ok = mom_itype (valarr[ix]) == MOMITY_ITEM;
  if (ok)
    {
      struct mom_hashmap_st *hm = (struct mom_hashmap_st *) itm->itm_payload;
      hm = mom_hashmap_reserve (hm, nbval / 2 + 2);
      for (unsigned ix = 1; ix < nbval && ok; ix += 2)
        {
          const struct mom_item_st *atitm = valarr[ix];
          assert (atitm && atitm->va_itype == MOMITY_ITEM);
          hm = mom_hashmap_put (hm, atitm, valarr[ix + 1]);
        }
      itm->itm_payload = (void *) hm;
      time (&itm->itm_mtime);
    }
  mom_item_unlock (itm);
  if (ok)
    return itm;
  else
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                               valarr[0]));
}                               /* end of momf_nanoeval_payl_hashmap_putany */


const char momsig_nanoeval_payl_hashmap_removeany[] = "signature_nanoevalany";
const void *
momf_nanoeval_payl_hashmap_removeany (struct mom_nanoeval_st *nev,
                                      struct mom_item_st *envitm,
                                      int depth,
                                      const struct mom_boxnode_st *expnod,
                                      const struct mom_boxnode_st *closnod,
                                      unsigned nbval, const void **valarr)
{
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_payl_hashmap_removeany start envitm=%s depth=%d expnod=%s closnod=%s nbval#%d",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   nbval);
  if (nbval == 0)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (arity), 1,
                                               mom_boxint_make (nbval)));
  struct mom_item_st *itm = mom_dyncast_item (valarr[0]);
  bool ok = itm != NULL;
  for (unsigned ix = 1; ix < nbval && ok; ix++)
    ok = mom_itype (valarr[ix]) == MOMITY_ITEM;
  if (!ok)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                               valarr[0]));
  mom_item_lock (itm);
  ok = mom_itype (itm->itm_payload) == MOMITY_HASHMAP;
  if (ok)
    {
      struct mom_hashmap_st *hm = (struct mom_hashmap_st *) itm->itm_payload;
      for (unsigned ix = 1; ix < nbval && ok; ix++)
        {
          const struct mom_item_st *atitm = valarr[ix];
          assert (atitm && atitm->va_itype == MOMITY_ITEM);
          hm = mom_hashmap_remove (hm, atitm);
        }
      if (nbval >= mom_hashmap_count (hm) / 3 + 1)
        hm = mom_hashmap_reserve (hm, 0);
      itm->itm_payload = (void *) hm;
      time (&itm->itm_mtime);
    }
  mom_item_unlock (itm);
  if (ok)
    return itm;
  else
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                               valarr[0]));
}                               /* end of momf_nanoeval_payl_hashmap_removeany */


const char momsig_nanoeval_payl_hashmap_keyset1[] = "signature_nanoeval1";
const void *
momf_nanoeval_payl_hashmap_keyset1 (struct mom_nanoeval_st *nev,
                                    struct mom_item_st *envitm,
                                    int depth,
                                    const struct mom_boxnode_st *expnod,
                                    const struct mom_boxnode_st *closnod,
                                    const void *arg0)
{
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_payl_hashmap_keyset1 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0));
  struct mom_item_st *itm = mom_dyncast_item (arg0);
  if (!itm)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                               arg0));
  bool ok = false;
  const void *res = NULL;
  mom_item_lock (itm);
  ok = mom_itype (itm->itm_payload) == MOMITY_HASHMAP;
  if (ok)
    {
      struct mom_hashmap_st *hm = (struct mom_hashmap_st *) itm->itm_payload;
      res = mom_hashmap_keyset (hm);
    }
  mom_item_unlock (itm);
  if (ok)
    return res;
  else
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                               arg0));
}                               /* end of momf_nanoeval_payl_hashmap_keyset1 */

const char momsig_nanoeval_payl_hashmap_count1[] = "signature_nanoeval1";
const void *
momf_nanoeval_payl_hashmap_count1 (struct mom_nanoeval_st *nev,
                                   struct mom_item_st *envitm,
                                   int depth,
                                   const struct mom_boxnode_st *expnod,
                                   const struct mom_boxnode_st *closnod,
                                   const void *arg0)
{
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_payl_hashmap_count1 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0));
  struct mom_item_st *itm = mom_dyncast_item (arg0);
  if (!itm)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                               arg0));
  bool ok = false;
  const void *res = NULL;
  mom_item_lock (itm);
  ok = mom_itype (itm->itm_payload) == MOMITY_HASHMAP;
  if (ok)
    {
      struct mom_hashmap_st *hm = (struct mom_hashmap_st *) itm->itm_payload;
      res = mom_boxint_make (mom_hashmap_count (hm));
    }
  mom_item_unlock (itm);
  if (ok)
    return res;
  else
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                               arg0));
}                               /* end of momf_nanoeval_payl_hashmap_count1 */

////////////////////////////////////////////////////////////////
//// hashassoc payload item support
const char momsig_nanoeval_payl_hashassoc1[] = "signature_nanoeval1";
const void *
momf_nanoeval_payl_hashassoc1 (struct mom_nanoeval_st *nev,
                               struct mom_item_st *envitm,
                               int depth,
                               const struct mom_boxnode_st *expnod,
                               const struct mom_boxnode_st *closnod,
                               const void *arg0)
{
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_payl_hashassoc1 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0));
  struct mom_item_st *itm = mom_dyncast_item (arg0);
  if (!itm)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                               arg0));
  bool ok = false;
  mom_item_lock (itm);
  ok = mom_unsync_item_clear_payload (itm);
  if (ok)
    {
      itm->itm_payload = (void *) mom_hashassoc_reserve (NULL, 5);
      time (&itm->itm_mtime);
    }
  mom_item_unlock (itm);
  if (ok)
    return itm;
  NANOEVAL_FAILURE_MOM (nev, expnod,
                        mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                             arg0));
}                               // end of nanoeval_payl_hashassoc1 



const char momsig_nanoeval_payl_hashassoc_get2[] = "signature_nanoeval2";
const void *
momf_nanoeval_payl_hashassoc_get2 (struct mom_nanoeval_st *nev,
                                   struct mom_item_st *envitm,
                                   int depth,
                                   const struct mom_boxnode_st *expnod,
                                   const struct mom_boxnode_st *closnod,
                                   const void *arg0, const void *arg1)
{
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_payl_hashassoc_get2 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s arg1=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0), mom_value_cstring (arg1));
  struct mom_item_st *itm = mom_dyncast_item (arg0);
  bool ok = itm != NULL && arg1 != NULL;
  if (!ok)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 2,
                                               arg0, arg1));
  const void *res = NULL;
  mom_item_lock (itm);
  ok = mom_itype (itm->itm_payload) == MOMITY_HASHASSOC;
  if (ok)
    {
      struct mom_hashassoc_st *hm =
        (struct mom_hashassoc_st *) itm->itm_payload;
      res = mom_hashassoc_get (hm, arg1);
    }
  mom_item_unlock (itm);
  if (ok)
    return res;
  else
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 2,
                                               arg0, arg1));

}                               /* end of momf_nanoeval_payl_hashassoc_get2 */



const char momsig_nanoeval_payl_hashassoc_putany[] = "signature_nanoevalany";
const void *
momf_nanoeval_payl_hashassoc_putany (struct mom_nanoeval_st *nev,
                                     struct mom_item_st *envitm,
                                     int depth,
                                     const struct mom_boxnode_st *expnod,
                                     const struct mom_boxnode_st *closnod,
                                     unsigned nbval, const void **valarr)
{
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_payl_hashassoc_put3 start envitm=%s depth=%d expnod=%s closnod=%s nbval#%d",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   nbval);
  if (nbval % 2 == 0)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (arity), 1,
                                               mom_boxint_make (nbval)));
  struct mom_item_st *itm = mom_dyncast_item (valarr[0]);
  if (!itm)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                               valarr[0]));
  bool ok = false;
  mom_item_lock (itm);
  ok = mom_itype (itm->itm_payload) == MOMITY_HASHASSOC;
  for (unsigned ix = 1; ix < nbval && ok; ix += 2)
    ok = mom_itype (valarr[ix]) != MOMITY_NONE;
  if (ok)
    {
      struct mom_hashassoc_st *ha =
        (struct mom_hashassoc_st *) itm->itm_payload;
      ha = mom_hashassoc_reserve (ha, nbval / 2 + 2);
      for (unsigned ix = 1; ix < nbval && ok; ix += 2)
        {
          const void *at = valarr[ix];
          assert (at);
          ha = mom_hashassoc_put (ha, at, valarr[ix + 1]);
        }
      itm->itm_payload = (void *) ha;
      time (&itm->itm_mtime);
    }
  mom_item_unlock (itm);
  if (ok)
    return itm;
  else
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                               valarr[0]));
}                               /* end of momf_nanoeval_payl_hashassoc_putany */


const char momsig_nanoeval_payl_hashassoc_removeany[] =
  "signature_nanoevalany";
const void *
momf_nanoeval_payl_hashassoc_removeany (struct mom_nanoeval_st *nev,
                                        struct mom_item_st *envitm,
                                        int depth,
                                        const struct mom_boxnode_st *expnod,
                                        const struct mom_boxnode_st *closnod,
                                        unsigned nbval, const void **valarr)
{
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_payl_hashassoc_removeany start envitm=%s depth=%d expnod=%s closnod=%s nbval#%d",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   nbval);
  if (nbval == 0)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (arity), 1,
                                               mom_boxint_make (nbval)));
  struct mom_item_st *itm = mom_dyncast_item (valarr[0]);
  bool ok = itm != NULL;
  for (unsigned ix = 1; ix < nbval && ok; ix++)
    ok = mom_itype (valarr[ix]) != MOMITY_NONE;
  if (!ok)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                               valarr[0]));
  mom_item_lock (itm);
  ok = mom_itype (itm->itm_payload) == MOMITY_HASHASSOC;
  if (ok)
    {
      struct mom_hashassoc_st *ha =
        (struct mom_hashassoc_st *) itm->itm_payload;
      for (unsigned ix = 1; ix < nbval && ok; ix++)
        {
          const void *at = valarr[ix];
          assert (at != NULL);
          ha = mom_hashassoc_remove (ha, at);
        }
      if (nbval >= mom_hashassoc_count (ha) / 3 + 1)
        ha = mom_hashassoc_reserve (ha, 0);
      itm->itm_payload = (void *) ha;
      time (&itm->itm_mtime);
    }
  mom_item_unlock (itm);
  if (ok)
    return itm;
  else
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                               valarr[0]));
}                               /* end of momf_nanoeval_payl_hashassoc_removeany */




const char momsig_nanoeval_payl_hashassoc_sorted_key_node2[] =
  "signature_nanoeval2";
const void *
momf_nanoeval_payl_hashassoc_sorted_key_node2 (struct mom_nanoeval_st *nev,
                                               struct mom_item_st *envitm,
                                               int depth,
                                               const struct mom_boxnode_st
                                               *expnod,
                                               const struct mom_boxnode_st
                                               *closnod, const void *arg0,
                                               const void *arg1)
{
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_payl_hashassoc_sorted_key_node2 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s arg1=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0), mom_value_cstring (arg1));
  struct mom_item_st *itm = mom_dyncast_item (arg0);
  struct mom_item_st *connitm = mom_dyncast_item (arg1);
  bool ok = itm != NULL && connitm != NULL;
  if (!ok)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 2,
                                               arg0, arg1));
  const void *res = NULL;
  mom_item_lock (itm);
  ok = mom_itype (itm->itm_payload) == MOMITY_HASHASSOC;
  if (ok)
    {
      struct mom_hashassoc_st *hm =
        (struct mom_hashassoc_st *) itm->itm_payload;
      res = mom_hashassoc_sorted_key_node (hm, connitm);
    }
  mom_item_unlock (itm);
  if (ok)
    return res;
  else
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 2,
                                               arg0, arg1));

}                               /* end of momf_nanoeval_payl_hashassoc_sorted_key_node2 */


const char momsig_nanoeval_payl_hashassoc_count1[] = "signature_nanoeval1";
const void *
momf_nanoeval_payl_hashassoc_count1 (struct mom_nanoeval_st *nev,
                                     struct mom_item_st *envitm,
                                     int depth,
                                     const struct mom_boxnode_st *expnod,
                                     const struct mom_boxnode_st *closnod,
                                     const void *arg0)
{
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_payl_hashassoc_count1 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0));
  struct mom_item_st *itm = mom_dyncast_item (arg0);
  if (!itm)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                               arg0));
  bool ok = false;
  const void *res = NULL;
  mom_item_lock (itm);
  ok = mom_itype (itm->itm_payload) == MOMITY_HASHASSOC;
  if (ok)
    {
      struct mom_hashassoc_st *ha =
        (struct mom_hashassoc_st *) itm->itm_payload;
      res = mom_boxint_make (mom_hashassoc_count (ha));
    }
  mom_item_unlock (itm);
  if (ok)
    return res;
  else
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                               arg0));
}                               /* end of momf_nanoeval_payl_hashassoc_count1 */


////////////////
const char momsig_nanoeval_clone1[] = "signature_nanoeval1";
const void *
momf_nanoeval_clone1 (struct mom_nanoeval_st *nev,
                      struct mom_item_st *envitm,
                      int depth,
                      const struct mom_boxnode_st *expnod,
                      const struct mom_boxnode_st *closnod, const void *arg0)
{

  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_clone1 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0));
  struct mom_item_st *itm = mom_dyncast_item (arg0);
  if (!itm)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                               arg0));
  struct mom_item_st *clonitm = mom_clone_item (itm);
  time (&clonitm->itm_mtime);

  if (!nev->nanev_transient)
    mom_item_put_space (clonitm, MOMSPA_GLOBAL);
  return clonitm;
}                               /* end of momf_nanoeval_clone1 */

////////////////
////////////////
const char momsig_nanoeval_payl_clear1[] = "signature_nanoeval1";
const void *
momf_nanoeval_payl_clear1 (struct mom_nanoeval_st *nev,
                           struct mom_item_st *envitm,
                           int depth,
                           const struct mom_boxnode_st *expnod,
                           const struct mom_boxnode_st *closnod,
                           const void *arg0)
{

  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_payl_clear1 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0));
  struct mom_item_st *itm = mom_dyncast_item (arg0);
  if (!itm)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                               arg0));
  bool ok = false;
  mom_item_lock (itm);
  ok = mom_unsync_item_clear_payload (itm);
  if (ok)
    time (&itm->itm_mtime);

  mom_item_unlock (itm);
  if (ok)
    return (itm);
  NANOEVAL_FAILURE_MOM (nev, expnod,
                        mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                             arg0));
}                               /* end of momf_nanoeval_payl_clear1 */

////////////////


//// filebuffer payload item support
const char momsig_nanoeval_payl_filebuffer1[] = "signature_nanoeval1";
const void *
momf_nanoeval_payl_filebuffer1 (struct mom_nanoeval_st *nev,
                                struct mom_item_st *envitm,
                                int depth,
                                const struct mom_boxnode_st *expnod,
                                const struct mom_boxnode_st *closnod,
                                const void *arg0)
{
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_payl_filebuffer1 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0));
  struct mom_item_st *itm = mom_dyncast_item (arg0);
  if (!itm)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                               arg0));
  bool ok = false;
  mom_item_lock (itm);
  ok = mom_unsync_item_clear_payload (itm);
  if (ok)
    {
      itm->itm_payload = (void *) mom_make_filebuffer ();
      time (&itm->itm_mtime);
    }
  mom_item_unlock (itm);
  if (ok)
    return itm;
  NANOEVAL_FAILURE_MOM (nev, expnod,
                        mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                             arg0));
}                               // end of nanoeval_payl_filebuffer1 


//// file payload item support
const char momsig_nanoeval_payl_file_write2[] = "signature_nanoeval2";
const void *
momf_nanoeval_payl_file_write2 (struct mom_nanoeval_st *nev,
                                struct mom_item_st *envitm,
                                int depth,
                                const struct mom_boxnode_st *expnod,
                                const struct mom_boxnode_st *closnod,
                                const void *arg0, const void *arg1)
{
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_payl_file_write2 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0));
  struct mom_item_st *itm = mom_dyncast_item (arg0);
  const char *path = mom_boxstring_cstr (arg1);
  if (!itm || !path)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 2,
                                               arg0, arg1));
  bool backupok = false;
  char backupath[MOM_PATH_MAX];
  memset (backupath, 0, sizeof (backupath));
  if (!access (path, F_OK))
    {
      if (snprintf (backupath, sizeof (backupath), "%s~", path) <
          MOM_PATH_MAX)
        {
          if (rename (path, backupath))
            MOM_WARNPRINTF ("failed to backup %s to %s~ : %s",
                            path, path, strerror (errno));
          else
            backupok = true;
        }
    }
  FILE *fout = fopen (path, "w");
  if (!fout)
    {
      int e = errno;
      MOM_WARNPRINTF ("failed to open %s for write (%s)", path, strerror (e));
      if (backupok)
        {
          if (rename (backupath, path))
            MOM_FATAPRINTF ("failed to revert backup %s to %s: %s",
                            backupath, path, strerror (errno));
        }
      NANOEVAL_FAILURE_MOM (nev, expnod,
                            mom_boxnode_make_va (MOM_PREDEFITM (the_system),
                                                 2,
                                                 mom_boxstring_make (strerror
                                                                     (e)),
                                                 arg1));
    }
  bool ok = false;
  mom_item_lock (itm);
  ok = mom_unsync_item_clear_payload (itm);
  if (ok)
    {
      itm->itm_payload = (void *) mom_make_file (fout);
      time (&itm->itm_mtime);
    }
  mom_item_unlock (itm);
  if (ok)
    return itm;
  fclose (fout);
  if (backupok)
    {
      if (rename (backupath, path))
        MOM_FATAPRINTF ("failed to revert backup %s to %s: %s",
                        backupath, path, strerror (errno));
    }
  NANOEVAL_FAILURE_MOM (nev, expnod,
                        mom_boxnode_make_va (MOM_PREDEFITM (type_error), 2,
                                             arg0, arg1));
}                               // end of nanoeval_payl_file_write2 

const char momsig_nanoeval_payl_file_close1[] = "signature_nanoeval1";
const void *
momf_nanoeval_payl_file_close1 (struct mom_nanoeval_st *nev,
                                struct mom_item_st *envitm,
                                int depth,
                                const struct mom_boxnode_st *expnod,
                                const struct mom_boxnode_st *closnod,
                                const void *arg0)
{
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_payl_file_close1 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0));
  struct mom_item_st *itm = mom_dyncast_item (arg0);
  if (!itm)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                               arg0));
  bool ok = false;
  const void *res = NULL;
  mom_item_lock (itm);
  unsigned pty = mom_itype (itm->itm_payload);
  ok = (pty == MOMITY_FILE || MOMITY_FILEBUFFER);
  if (ok)
    {
      if (pty == MOMITY_FILEBUFFER)
        res =
          mom_filebuffer_boxstring ((struct mom_filebuffer_st *)
                                    itm->itm_payload, MOM_FILEBUFFER_CLOSE);
      mom_file_close (itm->itm_payload);
      itm->itm_payload = NULL;
      time (&itm->itm_mtime);
    }
  mom_item_unlock (itm);
  if (ok)
    return res;
  NANOEVAL_FAILURE_MOM (nev, expnod,
                        mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                             arg0));
}                               // end of nanoeval_payl_file_close1 

////////////////////////////////////////////////////////////////


const char momsig_nanoeval_iter2[] = "signature_nanoeval2";
const void *
momf_nanoeval_iter2 (struct mom_nanoeval_st *nev,
                     struct mom_item_st *envitm,
                     int depth,
                     const struct mom_boxnode_st *expnod,
                     const struct mom_boxnode_st *closnod,
                     const void *arg0, const void *arg1)
{
  MOM_DEBUGPRINTF (run,
                   "nanoeval_iter2 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s arg1=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0), mom_value_cstring (arg1));
  if (mom_itype (arg0) != MOMITY_NODE)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                               arg0));
  const struct mom_boxnode_st *nodf = arg0;
  switch (mom_itype (arg1))
    {
    case MOMITY_NONE:
      return NULL;
    case MOMITY_BOXINT:
      {
        intptr_t hi = mom_boxint_val_def (arg1, -1);
        for (intptr_t ix = 0; ix < hi; ix++)
          {
            const struct mom_boxint_st *boxixv = mom_boxint_make (ix);
            mom_nanoapply (nev, envitm, nodf, expnod, 1,
                           (const void **) &boxixv, depth + 1);
          }
      }
      return arg1;
    case MOMITY_SET:
    case MOMITY_TUPLE:
      {
        const struct mom_seqitems_st *seq = arg1;
        unsigned sz = mom_raw_size (seq);
        for (unsigned ix = 0; ix < sz; ix++)
          {
            const struct mom_item_st *curitm = seq->seqitem[ix];
            mom_nanoapply (nev, envitm, nodf, expnod, 1,
                           (const void **) &curitm, depth + 1);
          }
      }
      return arg1;
    case MOMITY_NODE:
      {
        const struct mom_boxnode_st *nod = arg1;
        unsigned sz = mom_raw_size (nod);
        for (unsigned ix = 0; ix < sz; ix++)
          {
            const void *curson = nod->nod_sons[ix];
            mom_nanoapply (nev, envitm, nodf, expnod, 1, &curson, depth + 1);
          }
      }
      return arg1;
    default:
      NANOEVAL_FAILURE_MOM (nev, expnod,
                            mom_boxnode_make_va (MOM_PREDEFITM (type_error),
                                                 1, arg1));
    }
}                               // end momf_nanoeval_iter2



const char momsig_nanoeval_reviter2[] = "signature_nanoeval2";
const void *
momf_nanoeval_reviter2 (struct mom_nanoeval_st *nev,
                        struct mom_item_st *envitm,
                        int depth,
                        const struct mom_boxnode_st *expnod,
                        const struct mom_boxnode_st *closnod,
                        const void *arg0, const void *arg1)
{
  MOM_DEBUGPRINTF (run,
                   "nanoeval_reviter2 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s arg1=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0), mom_value_cstring (arg1));
  if (mom_itype (arg0) != MOMITY_NODE)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                               arg0));
  const struct mom_boxnode_st *nodf = arg0;
  switch (mom_itype (arg1))
    {
    case MOMITY_NONE:
      return NULL;
    case MOMITY_BOXINT:
      {
        intptr_t hi = mom_boxint_val_def (arg1, -1);
        for (intptr_t ix = hi - 1; ix >= 0; ix--)
          {
            const struct mom_boxint_st *boxixv = mom_boxint_make (ix);
            mom_nanoapply (nev, envitm, nodf, expnod, 1,
                           (const void **) &boxixv, depth + 1);
          }
      }
      return arg1;
    case MOMITY_SET:
    case MOMITY_TUPLE:
      {
        const struct mom_seqitems_st *seq = arg1;
        unsigned sz = mom_raw_size (seq);
        for (int ix = (int)sz - 1; ix >= 0; ix--)
          {
            const struct mom_item_st *curitm = seq->seqitem[ix];
            mom_nanoapply (nev, envitm, nodf, expnod, 1,
                           (const void **) &curitm, depth + 1);
          }
      }
      return arg1;
    case MOMITY_NODE:
      {
        const struct mom_boxnode_st *nod = arg1;
        unsigned sz = mom_raw_size (nod);
        for (int ix = (int)sz - 1; ix >= 0; ix--)
          {
            const void *curson = nod->nod_sons[ix];
            mom_nanoapply (nev, envitm, nodf, expnod, 1, &curson, depth + 1);
          }
      }
      return arg1;
    default:
      NANOEVAL_FAILURE_MOM (nev, expnod,
                            mom_boxnode_make_va (MOM_PREDEFITM (type_error),
                                                 1, arg1));
    }
}                               // end momf_nanoeval_reviter2


const char momsig_nanoeval_iterix2[] = "signature_nanoeval2";
const void *
momf_nanoeval_iterix2 (struct mom_nanoeval_st *nev,
                       struct mom_item_st *envitm,
                       int depth,
                       const struct mom_boxnode_st *expnod,
                       const struct mom_boxnode_st *closnod,
                       const void *arg0, const void *arg1)
{
  MOM_DEBUGPRINTF (run,
                   "nanoeval_iterix2 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s arg1=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0), mom_value_cstring (arg1));
  if (mom_itype (arg0) != MOMITY_NODE)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                               arg0));
  const struct mom_boxnode_st *nodf = arg0;
  switch (mom_itype (arg1))
    {
    case MOMITY_NONE:
      return NULL;
    case MOMITY_SET:
    case MOMITY_TUPLE:
      {
        const struct mom_seqitems_st *seq = arg1;
        unsigned sz = mom_raw_size (seq);
        for (unsigned ix = 0; ix < sz; ix++)
          {
            const struct mom_item_st *curitm = seq->seqitem[ix];
            const void *twoargs[2] = { NULL, NULL };
            twoargs[0] = curitm;
            twoargs[1] = mom_boxint_make (ix);
            mom_nanoapply (nev, envitm, nodf, expnod, 2, twoargs, depth + 1);
          }
      }
      return arg1;
    case MOMITY_NODE:
      {
        const struct mom_boxnode_st *nod = arg1;
        unsigned sz = mom_raw_size (nod);
        for (unsigned ix = 0; ix < sz; ix++)
          {
            const void *twoargs[2] = { NULL, NULL };
            twoargs[0] = nod->nod_sons[ix];
            twoargs[1] = mom_boxint_make (ix);
            mom_nanoapply (nev, envitm, nodf, expnod, 2, twoargs, depth + 1);
          }
      }
      return arg1;
    default:
      NANOEVAL_FAILURE_MOM (nev, expnod,
                            mom_boxnode_make_va (MOM_PREDEFITM (type_error),
                                                 1, arg1));
    }
}                               // end momf_nanoeval_iterix2




const char momsig_nanoeval_map3[] = "signature_nanoeval3";
const void *
momf_nanoeval_map3 (struct mom_nanoeval_st *nev,
                    struct mom_item_st *envitm,
                    int depth,
                    const struct mom_boxnode_st *expnod,
                    const struct mom_boxnode_st *closnod,
                    const void *arg0, const void *arg1, const void *arg2)
{
  MOM_DEBUGPRINTF (run,
                   "nanoeval_map3 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s arg1=%s arg2=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0), mom_value_cstring (arg1),
                   mom_value_cstring (arg2));
  if (mom_itype (arg0) != MOMITY_NODE)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                               arg0));
  if (mom_itype (arg2) != MOMITY_ITEM)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                               arg2));
  const struct mom_item_st *connitm = arg2;
  const struct mom_boxnode_st *nodf = arg0;
  const void *smallarr[16] = { };
  unsigned sz = mom_size (arg1);
  const void *resv = NULL;
  const void **arr = (sz < sizeof (smallarr) / sizeof (smallarr[0]))
    ? smallarr : mom_gc_alloc ((sz + 1) * sizeof (void *));
  switch (mom_itype (arg1))
    {
    case MOMITY_NONE:
      break;
    case MOMITY_BOXINT:
      {
        intptr_t hi = mom_boxint_val_def (arg1, -1);
        if (hi > MOM_SIZE_MAX)
          NANOEVAL_FAILURE_MOM (nev, expnod,
                                mom_boxnode_make_va (MOM_PREDEFITM
                                                     (undefined_result), 1,
                                                     arg1));
        arr = (hi < (intptr_t) (sizeof (smallarr) / sizeof (smallarr[0])))      //
          ? smallarr : mom_gc_alloc ((hi + 1) * sizeof (void *));
        for (intptr_t ix = 0; ix < hi; ix++)
          {
            const struct mom_boxint_st *boxixv = mom_boxint_make (ix);
            arr[ix] = mom_nanoapply (nev, envitm, nodf, expnod, 1,
                                     (const void **) &boxixv, depth + 1);
          }
        resv =
          mom_boxnode_make (connitm, hi,
                            (const struct mom_hashedvalue_st **) arr);
      }
      break;
    case MOMITY_SET:
    case MOMITY_TUPLE:
      {
        const struct mom_seqitems_st *seq = arg1;
        for (unsigned ix = 0; ix < sz; ix++)
          {
            const struct mom_item_st *curitm = seq->seqitem[ix];
            arr[ix] = mom_nanoapply (nev, envitm, nodf, expnod, 1,
                                     (const void **) &curitm, depth + 1);
          }
        resv =
          mom_boxnode_make (connitm, sz,
                            (const struct mom_hashedvalue_st **) arr);
      }
      break;
    case MOMITY_NODE:
      {
        const struct mom_boxnode_st *nod = arg1;
        unsigned sz = mom_raw_size (nod);
        for (unsigned ix = 0; ix < sz; ix++)
          {
            const void *curson = nod->nod_sons[ix];
            arr[ix] =
              mom_nanoapply (nev, envitm, nodf, expnod, 1, &curson,
                             depth + 1);
          }
        resv =
          mom_boxnode_make (connitm, sz,
                            (const struct mom_hashedvalue_st **) arr);
      }
      break;
    default:
      NANOEVAL_FAILURE_MOM (nev, expnod,
                            mom_boxnode_make_va (MOM_PREDEFITM (type_error),
                                                 1, arg1));
    }
  if (arr != smallarr)
    GC_FREE (arr);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_map3 depth#%d resv=%s", depth,
                   mom_value_cstring (resv));
  return resv;
}                               // end momf_nanoeval_map3


const char momsig_nanoeval_mapix3[] = "signature_nanoeval3";
const void *
momf_nanoeval_mapix3 (struct mom_nanoeval_st *nev,
                      struct mom_item_st *envitm,
                      int depth,
                      const struct mom_boxnode_st *expnod,
                      const struct mom_boxnode_st *closnod,
                      const void *arg0, const void *arg1, const void *arg2)
{
  MOM_DEBUGPRINTF (run,
                   "nanoeval_mapix3 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s arg1=%s arg2=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0), mom_value_cstring (arg1),
                   mom_value_cstring (arg2));
  if (mom_itype (arg0) != MOMITY_NODE)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                               arg0));
  if (mom_itype (arg2) != MOMITY_ITEM)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                               arg2));
  const struct mom_item_st *connitm = arg2;
  const struct mom_boxnode_st *nodf = arg0;
  unsigned sz = mom_size (arg1);
  const void *resv = NULL;
  const void *smallarr[16] = { };
  const void **arr = (sz < sizeof (smallarr) / sizeof (smallarr[0]))
    ? smallarr : mom_gc_alloc ((sz + 1) * sizeof (void *));
  switch (mom_itype (arg1))
    {
    case MOMITY_NONE:
      break;
    case MOMITY_SET:
    case MOMITY_TUPLE:
      {
        const struct mom_seqitems_st *seq = arg1;
        unsigned sz = mom_raw_size (seq);
        for (unsigned ix = 0; ix < sz; ix++)
          {
            const struct mom_item_st *curitm = seq->seqitem[ix];
            const void *twoargs[2] = { NULL, NULL };
            twoargs[0] = curitm;
            twoargs[1] = mom_boxint_make (ix);
            arr[ix] =
              mom_nanoapply (nev, envitm, nodf, expnod, 2, twoargs,
                             depth + 1);
          }
        resv =
          mom_boxnode_make (connitm, sz,
                            (const struct mom_hashedvalue_st **) arr);
      }
      break;
    case MOMITY_NODE:
      {
        const struct mom_boxnode_st *nod = arg1;
        unsigned sz = mom_raw_size (nod);
        for (unsigned ix = 0; ix < sz; ix++)
          {
            const void *twoargs[2] = { NULL, NULL };
            twoargs[0] = nod->nod_sons[ix];
            twoargs[1] = mom_boxint_make (ix);
            arr[ix] =
              mom_nanoapply (nev, envitm, nodf, expnod, 2, twoargs,
                             depth + 1);
          }
        resv =
          mom_boxnode_make (connitm, sz,
                            (const struct mom_hashedvalue_st **) arr);
      }
      break;
    default:
      NANOEVAL_FAILURE_MOM (nev, expnod,
                            mom_boxnode_make_va (MOM_PREDEFITM (type_error),
                                                 1, arg1));
    }
  if (arr != smallarr)
    GC_FREE (arr);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_mapix3 depth#%d resv=%s", depth,
                   mom_value_cstring (resv));
  return resv;
}                               // end momf_nanoeval_mapix3


// fold_left(nodefun,inival,compval)
const char momsig_nanoeval_fold_left3[] = "signature_nanoeval3";
const void *
momf_nanoeval_fold_left3 (struct mom_nanoeval_st *nev,
                          struct mom_item_st *envitm,
                          int depth,
                          const struct mom_boxnode_st *expnod,
                          const struct mom_boxnode_st *closnod,
                          const void *arg0, const void *arg1,
                          const void *arg2)
{
  MOM_DEBUGPRINTF (run,
                   "nanoeval_fold_left3 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s arg1=%s arg2=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0), mom_value_cstring (arg1),
                   mom_value_cstring (arg2));
  if (mom_itype (arg0) != MOMITY_NODE)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                               arg0));
  const struct mom_boxnode_st *nodf = arg0;
  const void *resv = arg1;
  switch (mom_itype (arg2))
    {
    case MOMITY_NONE:
      return NULL;
    case MOMITY_BOXINT:
      {
        intptr_t hi = mom_boxint_val_def (arg2, -1);
        for (intptr_t ix = 0; ix < hi; ix++)
          {
            const void *twoargs[2] = { NULL, NULL };
            twoargs[0] = resv;
            twoargs[1] = mom_boxint_make (ix);
            resv =
              mom_nanoapply (nev, envitm, nodf, expnod, 2, twoargs,
                             depth + 1);
          }
      }
      break;
    case MOMITY_SET:
    case MOMITY_TUPLE:
      {
        const struct mom_seqitems_st *seq = arg2;
        unsigned sz = mom_raw_size (seq);
        for (unsigned ix = 0; ix < sz; ix++)
          {
            const struct mom_item_st *curitm = seq->seqitem[ix];
            const void *twoargs[2] = { NULL, NULL };
            twoargs[0] = resv;
            twoargs[1] = curitm;
            resv =
              mom_nanoapply (nev, envitm, nodf, expnod, 2, twoargs,
                             depth + 1);
          }
      }
      break;
    case MOMITY_NODE:
      {
        const struct mom_boxnode_st *nod = arg2;
        unsigned sz = mom_raw_size (nod);
        for (unsigned ix = 0; ix < sz; ix++)
          {
            const void *twoargs[2] = { NULL, NULL };
            twoargs[0] = resv;
            twoargs[1] = nod->nod_sons[ix];
            resv =
              mom_nanoapply (nev, envitm, nodf, expnod, 2, twoargs,
                             depth + 1);
          }
      }
      break;
    default:
      NANOEVAL_FAILURE_MOM (nev, expnod,
                            mom_boxnode_make_va (MOM_PREDEFITM (type_error),
                                                 1, arg2));
    }
  MOM_DEBUGPRINTF (run,
                   "nanoeval_fold_left3 depth#%d end resv=%s", depth,
                   mom_value_cstring (resv));
  return resv;
}                               /* end momf_nanoeval_fold_left3 */



// fold_right(nodefun,compval,inival)
const char momsig_nanoeval_fold_right3[] = "signature_nanoeval3";
const void *
momf_nanoeval_fold_right3 (struct mom_nanoeval_st *nev,
                           struct mom_item_st *envitm,
                           int depth,
                           const struct mom_boxnode_st *expnod,
                           const struct mom_boxnode_st *closnod,
                           const void *arg0, const void *arg1,
                           const void *arg2)
{
  MOM_DEBUGPRINTF (run,
                   "nanoeval_fold_right3 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s arg1=%s arg2=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0), mom_value_cstring (arg1),
                   mom_value_cstring (arg2));
  if (mom_itype (arg0) != MOMITY_NODE)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                               arg0));
  const struct mom_boxnode_st *nodf = arg0;
  const void *resv = arg2;
  switch (mom_itype (arg1))
    {
    case MOMITY_NONE:
      return NULL;
    case MOMITY_BOXINT:
      {
        intptr_t hi = mom_boxint_val_def (arg1, -1);
        for (intptr_t ix = hi - 1; ix >= 0; ix--)
          {
            const void *twoargs[2] = { NULL, NULL };
            twoargs[0] = resv;
            twoargs[1] = mom_boxint_make (ix);
            resv =
              mom_nanoapply (nev, envitm, nodf, expnod, 2, twoargs,
                             depth + 1);
          }
      }
      break;
    case MOMITY_SET:
    case MOMITY_TUPLE:
      {
        const struct mom_seqitems_st *seq = arg1;
        unsigned sz = mom_raw_size (seq);
        for (unsigned ix = 0; ix < sz; ix++)
          {
            const struct mom_item_st *curitm = seq->seqitem[ix];
            const void *twoargs[2] = { NULL, NULL };
            twoargs[0] = curitm;
            twoargs[1] = resv;
            resv =
              mom_nanoapply (nev, envitm, nodf, expnod, 2, twoargs,
                             depth + 1);
          }
      }
      break;
    case MOMITY_NODE:
      {
        const struct mom_boxnode_st *nod = arg1;
        unsigned sz = mom_raw_size (nod);
        for (unsigned ix = 0; ix < sz; ix++)
          {
            const void *twoargs[2] = { NULL, NULL };
            twoargs[0] = nod->nod_sons[ix];
            twoargs[1] = resv;
            resv =
              mom_nanoapply (nev, envitm, nodf, expnod, 2, twoargs,
                             depth + 1);
          }
      }
      break;
    default:
      NANOEVAL_FAILURE_MOM (nev, expnod,
                            mom_boxnode_make_va (MOM_PREDEFITM (type_error),
                                                 1, arg2));
    }
  MOM_DEBUGPRINTF (run,
                   "nanoeval_fold_right3 depth#%d end resv=%s", depth,
                   mom_value_cstring (resv));
  return resv;
}                               /* end momf_nanoeval_fold_right3 */

// foldix_left(nodefun,inival,compval)
const char momsig_nanoeval_foldix_left3[] = "signature_nanoeval3";
const void *
momf_nanoeval_foldix_left3 (struct mom_nanoeval_st *nev,
                            struct mom_item_st *envitm,
                            int depth,
                            const struct mom_boxnode_st *expnod,
                            const struct mom_boxnode_st *closnod,
                            const void *arg0, const void *arg1,
                            const void *arg2)
{
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_fold_left3 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s arg1=%s arg2=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0), mom_value_cstring (arg1),
                   mom_value_cstring (arg2));
  if (mom_itype (arg0) != MOMITY_NODE)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                               arg0));
  const struct mom_boxnode_st *nodf = arg0;
  const void *resv = arg1;
  switch (mom_itype (arg2))
    {
    case MOMITY_NONE:
      return NULL;
    case MOMITY_SET:
    case MOMITY_TUPLE:
      {
        const struct mom_seqitems_st *seq = arg2;
        unsigned sz = mom_raw_size (seq);
        for (unsigned ix = 0; ix < sz; ix++)
          {
            const struct mom_item_st *curitm = seq->seqitem[ix];
            const void *threeargs[3] = { NULL, NULL, NULL };
            threeargs[0] = resv;
            threeargs[1] = curitm;
            threeargs[2] = mom_boxint_make (ix);
            resv =
              mom_nanoapply (nev, envitm, nodf, expnod, 3, threeargs,
                             depth + 1);
          }
      }
      break;
    case MOMITY_NODE:
      {
        const struct mom_boxnode_st *nod = arg2;
        unsigned sz = mom_raw_size (nod);
        for (unsigned ix = 0; ix < sz; ix++)
          {
            const void *threeargs[2] = { NULL, NULL, NULL };
            threeargs[0] = resv;
            threeargs[1] = nod->nod_sons[ix];
            threeargs[2] = mom_boxint_make (ix);
            resv =
              mom_nanoapply (nev, envitm, nodf, expnod, 3, threeargs,
                             depth + 1);
          }
      }
      break;
    default:
      NANOEVAL_FAILURE_MOM (nev, expnod,
                            mom_boxnode_make_va (MOM_PREDEFITM (type_error),
                                                 1, arg2));
    }
  MOM_DEBUGPRINTF (run,
                   "nanoeval_foldix_left3 depth#%d end resv=%s", depth,
                   mom_value_cstring (resv));
  return resv;
}                               /* end momf_nanoeval_foldix_left3 */



// foldix_right(nodefun,compval,inival)
const char momsig_nanoeval_foldix_right3[] = "signature_nanoeval3";
const void *
momf_nanoeval_foldix_right3 (struct mom_nanoeval_st *nev,
                             struct mom_item_st *envitm,
                             int depth,
                             const struct mom_boxnode_st *expnod,
                             const struct mom_boxnode_st *closnod,
                             const void *arg0, const void *arg1,
                             const void *arg2)
{
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_foldix_right3 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s arg1=%s arg2=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0), mom_value_cstring (arg1),
                   mom_value_cstring (arg2));
  if (mom_itype (arg0) != MOMITY_NODE)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                               arg0));
  const struct mom_boxnode_st *nodf = arg0;
  const void *resv = arg2;
  switch (mom_itype (arg1))
    {
    case MOMITY_NONE:
      return NULL;
    case MOMITY_SET:
    case MOMITY_TUPLE:
      {
        const struct mom_seqitems_st *seq = arg1;
        unsigned sz = mom_raw_size (seq);
        for (unsigned ix = 0; ix < sz; ix++)
          {
            const struct mom_item_st *curitm = seq->seqitem[ix];
            const void *threeargs[3] = { NULL, NULL, NULL };
            threeargs[0] = curitm;
            threeargs[1] = resv;
            threeargs[2] = mom_boxint_make (ix);
            resv =
              mom_nanoapply (nev, envitm, nodf, expnod, 3, threeargs,
                             depth + 1);
          }
      }
      break;
    case MOMITY_NODE:
      {
        const struct mom_boxnode_st *nod = arg1;
        unsigned sz = mom_raw_size (nod);
        for (unsigned ix = 0; ix < sz; ix++)
          {
            const void *threeargs[3] = { NULL, NULL, NULL };
            threeargs[0] = nod->nod_sons[ix];
            threeargs[1] = resv;
            threeargs[2] = mom_boxint_make (ix);
            resv =
              mom_nanoapply (nev, envitm, nodf, expnod, 3, threeargs,
                             depth + 1);
          }
      }
      break;
    default:
      NANOEVAL_FAILURE_MOM (nev, expnod,
                            mom_boxnode_make_va (MOM_PREDEFITM (type_error),
                                                 1, arg2));
    }
  MOM_DEBUGPRINTF (run,
                   "nanoeval_foldix_right3 depth#%d end resv=%s", depth,
                   mom_value_cstring (resv));
  return resv;
}                               /* end momf_nanoeval_foldix_right3 */



const char momsig_nanoeval_out_newline1[] = "signature_nanoeval1";
const void *
momf_nanoeval_out_newline1 (struct mom_nanoeval_st *nev,
                            struct mom_item_st *envitm,
                            int depth,
                            const struct mom_boxnode_st *expnod,
                            const struct mom_boxnode_st *closnod,
                            const void *arg0)
{
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (closnod != NULL);
  struct mom_item_st *fitm = mom_dyncast_item (arg0);
  MOM_DEBUGPRINTF (run, "nanoeval_out_newline1 depth#%d envitm %s arg0=%s",
                   depth, mom_item_cstring (envitm),
                   mom_value_cstring (arg0));
  bool ok = fitm != NULL;
  if (ok)
    {
      mom_item_lock (fitm);
      FILE *f = mom_file (fitm->itm_payload);
      ok = f != NULL;
      mom_file_newline (fitm->itm_payload);
      mom_item_unlock (fitm);
    }
  if (ok)
    return fitm;
  else
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error),
                                               1, arg0));
}                               /* end momf_nanoeval_out_newline1 */

const char momsig_nanoeval_out_indent1[] = "signature_nanoeval1";
const void *
momf_nanoeval_out_indent1 (struct mom_nanoeval_st *nev,
                           struct mom_item_st *envitm,
                           int depth,
                           const struct mom_boxnode_st *expnod,
                           const struct mom_boxnode_st *closnod,
                           const void *arg0)
{
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (closnod != NULL);
  MOM_DEBUGPRINTF (run, "nanoeval_out_indent1 depth#%d envitm %s arg0=%s",
                   depth, mom_item_cstring (envitm),
                   mom_value_cstring (arg0));
  struct mom_item_st *fitm = mom_dyncast_item (arg0);
  bool ok = fitm != NULL;
  if (ok)
    {
      mom_item_lock (fitm);
      FILE *f = mom_file (fitm->itm_payload);
      ok = f != NULL;
      mom_file_indent (fitm->itm_payload);
      mom_item_unlock (fitm);
    }
  if (ok)
    return fitm;
  else
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error),
                                               1, arg0));
}                               /* end momf_nanoeval_out_indent1 */

const char momsig_nanoeval_out_outdent1[] = "signature_nanoeval1";
const void *
momf_nanoeval_out_outdent1 (struct mom_nanoeval_st *nev,
                            struct mom_item_st *envitm,
                            int depth,
                            const struct mom_boxnode_st *expnod,
                            const struct mom_boxnode_st *closnod,
                            const void *arg0)
{
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (closnod != NULL);
  MOM_DEBUGPRINTF (run, "nanoeval_out_indent1 depth#%d envitm %s arg0=%s",
                   depth, mom_item_cstring (envitm),
                   mom_value_cstring (arg0));
  struct mom_item_st *fitm = mom_dyncast_item (arg0);
  bool ok = fitm != NULL;
  if (ok)
    {
      mom_item_lock (fitm);
      FILE *f = mom_file (fitm->itm_payload);
      ok = f != NULL;
      mom_file_outdent ((struct mom_file_st *) fitm->itm_payload);
      mom_item_unlock (fitm);
    }
  if (ok)
    return fitm;
  else
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error),
                                               1, arg0));
}                               /*end momf_nanoeval_out_outdent1 */

const char momsig_nanoeval_out_indentation1[] = "signature_nanoeval1";
const void *
momf_nanoeval_out_indentation1 (struct mom_nanoeval_st *nev,
                                struct mom_item_st *envitm,
                                int depth,
                                const struct mom_boxnode_st *expnod,
                                const struct mom_boxnode_st *closnod,
                                const void *arg0)
{
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (closnod != NULL);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_out_indentation1 depth#%d envitm %s arg0=%s",
                   depth, mom_item_cstring (envitm),
                   mom_value_cstring (arg0));
  struct mom_item_st *fitm = mom_dyncast_item (arg0);
  bool ok = fitm != NULL;
  const void *resv = NULL;
  if (ok)
    {
      mom_item_lock (fitm);
      FILE *f = mom_file (fitm->itm_payload);
      ok = f != NULL;
      if (ok)
        resv =
          mom_boxint_make (mom_file_indentation
                           ((struct mom_file_st *) (fitm->itm_payload)));
      mom_item_unlock (fitm);
    }
  MOM_DEBUGPRINTF (run, "nanoeval_out_last_line_width1 depth#%d resv=%s",
                   depth, mom_value_cstring (resv));
  if (ok)
    return resv;
  else
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error),
                                               1, arg0));
}                               /*end momf_nanoeval_out_indentation1 */


const char momsig_nanoeval_out_last_line_width1[] = "signature_nanoeval1";
const void *
momf_nanoeval_out_last_line_width1 (struct mom_nanoeval_st *nev,
                                    struct mom_item_st *envitm,
                                    int depth,
                                    const struct mom_boxnode_st *expnod,
                                    const struct mom_boxnode_st *closnod,
                                    const void *arg0)
{
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (closnod != NULL);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_out_last_line_width1 depth#%d envitm %s arg0=%s",
                   depth, mom_item_cstring (envitm),
                   mom_value_cstring (arg0));
  struct mom_item_st *fitm = mom_dyncast_item (arg0);
  bool ok = fitm != NULL;
  const void *resv = NULL;
  if (ok)
    {
      mom_item_lock (fitm);
      FILE *f = mom_file (fitm->itm_payload);
      ok = f != NULL;
      if (ok)
        resv =
          mom_boxint_make (mom_file_last_line_width
                           ((struct mom_file_st *) (fitm->itm_payload)));
      mom_item_unlock (fitm);
    }
  if (ok)
    return resv;
  else
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error),
                                               1, arg0));
}                               /*end momf_nanoeval_out_last_line_width1 */




const char momsig_nanoeval_out_puts2[] = "signature_nanoeval2";
const void *
momf_nanoeval_out_puts2 (struct mom_nanoeval_st *nev,
                         struct mom_item_st *envitm,
                         int depth,
                         const struct mom_boxnode_st *expnod,
                         const struct mom_boxnode_st *closnod,
                         const void *arg0, const void *arg1)
{
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (closnod != NULL);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_out_puts2 depth#%d envitm %s arg0=%s arg1=%s",
                   depth, mom_item_cstring (envitm), mom_value_cstring (arg0),
                   mom_value_cstring (arg1));
  struct mom_item_st *fitm = mom_dyncast_item (arg0);
  const struct mom_boxstring_st *strv = mom_dyncast_boxstring (arg1);
  bool ok = fitm != NULL && strv != NULL;
  if (ok)
    {
      mom_item_lock (fitm);
      FILE *f = mom_file (fitm->itm_payload);
      ok = f != NULL;
      mom_file_puts ((struct mom_file_st *) fitm->itm_payload, strv->cstr);
      mom_item_unlock (fitm);
    }
  if (ok)
    return fitm;
  else
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error),
                                               2, arg0, arg1));
}                               /*end momf_nanoeval_out_puts2 */



////////////////
const char momsig_nanoeval_applyany[] = "signature_nanoevalany";
const void *
momf_nanoeval_applyany (struct mom_nanoeval_st *nev,
                        struct mom_item_st *envitm,
                        int depth,
                        const struct mom_boxnode_st *expnod,
                        const struct mom_boxnode_st *closnod,
                        unsigned nbval, const void **valarr)
{
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_applyany start envitm=%s depth=%d expnod=%s closnod=%s nbval=%d",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   nbval);
  if (nbval == 0)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (arity), 1,
                                               mom_boxint_make (nbval)));
  const struct mom_boxnode_st *funod = mom_dyncast_node (valarr[0]);
  if (!funod)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 1,
                                               valarr[0]));
  MOM_DEBUGPRINTF (run,
                   "nanoeval_applyany depth#%d funod %s", depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) funod));
  return mom_nanoapply (nev, envitm, funod, expnod, nbval - 1, valarr + 1,
                        depth + 1);
}                               /* end of momf_nanoeval_applyany */


////////////////
const char momsig_nanoeval_apply_flattenany[] = "signature_nanoevalany";
const void *
momf_nanoeval_apply_flattenany (struct mom_nanoeval_st *nev,
                                struct mom_item_st *envitm,
                                int depth,
                                const struct mom_boxnode_st *expnod,
                                const struct mom_boxnode_st *closnod,
                                unsigned nbval, const void **valarr)
{
  const void *resv = NULL;
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_apply_flattenany start envitm=%s depth=%d expnod=%s closnod=%s nbval=%d",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   nbval);
  if (nbval <= 1)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (arity), 1,
                                               mom_boxint_make (nbval)));
  const struct mom_boxnode_st *funod = mom_dyncast_node (valarr[0]);
  const struct mom_item_st *flatitm = mom_dyncast_item (valarr[1]);
  if (!funod || !flatitm)
    NANOEVAL_FAILURE_MOM (nev, expnod,
                          mom_boxnode_make_va (MOM_PREDEFITM (type_error), 2,
                                               valarr[0], valarr[1]));
  unsigned nbarg = 0;
  for (unsigned ix = 2; ix < nbval; ix++)
    {
      const struct mom_boxnode_st *curnod = mom_dyncast_node (valarr[ix]);
      if (curnod && curnod->nod_connitm == flatitm)
        nbarg += mom_raw_size (curnod);
      else
        nbarg++;
    };
  const void *smallargs[16] = { 0 };
  const void **args =
    (nbarg < sizeof (smallargs) / sizeof (smallargs[0]))
    ? smallargs : mom_gc_alloc ((nbarg + 1) * sizeof (void *));
  unsigned argcnt = 0;
  for (unsigned ix = 2; ix < nbval; ix++)
    {
      const struct mom_boxnode_st *curnod = mom_dyncast_node (valarr[ix]);
      if (curnod && curnod->nod_connitm == flatitm)
        {
          unsigned cursiz = mom_raw_size (curnod);
          assert (argcnt + cursiz <= nbarg);
          for (unsigned ix = 0; ix < cursiz; ix++)
            args[argcnt++] = curnod->nod_sons[ix];
        }
      else
        {
          assert (argcnt < nbarg);
          args[argcnt++] = valarr[ix];
        }
    }
  assert (argcnt == nbarg);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_apply_flattenany depth#%d funod %s argcnt=%d",
                   depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) funod),
                   argcnt);
  resv = mom_nanoapply (nev, envitm, funod, expnod, argcnt, args, depth + 1);
  if (args != smallargs)
    GC_FREE (args);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_apply_flattenany depth#%d resv=%s", depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) resv));
  return resv;
}                               /* end of momf_nanoeval_apply_flattenany */


////////////////////////////////////////////////////////////////

////////////////
const char momsig_nanoeval_is_int1[] = "signature_nanoeval1";
const void *
momf_nanoeval_is_int1 (struct mom_nanoeval_st *nev,
                     struct mom_item_st *envitm,
                     int depth,
                     const struct mom_boxnode_st *expnod,
                     const struct mom_boxnode_st *closnod, const void *arg0)
{

  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_is_int1 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0));
  if (mom_itype(arg0) == MOMITY_BOXINT)
    return MOM_PREDEFITM(truth);
  return NULL;
} /* end momf_nanoeval_is_int1 */

const char momsig_nanoeval_is_double1[] = "signature_nanoeval1";
const void *
momf_nanoeval_is_double1 (struct mom_nanoeval_st *nev,
                     struct mom_item_st *envitm,
                     int depth,
                     const struct mom_boxnode_st *expnod,
                     const struct mom_boxnode_st *closnod, const void *arg0)
{

  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_is_double1 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0));
  if (mom_itype(arg0) == MOMITY_BOXDOUBLE)
    return MOM_PREDEFITM(truth);
  return NULL;
} /* end momf_nanoeval_is_double1 */

const char momsig_nanoeval_is_number1[] = "signature_nanoeval1";
const void *
momf_nanoeval_is_number1 (struct mom_nanoeval_st *nev,
                     struct mom_item_st *envitm,
                     int depth,
                     const struct mom_boxnode_st *expnod,
                     const struct mom_boxnode_st *closnod, const void *arg0)
{

  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_is_number1 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0));
  unsigned ty0 = mom_itype(arg0);
  if (ty0 == MOMITY_BOXINT || ty0 == MOMITY_BOXDOUBLE)
    return MOM_PREDEFITM(truth);
  return NULL;
} /* end momf_nanoeval_is_number1 */

const char momsig_nanoeval_is_string1[] = "signature_nanoeval1";
const void *
momf_nanoeval_is_string1 (struct mom_nanoeval_st *nev,
                     struct mom_item_st *envitm,
                     int depth,
                     const struct mom_boxnode_st *expnod,
                     const struct mom_boxnode_st *closnod, const void *arg0)
{

  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_is_string1 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0));
  if (mom_itype(arg0) == MOMITY_BOXSTRING)
    return MOM_PREDEFITM(truth);
  return NULL;
} /* end momf_nanoeval_is_string1 */

const char momsig_nanoeval_is_scalar1[] = "signature_nanoeval1";
const void *
momf_nanoeval_is_scalar1 (struct mom_nanoeval_st *nev,
                     struct mom_item_st *envitm,
                     int depth,
                     const struct mom_boxnode_st *expnod,
                     const struct mom_boxnode_st *closnod, const void *arg0)
{

  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_is_scalar1 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0));
  unsigned ty0 = mom_itype(arg0);
  if (ty0 == MOMITY_BOXINT || ty0 == MOMITY_BOXDOUBLE || ty0 == MOMITY_BOXSTRING)
    return MOM_PREDEFITM(truth);
  return NULL;
} /* end momf_nanoeval_is_scalar1 */

const char momsig_nanoeval_is_atom1[] = "signature_nanoeval1";
const void *
momf_nanoeval_is_atom1 (struct mom_nanoeval_st *nev,
                     struct mom_item_st *envitm,
                     int depth,
                     const struct mom_boxnode_st *expnod,
                     const struct mom_boxnode_st *closnod, const void *arg0)
{

  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_is_atom1 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0));
  unsigned ty0 = mom_itype(arg0);
  if (ty0 == MOMITY_BOXINT || ty0 == MOMITY_BOXDOUBLE || ty0 == MOMITY_BOXSTRING || ty0 == MOMITY_ITEM)
    return MOM_PREDEFITM(truth);
  return NULL;
} /* end momf_nanoeval_is_atom1 */

const char momsig_nanoeval_is_item1[] = "signature_nanoeval1";
const void *
momf_nanoeval_is_item1 (struct mom_nanoeval_st *nev,
                     struct mom_item_st *envitm,
                     int depth,
                     const struct mom_boxnode_st *expnod,
                     const struct mom_boxnode_st *closnod, const void *arg0)
{

  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_is_item1 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0));
  if (mom_itype(arg0) == MOMITY_ITEM)
    return MOM_PREDEFITM(truth);
  return NULL;
} /* end momf_nanoeval_is_item1 */


const char momsig_nanoeval_is_tuple1[] = "signature_nanoeval1";
const void *
momf_nanoeval_is_tuple1 (struct mom_nanoeval_st *nev,
                     struct mom_item_st *envitm,
                     int depth,
                     const struct mom_boxnode_st *expnod,
                     const struct mom_boxnode_st *closnod, const void *arg0)
{

  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_is_tuple1 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0));
  if (mom_itype(arg0) == MOMITY_TUPLE)
    return MOM_PREDEFITM(truth);
  return NULL;
} /* end momf_nanoeval_is_tuple1 */


const char momsig_nanoeval_is_set1[] = "signature_nanoeval1";
const void *
momf_nanoeval_is_set1 (struct mom_nanoeval_st *nev,
                     struct mom_item_st *envitm,
                     int depth,
                     const struct mom_boxnode_st *expnod,
                     const struct mom_boxnode_st *closnod, const void *arg0)
{

  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_is_set1 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0));
  if (mom_itype(arg0) == MOMITY_SET)
    return MOM_PREDEFITM(truth);
  return NULL;
} /* end momf_nanoeval_is_set1 */

const char momsig_nanoeval_is_sequence1[] = "signature_nanoeval1";
const void *
momf_nanoeval_is_sequence1 (struct mom_nanoeval_st *nev,
                     struct mom_item_st *envitm,
                     int depth,
                     const struct mom_boxnode_st *expnod,
                     const struct mom_boxnode_st *closnod, const void *arg0)
{

  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_is_sequence1 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0));
  unsigned ty0 = mom_itype(arg0);
  if (ty0 == MOMITY_SET || ty0 == MOMITY_TUPLE)
    return MOM_PREDEFITM(truth);
  return NULL;
} /* end momf_nanoeval_is_sequence1 */

const char momsig_nanoeval_is_node1[] = "signature_nanoeval1";
const void *
momf_nanoeval_is_node1 (struct mom_nanoeval_st *nev,
                     struct mom_item_st *envitm,
                     int depth,
                     const struct mom_boxnode_st *expnod,
                     const struct mom_boxnode_st *closnod, const void *arg0)
{

  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_is_node1 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0));
  if (mom_itype(arg0) == MOMITY_NODE)
    return MOM_PREDEFITM(truth);
  return NULL;
} /* end momf_nanoeval_is_node1 */


const char momsig_nanoeval_same2[] = "signature_nanoeval2";
const void *
momf_nanoeval_same2 (struct mom_nanoeval_st *nev,
                     struct mom_item_st *envitm,
                     int depth,
                     const struct mom_boxnode_st *expnod,
                     const struct mom_boxnode_st *closnod, const void *arg0, const void*arg1)
{

  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_same2 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s arg1=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0), mom_value_cstring (arg1));
  return (arg0==arg1)?MOM_PREDEFITM(truth):NULL;
} /* end of momf_nanoeval_same2 */



const char momsig_nanoeval_equal2[] = "signature_nanoeval2";
const void *
momf_nanoeval_equal2 (struct mom_nanoeval_st *nev,
                     struct mom_item_st *envitm,
                     int depth,
                     const struct mom_boxnode_st *expnod,
                     const struct mom_boxnode_st *closnod, const void *arg0, const void*arg1)
{
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_equal2 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s arg1=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0), mom_value_cstring (arg1));
  return (arg0==arg1
	  || mom_hashedvalue_equal ((const struct mom_hashedvalue_st *)arg0, (const struct mom_hashedvalue_st *)arg1))
    ?MOM_PREDEFITM(truth):NULL;
} /* end of momf_nanoeval_equal2 */


const char momsig_nanoeval_unequal2[] = "signature_nanoeval2";
const void *
momf_nanoeval_unequal2 (struct mom_nanoeval_st *nev,
                     struct mom_item_st *envitm,
                     int depth,
                     const struct mom_boxnode_st *expnod,
                     const struct mom_boxnode_st *closnod, const void *arg0, const void*arg1)
{
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_unequal2 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s arg1=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0), mom_value_cstring (arg1));
  return (arg0==arg1
	  || mom_hashedvalue_equal ((const struct mom_hashedvalue_st *)arg0, (const struct mom_hashedvalue_st *)arg1))
    ?NULL:MOM_PREDEFITM(truth);
} /* end of momf_nanoeval_unequal2 */



const char momsig_nanoeval_less2[] = "signature_nanoeval2";
const void *
momf_nanoeval_less2 (struct mom_nanoeval_st *nev,
                     struct mom_item_st *envitm,
                     int depth,
                     const struct mom_boxnode_st *expnod,
                     const struct mom_boxnode_st *closnod, const void *arg0, const void*arg1)
{
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_less2 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s arg1=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0), mom_value_cstring (arg1));
  return ( mom_hashedvalue_cmp ((const struct mom_hashedvalue_st *)arg0, (const struct mom_hashedvalue_st *)arg1)<0)
    ?MOM_PREDEFITM(truth):NULL;
} /* end of momf_nanoeval_less2 */



const char momsig_nanoeval_less_or_equal2[] = "signature_nanoeval2";
const void *
momf_nanoeval_less_or_equal2 (struct mom_nanoeval_st *nev,
                     struct mom_item_st *envitm,
                     int depth,
                     const struct mom_boxnode_st *expnod,
                     const struct mom_boxnode_st *closnod, const void *arg0, const void*arg1)
{
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_less_or_equal2 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s arg1=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0), mom_value_cstring (arg1));
  return ( mom_hashedvalue_cmp ((const struct mom_hashedvalue_st *)arg0, (const struct mom_hashedvalue_st *)arg1)<=0)
    ?MOM_PREDEFITM(truth):NULL;
} /* end of momf_nanoeval_less_or_equal2 */


const char momsig_nanoeval_greater2[] = "signature_nanoeval2";
const void *
momf_nanoeval_greater2 (struct mom_nanoeval_st *nev,
                     struct mom_item_st *envitm,
                     int depth,
                     const struct mom_boxnode_st *expnod,
                     const struct mom_boxnode_st *closnod, const void *arg0, const void*arg1)
{
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_greater2 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s arg1=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0), mom_value_cstring (arg1));
  return ( mom_hashedvalue_cmp ((const struct mom_hashedvalue_st *)arg0, (const struct mom_hashedvalue_st *)arg1)>0)
    ?MOM_PREDEFITM(truth):NULL;
} /* end of momf_nanoeval_greater2 */



const char momsig_nanoeval_greater_or_equal2[] = "signature_nanoeval2";
const void *
momf_nanoeval_greater_or_equal2 (struct mom_nanoeval_st *nev,
                     struct mom_item_st *envitm,
                     int depth,
                     const struct mom_boxnode_st *expnod,
                     const struct mom_boxnode_st *closnod, const void *arg0, const void*arg1)
{
  assert (nev && nev->nanev_magic == NANOEVAL_MAGIC_MOM);
  assert (envitm && envitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (run,
                   "nanoeval_greater_or_equal2 start envitm=%s depth=%d expnod=%s closnod=%s arg0=%s arg1=%s",
                   mom_item_cstring (envitm), depth,
                   mom_value_cstring ((struct mom_hashedvalue_st *) expnod),
                   mom_value_cstring ((struct mom_hashedvalue_st *) closnod),
                   mom_value_cstring (arg0), mom_value_cstring (arg1));
  return ( mom_hashedvalue_cmp ((const struct mom_hashedvalue_st *)arg0, (const struct mom_hashedvalue_st *)arg1)>=0)
    ?MOM_PREDEFITM(truth):NULL;
} /* end of momf_nanoeval_greater_or_equal2 */
