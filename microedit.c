// file microedit.c

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

#include "meltmoni.h"


enum microedit_sessfields_en
{
  mes_itmhset,
  mes__last
};

static void
showitemref_microedit_mom (struct mom_webexch_st *wexch,
                           struct mom_item_st *wexitm,
                           struct mom_item_st *thistatitm,
                           const struct mom_item_st *curitm)
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
      MOM_WEXCH_PRINTF (wexch, "<span class='itemref_cl empty_cl'>~</span>");
    }
  else
    {
      if (!
          (hsetitm =
           mom_dyncast_item (mom_vectvaldata_nth
                             (sessitm->itm_pcomp, mes_itmhset))))
        MOM_FATAPRINTF
          ("showitemref_microedit wexitm %s has sessitm %s without hashset",
           mom_item_cstring (wexitm), mom_item_cstring (sessitm));
      assert (mom_itype (hsetitm->itm_payload) == MOMITY_HASHSET);
      MOM_DEBUGPRINTF (web,
                       "showitemref_microedit wexitm=%s sessitm=%s hsetitm=%s curitm=%s",
                       mom_item_cstring (wexitm), mom_item_cstring (sessitm),
                       mom_item_cstring (hsetitm), mom_item_cstring (curitm));
      hsetitm->itm_payload = (struct mom_anyvalue_st *)
        mom_hashset_insert ((struct mom_hashset_st *) hsetitm->itm_payload,
                            (struct mom_item_st *) curitm);
      MOM_WEXCH_PRINTF (wexch, "<span class='itemref_cl'>%s</span>",
                        mom_item_cstring (curitm));
    }
}                               /* end of showitemref_microedit_mom */


