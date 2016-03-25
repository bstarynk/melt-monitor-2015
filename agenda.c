// file agenda.c

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




__thread int mom_worker_num;
static pthread_cond_t cond_agendachanged_mom = PTHREAD_COND_INITIALIZER;

static inline struct mom_queue_st *
agenda_queue_unsync_mom (void)
{
  struct mom_queue_st *agqu =
    (struct mom_queue_st *) (MOM_PREDEFITM (the_agenda)->itm_payload);
  if (MOM_UNLIKELY
      (!agqu || agqu == MOM_EMPTY_SLOT || agqu->va_itype != MOMITY_QUEUE))
    {
      agqu = mom_gc_alloc (sizeof (*agqu));
      agqu->va_itype = MOMITY_QUEUE;
      MOM_PREDEFITM (the_agenda)->itm_payload =
        (struct mom_anyvalue_st *) agqu;
    }
  return agqu;
}

void
mom_agenda_add_tasklet_front (const struct mom_item_st *tkletitm)
{
  if (!tkletitm || tkletitm == MOM_EMPTY_SLOT
      || tkletitm->va_itype != MOMITY_ITEM)
    return;
  mom_item_lock (MOM_PREDEFITM (the_agenda));
  struct mom_queue_st *agqu = agenda_queue_unsync_mom ();
  mom_queue_prepend (agqu, tkletitm);
  time (&MOM_PREDEFITM (the_agenda)->itm_mtime);
  mom_item_unlock (MOM_PREDEFITM (the_agenda));
  pthread_cond_broadcast (&cond_agendachanged_mom);
}                               /* end of mom_agenda_add_tasklet_front */


void
mom_agenda_add_tasklet_back (const struct mom_item_st *tkletitm)
{
  if (!tkletitm || tkletitm == MOM_EMPTY_SLOT
      || tkletitm->va_itype != MOMITY_ITEM)
    return;
  mom_item_lock (MOM_PREDEFITM (the_agenda));
  struct mom_queue_st *agqu = agenda_queue_unsync_mom ();
  mom_queue_append (agqu, tkletitm);
  time (&MOM_PREDEFITM (the_agenda)->itm_mtime);
  mom_item_unlock (MOM_PREDEFITM (the_agenda));
  pthread_cond_broadcast (&cond_agendachanged_mom);
}                               /* end of mom_agenda_add_tasklet_back */

void
mom_agenda_remove_tasklet (const struct mom_item_st *tkletitm)
{
  if (!tkletitm || tkletitm == MOM_EMPTY_SLOT
      || tkletitm->va_itype != MOMITY_ITEM)
    return;
  mom_item_lock (MOM_PREDEFITM (the_agenda));
  struct mom_queue_st *agqu = agenda_queue_unsync_mom ();
  for (struct mom_quelem_st * qe = agqu->qu_first; qe != NULL;
       qe = qe->qu_next)
    {
      for (unsigned ix = 0; ix < MOM_NB_QUELEM; ix++)
        if (qe->qu_elems[ix] == (struct mom_hashedvalue_st *) tkletitm)
          qe->qu_elems[ix] = NULL;
    }
  time (&MOM_PREDEFITM (the_agenda)->itm_mtime);
  mom_item_unlock (MOM_PREDEFITM (the_agenda));
  pthread_cond_broadcast (&cond_agendachanged_mom);
}                               /* end mom_agenda_remove_tasklet */


void
mom_agenda_remove_set_tasklets (const struct mom_boxset_st *set)
{
  if (!set || set == MOM_EMPTY_SLOT || set->va_itype != MOMITY_SET)
    return;
  mom_item_lock (MOM_PREDEFITM (the_agenda));
  struct mom_queue_st *agqu = agenda_queue_unsync_mom ();
  for (struct mom_quelem_st * qe = agqu->qu_first; qe != NULL;
       qe = qe->qu_next)
    {
      for (unsigned ix = 0; ix < MOM_NB_QUELEM; ix++)
        {
          const struct mom_item_st *tkitm =
            (const struct mom_item_st *) qe->qu_elems[ix];
          if (!tkitm || tkitm == MOM_EMPTY_SLOT
              || tkitm->va_itype != MOMITY_ITEM)
            continue;
          if (mom_set_contains (set, tkitm))
            qe->qu_elems[ix] = NULL;
        }
    }
  time (&MOM_PREDEFITM (the_agenda)->itm_mtime);
  mom_item_unlock (MOM_PREDEFITM (the_agenda));
  pthread_cond_broadcast (&cond_agendachanged_mom);
}                               /* end mom_agenda_remove_set_tasklets */



