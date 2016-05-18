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

enum miniedit_sessoff_en
{
  miss_contitm,
  miss__last
};

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
  miedgsty_stytup,
  miedgsty__last
};


void
mom_mini_genstyle (struct mom_item_st *wexitm,
                   struct mom_webexch_st *wexch, struct mom_item_st *styitm)
{
  MOM_FATAPRINTF
    ("unimplemented mom_mini_genstyle wexitm=%s styitm=%s",
     mom_item_cstring (wexitm), mom_item_cstring (styitm));
#warning mom_mini_genstyle unimplemented
}                               /* end mom_mini_genstyle */


extern mom_webhandler_sig_t momf_miniedit_genstyle;
const char momsig_miniedit_genstyle[] = "signature_webhandler";
void
momf_miniedit_genstyle (struct mom_item_st *wexitm,
                        struct mom_webexch_st *wexch,
                        const struct mom_boxnode_st *wclos)
{
  assert (wexch && wexch->va_itype == MOMITY_WEBEXCH);
  unsigned long reqcnt = wexch->webx_count;
  MOM_DEBUGPRINTF (web,
                   "momf_miniedit_genstyle start webreq#%ld wexitm=%s wclos=%s",
                   reqcnt, mom_item_cstring (wexitm),
                   mom_value_cstring ((const void *) wclos));
  if (!wclos || wclos->va_itype != MOMITY_NODE
      || mom_size (wclos) < miedgsty__last)
    MOM_FATAPRINTF ("minedit_genstyle  webreq#%ld wexitm %s has bad wclos %s",
                    reqcnt, mom_item_cstring (wexitm),
                    mom_value_cstring ((const void *) wclos));
  struct mom_item_st *sessitm = wexch->webx_sessitm;
  MOM_DEBUGPRINTF (web,
                   "miniedit_genstyle sessitm=%s wexch #%ld meth %s fupath %s path %s",
                   mom_item_cstring (sessitm),
                   wexch->webx_count,
                   mom_webmethod_name (wexch->webx_meth),
                   onion_request_get_fullpath (wexch->webx_requ),
                   onion_request_get_path (wexch->webx_requ));
  const struct mom_boxtuple_st *tupsty =
    mom_dyncast_tuple (wclos->nod_sons[miedgsty_stytup]);
  if (!tupsty)
    MOM_FATAPRINTF
      ("minedit_genstyle wexitm %s has bad tuple of styles@#%d in wclos %s",
       mom_item_cstring (wexitm), (int) miedgsty_stytup,
       mom_value_cstring ((const struct mom_hashedvalue_st *) wclos));
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
      mom_mini_genstyle (wexitm, wexch, curstyitm);
    }
  MOM_WEXCH_PRINTF (wexch, "\n/* generated %d styles */\n", nbsty);
  mom_wexch_flush (wexch);
  MOM_DEBUGPRINTF (web,
                   "miniedit_genstyle  wexch #%ld outputting %ld bytes:\n%.*s\n",
                   wexch->webx_count, ftell (wexch->webx_outfil),
                   (int) ftell (wexch->webx_outfil), wexch->webx_outbuf);
  mom_unsync_wexch_reply (wexitm, wexch, HTTP_OK, "text/css");
end:;
}                               /* end of momf_miniedit_genstyle */



