// file filebuf.c

/**   Copyright (C)  2016  Basile Starynkevitch and later the FSF
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


struct mom_filebuffer_st *
mom_make_filebuffer (void)
{
  struct mom_filebuffer_st *mb =
    mom_gc_alloc (sizeof (struct mom_filebuffer_st));
  mb->va_itype = MOMITY_FILEBUFFER;
  mb->mom_filebuffer = NULL;
  mb->mom_filebufsize = 0;
  mb->mom_file = open_memstream (&mb->mom_filebuffer, &mb->mom_filebufsize);
  if (!mb->mom_file)
    MOM_FATAPRINTF ("failed to open_memstream %m");
  return mb;
}                               /* end of mom_make_filebuffer */
