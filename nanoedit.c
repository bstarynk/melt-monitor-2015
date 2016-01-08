// file nanoedit.c

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


enum nanoedit_sessfields_en
{
  mes_itmhset,
  mes__last
};


static void
showitem_nanoedit_mom (struct mom_webexch_st *wexch,
                       struct mom_item_st *wexitm,
                       struct mom_item_st *thistatitm,
                       const struct mom_item_st *curitm, bool isval)
{
  struct mom_item_st *hsetitm = NULL;
  struct mom_item_st *sessitm = NULL;
  assert (wexch && wexch->va_itype == MOMITY_WEBEXCH);
  assert (wexitm && wexitm->va_itype == MOMITY_ITEM);
  assert (thistatitm && thistatitm->va_itype == MOMITY_ITEM);
  sessitm = wexch->webx_sessitm;
  assert (sessitm && sessitm->va_itype == MOMITY_ITEM);
  if (!curitm || curitm == MOM_EMPTY_SLOT)
    {
      if (isval)
        mom_wexch_puts (wexch, "<span class='momnilval_cl'>~</span>");
      else
        mom_wexch_puts (wexch, "<span class='momnilitem_cl'>~</span>");
    }
  else
    {
      if (!
          (hsetitm =
           mom_dyncast_item (mom_vectvaldata_nth
                             (sessitm->itm_pcomp, mes_itmhset))))
        MOM_FATAPRINTF
          ("showitem_nanoedit wexitm %s has sessitm %s without hashset",
           mom_item_cstring (wexitm), mom_item_cstring (sessitm));
      assert (mom_itype (hsetitm->itm_payload) == MOMITY_HASHSET);
      MOM_DEBUGPRINTF (web,
                       "showitem_nanoedit wexitm=%s sessitm=%s hsetitm=%s curitm=%s",
                       mom_item_cstring (wexitm), mom_item_cstring (sessitm),
                       mom_item_cstring (hsetitm), mom_item_cstring (curitm));
      hsetitm->itm_payload = (struct mom_anyvalue_st *)
        mom_hashset_insert ((struct mom_hashset_st *) hsetitm->itm_payload,
                            (struct mom_item_st *) curitm);
      if (isval)
        MOM_WEXCH_PRINTF (wexch,
                          "<span class='momitemval_cl'>%s</span>",
                          mom_item_cstring (curitm));
      else
        MOM_WEXCH_PRINTF (wexch,
                          "<span class='momitemref_cl'>%s</span>",
                          mom_item_cstring (curitm));
    }
}                               // end showitem_nanoedit_mom


static void
newline_nanoedit_mom (struct mom_webexch_st *wexch, int depth)
{
  char buf[24];
  memset (buf, 0, sizeof (buf));
  buf[0] = '\n';
  if (MOM_IS_DEBUGGING (web))
    {
      int d = depth % 16;
      for (int ix = 1; ix <= d; ix++)
        buf[ix] = ' ';
    }
  else
    {
      int d = depth % 4;
      for (int ix = 1; ix <= d; ix++)
        buf[ix] = ' ';
    }
  mom_wexch_puts (wexch, buf);
}                               /* end of newline_nanoedit_mom */


static void
utf8escape_nanoedit_mom (FILE *f, gunichar uc, const char *cescstr,
                         void *clientdata)
{
  struct mom_webexch_st *wexch = clientdata;
  assert (f == wexch->webx_outfil);
  switch (uc)
    {
    case '&':
      mom_wexch_puts (wexch, "&amp;");
      break;
    case '<':
      mom_wexch_puts (wexch, "&lt;");
      break;
    case '>':
      mom_wexch_puts (wexch, "&gt;");
      break;
    case '\'':
      mom_wexch_puts (wexch, "<span class='momcharesc_cl'>\\&apos;</span>");
      break;
    case '\"':
      mom_wexch_puts (wexch, "<span class='momcharesc_cl'>\\&quot;</span>");
      break;
    default:
      MOM_WEXCH_PRINTF (wexch, "<span class='momcharesc_cl'>%s</span>",
                        cescstr);
      break;
    }
}


