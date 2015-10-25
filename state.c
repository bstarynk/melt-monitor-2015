// file state.c - managing state

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

struct mom_loader_st *mom_loader;


void
mom_loader_push (struct mom_loader_st *ld, const struct mom_statelem_st el)
{
  if (!ld || ld == MOM_EMPTY_SLOT || ld->va_itype != MOMITY_LOADER)
    return;
  assert (ld->ld_magic == MOM_LOADER_MAGIC);
  unsigned siz = mom_raw_size (ld);
  unsigned top = ld->ld_stacktop;
  assert (siz == 0 || ld->ld_stackarr != NULL);
  if (top + 1 >= siz)
    {
      unsigned newsiz = mom_prime_above (5 * siz / 4 + 10);
      if (newsiz >= MOM_SIZE_MAX)
        MOM_FATAPRINTF ("too big new size %u for loader @%p", newsiz, ld);
      struct mom_statelem_st *newarr =
        mom_gc_alloc (newsiz * sizeof (newarr[0]));
      if (top > 0)
        memcpy (newarr, ld->ld_stackarr, top * sizeof (newarr[0]));
      ld->ld_stackarr = newarr;
      mom_put_size (ld, newsiz);
    }
  ld->ld_stackarr[top] = el;
  if (el.st_type == MOMSTA_MARK)
    ld->ld_prevmark = top;
  ld->ld_stacktop = top + 1;
}

int
mom_loader_push_mark (struct mom_loader_st *ld)
{
  if (!ld || ld == MOM_EMPTY_SLOT || ld->va_itype != MOMITY_LOADER)
    return -1;
  unsigned oldmark = ld->ld_prevmark;
  mom_loader_push (ld, mom_ldstate_make_mark (oldmark));
  return oldmark;
}

void
mom_loader_pop (struct mom_loader_st *ld, unsigned nb)
{
  if (!ld || ld == MOM_EMPTY_SLOT || ld->va_itype != MOMITY_LOADER)
    return;
  assert (ld->ld_magic == MOM_LOADER_MAGIC);
  if (nb == 0)
    return;
  unsigned siz = mom_raw_size (ld);
  unsigned top = ld->ld_stacktop;
  assert (siz == 0 || ld->ld_stackarr != NULL);
  assert (top <= siz);
  if (nb > top)
    nb = top;
  while (ld->ld_prevmark > (int) (top - nb) && ld->ld_prevmark >= 0)
    {
      ld->ld_prevmark = mom_ldstate_mark (ld->ld_stackarr[ld->ld_prevmark]);
    }
  memset (ld->ld_stackarr + top - nb - 1, 0,
          sizeof (struct mom_statelem_st) * nb);
  ld->ld_stacktop = top = (top - nb);
  if (siz > 20 && top < siz / 3)
    {
      unsigned newsiz = mom_prime_above (4 * top / 3 + 9);
      if (newsiz < siz)
        {
          struct mom_statelem_st *newarr =
            mom_gc_alloc (newsiz * sizeof (newarr[0]));
          memcpy (newarr, ld->ld_stackarr, top * sizeof (newarr[0]));
          ld->ld_stackarr = newarr;
          mom_put_size (ld, newsiz);
        }
    }
}

static void
initialize_load_state_mom (const char *statepath)
{
  struct stat st;
  memset (&st, 0, sizeof (st));
  mom_loader = mom_gc_alloc (sizeof (struct mom_loader_st));
  mom_loader->va_itype = MOMITY_LOADER;
  FILE *f = fopen (statepath, "r");
  if (!f)
    MOM_FATAPRINTF ("failed to open initial state file %s : %m", statepath);
  if (fstat (fileno (f), &st))
    MOM_FATAPRINTF ("failed to stat file %s : %m", statepath);
  long fisiz = st.st_size;
  {
    unsigned sizstack = ((10 + (int) sqrt (0.2 * fisiz)) | 0x1f) + 1;
    mom_loader->ld_stackarr =
      mom_gc_alloc (sizeof (struct mom_statelem_st) * sizstack);
    mom_put_size (mom_loader, sizstack);
  }
  {
    unsigned sizhset = ((30 + (int) sqrt (0.3 * fisiz)) | 0x1f) + 1;
    mom_loader->ld_hsetitems = mom_hashset_reserve (NULL, sizhset);
  }
  mom_loader->ld_magic = MOM_LOADER_MAGIC;
  mom_loader->ld_file = f;
  mom_loader->ld_path = GC_STRDUP (statepath);
  MOM_DEBUGPRINTF (load, "ld_path=%s fisiz=%ld", mom_loader->ld_path, fisiz);
}


