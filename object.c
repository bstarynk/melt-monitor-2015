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
  {
    _Atomic struct mo_obpayload_t fakepayload
      __attribute__ ((aligned (2 * sizeof (void *))));
    if (!atomic_is_lock_free (&fakepayload))
      MOM_FATAPRINTF
        ("struct mo_obpayload_t of size %zd is not atomic lock free",
         sizeof (fakepayload));
    if (!__atomic_always_lock_free (sizeof (struct mo_obpayload_t), NULL))
      MOM_WARNPRINTF
        ("struct mo_obpayload_t of size %zd is not always atomic lock free",
         sizeof (fakepayload));
  }
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
num80_to_char13_mom (mom_uint128_t num, char *buf)
{
  mom_uint128_t initnum = num;
  for (int ix = 12; ix > 0; ix--)
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
char13_to_num80_mom (const char *buf)
{
  mom_uint128_t num = 0;
  if (buf[0] < '0' || buf[0] > '9')
    return 0;
  for (int ix = 0; ix <= 13; ix++)
    {
      char c = buf[ix];
      const char *p = strchr (ID_DIGITS_MOM, c);
      if (!p)
        return 0;
      num = (num * ID_BASE_MOM + (mom_uint128_t) (p - ID_DIGITS_MOM));
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
  num80_to_char13_mom (wn, s16);
  char resbuf[MOM_CSTRIDLEN + 4];
  memset (resbuf, 0, sizeof (resbuf));
  snprintf (resbuf, sizeof (resbuf), "_%c%c%c%s", d0, c1, c2, s16);
  strcpy (buf, resbuf);
  return buf;
}                               /* end of mo_cstring_from_hi_lo_ids */


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
      loid = ((uint64_t) rm << 32) | ((uint64_t) rl << 32);
    }
  while (loid == 0);
  *phid = hid;
  *ploid = loid;
}                               /* end of mo_get_some_random_hi_lo_ids */

// end of file object.c