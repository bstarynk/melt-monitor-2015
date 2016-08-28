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

bool
mom_valid_name (const char *nam)
{
  if (!nam || nam == MOM_EMPTY_SLOT)
    return false;
  if (!isalpha (nam[0]))
    return false;
  for (const char *p = nam + 1; *p; p++)
    {
      if (isalnum (*p))
        continue;
      if (*p == '_')
        {
          if (p[-1] == '_')
            return false;
          if (!isalnum (p[1]))
            return false;
        }
    }
  return true;
}                               // end mom_valid_name

void
mom_init_objects (void)
{
  static bool inited;
  MOM_ASSERTPRINTF (!inited, "should not initialize objects twice");
  if (inited)
    return;
  inited = true;
}                               /* end mom_init_objects */

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
    buf = mom_gc_alloc_scalar (MOM_CSTRIDLEN + 4);
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
  char resbuf[MOM_CSTRIDLEN + 4];
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

mo_objref_t
mo_objref_find_hid_loid (mo_hid_t hid, mo_loid_t loid)
{
  if (hid == 0 && loid == 0)
    return NULL;
  unsigned bn = mo_hi_id_bucketnum (hid);
  MOM_ASSERTPRINTF (bn > 0 && bn < MOM_HID_BUCKETMAX, "bad bd%u", bn);
#warning mo_objref_find_hid_loid incomplete
  MOM_FATAPRINTF ("mo_objref_find_hid_loid incomplete");
}

// end of file object.c
