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
showcontentitem_nanoedit_mom (struct mom_filebuffer_st *fb,
                              struct mom_item_st *wexitm,
                              struct mom_item_st *thistatitm,
                              const struct mom_item_st *curitm);


static void
showitem_nanoedit_mom (struct mom_filebuffer_st *fb,
                       struct mom_item_st *wexitm,
                       const struct mom_item_st *curitm, bool isval)
{
  struct mom_item_st *hsetitm = NULL;
  struct mom_item_st *sessitm = NULL;
  assert (fb && fb->va_itype == MOMITY_FILEBUFFER);
  struct mom_file_st *fil = (struct mom_file_st *) fb;
  assert (wexitm && wexitm->va_itype == MOMITY_ITEM);
  struct mom_webexch_st *wexch = mom_item_unsync_webexch (wexitm);
  assert (wexch && wexch->va_itype == MOMITY_WEBEXCH);
  sessitm = wexch->webx_sessitm;
  assert (sessitm && sessitm->va_itype == MOMITY_ITEM);
  MOM_DEBUGPRINTF (web,
                   "showitem_nanoedit wexitm=%s sessitm=%s hsetitm=%s curitm=%s",
                   mom_item_cstring (wexitm), mom_item_cstring (sessitm),
                   mom_item_cstring (hsetitm), mom_item_cstring (curitm));
  if (!curitm || curitm == MOM_EMPTY_SLOT)
    {
      if (isval)
        mom_file_puts (fil, "<span class='momnilval_cl'>~</span>");
      else
        mom_file_puts (fil, "<span class='momnilitem_cl'>~</span>");
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
      hsetitm->itm_payload = (struct mom_anyvalue_st *)
        mom_hashset_insert ((struct mom_hashset_st *) hsetitm->itm_payload,
                            (struct mom_item_st *) curitm);
      if (isval)
        mom_file_printf (fil,
                         "<span class='momitemval_cl'>%s</span>",
                         mom_item_cstring (curitm));
      else
        mom_file_printf (fil,
                         "<span class='momitemref_cl'>%s</span>",
                         mom_item_cstring (curitm));
    }
}                               // end showitem_nanoedit_mom


static void
newline_nanoedit_mom (struct mom_filebuffer_st *fb, int depth)
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
  mom_file_puts ((struct mom_file_st *) fb, buf);
}                               /* end of newline_nanoedit_mom */


static void
utf8escape_nanoedit_mom (FILE *f, gunichar uc, const char *cescstr,
                         void *clientdata)
{
  assert (f != NULL);
  assert (clientdata != NULL);
  switch (uc)
    {
    case '&':
      fputs ("&amp;", f);
      break;
    case '<':
      fputs ("&lt;", f);
      break;
    case '>':
      fputs ("&gt;", f);
      break;
    case '\'':
      fputs ("<span class='momcharesc_cl'>\\&apos;</span>", f);
      break;
    case '\"':
      fputs ("<span class='momcharesc_cl'>\\&quot;</span>", f);
      break;
    default:
      fprintf (f, "<span class='momcharesc_cl'>%s</span>", cescstr);
      break;
    }
}


static void
showvalue_nanoedit_mom (struct mom_filebuffer_st *fb,
                        struct mom_item_st *wexitm,
                        struct mom_item_st *thistatitm, const void *pval,
                        int depth)
{
  assert (fb && fb->va_itype == MOMITY_FILEBUFFER);
  struct mom_file_st *fil = (struct mom_file_st *) fb;
  MOM_DEBUGPRINTF (web, "showvalue_nanoedit depth#%d pval=%s", depth,
                   mom_value_cstring (pval));
  switch (mom_itype (pval))
    {
    case MOMITY_NONE:
      mom_file_puts (fil, "<span class='momnilval_cl'>~</span>");
      return;
    case MOMITY_BOXINT:
      mom_file_printf (fil, "<span class='momintval_cl'>%lld</span>",
                       (long long) ((const struct mom_boxint_st *)
                                    pval)->boxi_int);
      return;
    case MOMITY_BOXDOUBLE:
      {
        char buf[48];
        memset (buf, 0, sizeof (buf));
        double x = ((const struct mom_boxdouble_st *) pval)->boxd_dbl;
        mom_file_printf
          (fil, "<span class='momdblval_cl'>%s</span>",
           mom_double_to_cstr (x, buf, sizeof (buf)));
      }
      return;
    case MOMITY_ITEM:
      showitem_nanoedit_mom (fb, wexitm, (struct mom_item_st *) pval, true);
      return;
    case MOMITY_BOXSTRING:
      mom_file_puts (fil, "\"<span class='momstrval_cl' data-momstring='");
      mom_output_utf8_html (mom_file (fil),
                            ((const struct mom_boxstring_st *) pval)->cstr,
                            mom_size (pval), false);
      mom_file_puts (fil, "'>");
      mom_output_utf8_escaped (mom_file (fil),
                               ((const struct mom_boxstring_st *) pval)->cstr,
                               mom_size (pval), utf8escape_nanoedit_mom,
                               wexitm);
      mom_file_puts (fil, "</span>\"");
      return;
    case MOMITY_TUPLE:
      {
        const struct mom_boxtuple_st *tup = pval;
        unsigned siz = mom_size (tup);
        mom_file_puts (fil, " <span class='momtup_cl'>[");
        for (unsigned ix = 0; ix < siz; ix++)
          {
            if (ix > 0)
              mom_file_puts (fil, ",");
            if (ix % 4 == 0)
              newline_nanoedit_mom (fb, depth);
            else
              mom_file_puts (fil, " ");
            showitem_nanoedit_mom (fb, wexitm, tup->seqitem[ix], false);
          }
        mom_file_puts (fil, "]</span>");
      }
      return;
    case MOMITY_SET:
      {
        const struct mom_boxset_st *set = pval;
        unsigned siz = mom_size (set);
        mom_file_puts (fil, " <span class='momset_cl'>{");
        for (unsigned ix = 0; ix < siz; ix++)
          {
            if (ix > 0)
              mom_file_puts (fil, ",");
            if (ix % 4 == 0)
              newline_nanoedit_mom (fb, depth);
            else
              mom_file_puts (fil, " ");
            showitem_nanoedit_mom (fb, wexitm, set->seqitem[ix], false);
          }
        mom_file_puts (fil, "}</span>");
      }
      return;
    case MOMITY_NODE:
      {
        const struct mom_boxnode_st *nod = pval;
        unsigned siz = mom_size (nod);
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
            const struct mom_hashedvalue_st *conndisplayerv = NULL;
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
                (*disprout) (conndispnod, fb, wexitm, thistatitm, pval,
                             depth);
                MOM_DEBUGPRINTF (web,
                                 "showvalue_nanoedit done wexitm=%s connitm=%s dispitm=%s",
                                 mom_item_cstring (wexitm),
                                 mom_item_cstring (connitm),
                                 mom_item_cstring (dispitm));
                return;
              }
          }
        mom_file_puts (fil, " <span class='momnode_cl'>%");
        showitem_nanoedit_mom (fb, wexitm, nod->nod_connitm, false);
        mom_file_puts (fil, "(");
        for (unsigned ix = 0; ix < siz; ix++)
          {
            if (ix > 0)
              mom_file_puts (fil, ",");
            if (ix % 2 == 0)
              newline_nanoedit_mom (fb, depth);
            else
              mom_file_puts (fil, " ");
            showvalue_nanoedit_mom (fb, wexitm, thistatitm,
                                    nod->nod_sons[ix], depth + 1);
          };
        mom_file_puts (fil, ")</span>");
      }
      return;
    default:
      MOM_FATAPRINTF ("showvalue_nanoedit_mom incomplete pval:%s",
                      mom_value_cstring ((struct mom_hashedvalue_st *) pval));
      break;
    }
}                               /* end showvalue_nanoedit_mom */



static void
showassovaldata_nanoedit_mom (struct mom_filebuffer_st *fb,
                              struct mom_item_st *wexitm,
                              struct mom_item_st *thistatitm,
                              const struct mom_assovaldata_st *ass)
{
  assert (fb != NULL && fb != MOM_EMPTY_SLOT
          && fb->va_itype == MOMITY_FILEBUFFER);
  assert (wexitm != NULL && wexitm != MOM_EMPTY_SLOT
          && wexitm->va_itype == MOMITY_ITEM);
  assert (thistatitm != NULL && thistatitm != MOM_EMPTY_SLOT
          && thistatitm->va_itype == MOMITY_ITEM);
  assert (ass != NULL && ass != MOM_EMPTY_SLOT
          && ass->va_itype == MOMITY_ASSOVALDATA);
  const struct mom_boxset_st *setat = mom_assovaldata_set_attrs (ass);
  assert (setat != NULL && setat->va_itype == MOMITY_SET);
  unsigned nbat = mom_size (setat);
  mom_file_puts (fb, "<dl class='mom_assoval_cl'>\n");
  for (unsigned ix = 0; ix < nbat; ix++)
    {
      const struct mom_item_st *atitm = setat->seqitem[ix];
      assert (atitm && atitm->va_itype == MOMITY_SET);
      const struct mom_hashedvalue_st *curval =
        mom_assovaldata_get (ass, atitm);
      mom_file_puts (fb, "<dt class='mom_assocattr_cl'>");
      showitem_nanoedit_mom (fb, wexitm, atitm, false);
      mom_file_puts (fb, "</dt>\n");
      mom_file_puts (fb, "<dd class='mom_assocval_cl'>");
      showvalue_nanoedit_mom (fb, wexitm, thistatitm, curval, 0);
      mom_file_puts (fb, "</dd>");
    }
  mom_file_puts (fb, "</dl>\n");
}                               /* end of showassovaldata_nanoedit_mom */



static void
showvectvaldata_nanoedit_mom (struct mom_filebuffer_st *fb,
                              struct mom_item_st *wexitm,
                              struct mom_item_st *thistatitm,
                              struct mom_vectvaldata_st *vec)
{
  assert (fb != NULL && fb != MOM_EMPTY_SLOT
          && fb->va_itype == MOMITY_FILEBUFFER);
  assert (wexitm != NULL && wexitm != MOM_EMPTY_SLOT
          && wexitm->va_itype == MOMITY_ITEM);
  assert (thistatitm != NULL && thistatitm != MOM_EMPTY_SLOT
          && thistatitm->va_itype == MOMITY_ITEM);
  assert (vec != NULL && vec != MOM_EMPTY_SLOT
          && vec->va_itype == MOMITY_ASSOVALDATA);
  unsigned cnt = mom_vectvaldata_count (vec);
  mom_file_printf (fb, "<i>vector of %d</i>\n", cnt);
  mom_file_puts (fb, "<ol class='mom_vectorval_cl'>\n");
  for (unsigned ix = 0; ix < cnt; ix++)
    {
      struct mom_hashedvalue_st *compv = mom_vectvaldata_nth (vec, ix);
      mom_file_puts (fb, "<li>");
      showvalue_nanoedit_mom (fb, wexitm, thistatitm, compv, 0);
      mom_file_puts (fb, "</li>\n");
    }
  mom_file_puts (fb, "</ol>\n");
}                               /* end of showvectvaldata_nanoedit_mom */



static void
showqueue_nanoedit_mom (struct mom_filebuffer_st *fb,
                        struct mom_item_st *wexitm,
                        struct mom_item_st *thistatitm,
                        struct mom_queue_st *que)
{
  assert (fb != NULL && fb != MOM_EMPTY_SLOT
          && fb->va_itype == MOMITY_FILEBUFFER);
  assert (wexitm != NULL && wexitm != MOM_EMPTY_SLOT
          && wexitm->va_itype == MOMITY_ITEM);
  assert (thistatitm != NULL && thistatitm != MOM_EMPTY_SLOT
          && thistatitm->va_itype == MOMITY_ITEM);
  assert (que != NULL && que != MOM_EMPTY_SLOT
          && que->va_itype == MOMITY_QUEUE);
  const struct mom_boxnode_st *qnod
    = mom_queue_node (que, MOM_PREDEFITM (queue));
  assert (qnod != NULL && qnod->va_itype == MOMITY_NODE);
  unsigned qlen = mom_size (qnod);
  mom_file_puts (fb, "<ol class='mom_queueval_cl'>\n");
  for (unsigned ix = 0; ix < qlen; ix++)
    {
      struct mom_hashedvalue_st *compv = qnod->nod_sons[ix];
      mom_file_puts (fb, "<li>");
      showvalue_nanoedit_mom (fb, wexitm, thistatitm, compv, 0);
      mom_file_puts (fb, "</li>\n");
    }
  qnod = NULL;
  mom_file_puts (fb, "</ul>\n");
}                               /* end of showqueue_nanoedit_mom */



