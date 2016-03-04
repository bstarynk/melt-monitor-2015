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

struct mom_file_st *
mom_make_file (FILE *f)
{
  if (!f)
    return NULL;
  struct mom_file_st *mb = mom_gc_alloc (sizeof (struct mom_file_st));
  mb->va_itype = MOMITY_FILE;
  mb->mom_filp = NULL;
  atomic_init (&mb->mom_filp, f);
  GC_REGISTER_FINALIZER_IGNORE_SELF (mb, file_finalize_mom, NULL, NULL, NULL);
  return mb;
}                               /* end of mom_make_file */

void
mom_file_indent (void *mfil)
{
  if (!mfil || mfil == MOM_EMPTY_SLOT)
    return;
  struct mom_filebuffer_st *mf = (struct mom_filebuffer_st *) mfil;
  if (mf->va_itype != MOMITY_FILEBUFFER && mf->va_itype != MOMITY_FILE)
    return;
  mf->mom_findent++;
}                               /* end of mom_file_indent */

void
mom_file_outdent (void *mfil)
{
  if (!mfil || mfil == MOM_EMPTY_SLOT)
    return;
  struct mom_filebuffer_st *mf = (struct mom_filebuffer_st *) mfil;
  if (mf->va_itype != MOMITY_FILEBUFFER && mf->va_itype != MOMITY_FILE)
    return;
  mf->mom_findent--;
}                               /* end of mom_file_outdent */

void
mom_file_set_indentation (void *mfil, int ind)
{
  if (!mfil || mfil == MOM_EMPTY_SLOT)
    return;
  struct mom_filebuffer_st *mf = (struct mom_filebuffer_st *) mfil;
  if (mf->va_itype != MOMITY_FILEBUFFER && mf->va_itype != MOMITY_FILE)
    return;
  mf->mom_findent = ind;
}                               /* end of mom_file_set_indentation */

void
mom_file_newline (void *mfil)
{
  if (!mfil || mfil == MOM_EMPTY_SLOT)
    return;
  struct mom_filebuffer_st *mf = (struct mom_filebuffer_st *) mfil;
  if (mf->va_itype != MOMITY_FILEBUFFER && mf->va_itype != MOMITY_FILE)
    return;
  FILE *f = mom_file (mf);
  if (!f)
    return;
  int ind = mf->mom_findent;
  fputc ('\n', f);
  mf->mom_flastnloff = ftell (f);
  for (int i = ind % 16; i >= 0; i--)
    fputc (' ', f);
}                               /* end of mom_file_newline */

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
      int ind = mf->mom_findent;
      if (ind > 0 && mf->mom_flastnloff < ftell (f))
        fputc ('\n', f);
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
  if (!f || !fmt)
    return;
  struct mom_file_st *mf = (struct mom_file_st *) mfil;
  va_list args;
  size_t flen = strlen (fmt);
  va_start (args, fmt);
  vfprintf (f, fmt, args);
  va_end (args);
  if (flen > 0 && fmt[flen - 1] == '\n')
    {
      int ind = mf->mom_findent;
      mf->mom_flastnloff = ftell (f);
      for (int i = ind % 16; i >= 0; i--)
        fputc (' ', f);
    }
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


void
mom_puts_filebuffer (FILE *outf, struct mom_filebuffer_st *fb, bool closeit)
{
  if (!fb || fb == MOM_EMPTY_SLOT || fb->va_itype != MOMITY_FILEBUFFER)
    return;
  if (!outf || outf == MOM_EMPTY_SLOT)
    return;
  FILE *f = (closeit == MOM_FILEBUFFER_KEEPOPEN)
    ? atomic_load (&fb->mom_filp) : atomic_exchange (&fb->mom_filp, NULL);
  if (!f)
    return;
  fflush (f);
  long ln = ftell (f);
  if (ln < 0)
    return;
  const char *b = fb->mom_filbuf;
  if (!b)
    return;
  fwrite (b, ln, 1, outf);
  if (closeit != MOM_FILEBUFFER_KEEPOPEN)
    {
      fclose (f);
      free (fb->mom_filbuf), fb->mom_filbuf = NULL;
      fb->mom_filbufsiz = 0;
    }
}                               /* end of mom_puts_filebuffer */


void
mom_dumpemit_filebuffer_payload (struct mom_dumper_st *du,
                                 struct mom_filebuffer_st *fb)
{
  if (!fb || fb == MOM_EMPTY_SLOT || fb->va_itype != MOMITY_FILEBUFFER)
    return;
  assert (du && du->va_itype == MOMITY_DUMPER);
  assert (du->du_state == MOMDUMP_EMIT);
  FILE *femit = du->du_emitfile;
  const struct mom_boxstring_st *bs =
    mom_filebuffer_boxstring (fb, MOM_FILEBUFFER_KEEPOPEN);
  if (!bs)
    return;
  const char *pc = bs->cstr;
  unsigned len = mom_raw_size (bs);
  assert (pc[len] == (char) 0);
  const char *end = pc + len;
  fputs ("(\n", femit);
  while (pc < end)
    {
      const char *npc = (pc + 20 < end) ? strchr (pc + 15, '\n') : end;
      if (!npc)
        npc = end;
      fputs ("\"", femit);
      mom_output_utf8_encoded (femit, pc, npc - pc);
      fputs ("\"\n", femit);
      pc = npc;
    };
  fputs (")payload_filebuffer\n", femit);
}                               /* end of mom_dumpemit_filebuffer_payload */

extern mom_loader_paren_sig_t momf_ldp_payload_filebuffer;



const char momsig_ldp_payload_filebuffer[] = "signature_loader_paren";
void
momf_ldp_payload_filebuffer (struct mom_item_st *itm,
                             struct mom_loader_st *ld,
                             struct mom_statelem_st *elemarr,
                             unsigned elemsize)
{
  MOM_DEBUGPRINTF (load,
                   "momf_ldp_payload_filebuffer start itm=%s elemsize=%u",
                   mom_item_cstring (itm), elemsize);
  assert (itm && itm->va_itype == MOMITY_ITEM);
  assert (ld && ld->va_itype == MOMITY_LOADER);
  struct mom_filebuffer_st *fb = mom_make_filebuffer ();
  assert (fb);
  for (unsigned ix = 0; ix < elemsize; ix++)
    {
      const char *s = mom_ldstate_cstring (elemarr[ix]);
      if (!s)
        continue;
      mom_file_puts (fb, s);
    }
}                               /* end momf_ldp_payload_filebuffer */
