// file miniedit.c

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

enum miniedit_closoff_en
{
  mic_wexitm,
  mic__last
};


struct mom_item_st *
mom_miniedit_nth_item (struct mom_item_st *taskitm, int off)
{
  struct mom_item_st *resitm = NULL;
  const struct mom_boxnode_st *nod = NULL;
  if (!taskitm || taskitm == MOM_EMPTY_SLOT
      || taskitm->va_itype != MOMITY_ITEM)
    return NULL;
  unsigned siz = 0;
  mom_item_lock (taskitm);
  nod = mom_dyncast_node (taskitm->itm_payload);
  mom_item_unlock (taskitm);
  if (nod && nod->va_itype == MOMITY_NODE
      && nod->nod_connitm == MOM_PREDEFITM (miniedit)
      && (siz = mom_raw_size (nod)) >= mic__last)
    {
      int realoff = off;
      if (realoff < 0)
        realoff += siz;
      if (realoff >= 0 && realoff < (int) siz)
        resitm = mom_dyncast_item (nod->nod_sons[realoff]);
    };
  MOM_DEBUGPRINTF (web, "miniedit_nth_item taskitm %s off#%d result itm %s",
                   mom_item_cstring (taskitm), off,
                   mom_item_cstring (resitm));
  return resitm;
}                               /* end mom_miniedit_nth_item */


extern mom_tasklet_sig_t momf_miniedit;
const char momsig_miniedit[] = "signature_tasklet";
void
momf_miniedit (struct mom_item_st *tkitm)
{
  struct mom_item_st *wexitm = NULL;
  struct mom_item_st *thistatitm = NULL;
  struct mom_item_st *sessitm = NULL;
  struct mom_item_st *hsetitm = NULL;
  mom_item_lock (tkitm);
  const struct mom_boxnode_st *tknod =
    (struct mom_boxnode_st *) tkitm->itm_payload;
  MOM_DEBUGPRINTF (web,
                   "momf_miniedit start tkitm=%s tknod=%s",
                   mom_item_cstring (tkitm),
                   mom_value_cstring ((const struct
                                       mom_hashedvalue_st *) tknod));
  if (mom_itype (tknod) != MOMITY_NODE || mom_raw_size (tknod) < mic__last)
    {
      /// should not happen
      MOM_WARNPRINTF ("momf_miniedit bad tknod %s",
                      mom_value_cstring ((const struct
                                          mom_hashedvalue_st *) tknod));
      goto end;
    }
  wexitm = mom_miniedit_nth_item (tkitm, mic_wexitm);
  MOM_DEBUGPRINTF (web, "momf_miniedit wexitm=%s", mom_item_cstring (wexitm));
  if (!wexitm)
    goto end;
  mom_item_lock (wexitm);
  struct mom_webexch_st *wexch =
    (struct mom_webexch_st *) wexitm->itm_payload;
  assert (wexch && wexch->va_itype == MOMITY_WEBEXCH);
  sessitm = wexch->webx_sessitm;
  MOM_DEBUGPRINTF (web,
                   "momf_miniedit sessitm=%s wexch #%ld meth %s fupath %s path %s",
                   mom_item_cstring (sessitm),
                   wexch->webx_count,
                   mom_webmethod_name (wexch->webx_meth),
                   onion_request_get_fullpath (wexch->webx_requ),
                   onion_request_get_path (wexch->webx_requ));
  mom_item_lock (sessitm);
end:
  if (sessitm)
    mom_item_unlock (sessitm);
  if (wexitm)
    mom_item_unlock (wexitm);
  mom_item_unlock (tkitm);
}                               /* end of momf_miniedit */





enum minieditgenstyle_closoff_en
{
  miedgsty_wexitm,
  miedgsty_stytup,
  miedgsty__last
};


void
mom_mini_genstyle (struct mom_item_st *tkitm, struct mom_item_st *wexitm,
                   struct mom_webexch_st *wexch, struct mom_item_st *styitm)
{
  MOM_FATAPRINTF
    ("unimplemented mom_mini_genstyle tkitm=%s wexitm=%s styitm=%s",
     mom_item_cstring (tkitm), mom_item_cstring (wexitm),
     mom_item_cstring (styitm));
#warning mom_mini_genstyle unimplemented
}                               /* end mom_mini_genstyle */

