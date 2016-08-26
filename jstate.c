// file jstate.c - persistency support in JSON & Sqlite

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

/*** IMPORTANT NOTICE: the list of tables created by function
 * mo_create_tables_for_dump should be kept in sync with the
 * monimelt-dump-state.sh script (look for lines with ".mode insert"
 * inside that script)
 ***/

typedef struct mo_dumper_st mo_dumper_ty;
typedef struct mo_loader_st mo_loader_ty;
void mo_create_tables_for_dump (mo_dumper_ty *);
#define MOM_LOADER_MAGIC  0x179128bd
struct mo_loader_st
{
  unsigned mo_ld_magic;         /* always MOM_LOADER_MAGIC */
  double mo_ld_startelapsedtime;
  double mo_ld_startcputime;
  unsigned mo_ld_nbitems;
};

#define MOM_DUMPER_MAGIC  0x372bb699
struct mo_dumper_st
{
  unsigned mo_du_magic;         /* always MOM_DUMPER_MAGIC */
};


void
mom_load_state (void)
{
}                               /* end mom_load_state */


void
mom_dump_state (void)
{
}                               /* end mom_dump_state */