void
first_pass_loader_mom (struct mom_loader_st *ld)
{
  assert (ld && ld->ld_magic == MOM_LOADER_MAGIC);
  size_t linsiz = 128;
  ssize_t linlen = 0;
  char *linbuf = malloc (linsiz);
  if (!linbuf)
    MOM_FATAPRINTF ("failed to malloc linbuf %zd", linsiz);
  memset (linbuf, 0, linsiz);
  rewind (ld->ld_file);
  int linecount = 0;
  do
    {
      linlen = getline (&linbuf, &linsiz, ld->ld_file);
      if (linlen <= 0)
        break;
      linecount++;
      if (linbuf[0] == '#' || linbuf[0] == '\n')
        continue;
      if (linbuf[0] == '*' && isalpha (linbuf[1]))
        {
          MOM_DEBUGPRINTF (load, "first_pass line#%d %s", linecount, linbuf);
          const char *end = NULL;
          struct mom_item_st *itm =
            mom_make_item_from_string (linbuf + 1, &end);
          MOM_DEBUGPRINTF (load, "first pass line#%d made item %s",
                           linecount, mom_item_cstring (itm));
          ld->ld_hsetitems = mom_hashset_insert (ld->ld_hsetitems, itm);
        }
    }
  while (!feof (ld->ld_file));
  free (linbuf);
}                               /* end first_pass_loader_mom */