static void
showvalue_nanoedit_mom (struct mom_webexch_st *wexch,
                        struct mom_item_st *wexitm,
                        struct mom_item_st *thistatitm, const void *pval,
                        int depth)
{
  switch (mom_itype (pval))
    {
    case MOMITY_NONE:
      mom_wexch_puts (wexch, "<span class='momnilval_cl'>~</span>");
      return;
    case MOMITY_BOXINT:
      MOM_WEXCH_PRINTF (wexch, "<span class='momintval_cl'>%lld</span>",
                        (long long) ((const struct mom_boxint_st *)
                                     pval)->boxi_int);
      return;
    case MOMITY_BOXDOUBLE:
      {
        char buf[48];
        memset (buf, 0, sizeof (buf));
        double x = ((const struct mom_boxdouble_st *) pval)->boxd_dbl;
        MOM_WEXCH_PRINTF
          (wexch, "<span class='momdblval_cl'>%s</span>",
           mom_double_to_cstr (x, buf, sizeof (buf)));
      }
      return;
    case MOMITY_ITEM:
      showitem_nanoedit_mom (wexch, wexitm, thistatitm,
                             (struct mom_item_st *) pval, true);
      return;
    case MOMITY_BOXSTRING:
      mom_wexch_puts (wexch, "\"<span class='momstrval_cl'>");
      mom_output_utf8_escaped (wexch->webx_outfil,
                               ((const struct mom_boxstring_st *) pval)->cstr,
                               mom_size (pval), utf8escape_nanoedit_mom,
                               wexch);
      mom_wexch_puts (wexch, "</span>\"");
      return;
    case MOMITY_TUPLE:
      {
        const struct mom_boxtuple_st *tup = pval;
        unsigned siz = mom_raw_size (tup);
        mom_wexch_puts (wexch, " <span class='momtup_cl'>[");
        for (unsigned ix = 0; ix < siz; ix++)
          {
            if (ix > 0)
              mom_wexch_puts (wexch, ",");
            if (ix % 4 == 0)
              newline_nanoedit_mom (wexch, depth);
            else
              mom_wexch_puts (wexch, " ");
            showitem_nanoedit_mom (wexch, wexitm, thistatitm,
                                   tup->seqitem[ix], false);
          }
        mom_wexch_puts (wexch, "]</span>");
      }
      return;
    case MOMITY_SET:
      {
        const struct mom_boxset_st *set = pval;
        unsigned siz = mom_raw_size (set);
        mom_wexch_puts (wexch, " <span class='momset_cl'>{");
        for (unsigned ix = 0; ix < siz; ix++)
          {
            if (ix > 0)
              mom_wexch_puts (wexch, ",");
            if (ix % 4 == 0)
              newline_nanoedit_mom (wexch, depth);
            else
              mom_wexch_puts (wexch, " ");
            showitem_nanoedit_mom (wexch, wexitm, thistatitm,
                                   set->seqitem[ix], false);
          }
        mom_wexch_puts (wexch, "}</span>");
      }
      return;
    case MOMITY_NODE:
      {
        const struct mom_boxnode_st *nod = pval;
        unsigned siz = mom_raw_size (nod);
        struct mom_item_st *moditm
          = mom_dyncast_item (mom_unsync_item_get_phys_attr (thistatitm,
                                                             MOM_PREDEFITM
                                                             (display)));
        MOM_DEBUGPRINTF (web,
                         "showvalue_nanoedit wexitm=%s thistatitm=%s moditm=%s, depth#%d, node %s",
                         mom_item_cstring (wexitm),
                         mom_item_cstring (thistatitm),
                         mom_item_cstring (moditm), depth,
                         mom_value_cstring (pval));
        if (moditm == MOM_PREDEFITM (cooked))
          {
            struct mom_item_st *connitm = nod->nod_connitm;
            struct mom_hashedvalue_st *conndisplayerv = NULL;
            struct mom_item_st *dispitm = NULL;
            struct mom_item_st *dsigitm = NULL;
            mom_displayer_sig_t *disprout = NULL;
            mom_item_lock (connitm);
            conndisplayerv =
              mom_unsync_item_get_phys_attr (connitm,
                                             MOM_PREDEFITM (displayer));
            mom_item_unlock (connitm);
            MOM_DEBUGPRINTF (web,
                             "showvalue_nanoedit wexitm=%s connitm=%s conndisplayerv=%s",
                             mom_item_cstring (wexitm),
                             mom_item_cstring (connitm),
                             mom_value_cstring (conndisplayerv));
            const struct mom_boxnode_st *conndispnod =
              mom_dyncast_node (conndisplayerv);
            if (conndispnod)
              {
                dispitm = conndispnod->nod_connitm;
                if (dispitm)
                  {
                    mom_item_lock (dispitm);
                    dsigitm = dispitm->itm_funsig;
                    if (dsigitm == MOM_PREDEFITM (signature_displayer))
                      disprout = (mom_displayer_sig_t *) dispitm->itm_funptr;
                    mom_item_unlock (dispitm);
                  }
              }
            if (disprout)
              {
                MOM_DEBUGPRINTF (web,
                                 "showvalue_nanoedit wexitm=%s connitm=%s dispitm=%s",
                                 mom_item_cstring (wexitm),
                                 mom_item_cstring (connitm),
                                 mom_item_cstring (dispitm));
                (*disprout) (conndispnod, wexch, wexitm, thistatitm, pval,
                             depth);
                MOM_DEBUGPRINTF (web,
                                 "showvalue_nanoedit done wexitm=%s connitm=%s dispitm=%s",
                                 mom_item_cstring (wexitm),
                                 mom_item_cstring (connitm),
                                 mom_item_cstring (dispitm));
                return;
              }
          }
        mom_wexch_puts (wexch, " <span class='momnode_cl'>%");
        showitem_nanoedit_mom (wexch, wexitm, thistatitm,
                               nod->nod_connitm, false);
        mom_wexch_puts (wexch, "(");
        for (unsigned ix = 0; ix < siz; ix++)
          {
            if (ix > 0)
              mom_wexch_puts (wexch, ",");
            if (ix % 2 == 0)
              newline_nanoedit_mom (wexch, depth);
            else
              mom_wexch_puts (wexch, " ");
            showvalue_nanoedit_mom (wexch, wexitm, thistatitm,
                                    nod->nod_sons[ix], depth + 1);
          };
        mom_wexch_puts (wexch, ")</span>");
      }
      return;
    default:
      MOM_FATAPRINTF ("showvalue_nanoedit_mom incomplete pval:%s",
                      mom_value_cstring ((struct mom_hashedvalue_st *) pval));
      break;
    }
}                               /* end showvalue_nanoedit_mom */



