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
mo_make_string_from_skipped_textual_file (FILE *fil, unsigned skiplines)
{
  if (!fil)
    return NULL;
  long offinit = ftell (fil);
  while (skiplines > 0)
    {
      int c = EOF;
      while ((c = fgetc (fil)) != '\n')
        if (c == EOF)
          return NULL;
      skiplines--;
    };
  long offskip = ftell (fil);
  size_t siz = 512;
  if (fileno (fil) >= 0)
    {
      struct stat st;
      memset (&st, 0, sizeof (st));
      if (!fstat (fileno (fil), &st) && st.st_size > 0
          && offskip > 0 && offinit >= 0 && offskip >= offinit)
        siz = 1 + ((10 + st.st_size - (offskip - offinit)) | 0x1ff);
    }
  if (siz > MOM_SIZE_MAX)
    MOM_FATAPRINTF ("string-from-textfile: too big size %zd", siz);
  char *buf = calloc (1, siz);
  if (!buf)
    MOM_FATAPRINTF
      ("string-from-textfile: failed to allocate buffer of %zd bytes", siz);
  FILE *fmem = open_memstream (&buf, &siz);
  if (!fmem)
    MOM_FATAPRINTF
      ("string-from-textfile: failed to open memstream of %zd bytes", siz);
  int c = EOF;
  unsigned cnt = 0;
  do
    {
      c = fgetc (fil);
      if (c == EOF)
        break;
      if (MOM_UNLIKELY (cnt > MOM_SIZE_MAX))
        MOM_FATAPRINTF ("string-from-textfile: too many %u bytes", cnt);
      if (MOM_UNLIKELY (c == 0))
        MOM_FATAPRINTF ("string-from-textfile: zero byte at offset %ld",
                        ftell (fil));
      if (MOM_UNLIKELY (cnt % 2048 == 0))
        fflush (fmem);
      if (MOM_UNLIKELY (fputc (c, fmem) == EOF))
        MOM_FATAPRINTF ("string-from-textfile: fputc failed cnt=%u", cnt);
      cnt++;
    }
  while (!feof (fil));
  fflush (fmem);
  MOM_ASSERTPRINTF (cnt == (unsigned) ftell (fmem), "wrong fmem cnt=%d", cnt);
  mo_value_t vstr = mo_make_string_len (buf, cnt);
  fclose (fmem);
  free (buf), buf = NULL;
  return vstr;
}                               /* end of mo_make_string_from_skipped_textual_file */

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
static_assert ((momhash_t) (MOM_TUPLE_H1_INIT ^ MOM_TUPLE_H2_INIT) != 0,
               "wrong MOM_TUPLE_H1_INIT & MOM_TUPLE_H2_INIT");
static const mo_sequencevalue_ty mo_empty_tuple = {
  {.mo_va_kind = mo_KTUPLE,
   .mo_va_index = 0,
   .mo_va_hash = (MOM_TUPLE_H1_INIT ^ MOM_TUPLE_H2_INIT)}
};

mo_value_t
mo_make_empty_tuple (void)
{
  return &mo_empty_tuple;
}

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
static_assert ((momhash_t) (MOM_SET_H1_INIT ^ MOM_SET_H2_INIT) != 0,
               "wrong MOM_SET_H1_INIT & MOM_SET_H2_INIT");
static const mo_sequencevalue_ty mo_empty_set = {
  {.mo_va_kind = mo_KSET,
   .mo_va_index = 0,
   .mo_va_hash = (MOM_SET_H1_INIT ^ MOM_SET_H2_INIT)}
};