#define MAXLEN_JAVASCRIPT_MOM (2<<20)
#define MAXDEPTH_JAVASCRIPT_MOM 384
enum minieditgenscript_closoff_en
{
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




extern mom_webhandler_sig_t momf_miniedit_genscript;
const char momsig_miniedit_genscript[] = "signature_webhandler";
void
momf_miniedit_genscript (struct mom_item_st *wexitm,
                         struct mom_webexch_st *wexch,
                         const struct mom_boxnode_st *wclos)
{
  assert (wexch && wexch->va_itype == MOMITY_WEBEXCH);
  struct mom_item_st *sessitm = wexch->webx_sessitm;
  MOM_DEBUGPRINTF (web,
                   "momf_miniedit_genscript start wexitm=%s wclos=%s",
                   mom_item_cstring (wexitm),
                   mom_value_cstring ((const void *) wclos));
  if (!wclos || wclos->va_itype != MOMITY_NODE
      || mom_size (wclos) < miedgscr__last)
    MOM_FATAPRINTF ("minedit_genscript wexitm %s has bad wclos %s",
                    mom_item_cstring (wexitm),
                    mom_value_cstring ((const void *) wclos));
  const struct mom_boxnode_st *scrnod =
    mom_dyncast_node (wclos->nod_sons[miedgscr_scrnod]);
  MOM_DEBUGPRINTF (web, "momf_miniedit_genscript wexitm %s scrnod %s",
                   mom_item_cstring (wexitm),
                   mom_value_cstring ((void *) scrnod));
  if (!scrnod || scrnod->va_itype != MOMITY_NODE)
    MOM_FATAPRINTF ("minedit_genscript wexitm %s has bad scrnod %s",
                    mom_item_cstring (wexitm),
                    mom_value_cstring ((void *) scrnod));
  MOM_DEBUGPRINTF (web,
                   "miniedit_genscript sessitm=%s wexch #%ld meth %s fupath %s path %s",
                   mom_item_cstring (sessitm),
                   wexch->webx_count,
                   mom_webmethod_name (wexch->webx_meth),
                   onion_request_get_fullpath (wexch->webx_requ),
                   onion_request_get_path (wexch->webx_requ));
  struct mom_minedjs_st mjst = { };
  memset (&mjst, 0, sizeof (mjst));
  mjst.miejs_wexitm = wexitm;
  mjst.miejs_wexch = wexch;
  mjst.miejs_magic = MOM_MINEDJS_MAGIC;
  int errlin = setjmp (mjst.miejs_jb);
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
              ("miniedit_genscript wexitm %s webrequest #%ld failed to reopen outfile (%m)",
               mom_item_cstring (wexitm), wexch->webx_count);
        };
      MOM_WEXCH_PRINTF (wexch, "//failure from %s:%d\n",
                        mjst.miejs_errfile ? : "??", errlin);
      mom_unsync_wexch_reply (wexitm, wexch, HTTP_INTERNAL_ERROR,
                              "application/javascript");
      goto end;
    };
  assert (errlin == 0);
  MOM_DEBUGPRINTF (run,
                   "miniedit_genscript wexitm %s webrequest #%ld"
                   "\n... emitting javascript for srcnod=%s\n",
                   mom_item_cstring (wexitm),
                   wexch->webx_count, mom_value_cstring ((void *) scrnod));
  mom_output_gplv3_notice (wexch->webx_outfil, "//! ", "",
                           "miniedit_genscript");
  mom_emit_javascript (&mjst, scrnod, 0);
  MOM_DEBUGPRINTF (run,
                   "miniedit_genscript wexitm %s webrequest #%ld end emitting javascript",
                   mom_item_cstring (wexitm), wexch->webx_count);
  MOM_WEXCH_PRINTF (wexch,
                    "\n// end of generated javascript for wexitm %s\n",
                    mom_item_cstring (wexitm));
  mom_wexch_flush (wexch);
  MOM_DEBUGPRINTF (web,
                   "miniedit_genscript wexch #%ld outputting %ld bytes:\n%.*s\n",
                   wexch->webx_count, ftell (wexch->webx_outfil),
                   (int) ftell (wexch->webx_outfil), wexch->webx_outbuf);
  mom_unsync_wexch_reply (wexitm, wexch, HTTP_OK, "application/javascript");
  MOM_DEBUGPRINTF (run,
                   "miniedit_genscript replied wexitm %s",
                   mom_item_cstring (wexitm));
end:;
}                               /* end of momf_miniedit_genscript */


