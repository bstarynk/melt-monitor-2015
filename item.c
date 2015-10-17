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
  struct mom_itemname_tu *tun = NULL;
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
      int c = strncmp (str, curad->itname_string.cstr, len);
      if (c == 0 && curad->itname_string.cstr[len] == (char) 0)
        {
          tun = curad;
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
          tun = curad;
          goto end;
        };
    }
end:
  pthread_mutex_unlock (&radix_mtx_mom);
  return tun;
}                               /* end of mom_find_name_radix  */



const struct mom_itemname_tu *
mom_make_name_radix (const char *str, int len)
{
  int tix = -1;
  struct mom_itemname_tu *tun = NULL;
  if (!str || !str[0])
    return NULL;
  if (len < 0)
    len = strlen (str);
  if (len >= 256)
    MOM_FATAPRINTF ("too big length %d for name radix %.*s", len, len, str);
  MOM_DEBUGPRINTF (item, "mom_make_name_radix %.*s", len, str);
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
          tun = nam;
          tix = 0;
          MOM_DEBUGPRINTF (item, "make first radix hash %u",
                           nam->itname_string.hva_hash);
          goto end;
        }
    };
  int lo = 0, hi = (int) radix_cnt_mom - 1;
  while (lo + 5 < hi)
    {
      int md = (lo + hi) / 2;
      struct mom_itemname_tu *curad = radix_arr_mom[md].rad_name;
      assert (curad != NULL);
      MOM_DEBUGPRINTF (item, "make radix loop lo=%d hi=%d md=%d curadname %s",
                       lo, hi, md, curad->itname_string.cstr);
      assert (curad->itname_rank == (unsigned) md);
      int c = strncmp (str, curad->itname_string.cstr, len);
      if (c == 0 && curad->itname_string.cstr[len] == (char) 0)
        {
          tun = curad;
          tix = md;
          goto end;
        };
      if (c <= 0)
        lo = md;
      else
        hi = md;
    };
  MOM_DEBUGPRINTF (item, "make radix loop lo=%d radix_cnt=%d", lo,
                   radix_cnt_mom);
  for (int ix = lo; ix < (int) radix_cnt_mom; ix++)
    {
      struct mom_itemname_tu *curad = radix_arr_mom[ix].rad_name;
      struct mom_itemname_tu *nextrad = NULL;
      assert (curad != NULL);
      assert (curad->itname_rank == (unsigned) ix);
      int c = strncmp (str, curad->itname_string.cstr, len);
      MOM_DEBUGPRINTF (item, "make radix loop ix=%d curadname %s c=%d", ix,
                       curad->itname_string.cstr, c);
      if (c == 0 && curad->itname_string.cstr[len] == (char) 0)
        {
          tun = curad;
          tix = ix;
          MOM_DEBUGPRINTF (item, "make radix loop found ix=%d curadname %s",
                           ix, curad->itname_string.cstr);
          goto end;
        }
      else if (ix + 1 >= (int) radix_cnt_mom
               || (c <= 0
                   && ((nextrad = radix_arr_mom[ix + 1].rad_name)
                       && strncmp (nextrad->itname_string.cstr, str,
                                   len) > 0)))
        {                       // insert at ix
          MOM_DEBUGPRINTF (item, "make radix loop inserting at ix=%d next %s",
                           ix,
                           nextrad ? nextrad->itname_string.cstr : "?none?");
          for (int j = radix_cnt_mom; j > ix; j--)
            {
              radix_arr_mom[j] = radix_arr_mom[j - 1];
              radix_arr_mom[j].rad_name->itname_rank = j;
            };
          struct mom_itemname_tu *nam =
            mom_gc_alloc_atomic (((sizeof (struct mom_itemname_tu) + len +
                                   2) | 3) + 1);
          nam->itname_rank = ix;
          nam->itname_string.va_itype = MOM_BOXSTRING_ITYPE;
          nam->itname_string.va_hsiz = len >> 8;
          nam->itname_string.va_lsiz = len & 0xffff;
          nam->itname_string.hva_hash = mom_cstring_hash_len (str, len);
          strncpy (nam->itname_string.cstr, str, len);
          nam->itname_string.cstr[len] = (char) 0;
          MOM_DEBUGPRINTF (item,
                           "make radix loop insert ix=%d name %s hash %u", ix,
                           nam->itname_string.cstr,
                           nam->itname_string.hva_hash);
          radix_arr_mom[ix].rad_name = nam;
          const unsigned itmsiz = 4;
          radix_arr_mom[ix].rad_items_arr =
            mom_gc_alloc (itmsiz * sizeof (struct mom_item_st *));
          radix_arr_mom[ix].rad_siz_items = itmsiz;
          radix_arr_mom[ix].rad_cnt_items = 0;
          radix_cnt_mom++;
          tun = nam;
          tix = ix;
          goto end;
        }
    }
end:
  MOM_DEBUGPRINTF (item, "make name radix final radix_cnt=%d", radix_cnt_mom);
  if (MOM_IS_DEBUGGING (item))
    for (int ix = 0; ix < (int) radix_cnt_mom; ix++)
      MOM_DEBUGPRINTF (item, "make name radix [%d] %s /%u", ix,
                       radix_arr_mom[ix].rad_name->itname_string.cstr,
                       radix_arr_mom[ix].rad_name->itname_string.hva_hash);
  pthread_mutex_unlock (&radix_mtx_mom);
  MOM_DEBUGPRINTF (item, "mom_make_name_radix done %.*s tix %d", len, str,
                   tix);
  return tun;
}                               /* end of mom_make_name_radix */
