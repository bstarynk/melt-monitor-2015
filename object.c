// file object.c - generic object support

/**   Copyright (C) 2016  Basile Starynkevitch and later the FSF
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

#define MOM_HAS_GLOBAL(Nam,Idstr,Hid,Loid,Hash) \
  mo_objref_t momglob_##Nam;
#include "_mom_global.h"

// we keep object in bucket
static struct mom_objbucket_st
{
  uint32_t bu_size;
  uint32_t bu_count;
  mo_objref_t *bu_obarr;        /* should be scalar! */
} mom_obuckarr[MOM_HID_BUCKETMAX];

static void mom_cleanup_object (void *objad, void *data);

// we choose base 60, because with a 0-9 decimal digit then 13 extended
// digits in base 60 we can express a 80-bit number.  Notice that
// log(2**80/10)/log(60) is 12.98112
//...  and capital letters O & Q are missing on purpose
#define ID_DIGITS_MOM "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNPRSTUVWXYZ"
#define ID_BASE_MOM 60

static_assert (sizeof (ID_DIGITS_MOM) - 1 == ID_BASE_MOM,
               "invalid number of id digits");


static inline const char *
num80_to_char14_mom (mom_uint128_t num, char *buf)
{
  mom_uint128_t initnum = num;
  for (int ix = 13; ix > 0; ix--)
    {
      unsigned dig = num % ID_BASE_MOM;
      num = num / ID_BASE_MOM;
      buf[ix] = ID_DIGITS_MOM[dig];
    }
  if (MOM_UNLIKELY (num > 9))
    MOM_FATAPRINTF ("bad num %d for initnum %16llx/%016llx", (int) num,
                    (unsigned long long) (initnum >> 64),
                    (unsigned long long) (initnum));
  buf[0] = '0' + num;
  return buf;
}

static inline mom_uint128_t
char14_to_num80_mom (const char *buf)
{
  mom_uint128_t num = 0;
  if (buf[0] < '0' || buf[0] > '9')
    return 0;
  const char *idigits = ID_DIGITS_MOM;
  for (int ix = 0; ix < 14; ix++)
    {
      char c = buf[ix];
      const char *p = strchr (idigits, c);
      if (!p)
        return 0;
      num = (num * ID_BASE_MOM + (mom_uint128_t) (p - idigits));
    }
  return num;
}

const char *
mo_cstring_from_hi_lo_ids (char *buf, mo_hid_t hid, mo_loid_t loid)
{
  if (buf == MOM_EMPTY_SLOT)
    buf = NULL;
  if (hid == 0 && loid == 0)
    {
      if (buf != NULL)
        {
          buf[0] = (char) 0;
          return buf;
        }
      else
        return "";
    }
  if (buf == NULL)
    buf = mom_gc_alloc_scalar (MOM_CSTRIDSIZ);
  unsigned bn = mo_hi_id_bucketnum (hid);
  char d0 = '0' + bn / (60 * 60);
  bn = bn % (60 * 60);
  char c1 = ID_DIGITS_MOM[bn / 60];
  char c2 = ID_DIGITS_MOM[bn % 60];
  mom_uint128_t wn =
    ((mom_uint128_t) (hid & 0xffff) << 64) + (mom_uint128_t) loid;
  char s16[16];
  memset (s16, 0, sizeof (s16));
  num80_to_char14_mom (wn, s16);
  char resbuf[MOM_CSTRIDSIZ];
  memset (resbuf, 0, sizeof (resbuf));
  snprintf (resbuf, sizeof (resbuf), "_%c%c%c%s", d0, c1, c2, s16);
  MOM_ASSERTPRINTF (strlen (resbuf) == MOM_CSTRIDLEN,
                    "bad result length %d in mo_cstring_from_hi_lo_ids for %s",
                    (int) strlen (resbuf), resbuf);
  strcpy (buf, resbuf);
  return buf;
}                               /* end of mo_cstring_from_hi_lo_ids */

