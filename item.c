// file item.c - managing items

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

/// we temporarily store the radix in a sorted dynamic array
static pthread_mutex_t radix_mtx_mom = PTHREAD_MUTEX_INITIALIZER;
// radix_arr_mom is mom_gc_alloc-ed but each element is
// mom_gc_alloc_atomic-ed
static struct mom_itemname_tu **radix_arr_mom;
static unsigned radix_siz_mom;  /* allocated size of radix_arr_mom */
static unsigned radix_cnt_mom;  /* used count of radix_arr_mom */

#warning should also have an array of hashtables, each holding items of same common radix


bool
mom_valid_name_radix (const char *str, int len)
{
  if (!str)
    return false;
  if (len < 0)
    len = strlen (str);
  if (len <= 0)
    return false;
  if (!isalpha (str[0]))
    return false;
  const char *end = str + len;
  for (const char *pc = str; pc < end; pc++)
    {
      if (isalnum (*pc))
        continue;
      else if (*pc == '_')
        if (pc[-1] == '_')
          return false;
    }
  return true;
}                               /* end mom_valid_name_radix */

const struct mom_itemname_tu *
mom_find_name_radix (const char *str, int len)
{
  struct mom_itemname_tu *rad = NULL;
  if (!str || !str[0])
    return NULL;
  if (len < 0)
    len = strlen (str);
  if (!mom_valid_name_radix (str, len))
    return false;
  pthread_mutex_lock (&radix_mtx_mom);
  assert (radix_cnt_mom <= radix_siz_mom);
  if (radix_cnt_mom == 0)
    goto end;
  int lo = 0, hi = (int) radix_cnt_mom - 1;
  while (lo + 5 < hi)
    {
      int md = (lo + hi) / 2;
      struct mom_itemname_tu *curad = radix_arr_mom[md];
      assert (curad != NULL);
      assert (curad->itname_rank == (unsigned)md);
      int c = strncmp (curad->itname_string.cstr, str, len);
      if (c == 0 && curad->itname_string.cstr[len] == (char) 0)
        {
          rad = curad;
          goto end;
        };
      if (c < 0)
        lo = md;
      else
        hi = md;
    };
  for (int ix = lo; ix < hi; ix++)
    {
      struct mom_itemname_tu *curad = radix_arr_mom[ix];
      assert (curad != NULL);
      assert (curad->itname_rank == (unsigned) ix);
      if (!strncmp (curad->itname_string.cstr, str, len)
          && curad->itname_string.cstr[len] == (char) 0)
        {
          rad = curad;
          goto end;
        };
    }
end:
  pthread_mutex_unlock (&radix_mtx_mom);
  return rad;
}                               /* end of mom_find_name_radix  */



const struct mom_itemname_tu *
mom_make_name_radix (const char *str, int len)
{
#warning mom_make_name_radix to be coded
}                               /* end of mom_make_name_radix */