static void
showhashset_nanoedit_mom (struct mom_filebuffer_st *fb,
                          struct mom_item_st *wexitm,
                          struct mom_item_st *thistatitm,
                          struct mom_hashset_st *hset)
{
  assert (fb != NULL && fb != MOM_EMPTY_SLOT
          && fb->va_itype == MOMITY_FILEBUFFER);
  assert (wexitm != NULL && wexitm != MOM_EMPTY_SLOT
          && wexitm->va_itype == MOMITY_ITEM);
  assert (thistatitm != NULL && thistatitm != MOM_EMPTY_SLOT
          && thistatitm->va_itype == MOMITY_ITEM);
  assert (hset != NULL && hset != MOM_EMPTY_SLOT
          && hset->va_itype == MOMITY_HASHSET);
  const struct mom_boxset_st *bs = mom_hashset_to_boxset (hset);
  assert (bs != NULL && bs->va_itype == MOMITY_SET);
  showvalue_nanoedit_mom (fb, wexitm, thistatitm, bs, 0);
}                               /* end of showhashset_nanoedit_mom */



static void
showhashmap_nanoedit_mom (struct mom_filebuffer_st *fb,
                          struct mom_item_st *wexitm,
                          struct mom_item_st *thistatitm,
                          struct mom_hashmap_st *hmap)
{
  assert (fb != NULL && fb != MOM_EMPTY_SLOT
          && fb->va_itype == MOMITY_FILEBUFFER);
  assert (wexitm != NULL && wexitm != MOM_EMPTY_SLOT
          && wexitm->va_itype == MOMITY_ITEM);
  assert (thistatitm != NULL && thistatitm != MOM_EMPTY_SLOT
          && thistatitm->va_itype == MOMITY_ITEM);
  assert (hmap != NULL && hmap != MOM_EMPTY_SLOT
          && hmap->va_itype == MOMITY_HASHMAP);
  const struct mom_boxset_st *ks = mom_hashmap_keyset (hmap);
  assert (ks != NULL && ks->va_itype == MOMITY_SET);
  mom_file_puts (fb, "<dl class='mom_hashmap_cl'>\n");
  unsigned cnt = mom_size (ks);
  for (unsigned ix = 0; ix < cnt; ix++)
    {
      const struct mom_item_st *keyitm = ks->seqitem[ix];
      assert (keyitm && keyitm->va_itype == MOMITY_ITEM);
      const struct mom_hashedvalue_st *val = mom_hashmap_get (hmap, keyitm);
      mom_file_printf (fb, "<dt class='mom_hashkeyitem_cl'>%s</dt>\n",
                       mom_item_cstring (keyitm));
      mom_file_puts (fb, "<dd class='mom_hashval_cl'>");
      showvalue_nanoedit_mom (fb, wexitm, thistatitm, val, 0);
      mom_file_puts (fb, "</dd>\n");
    }
  mom_file_puts (fb, "</dl>\n");
}                               /* end of showhashmap_nanoedit_mom */


static void
showhashassoc_nanoedit_mom (struct mom_filebuffer_st *fb,
                            struct mom_item_st *wexitm,
                            struct mom_item_st *thistatitm,
                            struct mom_hashassoc_st *hass)
{
  assert (fb != NULL && fb != MOM_EMPTY_SLOT
          && fb->va_itype == MOMITY_FILEBUFFER);
  assert (wexitm != NULL && wexitm != MOM_EMPTY_SLOT
          && wexitm->va_itype == MOMITY_ITEM);
  assert (thistatitm != NULL && thistatitm != MOM_EMPTY_SLOT
          && thistatitm->va_itype == MOMITY_ITEM);
  assert (hass != NULL && hass != MOM_EMPTY_SLOT
          && hass->va_itype == MOMITY_HASHASSOC);
  const struct mom_boxnode_st *keynod =
    mom_hashassoc_sorted_key_node (hass, MOM_PREDEFITM (sequence));
  assert (keynod != NULL && keynod->va_itype == MOMITY_NODE);
  unsigned nbkeys = mom_size (keynod);
  mom_file_puts (fb, "<dl class='mom_hashassoc_cl'>\n");
  for (unsigned ix = 0; ix < nbkeys; ix++)
    {
      const struct mom_hashedvalue_st *keyv = keynod->nod_sons[ix];
      const struct mom_hashedvalue_st *valv = mom_hashassoc_get (hass, keyv);
      mom_file_puts (fb, "<dt class='mom_hashassockey_cl'>");
      showvalue_nanoedit_mom (fb, wexitm, thistatitm, keyv, 0);
      mom_file_puts (fb, "</dt>\n");
      mom_file_puts (fb, "<dd class='mom_hashassocval_cl'>");
      showvalue_nanoedit_mom (fb, wexitm, thistatitm, valv, 0);
      mom_file_puts (fb, "</dd>\n");
    };
  mom_file_puts (fb, "</dl>\n");
}                               /* end of showhashassoc_nanoedit_mom */



static void
showcontentitem_nanoedit_mom (struct mom_filebuffer_st *fb,
                              struct mom_item_st *wexitm,
                              struct mom_item_st *thistatitm,
                              const struct mom_item_st *curitm)
{
  MOM_DEBUGPRINTF (web,
                   "showcontentitem_nanoedit wexitm=%s thistatitm=%s curitm=%s",
                   mom_item_cstring (wexitm), mom_item_cstring (thistatitm),
                   mom_item_cstring (curitm));
  assert (fb != NULL && fb != MOM_EMPTY_SLOT
          && fb->va_itype == MOMITY_FILEBUFFER);
  assert (wexitm != NULL && wexitm != MOM_EMPTY_SLOT
          && wexitm->va_itype == MOMITY_ITEM);
  assert (thistatitm != NULL && thistatitm != MOM_EMPTY_SLOT
          && thistatitm->va_itype == MOMITY_ITEM);
  assert (curitm != NULL && curitm != MOM_EMPTY_SLOT
          && curitm->va_itype == MOMITY_ITEM);
  mom_file_printf (fb,
                   "<div id='showcontentitem_%s_id' name='%s' class='mom_showcontentitem_cl'>\n",
                   mom_item_cstring (curitm), mom_item_cstring (curitm));
  mom_item_lock ((struct mom_item_st *) curitm);
  {
    mom_file_printf (fb,
                     "<p class='showcontent_title_cl' data-contentitem='%s'> * ",
                     mom_item_cstring (curitm));
    showitem_nanoedit_mom (fb, wexitm, curitm, false);
    {
      char timbuf[64];
      memset (timbuf, 0, sizeof (timbuf));
      mom_file_printf (fb,
                       "<span class='itemupdated_cl'>updated on <i>%s</i></span>",
                       mom_strftime_centi (timbuf, sizeof (timbuf), "%c %Z",
                                           (double) curitm->itm_mtime));
    }
    mom_file_printf (fb,
                     " &nbsp; <button class='buthideitem_cl' data-hideitem='%s'>hide</button>",
                     mom_item_cstring (curitm));
    mom_file_puts (fb, " * </p>\n");
    const struct mom_boxset_st *attset =
      mom_unsync_item_phys_set_attrs (curitm);
    unsigned nbattrs = mom_boxset_length (attset);
    MOM_DEBUGPRINTF (web,
                     "showcontent_nanoedit curitm=%s nbattrs#%d attset=%s",
                     mom_item_cstring (curitm), nbattrs,
                     mom_value_cstring ((struct mom_hashedvalue_st *)
                                        attset));
    if (nbattrs > 0)
      {
        mom_file_printf (fb,
                         "<p class='showcontent_attributes_cl' data-contentitem='%s'><span class='infocount_cl'>%d attributes.</span><br/>\n",
                         mom_item_cstring (curitm), nbattrs);
        mom_file_printf (fb,
                         "<dl class='showcontent_attributelist_cl' data-contentitem='%s'>\n",
                         mom_item_cstring (curitm));
        for (unsigned atix = 0; atix < nbattrs; atix++)
          {
            const struct mom_item_st *curatitm =
              mom_boxset_nth (attset, atix);
            const struct mom_hashedvalue_st *curval =
              mom_unsync_item_get_phys_attr (curitm, curatitm);
            MOM_DEBUGPRINTF (web,
                             "showcontent_nanoedit atix#%d curatitm=%s curval=%s",
                             atix, mom_item_cstring (curatitm),
                             mom_value_cstring (curval));
            mom_file_puts (fb, "<dt class='momattitem_cl'>");
            showitem_nanoedit_mom (fb, wexitm, curatitm, false);
            mom_file_puts (fb, "</dt>\n");
            mom_file_printf (fb, "<dd class='momattvalue_cl'>\n");
            showvalue_nanoedit_mom (fb, wexitm, thistatitm, curval, 0);
            mom_file_puts (fb, "</dd>\n");
          }
        mom_file_puts (fb, "</dl></p>\n");
      };
    unsigned nbcomp = mom_vectvaldata_count (curitm->itm_pcomp);
    MOM_DEBUGPRINTF (web,
                     "showcontent_nanoedit curitm=%s nbcomp=%d",
                     mom_item_cstring (curitm), nbcomp);
    if (nbcomp > 0)
      {
        mom_file_printf (fb,
                         "<p class='showcontent_componentslist_cl' data-contentitem='%s'>"
                         "<span class='infocount_cl'>%d components.</span><br/>\n",
                         mom_item_cstring (curitm), nbcomp);
        mom_file_printf (fb,
                         "<ul class='showcontent_components_cl' data-contentitem='%s'>\n",
                         mom_item_cstring (curitm));
        for (unsigned compix = 0; compix < nbcomp; compix++)
          {
            const struct mom_hashedvalue_st *curcomp =
              mom_vectvaldata_nth (curitm->itm_pcomp, compix);
            MOM_DEBUGPRINTF (web,
                             "showcontent_nanoedit curitm=%s compix#%d curcomp=%s",
                             mom_item_cstring (curitm), compix,
                             mom_value_cstring (curcomp));
            mom_file_puts (fb, "<li> ");
            showvalue_nanoedit_mom (fb, wexitm, thistatitm, curcomp, 0);
            mom_file_puts (fb, "</li>\n");
          }
        mom_file_puts (fb, "</ul>\n");
      }
    unsigned typayl = mom_itype (curitm->itm_payload);
    MOM_DEBUGPRINTF (web, "showcontent_nanoedit curitm %s typayl#%d (%s)",
                     mom_item_cstring (curitm), typayl,
                     mom_itype_str (curitm->itm_payload));
    if (typayl > 0)
      {
        mom_file_printf (fb,
                         "<div class='showcontent_payload_cl'  data-contentitem='%s'>\n",
                         mom_item_cstring (curitm));
        mom_file_printf (fb, "payload %s<br/>",
                         mom_itype_str (curitm->itm_payload));
        switch (typayl)
          {
          case MOMITY_BOXINT:
          case MOMITY_BOXDOUBLE:
          case MOMITY_BOXSTRING:
          case MOMITY_ITEM:
          case MOMITY_TUPLE:
          case MOMITY_SET:
          case MOMITY_NODE:
            showvalue_nanoedit_mom (fb, wexitm, thistatitm,
                                    curitm->itm_payload, 0);
            break;
          case MOMITY_ASSOVALDATA:
            showassovaldata_nanoedit_mom (fb, wexitm, thistatitm,
                                          (struct mom_assovaldata_st *)
                                          curitm->itm_payload);
            break;
          case MOMITY_VECTVALDATA:
            showvectvaldata_nanoedit_mom (fb, wexitm, thistatitm,
                                          (struct mom_vectvaldata_st *)
                                          curitm->itm_payload);
            break;
          case MOMITY_QUEUE:
            showqueue_nanoedit_mom (fb, wexitm, thistatitm,
                                    (struct mom_queue_st *)
                                    curitm->itm_payload);
            break;
          case MOMITY_HASHSET:
            showhashset_nanoedit_mom (fb, wexitm, thistatitm,
                                      (struct mom_hashset_st *)
                                      curitm->itm_payload);
            break;
          case MOMITY_HASHMAP:
            showhashmap_nanoedit_mom (fb, wexitm, thistatitm,
                                      (struct mom_hashmap_st *)
                                      curitm->itm_payload);
            break;
          case MOMITY_HASHASSOC:
            showhashassoc_nanoedit_mom (fb, wexitm, thistatitm,
                                        (struct mom_hashassoc_st *)
                                        curitm->itm_payload);
            break;
          }
#warning showcontentitem_nanoedit incomplete for payload
        mom_file_puts (fb, "</div>\n");
      }                         // end payload show
    if (curitm->itm_funptr && curitm->itm_funsig)
      {
        Dl_info dli = { };
        mom_file_printf (fb,
                         "<div class='mom_showcontent_function_cl' data-contentitem='%s'>\n",
                         mom_item_cstring (curitm));
        if (dladdr (curitm->itm_funptr, &dli))
          {
            if (dli.dli_saddr == curitm->itm_funptr)
              mom_file_printf (fb, "function <tt>%s</tt> @%p,",
                               dli.dli_sname, curitm->itm_funptr);
            else
              mom_file_printf (fb, "function <tt>%s</tt><i>+%ld</i> @%p,",
                               dli.dli_sname,
                               (long) ((char *) curitm->itm_funptr -
                                       (char *) dli.dli_saddr),
                               curitm->itm_funptr);
          }
        else
          mom_file_printf (fb, "function @%p,", curitm->itm_funptr);
        mom_file_puts (fb, " of signature ");
        showitem_nanoedit_mom (fb, wexitm, curitm->itm_funsig, false);
        mom_file_puts (fb, "</div>\n");
      }
  }
  mom_item_unlock ((struct mom_item_st *) curitm);
  mom_file_puts (fb, "</div>\n");
}                               /* end of showcontentitem_nanoedit_mom */


