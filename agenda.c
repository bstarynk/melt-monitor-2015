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

static __thread struct mom_item_st *current_taskitem_mom;
static __thread struct mom_tasklet_st *current_tasklet_mom;


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
  unsigned frtop = tkstk->tkl_frametop;
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
  tkstk->tkl_frametop = frtop = frtop - 1;
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
        tstep = mom_dyncast_taskstepper (connitm->itm_payload);
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
      current_tasklet_mom = NULL;
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
      unsigned frtop = tkstk->tkl_frametop;
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
          current_tasklet_mom = NULL;
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
          current_tasklet_mom = tkstk;
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
          unsigned frtop = tkstk->tkl_frametop;
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
      current_taskitem_mom = tkitm;
      if (tkpayl->va_itype == MOMITY_NODE)
        run =
          unsync_run_node_tasklet_mom (tkitm,
                                       (struct mom_boxnode_st *) tkpayl);
      else if (tkpayl->va_itype == MOMITY_TASKLET)
        run =
          unsync_run_stack_tasklet_mom (tkitm,
                                        (struct mom_tasklet_st *) tkpayl);
      current_taskitem_mom = NULL;
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
  long loopcnt = 0;
  memset (thnbuf, 0, sizeof (thnbuf));
  snprintf (thnbuf, sizeof (thnbuf), "momagw#%d", (int) ix);
  pthread_setname_np (pthread_self (), thnbuf);
  mom_worker_num = (int) ix;
  while (atomic_load (&mom_should_run))
    {
      loopcnt++;
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
          double waitdelay = (MOM_IS_DEBUGGING (run)
                              || MOM_IS_DEBUGGING (mutex)) ? 3.8 : 1.9;
          waitdelay += (ix + 1) * 0.03;
          if ((loopcnt + ix) % 4 == 0)
            waitdelay += ((mom_random_uint32 () & 0xffff) + 10) * 1.0e-5;
          double nowtim = mom_clock_time (CLOCK_REALTIME);
          struct timespec ts = mom_timespec (nowtim + waitdelay);
          MOM_DEBUGPRINTF (run,
                           "agenda_thread_worker waiting %.3f sec for change",
                           waitdelay);
          pthread_cond_timedwait (&cond_agendachanged_mom,
                                  &MOM_PREDEFITM (the_agenda)->itm_mtx, &ts);
          MOM_DEBUGPRINTF (run, "agenda_thread_worker done waiting");
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
      unsigned frtop = tkl->tkl_frametop;
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




void
mom_dumpscan_tasklet (struct mom_dumper_st *du, struct mom_tasklet_st *tklet)
{
  assert (du && du->du_state == MOMDUMP_SCAN);
  assert (tklet && tklet->va_itype == MOMITY_TASKLET);
  mom_dumpscan_value (du, (const void *) tklet->tkl_excnod);
  mom_dumpscan_item (du, tklet->tkl_statitm);
  for (unsigned lev = 0; lev < tklet->tkl_frametop; lev++)
    {
      struct mom_frame_st fr = mom_tasklet_nth_frame (tklet, lev);
      if (!fr.fr_sca || !fr.fr_ptr)
        break;
      const struct mom_boxnode_st *frnod = fr.fr_ptr->tfp_node;
      mom_dumpscan_value (du, (const void *) frnod);
      if (!frnod || frnod == MOM_EMPTY_SLOT || frnod->va_itype != MOMITY_NODE)
        break;
      struct mom_item_st *noditm = frnod->nod_connitm;
      if (!mom_dumped_item (du, noditm))
        break;
      void *funptr = NULL;
      struct mom_taskstepper_st *tstep = NULL;
      struct mom_item_st *sigitm = NULL;
      {
        mom_item_lock (noditm);
        sigitm = noditm->itm_funsig;
        if (sigitm == MOM_PREDEFITM (signature_nanotaskstep))
          funptr = noditm->itm_funptr;
        if (mom_itype (noditm->itm_payload) == MOMITY_TASKSTEPPER)
          tstep = (struct mom_taskstepper_st *) noditm->itm_payload;
        mom_item_unlock (noditm);
      }
      if (!funptr || !tstep)
        break;
      unsigned nbval = mom_raw_size (tstep);
      for (unsigned ix = 0; ix < nbval; ix++)
        mom_dumpscan_value (du, fr.fr_ptr->tfp_pointers[ix]);
    }
}                               /* end of mom_dumpscan_tasklet */


void
mom_dumpemit_tasklet_frame (struct mom_dumper_st *du,
                            struct mom_tasklet_st *tklet,
                            struct mom_item_st *noditm,
                            struct mom_taskstepper_st *tstep,
                            struct mom_frame_st fr)
{
  assert (du && du->du_state == MOMDUMP_EMIT);
  assert (tklet && tklet->va_itype == MOMITY_TASKLET);
  assert (noditm && noditm->va_itype == MOMITY_ITEM);
  assert (tstep && tstep->va_itype == MOMITY_TASKSTEPPER);
  assert (fr.fr_sca && fr.fr_ptr);
  FILE *femit = du->du_emitfile;
  assert (femit != NULL);
  MOM_DEBUGPRINTF (dump, "mom_dumpemit_tasklet_frame noditm %s frnod %s",
                   mom_item_cstring (noditm),
                   mom_value_cstring ((const void *) fr.fr_ptr->tfp_node));
  assert (fr.fr_ptr->tfp_node && fr.fr_ptr->tfp_node->nod_connitm == noditm);
  unsigned nbval = mom_taskstepper_nbval (tstep);
  unsigned nbint = mom_taskstepper_nbint (tstep);
  unsigned nbdbl = mom_taskstepper_nbdbl (tstep);
  MOM_DEBUGPRINTF (dump,
                   "mom_dumpemit_tasklet_frame nbval#%d nbint#%d nbdbl#%d",
                   nbval, nbint, nbdbl);
  fprintf (femit, "(\n");
  mom_dumpemit_value (du, (const void *) fr.fr_ptr->tfp_node);
  if (MOM_IS_DEBUGGING (dump))
    fprintf (femit, "#nbval=%d, nbint=%d, nbdbl=%d\n", nbval, nbint, nbdbl);
  fprintf (femit, "%lld\n", (long long) fr.fr_sca->tfs_state);
  for (unsigned ix = 0; ix < nbint; ix++)
    fprintf (femit, "%lld\n", (long long) fr.fr_sca->tfs_scalars[ix]);
  if (nbdbl > 0)
    {
      double *dptr = (double *) fr.fr_sca->tfs_scalars + nbint;
      char dbuf[40];
      memset (dbuf, 0, sizeof (dbuf));
      for (unsigned ix = 0; ix < nbdbl; ix++)
        {
          fprintf (femit, "%s\n",
                   mom_double_to_cstr (dptr[ix], dbuf, sizeof (dbuf)));
          memset (dbuf, 0, sizeof (dbuf));
        }
    }
  for (unsigned ix = 0; ix < nbval; ix++)
    {
      mom_dumpemit_value (du, (const void *) fr.fr_ptr->tfp_pointers[ix]);
    }
  fprintf (femit, ")tasklet_add_frame\n");
}                               /* end mom_dumpemit_tasklet_frame */


void momf_ldc_taskstepper (struct mom_item_st *itm, struct mom_loader_st *ld);

extern mom_loader_paren_sig_t momf_ldp_tasklet_add_frame;
const char momsig_ldp_tasklet_add_frame[] = "signature_loader_paren";
void
momf_ldp_tasklet_add_frame (struct mom_item_st *itm,
                            struct mom_loader_st *ld,
                            struct mom_statelem_st *elemarr,
                            unsigned elemsize)
{
  assert (ld != NULL && ld->va_itype == MOMITY_LOADER);
  MOM_DEBUGPRINTF (load, "momf_ldp_tasklet_add_frame itm=%s",
                   mom_item_cstring (itm));
  if (mom_itype (itm->itm_payload) != MOMITY_TASKLET)
    {
      MOM_WARNPRINTF
        ("tasklet_add_frame for non-tasklet item %s (%d elemsize)",
         mom_item_cstring (itm), elemsize);
      return;
    }
  if (elemsize < 2)
    {
      MOM_WARNPRINTF ("tasklet_add_frame for item %s not enough elemsize %d",
                      mom_item_cstring (itm), elemsize);
      return;
    }
  struct mom_tasklet_st *tklet = mom_dyncast_tasklet (itm->itm_payload);
  const struct mom_boxnode_st *nod = mom_ldstate_dynnode (elemarr[0]);
  MOM_DEBUGPRINTF (load,
                   "momf_ldp_tasklet_add_frame nod=%s",
                   mom_value_cstring ((const void *) nod));
  if (!nod)
    MOM_FATAPRINTF ("momf_ldp_tasklet_add_frame item %s lacking node",
                    mom_item_cstring (itm));
  if (!tklet)
    MOM_FATAPRINTF ("momf_ldp_tasklet_add_frame item %s not tasklet",
                    mom_item_cstring (itm));
  struct mom_item_st *connitm = nod->nod_connitm;
  assert (connitm && connitm->va_itype == MOMITY_ITEM);
  if (!connitm->itm_payload)
    momf_ldc_taskstepper (connitm, ld);
  const struct mom_taskstepper_st *tstep =
    mom_dyncast_taskstepper (connitm->itm_payload);
  if (!tstep)
    MOM_FATAPRINTF
      ("momf_ldp_tasklet_add_frame item %s connitm %s has no taskstepper",
       mom_item_cstring (itm), mom_item_cstring (connitm));
  int tsnbval = mom_taskstepper_nbval (tstep);
  int tsnbint = mom_taskstepper_nbint (tstep);
  int tsnbdbl = mom_taskstepper_nbdbl (tstep);
  MOM_DEBUGPRINTF (load,
                   "momf_ldp_tasklet_add_frame itm %s connitm %s tsnbval=%d tsnbint=%d tsnbdbl=%d",
                   mom_item_cstring (itm), mom_item_cstring (connitm),
                   tsnbval, tsnbint, tsnbdbl);
  if (!push_frame_tasklet_mom (itm, tklet, nod))
    MOM_FATAPRINTF
      ("momf_ldp_tasklet_add_frame item %s failed to push frame for node %s",
       mom_item_cstring (connitm), mom_value_cstring ((const void *) nod));
  if (tsnbval + tsnbint + tsnbdbl + 2 > (int) elemsize)
    MOM_FATAPRINTF
      ("momf_ldp_tasklet_add_frame item %s connitm %s elemsize %d"
       " too small for tsnbval=%d tsnbint=%d tsnbdbl=%d",
       mom_item_cstring (itm), mom_item_cstring (connitm), elemsize,
       tsnbval, tsnbint, tsnbdbl);
  struct mom_frame_st fr = mom_tasklet_nth_frame (tklet, -1);
  assert (fr.fr_sca != NULL && fr.fr_ptr != NULL);
  fr.fr_sca->tfs_state = mom_ldstate_int_def (elemarr[1], -1);
  if (tsnbint > 0)
    {
      unsigned intoff = 2;
      for (int ix = 0; ix < tsnbint; ix++)
        fr.fr_sca->tfs_scalars[ix] =
          mom_ldstate_int_def (elemarr[intoff + ix], 0);
    }
  if (tsnbdbl > 0)
    {
      unsigned dbloff = 2 + tsnbint;
      double *dptr = (double *) (fr.fr_sca->tfs_scalars + tsnbint);
      for (int ix = 0; ix < tsnbdbl; ix++)
        dptr[ix] = mom_ldstate_dbl (elemarr[dbloff + ix]);
    }
  if (tsnbval > 0)
    {
      unsigned valoff = 2 + tsnbint + tsnbdbl;
      for (int ix = 0; ix < tsnbval; ix++)
        fr.fr_ptr->tfp_pointers[ix] =
          (void *) mom_ldstate_val (elemarr[valoff + ix]);
    }
}                               /* end of momf_ldp_tasklet_add_frame */




void
mom_dumpemit_tasklet_payload (struct mom_dumper_st *du,
                              struct mom_tasklet_st *tklet)
{
  assert (du && du->du_state == MOMDUMP_EMIT);
  assert (tklet && tklet->va_itype == MOMITY_TASKLET);
  FILE *femit = du->du_emitfile;
  assert (tklet->tkl_frametop < mom_raw_size (tklet));
  fprintf (femit, "%d\n%d\n%d\n^payload_tasklet\n",
           tklet->tkl_frametop, tklet->tkl_scatop, tklet->tkl_ptrtop);
  if (mom_dumped_value (du, (const void *) tklet->tkl_excnod))
    {
      mom_dumpemit_value (du, (const void *) tklet->tkl_excnod);
      fputs ("^tasklet_excnod\n", femit);
    }
  if (mom_dumped_item (du, tklet->tkl_statitm))
    {
      mom_dumpemit_refitem (du, tklet->tkl_statitm);
      fputs ("^tasklet_statitm\n", femit);
    }
  for (unsigned lev = 0; lev < tklet->tkl_frametop; lev++)
    {
      struct mom_frame_st fr = mom_tasklet_nth_frame (tklet, lev);
      if (!fr.fr_sca || !fr.fr_ptr)
        break;
      const struct mom_boxnode_st *frnod = fr.fr_ptr->tfp_node;
      mom_dumpscan_value (du, (const void *) frnod);
      if (!frnod || frnod == MOM_EMPTY_SLOT || frnod->va_itype != MOMITY_NODE)
        break;
      struct mom_item_st *noditm = frnod->nod_connitm;
      if (!mom_dumped_item (du, noditm))
        break;
      void *funptr = NULL;
      struct mom_taskstepper_st *tstep = NULL;
      struct mom_item_st *sigitm = NULL;
      {
        mom_item_lock (noditm);
        sigitm = noditm->itm_funsig;
        if (sigitm == MOM_PREDEFITM (signature_nanotaskstep))
          funptr = noditm->itm_funptr;
        if (mom_itype (noditm->itm_payload) == MOMITY_TASKSTEPPER)
          tstep = (struct mom_taskstepper_st *) noditm->itm_payload;
        mom_item_unlock (noditm);
      }
      if (!funptr || !tstep)
        break;
      mom_dumpemit_tasklet_frame (du, tklet, noditm, tstep, fr);
    }
}                               /* end of mom_dumpemit_tasklet_payload */


const char momsig_ldc_payload_tasklet[] = "signature_loader_caret";
void
momf_ldc_payload_tasklet (struct mom_item_st *itm, struct mom_loader_st *ld)
{
  assert (itm && itm->va_itype == MOMITY_ITEM);
  assert (ld && ld->va_itype == MOMITY_LOADER);
  MOM_DEBUGPRINTF (load, "momf_ldc_payload_tasklet itm=%s",
                   mom_item_cstring (itm));
  int nbfra = mom_ldstate_int_def (mom_loader_top (ld, -2), -3);
  int nbsca = mom_ldstate_int_def (mom_loader_top (ld, -1), -2);
  int nbptr = mom_ldstate_int_def (mom_loader_top (ld, 0), -1);
  MOM_DEBUGPRINTF (load,
                   "momf_ldc_payload_tasklet itm=%s nbfra=%d nbsca=%d nbptr=%d",
                   mom_item_cstring (itm), nbfra, nbsca, nbptr);
  (void) mom_unsync_item_initialize_tasklet (itm, 9 * nbfra / 8 + 10,
                                             9 * nbsca / 8 + 20,
                                             9 * nbptr / 8 + 30);
}                               /* end of momf_ldc_payload_tasklet */


void
mom_tasklet_put_excnode (struct mom_tasklet_st *tklet,
                         const struct mom_boxnode_st *nod)
{
  if (!tklet || tklet == MOM_EMPTY_SLOT || tklet->va_itype != MOMITY_TASKLET)
    return;
  if (nod == MOM_EMPTY_SLOT)
    nod = NULL;
  if (nod && nod->va_itype != MOMITY_NODE)
    return;
  tklet->tkl_excnod = nod;
}                               /* end mom_tasklet_put_excnode */



void
mom_tasklet_put_statitm (struct mom_tasklet_st *tklet,
                         struct mom_item_st *itm)
{
  if (!tklet || tklet == MOM_EMPTY_SLOT || tklet->va_itype != MOMITY_TASKLET)
    return;
  if (itm == MOM_EMPTY_SLOT)
    itm = NULL;
  if (itm && itm->va_itype != MOMITY_ITEM)
    return;
  tklet->tkl_statitm = itm;
}                               /* end mom_tasklet_put_statitm */

const char momsig_ldc_tasklet_excnod[] = "signature_loader_caret";
void
momf_ldc_tasklet_excnod (struct mom_item_st *itm, struct mom_loader_st *ld)
{
  assert (itm && itm->va_itype == MOMITY_ITEM);
  assert (ld && ld->va_itype == MOMITY_LOADER);
  MOM_DEBUGPRINTF (load, "momf_ldc_tasklet_excnod itm=%s",
                   mom_item_cstring (itm));
  const struct mom_boxnode_st *nod =
    mom_ldstate_dynnode (mom_loader_top (ld, 0));
  if (itm->itm_payload && nod)
    mom_tasklet_put_excnode ((void *) itm->itm_payload, nod);
  mom_loader_pop (ld, 1);
}                               /* end of momf_ldc_tasklet_excnod */


const char momsig_ldc_tasklet_statitm[] = "signature_loader_caret";
void
momf_ldc_tasklet_statitm (struct mom_item_st *itm, struct mom_loader_st *ld)
{
  assert (itm && itm->va_itype == MOMITY_ITEM);
  assert (ld && ld->va_itype == MOMITY_LOADER);
  MOM_DEBUGPRINTF (load, "momf_ldc_tasklet_statitm itm=%s",
                   mom_item_cstring (itm));
  struct mom_item_st *stitm = mom_ldstate_dynitem (mom_loader_top (ld, 0));
  if (itm->itm_payload && stitm)
    mom_tasklet_put_statitm ((void *) itm->itm_payload, stitm);
  mom_loader_pop (ld, 1);
}                               /* end of momf_ldc_tasklet_statitm */




struct mom_tasklet_st *
mom_unsync_item_initialize_tasklet (struct mom_item_st *itm,
                                    unsigned frasiz,
                                    unsigned scasiz, unsigned ptrsiz)
{
  if (!itm || itm == MOM_EMPTY_SLOT || itm->va_itype != MOMITY_ITEM)
    return NULL;
  if (MOM_UNLIKELY
      (frasiz >= MOM_SIZE_MAX || scasiz >= MOM_SIZE_MAX
       || ptrsiz >= MOM_SIZE_MAX))
    {
      MOM_WARNPRINTF ("item %s failed to initialize to huge tasklet of"
                      " frasiz=%d scasiz=%d ptrsiz=%d",
                      mom_item_cstring (itm), frasiz, scasiz, ptrsiz);
      return NULL;
    }
  frasiz = mom_prime_above (frasiz);
  scasiz = mom_prime_above (scasiz);
  ptrsiz = mom_prime_above (ptrsiz);
  if (!mom_unsync_item_clear_payload (itm))
    return NULL;
  struct mom_tasklet_st *tkl = mom_gc_alloc (sizeof (struct mom_tasklet_st));
  tkl->va_itype = MOMITY_TASKLET;
  tkl->tkl_froffsets =
    mom_gc_alloc_atomic (frasiz * sizeof (struct mom_frameoffsets_st));
  tkl->tkl_scalars = mom_gc_alloc_atomic (scasiz * sizeof (intptr_t));
  tkl->tkl_pointers = mom_gc_alloc (ptrsiz * sizeof (void *));
  mom_put_size (tkl, frasiz);
  tkl->tkl_ptrsiz = ptrsiz;
  tkl->tkl_scasiz = scasiz;
  itm->itm_payload = (void *) tkl;
  return tkl;
}                               /* end of mom_unsync_item_initialize_tasklet */


void
mom_dumpemit_taskstepper (struct mom_dumper_st *du,
                          struct mom_item_st *itm,
                          struct mom_taskstepper_st *tstep)
{
  assert (du && du->du_state == MOMDUMP_EMIT);
  assert (itm && itm->va_itype == MOMITY_ITEM);
  assert (tstep && tstep->va_itype == MOMITY_TASKSTEPPER);
  Dl_info dinf = { };
  FILE *femit = du->du_emitfile;
  if (dladdr (tstep, &dinf) && dinf.dli_saddr == tstep
      && !strncmp (dinf.dli_sname, MOM_TASKSTEPPER_PREFIX,
                   strlen (MOM_TASKSTEPPER_PREFIX))
      && !strcmp (dinf.dli_sname + strlen (MOM_TASKSTEPPER_PREFIX),
                  mom_item_cstring (itm)) && femit != NULL)
    fputs ("^taskstepper\n", femit);
}                               /* end of mom_dumpemit_taskstepper */



const char momsig_ldc_taskstepper[] = "signature_loader_caret";
void
momf_ldc_taskstepper (struct mom_item_st *itm, struct mom_loader_st *ld)
{
  assert (itm && itm->va_itype == MOMITY_ITEM);
  assert (ld && ld->va_itype == MOMITY_LOADER);
  MOM_DEBUGPRINTF (load, "momf_ldc_taskstepper itm=%s",
                   mom_item_cstring (itm));
  char nambuf[MOM_PATH_MAX];
  const struct mom_taskstepper_st *tstep = NULL;
  memset (nambuf, 0, sizeof (nambuf));
  if (snprintf (nambuf, sizeof (nambuf), MOM_TASKSTEPPER_PREFIX "%s",
                mom_item_cstring (itm)) < (int) sizeof (nambuf)
      && (tstep = dlsym (mom_prog_dlhandle, nambuf)) != NULL
      && tstep->va_itype == MOMITY_TASKSTEPPER)
    itm->itm_payload = (void *) tstep;
  else
    MOM_WARNPRINTF ("failed to find tasktepper for %s : %s",
                    mom_item_cstring (itm), dlerror ());
}                               /* end of momf_ldc_taskstepper */



extern mom_nanotaskstep_sig_t momf_failure_nanostep;
MOM_TASKSTEPPER_DEFINE (failure_nanostep,
                        /*nbval: */ 2, /*nbint: */ 0, /*nbdbl: */ 0);
const char momsig_failure_nanostep[] = "signature_nanotaskstep";
struct mom_result_tasklet_st
momf_failure_nanostep (struct mom_nanotaskstep_st *nats,
                       const struct mom_boxnode_st *clos,
                       struct mom_framescalar_st *fscal,
                       struct mom_framepointer_st *fptr)
{
  assert (nats && nats->nats_magic == MOM_NANOTASKSTEP_MAGIC);
  assert (clos && clos->va_itype == MOMITY_NODE);
  assert (fscal != NULL);
  assert (fptr != NULL);
  MOM_NANOEVAL_FAILURE (&nats->nats_nanev,
                        fptr->tfp_pointers[0], fptr->tfp_pointers[1]);
}                               /* end of momf_failure_nanostep */

// eof agenda.c