static void
showvalue_microedit_mom (struct mom_webexch_st *wexch,
                         struct mom_item_st *wexitm,
                         struct mom_item_st *thistatitm, const void *pval)
{
  switch (mom_itype (pval))
    {
    case MOMITY_NONE:
      MOM_WEXCH_PRINTF (wexch, "<span class='nil_cl'>~</span>");
      return;
    case MOMITY_BOXINT:
      MOM_WEXCH_PRINTF (wexch, "<span class='momnumber_cl'>%lld</span>",
                        (long long) ((const struct mom_boxint_st *)
                                     pval)->boxi_int);
      return;
    case MOMITY_BOXDOUBLE:
      {
        char buf[48];
        memset (buf, 0, sizeof (buf));
        double x = ((const struct mom_boxdouble_st *) pval)->boxd_dbl;
        MOM_WEXCH_PRINTF
          (wexch, "<span class='momnumber_cl'>%s</span>",
           mom_double_to_cstr (x, buf, sizeof (buf)));
      }
      return;
    case MOMITY_ITEM:
      showitemref_microedit_mom (wexch, wexitm, thistatitm,
                                 (struct mom_item_st *) pval);
      return;
    case MOMITY_BOXSTRING:
      MOM_WEXCH_PRINTF (wexch, "&#8220;<span class='momstring_cl'>");   // “ U+201C LEFT DOUBLE QUOTATION MARK
      mom_output_utf8_html (wexch->webx_outfil,
                            ((const struct mom_boxstring_st *) pval)->cstr,
                            mom_size (pval), true);
      MOM_WEXCH_PRINTF (wexch, "</span>&#8221;");       // ” U+201D RIGHT DOUBLE QUOTATION MARK
      return;
    case MOMITY_TUPLE:
      {
        const struct mom_boxtuple_st *tup = pval;
        unsigned siz = mom_raw_size (tup);
        MOM_WEXCH_PRINTF (wexch, "<span class='momtuple_cl'>[");
        for (unsigned ix = 0; ix < siz; ix++)
          {
            if (ix > 0)
              MOM_WEXCH_PRINTF (wexch, " ");
            if (ix % 5 == 0)
              {
                if (ix > 0)
                  MOM_WEXCH_PRINTF (wexch,
                                    " &nbsp;<sup class='momindex_cl'>%d:</sup>",
                                    ix);
              };
            showitemref_microedit_mom (wexch, wexitm, thistatitm,
                                       tup->seqitem[ix]);
          }
        MOM_WEXCH_PRINTF (wexch, "]</span>");
      }
      return;
    case MOMITY_SET:
      {
        const struct mom_boxset_st *set = pval;
        unsigned siz = mom_raw_size (set);
        MOM_WEXCH_PRINTF (wexch, "<span class='momset_cl'>{");
        for (unsigned ix = 0; ix < siz; ix++)
          {
            if (ix > 0)
              MOM_WEXCH_PRINTF (wexch, " ");
            if (ix % 5 == 0)
              {
                if (ix > 0)
                  MOM_WEXCH_PRINTF (wexch,
                                    " &nbsp;<sup class='momindex_cl'>%d:</sup>",
                                    ix);
              };
            showitemref_microedit_mom (wexch, wexitm, thistatitm,
                                       set->seqitem[ix]);
          }
        MOM_WEXCH_PRINTF (wexch, "}</span>");
      }
      return;
    case MOMITY_NODE:
      {
        const struct mom_boxnode_st *nod = pval;
        unsigned siz = mom_raw_size (nod);
        MOM_WEXCH_PRINTF (wexch,
                          "<span class='momnode_cl'>*<span class='momconn_cl'>");
        showitemref_microedit_mom (wexch, wexitm, thistatitm,
                                   nod->nod_connitm);
        MOM_WEXCH_PRINTF (wexch, "</span> (");
        for (unsigned ix = 0; ix < siz; ix++)
          {
            if (ix > 0)
              MOM_WEXCH_PRINTF (wexch, " ");
            if (ix % 5 == 0)
              {
                if (ix > 0)
                  MOM_WEXCH_PRINTF (wexch,
                                    " &nbsp;<sup class='momindex_cl'>%d:</sup>",
                                    ix);
              };
            showvalue_microedit_mom (wexch, wexitm, thistatitm,
                                     nod->nod_sons[ix]);
          }
        MOM_WEXCH_PRINTF (wexch, ")");
      }
      return;
    default:
#warning showvalue_microedit_mom incomplete
      MOM_FATAPRINTF ("showvalue_microedit_mom incomplete pval:%s",
                      mom_value_cstring ((struct mom_hashedvalue_st *) pval));
      break;
    }
}                               /* end showvalue_microedit_mom */


