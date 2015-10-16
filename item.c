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
// radix_arr_mom is mom_gc_alloc-ed but each itemname is
// mom_gc_alloc_atomic-ed
struct radix_mom_st
{
  struct mom_itemname_tu *rad_name;
  struct mom_item_st **rad_items_arr;   /* hashtable of items of same radix */
  unsigned rad_siz_items;       /* allocated size of rad_items_arr */
  unsigned rad_cnt_items;       /* used count of rad_items_arr */
};
static struct radix_mom_st *radix_arr_mom;
static unsigned radix_siz_mom;  /* allocated size of radix_arr_mom */
static unsigned radix_cnt_mom;  /* used count of radix_arr_mom */


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
    return NULL;
  pthread_mutex_lock (&radix_mtx_mom);
  assert (radix_cnt_mom <= radix_siz_mom);
  if (radix_cnt_mom == 0)
    goto end;
  int lo = 0, hi = (int) radix_cnt_mom - 1;
  while (lo + 5 < hi)
    {
      int md = (lo + hi) / 2;
      struct mom_itemname_tu *curad = radix_arr_mom[md].rad_name;
      assert (curad != NULL);
      assert (curad->itname_rank == (unsigned) md);
      int c = strncmp (curad->itname_string.cstr, str, len);
      if (c == 0 && curad->itname_string.cstr[len] == (char) 0)
        {
          rad = curad;
          goto end;
        };
      if (c <= 0)
        lo = md;
      else
        hi = md;
    };
  for (int ix = lo; ix < hi; ix++)
    {
      struct mom_itemname_tu *curad = radix_arr_mom[ix].rad_name;
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
  struct mom_itemname_tu *rad = NULL;
  if (!str || !str[0])
    return NULL;
  if (len < 0)
    len = strlen (str);
  if (len >= 256)
    MOM_FATAPRINTF ("too big length %d for name radix %.*s", len, str, len);
  if (!mom_valid_name_radix (str, len))
    return NULL;
  pthread_mutex_lock (&radix_mtx_mom);
  assert (radix_cnt_mom <= radix_siz_mom);
  if (MOM_UNLIKELY (radix_cnt_mom + 2 >= radix_siz_mom))
    {
      unsigned newsiz = ((5 * radix_cnt_mom / 4 + 10) | 0xf) + 1;
      assert (newsiz > radix_siz_mom);
      struct radix_mom_st *newarr =
        mom_gc_alloc (newsiz * sizeof (struct radix_mom_st));
      if (radix_cnt_mom > 0)
        {
          memcpy (newarr, radix_arr_mom,
                  sizeof (struct radix_mom_st) * radix_cnt_mom);
          GC_FREE (radix_arr_mom);
        }
      radix_arr_mom = newarr;
      radix_siz_mom = newsiz;
      if (MOM_UNLIKELY (radix_cnt_mom == 0))
        {                       // create the first radix
          struct mom_itemname_tu *nam =
            mom_gc_alloc_atomic (((sizeof (struct mom_itemname_tu) + len +
                                   2) | 3) + 1);
          nam->itname_rank = 0;
          nam->itname_string.va_itype = MOM_BOXSTRING_ITYPE;
          nam->itname_string.va_hsiz = len >> 8;
          nam->itname_string.va_lsiz = len & 0xffff;
          nam->itname_string.hva_hash = mom_cstring_hash_len (str, len);
          strncpy (nam->itname_string.cstr, str, len);
          nam->itname_string.cstr[len] = (char) 0;
          radix_arr_mom[0].rad_name = nam;
          const unsigned itmsiz = 4;
          radix_arr_mom[0].rad_items_arr =
            mom_gc_alloc (itmsiz * sizeof (struct mom_item_st *));
          radix_arr_mom[0].rad_siz_items = itmsiz;
          radix_arr_mom[0].rad_cnt_items = 0;
          radix_cnt_mom = 1;
          goto end;
        }
    };
  int lo = 0, hi = (int) radix_cnt_mom - 1;
  while (lo + 5 < hi)
    {
      int md = (lo + hi) / 2;
      struct mom_itemname_tu *curad = radix_arr_mom[md].rad_name;
      assert (curad != NULL);
      assert (curad->itname_rank == (unsigned) md);
      int c = strncmp (curad->itname_string.cstr, str, len);
      if (c == 0 && curad->itname_string.cstr[len] == (char) 0)
        {
          rad = curad;
          goto end;
        };
      if (c <= 0)
        lo = md;
      else
        hi = md;
    };
  for (int ix = lo; ix < (int) radix_cnt_mom; ix++)
    {
      struct mom_itemname_tu *curad = radix_arr_mom[ix].rad_name;
      struct mom_itemname_tu *nextrad = NULL;
      assert (curad != NULL);
      assert (curad->itname_rank == (unsigned) ix);
      int c = strncmp (curad->itname_string.cstr, str, len);
      if (c == 0 && curad->itname_string.cstr[len] == (char) 0)
        {
          rad = curad;
          goto end;
        }
      else if (c <= 0 && (ix + 1 >= (int) radix_cnt_mom
                          || ((nextrad = radix_arr_mom[ix + 1].rad_name)
                              && strncmp (nextrad->itname_string.cstr, str,
                                          len) > 0)))
        {                       // insert at ix
          for (int j = radix_cnt_mom; j > ix; j--)
            {
              radix_arr_mom[j] = radix_arr_mom[j - 1];
              radix_arr_mom[j].rad_name->itname_rank = j;
            };
          struct mom_itemname_tu *nam =
            mom_gc_alloc_atomic (((sizeof (struct mom_itemname_tu) + len +
                                   2) | 3) + 1);
          nam->itname_rank = 0;
          nam->itname_string.va_itype = MOM_BOXSTRING_ITYPE;
          nam->itname_string.va_hsiz = len >> 8;
          nam->itname_string.va_lsiz = len & 0xffff;
          nam->itname_string.hva_hash = mom_cstring_hash_len (str, len);
          strncpy (nam->itname_string.cstr, str, len);
          nam->itname_string.cstr[len] = (char) 0;
          radix_arr_mom[ix].rad_name = nam;
          const unsigned itmsiz = 4;
          radix_arr_mom[ix].rad_items_arr =
            mom_gc_alloc (itmsiz * sizeof (struct mom_item_st *));
          radix_arr_mom[ix].rad_siz_items = itmsiz;
          radix_arr_mom[ix].rad_cnt_items = 0;
          radix_cnt_mom++;
	  rad = radix_arr_mom + ix;
          goto end;
        }
    }
end:
  pthread_mutex_unlock (&radix_mtx_mom);
  return rad;
}                               /* end of mom_make_name_radix */
