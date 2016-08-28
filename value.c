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


#define MOM_TUPLE_H1_INIT 5003
#define MOM_TUPLE_H2_INIT 600073
static_assert ((MOM_TUPLE_H1_INIT ^ MOM_TUPLE_H2_INIT) != 0,
               "wrong MOM_TUPLE_H1_INIT & MOM_TUPLE_H2_INIT");
static const mo_sequencevalue_ty mo_empty_tuple = {
  ._mo = {._mo = {.mo_va_kind = mo_KTUPLE,
                  .mo_va_index = 0,
                  .mo_va_hash = (MOM_TUPLE_H1_INIT ^ MOM_TUPLE_H2_INIT)}}
};

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
  if (MOM_UNLIKELY (sz == 0))
    return &mo_empty_tuple;
  momhash_t h1 = MOM_TUPLE_H1_INIT, h2 = MOM_TUPLE_H2_INIT;
  for (unsigned ix = 0; ix < sz; ix++)
    {
      mo_objref_t curobjr =
        (mo_objref_t) mo_dyncast_object (seq->mo_seqobj[ix]);
      if (MOM_UNLIKELY (curobjr == NULL))
        {
          unsigned goodcnt = ix;
          for (unsigned restx = ix + 1; restx < sz; restx++)
            {
              mo_objref_t robr =
                (mo_objref_t) mo_dyncast_object (seq->mo_seqobj[restx]);
              if (robr == NULL)
                continue;
              seq->mo_seqobj[goodcnt++] = robr;
            }
          memset (seq->mo_seqobj + goodcnt, 0,
                  (sz - goodcnt) * sizeof (mo_objref_t));
          mo_sequencevalue_ty *newseq = mo_sequence_allocate (goodcnt);
          memcpy (newseq->mo_seqobj, seq->mo_seqobj,
                  goodcnt * sizeof (mo_objref_t));
          return mo_make_tuple_closeq (newseq);
        };
      momhash_t curh = ((mo_hashedvalue_ty *) curobjr)->mo_va_hash;
      MOM_ASSERTPRINTF (curh != 0,
                        "mo_make_tuple_closeq: zero-hashed obj@%p", curobjr);
      if (ix % 2 != 0)
        {
          h1 = (h1 * 15017 + curh) ^ (h2 % 2600057 + ix);
          h2 = (3539 * h2 + 3 * ix) ^ (13 * curh);
        }
      else
        {
          h1 = (h1 % 32600081 + 11 * ix) ^ (3 * curh + 17 * h2);
          h2 = (11 * h2) ^ curh;
        }
    }
  momhash_t h = h1 ^ h2;
  if (MOM_UNLIKELY (h < 10))
    h = 11 * (h1 & 0xffff) + 31 * (h2 & 0xffff) + 2;
  ((mo_hashedvalue_ty *) seq)->mo_va_hash = h;
  ((mo_hashedvalue_ty *) seq)->mo_va_kind = mo_KTUPLE;
  return seq;
}                               /* end mo_make_tuple_closeq */



mo_value_t
mom_make_tuple_sized (unsigned siz, /*objref-s */ ...)
{
  mo_sequencevalue_ty *seq = mo_sequence_allocate (siz);
  va_list args;
  va_start (args, siz);
  for (unsigned ix = 0; ix < siz; ix++)
    seq->mo_seqobj[ix] = va_arg (args, mo_objref_t);
  va_end (args);
  return mo_make_tuple_closeq (seq);
}                               /* end of mom_make_tuple_sized */


mo_value_t
mom_make_sentinel_tuple_ (mo_objref_t ob1, ...)
{
  va_list args;
  unsigned cnt = 0;
  va_start (args, ob1);
  while (ob1 != NULL)
    {
      cnt++;
      ob1 = va_arg (args, mo_objref_t);
    }
  va_end (args);
  mo_sequencevalue_ty *seq = mo_sequence_allocate (cnt);
  va_start (args, ob1);
  for (unsigned ix = 0; ix < cnt; ix++)
    seq->mo_seqobj[ix] = va_arg (args, mo_objref_t);
  va_end (args);
  return mo_make_tuple_closeq (seq);
}                               /* end of mom_make_sentinel_tuple_ */



////////////////////////////////////////////////////////////////
#define MOM_SET_H1_INIT 123077
#define MOM_SET_H2_INIT 50236073
static_assert ((MOM_SET_H1_INIT ^ MOM_SET_H2_INIT) != 0,
               "wrong MOM_SET_H1_INIT & MOM_SET_H2_INIT");
static const mo_sequencevalue_ty mo_empty_set = {
  ._mo = {._mo = {.mo_va_kind = mo_KSET,
                  .mo_va_index = 0,
                  .mo_va_hash = (MOM_SET_H1_INIT ^ MOM_SET_H2_INIT)}}
};


