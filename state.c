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
struct mom_loader_st *mom_loader;


static void
initialize_load_state_mom (const char *statepath)
{
  struct stat st;
  memset (&st, 0, sizeof (st));
  mom_loader = mom_gc_alloc (sizeof (struct mom_loader_st));
  mom_loader->va_itype = MOMITY_LOADER;
  FILE *f = fopen (statepath, "r");
  if (!f)
    MOM_FATAPRINTF ("failed to open initial state file %s : %m", statepath);
  if (fstat (fileno (f), &st))
    MOM_FATAPRINTF ("failed to stat file %s : %m", statepath);
  long fisiz = st.st_size;
  {
    unsigned sizstack = ((10 + (int) sqrt (0.2 * fisiz)) | 0x1f) + 1;
    mom_loader->ld_stackarr =
      mom_gc_alloc (sizeof (struct mom_statelem_st) * sizstack);
    mom_put_size (mom_loader, sizstack);
  }
  {
    unsigned sizhset = ((30 + (int) sqrt (0.3 * fisiz)) | 0x1f) + 1;
    mom_loader->ld_hsetitems = mom_hashset_reserve (NULL, sizhset);
  }
  mom_loader->ld_magic = LOADER_MAGIC_MOM;
  mom_loader->ld_file = f;
  mom_loader->ld_path = GC_STRDUP (statepath);
  MOM_DEBUGPRINTF (load, "ld_path=%s fisiz=%ld", mom_loader->ld_path, fisiz);
}


void
first_pass_loader_mom (void)
{
  assert (mom_loader && mom_loader->ld_magic == LOADER_MAGIC_MOM);
  size_t linsiz = 128;
  ssize_t linlen = 0;
  char *linbuf = malloc (linsiz);
  if (!linbuf)
    MOM_FATAPRINTF ("failed to malloc linbuf %zd", linsiz);
  memset (linbuf, 0, linsiz);
  rewind (mom_loader->ld_file);
  int linecount = 0;
  do
    {
      linlen = getline (&linbuf, &linsiz, mom_loader->ld_file);
      if (linlen <= 0)
        break;
      linecount++;
      if (linbuf[0] == '#' || linbuf[0] == '\n')
        continue;
      if (linbuf[0] == '*' && isalpha (linbuf[1]))
        {
          MOM_DEBUGPRINTF (load, "first_pass line#%d %s", linecount, linbuf);
          const char *end = NULL;
          struct mom_item_st *itm =
            mom_make_item_from_string (linbuf + 1, &end);
          MOM_DEBUGPRINTF (load, "first pass line#%d made item %s",
                           linecount, mom_item_cstring (itm));
          mom_loader->ld_hsetitems =
            mom_hashset_insert (mom_loader->ld_hsetitems, itm);
        }
    }
  while (!feof (mom_loader->ld_file));
  free (linbuf);
}                               /* end first_pass_loader_mom */

void
second_pass_loader_mom (void)
{
}                               /* end second_pass_loader_mom */


void
mom_load_state (const char *statepath)
{
  if (!statepath || !statepath[0])
    {
      MOM_WARNPRINTF ("empty start path to load");
      return;
    }
  initialize_load_state_mom (statepath);
  first_pass_loader_mom ();
}
