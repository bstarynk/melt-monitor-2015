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
        mom_wexch_puts (wexch, " <span class='momnode_cl'>*");
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
        mom_wexch_puts (wexch, ")/<span>");
      }
      return;
    default:
      MOM_FATAPRINTF ("showvalue_nanoedit_mom incomplete pval:%s",
                      mom_value_cstring ((struct mom_hashedvalue_st *) pval));
      break;
    }
}                               /* end showvalue_nanoedit_mom */
