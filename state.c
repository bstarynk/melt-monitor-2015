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
#warning missing caret load action
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
  if (!statepath || !statepath[0])
    {
      MOM_WARNPRINTF ("empty start path to load");
      return;
    }
  initialize_load_state_mom (statepath);
  first_pass_loader_mom (mom_loader);
  second_pass_loader_mom (mom_loader);
}