void
second_pass_loader_mom (struct mom_loader_st *ld)
{
  assert (ld && ld->ld_magic == MOM_LOADER_MAGIC);
  size_t linsiz = 128;
  ssize_t linlen = 0;
  char *linbuf = malloc (linsiz);
  if (!linbuf)
    MOM_FATAPRINTF ("failed to malloc linbuf %zd", linsiz);
  memset (linbuf, 0, linsiz);
  rewind (ld->ld_file);
  int linecount = 0;
  struct mom_item_st *curitm = NULL;
  do
    {
      linlen = getline (&linbuf, &linsiz, ld->ld_file);
      if (linlen <= 0)
        break;
      linecount++;
      if (linbuf[0] == '#' || linbuf[0] == '\n')
        continue;
      MOM_DEBUGPRINTF (load, "second_pass line#%d %s", linecount, linbuf);
      /// 123 is pushing a raw integer
      /// -234_ is pushing a boxed integer
      /// 12.0 is pushing a raw double
      /// -12.3e-5_ is pushing a boxed double
      if (isdigit (linbuf[0])
          || ((linbuf[0] == '+' || linbuf[0] == '-') && isdigit (linbuf[1])))
        {
          char *end = NULL;
          if (strchr (linbuf, '.'))
            {
              double x = strtod (linbuf, &end);
              bool boxed = (*end == '_');
              MOM_DEBUGPRINTF (load, "second_pass %s double %g",
                               boxed ? "boxed" : "raw", x);
              if (boxed)
                mom_loader_push
                  (ld,
                   mom_ldstate_make_val ((const struct mom_anyvalue_st *)
                                         mom_boxdouble_make (x)));
              else
                mom_loader_push (ld, mom_ldstate_make_dbl (x));
            }
          else
            {
              long long ll = strtoll (linbuf, &end, 10);
              bool boxed = (*end == '_');
              MOM_DEBUGPRINTF (load, "second_pass %s integer %lld",
                               boxed ? "boxed" : "raw", ll);
              if (boxed)
                mom_loader_push
                  (ld,
                   mom_ldstate_make_val ((const struct mom_anyvalue_st *)
                                         mom_boxint_make (ll)));
              else
                mom_loader_push (ld, mom_ldstate_make_int (ll));
            }
        }
      /// then special cases like +NAN -INF_ ....
      else if ((linbuf[0] == '+' || linbuf[0] == '-')
               && (linbuf[1] == 'I' || linbuf[1] == 'N'))
        {
          if (!strncmp (linbuf, "+INF_", 5))
            {
              MOM_DEBUGPRINTF (load, "second_pass boxed +Infinity double");
              mom_loader_push
                (ld,
                 mom_ldstate_make_val ((const struct mom_anyvalue_st *)
                                       mom_boxdouble_make (INFINITY)));
            }
          else if (!strncmp (linbuf, "-INF_", 5))
            {
              MOM_DEBUGPRINTF (load, "second_pass boxed -Infinity double");
              mom_loader_push
                (ld,
                 mom_ldstate_make_val ((const struct mom_anyvalue_st *)
                                       mom_boxdouble_make (-INFINITY)));
            }
          else if (!strncmp (linbuf, "+NAN_", 5))
            {
              MOM_DEBUGPRINTF (load, "second_pass boxed NAN double");
              mom_loader_push
                (ld,
                 mom_ldstate_make_val ((const struct mom_anyvalue_st *)
                                       mom_boxdouble_make (NAN)));
            }
          else if (!strncmp (linbuf, "+INF", 4))
            {
              MOM_DEBUGPRINTF (load, "second_pass raw +Infinity double");
              mom_loader_push (ld, mom_ldstate_make_dbl (INFINITY));
            }
          else if (!strncmp (linbuf, "-INF", 4))
            {
              MOM_DEBUGPRINTF (load, "second_pass raw -Infinity double");
              mom_loader_push (ld, mom_ldstate_make_dbl (-INFINITY));
            }
          else if (!strncmp (linbuf, "+NAN", 4))
            {
              MOM_DEBUGPRINTF (load, "second_pass raw NAN double");
              mom_loader_push (ld, mom_ldstate_make_dbl (NAN));
            }
          else
            goto bad_line;
        }
      /// FooBar is pushing an item
      else if (isalpha (linbuf[0]))
        {
          const char *end = NULL;
          struct mom_item_st *itm = mom_find_item_from_string (linbuf, &end);
          if (!itm || (*end && !isspace (*end)))
            goto bad_line;
          MOM_DEBUGPRINTF (load, "second_pass item %s",
                           mom_item_cstring (itm));
          mom_loader_push (ld,
                           mom_ldstate_make_val ((const struct mom_anyvalue_st
                                                  *) itm));
        }
      /// "abc" is pushing a raw string & "a\n"_ is pushing a boxed string
      else if (linbuf[0] == '"')
        {
          FILE *flin = fmemopen (linbuf + 1, linlen, "r");
          if (!flin)
            MOM_FATAPRINTF ("failed to fmemopen %s (%m)", linbuf);
          struct mom_string_and_size_st ss = mom_input_quoted_utf8 (flin);
          long endpos = ftell (flin);
          assert (endpos >= 0 && endpos < linlen);
          char *end = linbuf + 1 + endpos;
          fclose (flin);
          if (*end != '"')
            goto bad_line;
          if (end[1] == '_')
            {
              MOM_DEBUGPRINTF (load, "second_pass boxed string %s", linbuf);
              mom_loader_push
                (ld,
                 mom_ldstate_make_val ((const struct mom_anyvalue_st *)
                                       mom_boxstring_make (ss.ss_str)));
            }
          else if (!end[1] || isspace (end[1]))
            {
              MOM_DEBUGPRINTF (load, "second_pass raw string %s", linbuf);
              mom_loader_push (ld, mom_ldstate_make_str (ss.ss_str));
            }
          else
            goto bad_line;
        }
      /// ~ is pushing a nil value
      else if (linbuf[0] == '~' && (!linbuf[1] || isspace (linbuf[1])))
        {
          MOM_DEBUGPRINTF (load, "second_pass nil value");
          mom_loader_push (ld, mom_ldstate_make_val (NULL));
        }
      //// ( is pushing a mark
      else if (linbuf[0] == '(' && (!linbuf[1] || isspace (linbuf[1])))
        {
          MOM_DEBUGPRINTF (load, "second_pass mark level %d previous %d",
                           ld->ld_stacktop, ld->ld_prevmark);
          mom_loader_push_mark (ld);
        }
      //// ^bar runs the action momf_ldc_bar
      else if (linbuf[0] == '^' && isalpha (linbuf[1]))
        {
          char nambuf[160];
          memset (nambuf, 0, sizeof (nambuf));
          if (linlen >= (int) sizeof (nambuf) - 20)
            MOM_FATAPRINTF ("too long caret line %s", linbuf);
          strcpy (nambuf, MOM_LOADER_CARET_PREFIX);
          char *eb = nambuf + strlen (nambuf);
          char *sb = linbuf + 1;
          while (eb < nambuf + sizeof (nambuf) - 1 &&
                 (isalnum (*sb) || *sb == '_'))
            *(eb++) = *(sb++);
          MOM_DEBUGPRINTF (load, "caret function %s", nambuf);
          mom_loader_caret_sig_t *caretfun =
            dlsym (mom_prog_dlhandle, nambuf);
          if (!caretfun)
            MOM_FATAPRINTF ("not found load caret function %s: %s",
                            nambuf, dlerror ());
          (*caretfun) (curitm, ld);
          MOM_DEBUGPRINTF (load, "done caret function %s", nambuf);
        }
      //// ^bar runs the action momf_ldc_bar
      else if (linbuf[0] == ')' && isalpha (linbuf[1]))
        {
          char nambuf[160];
          memset (nambuf, 0, sizeof (nambuf));
          if (linlen >= (int) sizeof (nambuf) - 20)
            MOM_FATAPRINTF ("too long paren line %s", linbuf);
          strcpy (nambuf, MOM_LOADER_PAREN_PREFIX);
          char *eb = nambuf + strlen (nambuf);
          char *sb = linbuf + 1;
          while (eb < nambuf + sizeof (nambuf) - 1 &&
                 (isalnum (*sb) || *sb == '_'))
            *(eb++) = *(sb++);
          MOM_DEBUGPRINTF (load, "parenthesis function %s", nambuf);
          mom_loader_paren_sig_t *parenfun =
            dlsym (mom_prog_dlhandle, nambuf);
          if (!parenfun)
            MOM_FATAPRINTF ("not found load paren function %s: %s",
                            nambuf, dlerror ());
          int pmark = ld->ld_prevmark;
          if (pmark >= 0 && pmark < (int) ld->ld_stacktop)
            {
              unsigned sizp = ld->ld_stacktop - pmark;
              struct mom_statelem_st *elemarr
                = mom_gc_alloc ((sizp + 1) * sizeof (*elemarr));
              memcpy (elemarr, ld->ld_stackarr + sizp,
                      sizp * sizeof (*elemarr));
              mom_loader_pop (ld, pmark + 1);
              MOM_DEBUGPRINTF (load,
                               "before parenfun %s on itm %s with #%d stackelems",
                               nambuf, mom_item_cstring (curitm), pmark);
              (*parenfun) (curitm, ld, elemarr, pmark);
              MOM_DEBUGPRINTF (load, "after parenfun %s on itm %s",
                               nambuf, mom_item_cstring (curitm));
            }
          else
            MOM_FATAPRINTF
              ("invalid previous mark#%d with stacktop #%d for parent function %s",
               pmark, ld->ld_stacktop, nambuf);
        }
      /// *foo is for defining item foo
      else if (linbuf[0] == '*' && isalpha (linbuf[1]))
        {
          curitm = mom_find_item_from_string (linbuf + 1, NULL);
          MOM_DEBUGPRINTF (load, "second_pass curitm %s",
                           mom_item_cstring (curitm));
        }
      else
      bad_line:
        {
          MOM_WARNPRINTF ("bad loader line %s:%d: %s",
                          ld->ld_path, linecount, linbuf);
        }
    }
  while (!feof (ld->ld_file));
  free (linbuf);
}                               /* end second_pass_loader_mom */