MOM_PRIVATE void
miniedit_outputedit_mom (struct mom_filebuffer_st *fbu,
                         struct mom_item_st *wexitm,
                         const struct mom_boxnode_st *wclos,
                         struct mom_item_st *contitm)
{
  assert (fbu && fbu->va_itype == MOMITY_FILEBUFFER);
  assert (wexitm && wexitm->va_itype == MOMITY_ITEM);
  assert (wclos && wclos->va_itype == MOMITY_NODE);
  MOM_DEBUGPRINTF (web, "miniedit_outputedit_mom contitm=%s start",
                   mom_item_cstring (contitm));
  mom_item_lock (contitm);
  struct mom_item_st *mieditm = //
    mom_dyncast_item (mom_unsync_item_get_phys_attr
                      (contitm, MOM_PREDEFITM (miniedit)));
  MOM_DEBUGPRINTF (web, "miniedit_outputedit mieditm=%s",
                   mom_item_cstring (mieditm));
  if (mieditm == NULL)
    MOM_FATAPRINTF ("miniedit_outputedit nil mieditm in contitm=%s",
                    mom_item_cstring (contitm));
  assert (mieditm != NULL && mieditm->va_itype == MOMITY_ITEM);
#define NBMEDIT_MOM 31
#define CASE_MIEDIT_MOM(Nam) momhashpredef_##Nam % NBMEDIT_MOM:	\
	  if (mieditm == MOM_PREDEFITM(Nam)) goto foundcasemedit_##Nam;	\
	  goto defaultcasemiedit; foundcasemedit_##Nam
  switch (mieditm->hva_hash % NBMEDIT_MOM)
    {
    case CASE_MIEDIT_MOM (value):
      mom_file_printf (fbu,
                       "<span class='mom_minieditvalue_cl' id='mom$%s'></span>",
                       mom_item_cstring (contitm));
      break;
    default:
    defaultcasemiedit:
      MOM_WARNPRINTF ("miniedit_outputedit contitm=%s unexpected mieditm=%s",
                      mom_item_cstring (contitm), mom_item_cstring (mieditm));
      break;
#undef CASE_MIEDIT_MOM
#undef NBMEDIT_MOM
    };
end:
  mom_item_unlock (contitm);
}                               /* end of miniedit_outputedit_mom */

enum minieditstartpage_closoff_en
{
  miedstpg_first,
  miedstpg__last
};
extern mom_webhandler_sig_t momf_miniedit_startpage;
const char momsig_miniedit_startpage[] = "signature_webhandler";
void
momf_miniedit_startpage (struct mom_item_st *wexitm,
                         struct mom_webexch_st *wexch,
                         const struct mom_boxnode_st *wclos)
{
  assert (wexch && wexch->va_itype == MOMITY_WEBEXCH);
  struct mom_item_st *sessitm = wexch->webx_sessitm;
  MOM_DEBUGPRINTF (web,
                   "momf_miniedit_startpage start wexitm=%s wclos=%s sessitm=%s",
                   mom_item_cstring (wexitm),
                   mom_value_cstring ((const void *) wclos),
                   mom_item_cstring (sessitm));
  assert (sessitm && sessitm != MOM_EMPTY_SLOT
          && sessitm->va_itype == MOMITY_ITEM);
  struct mom_item_st *econtitm = NULL;
  {
    mom_item_lock (sessitm);
    if (!sessitm->itm_pcomp)
      sessitm->itm_pcomp =
        mom_vectvaldata_reserve (sessitm->itm_pcomp, miss__last);
    econtitm =
      mom_dyncast_item (mom_vectvaldata_nth
                        (sessitm->itm_pcomp, miss_contitm));
    if (!econtitm)
      {
        econtitm = mom_clone_item (MOM_PREDEFITM (miniedit));
        MOM_DEBUGPRINTF (web,
                         "momf_miniedit_startpage new econtitm=%s in sessitm=%s",
                         mom_item_cstring (econtitm),
                         mom_item_cstring (sessitm));
        mom_unsync_item_put_phys_attr (econtitm, MOM_PREDEFITM (miniedit),
                                       MOM_PREDEFITM (value));
        mom_vectvaldata_put_nth (sessitm->itm_pcomp, miss_contitm,
                                 (void *) econtitm);
      }
    mom_item_unlock (sessitm);
  }
  if (!wclos || wclos->va_itype != MOMITY_NODE
      || mom_size (wclos) < miedstpg__last)
    MOM_FATAPRINTF ("minedit_startpage wexitm %s has bad wclos %s",
                    mom_item_cstring (wexitm),
                    mom_value_cstring ((const void *) wclos));
  char timbuf[72];
  memset (timbuf, 0, sizeof (timbuf));
  mom_now_strftime_centi (timbuf, sizeof (timbuf) - 1,
                          "%Y %b %d, %H:%M:%S.__ %Z");
  MOM_WEXCH_PRINTF (wexch,
                    "{\"progstatus\":\"<b>monimelt</b> stamped <i>%s</i>"
                    " gitcommit <tt>%.10s</tt>" " pid <i>%d</i>"
                    " host <tt>%s</tt>" " at <i>%s</i>" " session <tt>%s</tt>"
                    "\",\n", monimelt_timestamp, monimelt_lastgitcommit,
                    (int) getpid (), mom_hostname (), timbuf,
                    mom_item_cstring (sessitm));
  {
    struct mom_filebuffer_st *fbu = mom_make_filebuffer ();
    miniedit_outputedit_mom (fbu, wexitm, wclos, econtitm);
    const char *fdup = mom_filebuffer_strdup (fbu, MOM_FILEBUFFER_CLOSE);
    MOM_DEBUGPRINTF (web, "momf_miniedit_startpage fdup:\n%s\n", fdup);
    mom_wexch_puts (wexch, " \"contenthtml\": \"");
    mom_output_utf8_encoded (wexch->webx_outfil, fdup, -1);
    mom_wexch_puts (wexch, "\",\n");
  }
  MOM_WEXCH_PRINTF (wexch, " \"docontent\": \"mom_content_updated();\",\n");
  MOM_WEXCH_PRINTF (wexch, " \"$done\":\"startpage\"}\n");
  mom_unsync_wexch_reply (wexitm, wexch, HTTP_OK, "application/json");
}                               /* end miniedit_startpage */