extern mom_tasklet_sig_t momf_miniedit_genstyle;
const char momsig_miniedit_genstyle[] = "signature_tasklet";
void
momf_miniedit_genstyle (struct mom_item_st *tkitm)
{
  struct mom_item_st *wexitm = NULL;
  struct mom_item_st *thistatitm = NULL;
  struct mom_item_st *sessitm = NULL;
  mom_item_lock (tkitm);
  const struct mom_boxnode_st *tknod =
    (struct mom_boxnode_st *) tkitm->itm_payload;
  MOM_DEBUGPRINTF (web,
                   "momf_miniedit_genstyle start tkitm=%s tknod=%s",
                   mom_item_cstring (tkitm),
                   mom_value_cstring ((const struct
                                       mom_hashedvalue_st *) tknod));
  if (!tknod || tknod->va_itype != MOMITY_NODE
      || mom_size (tknod) < miedgsty__last)
    MOM_FATAPRINTF ("minedit_genstyle tkitm %s has bad tknod %s",
                    mom_item_cstring (tkitm),
                    mom_value_cstring ((const struct mom_hashedvalue_st *)
                                       tknod));
  wexitm = mom_dyncast_item (tknod->nod_sons[miedgsty_wexitm]);
  mom_item_lock (wexitm);
  MOM_DEBUGPRINTF (web,
                   "minedit_genstyle wexitm=%s", mom_item_cstring (wexitm));
  if (!wexitm)
    goto end;
  mom_item_lock (wexitm);
  struct mom_webexch_st *wexch =
    (struct mom_webexch_st *) wexitm->itm_payload;
  assert (wexch && wexch->va_itype == MOMITY_WEBEXCH);
  sessitm = wexch->webx_sessitm;
  MOM_DEBUGPRINTF (web,
                   "miniedit_genstyle sessitm=%s wexch #%ld meth %s fupath %s path %s",
                   mom_item_cstring (sessitm),
                   wexch->webx_count,
                   mom_webmethod_name (wexch->webx_meth),
                   onion_request_get_fullpath (wexch->webx_requ),
                   onion_request_get_path (wexch->webx_requ));
  mom_item_lock (sessitm);
  struct mom_tuple_st *tupsty =
    mom_dyncast_tuple (tknod->nod_sons[miedgsty_stytup]);
  if (!tupsty)
    MOM_FATAPRINTF
      ("minedit_genstyle tkitm %s has bad tuple of styles@#%d in tknod %s",
       mom_item_cstring (tkitm), (int) miedgsty_stytup,
       mom_value_cstring ((const struct mom_hashedvalue_st *) tknod));
  MOM_DEBUGPRINTF (web, "miniedit_genstyle wexch #%ld tupsty=%s",
                   wexch->webx_count, mom_value_cstring ((void *) tupsty));
  mom_output_gplv3_notice (wexch->webx_outfil, "/*", "*/",
                           "miniedit_genstyle");
  unsigned nbsty = mom_size (tupsty);
  MOM_WEXCH_PRINTF (wexch, "\n/* generating %d styles */\n", nbsty);
  for (unsigned ix = 0; ix < nbsty; ix++)
    {
      struct mom_item_st *curstyitm = mom_boxtuple_nth (tupsty, ix);
      if (!curstyitm)
        continue;
      MOM_WEXCH_PRINTF (wexch, "\n/* style #%d: %s */\n", ix,
                        mom_item_cstring (curstyitm));
      mom_mini_genstyle (tkitm, wexitm, wexch, curstyitm);
    }
  MOM_WEXCH_PRINTF (wexch, "\n/* generated %d styles */\n", nbsty);
  mom_wexch_flush (wexch);
  MOM_DEBUGPRINTF (web,
                   "miniedit_genstyle  wexch #%ld outputting %ld bytes:\n%.*s\n",
                   wexch->webx_count, ftell (wexch->webx_outfil),
                   (int) ftell (wexch->webx_outfil), wexch->webx_outbuf);
  mom_unsync_wexch_reply (wexitm, wexch, HTTP_OK, "text/css");
end:
  if (sessitm)
    mom_item_unlock (sessitm);
  if (wexitm)
    mom_item_unlock (wexitm);
  mom_item_unlock (tkitm);
}                               /* end of momf_miniedit_genstyle */



