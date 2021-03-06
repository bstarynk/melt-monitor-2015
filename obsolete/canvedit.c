// file canvedit.c

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


enum canvedit_sessfields_en
{
  mes_itmhset,
  mes__last
};

static void
showitem_canvedit_mom (struct mom_webexch_st *wexch,
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
        MOM_WEXCH_PRINTF (wexch, "momc_nil_val()");
      else
        MOM_WEXCH_PRINTF (wexch, "momc_nil_ref()");
    }
  else
    {
      if (!
          (hsetitm =
           mom_dyncast_item (mom_vectvaldata_nth
                             (sessitm->itm_pcomp, mes_itmhset))))
        MOM_FATAPRINTF
          ("showitem_canvedit wexitm %s has sessitm %s without hashset",
           mom_item_cstring (wexitm), mom_item_cstring (sessitm));
      assert (mom_itype (hsetitm->itm_payload) == MOMITY_HASHSET);
      MOM_DEBUGPRINTF (web,
                       "showitem_canvedit wexitm=%s sessitm=%s hsetitm=%s curitm=%s",
                       mom_item_cstring (wexitm), mom_item_cstring (sessitm),
                       mom_item_cstring (hsetitm), mom_item_cstring (curitm));
      hsetitm->itm_payload = (struct mom_anyvalue_st *)
        mom_hashset_insert ((struct mom_hashset_st *) hsetitm->itm_payload,
                            (struct mom_item_st *) curitm);
      if (isval)
        MOM_WEXCH_PRINTF (wexch,
                          "momc_item_val('%s')", mom_item_cstring (curitm));
      else
        MOM_WEXCH_PRINTF (wexch,
                          "momc_item_ref('%s')", mom_item_cstring (curitm));
    }
}                               /* end of showitem_canvedit_mom */


static void
showvalue_canvedit_mom (struct mom_webexch_st *wexch,
                        struct mom_item_st *wexitm,
                        struct mom_item_st *thistatitm, const void *pval)
{
  switch (mom_itype (pval))
    {
    case MOMITY_NONE:
      MOM_WEXCH_PRINTF (wexch, "momc_nil_val()");
      return;
    case MOMITY_BOXINT:
      MOM_WEXCH_PRINTF (wexch, "momc_int(%lld)",
                        (long long) ((const struct mom_boxint_st *)
                                     pval)->boxi_int);
      return;
    case MOMITY_BOXDOUBLE:
      {
        char buf[48];
        memset (buf, 0, sizeof (buf));
        double x = ((const struct mom_boxdouble_st *) pval)->boxd_dbl;
        MOM_WEXCH_PRINTF
          (wexch, "momc_double('%s')",
           mom_double_to_cstr (x, buf, sizeof (buf)));
      }
      return;
    case MOMITY_ITEM:
      showitem_canvedit_mom (wexch, wexitm, thistatitm,
                             (struct mom_item_st *) pval, true);
      return;
    case MOMITY_BOXSTRING:
      mom_wexch_puts (wexch, "momc_string(\"");
      mom_output_utf8_html (wexch->webx_outfil,
                            ((const struct mom_boxstring_st *) pval)->cstr,
                            mom_size (pval), true);
      mom_wexch_puts (wexch, "\")");
      return;
    case MOMITY_TUPLE:
      {
        const struct mom_boxtuple_st *tup = pval;
        unsigned siz = mom_raw_size (tup);
        mom_wexch_puts (wexch, "momc_tuple([");
        for (unsigned ix = 0; ix < siz; ix++)
          {
            if (ix > 0)
              mom_wexch_puts (wexch, ",");
            if (ix % 4 == 0)
              mom_wexch_puts (wexch, "\n");
            else
              mom_wexch_puts (wexch, " ");
            showitem_canvedit_mom (wexch, wexitm, thistatitm,
                                   tup->seqitem[ix], false);
          }
        mom_wexch_puts (wexch, "])\n");
      }
      return;
    case MOMITY_SET:
      {
        const struct mom_boxset_st *set = pval;
        unsigned siz = mom_raw_size (set);
        mom_wexch_puts (wexch, "momc_set([");
        for (unsigned ix = 0; ix < siz; ix++)
          {
            if (ix > 0)
              mom_wexch_puts (wexch, ",");
            if (ix % 4 == 0)
              mom_wexch_puts (wexch, "\n");
            else
              mom_wexch_puts (wexch, " ");
            showitem_canvedit_mom (wexch, wexitm, thistatitm,
                                   set->seqitem[ix], false);
          }
        mom_wexch_puts (wexch, "])\n");
      }
      return;
    case MOMITY_NODE:
      {
        const struct mom_boxnode_st *nod = pval;
        unsigned siz = mom_raw_size (nod);
        /// we may want to have vertical and horizontal appearing nodes
        mom_wexch_puts (wexch, "momc_node(");
        showitem_canvedit_mom (wexch, wexitm, thistatitm,
                               nod->nod_connitm, false);
        mom_wexch_puts (wexch, ",\n[");
        for (unsigned ix = 0; ix < siz; ix++)
          {
            if (ix > 0)
              mom_wexch_puts (wexch, ",");
            if (ix % 2 == 0)
              mom_wexch_puts (wexch, "\n");
            else
              mom_wexch_puts (wexch, " ");
            showvalue_canvedit_mom (wexch, wexitm, thistatitm,
                                    nod->nod_sons[ix]);
          };
        mom_wexch_puts (wexch, "])");
      }
      return;
    default:
#warning showvalue_canvedit_mom incomplete
      MOM_FATAPRINTF ("showvalue_canvedit_mom incomplete pval:%s",
                      mom_value_cstring ((struct mom_hashedvalue_st *) pval));
      break;
    }
}                               /* end showvalue_canvedit_mom */


