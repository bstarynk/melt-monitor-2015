// file value.c - managing values

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

mo_value_t
mo_make_string_len (const char *buf, int sz)
{
  if (buf == NULL || buf == MOM_EMPTY_SLOT)
    return NULL;
  if (sz < 0)
    sz = strlen (buf);
  if (MOM_UNLIKELY (sz >= MOM_SIZE_MAX))
    MOM_FATAPRINTF ("too long (%d) string starting with %.60s", sz, buf);
  momhash_t hs = mom_cstring_hash_len (buf, sz);
  mo_stringvalue_ty *vstr =
    mom_gc_alloc_scalar (sizeof (mo_stringvalue_ty) + sz + 1);
  ((mo_hashedvalue_ty *) vstr)->mo_va_kind = mo_KSTRING;
  ((mo_hashedvalue_ty *) vstr)->mo_va_index = 0;
  ((mo_hashedvalue_ty *) vstr)->mo_va_hash = hs;
  ((mo_sizedvalue_ty *) vstr)->mo_sva_size = sz;
  memcpy (((mo_stringvalue_ty *) vstr)->mo_cstr, buf, sz);
  return (mo_value_t) vstr;
}                               // end mo_make_string_len



mo_value_t
mo_make_string_sprintf (const char *fmt, ...)
{
  mo_value_t vres = NULL;
  va_list args;
  if (!fmt || fmt == MOM_EMPTY_SLOT)
    return NULL;
  char *mbuf = NULL;
  va_start (args, fmt);
  vasprintf (&mbuf, fmt, args);
  va_end (args);
  if (MOM_UNLIKELY (mbuf == NULL))
    MOM_FATAPRINTF ("mo_make_string_sprintf %s failed", fmt);
  vres = mo_make_string_cstr (mbuf);
  free (mbuf);
  return vres;
}                               // end mo_make_string_sprintf


mo_value_t
mo_make_tuple_closeq (mo_sequencevalue_ty * seq)
{
  MOM_ASSERTPRINTF (seq != NULL && seq != MOM_EMPTY_SLOT,
                    "mo_make_tuple_closeq invalid seq @%p", seq);
  MOM_ASSERTPRINTF (((mo_hashedvalue_ty *) seq)->mo_va_kind ==
                    MOM_UNFILLEDFAKESEQKIND
                    || ((mo_hashedvalue_ty *) seq)->mo_va_kind == 0,
                    "mo_make_tuple_closeq: seq of strange kind %u",
                    (unsigned) (((mo_hashedvalue_ty *) seq)->mo_va_kind));
  unsigned sz = ((mo_sizedvalue_ty *) seq)->mo_sva_size;
  MOM_ASSERTPRINTF (sz < MOM_SIZE_MAX,
                    "mo_make_tuple_closeq: seq of huge size %u", sz);
  momhash_t h = 5003;
  for (unsigned ix = 0; ix < sz; ix++)
    {
    }
}

/* end of value */