bool
mo_get_hi_lo_ids_from_cstring (mo_hid_t * phid, mo_loid_t * ploid,
                               const char *buf)
{
  if (!buf || buf == MOM_EMPTY_SLOT || buf[0] != '_' || !isdigit (buf[1]))
    return false;
  MOM_ASSERTPRINTF (phid != NULL && ploid != NULL,
                    "bad pointers phid@%p & ploid@%p", phid, ploid);
  const char *idigits = ID_DIGITS_MOM;
  for (int i = 2; i < MOM_CSTRIDLEN; i++)
    if (!strchr (idigits, buf[i]))
      return false;
  if (!isdigit (buf[4]))
    return false;
  unsigned bn = (buf[1] - '0') * 60 * 60
    + (strchr (idigits, buf[2]) - idigits) * 60
    + (strchr (idigits, buf[3]) - idigits);
  if (bn == 0 || bn >= MOM_HID_BUCKETMAX)
    return false;
  mom_uint128_t wn = char14_to_num80_mom (buf + 4);
  mo_hid_t hid = 0;
  mo_loid_t loid = 0;
  hid = (bn << 16) + (mo_hid_t) (wn >> 64);
  loid = wn & (mom_uint128_t) 0xffffffffffffffffLL;
  *phid = hid;
  *ploid = loid;
  return true;
}

void
mo_get_some_random_hi_lo_ids (mo_hid_t * phid, mo_loid_t * ploid)
{
  MOM_ASSERTPRINTF (phid != NULL && ploid != NULL,
                    "mo_get_some_random_hi_lo_ids: bad phid@%p ploid@%p",
                    phid, ploid);
  mo_hid_t hid = 0;
  mo_loid_t loid = 0;
  do
    {
      uint32_t rh = (uint32_t) momrand_genrand_int31 ();
      if ((rh >> 16) == 0 || (rh >> 16) >= MOM_HID_BUCKETMAX)
        continue;
      else
        hid = rh;
    }
  while (hid == 0);
  do
    {
      uint32_t rm = (uint32_t) momrand_genrand_int31 ();
      if (rm < 16)
        continue;
      uint32_t rl = (uint32_t) momrand_genrand_int32 ();
      if (rl < 16 || rl > UINT32_MAX - 16)
        continue;
      loid = ((uint64_t) rm << 32) | ((uint64_t) rl);
    }
  while (loid == 0);
  *phid = hid;
  *ploid = loid;
}                               /* end of mo_get_some_random_hi_lo_ids */

momhash_t
mo_hash_from_hi_lo_ids (mo_hid_t hid, mo_loid_t loid)
{
  if (hid == 0 && loid == 0)
    return 0;
  MOM_ASSERTPRINTF (mo_hi_id_bucketnum (hid) > 0,
                    "mo_hash_from_hi_lo_ids: bad hid %u", (unsigned) hid);
  momhash_t h = 0;
  h = (hid % 2500067) ^ ((momhash_t) (loid % 357313124579LL));
  if (MOM_UNLIKELY (h < 128))
    h = 17 + (hid % 1500043) + (momhash_t) (loid % 4500049);
  MOM_ASSERTPRINTF (h > 10,
                    "mo_hash_from_hi_lo_ids: bad hash from hid=%u loid=%llu",
                    (unsigned) hid, (unsigned long long) loid);
  return h;
}                               /* end mo_hash_from_hi_lo_ids */

int
mom_objref_cmp (const void *pl, const void *pr)
{
  MOM_ASSERTPRINTF (pl != NULL && pl != MOM_EMPTY_SLOT, "bad pl=%p", pl);
  MOM_ASSERTPRINTF (pr != NULL && pr != MOM_EMPTY_SLOT, "bad pr=%p", pr);
  return mo_objref_cmp (*(mo_objref_t *) pl, *(mo_objref_t *) pr);
}