#define MAXLEN_JAVASCRIPT_MOM (2<<20)
#define MAXDEPTH_JAVASCRIPT_MOM 384
enum minieditgenscript_closoff_en
{
  miedgscr_wexitm,
  miedgscr_scrnod,
  miedgscr__last
};


void
mom_emit_javascript (struct mom_minedjs_st *mj, const void *val, int depth)
{
  assert (mj != NULL && mj->miejs_magic == MOM_MINEDJS_MAGIC);
  struct mom_webexch_st *wexch = mj->miejs_wexch;
  assert (wexch != NULL && wexch->va_itype == MOMITY_WEBEXCH);
  assert (wexch->webx_outfil != NULL);
  if (ftell (wexch->webx_outfil) > MAXLEN_JAVASCRIPT_MOM)
    MOM_MINEDJS_FAILURE (mj, val, MOM_PREDEFITM (output));
  if (depth > MAXDEPTH_JAVASCRIPT_MOM)
    MOM_MINEDJS_FAILURE (mj, val, MOM_PREDEFITM (depth));
  MOM_DEBUGPRINTF (run, "emit_javascript start req#%ld depth#%d val=%s",
                   wexch->webx_count, depth, mom_value_cstring (val));
  unsigned vty = mom_itype (val);
  switch (vty)
    {
    case MOMITY_NONE:
      MOM_WEXCH_PRINTF (wexch, " null");
      break;
    case MOMITY_BOXINT:
    case MOMITY_BOXDOUBLE:
    case MOMITY_BOXSTRING:
      mom_output_value (wexch->webx_outfil, &mj->miejs_lastnl, depth, val);
      break;
    case MOMITY_ITEM:
      {
        mom_wexch_puts (wexch, mom_item_cstring (val));
        break;
      }
#warning mom_emit_javascript incomplete
    }
  MOM_DEBUGPRINTF (run, "emit_javascript end req#%ld depth#%d val=%s",
                   wexch->webx_count, depth, mom_value_cstring (val));
}                               /* end of mom_emit_javascript */