const char momsig_nano_displayer[] = "signature_displayer";
void
momf_nano_displayer (const struct mom_boxnode_st *closnod,
                     struct mom_webexch_st *wexch, struct mom_item_st *wexitm,
                     struct mom_item_st *thistatitm, const void *pval,
                     int depth)
{
  assert (closnod != NULL);
  MOM_DEBUGPRINTF (web,
                   "nano_displayer closnod=%s wexitm=%s thistatitm=%s depth#%d pval=%s",
                   mom_value_cstring ((void *) closnod),
                   mom_item_cstring (wexitm), mom_item_cstring (thistatitm),
                   depth, mom_value_cstring (pval));
  showvalue_nanoedit_mom (wexch, wexitm, thistatitm, pval, depth);
}                               /* end of momf_nano_displayer */


static void
dofillpage_nanoedit_mom (struct mom_webexch_st *wexch,
                         struct mom_item_st *tkitm,
                         struct mom_item_st *wexitm,
                         struct mom_item_st *thistatitm)
{
  struct mom_item_st *sessitm = wexch->webx_sessitm;
  const char *rawmode = onion_request_get_post (wexch->webx_requ, "rawmode");
  MOM_DEBUGPRINTF (web,
                   "dofillpage_nanoedit webr#%ld tkitm=%s wexitm=%s thistatitm=%s sessitm=%s rawmode=%s",
                   wexch->webx_count, mom_item_cstring (tkitm),
                   mom_item_cstring (wexitm), mom_item_cstring (thistatitm),
                   mom_item_cstring (sessitm), rawmode);
  bool israw = false;
  if (rawmode)
    {
      israw = (!strcmp (rawmode, "true") || !strcmp (rawmode, "1"));
      mom_unsync_item_put_phys_attr (thistatitm,
                                     MOM_PREDEFITM (display),
                                     israw ? MOM_PREDEFITM (raw) :
                                     MOM_PREDEFITM (cooked));
    }
  else
    {
      struct mom_item_st *moditm
        = mom_dyncast_item (mom_unsync_item_get_phys_attr (thistatitm,
                                                           MOM_PREDEFITM
                                                           (display)));
      israw = (moditm == MOM_PREDEFITM (raw));
      MOM_DEBUGPRINTF (web,
                       "dofillpage_nanoedit webr#%ld  thistatitm=%s moditm=%s :: mode %s",
                       wexch->webx_count, mom_item_cstring (thistatitm),
                       mom_item_cstring (moditm), israw ? "raw" : "cooked");

    }
  char modbuf[64];
  memset (modbuf, 0, sizeof (modbuf));
  MOM_WEXCH_PRINTF (wexch, "<h3>%s state <tt>%s</tt> on <i>%s</i></h3>\n",
                    israw ? "raw" : "cooked",
                    mom_item_cstring (thistatitm),
                    mom_strftime_centi (modbuf, sizeof (modbuf) - 1, "%c %Z",
                                        thistatitm->itm_mtime));
  char timbuf[80];
  memset (timbuf, 0, sizeof (timbuf));
  mom_now_strftime_centi (timbuf, sizeof (timbuf) - 1,
                          "%Y %b %d, %H:%M:%S.__ %Z");
  MOM_WEXCH_PRINTF (wexch, "<small>(generated on %s by commit: <tt>%s</tt>)</small>\n", timbuf, monimelt_lastgitcommit);
  mom_wexch_puts (wexch, israw
                  ? "<p id='momrawpara_id'>"
                  "<input type='checkbox' id='momrawbox_id' name='mode' value='raw' checked/>"
                  " raw display</p>\n"
                  : "<p id='momrawpara_id'>"
                  "<input type='checkbox' id='momrawbox_id' name='mode' value='raw'/>"
                  " raw display</p>\n");
  struct mom_hashmap_st *hmap = mom_hashmap_dyncast (thistatitm->itm_payload);
  const struct mom_boxset_st *atset = mom_hashmap_keyset (hmap);
  MOM_DEBUGPRINTF (web, "dofillpage_nanoedit webr#%ld atset %s",
                   wexch->webx_count,
                   mom_value_cstring ((struct mom_hashedvalue_st *) atset));
  unsigned nbat = mom_size (atset);
  mom_wexch_puts (wexch, "<dl class='momlocbind_cl'>\n");
  for (unsigned ix = 0; ix < nbat; ix++)
    {
      const struct mom_item_st *curatitm = atset->seqitem[ix];
      const struct mom_hashedvalue_st *curval =
        mom_hashmap_get (hmap, curatitm);
      MOM_DEBUGPRINTF (web,
                       "dofillpage_nanoedit webr#%ld ix%d curatitm %s curval %s",
                       wexch->webx_count, ix, mom_item_cstring (curatitm),
                       mom_value_cstring (curval));
      MOM_WEXCH_PRINTF (wexch, "<dt class='momlocvaritem_cl'>%s</dt>\n",
                        mom_item_cstring (curatitm));
      mom_wexch_puts (wexch, "<dd class='momlocvalue_cl'>\n");
      showvalue_nanoedit_mom (wexch, wexitm, thistatitm, curval, 0);
      mom_wexch_puts (wexch, "</dd>\n");
    }
  mom_wexch_puts (wexch, "</dl>\n");
  mom_wexch_reply (wexch, HTTP_OK, "text/html");
  MOM_DEBUGPRINTF (web,
                   "dofillpage_nanoedit done webr#%ld tkitm=%s",
                   wexch->webx_count, mom_item_cstring (tkitm));
}                               /* end of dofillpage_nanoedit_mom */