static inline int
mom_obucket_hid_loid_index (momhash_t h, mo_hid_t hid, mo_loid_t loid)
{
  unsigned bn = mo_hi_id_bucketnum (hid);
  unsigned bsz = mom_obuckarr[bn].bu_size;
  if (bsz == 0)
    return -1;
  mo_objref_t *barr = mom_obuckarr[bn].bu_obarr;
  int pos = -1;
  unsigned startix = h % bsz;
  for (unsigned ix = startix; ix < bsz; ix++)
    {
      mo_objref_t curobr = barr[ix];
      if (!curobr)
        {
          if (pos < 0)
            pos = (int) ix;
          return pos;
        }
      else if (curobr == MOM_EMPTY_SLOT)
        {
          if (pos < 0)
            pos = (int) ix;
          continue;
        }
      else if (mo_dyncast_objref (curobr) == NULL)
        {
          if (pos < 0)
            pos = (int) ix;
          continue;
        }
      else if (curobr->mo_ob_hid == hid && curobr->mo_ob_loid == loid)
        return (int) ix;
    }
  for (unsigned ix = 0; ix < startix; ix++)
    {
      mo_objref_t curobr = barr[ix];
      if (!curobr)
        {
          if (pos < 0)
            pos = (int) ix;
          return pos;
        }
      else if (curobr == MOM_EMPTY_SLOT)
        {
          if (pos < 0)
            pos = (int) ix;
          continue;
        }
      else if (mo_dyncast_objref (curobr) == NULL)
        {
          if (pos < 0)
            pos = (int) ix;
          continue;
        }
      else if (curobr->mo_ob_hid == hid && curobr->mo_ob_loid == loid)
        return (int) ix;
    }
  return pos;
}                               /* end of mom_obucket_hid_loid_index */

static void
mom_grow_obucket (unsigned bn, unsigned gap)
{
  MOM_ASSERTPRINTF (bn > 0 && bn < MOM_HID_BUCKETMAX, "bad bn:%u", bn);
  unsigned bsz = mom_obuckarr[bn].bu_size;
  unsigned cnt = mom_obuckarr[bn].bu_count;
  if (3 * (cnt + gap) + 1 < 2 * bsz)
    return;
  unsigned oldbsz = bsz;
  unsigned newsz =
    mom_prime_above ((3 * (cnt + gap)) / 2 + (cnt + gap) / 8 + 5);
  if (MOM_UNLIKELY (bsz == newsz))
    return;
  mo_objref_t *oldbarr = mom_obuckarr[bn].bu_obarr;
  mo_objref_t *newbarr = mom_gc_alloc_scalar (newsz * sizeof (mo_objref_t));
  mom_obuckarr[bn].bu_obarr = newbarr;
  mom_obuckarr[bn].bu_size = newsz;
  mom_obuckarr[bn].bu_count = 0;
  unsigned newcnt = 0;
  for (unsigned ix = 0; ix < oldbsz; ix++)
    {
      mo_objref_t oldobr = oldbarr[ix];
      if (!oldobr || oldobr == MOM_EMPTY_SLOT)
        continue;
      int pos =
        mom_obucket_hid_loid_index (((mo_hashedvalue_ty *)
                                     oldobr)->mo_va_hash,
                                    oldobr->mo_ob_hid, oldobr->mo_ob_loid);
      MOM_ASSERTPRINTF (pos >= 0
                        && newbarr[pos] == NULL, "bad pos=%d, newsz=%d", pos,
                        newsz);
      newbarr[pos] = oldobr;
      newcnt++;
    }
  MOM_ASSERTPRINTF (newcnt == cnt, "bad newcnt");
  mom_obuckarr[bn].bu_count = newcnt;
}                               /* end mom_grow_obucket */

mo_objref_t
mo_objref_find_hid_loid (mo_hid_t hid, mo_loid_t loid)
{
  if (hid == 0 && loid == 0)
    return NULL;
  unsigned bn = mo_hi_id_bucketnum (hid);
  MOM_ASSERTPRINTF (bn > 0 && bn < MOM_HID_BUCKETMAX, "bad bn:%u", bn);
  unsigned bsz = mom_obuckarr[bn].bu_size;
  if (bsz == 0)
    return NULL;
  momhash_t h = mo_hash_from_hi_lo_ids (hid, loid);
  int pos = mom_obucket_hid_loid_index (h, hid, loid);
  if (pos >= 0)
    {
      MOM_ASSERTPRINTF (pos < (int) bsz, "bad pos");
      mo_objref_t *barr = mom_obuckarr[bn].bu_obarr;
      mo_objref_t obr = barr[pos];
      if (obr && obr != MOM_EMPTY_SLOT && obr->mo_ob_hid == hid
          && obr->mo_ob_loid == loid)
        return obr;
    }
  return NULL;
}                               /* end mo_objref_find_hid_loid */