void
mom_load_state (const char *statepath)
{
  double startrealtime = mom_elapsed_real_time ();
  double startcputime = mom_process_cpu_time ();
  if (!statepath || !statepath[0])
    {
      MOM_WARNPRINTF ("empty start path to load");
      return;
    }
  initialize_load_state_mom (statepath);
  first_pass_loader_mom (mom_loader);
  second_pass_loader_mom (mom_loader);
  double endrealtime = mom_elapsed_real_time ();
  double endcputime = mom_process_cpu_time ();
  unsigned nbitems = mom_loader->ld_hsetitems->cda_count;
  MOM_INFORMPRINTF
    ("completed load of state from %s with %u items; "
     "in %.3f real, %.4f cpu seconds (%.3f real, %.3f cpu µs/item)",
     statepath, nbitems, endrealtime - startrealtime,
     endcputime - startcputime,
     (endrealtime - startrealtime) * (1.0e6 / nbitems),
     (endcputime - startcputime) * (1.0e6 / nbitems));
  mom_loader = NULL;
}                               /* end mom_load_state */

void
mom_dumpscan_payload (struct mom_dumper_st *du, struct mom_anyvalue_st *payl)
{
  assert (du && du->va_itype == MOMITY_DUMPER);
  if (!payl || payl == MOM_EMPTY_SLOT)
    return;
  if (payl->va_itype < MOMITY__LASTHASHED)
    mom_dumpscan_value (du, (struct mom_hashedvalue_st *) payl);
  else
    switch (payl->va_itype)
      {
      case MOMITY_ASSOVALDATA:
        mom_dumpscan_assovaldata (du, (struct mom_assovaldata_st *) payl);
        return;
      case MOMITY_VECTVALDATA:
        mom_dumpscan_vectvaldata (du, (struct mom_vectvaldata_st *) payl);
        return;
      case MOMITY_QUEUE:
        mom_dumpscan_queue (du, (struct mom_queue_st *) payl);
        return;
      case MOMITY_HASHSET:
        mom_dumpscan_hashset (du, (struct mom_hashset_st *) payl);
        return;
      case MOMITY_HASHMAP:
        mom_dumpscan_hashmap (du, (struct mom_hashmap_st *) payl);
        return;
      default:
        return;
      }
}