const struct mom_boxtuple_st *
mom_agenda_tuple_tasklets (void)
{
  const struct mom_boxtuple_st *tup = NULL;
  mom_item_lock (MOM_PREDEFITM (the_agenda));
  struct mom_queue_st *agqu = agenda_queue_unsync_mom ();
  unsigned cnt = 0;
  for (struct mom_quelem_st * qe = agqu->qu_first; qe != NULL;
       qe = qe->qu_next)
    {
      for (unsigned ix = 0; ix < MOM_NB_QUELEM; ix++)
        {
          const struct mom_item_st *tkitm =
            (const struct mom_item_st *) qe->qu_elems[ix];
          if (!tkitm || tkitm == MOM_EMPTY_SLOT
              || tkitm->va_itype != MOMITY_ITEM)
            continue;
          cnt++;
        }
    };
  const struct mom_item_st *smallarr[16] = { NULL };
  const struct mom_item_st **arr
    = (cnt < sizeof (smallarr) / sizeof (smallarr[0])) ? smallarr
    : mom_gc_alloc ((cnt + 1) * sizeof (void *));
  unsigned icnt = 0;
  for (struct mom_quelem_st * qe = agqu->qu_first; qe != NULL && icnt < cnt;
       qe = qe->qu_next)
    {
      for (unsigned ix = 0; ix < MOM_NB_QUELEM; ix++)
        {
          const struct mom_item_st *tkitm =
            (const struct mom_item_st *) qe->qu_elems[ix];
          if (!tkitm || tkitm == MOM_EMPTY_SLOT
              || tkitm->va_itype != MOMITY_ITEM)
            continue;
          if (icnt < cnt)
            arr[icnt++] = tkitm;
        }
    };
  tup = mom_boxtuple_make_arr (icnt, arr);
  mom_item_unlock (MOM_PREDEFITM (the_agenda));
  return tup;
}                               /* end of mom_agenda_tuple_tasklets */


MOM_PRIVATE bool
unsync_run_node_tasklet_mom (struct mom_item_st * tkitm,
                             struct mom_boxnode_st * tknod)
{
  bool run = false;
  struct mom_item_st *connitm = tknod->nod_connitm;
  assert (connitm && connitm->va_itype == MOMITY_ITEM);
  void *connfun = NULL;
  struct mom_item_st *connsigitm = NULL;
  mom_item_lock (connitm);
  connfun = connitm->itm_funptr;
  connsigitm = connitm->itm_funsig;
  mom_item_unlock (connitm);
  if (connsigitm == MOM_PREDEFITM (signature_tasklet) && connfun)
    {
      MOM_DEBUGPRINTF (run, "run_node_tasklet tkitm=%s connitm=%s connfun@%p",
                       mom_item_cstring (tkitm),
                       mom_item_cstring (connitm), connfun);
      mom_tasklet_sig_t *connrout = (mom_tasklet_sig_t *) connfun;
      (*connrout) (tkitm);
      run = true;
      MOM_DEBUGPRINTF (run, "run_node_tasklet done tkitm=%s connitm=%s",
                       mom_item_cstring (tkitm), mom_item_cstring (connitm));
    }
  else
    MOM_DEBUGPRINTF (run,
                     "run_node_tasklet tkitm=%s did not run connitm=%s connsigitm=%s confun@%p",
                     mom_item_cstring (tkitm), mom_item_cstring (connitm),
                     mom_item_cstring (connsigitm), (void *) connfun);
  return run;
}                               /* end unsync_run_node_tasklet_mom */