enum minieditdumpexit_closoff_en
{
  miedduxi_first,
  miedduxi__last
};
extern mom_webhandler_sig_t momf_miniedit_dumpexit;
const char momsig_miniedit_dumpexit[] = "signature_webhandler";
void
momf_miniedit_dumpexit (struct mom_item_st *wexitm,
                        struct mom_webexch_st *wexch,
                        const struct mom_boxnode_st *wclos)
{
  assert (wexch && wexch->va_itype == MOMITY_WEBEXCH);
  struct mom_item_st *sessitm = wexch->webx_sessitm;
  MOM_DEBUGPRINTF (web,
                   "momf_miniedit_dumpexit start wexitm=%s wclos=%s sessitm=%s",
                   mom_item_cstring (wexitm),
                   mom_value_cstring ((const void *) wclos),
                   mom_item_cstring (sessitm));
  if (!wclos || wclos->va_itype != MOMITY_NODE
      || mom_size (wclos) < miedduxi__last)
    MOM_FATAPRINTF ("minedit_dumpexit wexitm %s has bad wclos %s",
                    mom_item_cstring (wexitm),
                    mom_value_cstring ((const void *) wclos));
  char timbuf[72];
  memset (timbuf, 0, sizeof (timbuf));
  mom_now_strftime_centi (timbuf, sizeof (timbuf) - 1,
                          "%Y %b %d, %H:%M:%S.__ %Z");
  json_t *jreply = json_pack ("{s:f,s:f,s:s}", "elapsedreal",
                              mom_elapsed_real_time (), "processcpu",
                              mom_process_cpu_time (),
                              "now", timbuf);
  // json_dumps will use GC_STRDUP...
  MOM_DEBUGPRINTF (web, "minedit_dumpexit jreply=%s",
                   json_dumps (jreply, JSON_INDENT (1)));
  mom_wexch_puts (wexch, json_dumps (jreply, JSON_INDENT (1)));
  mom_unsync_wexch_reply (wexitm, wexch, HTTP_OK, "application/json");
  {
    struct mom_websession_st *wses = NULL;
    mom_item_lock (sessitm);
    if (mom_itype (sessitm->itm_payload) == MOMITY_WEBSESSION)
      wses = (void *) sessitm->itm_payload;
    mom_item_unlock (sessitm);
    if (wses != NULL && wses->wbss_websock)
      {
        MOM_DEBUGPRINTF (web,
                         "minedit_dumpexit closing websocket of session %s",
                         mom_item_cstring (sessitm));
        onion_websocket_close (wses->wbss_websock);
      }
  }
  MOM_DEBUGPRINTF (web, "minedit_dumpexit before mom_stop_and_dump");
  mom_stop_and_dump ();
  MOM_INFORMPRINTF ("miniedit dumpexit at %s", timbuf);
  MOM_DEBUGPRINTF (web, "minedit_dumpexit done");
}                               /* end miniedit_dumpexit */



