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

enum nanoedit_closoff_en
{
  mec_wexitm,
  mec_protowebstate,
  mec_thiswebstate,
  mec_delimiters,
  mec__last
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
      mom_wexch_puts (wexch, "\"<span class='momstrval_cl' data-momstring='");
      mom_output_utf8_html (wexch->webx_outfil,
                            ((const struct mom_boxstring_st *) pval)->cstr,
                            mom_size (pval), false);
      mom_wexch_puts (wexch, "'>");
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
  MOM_WEXCH_PRINTF (wexch,
                    "<small>(generated on %s by commit: <tt>%s</tt>)</small>\n",
                    timbuf, monimelt_lastgitcommit);
  mom_wexch_puts (wexch,
                  israw ? "<p id='momrawpara_id'>"
                  "<input type='checkbox' id='momrawbox_id' name='mode' value='raw' checked/>"
                  " raw display</p>\n" : "<p id='momrawpara_id'>"
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


static void
docreateitem_nanoedit_mom (struct mom_webexch_st *wexch,
                           struct mom_item_st *tkitm,
                           struct mom_item_st *wexitm,
                           struct mom_item_st *thistatitm,
                           struct mom_item_st *sessitm,
                           const char *itemnamestr, const char *commentstr)
{
  struct mom_item_st *hsetitm = NULL;
  struct mom_item_st *newitm = NULL;
  MOM_DEBUGPRINTF (web, "docreateitem_nanoedit tkitm=%s wexitm=%s"
                   " thistatitm=%s sessitm=%s itemnamestr=%s commentstr=%s",
                   mom_item_cstring (tkitm),
                   mom_item_cstring (wexitm),
                   mom_item_cstring (thistatitm),
                   mom_item_cstring (sessitm), itemnamestr, commentstr);
  if (!
      (hsetitm =
       mom_dyncast_item (mom_vectvaldata_nth
                         (sessitm->itm_pcomp, mes_itmhset))))
    MOM_FATAPRINTF
      ("docreateitem_nanoedit wexitm %s has sessitm %s without hashset",
       mom_item_cstring (wexitm), mom_item_cstring (sessitm));
  assert (mom_itype (hsetitm->itm_payload) == MOMITY_HASHSET);
  newitm = mom_make_item_by_string (itemnamestr);
  if (!newitm)
    {
      MOM_WARNPRINTF ("docreateitem_nanoedit failed to create item named %s",
                      itemnamestr);
      MOM_WEXCH_PRINTF (wexch, "<b>failed to create item <tt>%s</tt></b>\n",
                        itemnamestr);
      mom_wexch_reply (wexch, HTTP_FORBIDDEN, "text/html");
      return;
    }
  else
    {
      char timbuf[64];
      memset (timbuf, 0, sizeof (timbuf));
      time_t now = time (&newitm->itm_mtime);
      mom_strftime_centi (timbuf, sizeof (timbuf), "%c %Z", (double) now);
      mom_item_put_space (newitm, MOMSPA_GLOBAL);
      MOM_DEBUGPRINTF (web, "docreateitem_nanoedit created item %s at %ld",
                       mom_item_cstring (newitm), (long) now);
      newitm->itm_pattr =       //
        mom_assovaldata_put (NULL,
                             MOM_PREDEFITM (comment),
                             (struct mom_hashedvalue_st *)
                             mom_boxstring_make (commentstr));
      hsetitm->itm_payload = (struct mom_anyvalue_st *)
        mom_hashset_insert ((struct mom_hashset_st *) hsetitm->itm_payload,
                            (struct mom_item_st *) newitm);
      MOM_WEXCH_PRINTF (wexch,
                        "<b>created item <tt>%s</tt></b> on <i>%s</i>\n",
                        mom_item_cstring (newitm), timbuf);
      mom_wexch_reply (wexch, HTTP_OK, "text/html");
      return;
    }
}                               /* end of docreateitem_nanoedit_mom */


extern mom_tasklet_sig_t momf_nanoedit;
const char momsig_nanoedit[] = "signature_tasklet";

void
momf_nanoedit (struct mom_item_st *tkitm)
{
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
      const char *docreateitem = NULL;
      const char *commentstr = NULL;
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
      else
        if ((docreateitem =
             onion_request_get_post (wexch->webx_requ,
                                     "do_createitem")) != NULL
            && (commentstr =
                onion_request_get_post (wexch->webx_requ, "comment")) != NULL)
        docreateitem_nanoedit_mom (wexch, tkitm, wexitm, thistatitm, sessitm,
                                   docreateitem, commentstr);
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
  int nanop_pos;
  const struct mom_boxstring_st *nanop_errmsgv;
  const void *nanop_errval;
  const char *nanop_cmdstr;
  unsigned nanop_cmdlen;
  struct mom_webexch_st *nanop_wexch;
  struct mom_item_st *nanop_tkitm;
  struct mom_item_st *nanop_wexitm;
  struct mom_item_st *nanop_thistatitm;
  struct mom_item_st *nanop_queitm;
  struct mom_item_st *nanop_delimitm;
  const struct mom_boxnode_st *nanop_nodexpr;
  jmp_buf nanop_jb;
};

#define NANOPARSING_FAILURE_MOM(Np,Pos,Fmt,...) do {            \
    struct nanoparsing_mom_st *_np = (Np);                      \
    assert (_np && _np->nanop_magic == NANOPARSING_MAGIC_MOM);  \
    int _pos = (Pos);                                           \
    if (_pos!=0) _np->nanop_pos = _pos;                         \
    _np->nanop_errmsgv =                                        \
      mom_boxstring_printf((Fmt),##__VA_ARGS__);                \
    longjmp(_np->nanop_jb,__LINE__);                            \
  } while(0)


#define NANOPARSING_FAILURE_WITH_MOM(Np,Pos,Val,Fmt,...) do {	\
    struct nanoparsing_mom_st *_np = (Np);                      \
    assert (_np && _np->nanop_magic == NANOPARSING_MAGIC_MOM);  \
    int _pos = (Pos);                                           \
    if (_pos!=0) _np->nanop_pos = _pos;                         \
    _np->nanop_errval = (Val);					\
    _np->nanop_errmsgv =                                        \
      mom_boxstring_printf((Fmt),##__VA_ARGS__);                \
    longjmp(_np->nanop_jb,__LINE__);                            \
  } while(0)



static bool
is_utf8_delim_mom (gunichar uc)
{
  if (!uc)
    return false;
  if (uc < 128)
    return ispunct (uc);
  if (g_unichar_ispunct (uc))
    return true;
  GUnicodeType gty = g_unichar_type (uc);
  switch (gty)
    {
      //// see https://developer.gnome.org/glib/stable/glib-Unicode-Manipulation.html#GUnicodeType
    case G_UNICODE_ENCLOSING_MARK:
    case G_UNICODE_CONNECT_PUNCTUATION:
    case G_UNICODE_DASH_PUNCTUATION:
    case G_UNICODE_CLOSE_PUNCTUATION:
    case G_UNICODE_FINAL_PUNCTUATION:
    case G_UNICODE_INITIAL_PUNCTUATION:
    case G_UNICODE_OTHER_PUNCTUATION:
    case G_UNICODE_OPEN_PUNCTUATION:
    case G_UNICODE_CURRENCY_SYMBOL:
    case G_UNICODE_MODIFIER_SYMBOL:
    case G_UNICODE_MATH_SYMBOL:
    case G_UNICODE_OTHER_SYMBOL:
      return true;
    case G_UNICODE_CONTROL:
    case G_UNICODE_FORMAT:
    case G_UNICODE_UNASSIGNED:
    case G_UNICODE_PRIVATE_USE:
    case G_UNICODE_SURROGATE:
    case G_UNICODE_LOWERCASE_LETTER:
    case G_UNICODE_MODIFIER_LETTER:
    case G_UNICODE_OTHER_LETTER:
    case G_UNICODE_TITLECASE_LETTER:
    case G_UNICODE_UPPERCASE_LETTER:
    case G_UNICODE_SPACING_MARK:
    case G_UNICODE_NON_SPACING_MARK:
    case G_UNICODE_DECIMAL_NUMBER:
    case G_UNICODE_LETTER_NUMBER:
    case G_UNICODE_OTHER_NUMBER:
    case G_UNICODE_LINE_SEPARATOR:
    case G_UNICODE_PARAGRAPH_SEPARATOR:
    case G_UNICODE_SPACE_SEPARATOR:
      return false;
    }
  return false;
}                               /* end of is_utf8_delim_mom */




static void
parse_token_nanoedit_mom (struct nanoparsing_mom_st *np)
{
  struct mom_queue_st *que = NULL;
  const char *cmd = NULL;
  const char *pc = NULL;
  char rawprefix[16] = { 0 };
  int pos = 0;
  gunichar uc = 0;
  char nc = 0;
  assert (np && np->nanop_magic == NANOPARSING_MAGIC_MOM);
  cmd = np->nanop_cmdstr;
  assert (cmd != NULL);
  assert (np->nanop_pos <= (int) np->nanop_cmdlen);
  struct mom_item_st *queitm = np->nanop_queitm;
  assert (queitm && queitm->va_itype == MOMITY_ITEM);
  que = mom_dyncast_queue (queitm->itm_payload);
  assert (que != NULL && que->va_itype == MOMITY_QUEUE);
  struct mom_item_st *delimitm = np->nanop_delimitm;
  assert (delimitm && delimitm->va_itype == MOMITY_ITEM);
  struct mom_hashassoc_st *delimass =
    mom_hashassoc_dyncast (delimitm->itm_payload);
  assert (delimass != NULL);
  // skip ASCII spaces
  for (pc = cmd + np->nanop_pos; *pc; pc = g_utf8_next_char (pc), uc = 0)
    {
      uc = g_utf8_get_char (pc);
      if (uc < 128 && isspace (uc))
        continue;
      else
        break;
    }
  np->nanop_pos = pc - cmd;
  if (!uc || np->nanop_pos >= (int) np->nanop_cmdlen)
    {
      MOM_DEBUGPRINTF (web, "parse_token_nanoedit pos#%u end (cmdlen %u)",
                       np->nanop_pos, np->nanop_cmdlen);
      return;
    }
  MOM_DEBUGPRINTF (web, "parse_token_nanoedit pos#%u uc %#x %c",
                   np->nanop_pos, uc, (uc < 128) ? (char) uc : '?');
  if (uc < 128 && isalpha (uc))
    {
      bool asciiw = true;
      const char *startw = pc;
      const char *endw = NULL;
      for (; *pc; pc = g_utf8_next_char (pc), uc = 0)
        {
          uc = g_utf8_get_char (pc);
          if (uc < 128 && (isalnum (uc) || uc == '_'))
            continue;
          if (g_unichar_isalpha (uc))
            {
              asciiw = false;
              continue;
            }
          else
            break;
        };
      endw = pc;
      if (asciiw && mom_valid_name_radix (startw, endw - startw))
        {
          const char *endp = NULL;
          struct mom_item_st *itm = mom_find_item_from_string (startw, &endp);
          if (itm)
            {
              assert (endp == endw);
              mom_queue_append (que, itm);
              np->nanop_pos = endw - cmd;
              MOM_DEBUGPRINTF (web, "parse_token_nanoedit pos#%u item %s\n",
                               (unsigned) (endw - cmd),
                               mom_item_cstring (itm));
            }
          else
            {                   // valid item name, but not found
              const struct mom_boxstring_st *namev
                = mom_boxstring_make_len (startw, endw - startw);
              const struct mom_boxnode_st *nodv =
                mom_boxnode_meta_make_va (NULL, startw - cmd,
                                          MOM_PREDEFITM (name),
                                          1,
                                          namev);
              mom_queue_append (que, nodv);
              np->nanop_pos = endw - cmd;
              MOM_DEBUGPRINTF (web, "parse_token_nanoedit pos#%u name %s\n",
                               (unsigned) (endw - cmd),
                               mom_value_cstring ((const struct
                                                   mom_hashedvalue_st
                                                   *) (nodv)));
            }
        }
      else
        {
          /// invalid name so word
          const struct mom_boxstring_st *wordv
            = mom_boxstring_make_len (startw, endw - startw);
          const struct mom_boxnode_st *nodv =
            mom_boxnode_meta_make_va (NULL, startw - cmd,
                                      MOM_PREDEFITM (word),
                                      1,
                                      wordv);
          mom_queue_append (que, nodv);
          np->nanop_pos = endw - cmd;
          MOM_DEBUGPRINTF (web, "parse_token_nanoedit pos#%u word %s\n",
                           (unsigned) (startw - cmd),
                           mom_value_cstring ((const struct
                                               mom_hashedvalue_st *) (nodv)));
        }
      return;
    }                           /* end if uc is an ASCII letter */

  else if (uc == '0' && pc[1] == '\'' && pc[2])
    {                           // 0'z is inputting the integer
      // code of UTF8 character z
      pc += 2;
      gunichar u = g_utf8_get_char (pc);
      pc = g_utf8_next_char (pc);
      const struct mom_boxint_st *intv = mom_boxint_make (u);
      mom_queue_append (que, intv);
      MOM_DEBUGPRINTF (web, "parse_token_nanoedit pos#%u charcode int %s\n",
                       np->nanop_pos,
                       mom_value_cstring ((const struct
                                           mom_hashedvalue_st *) (intv)));
      np->nanop_pos = pc - cmd;
      return;
    }                           /* end of integer character code à la 0'z */

  else if (uc == '0' && pc[1] == '\\' && isalpha (pc[2]))
    {                           // 0\n is inputting the
      // escaped char \n
      char escbuf[4] = "";
      escbuf[0] = pc[1];
      escbuf[1] = pc[2];
      FILE *f = fmemopen ((void *) escbuf, 2, "r");
      struct mom_string_and_size_st ss = mom_input_quoted_utf8 (f);
      fclose (f);
      if (ss.ss_len == 1)
        {
          const struct mom_boxint_st *intv = mom_boxint_make (ss.ss_str[0]);
          mom_queue_append (que, intv);
          MOM_DEBUGPRINTF (web,
                           "parse_token_nanoedit pos#%u charcode int %s\n",
                           np->nanop_pos,
                           mom_value_cstring ((const struct mom_hashedvalue_st
                                               *) (intv)));
          np->nanop_pos += 3;
        }
      else
        NANOPARSING_FAILURE_MOM (np, np->nanop_pos, "bad escchar int 0%s",
                                 escbuf);

      ss.ss_str = NULL, ss.ss_len = 0;
    }                           /* end of integer escaped character code à la 0\n */

  else if ((uc < 128 && isdigit (uc))
           || ((uc == '+' || uc == '-') && (nc = pc[1])
               && (isdigit (nc)
                   || ((!strncasecmp (pc + 1, "INF", 3)
                        || !strncasecmp (pc + 1, "NAN", 3))
                       && !isalnum (*(pc + 4))))))
    {                           // some kind of number
      char *endlng = NULL;
      char *enddbl = NULL;
      long long l = strtoll (pc, &endlng, 0);
      double d = strtod (pc, &enddbl);
      if (endlng >= enddbl)
        {
          const struct mom_boxint_st *intv = mom_boxint_make (l);
          mom_queue_append (que, intv);
          MOM_DEBUGPRINTF (web, "parse_token_nanoedit pos#%u int %s\n",
                           np->nanop_pos,
                           mom_value_cstring ((const struct
                                               mom_hashedvalue_st *) (intv)));
          np->nanop_pos = endlng - cmd;
        }
      else
        {
          const struct mom_boxdouble_st *dblv = mom_boxdouble_make (d);
          mom_queue_append (que, dblv);
          MOM_DEBUGPRINTF (web, "parse_token_nanoedit pos#%u double %s\n",
                           np->nanop_pos,
                           mom_value_cstring ((const struct
                                               mom_hashedvalue_st *) (dblv)));
          np->nanop_pos = enddbl - cmd;
        }
      return;
    }                           /* end number parsing */

  else if (uc == '/' && pc[1] == '/')
    {                           // comment line
      const char *startc = pc + 2;
      const char *endc = pc + 2;
      while (*endc
             && (*endc != '\n' || *endc != '\r' || *endc != '\f'
                 || *endc != '\v'))
        endc++;
      const struct mom_boxstring_st *commv
        = mom_boxstring_make_len (startc, endc - startc);
      const struct mom_boxnode_st *nodv =
        mom_boxnode_meta_make_va (NULL, startc - cmd,
                                  MOM_PREDEFITM (comment),
                                  1,
                                  commv);
      mom_queue_append (que, nodv);
      np->nanop_pos = endc - cmd;
      MOM_DEBUGPRINTF (web, "parse_token_nanoedit pos#%u comment: %s\n",
                       (unsigned) (startc - cmd),
                       mom_value_cstring ((const struct
                                           mom_hashedvalue_st *) (nodv)));
      return;
    }                           /* end line comment parsing */

  else if (uc == '"')
    {                           // escaped string
      const char *starts = pc + 1;
      const char *ends = NULL;
      const struct mom_boxstring_st *strv = NULL;
      {
        struct mom_string_and_size_st ss = { NULL, 0 };
        const char *endl = starts;
        while (*endl
               && (*endl != '\n' || *endl != '\r' || *endl != '\f'
                   || *endl != '\v'))
          endl++;
        FILE *f = fmemopen ((void *) starts, endl - starts, "r");
        if (MOM_UNLIKELY (!f))
          MOM_FATAPRINTF ("failed to fmemopen %*s (%m)",
                          (int) (endl - starts), starts);
        ss = mom_input_quoted_utf8 (f);
        assert (ftell (f) >= 0);
        ends = starts + ftell (f);
        fclose (f);
        if (*ends == '"')
          ends++;
        strv = mom_boxstring_make_len (ss.ss_str, ss.ss_len);
        ss.ss_str = NULL, ss.ss_len = 0;
      }
      mom_queue_append (que, strv);
      np->nanop_pos = ends - cmd;
      MOM_DEBUGPRINTF (web, "parse_token_nanoedit pos#%u string: %s\n",
                       (unsigned) (starts - cmd),
                       mom_value_cstring ((const struct
                                           mom_hashedvalue_st *) (strv)));
      return;
    }                           /* end escaped string parsing */

  else if (uc == '(' && isalpha (pc[1])
           && ((pos = 0), (memset (rawprefix, 0, sizeof (rawprefix))),
               sscanf (pc, "(%10[A-Za-z]\"%n", rawprefix + 1, &pos)) >= 1
           && pos > 0 && pos < (int) sizeof (rawprefix))
    {                           // raw string parsing, like (ABC"some\escaped"ABC)  
      const char *starts = pc;
      rawprefix[0] = '"';
      const char *raws = pc + pos;
      const char *endraws = strstr (raws, rawprefix);
      const struct mom_boxstring_st *strv = NULL;
      if (!endraws || endraws[strlen (rawprefix + 1)] != ')')
        NANOPARSING_FAILURE_MOM (np, np->nanop_pos,
                                 "raw string not ended by %s)", rawprefix);
      strv = mom_boxstring_make_len (raws, endraws - raws);
      np->nanop_pos = endraws + strlen (rawprefix + 1) + 1 - cmd;
      MOM_DEBUGPRINTF (web,
                       "parse_token_nanoedit pos#%u raw prefixed %s string: %s\n",
                       (unsigned) (starts - cmd), rawprefix + 1,
                       mom_value_cstring ((const struct mom_hashedvalue_st
                                           *) (strv)));
      return;
    }                           /* end raw string parsing */

  else if (g_unichar_isalpha (uc))
    {                           // word starting by extended letter
      const char *startw = pc;
      const char *endw = NULL;
      for (; *pc; pc = g_utf8_next_char (pc), uc = 0)
        {
          uc = g_utf8_get_char (pc);
          if (uc < 128 && (isalnum (uc) || uc == '_'))
            continue;
          if (g_unichar_isalnum (uc))
            continue;
          else
            break;
        };
      endw = pc;
      const struct mom_boxstring_st *wordv
        = mom_boxstring_make_len (startw, endw - startw);
      const struct mom_boxnode_st *nodv =
        mom_boxnode_meta_make_va (NULL, startw - cmd,
                                  MOM_PREDEFITM (word),
                                  1,
                                  wordv);
      mom_queue_append (que, nodv);
      np->nanop_pos = endw - cmd;
      MOM_DEBUGPRINTF (web, "parse_token_nanoedit pos#%u extendedword %s\n",
                       (unsigned) (startw - cmd),
                       mom_value_cstring ((const struct
                                           mom_hashedvalue_st *) (nodv)));
      return;
    }                           /* end extended word */

  else if (is_utf8_delim_mom (uc))      /* delimiters - one to four UTF8 */
    {
      const char *endp = NULL;
      const char *pc0 = pc;
      const char *pc1 = NULL;
      const char *pc2 = NULL;
      const char *pc3 = NULL;
      gunichar uc1 = 0;
      gunichar uc2 = 0;
      gunichar uc3 = 0;
      const struct mom_boxstring_st *delimstrv = NULL;
      const struct mom_hashedvalue_st *delimval = NULL;
      pc1 = g_utf8_next_char (pc);
      uc1 = (pc1 && *pc1) ? g_utf8_get_char (pc1) : 0;
      if (is_utf8_delim_mom (uc1))
        {
          pc2 = g_utf8_next_char (pc1);
          uc2 = (pc2 && *pc2) ? g_utf8_get_char (pc2) : 0;
          if (is_utf8_delim_mom (uc2))
            {
              pc3 = g_utf8_next_char (pc2);
              uc3 = (pc3 && *pc3) ? g_utf8_get_char (pc3) : 0;
              if (is_utf8_delim_mom (uc3))
                {
                  endp = g_utf8_next_char (pc3);
                  delimstrv = mom_boxstring_make_len (pc, endp - pc);
                  delimval =
                    mom_hashassoc_get (delimass,
                                       (const struct mom_hashedvalue_st *)
                                       delimstrv);
                  if (!delimval)
                    goto threedelims;
                }
              else              /*uc3 not delim */
              threedelims:
                {
                  endp = pc3;
                  delimstrv = mom_boxstring_make_len (pc, endp - pc);
                  delimval =
                    mom_hashassoc_get (delimass,
                                       (const struct mom_hashedvalue_st *)
                                       delimstrv);
                  if (!delimval)
                    goto twodelims;
                };
            }
          else                  /*uc2 not delim */
          twodelims:
            {
              endp = pc2;
              delimstrv = mom_boxstring_make_len (pc, endp - pc);
              delimval =
                mom_hashassoc_get (delimass,
                                   (const struct mom_hashedvalue_st *)
                                   delimstrv);
              if (!delimval)
                goto onedelim;
            };
        }
      else                      /*uc1 not delim */
      onedelim:
        {
          endp = pc1;
          delimstrv = mom_boxstring_make_len (pc, endp - pc);
          delimval =
            mom_hashassoc_get (delimass,
                               (const struct mom_hashedvalue_st *) delimstrv);
          if (!delimval)
            delimval = (const struct mom_hashedvalue_st *) delimstrv;
        };
      assert (delimval != NULL);
      const struct mom_boxnode_st *nodv =
        mom_boxnode_meta_make_va (NULL, pc0 - cmd,
                                  MOM_PREDEFITM (delimiter),
                                  1,
                                  delimval);
      mom_queue_append (que, nodv);
      np->nanop_pos = endp - cmd;
      MOM_DEBUGPRINTF (web, "parse_token_nanoedit pos#%u delimiter %s\n",
                       (unsigned) (pc0 - cmd),
                       mom_value_cstring ((const struct
                                           mom_hashedvalue_st *) (nodv)));
      return;

    }

#warning incomplete parse_token_nanoedit_mom

  NANOPARSING_FAILURE_MOM (np, np->nanop_pos,
                           "bad lexical token %s", cmd + np->nanop_pos);
}                               /* end parse_token_nanoedit_mom */





static inline bool
isdelim_nanoedit_mom (struct nanoparsing_mom_st *np, int pos,
                      struct mom_item_st *delimitm)
{
  assert (np && np->nanop_magic == NANOPARSING_MAGIC_MOM);
  const struct mom_boxnode_st *nodexp = np->nanop_nodexpr;
  assert (nodexp && nodexp->va_itype == MOMITY_NODE);
  assert (delimitm && delimitm->va_itype == MOMITY_ITEM);
  unsigned nlen = mom_raw_size (nodexp);
  if (pos < 0 || pos >= (int) nlen)
    return false;
  struct mom_hashedvalue_st *tokv = nodexp->nod_sons[pos];
  const struct mom_boxnode_st *toknod = mom_dyncast_node (tokv);
  if (!toknod || mom_raw_size (toknod) != 1)
    return false;
  if (toknod->nod_connitm != MOM_PREDEFITM (delimiter))
    return false;
  return (const void *) toknod->nod_sons[0] == (const void *) delimitm;
}                               /* end of isdelim_nanoedit_mom */



static const void *parsexprprec_nanoedit_mom (struct nanoparsing_mom_st *np,
                                              int prec, int *posptr);

static const void *parsexpr_nanoedit_mom (struct nanoparsing_mom_st *np,
                                          int *posptr);

static const void *
parsprimary_nanoedit_mom (struct nanoparsing_mom_st *np, int *posptr)
{
  assert (np && np->nanop_magic == NANOPARSING_MAGIC_MOM);
  const struct mom_boxnode_st *nodexp = np->nanop_nodexpr;
  assert (nodexp && nodexp->va_itype == MOMITY_NODE);
  assert (posptr != NULL);
  int pos = *posptr;
  unsigned nlen = mom_raw_size (nodexp);
  if (pos < 0 || pos >= (int) nlen)
    return NULL;
  struct mom_hashedvalue_st *curtokv = nodexp->nod_sons[pos];
  if (!curtokv)
    NANOPARSING_FAILURE_MOM (np, -pos, "(primary) no token at position #%d",
                             pos);
  switch (mom_itype (curtokv))
    {
    case MOMITY_BOXINT:
    case MOMITY_BOXDOUBLE:
    case MOMITY_BOXSTRING:
    case MOMITY_ITEM:
      {
        *posptr = pos + 1;
        MOM_DEBUGPRINTF (web, "parsprimary_nanoedit scalar %s at pos#%d",
                         mom_value_cstring (curtokv), pos);
        return curtokv;
      }
    case MOMITY_NODE:
      {
        int off = 0;
        const struct mom_boxnode_st *nodtok =
          (const struct mom_boxnode_st *) curtokv;
        if (nodtok->nod_metarank > 0)
          off = nodtok->nod_metarank;
        else
          off = -(pos + 1);
        MOM_DEBUGPRINTF (web,
                         "parsprimary_nanoedit composite token %s at position #%d",
                         mom_value_cstring (curtokv), pos);
        if (pos + 3 < (int) nlen
            && isdelim_nanoedit_mom (np, pos, MOM_PREDEFITM (percent_delim)))
          {
            struct mom_hashedvalue_st *next1tokv = nodexp->nod_sons[pos + 1];
            struct mom_hashedvalue_st *next2tokv = nodexp->nod_sons[pos + 2];
            struct mom_item_st *connitm = mom_dyncast_item (next1tokv);
            if (!connitm)
              NANOPARSING_FAILURE_WITH_MOM (np, off, next1tokv,
                                            "parsprimary_nanoedit unexpected connective %s",
                                            mom_value_cstring (next1tokv));
            if (!isdelim_nanoedit_mom
                (np, pos + 2, MOM_PREDEFITM (left_paren_delim)))
              NANOPARSING_FAILURE_WITH_MOM (np, off, next2tokv,
                                            "parsprimary_nanoedit expected left parenthesis got %s after connective %s",
                                            mom_value_cstring (next2tokv),
                                            mom_item_cstring (connitm));
            MOM_DEBUGPRINTF (web,
                             "parsprimary_nanoedit percent-expr pos#%d off@%d connitm %s",
                             pos, off, mom_item_cstring (connitm));
            struct mom_vectvaldata_st *vec =
              mom_vectvaldata_reserve (NULL, 5);
            assert (vec != NULL);
            int curpos = pos + 3;
            while (curpos < (int) nlen
                   && !isdelim_nanoedit_mom (np, curpos,
                                             MOM_PREDEFITM
                                             (right_paren_delim)))
              {
                int prevpos = curpos;
                const void *subexpv = parsexpr_nanoedit_mom (np, &curpos);
                if (!subexpv && curpos == prevpos)
                  NANOPARSING_FAILURE_WITH_MOM (np, off, next2tokv,
                                                "parsprimary_nanoedit bad subexpression#%d in percent-node of connective %s",
                                                mom_vectvaldata_count (vec),
                                                mom_item_cstring (connitm));

                vec = mom_vectvaldata_append (vec, subexpv);
                if (isdelim_nanoedit_mom
                    (np, curpos, MOM_PREDEFITM (comma_delim)))
                  curpos++;
                else if (isdelim_nanoedit_mom (np, curpos,
                                               MOM_PREDEFITM
                                               (right_paren_delim)))
                  break;
              }
          }
#warning incomplete code parsprimary_nanoedit
      }
    default:
      break;
    }
  NANOPARSING_FAILURE_MOM (np, -pos,
                           "parsprimary_nanoedit unexpected token %s",
                           mom_value_cstring (curtokv));
}                               /* end parsprimary_nanoedit_mom */

// precedence 1 is the highest, precedence 15 is the lowest
// e.g. http://en.cppreference.com/w/c/language/operator_precedence 
#define NANOEDIT_MAX_PRECEDENCE_MOM 16
static const void *
parsexprprec_nanoedit_mom (struct nanoparsing_mom_st *np, int prec,
                           int *posptr)
{
  assert (np && np->nanop_magic == NANOPARSING_MAGIC_MOM);
  const struct mom_boxnode_st *nodexp = np->nanop_nodexpr;
  assert (nodexp && nodexp->va_itype == MOMITY_NODE);
  assert (posptr != NULL);
  assert (prec >= 0 && prec < NANOEDIT_MAX_PRECEDENCE_MOM);
  if (prec == 0)
    return parsprimary_nanoedit_mom (np, posptr);
  int startpos = *posptr;
  unsigned nlen = mom_raw_size (nodexp);
  if (startpos < 0 || startpos >= (int) nlen)
    return NULL;
  struct mom_hashedvalue_st *curtokv = nodexp->nod_sons[startpos];
  if (!curtokv)
    NANOPARSING_FAILURE_MOM (np, -startpos,
                             "(expr prec#%d) no token at position #%d", prec,
                             startpos);
  if (prec == 0)
    return parsprimary_nanoedit_mom (np, posptr);
#warning unimplemented parsexprprec_nanoedit_mom
  NANOPARSING_FAILURE_MOM (np, -startpos,
                           "unimplemented parsexprprec_nanoedit prec%d",
                           prec);
}                               /* end parsexprprec_nanoedit_mom */


static const void *
parsexpr_nanoedit_mom (struct nanoparsing_mom_st *np, int *posptr)
{
  return parsexprprec_nanoedit_mom (np, NANOEDIT_MAX_PRECEDENCE_MOM, posptr);
}

static void
doparsecommand_nanoedit_mom (struct mom_webexch_st *wexch,
                             struct mom_item_st *tkitm,
                             struct mom_item_st *wexitm,
                             struct mom_item_st *thistatitm, const char *cmd)
{
  struct mom_item_st *sessitm = wexch->webx_sessitm;
  struct mom_queue_st *que = NULL;
  MOM_DEBUGPRINTF (web,
                   "doparsecommand_nanoedit webr#%ld tkitm=%s wexitm=%s thistatitm=%s sessitm=%s cmd=%s",
                   wexch->webx_count, mom_item_cstring (tkitm),
                   mom_item_cstring (wexitm),
                   mom_item_cstring (thistatitm),
                   mom_item_cstring (sessitm), cmd);
  assert (tkitm && tkitm->va_itype == MOMITY_ITEM);
  const struct mom_boxnode_st *tknod = mom_dyncast_node (tkitm->itm_payload);
  assert (tknod != NULL && mom_raw_size (tknod) >= mec__last);
  struct mom_item_st *delimitm
    = mom_dyncast_item (tknod->nod_sons[mec_delimiters]);
  struct nanoparsing_mom_st npars;
  memset (&npars, 0, sizeof (npars));
  npars.nanop_magic = NANOPARSING_MAGIC_MOM;
  npars.nanop_pos = 0;
  npars.nanop_cmdstr = cmd;
  npars.nanop_cmdlen = strlen (cmd);
  npars.nanop_wexch = wexch;
  npars.nanop_tkitm = tkitm;
  npars.nanop_wexitm = wexitm;
  npars.nanop_thistatitm = thistatitm;
  int linerr = 0;
  // create the queue item
  {
    struct mom_item_st *itm = mom_clone_item (MOM_PREDEFITM (queue));
    que = mom_queue_make ();
    itm->itm_payload = (struct mom_anyvalue_st *) que;
    mom_item_lock (itm);
    npars.nanop_queitm = itm;
    MOM_DEBUGPRINTF (web, "doparsecommand_nanoedit queitm=%s",
                     mom_item_cstring (npars.nanop_queitm));
  }
  // retrieve the delim item
  {
    assert (delimitm != NULL && delimitm->va_itype == MOMITY_ITEM);
    MOM_DEBUGPRINTF (web, "doparsecommand_nanoedit delimitm=%s",
                     mom_item_cstring (delimitm));
    mom_item_lock (delimitm);
    assert (mom_hashassoc_dyncast (delimitm->itm_payload) != NULL);
    npars.nanop_delimitm = delimitm;
  }
  if ((linerr = setjmp (npars.nanop_jb)) != 0)
    {
      if (npars.nanop_pos >= 0)
        {
          if (npars.nanop_errval)
            MOM_WARNPRINTF
              ("doparsecommand_nanoedit parsing error from %s:%d with %s: %s"
               "\n.. at position %u of:\n%s\n", __FILE__, linerr,
               mom_value_cstring (npars.nanop_errval),
               mom_boxstring_cstr (npars.nanop_errmsgv), npars.nanop_pos,
               npars.nanop_cmdstr);
          else
            MOM_WARNPRINTF
              ("doparsecommand_nanoedit parsing error from %s:%d: %s"
               "\n.. at position %u of:\n%s\n", __FILE__, linerr,
               mom_boxstring_cstr (npars.nanop_errmsgv), npars.nanop_pos,
               npars.nanop_cmdstr);
        }
      else
        {
          if (npars.nanop_errval)
            MOM_WARNPRINTF
              ("doparsecommand_nanoedit parsing error from %s:%d with %s: %s"
               "\n.. at index %d of:\n%s\n", __FILE__, linerr,
               mom_value_cstring (npars.nanop_errval),
               mom_boxstring_cstr (npars.nanop_errmsgv), -npars.nanop_pos,
               npars.nanop_cmdstr);
          else
            MOM_WARNPRINTF
              ("doparsecommand_nanoedit parsing error from %s:%d: %s"
               "\n.. at index %d of:\n%s\n", __FILE__, linerr,
               mom_boxstring_cstr (npars.nanop_errmsgv), -npars.nanop_pos,
               npars.nanop_cmdstr);
        }
      const char *errhtml = NULL;
      const struct mom_boxstring_st *badnamstr = NULL;
      {
        const struct mom_boxnode_st *errnod = NULL;
        char *errbuf = NULL;
        size_t errsiz = 0;
        FILE *ferr = open_memstream (&errbuf, &errsiz);
        if (!ferr)
          MOM_FATAPRINTF ("failed to open memory stream for error %s - %m",
                          mom_boxstring_cstr (npars.nanop_errmsgv));
        fprintf (ferr, "<b>parsing error @%d</b>: <tt>", npars.nanop_pos);
        mom_output_utf8_html (ferr, mom_boxstring_cstr (npars.nanop_errmsgv),
                              -1, true);
        fputs ("</tt>", ferr);
        if ((errnod = mom_dyncast_node (npars.nanop_errval))
            && errnod->nod_connitm == MOM_PREDEFITM (name)
            && mom_raw_size (errnod) == 1
            && (badnamstr = mom_dyncast_boxstring (errnod->nod_sons[0])))
          {
            MOM_DEBUGPRINTF (web, "doparsecommand_nanoedit badnamstr=%s",
                             mom_boxstring_cstr (badnamstr));
          }
        fflush (ferr);
        errhtml = GC_STRDUP (errbuf);
        fclose (ferr);
        free (errbuf);
      }
      MOM_DEBUGPRINTF (web, "doparsecommand_nanoedit errhtml=%s", errhtml);
      mom_wexch_puts (wexch, "{ \"html\": \"");
      mom_output_utf8_encoded (wexch->webx_outfil, errhtml, -1);
      mom_wexch_puts (wexch, "\",\n");
      if (npars.nanop_pos > 0)
        MOM_WEXCH_PRINTF (wexch, " \"position\": %d,\n", npars.nanop_pos);
      if (badnamstr)
        MOM_WEXCH_PRINTF (wexch, " \"bad_name\": \"%s\",\n",
                          mom_boxstring_cstr (badnamstr));

      MOM_WEXCH_PRINTF (wexch, " \"error_from\": %d }\n", linerr);
      mom_wexch_reply (wexch, HTTP_OK, "application/json");
      goto end;
    }                           /* end error processing after longjmp */
  else                          /* no error, i.e. first time... */
    {
      const char *gend = NULL;
      if (!g_utf8_validate (cmd, -1, &gend))
        NANOPARSING_FAILURE_MOM (&npars, gend - cmd, "invalid UTF8: %s", cmd);
      while (cmd[npars.nanop_pos] != (char) 0)
        parse_token_nanoedit_mom (&npars);
      const struct mom_boxnode_st *lexqnod =
        mom_queue_node (mom_dyncast_queue (npars.nanop_queitm->itm_payload),
                        MOM_PREDEFITM (queue));
      MOM_DEBUGPRINTF (web, "doparsecommand_nanoedit lexqnod=%s",
                       mom_value_cstring ((struct mom_hashedvalue_st *)
                                          lexqnod));
      npars.nanop_nodexpr = lexqnod;
      int pos = 0;
      const void *exprv = parsexpr_nanoedit_mom (&npars, &pos);
      MOM_DEBUGPRINTF (web, "doparsecommand_nanoedit exprv=%s final pos#%d",
                       mom_value_cstring (exprv), pos);
#warning doparsecommand_nanoedit_mom unimplemented
    }
end:
  if (npars.nanop_queitm)
    mom_item_unlock (npars.nanop_queitm);
  if (npars.nanop_delimitm)
    mom_item_unlock (npars.nanop_delimitm);
  memset (&npars, 0, sizeof (npars));
}                               /* end of doparsecommand_nanoedit_mom */



/********************* end of file nanoedit.c **********************/