mo_value_t
mo_make_empty_set (void)
{
  return &mo_empty_set;
}

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
mo_make_set_closortedseq (mo_sequencevalue_ty * seq)
{
  MOM_ASSERTPRINTF (seq != NULL && seq != MOM_EMPTY_SLOT,
                    "mo_make_set_closortedseq invalid seq @%p", seq);
  MOM_ASSERTPRINTF (((mo_hashedvalue_ty *) seq)->mo_va_kind ==
                    MOM_UNFILLEDFAKESEQKIND
                    || ((mo_hashedvalue_ty *) seq)->mo_va_kind == 0,
                    "mo_make_set_closortedseq: seq of strange kind %u",
                    (unsigned) (((mo_hashedvalue_ty *) seq)->mo_va_kind));
  unsigned sz = ((mo_sizedvalue_ty *) seq)->mo_sva_size;
  if (MOM_UNLIKELY (sz == 0))
    return &mo_empty_set;
  momhash_t h1 = MOM_SET_H1_INIT, h2 = MOM_SET_H2_INIT;
  for (unsigned ix = 0; ix < sz; ix++)
    {
      mo_objref_t curobr = seq->mo_seqobj[ix];
      momhash_t curh = ((mo_hashedvalue_ty *) curobr)->mo_va_hash;
      MOM_ASSERTPRINTF (curh != 0,
                        "mo_make_set_closortedseq: zero-hashed obj@%p",
                        curobr);
      MOM_ASSERTPRINTF (ix == 0
                        || mo_objref_cmp (seq->mo_seqobj[ix - 1], curobr) < 0,
                        "mo_make_set_closortedseq: unsorted ix=%u", ix);
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

mo_value_t
mo_set_union (mo_value_t vset1, mo_value_t vset2)
{
  unsigned card1 = 0, card2 = 0;
  const mo_setvalue_ty *set1 = mo_dyncast_set (vset1);
  const mo_setvalue_ty *set2 = mo_dyncast_set (vset2);
  if (set1)
    card1 = ((mo_sizedvalue_ty *) set1)->mo_sva_size;
  if (set2)
    card2 = ((mo_sizedvalue_ty *) set2)->mo_sva_size;
  if (MOM_UNLIKELY (card1 == 0 && card2 == 0))
    return &mo_empty_set;
  if (!card1)
    return set2;
  if (!card2)
    return set1;
  if (card1 > MOM_SIZE_MAX)
    MOM_FATAPRINTF ("too big set1@%p of size %u", set1, card1);
  if (card2 > MOM_SIZE_MAX)
    MOM_FATAPRINTF ("too big set2@%p of size %u", set2, card2);
  unsigned siz = card1 + card2 + 1;
  mo_objref_t *arr = mom_gc_alloc (siz * sizeof (mo_objref_t));
  unsigned i1 = 0, i2 = 0;
  unsigned nbun = 0;
  while (i1 < card1 && i2 < card2)
    {
      mo_objref_t ob1 = ((mo_sequencevalue_ty *) set1)->mo_seqobj[i1];
      mo_objref_t ob2 = ((mo_sequencevalue_ty *) set2)->mo_seqobj[i2];
      MOM_ASSERTPRINTF (ob1
                        && ((mo_hashedvalue_ty *) ob1)->mo_va_kind ==
                        mo_KOBJECT, "bad ob1@%p", ob1);
      MOM_ASSERTPRINTF (ob2
                        && ((mo_hashedvalue_ty *) ob2)->mo_va_kind ==
                        mo_KOBJECT, "bad ob2@%p", ob2);
      MOM_ASSERTPRINTF (nbun < siz, "nbun=%u siz=%u", nbun, siz);
      int cmp = mom_objref_cmp (ob1, ob2);
      if (cmp < 0)
        {
          arr[nbun++] = ob1;
          i1++;
        }
      else if (cmp > 0)
        {
          arr[nbun++] = ob2;
          i2++;
        }
      else
        {
          MOM_ASSERTPRINTF (ob1 == ob2, "not same ob1@%p ob2@%p", ob1, ob2);
          arr[nbun++] = ob1;
          i1++, i2++;
        }
    }
  if (i1 < card1)
    for (unsigned ix = i1; ix < card1; ix++)
      {
        mo_objref_t ob1 = ((mo_sequencevalue_ty *) set1)->mo_seqobj[ix];
        MOM_ASSERTPRINTF (ob1
                          && ((mo_hashedvalue_ty *) ob1)->mo_va_kind ==
                          mo_KOBJECT, "bad ob1@%p", ob1);
        arr[nbun++] = ob1;
      }
  else if (i2 < card2)
    for (unsigned ix = i2; ix < card2; ix++)
      {
        mo_objref_t ob2 = ((mo_sequencevalue_ty *) set2)->mo_seqobj[ix];
        MOM_ASSERTPRINTF (ob2
                          && ((mo_hashedvalue_ty *) ob2)->mo_va_kind ==
                          mo_KOBJECT, "bad ob2@%p", ob2);
        arr[nbun++] = ob2;
      }
  return mo_make_set_closortedseq (mo_sequence_filled_allocate (nbun, arr));
}                               /* end of mo_set_union */



mo_value_t
mo_set_intersection (mo_value_t vset1, mo_value_t vset2)
{
  unsigned card1 = 0, card2 = 0;
  const mo_setvalue_ty *set1 = mo_dyncast_set (vset1);
  const mo_setvalue_ty *set2 = mo_dyncast_set (vset2);
  if (set1)
    card1 = ((mo_sizedvalue_ty *) set1)->mo_sva_size;
  if (set2)
    card2 = ((mo_sizedvalue_ty *) set2)->mo_sva_size;
  if (MOM_UNLIKELY (card1 == 0 || card2 == 0))
    return &mo_empty_set;
  unsigned siz = ((card1 < card2) ? card1 : card2) + 1;
  mo_objref_t *arr = mom_gc_alloc (siz * sizeof (mo_objref_t));
  unsigned i1 = 0, i2 = 0, nbin = 0;
  while (i1 < card1 && i2 < card2)
    {
      mo_objref_t ob1 = ((mo_sequencevalue_ty *) set1)->mo_seqobj[i1];
      mo_objref_t ob2 = ((mo_sequencevalue_ty *) set2)->mo_seqobj[i2];
      MOM_ASSERTPRINTF (ob1
                        && ((mo_hashedvalue_ty *) ob1)->mo_va_kind ==
                        mo_KOBJECT, "bad ob1@%p", ob1);
      MOM_ASSERTPRINTF (ob2
                        && ((mo_hashedvalue_ty *) ob2)->mo_va_kind ==
                        mo_KOBJECT, "bad ob2@%p", ob2);
      MOM_ASSERTPRINTF (nbin < siz, "nbin=%u siz=%u", nbin, siz);
      assert (nbin < siz);
      if (ob1 == ob2)
        goto same;
      int cmp = mom_objref_cmp (ob1, ob2);
      if (cmp < 0)
        i1++;
      else if (cmp > 0)
        i2++;
      else
      same:
        {
          MOM_ASSERTPRINTF (ob1 == ob2, "not same ob1@%p ob2@%p", ob1, ob2);
          arr[nbin++] = ob1;
          i1++, i2++;
        }
    }
  return mo_make_set_closortedseq (mo_sequence_filled_allocate (nbin, arr));
}                               /* end of mo_set_intersection */


mo_value_t
mo_set_difference (mo_value_t vset1, mo_value_t vset2)
{
  unsigned card1 = 0, card2 = 0;
  const mo_setvalue_ty *set1 = mo_dyncast_set (vset1);
  const mo_setvalue_ty *set2 = mo_dyncast_set (vset2);
  if (set1)
    card1 = ((mo_sizedvalue_ty *) set1)->mo_sva_size;
  if (set2)
    card2 = ((mo_sizedvalue_ty *) set2)->mo_sva_size;
  if (card1 == 0 || set1 == set2)
    return &mo_empty_set;
  if (card2 == 0)
    return set1;
  unsigned siz = card1 + 1;
  mo_objref_t *arr = mom_gc_alloc (sizeof (mo_objref_t) * siz);
  unsigned i1 = 0, i2 = 0, nbdi = 0;
  while (i1 < card1 && i2 < card2)
    {
      mo_objref_t ob1 = ((mo_sequencevalue_ty *) set1)->mo_seqobj[i1];
      mo_objref_t ob2 = ((mo_sequencevalue_ty *) set2)->mo_seqobj[i2];
      MOM_ASSERTPRINTF (ob1
                        && ((mo_hashedvalue_ty *) ob1)->mo_va_kind ==
                        mo_KOBJECT, "bad ob1@%p", ob1);
      MOM_ASSERTPRINTF (ob2
                        && ((mo_hashedvalue_ty *) ob2)->mo_va_kind ==
                        mo_KOBJECT, "bad ob2@%p", ob2);
      assert (nbdi < siz);
      if (ob1 == ob2)
        goto same;
      int cmp = mom_objref_cmp (ob1, ob2);
      if (cmp < 0)
        {
          i1++;
          arr[nbdi++] = ob1;
        }
      else if (cmp > 0)
        i2++;
      else
      same:
        {
          MOM_ASSERTPRINTF (ob1 == ob2, "not same ob1@%p ob2@%p", ob1, ob2);
          i1++, i2++;
        }
    }
  if (i1 < card1)
    {
      for (; i1 < card1; i1++)
        {
          mo_objref_t ob1 = ((mo_sequencevalue_ty *) set1)->mo_seqobj[i1];
          MOM_ASSERTPRINTF (ob1
                            && ((mo_hashedvalue_ty *) ob1)->mo_va_kind ==
                            mo_KOBJECT, "bad ob1@%p", ob1);
          assert (nbdi < siz);
          arr[nbdi++] = ob1;
        };
    }
  return mo_make_set_closortedseq (mo_sequence_filled_allocate (nbdi, arr));
}                               /* end of mo_set_difference */


// the vectval routines are in value.c because they are easy
mo_vectvaldatapayl_ty *
mo_vectval_reserve (mo_vectvaldatapayl_ty * vect, unsigned gap)
{
  vect = mo_dyncastpayl_vectval (vect);
  if (!vect)
    {
      if (gap > MOM_SIZE_MAX)
        MOM_FATAPRINTF ("too huge gap %u for nil vector", gap);
      unsigned sz = (gap < 5) ? 7 : mom_prime_above (gap + gap / 8 + 3);
      vect =
        mom_gc_alloc (sizeof (mo_vectvaldatapayl_ty) +
                      sz * sizeof (mo_value_t));
      ((mo_hashedvalue_ty *) vect)->mo_va_kind = mo_PVECTVALDATA;
      ((mo_hashedvalue_ty *) vect)->mo_va_hash =
        (momrand_genrand_int31 () & 0xfffffff) + 2;
      ((mo_sizedvalue_ty *) vect)->mo_sva_size = sz;
      ((mo_countedpayl_ty *) vect)->mo_cpl_count = 0;
      return vect;
    }
  unsigned sz = ((mo_sizedvalue_ty *) vect)->mo_sva_size;
  unsigned cnt = ((mo_countedpayl_ty *) vect)->mo_cpl_count;
  MOM_ASSERTPRINTF (cnt <= sz, "cnt %u should be less than sz %u", cnt, sz);
  if (cnt + gap < sz)
    return vect;
  if (MOM_UNLIKELY (cnt + gap > MOM_SIZE_MAX))
    MOM_FATAPRINTF
      ("vectval_reserve wont grow vector with too large gap %u for count %u",
       gap, cnt);
  unsigned newsz = mom_prime_above (cnt + gap + cnt / 8 + gap / 8 + 7);
  if (sz == newsz)
    return vect;
  mo_vectvaldatapayl_ty *newvect =
    mom_gc_alloc (sizeof (mo_vectvaldatapayl_ty) +
                  newsz * sizeof (mo_value_t));
  ((mo_hashedvalue_ty *) newvect)->mo_va_kind = mo_PASSOVALDATA;
  ((mo_hashedvalue_ty *) newvect)->mo_va_hash =
    (momrand_genrand_int31 () & 0xfffffff) + 2;
  ((mo_sizedvalue_ty *) newvect)->mo_sva_size = newsz;
  memcpy (newvect->mo_seqval, vect->mo_seqval, cnt);
  ((mo_countedpayl_ty *) newvect)->mo_cpl_count = cnt;
  return newvect;
}                               /* end of mo_vectval_reserve */

mo_vectvaldatapayl_ty *
mo_vectval_resize (mo_vectvaldatapayl_ty * vect, unsigned newcnt)
{
  vect = mo_dyncastpayl_vectval (vect);
  if (MOM_UNLIKELY (newcnt > MOM_SIZE_MAX))
    MOM_FATAPRINTF ("vectval_resize with a huge %u new count", newcnt);
  if (!vect)
    {
      vect = mo_vectval_reserve (NULL, newcnt + newcnt / 8 + newcnt / 16 + 6);
      ((mo_countedpayl_ty *) vect)->mo_cpl_count = newcnt;
      return vect;
    }
  unsigned sz = ((mo_sizedvalue_ty *) vect)->mo_sva_size;
  unsigned cnt = ((mo_countedpayl_ty *) vect)->mo_cpl_count;
  if (cnt == newcnt)
    return vect;
  if (newcnt < cnt)
    {
      memset (vect->mo_seqval + newcnt, 0,
              (cnt - newcnt) * sizeof (mo_value_t));
      ((mo_countedpayl_ty *) vect)->mo_cpl_count = newcnt;
      if (sz > 100 && newcnt < sz / 3)
        vect = mo_vectval_reserve (vect, 0);
    }
  else if (cnt < newcnt && newcnt < sz)
    {
      memset (vect->mo_seqval + cnt, 0, (newcnt - cnt) * sizeof (mo_value_t));
      ((mo_countedpayl_ty *) vect)->mo_cpl_count = newcnt;
    }
  else
    {
      vect =
        mo_vectval_reserve (vect, 5 * (newcnt - cnt) / 4 + cnt / 16 + 10);
      ((mo_countedpayl_ty *) vect)->mo_cpl_count = newcnt;
    }
  return vect;
}                               /* end mo_vectval_resize */

mo_vectvaldatapayl_ty *
mo_vectval_append (mo_vectvaldatapayl_ty * vect, mo_value_t val)
{
  vect = mo_dyncastpayl_vectval (vect);
  if (!vect)
    {
      vect = mo_vectval_reserve (NULL, 4);
      ((mo_countedpayl_ty *) vect)->mo_cpl_count = 1;
      vect->mo_seqval[0] = val;
      return vect;
    };
  unsigned sz = ((mo_sizedvalue_ty *) vect)->mo_sva_size;
  MOM_ASSERTPRINTF (sz > 2, "too small sz %u", sz);
  unsigned cnt = ((mo_countedpayl_ty *) vect)->mo_cpl_count;
  MOM_ASSERTPRINTF (cnt <= sz, "cnt %u not less than sz %u", cnt, sz);
  if (cnt + 1 <= sz)
    {
      vect->mo_seqval[cnt] = val;
      ((mo_countedpayl_ty *) vect)->mo_cpl_count = cnt + 1;
      return vect;
    }
  vect = mo_vectval_reserve (vect, 5 + cnt / 4 + cnt / 32);
  vect->mo_seqval[cnt] = val;
  ((mo_countedpayl_ty *) vect)->mo_cpl_count = cnt;
  return vect;
}                               /* end of mo_vectval_append */


void
mo_dump_scan_vectval (mo_dumper_ty * du, mo_vectvaldatapayl_ty * vect)
{
  if (!mo_dyncastpayl_vectval (vect))
    return;
  MOM_ASSERTPRINTF (mo_dump_scanning (du), "bad du");
  unsigned sz = ((mo_sizedvalue_ty *) vect)->mo_sva_size;
  MOM_ASSERTPRINTF (sz > 2, "too small sz %u", sz);
  unsigned cnt = ((mo_countedpayl_ty *) vect)->mo_cpl_count;
  MOM_ASSERTPRINTF (cnt <= sz, "cnt %u not less than sz %u", cnt, sz);
  for (unsigned ix = 0; ix < cnt; ix++)
    {
      mo_value_t valv = vect->mo_seqval[ix];
      mo_dump_scan_value (du, valv);
    }
}                               /* end of mo_dump_scan_vectval */


mo_json_t
mo_dump_json_of_vectval (mo_dumper_ty * du, mo_vectvaldatapayl_ty * vect)
{
  if (!mo_dyncastpayl_vectval (vect))
    return json_null ();
  MOM_ASSERTPRINTF (mo_dump_emitting (du), "bad du");
  unsigned sz = ((mo_sizedvalue_ty *) vect)->mo_sva_size;
  MOM_ASSERTPRINTF (sz > 2, "too small sz %u", sz);
  unsigned cnt = ((mo_countedpayl_ty *) vect)->mo_cpl_count;
  MOM_ASSERTPRINTF (cnt <= sz, "cnt %u not less than sz %u", cnt, sz);
  json_t *jarr = json_array ();
  for (unsigned ix = 0; ix < cnt; ix++)
    {
      mo_value_t valv = vect->mo_seqval[ix];
      mo_objref_t valobr = mo_dyncast_objref (valv);
      if (valobr && !mo_dump_is_emitted_objref (du, valobr))
        json_array_append_new (jarr, json_null ());
      else
        json_array_append_new (jarr, mo_dump_json_of_value (du, valv));
    }
  return json_pack ("{so}", "vectval", jarr);
}                               /* end of mo_dump_json_of_vectval */


mo_vectvaldatapayl_ty *
mo_vectval_of_json (mo_json_t js)
{
  json_t *jarr = NULL;
  if (json_is_object (js) && (jarr = json_object_get (js, "vectval")) != NULL
      && json_is_array (jarr))
    {
      unsigned sz = json_array_size (jarr);
      mo_vectvaldatapayl_ty *vect =
        mo_vectval_reserve (NULL, sz + sz / 16 + 1);
      for (unsigned ix = 0; ix < sz; ix++)
        {
          json_t *jcomp = json_array_get (jarr, ix);
          vect = mo_vectval_append (vect, mo_value_of_json (jcomp));
        }
      return vect;
    }
  return NULL;
}                               /* end of mo_vectval_of_json */

/* end of value.c */