mo_objref_t
mo_objref_create_hid_loid (mo_hid_t hid, mo_loid_t loid)
{
  if (hid == 0 && loid == 0)
    return NULL;
  mo_objectvalue_ty *pob = NULL;
  unsigned bn = mo_hi_id_bucketnum (hid);
  MOM_ASSERTPRINTF (bn > 0 && bn < MOM_HID_BUCKETMAX, "bad bn:%u", bn);
  MOM_ASSERTPRINTF (hid > 0, "bad hid");
  MOM_ASSERTPRINTF (loid > 0, "bad loid");
  unsigned bsiz = mom_obuckarr[bn].bu_size;
  unsigned bcnt = mom_obuckarr[bn].bu_count;
  if (3 * bcnt + 1 > 2 * bsiz)
    {
      mom_grow_obucket (bn, bcnt / 16 + 2);
      bsiz = mom_obuckarr[bn].bu_size;
    }
  momhash_t h = mo_hash_from_hi_lo_ids (hid, loid);
  int pos = mom_obucket_hid_loid_index (h, hid, loid);
  MOM_ASSERTPRINTF (pos >= 0 && pos < (int) bsiz, "bad pos");
  mo_objref_t obr = mom_obuckarr[bn].bu_obarr[pos];
  if (obr && obr != MOM_EMPTY_SLOT && obr->mo_ob_hid == hid
      && obr->mo_ob_loid == loid)
    return obr;
  pob = mom_gc_alloc (sizeof (mo_objectvalue_ty));
  obr = pob;
  mom_obuckarr[bn].bu_obarr[pos] = pob;
  mom_obuckarr[bn].bu_count = bcnt + 1;
  pob->mo_va_kind = mo_KOBJECT;
  pob->mo_va_index = 0;
  pob->mo_va_hash = h;
  pob->mo_ob_hid = hid;
  pob->mo_ob_loid = loid;
  pob->mo_ob_mtime = 0;
  pob->mo_ob_class = NULL;
  pob->mo_ob_attrs = NULL;
  pob->mo_ob_comps = NULL;
  pob->mo_ob_paylkind = NULL;
  pob->mo_ob_payldata = NULL;
  GC_REGISTER_FINALIZER_IGNORE_SELF (pob, mom_cleanup_object, NULL, NULL,
                                     NULL);
  return pob;
}                               /* end mo_objref_create_hid_loid */



mo_objref_t
mo_make_object (void)
{
  mo_hid_t hid = 0;
  mo_loid_t loid = 0;
  mo_objref_t obr = NULL;
  do
    {
      mo_get_some_random_hi_lo_ids (&hid, &loid);
      unsigned bn = mo_hi_id_bucketnum (hid);
      MOM_ASSERTPRINTF (bn > 0 && bn < MOM_HID_BUCKETMAX, "bad bn:%u", bn);
      MOM_ASSERTPRINTF (hid > 0, "bad hid");
      MOM_ASSERTPRINTF (loid > 0, "bad loid");
      unsigned bsiz = mom_obuckarr[bn].bu_size;
      unsigned bcnt = mom_obuckarr[bn].bu_count;
      if (bsiz == 0)
        {
          bsiz = 7;
          mom_obuckarr[bn].bu_obarr =
            mom_gc_alloc_scalar (bsiz * sizeof (mo_objref_t));
          mom_obuckarr[bn].bu_size = bsiz;
          mom_obuckarr[bn].bu_count = 0;
        }
      else if (3 * bcnt + 1 > 2 * bsiz)
        {
          mom_grow_obucket (bn, bcnt / 16 + 2);
          bsiz = mom_obuckarr[bn].bu_size;
        }
      momhash_t h = mo_hash_from_hi_lo_ids (hid, loid);
      int pos = mom_obucket_hid_loid_index (h, hid, loid);
      MOM_ASSERTPRINTF (pos >= 0 && pos < (int) bsiz, "bad pos");
      mo_objref_t oldobr = mom_obuckarr[bn].bu_obarr[pos];
      if (oldobr && oldobr != MOM_EMPTY_SLOT
          && oldobr && oldobr->mo_ob_hid == hid && oldobr->mo_ob_loid == loid)
        continue;
      mo_objectvalue_ty *pob =
        obr = mom_gc_alloc (sizeof (mo_objectvalue_ty));
      mom_obuckarr[bn].bu_obarr[pos] = obr;
      mom_obuckarr[bn].bu_count++;
      pob->mo_va_kind = mo_KOBJECT;
      pob->mo_va_index = 0;
      pob->mo_va_hash = h;
      pob->mo_ob_hid = hid;
      pob->mo_ob_loid = loid;
      time (&pob->mo_ob_mtime);
      pob->mo_ob_class = NULL;
      pob->mo_ob_attrs = NULL;
      pob->mo_ob_comps = NULL;
      pob->mo_ob_paylkind = NULL;
      pob->mo_ob_payldata = NULL;
      GC_REGISTER_FINALIZER_IGNORE_SELF (obr, mom_cleanup_object, NULL, NULL,
                                         NULL);
    }
  while (MOM_UNLIKELY (obr == NULL));
  return obr;
}                               /* end of mo_make_object */