static void
doloadpage_microedit_mom (struct mom_webexch_st *wexch,
                          struct mom_item_st *tkitm,
                          struct mom_item_st *wexitm,
                          struct mom_item_st *thistatitm)
{
  struct mom_item_st *sessitm = wexch->webx_sessitm;
  MOM_DEBUGPRINTF (web,
                   "doloadpage_microedit webr#%ld tkitm=%s wexitm=%s thistatitm=%s sessitm=%s",
                   wexch->webx_count, mom_item_cstring (tkitm),
                   mom_item_cstring (wexitm), mom_item_cstring (thistatitm),
                   mom_item_cstring (sessitm));
  MOM_WEXCH_PRINTF (wexch, "<h2>editing state <tt>%s</tt></h2>\n",
                    mom_item_cstring (thistatitm));
  char modbuf[64];
  memset (modbuf, 0, sizeof (modbuf));
  MOM_WEXCH_PRINTF (wexch, "<small>(modified %s)</small>",
                    mom_strftime_centi (modbuf, sizeof (modbuf) - 1, "%c %Z",
                                        thistatitm->itm_mtime));
  struct mom_hashmap_st *hmap = mom_hashmap_dyncast (thistatitm->itm_payload);
  const struct mom_boxset_st *atset = mom_hashmap_keyset (hmap);
  mom_assovaldata_dyncast (thistatitm->itm_payload);
  MOM_DEBUGPRINTF (web, "doloadpage_microedit webr#%ld atset %s",
                   wexch->webx_count,
                   mom_value_cstring ((struct mom_hashedvalue_st *) atset));
  unsigned nbat = mom_size (atset);
  MOM_WEXCH_PRINTF (wexch, "<dl class='attrlist_cl'>\n");
  for (unsigned ix = 0; ix < nbat; ix++)
    {
      const struct mom_item_st *curatitm = atset->seqitem[ix];
      const struct mom_hashedvalue_st *curval =
        mom_hashmap_get (hmap, curatitm);
      MOM_DEBUGPRINTF (web,
                       "doloadpage_microedit webr#%ld ix%d curatitm %s curval %s",
                       wexch->webx_count, ix, mom_item_cstring (curatitm),
                       mom_value_cstring (curval));
      MOM_WEXCH_PRINTF (wexch, "<dt class='statattr_cl'> &#8227; ");    /* ‣ U+2023 TRIANGULAR BULLET */
      showitemref_microedit_mom (wexch, wexitm, thistatitm, curatitm);
      MOM_WEXCH_PRINTF (wexch, " : </dt>\n");
      MOM_WEXCH_PRINTF (wexch, "<dd class='statval_cl'> &#9653; ");     /* ▵ U+25B5 WHITE UP-POINTING SMALL TRIANGLE */
      showvalue_microedit_mom (wexch, wexitm, thistatitm, curval);
      MOM_WEXCH_PRINTF (wexch, " ;</dd>\n");
    }
  MOM_WEXCH_PRINTF (wexch, "</dl>\n");
  {
    char timbuf[64];
    memset (timbuf, 0, sizeof (timbuf));
    MOM_WEXCH_PRINTF (wexch, "<p class='notice_cl'>(generated %s)</p>\n",
                      mom_now_strftime_centi (timbuf, sizeof (timbuf),
                                              "%c %Z"));
  }
  mom_wexch_reply (wexch, HTTP_OK, "text/html");
  MOM_DEBUGPRINTF (web,
                   "doloadpage_microedit done webr#%ld tkitm=%s",
                   wexch->webx_count, mom_item_cstring (tkitm));
}                               /* end doloadpage_microedit_mom */



extern mom_tasklet_sig_t momf_microedit;
const char momsig_microedit[] = "signature_tasklet";

void
momf_microedit (struct mom_item_st *tkitm)
{
  enum microedit_closoff_en
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
  MOM_DEBUGPRINTF (web, "momf_microedit start tkitm=%s tknod=%s",
                   mom_item_cstring (tkitm),
                   mom_value_cstring ((const struct mom_hashedvalue_st *)
                                      tknod));
  if (mom_itype (tknod) != MOMITY_NODE || mom_raw_size (tknod) < mec__last)
    {
      /// should not happen
      MOM_WARNPRINTF ("momf_microedit bad tknod %s",
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
                     "momf_microedit tkitm=%s wexitm=%s paylkind %s thistatitm=%s",
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
                   "momf_microedit tkitm=%s wexch #%ld meth %s fupath %s sessitm %s",
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
                       "momf_microedit tkitm=%s wexch #%ld new hsetitm %s",
                       mom_item_cstring (tkitm), wexch->webx_count,
                       mom_item_cstring (hsetitm));
    }
  else
    MOM_DEBUGPRINTF (web,
                     "momf_microedit tkitm=%s wexch #%ld got hsetitm %s",
                     mom_item_cstring (tkitm), wexch->webx_count,
                     mom_item_cstring (hsetitm));

  mom_item_lock (hsetitm);
  if (wexch->webx_meth == MOMWEBM_POST)
    {
      const char *doloadpage =
        onion_request_get_post (wexch->webx_requ, "do_loadpage");
      MOM_DEBUGPRINTF (web,
                       "momf_microedit tkitm=%s wexch #%ld doloadpage %s",
                       mom_item_cstring (tkitm), wexch->webx_count,
                       doloadpage);
      if (doloadpage)
        doloadpage_microedit_mom (wexch, tkitm, wexitm, thistatitm);
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
}                               /* end of momf_microedit */
