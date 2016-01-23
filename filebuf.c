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

static void
file_finalize_mom (void *mfil, void *data)
{
  if (!mfil || mfil == MOM_EMPTY_SLOT)
    return;
  assert (!data);
  struct mom_filebuffer_st *mf = (struct mom_filebuffer_st *) mfil;
  if (mf->va_itype != MOMITY_FILEBUFFER && mf->va_itype != MOMITY_FILE)
    return;
  FILE *f = atomic_exchange (&mf->mom_filp, NULL);
  if (f)
    {
      fclose (f);
      if (mf->va_itype == MOMITY_FILEBUFFER)
        {
          free (mf->mom_filbuf), mf->mom_filbuf = NULL;
          mf->mom_filbufsiz = 0;
        }
    }
}                               /* end of file_finalize_mom */

struct mom_filebuffer_st *
mom_make_filebuffer (void)
{
  struct mom_filebuffer_st *mb =
    mom_gc_alloc (sizeof (struct mom_filebuffer_st));
  mb->va_itype = MOMITY_FILEBUFFER;
  mb->mom_filbuf = NULL;
  mb->mom_filbufsiz = 0;
  mb->mom_filp = NULL;
  FILE *f = open_memstream (&mb->mom_filbuf, &mb->mom_filbufsiz);
  if (!f)
    MOM_FATAPRINTF ("failed to open_memstream %m");
  atomic_init (&mb->mom_filp, f);
  GC_REGISTER_FINALIZER_IGNORE_SELF (mb, file_finalize_mom, NULL, NULL, NULL);
  return mb;
}                               /* end of mom_make_filebuffer */

void
mom_file_close (void *mfil)
{
  if (!mfil || mfil == MOM_EMPTY_SLOT)
    return;
  struct mom_filebuffer_st *mf = (struct mom_filebuffer_st *) mfil;
  if (mf->va_itype != MOMITY_FILEBUFFER && mf->va_itype != MOMITY_FILE)
    return;
  FILE *f = atomic_exchange (&mf->mom_filp, NULL);
  if (f)
    {
      fclose (f);
      if (mf->va_itype == MOMITY_FILEBUFFER)
        {
          free (mf->mom_filbuf), mf->mom_filbuf = NULL;
          mf->mom_filbufsiz = 0;
        }
    }
}                               /* end mom_file_close */

void
mom_file_printf (void *mfil, const char *fmt, ...)
{
  FILE *f = mom_file (mfil);
  if (!f)
    return;
  va_list args;
  va_start (args, fmt);
  vfprintf (f, fmt, args);
  va_end (args);
}                               /* end of mom_file_printf */


const struct mom_boxstring_st *
mom_filebuffer_boxstring (struct mom_filebuffer_st *mf, bool closeit)
{
  const struct mom_boxstring_st *bs = NULL;
  if (!mf || mf == MOM_EMPTY_SLOT || mf->va_itype != MOMITY_FILEBUFFER)
    return NULL;
  FILE *f = (closeit == MOM_FILEBUFFER_KEEPOPEN)
    ? atomic_load (&mf->mom_filp) : atomic_exchange (&mf->mom_filp, NULL);
  if (!f)
    return NULL;
  fflush (f);
  long ln = ftell (f);
  if (ln < 0)
    return NULL;
  const char *b = mf->mom_filbuf;
  if (!b)
    return NULL;
  bs = mom_boxstring_make_len (b, ln);
  if (closeit != MOM_FILEBUFFER_KEEPOPEN)
    {
      fclose (f);
      free (mf->mom_filbuf), mf->mom_filbuf = NULL;
      mf->mom_filbufsiz = 0;
    }
  return bs;
}                               /* end of mom_filebuffer_boxstring */


const char *
mom_filebuffer_strdup (struct mom_filebuffer_st *mf, bool closeit)
{
  const char *bs = NULL;
  if (!mf || mf == MOM_EMPTY_SLOT || mf->va_itype != MOMITY_FILEBUFFER)
    return NULL;
  FILE *f = (closeit == MOM_FILEBUFFER_KEEPOPEN)
    ? atomic_load (&mf->mom_filp) : atomic_exchange (&mf->mom_filp, NULL);
  if (!f)
    return NULL;
  fflush (f);
  long ln = ftell (f);
  if (ln < 0)
    return NULL;
  const char *b = mf->mom_filbuf;
  if (!b)
    return NULL;
  {
    char *p = mom_gc_alloc_atomic (ln + 1);
    memcpy (p, b, ln);
    bs = p;
  }
  if (closeit != MOM_FILEBUFFER_KEEPOPEN)
    {
      fclose (f);
      free (mf->mom_filbuf), mf->mom_filbuf = NULL;
      mf->mom_filbufsiz = 0;
    }
  return bs;
}                               /* end of mom_filebuffer_strdup */