void
mom_dumpscan_content_item (struct mom_dumper_st *du, struct mom_item_st *itm)
{
  MOM_DEBUGPRINTF (dump, "dumpscan_content_item start %s",
                   mom_item_cstring (itm));
  pthread_mutex_lock (&itm->itm_mtx);
  if (itm->itm_pattr)
    mom_dumpscan_assovaldata (du, itm->itm_pattr);
  if (itm->itm_pcomp)
    mom_dumpscan_vectvaldata (du, itm->itm_pcomp);
  if (itm->itm_payload)
    mom_dumpscan_payload (du, itm->itm_payload);
  pthread_mutex_unlock (&itm->itm_mtx);
  MOM_DEBUGPRINTF (dump, "dumpscan_content_item done %s",
                   mom_item_cstring (itm));
}                               /* end of mom_dumpscan_content_item */

static void
dump_scan_pass_mom (struct mom_dumper_st *du)
{
  assert (du && du->va_itype == MOMITY_DUMPER);
  assert (du->du_state == MOMDUMP_NONE);
  du->du_state = MOMDUMP_SCAN;
  const struct mom_boxset_st *predset = du->du_predefset;
  assert (predset && predset->va_itype == MOMITY_SET);
  unsigned nbpred = mom_size (predset);
  MOM_DEBUGPRINTF (dump, "should scan %d predefined", nbpred);
  for (int ix = 0; ix < (int) nbpred; ix++)
    mom_dumpscan_item (du, mom_seqitems_nth (predset, ix));
  long scancount = 0;
  while (mom_queue_nonempty (du->du_itemque))
    {
      struct mom_item_st *curitm =
        (struct mom_item_st *) mom_queue_front (du->du_itemque);
      mom_queue_pop_front (du->du_itemque);
      assert (curitm && curitm->va_itype == MOMITY_ITEM);
      scancount++;
      MOM_DEBUGPRINTF (dump, "scan#%ld inside curitm %s",
                       scancount, mom_item_cstring (curitm));
      mom_dumpscan_content_item (du, curitm);
    }
  MOM_INFORMPRINTF ("scanned %ld items", scancount);
}                               /* end dump_scan_pass_mom */