const char momsig_nano_displayer[] = "signature_displayer";
void
momf_nano_displayer (const struct mom_boxnode_st
                     *closnod,
                     struct mom_filebuffer_st
                     *fb,
                     struct mom_item_st *wexitm,
                     struct mom_item_st
                     *thistatitm, const void *pval, int depth)
{
  assert (closnod != NULL);
  MOM_DEBUGPRINTF (web,
                   "nano_displayer closnod=%s wexitm=%s thistatitm=%s depth#%d pval=%s",
                   mom_value_cstring ((void *)
                                      closnod),
                   mom_item_cstring (wexitm),
                   mom_item_cstring (thistatitm),
                   depth, mom_value_cstring (pval));
  showvalue_nanoedit_mom (fb, wexitm, thistatitm, pval, depth);
}                               /* end of momf_nano_displayer */


static void
dofillpage_nanoedit_mom (struct mom_webexch_st
                         *wexch,
                         struct mom_item_st *tkitm,
                         struct mom_item_st *wexitm,
                         struct mom_item_st *thistatitm)
{
  struct mom_item_st *sessitm = wexch->webx_sessitm;
  const char *rawmode = onion_request_get_post (wexch->webx_requ, "rawmode");
  MOM_DEBUGPRINTF (web,
                   "dofillpage_nanoedit webr#%ld tkitm=%s wexitm=%s thistatitm=%s sessitm=%s rawmode=%s",
                   wexch->webx_count,
                   mom_item_cstring (tkitm),
                   mom_item_cstring (wexitm),
                   mom_item_cstring (thistatitm),
                   mom_item_cstring (sessitm), rawmode);
  bool israw = false;
  if (rawmode)
    {
      israw = (!strcmp (rawmode, "true") || !strcmp (rawmode, "1"));
      mom_unsync_item_put_phys_attr (thistatitm,
                                     MOM_PREDEFITM
                                     (display),
                                     israw ?
                                     MOM_PREDEFITM (raw) :
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
                       wexch->webx_count,
                       mom_item_cstring (thistatitm),
                       mom_item_cstring (moditm), israw ? "raw" : "cooked");
    }
  char modbuf[64];
  memset (modbuf, 0, sizeof (modbuf));
  MOM_WEXCH_PRINTF (wexch,
                    "<h3>%s state <tt>%s</tt> on <i>%s</i></h3>\n",
                    israw ? "raw" : "cooked",
                    mom_item_cstring (thistatitm),
                    mom_strftime_centi (modbuf,
                                        sizeof (modbuf)
                                        - 1, "%c %Z", thistatitm->itm_mtime));
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
                  " raw display</p>\n" :
                  "<p id='momrawpara_id'>"
                  "<input type='checkbox' id='momrawbox_id' name='mode' value='raw'/>"
                  " raw display</p>\n");
  struct mom_hashmap_st *hmap = mom_hashmap_dyncast (thistatitm->itm_payload);
  const struct mom_hashedvalue_st *dispitemv =
    mom_unsync_item_get_phys_attr (thistatitm,
                                   MOM_PREDEFITM (item));
  const struct mom_boxset_st *dispset = mom_dyncast_set (dispitemv);
  const struct mom_boxset_st *atset = mom_hashmap_keyset (hmap);
  MOM_DEBUGPRINTF (web,
                   "dofillpage_nanoedit webr#%ld atset %s dispv %s",
                   wexch->webx_count,
                   mom_value_cstring ((struct
                                       mom_hashedvalue_st
                                       *) atset),
                   mom_value_cstring (dispitemv));
  unsigned nbat = mom_size (atset);
  if (nbat > 0)
    {
      mom_wexch_puts (wexch, "<dl class='momlocbind_cl'>\n");
      for (unsigned ix = 0; ix < nbat; ix++)
        {
          const struct mom_item_st *curatitm = atset->seqitem[ix];
          const struct mom_hashedvalue_st *curval =
            mom_hashmap_get (hmap, curatitm);
          struct mom_filebuffer_st *fb = mom_make_filebuffer ();
          MOM_DEBUGPRINTF (web,
                           "dofillpage_nanoedit webr#%ld ix%d curatitm %s curval %s",
                           wexch->webx_count, ix,
                           mom_item_cstring (curatitm),
                           mom_value_cstring (curval));
          assert (fb != NULL);
          MOM_WEXCH_PRINTF (wexch,
                            "<dt class='momlocvaritem_cl'>%s</dt>\n",
                            mom_item_cstring (curatitm));
          mom_wexch_puts (wexch, "<dd class='momlocvalue_cl'>\n");
          showvalue_nanoedit_mom (fb, wexitm, thistatitm, curval, 0);
          mom_puts_filebuffer (wexch->webx_outfil, fb, MOM_FILEBUFFER_CLOSE);
          mom_wexch_puts (wexch, "</dd>\n");
        }
      mom_wexch_puts (wexch, "</dl>\n");
    }
  unsigned nbdisp = mom_size (dispset);
  MOM_DEBUGPRINTF (web,
                   "dofillpage_nanoedit webr#%ld nbdisp=%d dispset %s",
                   wexch->webx_count, nbdisp,
                   mom_value_cstring ((struct mom_hashedvalue_st *) dispset));
  if (nbdisp > 0)
    {
      for (unsigned ix = 0; ix < nbdisp; ix++)
        {
          const struct mom_item_st *curdispitm = dispset->seqitem[ix];
          MOM_DEBUGPRINTF (web,
                           "dofillpage_nanoedit webr#%ld ix%d curdispitm %s",
                           wexch->webx_count, ix,
                           mom_item_cstring (curdispitm));
          struct mom_filebuffer_st *fb = mom_make_filebuffer ();
          assert (fb != NULL);
          showcontentitem_nanoedit_mom (fb, wexitm, thistatitm, curdispitm);
          MOM_WEXCH_PRINTF (wexch,
                            "<div class='mom_itemdisplaycontent_cl' data-dispitem='%s' data-ix='%d'>\n",
                            mom_item_cstring (curdispitm), ix);
          mom_puts_filebuffer (wexch->webx_outfil, fb, MOM_FILEBUFFER_CLOSE);
          mom_wexch_puts (wexch, "</div>\n");
        }
    }
  mom_wexch_reply (wexch, HTTP_OK, "text/html");
  MOM_DEBUGPRINTF (web, "dofillpage_nanoedit done webr#%ld tkitm=%s",
                   wexch->webx_count, mom_item_cstring (tkitm));
}                               /* end of dofillpage_nanoedit_mom */