static void
pop_top_frame_tasklet_mom (struct mom_tasklet_st *tkstk)
{
  assert (tkstk && tkstk->va_itype == MOMITY_TASKLET);
  unsigned frtop = tkstk->tlk_frametop;
  assert (frtop > 0);
  struct mom_frameoffsets_st topfo = tkstk->tkl_froffsets[frtop - 1];
  unsigned scaoff = topfo.fo_scaoff;
  unsigned ptroff = topfo.fo_ptroff;
  unsigned scatop = tkstk->tkl_scatop;
  unsigned ptrtop = tkstk->tkl_ptrtop;
  unsigned scasiz = tkstk->tkl_scasiz;
  unsigned ptrsiz = tkstk->tkl_ptrsiz;
  unsigned frsiz = mom_raw_size (tkstk);
  assert (scaoff <= scatop && scatop <= scasiz);
  assert (ptroff <= ptrtop && ptrtop <= ptrsiz);
  if (scatop > scaoff)
    memset (tkstk->tkl_scalars + scaoff, 0,
            (scatop - scaoff) * sizeof (intptr_t));
  if (ptrtop > ptroff)
    memset (tkstk->tkl_pointers + ptroff, 0,
            (ptrtop - ptroff) * sizeof (void *));
  tkstk->tkl_scatop = scatop = scaoff;
  tkstk->tkl_ptrtop = ptrtop = ptroff;
  tkstk->tlk_frametop = frtop = frtop - 1;
  if (MOM_UNLIKELY (scasiz > 50 && (4 * scatop < scasiz)))
    {
      unsigned newscasiz = ((3 * scatop / 2 + 20) | 0x1f) + 1;
      if (newscasiz < scasiz)
        {
          intptr_t *newsca =
            mom_gc_alloc_atomic (newscasiz * sizeof (intptr_t));
          intptr_t *oldsca = tkstk->tkl_scalars;
          memcpy (newsca, oldsca, scatop * sizeof (intptr_t));
          tkstk->tkl_scasiz = newscasiz;
          tkstk->tkl_scalars = newsca;
          GC_FREE (oldsca);
        }
    }
  if (MOM_UNLIKELY (ptrsiz > 50 && (4 * ptrtop < ptrsiz)))
    {
      unsigned newptrsiz = ((3 * ptrtop / 2 + 20) | 0x1f) + 1;
      if (newptrsiz < ptrsiz)
        {
          void **newptr = mom_gc_alloc (newptrsiz * sizeof (void *));
          void **oldptr = tkstk->tkl_pointers;
          memcpy (newptr, oldptr, ptrtop * sizeof (void *));
          tkstk->tkl_ptrsiz = newptrsiz;
          tkstk->tkl_pointers = newptr;
          GC_FREE (oldptr);
        }
    }
  if (MOM_UNLIKELY (frsiz > 50 && (4 * frtop < frsiz)))
    {
      unsigned newfrsiz = ((3 * frtop / 2 + 20) | 0x1f) + 1;
      if (newfrsiz < frsiz)
        {
          struct mom_frameoffsets_st *newfro =
            mom_gc_alloc_atomic (newfrsiz * sizeof (intptr_t));
          struct mom_frameoffsets_st *oldfro = tkstk->tkl_froffsets;
          memcpy (newfro, oldfro,
                  frtop * sizeof (struct mom_frameoffsets_st));
          tkstk->tkl_froffsets = newfro;
          mom_put_size (tkstk, newfrsiz);
          GC_FREE (oldfro);
        }
    }
}                               /* end of pop_top_frame_tasklet_mom */

// return true if pushed a frame
static bool
push_frame_tasklet_mom (struct mom_item_st *tskitm,
                        struct mom_tasklet_st *tkstk,
                        const struct mom_boxnode_st *nod)
{
  assert (tskitm != NULL && tskitm->va_itype == MOMITY_ITEM
          && tskitm->itm_payload == (void *) tkstk);
  assert (tkstk != NULL && tkstk->va_itype == MOMITY_TASKLET);
  if (!nod || nod == MOM_EMPTY_SLOT || nod->va_itype != MOMITY_NODE)
    {
      MOM_WARNPRINTF ("for tasklet item %s invalid node %s to push",
                      mom_item_cstring (tskitm),
                      mom_value_cstring ((void *) nod));
      return false;
    };
  struct mom_item_st *connitm = nod->nod_connitm;
  struct mom_item_st *sigitm = NULL;
  const struct mom_taskstepper_st *tstep = NULL;
  void *funptr = NULL;
  assert (connitm != NULL && connitm->va_itype == MOMITY_ITEM);
  mom_item_lock (connitm);
  funptr = connitm->itm_funptr;
  if (funptr)
    {
      sigitm = connitm->itm_funsig;
      if (sigitm != MOM_PREDEFITM (signature_nanotaskstep))
        funptr = NULL;
      else
        tstep = mom_taskstepper_dyncast (connitm->itm_payload);
    }
  mom_item_unlock (connitm);
  if (tstep && funptr)
    {
      assert (tstep->va_itype == MOMITY_TASKSTEPPER);
      unsigned nbval = mom_raw_size (tstep);
      unsigned short nbint = tstep->tksp_nbint;
      unsigned short nbdbl = tstep->tksp_nbdbl;
      mom_tasklet_reserve (tkstk, 1,
                           ((nbint + 1) | 3) + 1 +
                           nbdbl * sizeof (double) / sizeof (intptr_t),
                           nbval);
      return true;
    }
  else
    {
      MOM_WARNPRINTF
        ("for tasklet item %s bad node %s (of signature %s) to push",
         mom_item_cstring (tskitm), mom_value_cstring ((void *) nod),
         mom_item_cstring (sigitm));
      return false;
    }
}                               /* end of push_frame_tasklet_mom */

