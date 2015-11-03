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


static void
showitemref_microedit_mom (struct mom_webexch_st *wexch,
                           struct mom_item_st *wexitm,
                           struct mom_item_st *thistatitm,
                           const struct mom_item_st *curitm)
{
  assert (wexch && wexch->va_itype == MOMITY_WEBEXCH);
  assert (wexitm && wexitm->va_itype == MOMITY_ITEM);
  assert (thistatitm && thistatitm->va_itype == MOMITY_ITEM);
  if (!curitm || curitm == MOM_EMPTY_SLOT)
    {
      MOM_WEXCH_PRINTF (wexch, "<span class='itemref_cl empty_cl'>~</span>");
    }
  else
    {
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
    case MOMITY_SET:
    case MOMITY_NODE:
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
  MOM_DEBUGPRINTF (web,
                   "doloadpage_microedit webr#%ld tkitm=%s wexitm=%s thistatitm=%s",
                   wexch->webx_count, mom_item_cstring (tkitm),
                   mom_item_cstring (wexitm), mom_item_cstring (thistatitm));
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
      MOM_DEBUGPRINTF (web, "doloadpage_microedit webr#%ld ix%d curatitm %s",
                       wexch->webx_count, ix, mom_item_cstring (curatitm));
      MOM_WEXCH_PRINTF (wexch, "<dt class='statattr_cl'>");
      showitemref_microedit_mom (wexch, wexitm, thistatitm, curatitm);
      MOM_WEXCH_PRINTF (wexch, " : </dt>\n");
#warning doloadpage_microedit should output the <dd> tag for the value
    }
  MOM_WEXCH_PRINTF (wexch, "</dl>\n");

#warning doloadpage_microedit should output according to thistatitm
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
  MOM_DEBUGPRINTF (web,
                   "momf_microedit tkitm=%s wexch #%ld meth %s fupath %s sessitm %s",
                   mom_item_cstring (tkitm), wexch->webx_count,
                   mom_webmethod_name (wexch->webx_meth),
                   onion_request_get_fullpath (wexch->webx_requ),
                   mom_item_cstring (wexch->webx_sessitm));
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
  if (thistatitm)
    mom_item_unlock (thistatitm);
  if (wexitm)
    mom_item_unlock (wexitm);
  mom_item_unlock (tkitm);
}                               /* end of momf_microedit */