static void
doparsecommand_nanoedit_mom (struct mom_webexch_st *wexch,
                             struct mom_item_st *tkitm,
                             struct mom_item_st *wexitm,
                             struct mom_item_st *thistatitm, const char *cmd);


static void
doknownitem_nanoedit_mom (struct mom_webexch_st *wexch,
                          struct mom_item_st *tkitm,
                          struct mom_item_st *wexitm,
                          struct mom_item_st *thistatitm, const char *name)
{
  struct mom_item_st *sessitm = wexch->webx_sessitm;
  bool known = false;
  MOM_DEBUGPRINTF (web,
                   "doknownitem_nanoedit webr#%ld tkitm=%s wexitm=%s thistatitm=%s sessitm=%s name=%s",
                   wexch->webx_count, mom_item_cstring (tkitm),
                   mom_item_cstring (wexitm), mom_item_cstring (thistatitm),
                   mom_item_cstring (sessitm), name);
  if (name)
    {
      struct mom_item_st *itm = mom_find_item_by_string (name);
      MOM_DEBUGPRINTF (web,
                       "doknownitem_nanoedit webr#%ld name=%s itm=%s\n",
                       wexch->webx_count, name, mom_item_cstring (itm));
      known = itm != NULL;
    };
  mom_wexch_puts (wexch, known ? "true\n" : "false\n");
  mom_wexch_reply (wexch, HTTP_OK, "application/json");
  MOM_DEBUGPRINTF (web,
                   "doknowitem_nanoedit done webr#%ld tkitm=%s name %s known %s",
                   wexch->webx_count, mom_item_cstring (tkitm), name,
                   known ? "yes" : "no");
}                               /* end doknownitem_nanoedit_mom */




