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

#include "meltmoni.h"

struct mom_loader_st *mom_loader;


void
mom_fprint_ldstate (FILE *f, const struct mom_statelem_st se)
{
  if (!f)
    return;
  switch (se.st_type)
    {
    case MOMSTA_EMPTY:
      fputs ("empty", f);
      break;
    case MOMSTA_MARK:
      fprintf (f, "mark %d", se.st_mark);
      break;
    case MOMSTA_INT:
      fprintf (f, "int %ld", (long) se.st_int);
      break;
    case MOMSTA_DBL:
      fprintf (f, "double %g", se.st_dbl);
      break;
    case MOMSTA_STRING:
      fputs ("string \"", f);
      mom_output_utf8_encoded (f, se.st_str, -1);
      fputc ('"', f);
      break;
    case MOMSTA_VAL:
      {
        long lastnl = ftell (f);
        fputs ("val:", f);
        mom_output_value (f, &lastnl, 0, se.st_val);
        fputc (';', f);
      }
      break;
    default:
      fprintf (f, "?typ#%d?", (int) se.st_type);
      break;
    }
}                               /* end mom_fprint_ldstate */



const char *
mom_ldstate_cstring (const struct mom_statelem_st se)
{
  char *res = NULL;
  char *buf = NULL;
  size_t bsiz = 0;
  if (se.st_type == MOMSTA_EMPTY)
    return "empty";
  FILE *f = open_memstream (&buf, &bsiz);
  mom_fprint_ldstate (f, se);
  fflush (f);
  if (buf)
    res = GC_STRDUP (buf);
  fclose (f);
  free (buf);
  return res;
}                               /* end mom_ldstate_cstring */


#ifdef NDEBUG
void
mom_loader_push (struct mom_loader_st *ld, const struct mom_statelem_st el)
#else
void
mom_loader_push_at (struct mom_loader_st *ld, const struct mom_statelem_st el,
                    const char *fil, int line)
#endif
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
#ifdef NDEBUG
        MOM_FATAPRINTF ("too big new size %u for loader @%p", newsiz, ld);
#else
        MOM_FATAPRINTF_AT (fil, line, "too big new size %u for loader @%p",
                           newsiz, ld);
#endif
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
#ifdef NDEBUG
  MOM_DEBUGPRINTF (load, "loader_push [%d] : %s",
                   top, mom_ldstate_cstring (el));
#else
  MOM_DEBUGPRINTF_AT (fil, line, load, "loader_push [%d] :: %s",
                      top, mom_ldstate_cstring (el));
#endif
  ld->ld_stacktop = top + 1;
}

#ifdef NDEBUG
int
mom_loader_push_mark (struct mom_loader_st *ld)
#else
int
mom_loader_push_mark_at (struct mom_loader_st *ld, const char *fil, int lin)
#endif
{
  if (!ld || ld == MOM_EMPTY_SLOT || ld->va_itype != MOMITY_LOADER)
    return -1;
  unsigned oldmark = ld->ld_prevmark;
#ifdef NDEBUG
  mom_loader_push (ld, mom_ldstate_make_mark (oldmark));
#else
  mom_loader_push_at (ld, mom_ldstate_make_mark (oldmark), fil, lin);
#endif
  return oldmark;
}

#ifdef NDEBUG
void
mom_loader_pop (struct mom_loader_st *ld, unsigned nb)
#else
void
mom_loader_pop_at (struct mom_loader_st *ld, unsigned nb, const char *fil,
                   int line)
#endif
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
  while (ld->ld_prevmark >= (int) (top - nb) && ld->ld_prevmark >= 0)
    {
      ld->ld_prevmark = mom_ldstate_mark (ld->ld_stackarr[ld->ld_prevmark]);
    }
  memset (ld->ld_stackarr + top - nb - 1, 0,
          sizeof (struct mom_statelem_st) * nb);
  ld->ld_stacktop = top = (top - nb);
#ifdef NDEBUG
  MOM_DEBUGPRINTF (load, "loader_pop: nb=%d now top=%d prevmark=%d", nb,
                   ld->ld_stacktop, ld->ld_prevmark);
#else
  MOM_DEBUGPRINTF_AT (fil, line, load,
                      "loader_pop:: nb=%d now top=%d prevmark=%d", nb,
                      ld->ld_stacktop, ld->ld_prevmark);