static void
doparsecommand_nanoedit_mom (struct mom_webexch_st
                             *wexch,
                             struct mom_item_st
                             *tkitm,
                             struct mom_item_st
                             *wexitm,
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
                   wexch->webx_count,
                   mom_item_cstring (tkitm),
                   mom_item_cstring (wexitm),
                   mom_item_cstring (thistatitm),
                   mom_item_cstring (sessitm), name);
  if (name)
    {
      struct mom_item_st *itm = mom_find_item_by_string (name);
      MOM_DEBUGPRINTF (web, "doknownitem_nanoedit webr#%ld name=%s itm=%s\n",
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
docompletename_nanoedit_mom (struct mom_webexch_st
                             *wexch,
                             struct mom_item_st
                             *tkitm,
                             struct mom_item_st
                             *wexitm,
                             struct mom_item_st *thistatitm, const char *name)
{
  struct mom_item_st *sessitm = wexch->webx_sessitm;
  MOM_DEBUGPRINTF (web,
                   "docompletename_nanoedit webr#%ld tkitm=%s wexitm=%s thistatitm=%s sessitm=%s name=%s",
                   wexch->webx_count,
                   mom_item_cstring (tkitm),
                   mom_item_cstring (wexitm),
                   mom_item_cstring (thistatitm),
                   mom_item_cstring (sessitm), name);
  const struct mom_boxset_st *set = mom_set_items_prefixed (name, -1);
  MOM_DEBUGPRINTF (web,
                   "docompletename_nanoedit webr#%ld set %s",
                   wexch->webx_count,
                   mom_value_cstring ((const struct
                                       mom_hashedvalue_st *) set));
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
  MOM_DEBUGPRINTF (web, "docompletename_nanoedit webr#%ld done",
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
                   wexch->webx_count,
                   mom_item_cstring (tkitm),
                   mom_item_cstring (wexitm),
                   mom_item_cstring (thistatitm),
                   mom_item_cstring (sessitm), doexit);
  json_t *jreply = json_pack ("{s:f,s:f}", "elapsedreal",
                              mom_elapsed_real_time (), "processcpu",
                              mom_process_cpu_time ());;
  // json_dumps will use GC_STRDUP...
  MOM_DEBUGPRINTF (web, "doexit_nanoedit jreply=%s",
                   json_dumps (jreply, JSON_INDENT (1)));
  mom_wexch_puts (wexch, json_dumps (jreply, JSON_INDENT (1)));
  mom_wexch_reply (wexch, HTTP_OK, "application/json");
  mom_stop_and_dump ();
}                               /* end of doexit_nanoedit_mom */


static void
docreateitem_nanoedit_mom (struct mom_webexch_st
                           *wexch,
                           struct mom_item_st *tkitm,
                           struct mom_item_st
                           *wexitm,
                           struct mom_item_st
                           *thistatitm,
                           struct mom_item_st
                           *sessitm,
                           const char *itemnamestr, const char *commentstr)
{
  struct mom_item_st *hsetitm = NULL;
  struct mom_item_st *newitm = NULL;
  MOM_DEBUGPRINTF (web,
                   "docreateitem_nanoedit tkitm=%s wexitm=%s"
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
      MOM_WARNPRINTF
        ("docreateitem_nanoedit failed to create item named %s", itemnamestr);
      MOM_WEXCH_PRINTF (wexch,
                        "<b>failed to create item <tt>%s</tt></b>\n",
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
      hsetitm->itm_payload =
        (struct mom_anyvalue_st *)
        mom_hashset_insert ((struct mom_hashset_st *)
                            hsetitm->itm_payload,
                            (struct mom_item_st *) newitm);
      MOM_WEXCH_PRINTF (wexch,
                        "<b>created item <tt>%s</tt></b> on <i>%s</i>\n",
                        mom_item_cstring (newitm), timbuf);
      mom_wexch_reply (wexch, HTTP_OK, "text/html");
      return;
    }
}                               /* end of docreateitem_nanoedit_mom */


static void
dohideitem_nanoedit_mom (struct mom_webexch_st *wexch,
                         struct mom_item_st *tkitm,
                         struct mom_item_st *wexitm,
                         struct mom_item_st *thistatitm,
                         const char *dohideitem)
{
  MOM_DEBUGPRINTF (web,
                   "dohideitem_nanoedit tkitm=%s wexitm=%s thistatitm=%s dohideitem=%s",
                   mom_item_cstring (tkitm), mom_item_cstring (wexitm),
                   mom_item_cstring (thistatitm), dohideitem);
  struct mom_item_st *hiditm = mom_find_item_by_string (dohideitem);
  MOM_DEBUGPRINTF (web,
                   "dohideitem_nanoedit tkitm=%s hiditm=%s",
                   mom_item_cstring (tkitm), mom_item_cstring (hiditm));
  if (!hiditm)
    {
      MOM_WARNPRINTF ("dohideitem_nanoedit tkitm=%s missing item to hide %s",
                      mom_item_cstring (tkitm), dohideitem);
      MOM_WEXCH_PRINTF (wexch,
                        "<b>missing</b> item <tt>%s</tt> to hide\n",
                        dohideitem);
      mom_wexch_reply (wexch, HTTP_OK, "text/html");
      return;
    }
  const struct mom_hashedvalue_st *dispitemv =
    mom_unsync_item_get_phys_attr (thistatitm,
                                   MOM_PREDEFITM (item));
  MOM_DEBUGPRINTF (web, "dohideitem dispitemv=%s",
                   mom_value_cstring (dispitemv));
  const struct mom_boxset_st *dispset = mom_dyncast_set (dispitemv);
  const struct mom_boxset_st *hidsingleset = mom_boxset_make_va (1, hiditm);
  const struct mom_boxset_st *newdispset =
    mom_boxset_difference (dispset, hidsingleset);
  MOM_DEBUGPRINTF (web, "dohideitem dispset=%s hidsingleset=%s newdispset=%s",
                   mom_value_cstring ((struct mom_hashedvalue_st *) dispset),
                   mom_value_cstring ((struct mom_hashedvalue_st *)
                                      hidsingleset),
                   mom_value_cstring ((struct mom_hashedvalue_st *)
                                      newdispset));
  mom_unsync_item_put_phys_attr (thistatitm, MOM_PREDEFITM (item),
                                 newdispset);
  struct mom_filebuffer_st *fb = mom_make_filebuffer ();
  assert (fb && fb->va_itype == MOMITY_FILEBUFFER);
  mom_file_puts (fb, "<i>still shown:</i> ");
  showvalue_nanoedit_mom (fb, wexitm, thistatitm, newdispset, 0);
  mom_puts_filebuffer (wexch->webx_outfil, fb, MOM_FILEBUFFER_CLOSE);
  mom_wexch_reply (wexch, HTTP_OK, "text/html");
}                               /* end dohideitem_nanoedit_mom */


static void
dodispitem_nanoedit_mom (struct mom_webexch_st *wexch,
                         struct mom_item_st *tkitm,
                         struct mom_item_st *wexitm,
                         struct mom_item_st *thistatitm,
                         const char *dodispitem)
{
  MOM_DEBUGPRINTF (web,
                   "dodispitem_nanoedit tkitm=%s wexitm=%s thistatitm=%s dodispitem=%s",
                   mom_item_cstring (tkitm), mom_item_cstring (wexitm),
                   mom_item_cstring (thistatitm), dodispitem);
  struct mom_item_st *dispitm = mom_find_item_by_string (dodispitem);
  MOM_DEBUGPRINTF (web,
                   "dodispitem_nanoedit tkitm=%s dispitm=%s",
                   mom_item_cstring (tkitm), mom_item_cstring (dispitm));
  if (!dispitm)
    {
      MOM_WARNPRINTF
        ("dodispitem_nanoedit tkitm=%s missing item to display %s",
         mom_item_cstring (tkitm), dodispitem);
      MOM_WEXCH_PRINTF (wexch, "<b>missing</b> item <tt>%s</tt> to display\n",
                        dodispitem);
      mom_wexch_reply (wexch, HTTP_OK, "text/html");
      return;
    }
  const struct mom_hashedvalue_st *dispitemv =
    mom_unsync_item_get_phys_attr (thistatitm,
                                   MOM_PREDEFITM (item));
  MOM_DEBUGPRINTF (web, "dodispitem dispitemv=%s",
                   mom_value_cstring (dispitemv));
  const struct mom_boxset_st *dispset = mom_dyncast_set (dispitemv);
  const struct mom_boxset_st *singleset = mom_boxset_make_va (1, dispitm);
  const struct mom_boxset_st *newdispset =
    mom_boxset_union (dispset, singleset);
  MOM_DEBUGPRINTF (web, "dodispitem dispset=%s singleset=%s newdispset=%s",
                   mom_value_cstring ((struct mom_hashedvalue_st *) dispset),
                   mom_value_cstring ((struct mom_hashedvalue_st *)
                                      singleset),
                   mom_value_cstring ((struct mom_hashedvalue_st *)
                                      newdispset));
  mom_unsync_item_put_phys_attr (thistatitm, MOM_PREDEFITM (item),
                                 newdispset);
  struct mom_filebuffer_st *fb = mom_make_filebuffer ();
  assert (fb && fb->va_itype == MOMITY_FILEBUFFER);
  mom_file_puts (fb, "<i>now shown:</i> ");
  showvalue_nanoedit_mom (fb, wexitm, thistatitm, newdispset, 0);
  mom_puts_filebuffer (wexch->webx_outfil, fb, MOM_FILEBUFFER_CLOSE);
  mom_wexch_reply (wexch, HTTP_OK, "text/html");
}                               /* end of dodispitem_nanoedit_mom */



static void
doeval_nanoedit_mom (struct mom_webexch_st *wexch,
                     struct mom_item_st *tkitm,
                     struct mom_item_st *wexitm,
                     struct mom_item_st *thistatitm,
                     struct mom_item_st *sessitm, const char *doeval)
{
  long maxstep = MOM_IS_DEBUGGING (run) ? 2097152 : 16777216;
  MOM_DEBUGPRINTF (web,
                   "doeval_nanoedit start tkitm=%s wexitm=%s thistatitm=%s"
                   " sessitm=%s doeval=%s maxstep=%ld",
                   mom_item_cstring (tkitm),
                   mom_item_cstring (wexitm),
                   mom_item_cstring (thistatitm),
                   mom_item_cstring (sessitm), doeval, maxstep);
  assert (doeval != NULL);
  assert (thistatitm != NULL
          && thistatitm != MOM_EMPTY_SLOT
          && thistatitm->va_itype == MOMITY_ITEM);
  if (strcmp (doeval, mom_item_cstring (thistatitm)))
    MOM_FATAPRINTF
      ("doeval_nanoedit tkitm=%s doeval=%s different of thistatitm=%s",
       mom_item_cstring (tkitm), doeval, mom_item_cstring (thistatitm));
  const struct mom_hashedvalue_st *curexpv = NULL;
  {
    mom_item_lock (thistatitm);
    curexpv =
      mom_unsync_item_get_phys_attr (thistatitm, MOM_PREDEFITM (expression));
    mom_item_unlock (thistatitm);
  }
  MOM_DEBUGPRINTF (web, "doeval_nanoedit curexprv=%s",
                   mom_value_cstring (curexpv));
  struct mom_nanoeval_st nev = { 0 };
  memset (&nev, 0, sizeof (nev));
  nev.nanev_magic = NANOEVAL_MAGIC_MOM;
  nev.nanev_tkitm = tkitm;
  nev.nanev_thistatitm = thistatitm;
  nev.nanev_count = 0;
  nev.nanev_maxstep = maxstep;
  nev.nanev_errfile = NULL;
  int errlin = 0;
  if ((errlin = setjmp (nev.nanev_jb)) > 0)
    {
      MOM_WARNPRINTF_AT (nev.nanev_errfile ? : "??", errlin,
                         "nanoedit failure %s with expr %s",
                         mom_value_cstring (nev.nanev_fail),
                         mom_value_cstring (nev.nanev_expr));
#warning should output JSON for error case
      MOM_FATAPRINTF ("unhandled nanoedit eval failure from %s:%d",
                      nev.nanev_errfile ? : "??", errlin);
    }
  else
    {
      MOM_DEBUGPRINTF (run,
                       "doeval_nanoedit evaluating curexprv=%s in thistatitm.env=%s",
                       mom_value_cstring (curexpv),
                       mom_item_cstring (thistatitm));
      mom_bind_nanoev (&nev, thistatitm, MOM_PREDEFITM (web_session),
                       sessitm);
      mom_bind_nanoev (&nev, thistatitm, MOM_PREDEFITM (web_exchange),
                       wexitm);
      const void *res = mom_nanoeval (&nev, thistatitm, curexpv, 0);
      MOM_DEBUGPRINTF (run,
                       "doeval_nanoedit curexpv=%s evaluated to res=%s in %ld steps",
                       mom_value_cstring (curexpv), mom_value_cstring (res),
                       nev.nanev_count);
      struct mom_filebuffer_st *fb = mom_make_filebuffer ();
      assert (fb && fb->va_itype == MOMITY_FILEBUFFER);
      mom_file_puts (fb, "<i>result:</i> ");
      showvalue_nanoedit_mom (fb, wexitm, thistatitm, res, 0);
      const char *reshtml = mom_filebuffer_strdup (fb, MOM_FILEBUFFER_CLOSE);
      MOM_DEBUGPRINTF (run,
                       "doeval_nanoedit curexpv=%s reshtml=%s",
                       mom_value_cstring (curexpv), reshtml);
      // we output some JSON { "html": result, "resultcount": ...}  to the webrequest      
      mom_wexch_puts (wexch, "{ \"html\": \"");
      mom_output_utf8_encoded (wexch->webx_outfil, reshtml, -1);
      mom_wexch_puts (wexch, "\",\n");
      MOM_WEXCH_PRINTF (wexch, " \"resultcount\": %ld }\n", nev.nanev_count);
      GC_FREE ((void *) reshtml), reshtml = NULL;
      mom_wexch_reply (wexch, HTTP_OK, "application/json");
    };
}                               /* end of doeval_nanoedit_mom */




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
  MOM_DEBUGPRINTF (web,
                   "momf_nanoedit start tkitm=%s tknod=%s",
                   mom_item_cstring (tkitm),
                   mom_value_cstring ((const struct
                                       mom_hashedvalue_st *) tknod));
  if (mom_itype (tknod) != MOMITY_NODE || mom_raw_size (tknod) < mec__last)
    {
      /// should not happen
      MOM_WARNPRINTF ("momf_nanoedit bad tknod %s",
                      mom_value_cstring ((const struct
                                          mom_hashedvalue_st *) tknod));
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
                     mom_item_cstring (tkitm),
                     mom_item_cstring (wexitm),
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
                   mom_item_cstring (tkitm),
                   wexch->webx_count,
                   mom_webmethod_name
                   (wexch->webx_meth),
                   onion_request_get_fullpath
                   (wexch->webx_requ), mom_item_cstring (sessitm));
  sessitm->itm_pcomp = mom_vectvaldata_resize (sessitm->itm_pcomp, mes__last);
  if (!
      (hsetitm =
       mom_dyncast_item (mom_vectvaldata_nth
                         (sessitm->itm_pcomp, mes_itmhset))))
    {
      hsetitm = mom_clone_item (MOM_PREDEFITM (hashset));
      mom_vectvaldata_put_nth (sessitm->itm_pcomp,
                               mes_itmhset,
                               (struct mom_hashedvalue_st *) hsetitm);
      hsetitm->itm_payload =
        (struct mom_anyvalue_st *) mom_hashset_reserve (NULL, 20);
      MOM_DEBUGPRINTF (web,
                       "momf_nanoedit tkitm=%s wexch #%ld new hsetitm %s",
                       mom_item_cstring (tkitm),
                       wexch->webx_count, mom_item_cstring (hsetitm));
    }
  else
    MOM_DEBUGPRINTF (web,
                     "momf_nanoedit tkitm=%s wexch #%ld got hsetitm %s",
                     mom_item_cstring (tkitm),
                     wexch->webx_count, mom_item_cstring (hsetitm));
  mom_item_lock (hsetitm);
  if (wexch->webx_meth == MOMWEBM_POST)
    {
      const char *dofillpage = NULL;
      const char *doknownitem = NULL;
      const char *docompletename = NULL;
      const char *doexit = NULL;
      const char *doparsecommand = NULL;
      const char *docreateitem = NULL;
      const char *doeval = NULL;
      const char *dohideitem = NULL;
      const char *dodispitem = NULL;
      const char *commentstr = NULL;
      MOM_DEBUGPRINTF (web,
                       "momf_nanoedit tkitm=%s POST wexch #%ld",
                       mom_item_cstring (tkitm), wexch->webx_count);
      if ((dofillpage =
           onion_request_get_post (wexch->webx_requ, "do_fillpage")) != NULL)
        dofillpage_nanoedit_mom (wexch, tkitm, wexitm, thistatitm);
      else
        if ((doknownitem =
             onion_request_get_post (wexch->webx_requ,
                                     "do_knownitem")) != NULL)
        doknownitem_nanoedit_mom (wexch, tkitm,
                                  wexitm, thistatitm, doknownitem);
      else
        if ((docompletename =
             onion_request_get_post (wexch->webx_requ,
                                     "do_completename")) != NULL)
        docompletename_nanoedit_mom (wexch, tkitm,
                                     wexitm, thistatitm, docompletename);
      else
        if ((doparsecommand =
             onion_request_get_post (wexch->webx_requ,
                                     "do_parsecommand")) != NULL)
        doparsecommand_nanoedit_mom (wexch, tkitm,
                                     wexitm, thistatitm, doparsecommand);
      else
        if ((doexit =
             onion_request_get_post (wexch->webx_requ, "do_exit")) != NULL)
        doexit_nanoedit_mom (wexch, tkitm, wexitm, thistatitm, doexit);
      else
        if ((doeval =
             onion_request_get_post (wexch->webx_requ, "do_eval")) != NULL)
        doeval_nanoedit_mom (wexch, tkitm, wexitm,
                             thistatitm, sessitm, doeval);
      else
        if ((docreateitem =
             onion_request_get_post (wexch->webx_requ, "do_createitem"))
            != NULL
            && (commentstr =
                onion_request_get_post (wexch->webx_requ, "comment")) != NULL)
        docreateitem_nanoedit_mom (wexch, tkitm,
                                   wexitm,
                                   thistatitm,
                                   sessitm, docreateitem, commentstr);
      else if ((dohideitem =
                onion_request_get_post (wexch->webx_requ, "do_hideitem"))
               != NULL)
        dohideitem_nanoedit_mom (wexch, tkitm, wexitm, thistatitm,
                                 dohideitem);
      else if ((dodispitem =
                onion_request_get_post (wexch->webx_requ, "do_displayitem"))
               != NULL)
        dodispitem_nanoedit_mom (wexch, tkitm, wexitm, thistatitm,
                                 dodispitem);
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
  char rawprefix[16] = {
    0
  };
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
      MOM_DEBUGPRINTF (web,
                       "parse_token_nanoedit pos#%u end (cmdlen %u)",
                       np->nanop_pos, np->nanop_cmdlen);
      return;
    }
  MOM_DEBUGPRINTF (web,
                   "parse_token_nanoedit pos#%u uc %#x %c",
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
      int wordlen = (int) (endw - startw);
      MOM_DEBUGPRINTF (web,
                       "parse_token_nanoedit asciiw %s pos#%u wordlen=%d startw='%.*s'",
                       asciiw ? "true" : "false", np->nanop_pos, wordlen,
                       wordlen, startw);
      int radlen = wordlen;
      const char *twound = strstr (startw, "__");
      if (twound && twound - startw < wordlen)
        radlen = twound - startw;
      if (asciiw && mom_valid_name_radix_len (startw, radlen))
        {
          const char *endp = NULL;
          struct mom_item_st *itm = mom_find_item_from_string (startw, &endp);
          if (itm && endp == endw)
            {
              mom_queue_append (que, itm);
              np->nanop_pos = endw - cmd;
              MOM_DEBUGPRINTF (web,
                               "parse_token_nanoedit pos#%u item %s\n",
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
              MOM_DEBUGPRINTF (web,
                               "parse_token_nanoedit pos#%u name %s\n",
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
          MOM_DEBUGPRINTF (web,
                           "parse_token_nanoedit pos#%u word %s\n",
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
                       mom_value_cstring ((const struct mom_hashedvalue_st
                                           *) (intv)));
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
                           mom_value_cstring ((const struct
                                               mom_hashedvalue_st *) (intv)));
          np->nanop_pos += 3;
        }
      else
        NANOPARSING_FAILURE_MOM (np, np->nanop_pos, "bad escchar int 0%s",
                                 escbuf);
      ss.ss_str = NULL, ss.ss_len = 0;
    }                           /* end of integer escaped character code à la 0\n */

  else
    if ((uc < 128 && isdigit (uc))
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
          MOM_DEBUGPRINTF (web,
                           "parse_token_nanoedit pos#%u int %s\n",
                           np->nanop_pos,
                           mom_value_cstring ((const struct
                                               mom_hashedvalue_st *) (intv)));
          np->nanop_pos = endlng - cmd;
        }
      else
        {
          const struct mom_boxdouble_st *dblv = mom_boxdouble_make (d);
          mom_queue_append (que, dblv);
          MOM_DEBUGPRINTF (web,
                           "parse_token_nanoedit pos#%u double %s\n",
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
      const struct mom_boxstring_st *commv =
        mom_boxstring_make_len (startc, endc - startc);
      const struct mom_boxnode_st *nodv =
        mom_boxnode_meta_make_va (NULL, startc - cmd, MOM_PREDEFITM (comment),
                                  1, commv);
      mom_queue_append (que, nodv);
      np->nanop_pos = endc - cmd;
      MOM_DEBUGPRINTF (web, "parse_token_nanoedit pos#%u comment: %s\n",
                       (unsigned) (startc - cmd),
                       mom_value_cstring ((const struct mom_hashedvalue_st
                                           *) (nodv)));
      return;
    }                           /* end line comment parsing */

  else if (uc == '"')
    {                           // escaped string
      const char *starts = pc + 1;
      const char *ends = NULL;
      const struct mom_boxstring_st *strv = NULL;
      {
        struct mom_string_and_size_st ss = {
          NULL, 0
        };
        const char *endl = starts;
        while (*endl
               && (*endl != '\n' || *endl != '\r'
                   || *endl != '\f' || *endl != '\v'))
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
                       mom_value_cstring ((const struct mom_hashedvalue_st
                                           *) (strv)));
      return;
    }                           /* end escaped string parsing */

  else
    if (uc == '(' && isalpha (pc[1])
        && ((pos = 0),
            (memset (rawprefix, 0, sizeof (rawprefix))),
            sscanf (pc, "(%10[A-Za-z]\"%n",
                    rawprefix + 1, &pos)) >= 1 && pos > 0
        && pos < (int) sizeof (rawprefix))
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
      const struct mom_boxstring_st *wordv =
        mom_boxstring_make_len (startw, endw - startw);
      const struct mom_boxnode_st *nodv =
        mom_boxnode_meta_make_va (NULL, startw - cmd, MOM_PREDEFITM (word), 1,
                                  wordv);
      mom_queue_append (que, nodv);
      np->nanop_pos = endw - cmd;
      MOM_DEBUGPRINTF (web, "parse_token_nanoedit pos#%u extendedword %s\n",
                       (unsigned) (startw - cmd),
                       mom_value_cstring ((const struct mom_hashedvalue_st
                                           *) (nodv)));
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
                                       (const struct mom_hashedvalue_st
                                        *) delimstrv);
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
                                       (const struct mom_hashedvalue_st
                                        *) delimstrv);
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
                                   (const struct mom_hashedvalue_st
                                    *) delimstrv);
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
                                  1, delimval);
      mom_queue_append (que, nodv);
      np->nanop_pos = endp - cmd;
      MOM_DEBUGPRINTF (web,
                       "parse_token_nanoedit pos#%u delimiter %s\n",
                       (unsigned) (pc0 - cmd),
                       mom_value_cstring ((const struct
                                           mom_hashedvalue_st *) (nodv)));
      return;
    }

#warning incomplete parse_token_nanoedit_mom

  NANOPARSING_FAILURE_MOM (np, np->nanop_pos, "bad lexical token %s",
                           cmd + np->nanop_pos);
}                               /* end parse_token_nanoedit_mom */