static void
docompletename_nanoedit_mom (struct mom_webexch_st *wexch,
                             struct mom_item_st *tkitm,
                             struct mom_item_st *wexitm,
                             struct mom_item_st *thistatitm, const char *name)
{
  struct mom_item_st *sessitm = wexch->webx_sessitm;
  MOM_DEBUGPRINTF (web,
                   "docompletename_nanoedit webr#%ld tkitm=%s wexitm=%s thistatitm=%s sessitm=%s name=%s",
                   wexch->webx_count, mom_item_cstring (tkitm),
                   mom_item_cstring (wexitm), mom_item_cstring (thistatitm),
                   mom_item_cstring (sessitm), name);
  const struct mom_boxset_st *set = mom_set_items_prefixed (name, -1);
  MOM_DEBUGPRINTF (web,
                   "docompletename_nanoedit webr#%ld set %s",
                   wexch->webx_count,
                   mom_value_cstring ((const struct mom_hashedvalue_st *)
                                      set));
  if (set)
    {
      unsigned siz = mom_size (set);
      mom_wexch_puts (wexch, "[");
      for (unsigned ix = 0; ix < siz; ix++)
        {
          const struct mom_item_st *curitm = set->seqitem[ix];
          assert (curitm && curitm->va_itype == MOMITY_ITEM);
          if (ix > 0)
            mom_wexch_puts (wexch, MOM_IS_DEBUGGING (web) ? ",\n " : ",");
          MOM_WEXCH_PRINTF (wexch, "\"%s\"", mom_item_cstring (curitm));
        }
      mom_wexch_puts (wexch, "]\n");
    }
  else
    {
      mom_wexch_puts (wexch, "null\n");
    }
  mom_wexch_reply (wexch, HTTP_OK, "application/json");
  MOM_DEBUGPRINTF (web,
                   "docompletename_nanoedit webr#%ld done",
                   wexch->webx_count);
}                               /* end of docompletename_nanoedit_mom */


static void
doexit_nanoedit_mom (struct mom_webexch_st *wexch,
                     struct mom_item_st *tkitm,
                     struct mom_item_st *wexitm,
                     struct mom_item_st *thistatitm, const char *doexit)
{
  struct mom_item_st *sessitm = wexch->webx_sessitm;
  MOM_DEBUGPRINTF (web,
                   "doexit_nanoedit webr#%ld tkitm=%s wexitm=%s thistatitm=%s sessitm=%s doexit=%s",
                   wexch->webx_count, mom_item_cstring (tkitm),
                   mom_item_cstring (wexitm), mom_item_cstring (thistatitm),
                   mom_item_cstring (sessitm), doexit);
  json_t *jreply = json_pack ("{s:f,s:f}",
                              "elapsedreal", mom_elapsed_real_time (),
                              "processcpu", mom_process_cpu_time ());;
  // json_dumps will use GC_STRDUP...
  MOM_DEBUGPRINTF (web, "doexit_nanoedit jreply=%s",
                   json_dumps (jreply, JSON_INDENT (1)));
  mom_wexch_puts (wexch, json_dumps (jreply, JSON_INDENT (1)));
  mom_wexch_reply (wexch, HTTP_OK, "application/json");
  mom_stop_and_dump ();
}                               /* end of doexit_nanoedit_mom */