void
mo_objref_really_clear_payload (mo_objref_t obr)
{
  mo_objectvalue_ty *pob = (mo_objectvalue_ty *) mo_dyncast_objref (obr);
  if (!pob)
    return;
  mo_objref_t paylkindobr = pob->mo_ob_paylkind;
  void *payldata = pob->mo_ob_payldata;
  momhash_t hpk = mo_objref_hash (paylkindobr);
  if (hpk)
    {
#define MOM_NBCASE_PAYLOAD 307
#define CASE_PAYLOAD_MOM(Ob) momphash_##Ob % MOM_NBCASE_PAYLOAD: \
      if (paylkindobr != MOM_PREDEF(Ob)) goto defaultpayloadcase; \
	goto labpayl_##Ob; labpayl_##Ob
      switch (hpk % MOM_NBCASE_PAYLOAD)
        {
        case CASE_PAYLOAD_MOM (payload_file):
          if (payldata)
            fclose ((FILE *) payldata);
          break;
        case CASE_PAYLOAD_MOM (payload_buffer):
          {
            if (payldata)
              {
                mo_bufferpayl_ty *bpy = (mo_bufferpayl_ty *) payldata;
                if (bpy->mo_buffer_nmagic != MOM_BUFFER_MAGIC)
                  MOM_FATAPRINTF ("clean_payload obr=%s corrupted buffer",
                                  mo_objref_pnamestr (obr));
                if (bpy->mo_buffer_memstream)
                  {
                    fclose (bpy->mo_buffer_memstream);
                    bpy->mo_buffer_memstream = NULL;
                  };
                if (bpy->mo_buffer_zone)
                  {
                    free (bpy->mo_buffer_zone);
                    bpy->mo_buffer_zone = NULL;
                  }
                memset (bpy, 0, sizeof (mo_bufferpayl_ty));
                free (bpy);
              }
          }
          break;
        case CASE_PAYLOAD_MOM (payload_gobject):
          {
            extern void mo_objref_cleanup_gobject (mo_objref_t);
            mo_objref_cleanup_gobject (obr);
          }
          break;
        case CASE_PAYLOAD_MOM (payload_c_emit):
          {
            extern void mo_objref_cleanup_cemit (mo_objref_t);
            mo_objref_cleanup_cemit (obr);
          }
          break;
          ///
        default:
        defaultpayloadcase:
          break;
        }
#undef MOM_NBCASE_PAYLOAD
#undef CASE_PAYLOAD_MOM
    }
  obr->mo_ob_payldata = NULL;
  obr->mo_ob_paylkind = NULL;
}                               /* end of mo_objref_really_clear_payload */