static inline bool
isdelim_nanoedit_mom (struct nanoparsing_mom_st *np,
                      int pos, struct mom_item_st *delimitm)
{
  assert (np && np->nanop_magic == NANOPARSING_MAGIC_MOM);
  const struct mom_boxnode_st *nodexp = np->nanop_nodexpr;
  assert (nodexp && nodexp->va_itype == MOMITY_NODE);
  assert (delimitm && delimitm->va_itype == MOMITY_ITEM);
  unsigned nlen = mom_size (nodexp);
  if (pos < 0 || pos >= (int) nlen)
    return false;
  struct mom_hashedvalue_st *tokv = nodexp->nod_sons[pos];
  const struct mom_boxnode_st *toknod = mom_dyncast_node (tokv);
  if (!toknod || mom_size (toknod) != 1)
    return false;
  if (toknod->nod_connitm != MOM_PREDEFITM (delimiter))
    return false;
  return (const void *) toknod->nod_sons[0] == (const void *) delimitm;
}                               /* end of isdelim_nanoedit_mom */



static const void *parsexprprec_nanoedit_mom (struct
                                              nanoparsing_mom_st
                                              *np, int prec, int *posptr);
static const void *parsexpr_nanoedit_mom (struct
                                          nanoparsing_mom_st
                                          *np, int *posptr);