extern mom_webhandler_sig_t momf_miniedit_websocket;
const char momsig_miniedit_websocket[] = "signature_webhandler";
void
momf_miniedit_websocket (struct mom_item_st *wexitm,
                         struct mom_webexch_st *wexch,
                         const struct mom_boxnode_st *wclos)
{
  assert (wexch && wexch->va_itype == MOMITY_WEBEXCH);
  struct mom_item_st *sessitm = wexch->webx_sessitm;
  MOM_DEBUGPRINTF (web,
                   "miniedit_websocket start wexitm=%s sessitm=%s wclos=%s",
                   mom_item_cstring (wexitm),
                   mom_item_cstring (sessitm),
                   mom_value_cstring ((const void *) wclos));
  struct mom_websession_st *wses = (void *) sessitm->itm_payload;
  if (wses->va_itype != MOMITY_WEBSESSION)
    MOM_FATAPRINTF ("bad sessitm %s for wexitm %s",
                    mom_item_cstring (sessitm), mom_item_cstring (wexitm));
  if (wses->wbss_websock)
    {
      MOM_WARNPRINTF
        ("miniedit_websocket  wexitm=%s sessitm=%s already have a websocket",
         mom_item_cstring (wexitm), mom_item_cstring (sessitm));
      return;
    }
  wses->wbss_websock =
    onion_websocket_new (wexch->webx_requ, wexch->webx_resp);
  MOM_WARNPRINTF ("miniedit_websocket wexitm=%s incomplete");
#warning miniedit_websocket incomplete
}                               /* end of momf_miniedit_websocket */