#define MOM_TASKLET_DELAY 0.005
#define MOM_TASKLET_STEPMAX 16384
MOM_PRIVATE bool
unsync_run_stack_tasklet_mom (struct mom_item_st * tkitm,
                              struct mom_tasklet_st * volatile tkstk)
{
  struct mom_nanotaskstep_st znats;
  memset (&znats, 0, sizeof (znats));
  assert (tkstk != NULL && tkstk->va_itype == MOMITY_TASKLET);
  znats.nats_magic = MOM_NANOTASKSTEP_MAGIC;
  znats.nats_startim = mom_clock_time (CLOCK_MONOTONIC);
  znats.nats_tasklet = tkstk;
  znats.nats_nanev.nanev_magic = NANOEVAL_MAGIC_MOM;
  znats.nats_nanev.nanev_maxstep = MOM_TASKLET_STEPMAX;
  assert (tkitm != NULL && tkitm->va_itype == MOMITY_ITEM);
  for (;;)
    {
      znats.nats_nanev.nanev_tkitm = tkitm;
      znats.nats_tasklet = tkstk;
      assert (tkstk != NULL && tkstk->va_itype == MOMITY_TASKLET);
      double tcur = mom_clock_time (CLOCK_MONOTONIC);
      if (tcur - znats.nats_startim > MOM_TASKLET_DELAY)
        break;
      memset (&znats.nats_nanev.nanev_jb, 0, sizeof (jmp_buf));
      znats.nats_nanev.nanev_count++;
      if (znats.nats_nanev.nanev_count >= znats.nats_nanev.nanev_maxstep)
        break;
      unsigned frsiz = mom_raw_size (tkstk);
      unsigned frtop = tkstk->tlk_frametop;
      if (frtop == 0)
        return false;
      if (MOM_UNLIKELY (frtop >= frsiz))
        MOM_FATAPRINTF ("corrupted tasklet item %s (top=%u >= siz=%u)",
                        mom_item_cstring (tkitm), frtop, frsiz);
      struct mom_frameoffsets_st topfo = tkstk->tkl_froffsets[frtop - 1];
      unsigned scaoff = topfo.fo_scaoff;
      unsigned ptroff = topfo.fo_ptroff;
      unsigned scasiz = tkstk->tkl_scasiz;
      unsigned scatop = tkstk->tkl_scatop;
      unsigned ptrsiz = tkstk->tkl_ptrsiz;
      unsigned ptrtop = tkstk->tkl_ptrtop;
      if (MOM_UNLIKELY (scatop >= scasiz))
        MOM_FATAPRINTF ("corrupted tasklet item %s (scatop=%u >= scasiz=%u)",
                        mom_item_cstring (tkitm), scatop, scasiz);
      if (MOM_UNLIKELY (ptrtop >= ptrsiz))
        MOM_FATAPRINTF ("corrupted tasklet item %s (ptrtop=%u >= ptrsiz=%u)",
                        mom_item_cstring (tkitm), ptrtop, ptrsiz);
      if (MOM_UNLIKELY (ptroff >= ptrtop))
        MOM_FATAPRINTF ("corrupted tasklet item %s (ptroff=%u >= ptrtop=%u)",
                        mom_item_cstring (tkitm), ptroff, ptrtop);
      if (MOM_UNLIKELY (scaoff >= scatop))
        MOM_FATAPRINTF ("corrupted tasklet item %s (scaoff=%u >= scatop=%u)",
                        mom_item_cstring (tkitm), scaoff, scatop);
      struct mom_framescalar_st *volatile frsca =       //
        (struct mom_framescalar_st *) tkstk->tkl_scalars + scaoff;
      struct mom_framepointer_st *volatile frptr =      //
        (struct mom_framepointer_st *) tkstk->tkl_pointers + ptroff;
      const struct mom_boxnode_st *volatile frnod = frptr->tfp_node;
      if (MOM_UNLIKELY
          (!frnod || frnod == MOM_EMPTY_SLOT
           || frnod->va_itype != MOMITY_NODE))
        MOM_FATAPRINTF
          ("corrupted tasklet item %s (bad frnod %s at frame level#%d)",
           mom_item_cstring (tkitm), mom_value_cstring ((const void *) frnod),
           frtop);
      struct mom_item_st *noditm = frnod->nod_connitm;
      assert (noditm && noditm->va_itype == MOMITY_ITEM);
      struct mom_item_st *nodsigitm = NULL;
      const struct mom_taskstepper_st *nodstepper = NULL;
      void *nodfunptr = NULL;
      {
        mom_item_lock (noditm);
        nodsigitm = noditm->itm_funsig;
        nodstepper = (const void *) noditm->itm_payload;
        nodfunptr = noditm->itm_funptr;
        mom_item_unlock (noditm);
      }
      if (MOM_UNLIKELY (nodsigitm != MOM_PREDEFITM (signature_nanotaskstep)
                        || nodstepper == NULL
                        || nodstepper->va_itype != MOMITY_TASKSTEPPER
                        || !nodfunptr))
        {
          MOM_WARNPRINTF ("bad tasklet item %s (bad frnod %s"
                          " at frame level#%d, noditm=%s, nodsigitm=%s)",
                          mom_item_cstring (tkitm),
                          mom_value_cstring ((const void *) frnod), frtop,
                          mom_item_cstring (noditm),
                          mom_item_cstring (nodsigitm));
          pop_top_frame_tasklet_mom (tkstk);
          return false;
        };
      mom_nanotaskstep_sig_t *stepfun = nodfunptr;
      int errlin = setjmp (znats.nats_nanev.nanev_jb);
      if (errlin)
        {
          MOM_WARNPRINTF_AT (znats.nats_nanev.nanev_errfile ? : "??",
                             errlin,
                             "tasklet item %s frame level#%d (%s) got exception failure %s expr %s",
                             mom_item_cstring (tkitm), frtop,
                             mom_value_cstring ((const void *) frnod),
                             mom_value_cstring (znats.nats_nanev.nanev_fail),
                             mom_value_cstring (znats.nats_nanev.nanev_expr));
          tkstk = znats.nats_tasklet;
          const struct mom_boxnode_st *excnod = NULL;
          struct mom_item_st *excitm = NULL;
          struct mom_item_st *excsigitm = NULL;
          void *excfun = NULL;
          if (tkstk && tkstk != MOM_EMPTY_SLOT
              && tkstk->va_itype == MOMITY_TASKLET
              && (excnod = tkstk->tkl_excnod) != NULL
              && excnod != MOM_EMPTY_SLOT && excnod->va_itype == MOMITY_NODE)
            {
              excitm = excnod->nod_connitm;
              assert (excitm && excitm->va_itype == MOMITY_ITEM);
              {
                mom_item_lock (excitm);
                excsigitm = excitm->itm_funsig;
                if (excsigitm == MOM_PREDEFITM (signature_nanotaskexception))
                  excfun = excitm->itm_funptr;
                mom_item_unlock (excitm);
              }
              if (excfun != NULL)
                {
                  mom_nanotaskexception_sig_t *funexcptr = excfun;
                  (*funexcptr) (excnod, tkitm, znats.nats_nanev.nanev_fail,
                                znats.nats_nanev.nanev_expr,
                                znats.nats_nanev.nanev_errfile, errlin);
                  continue;
                }
              else
                {
                  MOM_WARNPRINTF_AT (znats.nats_nanev.nanev_errfile ? : "??",
                                     errlin,
                                     "tasklet item %s frame level#%d (%s) got unhandled exception failure %s expr %s",
                                     mom_item_cstring (tkitm), frtop,
                                     mom_value_cstring ((const void *) frnod),
                                     mom_value_cstring (znats.
                                                        nats_nanev.nanev_fail),
                                     mom_value_cstring (znats.
                                                        nats_nanev.nanev_expr));
                  return false;
                }
            }
        }
      else                      // no errlin
        {
          znats.nats_nanev.nanev_fail = NULL;
          znats.nats_nanev.nanev_expr = NULL;
          znats.nats_nanev.nanev_errfile = NULL;
          znats.nats_nblevels = 0;
          memset (znats.nats_ptrs, 0, sizeof (znats.nats_ptrs));
          memset (znats.nats_nums, 0, sizeof (znats.nats_nums));
          memset (znats.nats_dbls, 0, sizeof (znats.nats_dbls));
          MOM_DEBUGPRINTF (run,
                           "before running stepfun@%p node %s level#%d taskitm %s",
                           stepfun, mom_value_cstring ((const void *) frnod),
                           frtop, mom_item_cstring (tkitm));
          struct mom_result_tasklet_st res =
            (*stepfun) (&znats, (const struct mom_boxnode_st *) frnod,
                        (struct mom_framescalar_st *) frsca,
                        (struct mom_framepointer_st *) frptr);
          switch (mom_what_taskstep (res.r_what))
            {
            case MOMTKS_NOP:
              continue;
            case MOMTKS_POP_ONE:
              {
                pop_top_frame_tasklet_mom (tkstk);
              }
              break;
            case MOMTKS_PUSH_ONE:
              {
                push_frame_tasklet_mom (tkitm, tkstk, res.r_val);
              }
              break;
            case MOMTKS_REPLACE:
              {
                pop_top_frame_tasklet_mom (tkstk);
                push_frame_tasklet_mom (tkitm, tkstk, res.r_val);
              }
              break;
            case MOMTKS_POP_MANY:
              {
                unsigned nblev = znats.nats_nblevels;
                for (unsigned ix = 0; ix < nblev; ix++)
                  pop_top_frame_tasklet_mom (tkstk);

              }
              break;
            }
          /// now, transmit the data into the frame
          unsigned wnbdbl = mom_what_nbdbl (res.r_what);
          unsigned wnbint = mom_what_nbint (res.r_what);
          unsigned wnbval = mom_what_nbval (res.r_what);
          unsigned frsiz = mom_raw_size (tkstk);
          unsigned frtop = tkstk->tlk_frametop;
          if (frtop == 0)
            return false;
          if (MOM_UNLIKELY (frtop >= frsiz))
            MOM_FATAPRINTF
              ("corrupted tasklet item %s (top=%u >= siz=%u)",
               mom_item_cstring (tkitm), frtop, frsiz);
          {
            struct mom_frameoffsets_st poptopfo =
              tkstk->tkl_froffsets[frtop - 1];
            scaoff = poptopfo.fo_scaoff;
            ptroff = poptopfo.fo_ptroff;
          }
          scasiz = tkstk->tkl_scasiz;
          scatop = tkstk->tkl_scatop;
          ptrsiz = tkstk->tkl_ptrsiz;
          ptrtop = tkstk->tkl_ptrtop;
          if (MOM_UNLIKELY (scatop >= scasiz))
            MOM_FATAPRINTF
              ("corrupted tasklet item %s (scatop=%u >= scasiz=%u)",
               mom_item_cstring (tkitm), scatop, scasiz);
          if (MOM_UNLIKELY (ptrtop >= ptrsiz))
            MOM_FATAPRINTF
              ("corrupted tasklet item %s (ptrtop=%u >= ptrsiz=%u)",
               mom_item_cstring (tkitm), ptrtop, ptrsiz);
          if (MOM_UNLIKELY (ptroff >= ptrtop))
            MOM_FATAPRINTF
              ("corrupted tasklet item %s (ptroff=%u >= ptrtop=%u)",
               mom_item_cstring (tkitm), ptroff, ptrtop);
          if (MOM_UNLIKELY (scaoff >= scatop))
            MOM_FATAPRINTF
              ("corrupted tasklet item %s (scaoff=%u >= scatop=%u)",
               mom_item_cstring (tkitm), scaoff, scatop);
          frsca =               //
            (struct mom_framescalar_st *) tkstk->tkl_scalars + scaoff;
          frptr =               //
            (struct mom_framepointer_st *) tkstk->tkl_pointers + ptroff;
          frnod = frptr->tfp_node;
          if (MOM_UNLIKELY
              (!frnod || frnod == MOM_EMPTY_SLOT
               || frnod->va_itype != MOMITY_NODE))
            MOM_FATAPRINTF
              ("corrupted tasklet item %s (bad frnod %s at frame level#%d)",
               mom_item_cstring (tkitm),
               mom_value_cstring ((const void *) frnod), frtop);
          noditm = frnod->nod_connitm;
          assert (noditm && noditm->va_itype == MOMITY_ITEM);
          {
            mom_item_lock (noditm);
            nodsigitm = noditm->itm_funsig;
            nodstepper = (const void *) noditm->itm_payload;
            nodfunptr = noditm->itm_funptr;
            mom_item_unlock (noditm);
          }
          if (MOM_UNLIKELY
              (nodsigitm != MOM_PREDEFITM (signature_nanotaskstep)
               || nodstepper == NULL
               || nodstepper->va_itype != MOMITY_TASKSTEPPER || !nodfunptr))
            {
              MOM_WARNPRINTF ("bad tasklet item %s (bad frnod %s"
                              " at frame level#%d, noditm=%s, nodsigitm=%s)",
                              mom_item_cstring (tkitm),
                              mom_value_cstring ((const void *) frnod), frtop,
                              mom_item_cstring (noditm),
                              mom_item_cstring (nodsigitm));
              pop_top_frame_tasklet_mom (tkstk);
              return false;
            };
          if (wnbval > 0)
            {
              if (wnbval < mom_raw_size (nodstepper)
                  && wnbval < MOM_TASKSTEP_MAX_DATA
                  && ptroff + wnbval < ptrtop)
                {
                  for (unsigned ix = 0; ix < wnbval; ix++)
                    frptr->tfp_pointers[ix] = znats.nats_ptrs[ix];
                }
              else
                MOM_WARNPRINTF
                  ("bad number of values %d to transmit in tasklet item %s "
                   "(bad frnod %s at frame level#%d, noditm=%s)",
                   wnbval,
                   mom_item_cstring (tkitm),
                   mom_value_cstring ((const void *) frnod), frtop,
                   mom_item_cstring (noditm));
            };
          if (wnbint > 0)
            {
              if (wnbint < nodstepper->tksp_nbint
                  && wnbint < MOM_TASKSTEP_MAX_DATA
                  && scaoff + wnbint < scatop)
                {
                  for (unsigned ix = 0; ix < wnbint; ix++)
                    frsca->tfs_scalars[ix] = znats.nats_nums[ix];
                }
              else
                MOM_WARNPRINTF
                  ("bad number of integers %d to transmit in tasklet item %s "
                   "(bad frnod %s at frame level#%d, noditm=%s)",
                   wnbint,
                   mom_item_cstring (tkitm),
                   mom_value_cstring ((const void *) frnod), frtop,
                   mom_item_cstring (noditm));
            };
          if (MOM_UNLIKELY (wnbdbl > 0))
            {
              unsigned fnbint = nodstepper->tksp_nbint;
              assert (fnbint % 2 != 0);
              if (wnbdbl < nodstepper->tksp_nbdbl
                  && wnbdbl < MOM_TASKSTEP_MAX_DATA
                  && scaoff + fnbint +
                  (wnbdbl * sizeof (double)) / sizeof (intptr_t) < scatop)
                {
                  double *dptr = (double *) (frsca->tfs_scalars + fnbint);
                  for (unsigned ix = 0; ix < wnbdbl; ix++)
                    dptr[ix] = znats.nats_dbls[ix];
                }
              else
                MOM_WARNPRINTF
                  ("bad number of doubles %d to transmit in tasklet item %s "
                   "(bad frnod %s at frame level#%d, noditm=%s)",
                   wnbint,
                   mom_item_cstring (tkitm),
                   mom_value_cstring ((const void *) frnod), frtop,
                   mom_item_cstring (noditm));
            };
          continue;
        };
    };
  return true;
}                               /* end of unsync_run_stack_tasklet_mom */