extern mom_tasklet_sig_t momf_miniedit_genscript;
const char momsig_miniedit_genscript[] = "signature_tasklet";
void
momf_miniedit_genscript (struct mom_item_st *tkitm)
{
  struct mom_item_st *wexitm = NULL;
  struct mom_item_st *sessitm = NULL;
  struct mom_boxnode_st *scrnod = NULL;
  mom_item_lock (tkitm);
  const struct mom_boxnode_st *tknod =
    (struct mom_boxnode_st *) tkitm->itm_payload;
  MOM_DEBUGPRINTF (web,
                   "momf_miniedit_genscript start tkitm=%s tknod=%s",
                   mom_item_cstring (tkitm),
                   mom_value_cstring ((const struct
                                       mom_hashedvalue_st *) tknod));
  if (!tknod || tknod->va_itype != MOMITY_NODE
      || mom_size (tknod) < miedgscr__last)
    MOM_FATAPRINTF ("minedit_genscript tkitm %s has bad tknod %s",
                    mom_item_cstring (tkitm),
                    mom_value_cstring ((const struct mom_hashedvalue_st *)
                                       tknod));
  wexitm = mom_dyncast_item (tknod->nod_sons[miedgscr_wexitm]);
  mom_item_lock (wexitm);
  scrnod = mom_dyncast_node (tknod->nod_sons[miedgscr_scrnod]);
  MOM_DEBUGPRINTF (web,
                   "momf_miniedit_genscript wexitm %s scrnod %s",
                   mom_item_cstring (wexitm),
                   mom_value_cstring ((void *) scrnod));
  if (!scrnod || scrnod->va_itype != MOMITY_NODE)
    MOM_FATAPRINTF ("minedit_genscript tkitm %s has bad scrnod %s",
                    mom_item_cstring (tkitm),
                    mom_value_cstring ((void *) scrnod));
  mom_item_lock (wexitm);
  struct mom_webexch_st *wexch =
    (struct mom_webexch_st *) wexitm->itm_payload;
  assert (wexch && wexch->va_itype == MOMITY_WEBEXCH);
  sessitm = wexch->webx_sessitm;
  MOM_DEBUGPRINTF (web,
                   "miniedit_genscript sessitm=%s wexch #%ld meth %s fupath %s path %s",
                   mom_item_cstring (sessitm),
                   wexch->webx_count,
                   mom_webmethod_name (wexch->webx_meth),
                   onion_request_get_fullpath (wexch->webx_requ),
                   onion_request_get_path (wexch->webx_requ));
  mom_item_lock (sessitm);
  struct mom_minedjs_st mjst = { };
  memset (&mjst, 0, sizeof (mjst));
  mjst.miejs_taskitm = tkitm;
  mjst.miejs_wexitm = wexitm;
  mjst.miejs_wexch = wexch;
  mjst.miejs_magic = MOM_MINEDJS_MAGIC;
  int errlin = setjmp (&mjst.miejs_jb);
  if (errlin > 0)
    {
      MOM_WARNPRINTF_AT (mjst.miejs_errfile ? : "??", errlin,
                         "miniedit_genscript failure %s\n"
                         "... with expr %s",
                         mom_value_cstring (mjst.miejs_fail),
                         mom_value_cstring (mjst.miejs_expr));
      if (wexch->webx_outbuf)
        {
          fclose (wexch->webx_outfil), wexch->webx_outfil = NULL;;
          free (wexch->webx_outbuf);
          wexch->webx_outbuf = NULL;
          wexch->webx_outsiz = 0;
          wexch->webx_outfil =
            open_memstream (&wexch->webx_outbuf, &wexch->webx_outsiz);
          if (MOM_UNLIKELY (!wexch->webx_outfil))
            MOM_FATAPRINTF
              ("miniedit_genscript taskitm %s webrequest #%ld failed to reopen outfile (%m)",
               mom_item_cstring (tkitm), wexch->webx_count);
        };
      MOM_WEXCH_PRINTF (wexch, "//failure from %s:%d\n",
                        mjst.miejs_errfile ? : "??", errlin);
      mom_unsync_wexch_reply (wexitm, wexch, HTTP_INTERNAL_ERROR,
                              "application/javascript");
      goto end;
    };
  assert (errlin == 0);
  MOM_DEBUGPRINTF (run,
                   "miniedit_genscript taskitm %s wexitm %s webrequest #%ld"
                   "\n... emitting javascript for srcnod=%s\n",
                   mom_item_cstring (tkitm), mom_item_cstring (wexitm),
                   wexch->webx_count, mom_value_cstring (scrnod));
  mom_output_gplv3_notice (wexch->webx_outfil, "//! ", "",
                           "miniedit_genscript");
  mom_emit_javascript (&mjst, scrnod, 0);
  MOM_DEBUGPRINTF (run,
                   "miniedit_genscript taskitm %s wexitm %s webrequest #%ld end emitting javascript",
                   mom_item_cstring (tkitm), mom_item_cstring (wexitm),
                   wexch->webx_count);
  MOM_WEXCH_PRINTF (wexch,
                    "\n// end of generated javascript for wexitm %s tkitm %s\n",
                    mom_item_cstring (wexitm), mom_item_cstring (tkitm));
  mom_wexch_flush (wexch);
  MOM_DEBUGPRINTF (web,
                   "miniedit_genscript wexch #%ld outputting %ld bytes:\n%.*s\n",
                   wexch->webx_count, ftell (wexch->webx_outfil),
                   (int) ftell (wexch->webx_outfil), wexch->webx_outbuf);
  mom_unsync_wexch_reply (wexitm, wexch, HTTP_OK, "application/javascript");
  MOM_DEBUGPRINTF (run,
                   "miniedit_genscript replied wexitm %s",
                   mom_item_cstring (wexitm));
end:
  if (sessitm)
    mom_item_unlock (sessitm);
  if (wexitm)
    mom_item_unlock (wexitm);
  mom_item_unlock (tkitm);
}                               /* end of momf_miniedit_genscript */