void
dump_emit_predefined_header_mom (struct mom_dumper_st *du)
{
  unsigned nbpredef = mom_size (du->du_predefset);
  MOM_DEBUGPRINTF (dump, "start predefined header nbpredef=%d", nbpredef);
  int nbtry = 0;
  char predbuf[128];
  do
    {
      memset (predbuf, 0, sizeof (predbuf));
      nbtry++;
      snprintf (predbuf, sizeof (predbuf), MOM_HEADER "-r%x-p%d-t%d.tmp",
                mom_random_uint32 (), (int) getpid (), nbtry);
    }
  while (nbtry < 20 && access (predbuf, F_OK));
  du->du_predefhtmpath = mom_boxstring_make (predbuf);
  FILE *fpred = fopen (du->du_predefhtmpath->cstr, "w");
  if (!fpred)
    {
      int e = errno;
      char cwdbuf[200];
      memset (cwdbuf, 0, sizeof (cwdbuf));
      if (!getcwd (cwdbuf, sizeof (cwdbuf) - 1))
        strcpy (cwdbuf, "./");
      MOM_FATAPRINTF ("failed to open predefined header %s in %s: %s",
                      du->du_predefhtmpath->cstr, cwdbuf, strerror (e));
    }
  mom_output_gplv3_notice (fpred, "///", "", MOM_HEADER);
  fputs ("\n", fpred);
  fputs ("#ifndef" " MOM_HAS_PREDEFINED\n", fpred);
  fputs ("#error missing MOM_HAS_PREDEFINED\n", fpred);
  fputs ("#endif\n\n", fpred);
  fputs ("#undef MOM_NB_PREDEFINED\n", fpred);
  fprintf (fpred, "#define MOM_NB_PREDEFINED %d\n", nbpredef);
  fputs ("\n\n", fpred);
  for (unsigned ix = 0; ix < nbpredef; ix++)
    {
      const struct mom_item_st *predefitm
        = mom_seqitems_nth (du->du_predefset, ix);
      assert (predefitm && predefitm->va_itype == MOMITY_ITEM);
      fprintf (fpred, "MOM_HAS_PREDEFINED(%s,%u)\n",
               mom_item_cstring (predefitm), predefitm->hva_hash);
    }
  fputs ("\n\n#undef MOM_HAS_PREDEFINED\n", fpred);
  fputs ("// eof generated _mom_predef.h\n", fpred);
  fclose (fpred);
}                               /* end dump_emit_predefined_header_mom  */

static void
dump_emit_global_mom (struct mom_dumper_st *du)
{
  const struct mom_boxset_st *setitems
    = mom_hashset_to_boxset (du->du_itemset);
  unsigned nbitems = mom_size (setitems);
  MOM_DEBUGPRINTF (dump, "start globals nbitems=%d setitems=%s", nbitems,
                   mom_value_cstring ((struct mom_hashedvalue_st *)
                                      setitems));
  int nbtry = 0;
  char globuf[128];
  do
    {
      memset (globuf, 0, sizeof (globuf));
      nbtry++;
      snprintf (globuf, sizeof (globuf), MOM_GLOBAL_STATE "-r%x-p%d-t%d.tmp",
                mom_random_uint32 (), (int) getpid (), nbtry);
    }
  while (nbtry < 20 && access (globuf, F_OK));
  du->du_globaltmpath = mom_boxstring_make (globuf);
  MOM_DEBUGPRINTF (dump, "globaltmpath %s", du->du_globaltmpath->cstr);
  FILE *fglob = fopen (du->du_globaltmpath->cstr, "w");
  if (!fglob)
    {
      int e = errno;
      char cwdbuf[200];
      memset (cwdbuf, 0, sizeof (cwdbuf));
      if (!getcwd (cwdbuf, sizeof (cwdbuf) - 1))
        strcpy (cwdbuf, "./");
      MOM_FATAPRINTF ("failed to open global %s in %s: %s",
                      du->du_globaltmpath->cstr, cwdbuf, strerror (e));
    }
  du->du_emitfile = fglob;
  mom_output_gplv3_notice (fglob, "##", "", MOM_GLOBAL_STATE);
  fputs ("\n\n", fglob);
  for (unsigned ix = 0; ix < nbitems; ix++)
    {
      struct mom_item_st *curitm = mom_seqitems_nth (setitems, ix);
      assert (curitm && curitm->va_itype == MOMITY_ITEM);
      MOM_DEBUGPRINTF (dump, "should dump %s", mom_item_cstring (curitm));
      fputs ("\n", fglob);
      fprintf (fglob, "*%s\n", mom_item_cstring (curitm));
      pthread_mutex_lock (&curitm->itm_mtx);
      mom_dumpemit_item_content (du, curitm);
      pthread_mutex_unlock (&curitm->itm_mtx);
      fputs ("\n", fglob);
    }
  fputs ("\n### eof " MOM_GLOBAL_STATE "\n", fglob);
  du->du_emitfile = NULL;
  fclose (fglob);
}                               /* end of dump_emit_global_mom */