MOM_PRIVATE void
run_tasklet_mom (struct mom_item_st *tkitm)
{
  if (!tkitm || tkitm == MOM_EMPTY_SLOT || tkitm->va_itype != MOMITY_ITEM)
    return;
  bool run = false;
  mom_item_lock (tkitm);
  struct mom_anyvalue_st *tkpayl = tkitm->itm_payload;
  if (tkpayl && tkpayl != MOM_EMPTY_SLOT)
    {
      if (tkpayl->va_itype == MOMITY_NODE)
        run =
          unsync_run_node_tasklet_mom (tkitm,
                                       (struct mom_boxnode_st *) tkpayl);
      else if (tkpayl->va_itype == MOMITY_TASKLET)
        run =
          unsync_run_stack_tasklet_mom (tkitm,
                                        (struct mom_tasklet_st *) tkpayl);
    }
  mom_item_unlock (tkitm);
  if (!run)
    MOM_WARNPRINTF ("run_tasklet did not run tkitm %s successfully",
                    mom_item_cstring (tkitm));
}                               /* end of run_tasklet_mom */

MOM_PRIVATE void *
agenda_thread_worker_mom (struct GC_stack_base *sb, void *data)
{
  intptr_t ix = (intptr_t) data;
  assert (ix > 0 && ix <= MOM_JOB_MAX);
  assert (sb);
  char thnbuf[16];
  memset (thnbuf, 0, sizeof (thnbuf));
  snprintf (thnbuf, sizeof (thnbuf), "momagw#%d", (int) ix);
  pthread_setname_np (pthread_self (), thnbuf);
  mom_worker_num = (int) ix;
  while (atomic_load (&mom_should_run))
    {
      struct mom_item_st *tkitm = NULL;
      mom_item_lock (MOM_PREDEFITM (the_agenda));
      struct mom_queue_st *agqu = agenda_queue_unsync_mom ();
      if (mom_queue_nonempty (agqu))
        {
          tkitm = (struct mom_item_st *) mom_queue_front (agqu);
          mom_queue_pop_front (agqu);
        }
      else
        {
          struct timespec ts = {
            0, 0
          };
          clock_gettime (CLOCK_REALTIME, &ts);
          ts.tv_nsec += 150 * 1000 * 1000;      // 150 milliseconds
          while (ts.tv_nsec > 1000 * 1000 * 1000)
            {
              ts.tv_sec++;
              ts.tv_nsec -= 1000 * 1000 * 1000;
            };
          pthread_cond_timedwait (&cond_agendachanged_mom,
                                  &MOM_PREDEFITM (the_agenda)->itm_mtx, &ts);
        }
      mom_item_unlock (MOM_PREDEFITM (the_agenda));
      if (tkitm && tkitm != MOM_EMPTY_SLOT && tkitm->va_itype == MOMITY_ITEM)
        run_tasklet_mom (tkitm);
    };
  return NULL;
}                               /* end agenda_thread_worker_mom */