#endif
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
}                               /* end mom_loader_pop */

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
          char *eol = strchr (linbuf, '\n');
          if (eol)
            *eol = (char) 0;
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
  ld->ld_prevmark = -1;
  ld->ld_stacktop = 0;
  memset (ld->ld_stackarr, 0,
          mom_raw_size (ld) * sizeof (struct mom_statelem_st));
  do
    {
      linlen = getline (&linbuf, &linsiz, ld->ld_file);
      if (linlen <= 0)
        break;
      linecount++;
      if (linbuf[0] == '#' || linbuf[0] == '\n')
        continue;
      char *eol = strchr (linbuf, '\n');
      if (eol)
        *eol = (char) 0;
      MOM_DEBUGPRINTF (load, "second_pass top%d siz%d prevmark%d line#%d %s",
                       ld->ld_stacktop, mom_raw_size (ld), ld->ld_prevmark,
                       linecount, linbuf);
      if (MOM_IS_DEBUGGING (load))
        {
          for (int i = (int)ld->ld_stacktop - 5; i < (int) ld->ld_stacktop;
               i++)
            if (i >= 0)
              MOM_DEBUGPRINTF (load, "second_pass stack[%d]=%s",
                               i, mom_ldstate_cstring (ld->ld_stackarr[i]));
        }
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
                   mom_ldstate_make_val ((const struct mom_hashedvalue_st *)
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
                   mom_ldstate_make_val ((const struct mom_hashedvalue_st *)
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
                 mom_ldstate_make_val ((const struct mom_hashedvalue_st *)
                                       mom_boxdouble_make (INFINITY)));
            }
          else if (!strncmp (linbuf, "-INF_", 5))
            {
              MOM_DEBUGPRINTF (load, "second_pass boxed -Infinity double");
              mom_loader_push
                (ld,
                 mom_ldstate_make_val ((const struct mom_hashedvalue_st *)
                                       mom_boxdouble_make (-INFINITY)));
            }
          else if (!strncmp (linbuf, "+NAN_", 5))
            {
              MOM_DEBUGPRINTF (load, "second_pass boxed NAN double");
              mom_loader_push
                (ld,
                 mom_ldstate_make_val ((const struct mom_hashedvalue_st *)
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
                           mom_ldstate_make_val ((const struct
                                                  mom_hashedvalue_st *) itm));
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
                 mom_ldstate_make_val ((const struct mom_hashedvalue_st *)
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
          MOM_DEBUGPRINTF (load,
                           "second_pass leftparen mark level %d previous %d",
                           ld->ld_stacktop, ld->ld_prevmark);
          mom_loader_push_mark (ld);
          if (MOM_IS_DEBUGGING (load))
            for (int d = ld->ld_stacktop - 6; d < (int) ld->ld_stacktop; d++)
              if (d >= 0)
                MOM_DEBUGPRINTF (load, "after leftparen line#%d [%d] %s",
                                 linecount, d,
                                 mom_ldstate_cstring (ld->ld_stackarr[d]));
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
          MOM_DEBUGPRINTF (load, "before caret function %s curitm=%s top#%d",
                           nambuf, mom_item_cstring (curitm),
                           ld->ld_stacktop);
          if (MOM_IS_DEBUGGING (load))
            for (int d = 1; d < 4 && d <= (int) ld->ld_stacktop; d++)
              MOM_DEBUGPRINTF (load,
                               "before caret function %s stack[%d] : %s",
                               nambuf, (int) ld->ld_stacktop - d,
                               mom_ldstate_cstring (ld->ld_stackarr
                                                    [(int) ld->ld_stacktop -
                                                     d]));
          (*caretfun) (curitm, ld);
          MOM_DEBUGPRINTF (load, "done caret function %s top#%d", nambuf,
                           ld->ld_stacktop);
        }
      //// )bar runs the action momf_ldp_bar
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
          MOM_DEBUGPRINTF (load,
                           "parenthesis function %s line#%d top#%d prevmark%d",
                           nambuf, linecount, ld->ld_stacktop,
                           ld->ld_prevmark);
          if (MOM_IS_DEBUGGING (load))
            for (int i = ld->ld_stacktop - 7; i < (int) ld->ld_stacktop; i++)
              if (i >= 0)
                MOM_DEBUGPRINTF (load, "before parenfun %s [%d] : %s", nambuf,
                                 i, mom_ldstate_cstring (ld->ld_stackarr[i]));
          mom_loader_paren_sig_t *parenfun =
            dlsym (mom_prog_dlhandle, nambuf);
          if (!parenfun)
            MOM_FATAPRINTF ("not found load paren function %s: %s",
                            nambuf, dlerror ());
          int pmark = ld->ld_prevmark;
          if (pmark >= 0 && pmark < (int) ld->ld_stacktop)
            {
              unsigned sizp = ld->ld_stacktop - pmark - 1;
              struct mom_statelem_st *elemarr
                = mom_gc_alloc ((sizp + 1) * sizeof (*elemarr));
              memcpy (elemarr, ld->ld_stackarr + pmark + 1,
                      sizp * sizeof (*elemarr));
              MOM_DEBUGPRINTF (load,
                               "before parenfun %s on itm %s sizp=%d pmark=%d",
                               nambuf, mom_item_cstring (curitm), sizp,
                               pmark);
              assert (ld->ld_stackarr[pmark].st_type == MOMSTA_MARK);
              mom_loader_pop (ld, sizp + 1);
              MOM_DEBUGPRINTF (load,
                               "before parenfun %s on itm %s with #%d stackelems&mark popped, top=%d",
                               nambuf, mom_item_cstring (curitm), sizp,
                               ld->ld_stacktop);
              if (MOM_IS_DEBUGGING (load))
                {
                  for (unsigned ix = 0; ix < sizp; ix++)
                    MOM_DEBUGPRINTF (load,
                                     "before parenfun %s elemarr[%d]= %s",
                                     nambuf, ix,
                                     mom_ldstate_cstring (elemarr[ix]));
                  MOM_DEBUGPRINTF (load,
                                   "before parenfun %s top%d siz%d prevmark%d",
                                   nambuf, ld->ld_stacktop, mom_raw_size (ld),
                                   ld->ld_prevmark);
                  for (int i = (int)ld->ld_stacktop - 6;
                       i < (int) ld->ld_stacktop; i++)
                    if (i >= 0)
                      MOM_DEBUGPRINTF (load,
                                       "before parenfun %s stack[%d]=%s",
                                       nambuf, i,
                                       mom_ldstate_cstring (ld->ld_stackarr
                                                            [i]));
                }
              (*parenfun) (curitm, ld, elemarr, sizp);
              MOM_DEBUGPRINTF (load, "after parenfun %s on itm %s",
                               nambuf, mom_item_cstring (curitm));
              MOM_DEBUGPRINTF (load,
                               "after parenfun %s top%d siz%d prevmark%d",
                               nambuf, ld->ld_stacktop, mom_raw_size (ld),
                               ld->ld_prevmark);
              for (int i = (int)ld->ld_stacktop - 6;
                   i < (int) ld->ld_stacktop; i++)
                if (i >= 0)
                  MOM_DEBUGPRINTF (load, "after parenfun %s stack[%d]=%s",
                                   nambuf, i,
                                   mom_ldstate_cstring (ld->ld_stackarr[i]));
            }
          else
            MOM_FATAPRINTF
              ("invalid previous mark#%d with stacktop #%d for parent function %s",
               pmark, ld->ld_stacktop, nambuf);
        }
      /// *foo is for defining item foo
      else if (linbuf[0] == '*' && isalpha (linbuf[1]))
        {
          const char *end = NULL;
          char *eol = strchr (linbuf, '\n');
          if (eol)
            *eol = (char) 0;
          curitm = mom_find_item_from_string (linbuf + 1, &end);
          MOM_DEBUGPRINTF (load, "second_pass curitm %s linbuf '%s'",
                           mom_item_cstring (curitm), linbuf);
          if (!curitm)
            MOM_FATAPRINTF ("failed to find defined item for line#%d: '%s'",
                            linecount, linbuf);
          unsigned siz = mom_size (ld);
          if (siz > 0)
            memset (ld->ld_stackarr, 0,
                    siz * sizeof (struct mom_statelem_st));
          ld->ld_stacktop = 0;
          ld->ld_prevmark = -1;
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
      case MOMITY_HASHASSOC:
        mom_dumpscan_hashassoc (du, (struct mom_hashassoc_st *) payl);
        return;
      default:
        MOM_DEBUGPRINTF (dump, "dumpscan_payload@%p type=%s not scanned",
                         payl, mom_itype_str (payl));
        return;
      }
}

void
mom_dumpscan_content_item (struct mom_dumper_st *du, struct mom_item_st *itm)
{
  MOM_DEBUGPRINTF (dump, "dumpscan_content_item start itm@%p %s",
                   itm, mom_item_cstring (itm));
  pthread_mutex_lock (&itm->itm_mtx);
  if (itm->itm_pattr)
    mom_dumpscan_assovaldata (du, itm->itm_pattr);
  if (itm->itm_pcomp)
    mom_dumpscan_vectvaldata (du, itm->itm_pcomp);
  if (itm->itm_funsig)
    mom_dumpscan_item (du, itm->itm_funsig);
  if (itm->itm_payload)
    mom_dumpscan_payload (du, itm->itm_payload);
  pthread_mutex_unlock (&itm->itm_mtx);
  MOM_DEBUGPRINTF (dump, "dumpscan_content_item done itm %s",
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
      struct mom_item_st *curitm =
        (struct mom_item_st *) mom_seqitems_nth (setitems, ix);
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
      du->du_itemset =
        mom_hashset_insert (du->du_itemset, (struct mom_item_st *) itm);
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
                   mom_value_cstring ((const struct mom_hashedvalue_st *)
                                      mom_hashset_to_boxset
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