static const void *
parsprimary_nanoedit_mom (struct nanoparsing_mom_st *np, int *posptr)
{
  assert (np && np->nanop_magic == NANOPARSING_MAGIC_MOM);
  const struct mom_boxnode_st *nodexp = np->nanop_nodexpr;
  assert (nodexp && nodexp->va_itype == MOMITY_NODE);
  assert (posptr != NULL);
  int pos = *posptr;
  unsigned nlen = mom_size (nodexp);
  if (pos < 0 || pos >= (int) nlen)
    return NULL;
  struct mom_hashedvalue_st *curtokv = nodexp->nod_sons[pos];
  if (!curtokv)
    NANOPARSING_FAILURE_MOM (np, -pos,
                             "(primary) no token at position #%d", pos);
  switch (mom_itype (curtokv))
    {
    case MOMITY_BOXINT:
    case MOMITY_BOXDOUBLE:
    case MOMITY_BOXSTRING:
    case MOMITY_ITEM:
      {
        *posptr = pos + 1;
        MOM_DEBUGPRINTF (web,
                         "parsprimary_nanoedit scalar %s at pos#%d",
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
                         "parsprimary_nanoedit composite token %s at position #%d (nlen=%d)",
                         mom_value_cstring (curtokv), pos, nlen);
        //////////////// ~ is for nil
        if (pos < (int) nlen
            && isdelim_nanoedit_mom (np, pos, MOM_PREDEFITM (tilde_delim)))
          {
            MOM_DEBUGPRINTF (web,
                             "parsprimary_nanoedit tilde-expr for nil pos#%d",
                             pos);
            *posptr = pos + 1;
            return NULL;
          }
        //////////////// %name(comma-seperated-subexpr...)
        if (pos + 3 < (int) nlen
            && isdelim_nanoedit_mom (np, pos, MOM_PREDEFITM (percent_delim)))
          {
            struct mom_hashedvalue_st *next1tokv = nodexp->nod_sons[pos + 1];
            struct mom_hashedvalue_st *next2tokv = nodexp->nod_sons[pos + 2];
            struct mom_item_st *connitm = mom_dyncast_item (next1tokv);
            if (!connitm)
              NANOPARSING_FAILURE_WITH_MOM (np, off,
                                            next1tokv,
                                            "parsprimary_nanoedit unexpected connective %s",
                                            mom_value_cstring (next1tokv));
            if (!isdelim_nanoedit_mom
                (np, pos + 2, MOM_PREDEFITM (left_paren_delim)))
              NANOPARSING_FAILURE_WITH_MOM (np, off, next2tokv,
                                            "parsprimary_nanoedit expected left parenthesis got %s after connective %s",
                                            mom_value_cstring (next2tokv),
                                            mom_item_cstring (connitm));
            MOM_DEBUGPRINTF (web,
                             "parsprimary_nanoedit percent-expr pos#%d off@%d connitm %s after leftparen",
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
                MOM_DEBUGPRINTF (web,
                                 "parsprimary_nanoedit percent-expr prevpos#%d curpos#%d"
                                 " subexpv %s count %d", prevpos,
                                 curpos, mom_value_cstring (subexpv),
                                 mom_vectvaldata_count (vec));
                if (!subexpv && curpos == prevpos)
                  NANOPARSING_FAILURE_WITH_MOM (np, off,
                                                next2tokv,
                                                "parsprimary_nanoedit bad subexpression#%d in percent-node of connective %s",
                                                mom_vectvaldata_count
                                                (vec),
                                                mom_item_cstring (connitm));
                vec = mom_vectvaldata_append (vec, subexpv);
                if (isdelim_nanoedit_mom
                    (np, curpos, MOM_PREDEFITM (comma_delim)))
                  {
                    MOM_DEBUGPRINTF (web,
                                     "parsprimary_nanoedit comma curpos=%d",
                                     curpos);
                    curpos++;
                  }
                else if (isdelim_nanoedit_mom (np, curpos,
                                               MOM_PREDEFITM
                                               (right_paren_delim)))
                  {
                    MOM_DEBUGPRINTF (web,
                                     "parsprimary_nanoedit rightparen curpos=%d",
                                     curpos);
                    curpos++;
                    break;
                  }
              }
            MOM_DEBUGPRINTF (web,
                             "parsprimary_nanoedit percent-expr after rightparenfinal   curpos=%d connitm=%s",
                             curpos, mom_item_cstring (connitm));
            // use mom_boxnode_make_meta...
            const struct mom_boxnode_st *nodres =
              mom_boxnode_make_meta (connitm,
                                     mom_vectvaldata_count (vec),
                                     ((const struct
                                       mom_hashedvalue_st **)
                                      mom_vectvaldata_valvect
                                      (vec)), np->nanop_wexitm,
                                     pos);
            MOM_DEBUGPRINTF (web,
                             "parsprimary_nanoedit percent-expr curpos#%d nodres=%s ",
                             curpos,
                             mom_value_cstring ((const struct
                                                 mom_hashedvalue_st
                                                 *) nodres));
            *posptr = curpos;
            return nodres;
          }
        /////////////////////// (subexpr)  --- starting with a leftparen
        else if (pos + 2 < (int) nlen
                 && isdelim_nanoedit_mom (np, pos,
                                          MOM_PREDEFITM (left_paren_delim)))
          {
            int curpos = pos + 1;
            struct mom_hashedvalue_st *next1tokv = nodexp->nod_sons[pos + 1];
            MOM_DEBUGPRINTF (web,
                             "parsprimary_nanoedit leftparen-expr pos=%d next1tokv=%s",
                             pos, mom_value_cstring (next1tokv));
            int prevpos = curpos;
            const void *subexpv = parsexpr_nanoedit_mom (np, &curpos);
            MOM_DEBUGPRINTF (web,
                             "parsprimary_nanoedit leftparen-expr prevpos#%d curpos#%d"
                             " subexpv %s", prevpos,
                             curpos, mom_value_cstring (subexpv));
            if (!subexpv && curpos == prevpos)
              NANOPARSING_FAILURE_WITH_MOM (np, off,
                                            next1tokv,
                                            "parsprimary_nanoedit missing subexpression in leftparen-node");
            if (isdelim_nanoedit_mom (np, curpos,
                                      MOM_PREDEFITM (right_paren_delim)))
              {
                MOM_DEBUGPRINTF (web,
                                 "parsprimary_nanoedit rightparen curpos=%d",
                                 curpos);
                curpos++;
              }
            const struct mom_boxnode_st *nodres =       //
              mom_boxnode_meta_make_va (np->nanop_wexitm,
                                        curpos,
                                        MOM_PREDEFITM (parenthesis), 1,
                                        subexpv);

            MOM_DEBUGPRINTF (web,
                             "parsprimary_nanoedit parenexpr %s curpos=%d",
                             mom_value_cstring ((const struct
                                                 mom_hashedvalue_st *)
                                                nodres), curpos);

            *posptr = curpos;
            return nodres;
          }                     /// end left-paren
        //////////////// [comp,...] ---- tuple starting with left bracket
        else if (pos + 1 < (int) nlen
                 && isdelim_nanoedit_mom (np, pos,
                                          MOM_PREDEFITM (left_bracket_delim)))
          {
            int curpos = pos + 1;
            struct mom_hashedvalue_st *next1tokv = nodexp->nod_sons[pos + 1];
            MOM_DEBUGPRINTF (web,
                             "parsprimary_nanoedit leftbracket-expr pos=%d next1tokv=%s",
                             pos, mom_value_cstring (next1tokv));
            struct mom_vectvaldata_st *vec =
              mom_vectvaldata_reserve (NULL, 5);
            assert (vec != NULL);
            while (curpos < (int) nlen)
              {
                int prevpos = curpos;
                if (isdelim_nanoedit_mom (np, curpos,
                                          MOM_PREDEFITM
                                          (right_bracket_delim)))
                  {
                    curpos++;
                    break;
                  }
                const void *subexpv = parsexpr_nanoedit_mom (np, &curpos);
                MOM_DEBUGPRINTF (web,
                                 "parsprimary_nanoedit bracket-expr prevpos#%d curpos#%d"
                                 " subexpv %s count %d", prevpos,
                                 curpos, mom_value_cstring (subexpv),
                                 mom_vectvaldata_count (vec));
                if (!subexpv && curpos == prevpos)
                  NANOPARSING_FAILURE_WITH_MOM (np, off,
                                                next1tokv,
                                                "parsprimary_nanoedit bad subexpression#%d in bracket-node",
                                                mom_vectvaldata_count (vec));
                vec = mom_vectvaldata_append (vec, subexpv);
                if (isdelim_nanoedit_mom
                    (np, curpos, MOM_PREDEFITM (comma_delim)))
                  {
                    MOM_DEBUGPRINTF (web,
                                     "parsprimary_nanoedit comma curpos=%d",
                                     curpos);
                    curpos++;
                    continue;
                  }
                else if (curpos < (int) nlen
                         && !isdelim_nanoedit_mom (np, curpos,
                                                   MOM_PREDEFITM
                                                   (right_bracket_delim)))
                  {
                    struct mom_hashedvalue_st *next1tokv =
                      nodexp->nod_sons[curpos];
                    NANOPARSING_FAILURE_WITH_MOM (np, off, next1tokv,
                                                  "parsprimary_nanoedit expecting right bracket");

                  }
              }
            MOM_DEBUGPRINTF (web,
                             "parsprimary_nanoedit bracket-expr after rightbracketfinal curpos=%d",
                             curpos);
            // use mom_boxnode_make_meta...
            const struct mom_boxnode_st *nodres =
              mom_boxnode_make_meta (MOM_PREDEFITM (tuple),
                                     mom_vectvaldata_count (vec),
                                     ((const struct mom_hashedvalue_st **)
                                      mom_vectvaldata_valvect (vec)),
                                     np->nanop_wexitm,
                                     pos);
            MOM_DEBUGPRINTF (web,
                             "parsprimary_nanoedit bracket-expr curpos#%d tuple nodres=%s",
                             curpos,
                             mom_value_cstring ((const struct
                                                 mom_hashedvalue_st *)
                                                nodres));
            *posptr = curpos;
            return nodres;
          }                     /// end left-bracket
        //////////////// {element,...} --- braces are for sets
        else if (pos + 1 < (int) nlen
                 && isdelim_nanoedit_mom (np, pos,
                                          MOM_PREDEFITM (left_brace_delim)))
          {
            int curpos = pos + 1;
            struct mom_hashedvalue_st *next1tokv = nodexp->nod_sons[pos + 1];
            MOM_DEBUGPRINTF (web,
                             "parsprimary_nanoedit leftbrace-expr pos=%d next1tokv=%s",
                             pos, mom_value_cstring (next1tokv));
            struct mom_vectvaldata_st *vec =
              mom_vectvaldata_reserve (NULL, 5);
            assert (vec != NULL);
            while (curpos < (int) nlen)
              {
                MOM_DEBUGPRINTF (web,
                                 "parsprimary_nanoedit brace-expr curpos=%d tok %s",
                                 curpos,
                                 mom_value_cstring (nodexp->nod_sons
                                                    [curpos]));
                if (isdelim_nanoedit_mom
                    (np, curpos, MOM_PREDEFITM (right_brace_delim)))
                  {
                    MOM_DEBUGPRINTF (web,
                                     "parsprimary_nanoedit rightbrace curpos=%d",
                                     curpos);
                    curpos++;
                    break;
                  }
                else
                  MOM_DEBUGPRINTF (web,
                                   "parsprimary_nanoedit notrightbrace curpos=%d",
                                   curpos);
                int prevpos = curpos;
                const void *subexpv = parsexpr_nanoedit_mom (np, &curpos);
                MOM_DEBUGPRINTF (web,
                                 "parsprimary_nanoedit brace-expr prevpos#%d curpos#%d"
                                 " subexpv %s count %d", prevpos,
                                 curpos, mom_value_cstring (subexpv),
                                 mom_vectvaldata_count (vec));
                if (!subexpv && curpos == prevpos)
                  NANOPARSING_FAILURE_WITH_MOM (np, off,
                                                next1tokv,
                                                "parsprimary_nanoedit bad subexpression#%d in bracket-node",
                                                mom_vectvaldata_count (vec));
                vec = mom_vectvaldata_append (vec, subexpv);
                if (isdelim_nanoedit_mom
                    (np, curpos, MOM_PREDEFITM (comma_delim)))
                  {
                    MOM_DEBUGPRINTF (web,
                                     "parsprimary_nanoedit comma curpos=%d",
                                     curpos);
                    curpos++;
                  }
                else if (curpos < (int) nlen
                         && !isdelim_nanoedit_mom (np, curpos,
                                                   MOM_PREDEFITM
                                                   (right_brace_delim)))
                  {
                    struct mom_hashedvalue_st *next1tokv =
                      nodexp->nod_sons[curpos];
                    NANOPARSING_FAILURE_WITH_MOM (np, off, next1tokv,
                                                  "parsprimary_nanoedit expecting right bracket");

                  }
              }
            MOM_DEBUGPRINTF (web,
                             "parsprimary_nanoedit brace-expr after rightbracefinal curpos=%d",
                             curpos);
            // use mom_boxnode_make_meta...
            const struct mom_boxnode_st *nodres =
              mom_boxnode_make_meta (MOM_PREDEFITM (set),
                                     mom_vectvaldata_count (vec),
                                     ((const struct mom_hashedvalue_st **)
                                      mom_vectvaldata_valvect (vec)),
                                     np->nanop_wexitm,
                                     pos);
            MOM_DEBUGPRINTF (web,
                             "parsprimary_nanoedit brace-expr curpos#%d set nodres=%s",
                             curpos,
                             mom_value_cstring ((const struct
                                                 mom_hashedvalue_st *)
                                                nodres));
            *posptr = curpos;
            return nodres;
          }                     /// end left-brace

#warning incomplete code parsprimary_nanoedit
      }
    default:
      MOM_WARNPRINTF ("unhandled token %s at pos %d",
                      mom_value_cstring (curtokv), pos);
      break;
    }
  NANOPARSING_FAILURE_WITH_MOM (np, -pos, curtokv,
                                "parsprimary_nanoedit unexpected token %s",
                                mom_value_cstring (curtokv));
}                               /* end parsprimary_nanoedit_mom */