extern mom_webhandler_sig_t momf_miniedit_keypressajax;
const char momsig_miniedit_keypressajax[] = "signature_webhandler";
void
momf_miniedit_keypressajax (struct mom_item_st *wexitm,
                            struct mom_webexch_st *wexch,
                            const struct mom_boxnode_st *wclos)
{
  assert (wexch && wexch->va_itype == MOMITY_WEBEXCH);
  struct mom_item_st *sessitm = wexch->webx_sessitm;
  MOM_DEBUGPRINTF (web,
                   "miniedit_keypressajax start wexitm=%s wclos=%s sessitm=%s",
                   mom_item_cstring (wexitm),
                   mom_value_cstring ((const void *) wclos),
                   mom_item_cstring (sessitm));
  const char *midstr = onion_request_get_post (wexch->webx_requ, "mom_id");
  const char *whichstr = onion_request_get_post (wexch->webx_requ, "which");
  const char *offsetstr = onion_request_get_post (wexch->webx_requ, "offset");
  const char *keystr = onion_request_get_post (wexch->webx_requ, "key");
  const char *altstr = onion_request_get_post (wexch->webx_requ, "alt");
  const char *ctrlstr = onion_request_get_post (wexch->webx_requ, "ctrl");
  const char *metastr = onion_request_get_post (wexch->webx_requ, "meta");
  const char *timestampstr =
    onion_request_get_post (wexch->webx_requ, "timestamp");
  struct mom_item_st *miditm = mom_find_item_by_string (midstr);
  MOM_DEBUGPRINTF (web,
                   "miniedit_keypressajax midstr=%s whichstr=%s offsetstr=%s"
                   " keystr=%s altstr=%s ctrlstr=%s metastr=%s timestampstr=%s miditm=%s",
                   midstr, whichstr, offsetstr, keystr,
                   altstr, ctrlstr, metastr, timestampstr,
                   mom_item_cstring (miditm));
  bool isalt = altstr && (altstr[0] == 't' || altstr[0] == '1');
  bool isctrl = ctrlstr && (ctrlstr[0] == 't' || ctrlstr[0] == '1');
  bool ismeta = metastr && (metastr[0] == 't' || metastr[0] == '1');
  long timestampl = timestampstr ? atol (timestampstr) : -1;
  if (!miditm)
    {
      MOM_WARNPRINTF
        ("miniedit_keypressajax req#%ld bad midstr=%s sessitm=%s",
         wexch->webx_count, midstr, mom_item_cstring (sessitm));
      MOM_WEXCH_PRINTF (wexch, "bad midstr %s", midstr);
      mom_unsync_wexch_reply (wexitm, wexch, HTTP_NOT_FOUND, "text/plain");
      goto end;
    }
  int whichint = atoi (whichstr);
  int offsetint = atoi (offsetstr);
  mom_item_lock (miditm);
  struct mom_item_st *mieditm = //
    mom_dyncast_item (mom_unsync_item_get_phys_attr
                      (miditm, MOM_PREDEFITM (miniedit)));
  MOM_DEBUGPRINTF (web,
                   "miniedit_keypressajax req#%ld mieditm=%s whichint=%d offsetint=%d timestampl=%ld",
                   wexch->webx_count, mom_item_cstring (mieditm), whichint,
                   offsetint, timestampl);
  if (!mieditm)
    {
      MOM_WARNPRINTF
        ("miniedit_keypressajax req#%ld  miditm=%s without miniedit sessitm=%s",
         wexch->webx_count, mom_item_cstring (miditm),
         mom_item_cstring (sessitm));
      MOM_WEXCH_PRINTF (wexch, "bad miditm %s", mom_item_cstring (miditm));
      mom_unsync_wexch_reply (wexitm, wexch, HTTP_NOT_FOUND, "text/plain");
      goto end;
    }
#define NBMEDIT_MOM 31
#define CASE_MIEDIT_MOM(Nam) momhashpredef_##Nam % NBMEDIT_MOM:	\
	  if (mieditm == MOM_PREDEFITM(Nam)) goto foundcasemedit_##Nam;	\
	  goto defaultcasemiedit; foundcasemedit_##Nam
  switch (mieditm->hva_hash % NBMEDIT_MOM)
    {
    case CASE_MIEDIT_MOM (value):
      MOM_DEBUGPRINTF (web,
                       "miniedit_keypressajax value miditm=%s key=%s which#%d",
                       mom_item_cstring (miditm), keystr, whichint);
      if (keystr && !isctrl && !ismeta && !isalt && keystr[0] && !keystr[1]
          && isalpha (keystr[0]) && keystr[0] == (char) whichint)
        {
          struct mom_item_st *keyitm = mom_find_item_by_string (keystr);
          MOM_DEBUGPRINTF (web,
                           "miniedit_keypressajax value miditm=%s good key=%s keyitm=%s",
                           mom_item_cstring (miditm), keystr,
                           mom_item_cstring (keyitm));
          void *valv = NULL;
          const char *valcss = NULL;
          if (keyitm)
            {
              mom_unsync_item_put_phys_attr (miditm, MOM_PREDEFITM (miniedit),
                                             MOM_PREDEFITM (item));
              valv = keyitm;
              mom_unsync_item_put_phys_attr (miditm, MOM_PREDEFITM (item),
                                             valv);
              valcss = "mom_minieditname_cl";
            }
          else
            {
              mom_unsync_item_put_phys_attr (miditm, MOM_PREDEFITM (miniedit),
                                             (void *) MOM_PREDEFITM (word));
              valv = mom_boxstring_make (keystr);
              mom_unsync_item_put_phys_attr (miditm, MOM_PREDEFITM (word),
                                             valv);
              valcss = "mom_minieditword_cl";
            }
          MOM_DEBUGPRINTF (web,
                           "miniedit_keypressajax value miditm=%s valv=%s valcss=%s",
                           mom_item_cstring (miditm),
                           mom_value_cstring (valv), valcss);
          MOM_WEXCH_PRINTF (wexch, "{\"replaceid\": \"%s\",\n",
                            mom_item_cstring (miditm));
          MOM_WEXCH_PRINTF (wexch, " \"replacecss\": \"%s\",\n", valcss);
          MOM_WEXCH_PRINTF (wexch, " \"replacehtml\": \"%s\" }\n", keystr);
          mom_unsync_wexch_reply (wexitm, wexch, HTTP_OK, "application/json");
          goto end;
        }
      MOM_WARNPRINTF ("miniedit_keypressajax req#%ld incomplete",
                      wexch->webx_count);
      break;
    default:
    defaultcasemiedit:
      MOM_WARNPRINTF
        ("miniedit_keypressajax req#%ld bad miditm=%s sessitm=%s has unexpected mieditm=%s",
         wexch->webx_count, mom_item_cstring (miditm),
         mom_item_cstring (sessitm), mom_item_cstring (mieditm));
      MOM_WEXCH_PRINTF (wexch, "miditm %s with strange mieditm %s",
                        mom_item_cstring (miditm),
                        mom_item_cstring (mieditm));
      mom_unsync_wexch_reply (wexitm, wexch, HTTP_NOT_FOUND, "text/plain");
      goto end;
    }
#undef CASE_MIEDIT_MOM
#undef NBMEDIT_MOM
end:
  if (miditm)
    mom_item_unlock (miditm);

}                               /* end of momf_miniedit_keypressajax */