MOM_PRIVATE void *
agenda_thread_wrapper_mom (void *data)
{
  GC_call_with_stack_base (agenda_thread_worker_mom, data);
  return NULL;
}

static pthread_t workthread_mom[MOM_JOB_MAX + 1];
void
mom_start_agenda (void)
{
  unsigned nbjobs = mom_nbjobs;
  if (nbjobs < 2 || nbjobs > MOM_JOB_MAX)
    MOM_FATAPRINTF ("start_agenda bad nbjobs %d", nbjobs);
  pthread_attr_t at = {
  };
  pthread_attr_init (&at);
  pthread_attr_setdetachstate (&at, PTHREAD_CREATE_DETACHED);
  for (int ix = 1; ix <= (int) nbjobs; ix++)
    pthread_create (&workthread_mom[ix],
                    &at, agenda_thread_wrapper_mom, (void *) (intptr_t) ix);
  MOM_INFORMPRINTF ("started agenda with %d workers", nbjobs);
}


////////////////////////////////////////////////////////////////
//// TASKLET SUPPORT

void
mom_tasklet_reserve (struct mom_tasklet_st *tkl,
                     unsigned nbframes,
                     unsigned nbscalars, unsigned nbpointers)
{
  if (MOM_UNLIKELY
      (!tkl || tkl == MOM_EMPTY_SLOT || tkl->va_itype != MOMITY_TASKLET))
    return;
  if (nbframes > 0)
    {
      if (MOM_UNLIKELY (nbframes > MOM_SIZE_MAX / 2))
        MOM_FATAPRINTF ("too many %u additional frames", nbframes);
      unsigned frsiz = mom_raw_size (tkl);
      unsigned frtop = tkl->tlk_frametop;
      assert (frtop <= frsiz);
      if (MOM_UNLIKELY (frtop + nbframes >= frsiz))
        {
          unsigned newfrsiz =
            mom_prime_above (((9 * frtop / 8 + nbframes + 30) | 0xf) + 1);
          if (MOM_UNLIKELY (newfrsiz == 0 || newfrsiz >= MOM_SIZE_MAX))
            MOM_FATAPRINTF
              ("too big frame size %u (for %u additional frames)", newfrsiz,
               nbframes);
          struct mom_frameoffsets_st *oldfroptr = tkl->tkl_froffsets;
          assert (oldfroptr != NULL);
          struct mom_frameoffsets_st *newfroptr =       //
            mom_gc_alloc_atomic (sizeof
                                 (struct mom_frameoffsets_st) * newfrsiz);
          memcpy (newfroptr, oldfroptr,
                  frtop * sizeof (struct mom_frameoffsets_st));
          tkl->tkl_froffsets = newfroptr;
          mom_put_size (tkl, newfrsiz);
        }
    }
  if (nbscalars > 0)
    {
      if (MOM_UNLIKELY (nbscalars > MOM_SIZE_MAX / 2))
        MOM_FATAPRINTF ("too many %u additional scalars", nbscalars);
      unsigned scatop = tkl->tkl_scatop;
      unsigned scasiz = tkl->tkl_scasiz;
      assert (scatop <= scasiz);
      if (MOM_UNLIKELY (scatop + nbscalars >= scasiz))
        {
          unsigned newscasiz =
            mom_prime_above (((9 * scatop / 8 + nbscalars + 15) | 7) + 1);
          if (MOM_UNLIKELY (newscasiz == 0 || newscasiz >= MOM_SIZE_MAX))
            MOM_FATAPRINTF
              ("too big scalar size %u (for %u additional scalars)",
               newscasiz, nbscalars);
          intptr_t *oldscaptr = tkl->tkl_scalars;
          assert (oldscaptr != NULL);
          intptr_t *newscaptr =
            mom_gc_alloc_atomic (sizeof (intptr_t) * newscasiz);
          memcpy (newscaptr, oldscaptr, scatop * sizeof (intptr_t));
          tkl->tkl_scalars = newscaptr;
          tkl->tkl_scasiz = newscasiz;
          GC_FREE (oldscaptr);
        }
    }
  if (nbpointers > 0)
    {
      if (MOM_UNLIKELY (nbpointers > MOM_SIZE_MAX / 2))
        MOM_FATAPRINTF ("too many %u additional pointers", nbpointers);
      unsigned ptrtop = tkl->tkl_ptrtop;
      unsigned ptrsiz = tkl->tkl_ptrsiz;
      assert (ptrtop <= ptrsiz);
      if (MOM_UNLIKELY (ptrtop + nbpointers >= ptrsiz))
        {
          unsigned newptrsiz =
            mom_prime_above (((9 * ptrtop / 8 + nbpointers + 15) | 7) + 1);
          if (MOM_UNLIKELY (newptrsiz == 0 || newptrsiz >= MOM_SIZE_MAX))
            MOM_FATAPRINTF
              ("too big pointer size %u (for %u additional pointers)",
               newptrsiz, nbpointers);
          void **oldptrptr = tkl->tkl_pointers;
          assert (oldptrptr != NULL);
          void **newptrptr = mom_gc_alloc (sizeof (void *) * newptrsiz);
          memcpy (newptrptr, oldptrptr, ptrtop * sizeof (void *));
          tkl->tkl_pointers = newptrptr;
          tkl->tkl_ptrsiz = newptrsiz;
          GC_FREE (oldptrptr);
        }
    }
}                               /* end mom_tasklet_reserve */

// eof agenda.c