// precedence 1 is the highest, precedence 15 is the lowest
// e.g. http://en.cppreference.com/w/c/language/operator_precedence 
#define NANOEDIT_MAX_PRECEDENCE_MOM 16
static const void *
parsexprprec_nanoedit_mom (struct
                           nanoparsing_mom_st *np, int prec, int *posptr)
{
  assert (np && np->nanop_magic == NANOPARSING_MAGIC_MOM);
  const struct mom_boxnode_st *nodexp = np->nanop_nodexpr;
  assert (nodexp && nodexp->va_itype == MOMITY_NODE);
  assert (posptr != NULL);
  int startpos = *posptr;
  assert (prec >= 0 && prec <= NANOEDIT_MAX_PRECEDENCE_MOM);
  if (prec <= 0)
    return parsprimary_nanoedit_mom (np, posptr);
  unsigned nlen = mom_size (nodexp);
  if (startpos < 0 || startpos >= (int) nlen)
    return NULL;
  struct mom_hashedvalue_st *curtokv = nodexp->nod_sons[startpos];
  if (!curtokv)
    NANOPARSING_FAILURE_MOM (np, -startpos,
                             "(expr prec#%d) no token at position #%d",
                             prec, startpos);
  MOM_DEBUGPRINTF (web,
                   "parsexprprec_nanoedit prec=%d startpos=%d curtokv=%s",
                   prec, startpos, mom_value_cstring (curtokv));
  int curpos = startpos;
  const void *leftexprv = parsexprprec_nanoedit_mom (np, prec - 1, &curpos);
  MOM_DEBUGPRINTF (web,
                   "parsexprprec_nanoedit prec=%d leftexprv=%s curpos=%d startpos=%d",
                   prec, mom_value_cstring (leftexprv), curpos, startpos);
  if (!leftexprv && curpos <= startpos)
    NANOPARSING_FAILURE_MOM (np, -startpos,
                             "failed to parse subexpression of precedence %d",
                             prec - 1);
  struct mom_hashedvalue_st *nexttokv = NULL;
  if (curpos >= 0 && curpos < (int) nlen)
    nexttokv = nodexp->nod_sons[curpos];
  MOM_DEBUGPRINTF (web,
                   "parsexprprec_nanoedit prec=%d curpos=%d nexttokv=%s",
                   prec, curpos, mom_value_cstring (nexttokv));
  if (!nexttokv
      /// this are the terminating delimiters
      || isdelim_nanoedit_mom (np, curpos,
                               MOM_PREDEFITM
                               (comma_delim))
      || isdelim_nanoedit_mom (np, curpos,
                               MOM_PREDEFITM
                               (semicolon_delim))
      || isdelim_nanoedit_mom (np, curpos,
                               MOM_PREDEFITM
                               (right_paren_delim))
      || isdelim_nanoedit_mom (np, curpos,
                               MOM_PREDEFITM
                               (right_bracket_delim))
      || isdelim_nanoedit_mom (np, curpos, MOM_PREDEFITM (right_brace_delim)))
    {
      MOM_DEBUGPRINTF
        (web,
         "parsexprprec_nanoedit prec=%d startpos=%d curpos=%d returning leftexprv=%s",
         prec, startpos, curpos, mom_value_cstring (leftexprv));
      *posptr = curpos;
      return leftexprv;
    };
  const struct mom_boxnode_st *nexttoknod = NULL;
  struct mom_item_st *delimitm = NULL;
  unsigned ecount = 0, esize = 0;
  struct mom_item_st **delitmarr = NULL;
  int *posarr = NULL;
  void **exparr = NULL;
  while (curpos >= 0 && curpos < (int) nlen
         && (nexttokv = nodexp->nod_sons[curpos]) != NULL
         && (nexttoknod = mom_dyncast_node (nexttokv)) != NULL
         && nexttoknod->nod_connitm == MOM_PREDEFITM (delimiter)
         && mom_size (nexttoknod) == 1
         && (delimitm = mom_dyncast_item (nexttoknod->nod_sons[0])) != NULL)
    {
      int delimprec = -1;
      struct mom_item_st *associtm = NULL;
      MOM_DEBUGPRINTF (web,
                       "parsexprprec_nanoedit prec=%d curpos=%d nexttokv=%s delimitm=%s",
                       prec, curpos, mom_value_cstring (nexttokv),
                       mom_item_cstring (delimitm));
      {
        mom_item_lock (delimitm);
        const struct mom_hashedvalue_st *delimprecv
          = mom_unsync_item_get_phys_attr (delimitm,
                                           MOM_PREDEFITM (precedence));
        const struct mom_hashedvalue_st *assocv
          = mom_unsync_item_get_phys_attr (delimitm,
                                           MOM_PREDEFITM (associativity));
        delimprec = mom_boxint_val_def (delimprecv, -1);
        associtm = mom_dyncast_item (assocv);
        mom_item_unlock (delimitm);
        MOM_DEBUGPRINTF (web,
                         "parsexprprec_nanoedit delimprecv=%s delimprec#%d assocv=%s",
                         mom_value_cstring (delimprecv), delimprec,
                         mom_value_cstring (assocv));
      }
      if (delimprec != prec)
        {
          MOM_DEBUGPRINTF
            (web,
             "parsexprprec_nanoedit prec=%d startpos=%d curpos=%d breaking with delimprec %d",
             prec, startpos, curpos, delimprec);
          break;
        }
      if ((!associtm || associtm == MOM_PREDEFITM (no_assoc)) && ecount > 1)
        NANOPARSING_FAILURE_MOM (np, -startpos,
                                 "non-associative operator %s after %s with more than two operands for precedence %d (curpos %d)",
                                 mom_item_cstring (delimitm),
                                 mom_value_cstring (leftexprv), prec, curpos);
      if (MOM_UNLIKELY (ecount + 1 >= esize))
        {
          unsigned newsiz = ((3 * ecount / 2 + 2) | 7) + 1;
          struct mom_item_st **newdelitmarr =
            mom_gc_alloc (newsiz * sizeof (struct mom_item_st *));
          void **newexparr = mom_gc_alloc (newsiz * sizeof (void *));
          int *newposarr = mom_gc_alloc_atomic (newsiz * sizeof (int));
          if (ecount > 0)
            {
              memcpy (newdelitmarr, delitmarr,
                      ecount * sizeof (struct mom_item_st *));
              memcpy (newexparr, exparr, ecount * sizeof (void *));
              memcpy (newposarr, posarr, ecount * sizeof (int));
              GC_FREE (delitmarr), delitmarr = NULL;
              GC_FREE (exparr), exparr = NULL;
              GC_FREE (posarr), posarr = NULL;
            }
          delitmarr = newdelitmarr;
          exparr = newexparr;
          posarr = newposarr;
          esize = newsiz;
        };
      int delimpos = curpos;
      curpos++;                 // to consume the delimiter
      MOM_DEBUGPRINTF
        (web,
         "parsexprprec_nanoedit prec=%d parsing subexpr#%d at curpos=%d after operator %s before subexpr parsing",
         prec, ecount, curpos, mom_item_cstring (delimitm));
      void *subexprv = parsexprprec_nanoedit_mom (np, prec - 1, &curpos);
      MOM_DEBUGPRINTF (web,
                       "parsexprprec_nanoedit prec=%d subexprv#%d=%s curpos=%d startpos=%d",
                       prec, ecount, mom_value_cstring (subexprv), curpos,
                       startpos);
      if (!subexprv && curpos <= startpos)
        NANOPARSING_FAILURE_MOM (np, -startpos,
                                 "failed to parse subexpression of precedence %d after operator %s",
                                 prec - 1, mom_item_cstring (delimitm));
      delitmarr[ecount] = delimitm;
      exparr[ecount] = subexprv;
      posarr[ecount] = delimpos;
      MOM_DEBUGPRINTF (web,
                       "parsexprprec_nanoedit prec=%d ecount=%d delimitm=%s delimpos=%d associtm=%s subexprv=%s",
                       prec, ecount, mom_item_cstring (delimitm), delimpos,
                       mom_item_cstring (associtm),
                       mom_value_cstring (subexprv));
      ecount++;
    }
  MOM_DEBUGPRINTF (web,
                   "parsexprprec_nanoedit startpos=%d curpos=%d prec=%d final ecount=%d",
                   startpos, curpos, prec, ecount);
  if (MOM_IS_DEBUGGING (web))
    {
      for (int ix = 0; ix < (int) ecount; ix++)
        MOM_DEBUGPRINTF (web,
                         "parsexprprec_nanoedit delitmarr[%d]=%s posarr[%d]=%d exparr[%d]=%s",
                         ix, mom_item_cstring (delitmarr[ix]),
                         ix, posarr[ix], ix, mom_value_cstring (exparr[ix]));
    }
  if (ecount == 0)
    {
      MOM_DEBUGPRINTF
        (web,
         "parsexprprec_nanoedit prec=%d startpos=%d curpos=%d zero-ecount returning leftexprv=%s",
         prec, startpos, curpos, mom_value_cstring (leftexprv));
      *posptr = curpos;
      return leftexprv;
    }
  else if (ecount == 1)
    {
      MOM_DEBUGPRINTF
        (web,
         "parsexprprec_nanoedit prec=%d startpos=%d curpos=%d single-ecount leftexprv=%s delitmarr[0]=%s posarr[0]=%d exparr[0]=%s",
         prec, startpos, curpos, mom_value_cstring (leftexprv),
         mom_item_cstring (delitmarr[0]), posarr[0],
         mom_value_cstring (exparr[0]));
      delimitm = delitmarr[0];
      struct mom_item_st *operitm = NULL;
      {
        mom_item_lock (delimitm);
        const struct mom_hashedvalue_st *operv
          = mom_unsync_item_get_phys_attr (delimitm,
                                           MOM_PREDEFITM (operator));
        operitm = mom_dyncast_item (operv);
        mom_item_unlock (delimitm);
      }
      MOM_DEBUGPRINTF (web,
                       "parsexprprec_nanoedit operitm=%s from delimitm=%s prec=%d",
                       mom_item_cstring (operitm),
                       mom_item_cstring (delimitm), prec);
      if (!operitm)
        NANOPARSING_FAILURE_MOM (np, -posarr[0],
                                 "delimiter %s without operator for precedence %d",
                                 mom_item_cstring (delimitm), prec);
      struct mom_item_st *metaitm = NULL;
      int metarank = posarr[0];
      const struct mom_boxnode_st *nodres =
        mom_boxnode_meta_make_va (metaitm, metarank, operitm, 2, leftexprv,
                                  exparr[0]);
      MOM_DEBUGPRINTF (web,
                       "parsexprprec_nanoedit nodres=%s prec=%d curpos=%d",
                       mom_value_cstring ((struct mom_hashedvalue_st *)
                                          nodres), prec, curpos);
      *posptr = curpos;
      return nodres;
    }
  else
    {
      /* ecount > 1 */
      MOM_DEBUGPRINTF (web,
                       "parsexprprec_nanoedit prec=%d startpos=%d curpos=%d ecount=%d leftexprv=%s",
                       prec, startpos, curpos, ecount,
                       mom_value_cstring (leftexprv));
      int nbleft = 0, nbright = 0;
      struct mom_item_st **operarr =
        mom_gc_alloc (sizeof (struct mom_item_st *) * (ecount + 1));
      struct mom_item_st *associtm = NULL;
      for (unsigned ix = 0; ix < ecount; ix++)
        {
          struct mom_item_st *operitm = NULL;
          struct mom_item_st *curassocitm = NULL;
          delimitm = delitmarr[ix];
          void *subexpv = exparr[ix];
          int subpos = posarr[ix];
          {
            mom_item_lock (delimitm);
            const struct mom_hashedvalue_st *operv
              = mom_unsync_item_get_phys_attr (delimitm,
                                               MOM_PREDEFITM (operator));
            operitm = mom_dyncast_item (operv);
            const struct mom_hashedvalue_st *assocv
              = mom_unsync_item_get_phys_attr (delimitm,
                                               MOM_PREDEFITM (associativity));
            curassocitm = mom_dyncast_item (assocv);
            if (!curassocitm)
              curassocitm = MOM_PREDEFITM (no_assoc);
            mom_item_unlock (delimitm);
          }
          MOM_DEBUGPRINTF
            (web,
             "parsexprprec_nanoedit prec=%d ix=%d delimitm=%s operitm=%s curassocitm=%s subexpv=%s subpos=%d",
             prec, ix, mom_item_cstring (delimitm),
             mom_item_cstring (operitm),
             mom_item_cstring (curassocitm),
             mom_value_cstring (subexpv), subpos);
          if (ix == 0)
            associtm = curassocitm;
          else if (associtm != curassocitm)
            NANOPARSING_FAILURE_MOM (np, -subpos,
                                     "different associativities for delimiters %s and %s of same precedence %d",
                                     mom_item_cstring (delitmarr[0]),
                                     mom_item_cstring (delimitm), prec);
          else if (!associtm || curassocitm == MOM_PREDEFITM (no_assoc))
            NANOPARSING_FAILURE_MOM (np, -subpos,
                                     "non-associative delimiter %s with more than two operands of same precedence %d",
                                     mom_item_cstring (delimitm), prec);

          if (!operitm)
            NANOPARSING_FAILURE_MOM (np, -subpos,
                                     "missing operator in delimiter %s of precedence %d",
                                     mom_item_cstring (delimitm), prec);
          operarr[ix] = operitm;
        };
      /// now, build the nodes using the associativity
      if (associtm == MOM_PREDEFITM (left_assoc))
        {                       /// the expression is left op1 right1 op2 right2 ... opn rightn
          /// to be parsed as (left right1 op1) right2 ... ) opn rightn
          const void *resexprv = leftexprv;
          for (unsigned ix = 0; ix < ecount; ix++)
            {
              int metarank = posarr[ix];
              struct mom_item_st *metaitm = NULL;
              struct mom_item_st *operitm = operarr[ix];
              const struct mom_boxnode_st *nodres =
                mom_boxnode_meta_make_va (metaitm, metarank, operitm, 2,
                                          resexprv,
                                          exparr[ix]);
              resexprv = nodres;
            }
          *posptr = curpos;
          MOM_DEBUGPRINTF
            (web,
             "parsexprprec_nanoedit leftassoc prec=%d ecount=%d curpos=%d resexprv=%s",
             prec, ecount, curpos, mom_value_cstring (resexprv));
          return resexprv;
        }
      else if (associtm == MOM_PREDEFITM (right_assoc))
        {
          const void *resexprv = exparr[ecount - 1];
          for (int ix = (int)ecount - 2; ix > 0; ix--)
            {
              int metarank = posarr[ix];
              struct mom_item_st *metaitm = NULL;
              struct mom_item_st *operitm = operarr[ix];
              const struct mom_boxnode_st *nodres =
                mom_boxnode_meta_make_va (metaitm, metarank, operitm, 2,
                                          exparr[ix], resexprv);
              resexprv = nodres;
            }
          *posptr = curpos;
          MOM_DEBUGPRINTF
            (web,
             "parsexprprec_nanoedit rightassoc prec=%d ecount=%d curpos=%d resexprv=%s",
             prec, ecount, curpos, mom_value_cstring (resexprv));
          return resexprv;
        }
      else                      /// should never happen
        MOM_FATAPRINTF ("corruption, invalid associtm %s delimitm %s",
                        mom_item_cstring (associtm),
                        mom_item_cstring (delimitm));

#warning incomplete parsexprprec_nanoedit_mom
    }

  NANOPARSING_FAILURE_MOM (np, -startpos,
                           "unimplemented parsexprprec_nanoedit prec%d (curpos %d)",
                           prec, curpos);
}                               /* end parsexprprec_nanoedit_mom */




