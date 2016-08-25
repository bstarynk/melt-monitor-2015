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
    _Atomic struct mo_obpayload_t fakepayload;
    if (!atomic_is_lock_free (&fakepayload))
      MOM_FATAPRINTF
        ("struct mo_obpayload_t of size %zd is not atomic lock free",
         sizeof (fakepayload));
    if (!__atomic_always_lock_free (sizeof (struct mo_obpayload_t), NULL))
      MOM_FATAPRINTF
        ("struct mo_obpayload_t of size %zd is not always atomic lock free",
         sizeof (fakepayload));
  }
}                               /* end mom_init_objects */
