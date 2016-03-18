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



MOM_PRIVATE bool
unsync_run_stack_tasklet_mom (struct mom_item_st * tkitm,
                              struct mom_tasklet_st * tkstack)
{
  MOM_FATAPRINTF
    ("unimplemented unsync_run_stack_tasklet_mom tkitm=%s tkstack@%p",
     mom_item_cstring (tkitm), tkstack);
#warning unsync_run_stack_tasklet_mom unimplemented see mom_tasklet_st
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
    MOM_WARNPRINTF ("run_tasklet did not run tkitm %s",
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
          struct timespec ts = { 0, 0 };
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
  pthread_attr_t at = { };
  pthread_attr_init (&at);
  pthread_attr_setdetachstate (&at, PTHREAD_CREATE_DETACHED);
  for (int ix = 1; ix <= (int) nbjobs; ix++)
    pthread_create (&workthread_mom[ix], &at, agenda_thread_wrapper_mom,
                    (void *) (intptr_t) ix);
  MOM_INFORMPRINTF ("started agenda with %d workers", nbjobs);
}


////////////////////////////////////////////////////////////////
//// TASKLET SUPPORT

void
mom_tasklet_reserve (struct mom_tasklet_st *tkl, unsigned nbframes,
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
              ("too big frame size %u (for %u additional frames)",
               newfrsiz, nbframes);
          struct mom_frameoffsets_st *oldfroptr = tkl->tkl_froffsets;
          assert (oldfroptr != NULL);
          struct mom_frameoffsets_st *newfroptr =       //
            mom_gc_alloc_atomic (sizeof (struct mom_frameoffsets_st) *
                                 newfrsiz);
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