mo_value_t
mo_make_set_closeq (mo_sequencevalue_ty * seq)
{
  MOM_ASSERTPRINTF (seq != NULL && seq != MOM_EMPTY_SLOT,
                    "mo_make_set_closeq invalid seq @%p", seq);
  MOM_ASSERTPRINTF (((mo_hashedvalue_ty *) seq)->mo_va_kind ==
                    MOM_UNFILLEDFAKESEQKIND
                    || ((mo_hashedvalue_ty *) seq)->mo_va_kind == 0,
                    "mo_make_set_closeq: seq of strange kind %u",
                    (unsigned) (((mo_hashedvalue_ty *) seq)->mo_va_kind));
  unsigned sz = ((mo_sizedvalue_ty *) seq)->mo_sva_size;
  if (MOM_LIKELY (sz > 1))
    qsort (seq->mo_seqobj, sz, sizeof (mo_objref_t), mom_objref_cmp);
  unsigned ucnt = 0;            // count the number of unique non-null objref-s
  for (unsigned ix = 0; ix < sz; ix++)
    {
      mo_objref_t curobr = seq->mo_seqobj[ix] =
        mo_dyncast_objref (seq->mo_seqobj[ix]);
      if (curobr && (ix == 0 || curobr != seq->mo_seqobj[ix - 1]))
        ucnt++;
    }
  if (MOM_UNLIKELY (ucnt < sz))
    {
      mo_sequencevalue_ty *newseq = mo_sequence_allocate (ucnt);
      unsigned cnt = 0;
      for (unsigned ix = 0; ix < sz; ix++)
        {
          mo_objref_t curobr = seq->mo_seqobj[ix];
          if (curobr && (ix == 0 || curobr != seq->mo_seqobj[ix - 1]))
            newseq->mo_seqobj[cnt++] = curobr;
        };
      MOM_ASSERTPRINTF (cnt == ucnt, "cnt %u, ucnt %u", cnt, ucnt);
      seq = newseq;
      sz = ucnt;
    };
  if (MOM_UNLIKELY (sz == 0))
    return &mo_empty_set;
  momhash_t h1 = MOM_SET_H1_INIT, h2 = MOM_SET_H2_INIT;
  for (unsigned ix = 0; ix < sz; ix++)
    {
      mo_objref_t curobr = seq->mo_seqobj[ix];
      momhash_t curh = ((mo_hashedvalue_ty *) curobr)->mo_va_hash;
      MOM_ASSERTPRINTF (curh != 0,
                        "mo_make_setcloseq: zero-hashed obj@%p", curobr);
      if (ix % 2 == 0)
        {
          h1 = (h1 * 503 + curh * 17) ^ (h2 * 7 + ix);
          h2 = h2 * 2503 - curh * 31 + (ix * (1 + (curh & 0xf)));
        }
      else
        {
          h1 = (curh * 157 + ix - 3 * h1) ^ (11 * h2);
          h2 = (h2 * 163 + curh) ^ (ix + (h2 & 0x1f));
        }
    }
  momhash_t h = h1 ^ h2;
  if (MOM_UNLIKELY (h < 5))
    h = 11 * (h1 & 0xffff) + 17 * (h2 & 0xffff) + (sz & 0xff) + 1;
  ((mo_hashedvalue_ty *) seq)->mo_va_hash = h;
  ((mo_hashedvalue_ty *) seq)->mo_va_kind = mo_KSET;
  return (mo_value_t) seq;
}

mo_value_t
mom_make_set_sized (unsigned siz, /*objref-s */ ...)
{
  mo_sequencevalue_ty *seq = mo_sequence_allocate (siz);
  va_list args;
  va_start (args, siz);
  for (unsigned ix = 0; ix < siz; ix++)
    seq->mo_seqobj[ix] = va_arg (args, mo_objref_t);
  va_end (args);
  return mo_make_set_closeq (seq);
}                               /* end of mom_make_set_sized */


mo_value_t
mom_make_sentinel_set_ (mo_objref_t ob1, ...)
{
  va_list args;
  unsigned cnt = 0;
  va_start (args, ob1);
  while (ob1 != NULL)
    {
      cnt++;
      ob1 = va_arg (args, mo_objref_t);
    }
  va_end (args);
  mo_sequencevalue_ty *seq = mo_sequence_allocate (cnt);
  va_start (args, ob1);
  for (unsigned ix = 0; ix < cnt; ix++)
    seq->mo_seqobj[ix] = va_arg (args, mo_objref_t);
  va_end (args);
  return mo_make_set_closeq (seq);
}                               /* end of mom_make_sentinel_set_ */

/* end of value.c */