static void
dump_emit_pass_mom (struct mom_dumper_st *du)
{
  assert (du && du->va_itype == MOMITY_DUMPER);
  assert (du->du_state == MOMDUMP_SCAN);
  du->du_state = MOMDUMP_EMIT;
  dump_emit_predefined_header_mom (du);
  dump_emit_global_mom (du);
}

void
mom_dumpscan_item (struct mom_dumper_st *du, const struct mom_item_st *itm)
{
  assert (du && du->va_itype == MOMITY_DUMPER);
  if (!mom_dumpable_item (itm))
    return;
  assert (itm && itm->va_itype == MOMITY_ITEM);
  if (!mom_hashset_contains (du->du_itemset, itm))
    {
      du->du_itemset = mom_hashset_insert (du->du_itemset, itm);
      mom_queue_append (du->du_itemque, itm);
    }
}


void
mom_dump_state (void)
{
  double startrealtime = mom_elapsed_real_time ();
  double startcputime = mom_process_cpu_time ();
  struct mom_dumper_st *du = mom_gc_alloc (sizeof (struct mom_dumper_st));
  du->va_itype = MOMITY_DUMPER;
  du->du_state = MOMDUMP_NONE;
  {
    char cwdbuf[128];
    memset (cwdbuf, 0, sizeof (cwdbuf));
    if (getcwd (cwdbuf, sizeof (cwdbuf) - 1))
      MOM_INFORMPRINTF ("start of dump in %s", cwdbuf);
    else
      MOM_INFORMPRINTF ("start of dump");
  }
  du->du_itemset = mom_hashset_reserve (NULL, 100);
  du->du_itemque = mom_gc_alloc (sizeof (struct mom_queue_st));
  du->du_predefset = mom_predefined_items_boxset ();
  du->du_itemque->va_itype = MOMITY_QUEUE;
  dump_scan_pass_mom (du);
  dump_emit_pass_mom (du);
  /// should rename the temporary files
  (void) rename ("global.mom", "global.mom%");
  if (rename (du->du_globaltmpath->cstr, "global.mom"))
    MOM_FATAPRINTF ("failed to rename %s to global.mom : %m",
                    du->du_globaltmpath->cstr);
  (void) rename (MOM_HEADER, MOM_HEADER "%");
  if (rename (du->du_predefhtmpath->cstr, "_mom_predef.h"))
    MOM_FATAPRINTF ("failed to rename %s to " MOM_HEADER " : %m",
                    du->du_predefhtmpath->cstr);
  MOM_DEBUGPRINTF (dump, "itemset=%s",
                   mom_value_cstring (mom_hashset_to_boxset
                                      (du->du_itemset)));
  {
    unsigned nbitems = du->du_itemset->cda_count;
    double endrealtime = mom_elapsed_real_time ();
    double endcputime = mom_process_cpu_time ();
    MOM_INFORMPRINTF
      ("end of dump of %u items in %.3f real, %.4f cpu time (%.2f real %.2f cpu µs/item)",
       nbitems, (endrealtime - startrealtime), (endcputime - startcputime),
       (endrealtime - startrealtime) * (1.0e6 / nbitems),
       (endcputime - startcputime) * (1.0e6 / nbitems));
  };
}                               /* end mom_dump_state */
