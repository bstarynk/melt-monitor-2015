// file state.c - managing state

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

#define LOADER_MAGIC_MOM 0x1f3fd30f     /*524276495 */
struct
{
  unsigned ld_stacksize;
  unsigned ld_stacktop;
  struct mom_statelem_st *ld_stackarr;
  struct mom_hashset_st *ld_hsetitems;
  unsigned ld_magic;            /* always LOADER_MAGIC_MOM */
  int ld_prevmark;
  FILE *ld_file;
  const char *ld_path;
} mom_loader;


static void
initialize_load_state_mom (const char *statepath)
{
  struct stat st;
  memset (&st, 0, sizeof (mom_loader));
  memset (&mom_loader, 0, sizeof (mom_loader));
  FILE *f = fopen (statepath, "r");
  if (!f)
    MOM_FATAPRINTF ("failed to open initial state file %s : %m", statepath);
  if (fstat (fileno (f), &st))
    MOM_FATAPRINTF ("failed to stat file %s : %m", statepath);
  long fisiz = st.st_size;
  {
    unsigned sizstack = ((10 + (int) sqrt (0.2 * fisiz)) | 0x1f) + 1;
    mom_loader.ld_stackarr =
      mom_gc_alloc (sizeof (struct mom_statelem_st) * sizstack);
    mom_loader.ld_stacksize = sizstack;
  }
  {
    unsigned sizhset = ((30 + (int) sqrt (0.3 * fisiz)) | 0x1f) + 1;
    mom_loader.ld_hsetitems = mom_hashset_reserve (NULL, sizhset);
  }
  mom_loader.ld_magic = LOADER_MAGIC_MOM;
  mom_loader.ld_file = f;
  mom_loader.ld_path = GC_STRDUP (statepath);
}

void
mom_load_state (const char *statepath)
{
  if (!statepath || !statepath[0])
    {
      MOM_WARNPRINTF ("empty start path to load");
      return;
    }
  initialize_load_state_mom (statepath);
}