static void
mom_cleanup_object (void *objad, void *data MOM_UNUSED)
{
  mo_objref_t obr = objad;
  if (obr->mo_va_kind != mo_KOBJECT)
    return;
  momhash_t h = obr->mo_va_hash;
  mo_hid_t hid = obr->mo_ob_hid;
  mo_loid_t loid = obr->mo_ob_loid;
  unsigned bn = mo_hi_id_bucketnum (hid);
  int pos = mom_obucket_hid_loid_index (h, hid, loid);
  MOM_ASSERTPRINTF (pos >= 0 && mom_obuckarr[bn].bu_obarr[pos] == obr,
                    "corrupted bucket#%d pos:%d", bn, pos);
  MOM_ASSERTPRINTF (mo_objref_find_hid_loid (hid, loid) == obr,
                    "unfound obr");
  if (obr->mo_ob_paylkind)
    mo_objref_really_clear_payload (obr);
  mom_obuckarr[bn].bu_obarr[pos] = MOM_EMPTY_SLOT;
  mom_obuckarr[bn].bu_count--;
  memset (obr, 0, sizeof (mo_objectvalue_ty));
}                               /* end mom_cleanup_object */



static mo_hashsetpayl_ty *mom_predefined_hset;
static void
mom_add_predefined (mo_objectvalue_ty * ob)
{
  MOM_ASSERTPRINTF (ob && ob->mo_va_kind == mo_KOBJECT, "bad ob");
  mo_hid_t hid = ob->mo_ob_hid;
  mo_loid_t loid = ob->mo_ob_loid;
  momhash_t h = mo_hash_from_hi_lo_ids (hid, loid);
  MOM_ASSERTPRINTF (h > 0 && h == ob->mo_va_hash, "bad hash");
  unsigned bn = mo_hi_id_bucketnum (hid);
  MOM_ASSERTPRINTF (bn > 0 && bn < MOM_HID_BUCKETMAX, "bad bn:%u", bn);
  if (MOM_LIKELY (mom_obuckarr[bn].bu_obarr == NULL))
    {
      unsigned bsz = 7;
      mom_obuckarr[bn].bu_obarr =
        mom_gc_alloc_scalar (bsz * sizeof (mo_objref_t));
      mom_obuckarr[bn].bu_size = bsz;
      mom_obuckarr[bn].bu_count = 0;
    }
  else
    mom_grow_obucket (bn, 3);
  int pos = mom_obucket_hid_loid_index (h, hid, loid);
  MOM_ASSERTPRINTF (pos >= 0
                    && mom_obuckarr[bn].bu_obarr[pos] == NULL, "bad pos");
  mom_obuckarr[bn].bu_obarr[pos] = ob;
  mom_obuckarr[bn].bu_count++;
  mom_predefined_hset = mo_hashset_put (mom_predefined_hset, ob);
}                               /* end mom_add_predefined */



/***************** PREDEFINED ****************/
/* define each predefined */
#define MOM_HAS_PREDEFINED(Nam,Idstr,Hid,Loid,Hash)	\
mo_objectvalue_ty MOM_VARPREDEF(Nam) = {		\
  .mo_va_kind= mo_KOBJECT,			        \
  .mo_va_index= mo_SPACE_PREDEF,			\
  .mo_va_hash= Hash,					\
  .mo_ob_mtime= 0,     					\
  .mo_ob_hid= Hid,					\
  .mo_ob_loid= Loid,					\
  .mo_ob_class= NULL,					\
  .mo_ob_attrs= NULL,					\
  .mo_ob_comps= NULL,					\
  .mo_ob_paylkind= NULL,	       			\
  .mo_ob_payldata= NULL					\
};                              /* end predefined */

#include "_mom_predef.h"


void
mo_objref_put_space (mo_objref_t obr, enum mo_space_en spa)
{
  mo_objectvalue_ty *pob = mo_dyncast_objref (obr);
  if (!pob)
    return;
  enum mo_space_en oldspa = mo_objref_space (obr);
  if (oldspa == spa)
    return;
  if (MOM_UNLIKELY (oldspa == mo_SPACE_PREDEF))
    {
      MOM_ASSERTPRINTF (mo_hashset_contains (mom_predefined_hset, obr),
                        "obr was pedefined");
      mom_predefined_hset = mo_hashset_remove (mom_predefined_hset, obr);
    }
  if (MOM_UNLIKELY (spa == mo_SPACE_PREDEF))
    {
      mom_predefined_hset = mo_hashset_put (mom_predefined_hset, obr);
    }
  pob->mo_va_index = spa;
}                               /* end mo_objref_put_space */