extern mom_tasklet_sig_t momf_nanoedit;
const char momsig_nanoedit[] = "signature_tasklet";

void
momf_nanoedit (struct mom_item_st *tkitm)
{
  enum nanoedit_closoff_en
  {
    mec_wexitm,
    mec_protowebstate,
    mec_thiswebstate,
    mec__last
  };
  struct mom_item_st *wexitm = NULL;
  struct mom_item_st *thistatitm = NULL;
  struct mom_item_st *sessitm = NULL;
  struct mom_item_st *hsetitm = NULL;
  mom_item_lock (tkitm);
  const struct mom_boxnode_st *tknod =
    (struct mom_boxnode_st *) tkitm->itm_payload;
  MOM_DEBUGPRINTF (web, "momf_nanoedit start tkitm=%s tknod=%s",
                   mom_item_cstring (tkitm),
                   mom_value_cstring ((const struct mom_hashedvalue_st *)
                                      tknod));
  if (mom_itype (tknod) != MOMITY_NODE || mom_raw_size (tknod) < mec__last)
    {
      /// should not happen
      MOM_WARNPRINTF ("momf_nanoedit bad tknod %s",
                      mom_value_cstring ((const struct mom_hashedvalue_st *)
                                         tknod));
      goto end;
    }
  {
    wexitm = mom_dyncast_item (tknod->nod_sons[mec_wexitm]);
    assert (wexitm && wexitm->va_itype == MOMITY_ITEM);
    mom_item_lock (wexitm);
    thistatitm = mom_dyncast_item (tknod->nod_sons[mec_thiswebstate]);
    assert (thistatitm && thistatitm->va_itype == MOMITY_ITEM);
    MOM_DEBUGPRINTF (web,
                     "momf_nanoedit tkitm=%s wexitm=%s paylkind %s thistatitm=%s",
                     mom_item_cstring (tkitm), mom_item_cstring (wexitm),
                     mom_itype_str (wexitm->itm_payload),
                     mom_item_cstring (thistatitm));
    mom_item_lock (thistatitm);
  }
  struct mom_webexch_st *wexch =
    (struct mom_webexch_st *) wexitm->itm_payload;
  assert (wexch && wexch->va_itype == MOMITY_WEBEXCH);
  sessitm = wexch->webx_sessitm;
  mom_item_lock (sessitm);
  MOM_DEBUGPRINTF (web,
                   "momf_nanoedit tkitm=%s wexch #%ld meth %s fupath %s sessitm %s",
                   mom_item_cstring (tkitm), wexch->webx_count,
                   mom_webmethod_name (wexch->webx_meth),
                   onion_request_get_fullpath (wexch->webx_requ),
                   mom_item_cstring (sessitm));
  sessitm->itm_pcomp = mom_vectvaldata_resize (sessitm->itm_pcomp, mes__last);
  if (!
      (hsetitm =
       mom_dyncast_item (mom_vectvaldata_nth
                         (sessitm->itm_pcomp, mes_itmhset))))
    {
      hsetitm = mom_clone_item (MOM_PREDEFITM (hashset));
      mom_vectvaldata_put_nth (sessitm->itm_pcomp, mes_itmhset,
                               (struct mom_hashedvalue_st *) hsetitm);
      hsetitm->itm_payload =
        (struct mom_anyvalue_st *) mom_hashset_reserve (NULL, 20);
      MOM_DEBUGPRINTF (web,
                       "momf_nanoedit tkitm=%s wexch #%ld new hsetitm %s",
                       mom_item_cstring (tkitm), wexch->webx_count,
                       mom_item_cstring (hsetitm));
    }
  else
    MOM_DEBUGPRINTF (web,
                     "momf_nanoedit tkitm=%s wexch #%ld got hsetitm %s",
                     mom_item_cstring (tkitm), wexch->webx_count,
                     mom_item_cstring (hsetitm));

  mom_item_lock (hsetitm);
  if (wexch->webx_meth == MOMWEBM_POST)
    {
      const char *dofillpage = NULL;
      const char *doknownitem = NULL;
      const char *docompletename = NULL;
      const char *doexit = NULL;
      const char *doparsecommand = NULL;
      MOM_DEBUGPRINTF (web,
                       "momf_nanoedit tkitm=%s POST wexch #%ld",
                       mom_item_cstring (tkitm), wexch->webx_count);
      if ((dofillpage =
           onion_request_get_post (wexch->webx_requ, "do_fillpage")) != NULL)
        dofillpage_nanoedit_mom (wexch, tkitm, wexitm, thistatitm);
      else if ((doknownitem =
                onion_request_get_post (wexch->webx_requ,
                                        "do_knownitem")) != NULL)
        doknownitem_nanoedit_mom (wexch, tkitm, wexitm, thistatitm,
                                  doknownitem);
      else if ((docompletename =
                onion_request_get_post (wexch->webx_requ,
                                        "do_completename")) != NULL)
        docompletename_nanoedit_mom (wexch, tkitm, wexitm, thistatitm,
                                     docompletename);
      else if ((doparsecommand = onion_request_get_post (wexch->webx_requ,
                                                         "do_parsecommand"))
               != NULL)
        doparsecommand_nanoedit_mom (wexch, tkitm, wexitm, thistatitm,
                                     doparsecommand);
      else if ((doexit = onion_request_get_post (wexch->webx_requ, "do_exit"))
               != NULL)
        doexit_nanoedit_mom (wexch, tkitm, wexitm, thistatitm, doexit);
    }
end:
  if (hsetitm)
    mom_item_unlock (hsetitm);
  if (thistatitm)
    mom_item_unlock (thistatitm);
  if (wexitm)
    mom_item_unlock (wexitm);
  if (sessitm)
    mom_item_unlock (sessitm);
  mom_item_unlock (tkitm);
}                               /* end of momf_nanoedit */