static void
dofillcanvas_canvedit_mom (struct mom_webexch_st *wexch,
                           struct mom_item_st *tkitm,
                           struct mom_item_st *wexitm,
                           struct mom_item_st *thistatitm)
{
  struct mom_item_st *sessitm = wexch->webx_sessitm;
  MOM_DEBUGPRINTF (web,
                   "dofillcanvas_canvedit webr#%ld tkitm=%s wexitm=%s thistatitm=%s sessitm=%s",
                   wexch->webx_count, mom_item_cstring (tkitm),
                   mom_item_cstring (wexitm), mom_item_cstring (thistatitm),
                   mom_item_cstring (sessitm));
  struct mom_hashmap_st *hmap = mom_hashmap_dyncast (thistatitm->itm_payload);
  const struct mom_boxset_st *atset = mom_hashmap_keyset (hmap);
  MOM_DEBUGPRINTF (web, "dofillcanvas_canvedit webr#%ld atset %s",
                   wexch->webx_count,
                   mom_value_cstring ((struct mom_hashedvalue_st *) atset));
  assert (atset != NULL && atset->va_itype == MOMITY_SET);
  {
    unsigned siz = mom_size (atset);
    MOM_WEXCH_PRINTF (wexch,
                      " console.log('dofillcanvas %s:%d siz %d; this=', this,\n"
                      "   ' momc_display_canvas=',momc_display_canvas);\n",
                      __FILE__, __LINE__, siz);
    {
      time_t now = 0;
      time (&now);
      struct tm nowtm = { };
      localtime_r (&now, &nowtm);
      char timbuf[64];
      memset (timbuf, 0, sizeof (timbuf));
      strftime (timbuf, sizeof (timbuf) - 1, "%T", &nowtm);
      MOM_WEXCH_PRINTF (wexch, "momc_display_canvas('%s#%ld',[\n", timbuf,
                        wexch->webx_count);
    }
    for (unsigned ix = 0; ix < siz; ix++)
      {
        const struct mom_item_st *curatitm = atset->seqitem[ix];
        const struct mom_hashedvalue_st *curval =
          mom_hashmap_get (hmap, curatitm);
        MOM_DEBUGPRINTF (web,
                         "dofillcanvas_canvedit webr#%ld ix%d curatitm %s curval %s",
                         wexch->webx_count, ix, mom_item_cstring (curatitm),
                         mom_value_cstring (curval));
        assert (curatitm != NULL && curatitm->va_itype == MOMITY_ITEM);
        assert (curval != NULL);
        if (ix > 0)
          mom_wexch_puts (wexch, ",");
        mom_wexch_puts (wexch, "\n momc_top_entry(");
        showitem_canvedit_mom (wexch, wexitm, thistatitm, curatitm, false);
        mom_wexch_puts (wexch, ", ");
        showvalue_canvedit_mom (wexch, wexitm, thistatitm, curval);
        mom_wexch_puts (wexch, ")");
      }
    mom_wexch_puts (wexch, "]); ////////end-display-canvas\n");
    MOM_WEXCH_PRINTF (wexch,
                      "console.log('dofillcanvas %s:%d this=', this);\n",
                      __FILE__, __LINE__);
  }
  {
    char modbuf[64];
    memset (modbuf, 0, sizeof (modbuf));
    double nowd = mom_clock_time (CLOCK_REALTIME);
    MOM_WEXCH_PRINTF (wexch, "addupdatehtml('displayed <tt>%s</tt>');\n",
                      mom_strftime_centi (modbuf, sizeof (modbuf) - 1,
                                          "%c %Z", nowd));
    MOM_WEXCH_PRINTF (wexch,
                      " console.log('dofillcanvas %s:%d done %s this=', this);\n",
                      __FILE__, __LINE__,
                      mom_strftime_centi (modbuf, sizeof (modbuf) - 1,
                                          "%T.__", nowd));
    MOM_DEBUGPRINTF (web, "dofillcanvas_canvedit webr#%ld nowd %.4f",
                     wexch->webx_count, nowd);
  }
  mom_wexch_reply (wexch, HTTP_OK,
                   /* could be "text/javascript", see
                      http://stackoverflow.com/a/4101763 */
                   "application/javascript; charset=UTF-8");
  MOM_DEBUGPRINTF (web,
                   "dofillcanvas_canvedit done webr#%ld tkitm=%s",
                   wexch->webx_count, mom_item_cstring (tkitm));
}                               /* end dofillcanvas_canvedit_mom */