mo_value_t
mo_predefined_objects_set (void)
{
  return mo_hashset_elements_set (mom_predefined_hset);
}                               /* end mo_predefined_objects_set */



void
mo_objref_put_signature_payload (mo_objref_t obr, mo_objref_t sigobr)
{
  if (!mo_dyncast_objref (obr))
    return;
  if (!mo_dyncast_objref (sigobr))
    return;
  if (sigobr->mo_ob_class != MOM_PREDEF (signature_class))
    {
      MOM_WARNPRINTF
        ("put_signature_payload: in obr=%s sigobr=%s (of bad class %s)",
         mo_objref_pnamestr (obr), mo_objref_pnamestr (sigobr),
         mo_objref_pnamestr (sigobr->mo_ob_class));
      return;
    }
  mo_objref_clear_payload (obr);
  char bufobrid[MOM_CSTRIDSIZ];
  memset (bufobrid, 0, sizeof (bufobrid));
  mo_cstring_from_hi_lo_ids (bufobrid, obr->mo_ob_hid, obr->mo_ob_loid);
  char bufsigid[MOM_CSTRIDSIZ];
  memset (bufsigid, 0, sizeof (bufsigid));
  mo_cstring_from_hi_lo_ids (bufsigid, sigobr->mo_ob_hid, sigobr->mo_ob_loid);
  char synamebuf[MOM_CSTRIDSIZ + 40];
  memset (synamebuf, 0, sizeof (synamebuf));
  snprintf (synamebuf, sizeof (synamebuf), MOM_FUNC_PREFIX "%s", bufobrid);
  void *syad = dlsym (mom_prog_dlhandle, synamebuf);
  mo_value_t errmsgv = NULL;
  if (!syad)
    {
      errmsgv =
        mo_make_string_sprintf ("dlsym'%s' failed: %s", synamebuf,
                                dlerror ());
      memset (synamebuf, 0, sizeof (synamebuf));
      snprintf (synamebuf, sizeof (synamebuf), MOM_CODE_PREFIX "%s",
                bufobrid);
      syad = dlsym (mom_prog_dlhandle, synamebuf);
      if (!syad)
        errmsgv =
          mo_make_string_sprintf ("dlsym'%s' failed: %s", synamebuf,
                                  dlerror ());
      else
        errmsgv = NULL;
      if (errmsgv)
        {
          MOM_WARNPRINTF
            ("put_signature_payload: in obr=%s sigobr=%s failure %s",
             mo_objref_pnamestr (obr), mo_objref_pnamestr (sigobr),
             mo_string_cstr (errmsgv));
          return;
        }
    };
  MOM_ASSERTPRINTF (syad != NULL,
                    "put_signature_payload: syad is null obr=%s sigobr=%s",
                    mo_objref_pnamestr (obr), mo_objref_pnamestr (sigobr));
  memset (synamebuf, 0, sizeof (synamebuf));
  snprintf (synamebuf, sizeof (synamebuf), MOM_SIGNATURE_PREFIX "%s",
            bufobrid);
  char *sigstr = dlsym (mom_prog_dlhandle, synamebuf);
  if (!sigstr)
    {
      MOM_WARNPRINTF
        ("put_signature_payload: in obr=%s sigobr=%s missing signature; dlsym'%s' failed: %s",
         mo_objref_pnamestr (obr), mo_objref_pnamestr (sigobr), synamebuf,
         dlerror ());
      return;
    }
  if (!(isalpha (sigstr[0]) || sigstr[0] == '_') || !(isalnum (sigstr[1]))
      || !strcmp (sigstr, bufsigid))
    {
      MOM_WARNPRINTF
        ("put_signature_payload: in obr=%s sigobr=%s incompatible signature got %s expecting %s",
         mo_objref_pnamestr (obr), mo_objref_pnamestr (sigobr), sigstr,
         bufsigid);
      return;
    }
  obr->mo_ob_payldata = syad;
  obr->mo_ob_paylkind = sigobr;
}                               /* end of mo_objref_put_signature_payload */