#define NANOPARSING_MAGIC_MOM 886322565 /*0x34d43585 */
struct nanoparsing_mom_st
{
  unsigned nanop_magic;
  unsigned nanop_pos;
  struct mom_boxstring_st *nanop_errmsgv;
  const char *nanop_cmdstr;
  jmp_buf nanop_jb;
};

#define NANOPARSING_FAILURE_MOM(Np,Pos,Fmt,...) do {            \
    struct nanoparsing_mom_st *_np = (Np);                      \
    assert (_np && _np->nanop_magic == NANOPARSING_MAGIC_MOM);  \
    int _pos = (Pos);                                           \
    if (_pos>0) _np->nanop_pos = _pos;                          \
    _np->nanop_errmsgv =                                        \
      mom_boxstring_printf((Fmt),#__VA_ARGS__);                 \
    longjmp(_np->nanop_jb,__LINE__);                            \
  } while(0)



static void
doparsecommand_nanoedit_mom (struct mom_webexch_st *wexch,
                             struct mom_item_st *tkitm,
                             struct mom_item_st *wexitm,
                             struct mom_item_st *thistatitm, const char *cmd)
{
  struct mom_item_st *sessitm = wexch->webx_sessitm;
  MOM_DEBUGPRINTF (web,
                   "doparsecommand_nanoedit webr#%ld tkitm=%s wexitm=%s thistatitm=%s sessitm=%s cmd=%s",
                   wexch->webx_count, mom_item_cstring (tkitm),
                   mom_item_cstring (wexitm), mom_item_cstring (thistatitm),
                   mom_item_cstring (sessitm), cmd);
  struct nanoparsing_mom_st nanopars;
  memset (&nanopars, 0, sizeof (nanopars));
  nanopars.nanop_magic = NANOPARSING_MAGIC_MOM;
  nanopars.nanop_pos = 0;
  nanopars.nanop_cmdstr = cmd;
  int linerr = 0;
  if ((linerr = setjmp (nanopars.nanop_jb)) != 0)
    {
      MOM_WARNPRINTF ("doparsecommand_nanoedit parsing error from %s:%d: %s"
                      "\n.. at position %u of:\n%s\n",
                      __FILE__, linerr,
                      mom_boxstring_cstr (nanopars.nanop_errmsgv),
                      nanopars.nanop_pos, nanopars.nanop_cmdstr);
    }
  else
    {
      MOM_WARNPRINTF ("unimplemented doparsecommand_nanoedit webr#%ld cmd=%s",
                      wexch->webx_count, cmd);
#warning doparsecommand_nanoedit_mom unimplemented
    }
  memset (&nanopars, 0, sizeof (nanopars));
}                               /* end of doparsecommand_nanoedit_mom */

/*** end of file nanoedit.c ***/
