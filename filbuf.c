// file filbuf.c - file & buffer payloads

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
mo_objref_open_file (mo_objref_t obr, const char *path, const char *mods)
{
  if (!mo_dyncast_objref (obr))
    return false;
  if (!path || path == MOM_EMPTY_SLOT)
    return false;
  if (!mods || mods == MOM_EMPTY_SLOT)
    return false;
  FILE *fil = fopen (path, mods);
  if (!fil)
    {
      MOM_WARNPRINTF ("open_file obr@%p:%s path=%s mods=%s failed",
                      obr, mo_objref_pnamestr (obr), path, mods);
      return false;
    }
  mo_objref_clear_payload (obr);
  obr->mo_ob_paylkind = MOM_PREDEF (payload_file);
  obr->mo_ob_payldata = fil;
  return true;
}                               /* end of mo_objref_open_file */

#define MOM_MIN_BUFSIZ 1024     /* a power of two */
bool
mo_objref_open_buffer (mo_objref_t obr)
{
  if (!mo_dyncast_objref (obr))
    return false;
  mo_bufferpayl_ty *bpy = calloc (1, sizeof (mo_bufferpayl_ty));
  if (MOM_UNLIKELY (bpy == NULL))
    MOM_FATAPRINTF
      ("failed to calloc a buffer payload (%zd bytes) for obr %s",
       sizeof (mo_bufferpayl_ty), mo_objref_pnamestr (obr));
  bpy->mo_buffer_zone = NULL;
  bpy->mo_buffer_size = 0;
  if (MOM_UNLIKELY ((bpy->mo_buffer_memstream   //
                     = open_memstream (&bpy->mo_buffer_zone,
                                       &bpy->mo_buffer_size)) == NULL))
    MOM_FATAPRINTF ("failed to open_memstream a buffer payload for obr %s",
                    mo_objref_pnamestr (obr));
  mo_objref_clear_payload (obr);
  bpy->mo_va_kind = mo_PBUFFER;
  bpy->mo_va_hash = (momrand_genrand_int31 () & 0xfffffff) + 2;
  bpy->mo_buffer_nmagic = MOM_BUFFER_MAGIC;
  obr->mo_ob_paylkind = MOM_PREDEF (payload_buffer);
  obr->mo_ob_payldata = bpy;
  return true;
}                               /* end of mo_objref_open_buffer */

json_t *
mo_dump_json_for_buffer_objref (mo_dumper_ty * du, mo_objref_t obr)
{
  MOM_ASSERTPRINTF (mo_dump_emitting (du), "bad du");
  if (!mo_dyncast_objref (obr))
    return NULL;
  if (obr->mo_ob_paylkind != MOM_PREDEF (payload_buffer))
    return NULL;
  if (!obr->mo_ob_payldata)
    return NULL;
  mo_bufferpayl_ty *bpy = (mo_bufferpayl_ty *) (obr->mo_ob_payldata);
  MOM_ASSERTPRINTF (bpy != MOM_EMPTY_SLOT, "empty bpy for obr %s",
                    mo_objref_pnamestr (obr));
  if (MOM_UNLIKELY (bpy->mo_buffer_nmagic != MOM_BUFFER_MAGIC))
    MOM_FATAPRINTF ("corrupted buffer @%p for obr %s",
                    bpy, mo_objref_pnamestr (obr));
  if (MOM_UNLIKELY (bpy->mo_buffer_memstream == NULL))
    MOM_FATAPRINTF ("invalid closed buffer @%p for obr %s",
                    bpy, mo_objref_pnamestr (obr));
  long cpos = ftell (bpy->mo_buffer_memstream);
  if (cpos < 0)
    MOM_FATAPRINTF ("weird buffer @%p for obr %s",
                    bpy, mo_objref_pnamestr (obr));
  fflush (bpy->mo_buffer_memstream);
  json_t *jarr = json_array ();
  const char *zone = bpy->mo_buffer_zone;
  const char *bend = zone + cpos;
  int chklen = 0;
  gunichar uc = 0;
  const char *chkp = zone;
  for (const char *pc = zone; pc < bend && *pc;
       pc = g_utf8_next_char (pc), uc = 0)
    {
      chklen++;
      uc = g_utf8_get_char (pc);
      if ((chklen > 24 && uc == (gunichar) '\n')
          || (chklen > 40 && (g_unichar_iscntrl (uc)
                              || g_unichar_isspace (uc)))
          || chklen > 72 || pc + 1 == bend)
        {
          json_t *jchk = json_stringn (chkp, pc - chkp);
          json_array_append_new (jarr, jchk);
          chkp = pc;
          chklen = 0;
        }
    }
  return json_pack ("{sosI}", "buffer", jarr, "buflen", (json_int_t) cpos);
}                               /* end of mo_dump_json_for_buffer_objref */


void
mo_objref_set_buffer_from_json (mo_objref_t obr, json_t *js)
{
  if (!mo_dyncast_objref (obr))
    return;
  json_t *jarr = NULL;
  if (!js || !json_is_object (js) || !(jarr = json_object_get (js, "buffer"))
      || !json_is_array (jarr))
    return;
  json_int_t blen = 0;
  json_t *jlen = json_object_get (js, "buflen");
  if (jlen && json_is_integer (jlen))
    blen = json_integer_value (jlen);
  if (MOM_UNLIKELY (!mo_objref_open_buffer (obr)))
    MOM_FATAPRINTF
      ("set_buffer_from_json obr %s failed to open buffer (blen=%ld)",
       mo_objref_pnamestr (obr), (long) blen);
  FILE *fbu = mo_objref_file (obr);
  MOM_ASSERTPRINTF (fbu != NULL,
                    "set_buffer_from_json obr %s failed to get file",
                    mo_objref_pnamestr (obr));
  size_t alen = json_array_size (jarr);
  for (size_t ix = 0; ix < alen; ix++)
    {
      json_t *jchk = json_array_get (jarr, ix);
      if (!jchk || !json_is_string (jchk))
        continue;
      fputs (json_string_value (jchk), fbu);
    }
}                               /* end of mo_objref_set_buffer_from_json */

FILE *
mo_objref_file (mo_objref_t obr)
{
  if (!mo_dyncast_objref (obr))
    return NULL;
  if (obr->mo_ob_paylkind == MOM_PREDEF (payload_file))
    return (FILE *) obr->mo_ob_payldata;
  else if (obr->mo_ob_paylkind == MOM_PREDEF (payload_buffer))
    {
      mo_bufferpayl_ty *bupayl = (mo_bufferpayl_ty *) (obr->mo_ob_payldata);
      if (bupayl)
        {
          MOM_ASSERTPRINTF (bupayl != MOM_EMPTY_SLOT,
                            "empty bupayl for buffer object in in obr@%p=%s",
                            obr, mo_objref_pnamestr (obr));
          MOM_ASSERTPRINTF (bupayl->mo_va_kind ==
                            mo_PBUFFER,
                            "invalid bupayl for buffer object in in obr@%p=%s",
                            obr, mo_objref_pnamestr (obr));
          if (MOM_UNLIKELY (bupayl->mo_buffer_nmagic != MOM_BUFFER_MAGIC))
            MOM_FATAPRINTF ("corrupted buffer object in obr@%p=%s",
                            obr, mo_objref_pnamestr (obr));
          return bupayl->mo_buffer_memstream;
        }
    }
  return NULL;
}                               /* end of mo_objref_file */