/// see also e.g. http://stackoverflow.com/q/39366248/841108
mo_value_t
mom_set_complete_objectid (const char *prefix)
{
  if (!prefix || prefix == MOM_EMPTY_SLOT
      || prefix[0] != '_'
      || !isdigit (prefix[1]) || !isalnum (prefix[2]) || !isalnum (prefix[3]))
    return NULL;
  size_t preflen = strlen (prefix);
  if (preflen > MOM_CSTRIDLEN)
    return NULL;
  unsigned bn = (prefix[1] - '0') * 3600;
  const char *idigits = ID_DIGITS_MOM;
  char *pc2 = strchr (idigits, prefix[2]);
  if (!pc2)
    return NULL;
  char *pc3 = strchr (idigits, prefix[3]);
  if (!pc3)
    return NULL;
  bn += 60 * (pc2 - idigits) + (pc3 - idigits);
  if (bn == 0 || bn >= MOM_HID_BUCKETMAX)
    return NULL;
  unsigned busiz = mom_obuckarr[bn].bu_size;
  unsigned bucnt = mom_obuckarr[bn].bu_count;
  mo_objref_t *buarr = mom_obuckarr[bn].bu_obarr;
  if (!busiz || !buarr || !bucnt)
    return mo_make_empty_set ();
  unsigned matchsiz = mom_prime_above (bucnt / 2 + 10);
  mo_objref_t *matcharr = mom_gc_alloc (matchsiz * sizeof (mo_objref_t));
  unsigned matchcnt = 0;
  for (unsigned ix = 0; ix < busiz; ix++)
    {
      mo_objref_t curobj = buarr[ix];
      if (!curobj || curobj == MOM_EMPTY_SLOT)
        continue;
      MOM_ASSERTPRINTF (mo_dyncast_objref (curobj), "bad curobj ix#%d", ix);
      char bufid[MOM_CSTRIDSIZ];
      memset (bufid, 0, sizeof (bufid));
      mo_objref_idstr (bufid, curobj);
      if (!strncmp (prefix, bufid, preflen))
        {
          if (MOM_UNLIKELY (matchcnt + 1 >= matchsiz))
            {
              unsigned newsiz = mom_prime_above (3 * matchcnt / 2 + 10);
              mo_objref_t *newarr =
                mom_gc_alloc (newsiz * sizeof (mo_objref_t));
              memcpy (newarr, matcharr, matchcnt * sizeof (mo_objref_t));
              matcharr = newarr;
              matchsiz = newsiz;
            }
          MOM_ASSERTPRINTF (matchcnt < bucnt, "buggy matchcnt=%d bucnt=%d",
                            matchcnt, bucnt);
          matcharr[matchcnt++] = curobj;
        }
    }
  if (!matchcnt)
    return mo_make_empty_set ();
  return
    mo_make_set_closeq (mo_sequence_filled_allocate (matchcnt, matcharr));
}                               /* end mom_set_complete_objectid */

void
mom_init_objects (void)
{
  static bool inited;
  MOM_ASSERTPRINTF (!inited, "should not initialize objects twice");
  if (inited)
    return;
  inited = true;
  mom_predefined_hset = mo_hashset_reserve (NULL, 5 + MOM_NB_PREDEFINED);
#define MOM_HAS_PREDEFINED(Nam,Idstr,Hid,Loid,Hash)	\
  mom_add_predefined(MOM_PREDEF(Nam));
#include "_mom_predef.h"

  int cnt = 0;
#define MOM_HAS_PREDEFINED(Nam,Idstr,Hid,Loid,Hash) do {	\
    MOM_ASSERTPRINTF(MOM_PREDEF(Nam)->mo_ob_hid == Hid		\
		     && MOM_PREDEF(Nam)->mo_ob_loid == Loid,	\
		     "bad predef " #Nam);			\
    mo_register_named(MOM_PREDEF(Nam),#Nam);			\
    cnt++;							\
} while(0);
#include "_mom_predef.h"

  MOM_INFORMPRINTF ("initialized %d predefined", cnt);
}                               /* end mom_init_objects */


// end of file object.c