static const void *
parsexpr_nanoedit_mom (struct nanoparsing_mom_st *np, int *posptr)
{
  return parsexprprec_nanoedit_mom (np, NANOEDIT_MAX_PRECEDENCE_MOM, posptr);
}

static void
doparsecommand_nanoedit_mom (struct mom_webexch_st
                             *wexch,
                             struct mom_item_st
                             *tkitm,
                             struct mom_item_st
                             *wexitm,
                             struct mom_item_st *thistatitm, const char *cmd)
{
  struct mom_item_st *sessitm = wexch->webx_sessitm;
  struct mom_queue_st *que = NULL;
  MOM_DEBUGPRINTF (web,
                   "doparsecommand_nanoedit webr#%ld tkitm=%s wexitm=%s thistatitm=%s sessitm=%s cmd=%s",
                   wexch->webx_count,
                   mom_item_cstring (tkitm),
                   mom_item_cstring (wexitm),
                   mom_item_cstring (thistatitm),
                   mom_item_cstring (sessitm), cmd);
  assert (tkitm && tkitm->va_itype == MOMITY_ITEM);
  const struct mom_boxnode_st *tknod = mom_dyncast_node (tkitm->itm_payload);
  assert (tknod != NULL && mom_size (tknod) >= mec__last);
  struct mom_item_st *delimitm =
    mom_dyncast_item (tknod->nod_sons[mec_delimiters]);
  // remove any previous expression
  MOM_DEBUGPRINTF (web,
                   "doparsecommand_nanoedit thistatitm=%s forgetexpr",
                   mom_item_cstring (thistatitm));
  mom_unsync_item_remove_phys_attr (thistatitm, MOM_PREDEFITM (expression));
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
    MOM_DEBUGPRINTF (web,
                     "doparsecommand_nanoedit queitm=%s",
                     mom_item_cstring (npars.nanop_queitm));
  }
  // retrieve the delim item
  {
    assert (delimitm != NULL && delimitm->va_itype == MOMITY_ITEM);
    MOM_DEBUGPRINTF (web,
                     "doparsecommand_nanoedit delimitm=%s",
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
            MOM_WARNPRINTF_AT
              (__FILE__, linerr,
               "doparsecommand_nanoedit parsing error with %s: %s"
               "\n.. at position %u of:\n%s\n",
               mom_value_cstring (npars.nanop_errval),
               mom_boxstring_cstr (npars.nanop_errmsgv), npars.nanop_pos,
               npars.nanop_cmdstr);
          else
            MOM_WARNPRINTF_AT
              (__FILE__, linerr, "doparsecommand_nanoedit parsing error: %s"
               "\n.. at position %u of:\n%s\n",
               mom_boxstring_cstr (npars.nanop_errmsgv),
               npars.nanop_pos, npars.nanop_cmdstr);
        }
      else
        {
          if (npars.nanop_errval)
            MOM_WARNPRINTF_AT
              (__FILE__, linerr,
               "doparsecommand_nanoedit parsing error with %s: %s"
               "\n.. at index %d of:\n%s\n",
               mom_value_cstring (npars.nanop_errval),
               mom_boxstring_cstr (npars.nanop_errmsgv), -npars.nanop_pos,
               npars.nanop_cmdstr);
          else
            MOM_WARNPRINTF_AT
              (__FILE__, linerr, "doparsecommand_nanoedit parsing error: %s"
               "\n.. at index %d of:\n%s\n",
               mom_boxstring_cstr (npars.nanop_errmsgv),
               -npars.nanop_pos, npars.nanop_cmdstr);
        }
      const char *errhtml = NULL;
      const struct mom_boxstring_st *badnamstr = NULL;
      MOM_DEBUGPRINTF (web, "doparsecommand_nanoedit  webr#%ld errval=%s",
                       wexch->webx_count,
                       mom_value_cstring (npars.nanop_errval));
      {
        const struct mom_boxnode_st *errnod = NULL;
        char *errbuf = NULL;
        size_t errsiz = 0;
        FILE *ferr = open_memstream (&errbuf, &errsiz);
        if (!ferr)
          MOM_FATAPRINTF
            ("failed to open memory stream for error %s - %m",
             mom_boxstring_cstr (npars.nanop_errmsgv));
        fprintf (ferr, "<b>parsing error @%d</b>: <tt>", npars.nanop_pos);
        mom_output_utf8_html (ferr,
                              mom_boxstring_cstr
                              (npars.nanop_errmsgv), -1, true);
        fputs ("</tt>", ferr);
        if ((errnod = mom_dyncast_node (npars.nanop_errval))
            && errnod->nod_connitm == MOM_PREDEFITM (name)
            && mom_raw_size (errnod) == 1
            && (badnamstr = mom_dyncast_boxstring (errnod->nod_sons[0])))
          {
            MOM_DEBUGPRINTF (web,
                             "doparsecommand_nanoedit badnamstr=%s",
                             mom_boxstring_cstr (badnamstr));
          }
        fflush (ferr);
        errhtml = GC_STRDUP (errbuf);
        fclose (ferr);
        free (errbuf);
      }
      MOM_DEBUGPRINTF (web,
                       "doparsecommand_nanoedit errhtml=%s errval=%s badnamstr=%s",
                       errhtml, mom_value_cstring (npars.nanop_errval),
                       mom_value_cstring ((void *) badnamstr));
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
      // remove any previous expression
      MOM_DEBUGPRINTF (web,
                       "doparsecommand_nanoedit thistatitm=%s forgetexpr",
                       mom_item_cstring (thistatitm));
      mom_unsync_item_remove_phys_attr (thistatitm,
                                        MOM_PREDEFITM (expression));
      while (cmd[npars.nanop_pos] != (char) 0)
        parse_token_nanoedit_mom (&npars);
      const struct mom_boxnode_st *lexqnod =
        mom_queue_node (mom_dyncast_queue (npars.nanop_queitm->itm_payload),
                        MOM_PREDEFITM (queue));
      MOM_DEBUGPRINTF (web,
                       "doparsecommand_nanoedit lexqnod=%s",
                       mom_value_cstring ((struct
                                           mom_hashedvalue_st *) lexqnod));
      npars.nanop_nodexpr = lexqnod;
      int pos = 0;
      const void *exprv = parsexpr_nanoedit_mom (&npars, &pos);
      bool rawmode =            //
        (const void *) mom_unsync_item_get_phys_attr (thistatitm,
                                                      MOM_PREDEFITM (display))
        == (const void *) MOM_PREDEFITM (raw);
      MOM_DEBUGPRINTF (web,
                       "doparsecommand_nanoedit exprv=%s final pos#%d %s",
                       mom_value_cstring (exprv), pos,
                       rawmode ? "raw" : "cooked");
      mom_unsync_item_put_phys_attr (thistatitm, MOM_PREDEFITM (expression),
                                     exprv);
      struct mom_filebuffer_st *fb = mom_make_filebuffer ();
      showvalue_nanoedit_mom (fb, wexitm, thistatitm, exprv, 0);
      const char *exbuf = mom_filebuffer_strdup (fb, MOM_FILEBUFFER_CLOSE);
      MOM_DEBUGPRINTF (web, "doparsecommand_nanoedit exbuf=%s", exbuf);
      mom_wexch_puts (wexch, "{ \"html\": \"");
      mom_output_utf8_encoded (wexch->webx_outfil, exbuf, -1);
      mom_wexch_puts (wexch, "\",\n");
      MOM_WEXCH_PRINTF (wexch,
                        " \"expr_inside\": \"%s\" }\n",
                        mom_item_cstring (thistatitm));
      mom_wexch_reply (wexch, HTTP_OK, "application/json");
      goto end;
    }
end:
  if (npars.nanop_queitm)
    mom_item_unlock (npars.nanop_queitm);
  if (npars.nanop_delimitm)
    mom_item_unlock (npars.nanop_delimitm);
  memset (&npars, 0, sizeof (npars));
}                               /* end of doparsecommand_nanoedit_mom */



/********************* end of file nanoedit.c **********************/