extern mom_tasklet_sig_t momf_canvasedit;
const char momsig_canvasedit[] = "signature_tasklet";

void
momf_canvasedit (struct mom_item_st *tkitm)
{
  enum canvedit_closoff_en
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
  MOM_DEBUGPRINTF (web, "momf_canvedit start tkitm=%s tknod=%s",
                   mom_item_cstring (tkitm),
                   mom_value_cstring ((const struct mom_hashedvalue_st *)
                                      tknod));
  if (mom_itype (tknod) != MOMITY_NODE || mom_raw_size (tknod) < mec__last)
    {
      /// should not happen
      MOM_WARNPRINTF ("momf_canvedit bad tknod %s",
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
                     "momf_canvedit tkitm=%s wexitm=%s paylkind %s thistatitm=%s",
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
                   "momf_canvedit tkitm=%s wexch #%ld meth %s fupath %s sessitm %s",
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
                       "momf_canvedit tkitm=%s wexch #%ld new hsetitm %s",
                       mom_item_cstring (tkitm), wexch->webx_count,
                       mom_item_cstring (hsetitm));
    }
  else
    MOM_DEBUGPRINTF (web,
                     "momf_canvedit tkitm=%s wexch #%ld got hsetitm %s",
                     mom_item_cstring (tkitm), wexch->webx_count,
                     mom_item_cstring (hsetitm));

  mom_item_lock (hsetitm);
  if (wexch->webx_meth == MOMWEBM_POST)
    {
      const char *dofillcanvas =
        onion_request_get_post (wexch->webx_requ, "do_fillcanvas");
      MOM_DEBUGPRINTF (web,
                       "momf_canvedit tkitm=%s wexch #%ld do_fillcanvas %s",
                       mom_item_cstring (tkitm), wexch->webx_count,
                       dofillcanvas);
      if (dofillcanvas)
        dofillcanvas_canvedit_mom (wexch, tkitm, wexitm, thistatitm);
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
}                               /* end of momf_canvedit */
