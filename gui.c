// file gui.c - primitive GTK graphical user interface

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

static GtkApplication *mom_gtkapp;
static GtkCssProvider *mom_gtkcssprov;
static GQuark mom_gquark;
static GtkTextBuffer *mom_obtextbuf;
static GtkTextTagTable *mom_tagtable;
static GtkTextTag *mom_tag_toptitle;    // tag for top text
static GtkTextTag *mom_tag_objtitle;    // tag for object title line
static GtkTextTag *mom_tag_objsubtitle; // tag for object subtitle line
static GtkTextTag *mom_tag_objname;     // tag for object names
static GtkTextTag *mom_tag_class;       // tag for class
static GtkTextTag *mom_tag_payload;     // tag for payload
static GtkTextTag *mom_tag_attr;        // tag for attributes
static GtkTextTag *mom_tag_idstart;     // tag for first characters of objids
static GtkTextTag *mom_tag_idrest;      // tag for rest of objids
static GtkTextTag *mom_tag_number;      // tag for numbers
static GtkTextTag *mom_tag_string;      // tag for strings
static GtkTextTag *mom_tag_sequence;    // tag for sequences (tuples & sets)
static GtkTextTag *mom_tag_time;        // tag for time
static GtkTextTag *mom_tag_comment;     // tag for comment
static GtkTextTag *mom_tag_index;       // tag for indexes
static GtkTextTag *mom_tag_json;        // tag for JSON
static GtkWidget *mom_appwin;
static GtkWidget *mom_tview1;
static GtkWidget *mom_tview2;
static GtkWidget *mom_checkitemcmd;
static bool mom_cmdcomplwithname;
static GtkWidget *mom_cmdcomplmenu;     // the (temporary) command completion menu
static gint mom_cmdcomplstartoff;       // start offset of word to be completed
static gint mom_cmdcomplendoff; // end offset of word to be completed
static mo_value_t mom_cmdcomplset;      // the set containing the current completion
static GtkWidget *mom_cmdwin;
static GtkTextBuffer *mom_cmdtextbuf;
static GtkTextTag *mom_cmdtag_fail;     // tag for failure rest in command
static GtkTextTag *mom_cmdtag_number;   // tag for number
static GtkTextTag *mom_cmdtag_string;   // tag for strings
static GtkTextTag *mom_cmdtag_delim;    // tag for delimiters
static GtkTextTag *mom_cmdtag_name;     // tag for existing named object
static GtkTextTag *mom_cmdtag_anon;     // tag for existing anonymous object
static GtkTextTag *mom_cmdtag_unknown;  // tag for unknown object
static GtkTextTag *mom_cmdtag_newglob;  // tag for new anonymous global object
static GtkTextTag *mom_cmdtag_newtrans; // tag for new anonymous transient object
static GtkTextTag *mom_cmdtag_newcomm;  // tag for new comment

static GtkWidget *mom_cmdtview;
static GtkWidget *mom_cmdstatusbar;

#define MOMGUI_IDSTART_LEN 7

// an object is displayed (once) when we are showing its entire
// content or it might be simply shown (without showing the content).
// an object can also be sub-displayed (once) inside the display of
// another object.

/** Both values below are mostly to keep Boehm's GC happy, so the
 * displayed or shown objects are never freed prematurely -even if no
 * value references them anymore.
 **/
// The association of displayed objects to depth
static mo_assovaldatapayl_ty *momgui_displayed_objasso;
#define MOMGUI_MAX_DEPTH 8
#define MOMGUI_INITIAL_DEPTH 3
// the hashset of shown object occurrences
static mo_hashsetpayl_ty *momgui_shown_obocchset;


static void
momgui_cmdstatus_printf (const char *fmt, ...)
__attribute__ ((format (printf, 1, 2)));

static inline int
momgui_gobrefcount (GObject * ob)
{
  if (!ob)
    return -9999;
  else
    return (int) (ob->ref_count);
}

#define MOMGUI_GOBREFCOUNT(Ob) momgui_gobrefcount(G_OBJECT(Ob))


// Each displayed object is displayed only once, and we keep the
// following GTK information about it
typedef struct momgui_dispobjinfo_st momgui_dispobjinfo_ty;
struct momgui_dispobjinfo_st
{
  // the displayed object reference
  mo_objref_t mo_gdo_dispobr;
  // for subdisplayed objects, the reference of the containing display
  mo_objref_t mo_gdo_inobr;
  // the start and end mark of the text buffer slice displaying that object
  GtkTextMark *mo_gdo_startmark;
  GtkTextMark *mo_gdo_endmark;
  // the anchor for the hidezoom minibutton
  GtkTextChildAnchor *mo_gdo_hidezoomanchor;
  // the hidezoom minibutton in tview1 & tview2
  GtkWidget *mo_gdo_hidezoombutton1;
  GtkWidget *mo_gdo_hidezoombutton2;
};
typedef struct momgui_dispctxt_st momgui_dispctxt_ty;
#define MOMGUI_DISPCTXT_MAGIC 0x38a104bdU       /*950076605u */
struct momgui_dispctxt_st
{                               // stack allocated 
  unsigned mo_gdx_nmagic;       // always MOMGUI_DISPCTXT_MAGIC
  unsigned short mo_gdx_maxdepth;       // maximal depth
  unsigned short mo_gdx_curdepth;       // current depth of object
  GtkTextIter mo_gdx_iter;      // current textiter
  mo_objref_t mo_gdx_obr;       // the containing object reference
  momgui_dispobjinfo_ty *mo_gdx_dispinfo;       // top display information
};                              /* end struct momgui_dispctxt_st */

// The glib hashtable mapping objects to above momgui_dispobjinfo_ty
static GHashTable *mom_dispobjinfo_hashtable;


// Each shown object can be shown in many occurrences. We keep the
// following GTK information about it
typedef struct momgui_shownobocc_st momgui_shownobocc_ty;
struct momgui_shownobocc_st
{
  // the shown object reference
  mo_objref_t mo_gso_showobr;
  // the particular tag for that object (e.g. we could highlight all
  // occurrences with that tag)
  GtkTextTag *mo_gso_txtag;
};
// The glib hashtable mapping objects to above
static GHashTable *mom_shownobjocc_hashtable;


// stack-allocated struct for parsing & decorating the command buffer
#define MOMGUI_CMDPARSE_MAGIC 0x3201f56b        /* 838989163 */
struct momgui_cmdparse_st
{
  unsigned mo_gcp_nmagic;       // always MOMGUI_CMDPARSE_MAGIC
  bool mo_gcp_onlyparse;        /* only do the parsing, don't create objects */
  bool mo_gcp_statusupdate;     // update the message in the cmdstatus bar
  GtkTextIter mo_gcp_curiter;
  mo_value_t mo_gcp_errstrv;    // error string value
  mo_hashsetpayl_ty *mo_gcp_setparsed;  /* hashset of parsed objects */
  mo_hashsetpayl_ty *mo_gcp_setcreated; /* hashset of created objects */
  jmp_buf mo_gcp_failjb;        // for escaping on error
};                              /* end of momgui_cmdparse_st */
static void
momgui_cmdparsefailure (struct momgui_cmdparse_st *cpar, int lineno);
#define MOMGUI_CMDPARSEFAIL_AT(Lin,Cpars,Fmt,...) do {	\
  struct momgui_cmdparse_st* cpars_##Lin = (Cpars);	\
  MOM_ASSERTPRINTF(cpars_##Lin				\
		   && cpars_##Lin->mo_gcp_nmagic	\
		      == MOMGUI_CMDPARSE_MAGIC,		\
		   "bad cpars " #Cpars " @%p",		\
		   cpars_##Lin);			\
  cpars_##Lin->mo_gcp_errstrv =				\
    mo_make_string_sprintf(Fmt,##__VA_ARGS__);		\
  momgui_cmdparsefailure(cpars_##Lin, Lin);		\
  longjmp(cpars_##Lin->mo_gcp_failjb, Lin);		\
 } while(0)
#define MOMGUI_CMDPARSEFAIL_AT_BIS(Lin,Cpars,Fmt,...) \
  MOMGUI_CMDPARSEFAIL_AT(Lin,Cpars,Fmt,##__VA_ARGS__)
#define MOMGUI_CMDPARSEFAIL(Cpars,Fmt,...) \
  MOMGUI_CMDPARSEFAIL_AT_BIS(__LINE__,Cpars,Fmt,##__VA_ARGS__)

// destructor for momgui_dispobjinfo_ty, for g_hash_table_new_full
static void
mom_destroy_dispobjinfo (momgui_dispobjinfo_ty * dinf)
{
  MOM_BACKTRACEPRINTF ("destroy_dispobjinfo dinf@%p dispobr=%s inobr=%s",
                       dinf, mo_objref_pnamestr (dinf->mo_gdo_dispobr),
                       mo_objref_pnamestr (dinf->mo_gdo_inobr));
  momgui_displayed_objasso =    //
    mo_assoval_remove (momgui_displayed_objasso, dinf->mo_gdo_dispobr);
  if (dinf->mo_gdo_hidezoombutton1)
    {
      gtk_widget_destroy (dinf->mo_gdo_hidezoombutton1);
      dinf->mo_gdo_hidezoombutton1 = NULL;
    }
  if (dinf->mo_gdo_hidezoombutton2)
    {
      gtk_widget_destroy (dinf->mo_gdo_hidezoombutton2);
      dinf->mo_gdo_hidezoombutton2 = NULL;
    }
  if (dinf->mo_gdo_startmark && dinf->mo_gdo_endmark
      && !gtk_text_mark_get_deleted (dinf->mo_gdo_startmark)
      && !gtk_text_mark_get_deleted (dinf->mo_gdo_endmark))
    {
      GtkTextIter startobit = { };
      GtkTextIter endobit = { };
      gtk_text_buffer_get_iter_at_mark (mom_obtextbuf, &startobit,
                                        dinf->mo_gdo_startmark);
      gtk_text_buffer_get_iter_at_mark (mom_obtextbuf, &endobit,
                                        dinf->mo_gdo_endmark);
      gtk_text_buffer_delete (mom_obtextbuf, &startobit, &endobit);
    }
  if (dinf->mo_gdo_startmark)
    {
      gtk_text_buffer_delete_mark (mom_obtextbuf, dinf->mo_gdo_startmark);
      dinf->mo_gdo_startmark = NULL;
    }
  if (dinf->mo_gdo_endmark)
    {
      gtk_text_buffer_delete_mark (mom_obtextbuf, dinf->mo_gdo_endmark);
      dinf->mo_gdo_endmark = NULL;
    };
  memset (dinf, 0, sizeof (*dinf));
  free (dinf);
}                               /* end of mom_destroy_dispobjinfo */

static void
mom_destroy_shownobocc (momgui_shownobocc_ty * shoc)
{
  MOM_BACKTRACEPRINTF ("destroy_shownobocc shoc@%p txtag@%p#r%d showobr=%s",
                       shoc, shoc->mo_gso_txtag,
                       MOMGUI_GOBREFCOUNT (shoc->mo_gso_txtag),
                       mo_objref_pnamestr (shoc->mo_gso_showobr));
  momgui_shown_obocchset =      //
    mo_hashset_remove (momgui_shown_obocchset, shoc->mo_gso_showobr);
  if (shoc->mo_gso_txtag)
    {
      GtkTextIter itstart = { };
      GtkTextIter itend = { };
      gtk_text_buffer_get_bounds (GTK_TEXT_BUFFER (mom_cmdtextbuf), &itstart,
                                  &itend);
      gtk_text_buffer_remove_tag (GTK_TEXT_BUFFER (mom_cmdtextbuf),
                                  shoc->mo_gso_txtag, &itstart, &itend);
      gtk_text_tag_table_remove (mom_tagtable, shoc->mo_gso_txtag);
      shoc->mo_gso_txtag = NULL;
    }
  memset (shoc, 0, sizeof (*shoc));
  MOM_INFORMPRINTF ("destroy_shownobocc shoc@%p free", shoc);
  free (shoc);
}                               /* end of mom_destroy_shownobocc */


// compare function for displayed objects
int
mom_dispobj_cmp (const void *p1, const void *p2)
{
  mo_objref_t ob1 = *(mo_objref_t *) p1;
  mo_objref_t ob2 = *(mo_objref_t *) p2;
  if (ob1 == ob2)
    return 0;
  MOM_ASSERTPRINTF (mo_dyncast_objref (ob1), "ob1 not object");
  MOM_ASSERTPRINTF (mo_dyncast_objref (ob2), "ob2 not object");
  mo_value_t nam1 = mo_objref_namev (ob1);
  mo_value_t nam2 = mo_objref_namev (ob2);
  if (nam1 && nam2)
    return strcmp (mo_string_cstr (nam1), mo_string_cstr (nam2));
  if (nam1)
    return -1;
  if (nam2)
    return 1;
  enum mo_space_en sp1 = mo_objref_space (ob1);
  enum mo_space_en sp2 = mo_objref_space (ob2);
  if (sp1 < sp2)
    return -1;
  if (sp1 > sp2)
    return 1;
  return mo_objref_cmp (ob1, ob2);
}                               /* end mom_dispobj_cmp */


void
mom_activate_app_menu (GtkMenuItem * menuitem MOM_UNUSED,
                       void *data MOM_UNUSED)
{
  bool cmdvisible = gtk_widget_get_visible (mom_cmdwin);
  MOM_INFORMPRINTF ("activateapp cmdvisible %s",
                    cmdvisible ? "true" : "false");
  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (mom_checkitemcmd),
                                  cmdvisible);
}                               /* end mom_activate_app_menu */

void
mom_toggled_command_shown (GtkCheckMenuItem * chkitm, void *data MOM_UNUSED)
{
  if (gtk_check_menu_item_get_active (chkitm))
    {
      MOM_INFORMPRINTF ("toggled_command show");
      gtk_widget_show_all (mom_cmdwin);
    }
  else
    {
      MOM_INFORMPRINTF ("toggled_command hide");
      gtk_widget_hide (mom_cmdwin);
    }
}                               /* end mom_toggled_command_shown */

#define MOM_DISPLAY_INDENTED_NEWLINE(Pdispx,Depth,...) do {	\
    momgui_dispctxt_ty*_pdx = (Pdispx);				\
    int _depth = (Depth);					\
    MOM_ASSERTPRINTF(_pdx != NULL				\
		     && _pdx->mo_gdx_nmagic			\
		     == MOMGUI_DISPCTXT_MAGIC,			\
		     "bad _pdx");				\
  gtk_text_buffer_insert_with_tags				\
  (mom_obtextbuf, &_pdx->mo_gdx_iter,				\
   "                \n"+(16-_depth%16), 1+_depth%16,		\
  ##__VA_ARGS__, NULL);						\
} while(0)

#define MOM_DISPLAY_OPENING(Pdispx,VarOff,Str,...) do {	\
    momgui_dispctxt_ty*_pdx = (Pdispx);			\
    MOM_ASSERTPRINTF(_pdx != NULL			\
		     && _pdx->mo_gdx_nmagic		\
		     == MOMGUI_DISPCTXT_MAGIC,		\
		     "bad _pdx");			\
    VarOff =						\
      gtk_text_iter_get_offset (&_pdx->mo_gdx_iter);	\
    gtk_text_buffer_insert_with_tags			\
      (mom_obtextbuf, &_pdx->mo_gdx_iter,		\
       (Str), strlen((Str)),				\
       ##__VA_ARGS__, NULL);				\
  } while(0)

#define MOM_DISPLAY_CLOSING(Pdispx,VarOff,Str,...) do {	\
    momgui_dispctxt_ty*_pdx = (Pdispx);			\
    MOM_ASSERTPRINTF(_pdx != NULL			\
		     && _pdx->mo_gdx_nmagic		\
		     == MOMGUI_DISPCTXT_MAGIC,		\
		     "bad _pdx");			\
    gtk_text_buffer_insert_with_tags			\
      (mom_obtextbuf, &_pdx->mo_gdx_iter,		\
       (Str), strlen((Str)),				\
       ##__VA_ARGS__, NULL);				\
    VarOff =						\
      gtk_text_iter_get_offset (&_pdx->mo_gdx_iter);	\
} while(0)

// display the object in the display context
void mom_display_ctx_object (momgui_dispctxt_ty * pdx, int depth);

static void
mom_display_objref (mo_objref_t obr, momgui_dispctxt_ty * pdx,
                    GtkTextTag * xobtag)
{
  MOM_ASSERTPRINTF (pdx != NULL
                    && pdx->mo_gdx_nmagic == MOMGUI_DISPCTXT_MAGIC,
                    "bad pdx");
  GtkTextIter *piter = &pdx->mo_gdx_iter;
  MOM_ASSERTPRINTF (mo_dyncast_objref (obr), "bad obr");
  MOM_ASSERTPRINTF (piter != NULL, "bad piter");
  mo_value_t namv = mo_objref_namev (obr);
  GtkTextTag *objtag = NULL;
  char idbuf[MOM_CSTRIDSIZ];
  memset (idbuf, 0, sizeof (idbuf));
  mo_objref_idstr (idbuf, obr);
  {
    momgui_shownobocc_ty *shoc =
      (momgui_shownobocc_ty *) g_hash_table_lookup (mom_shownobjocc_hashtable,
                                                    obr);
    if (!shoc)
      {
        shoc = calloc (1, sizeof (*shoc));
        if (!shoc)
          MOM_FATAPRINTF
            ("display_objref failed to allocate shownobocc_ty for obr=%s",
             mo_objref_pnamestr (obr));
        shoc->mo_gso_showobr = obr;
        shoc->mo_gso_txtag = NULL;
        MOM_INFORMPRINTF ("display_objref create shoc@%p for obr=%s",
                          shoc, mo_objref_pnamestr (obr));
        objtag = gtk_text_tag_table_lookup (mom_tagtable, idbuf);
        if (objtag)
          {
            shoc->mo_gso_txtag = objtag;
          }
        else
          {
            objtag = shoc->mo_gso_txtag
              = gtk_text_buffer_create_tag (mom_obtextbuf, idbuf,
                                            "font", "DejaVu Serif, Book",
                                            "background", "ivory", NULL);
          }
        g_hash_table_insert (mom_shownobjocc_hashtable, obr, shoc);
      }
    else
      {
        objtag = shoc->mo_gso_txtag;
      }
  }
  MOM_ASSERTPRINTF (objtag != NULL, "no obtag for %s",
                    mo_objref_pnamestr (obr));
  if (namv)
    {
      gtk_text_buffer_insert_with_tags (mom_obtextbuf, piter,
                                        mo_string_cstr (namv),
                                        mo_string_size (namv),
                                        objtag, mom_tag_objname,
                                        xobtag, NULL);
      return;
    }
  gtk_text_buffer_insert_with_tags (mom_obtextbuf, piter,
                                    idbuf,
                                    MOMGUI_IDSTART_LEN,
                                    objtag, mom_tag_idstart, xobtag, NULL);
  gtk_text_buffer_insert_with_tags (mom_obtextbuf, piter,
                                    idbuf + MOMGUI_IDSTART_LEN,
                                    MOM_CSTRIDLEN - MOMGUI_IDSTART_LEN,
                                    objtag, mom_tag_idrest, xobtag, NULL);
  mo_value_t commv = NULL;
  if ((commv = mo_objref_get_attr (obr, MOM_PREDEF (comment))) != NULL
      && mo_dyncast_string (commv))
    {
      char combuf[72];
      memset (combuf, 0, sizeof (combuf));
      const char *pc = NULL;
      const char *pn = NULL;
      char *pb = combuf;
      for (pc = mo_string_cstr (commv);
           pc && *pc
           && (((pn = g_utf8_next_char (pc)),
                pb - combuf < (int) sizeof (combuf) - 10)); pc = pn)
        {
          if (*pc == '\n')
            break;
          if (pb > combuf + 2 * sizeof (combuf) / 3 && isspace (*pc))
            break;
          if (pn == pc + 1)
            *(pb++) = *pc;
          else
            {
              memcpy (pb, pc, pn - pc);
              pb += pn - pc;
            }
        };
      gtk_text_buffer_insert_with_tags (mom_obtextbuf, piter,
                                        ": ", -1, objtag, xobtag, NULL);
      gtk_text_buffer_insert_with_tags (mom_obtextbuf, piter,
                                        combuf, pb - combuf,
                                        objtag, mom_tag_comment, xobtag,
                                        NULL);
      if (pc && *pc)
        gtk_text_buffer_insert_with_tags        //
          (mom_obtextbuf, piter, "\342\200\246",        // U+2026 HORIZONTAL ELLIPSIS …
           3, objtag, xobtag, NULL);
    }                           // end if commv is string and anonymous
}                               /* end mom_display_objref */


static void
mom_display_full_subobject (mo_objref_t subobr, momgui_dispctxt_ty * pdx,
                            int depth)
{
  MOM_ASSERTPRINTF (pdx != NULL
                    && pdx->mo_gdx_nmagic == MOMGUI_DISPCTXT_MAGIC,
                    "bad pdx");
  MOM_ASSERTPRINTF (mo_dyncast_objref (subobr),
                    "display_full_subobject: bad subobr");
  momgui_dispctxt_ty subdispx;
  memset (&subdispx, 0, sizeof (subdispx));
  momgui_dispobjinfo_ty *subdinf = calloc (1, sizeof (momgui_dispobjinfo_ty));
  if (!subdinf)
    MOM_FATAPRINTF
      ("failed to allocate display info for subobject %s in %s depth %d",
       mo_objref_pnamestr (subobr), mo_objref_pnamestr (pdx->mo_gdx_obr),
       depth + 1);
  subdinf->mo_gdo_dispobr = subobr;
  subdinf->mo_gdo_inobr = pdx->mo_gdx_obr;
  g_hash_table_insert (mom_dispobjinfo_hashtable, subobr, subdinf);
  subdispx.mo_gdx_nmagic = MOMGUI_DISPCTXT_MAGIC;
  subdispx.mo_gdx_maxdepth = pdx->mo_gdx_maxdepth;
  subdispx.mo_gdx_curdepth = depth;
  subdispx.mo_gdx_iter = pdx->mo_gdx_iter;
  subdispx.mo_gdx_obr = subobr;
  subdispx.mo_gdx_dispinfo = subdinf;
  mom_display_ctx_object (&subdispx, depth);
  pdx->mo_gdx_iter = subdispx.mo_gdx_iter;
}                               /* end mom_display_full_subobject */

static void
mom_display_value (mo_value_t val, momgui_dispctxt_ty * pdx,
                   int depth, GtkTextTag * valtag)
{
  MOM_ASSERTPRINTF (pdx != NULL
                    && pdx->mo_gdx_nmagic == MOMGUI_DISPCTXT_MAGIC,
                    "bad pdx");
  GtkTextIter *piter = &pdx->mo_gdx_iter;
  int maxdepth = pdx->mo_gdx_maxdepth;
  MOM_ASSERTPRINTF (depth >= 0 && depth <= maxdepth
                    && maxdepth <= MOMGUI_MAX_DEPTH,
                    "bad depth:%d maxdepth:%d MOMGUI_MAX_DEPTH:%d", depth,
                    maxdepth, MOMGUI_MAX_DEPTH);
  enum mo_valkind_en kd = mo_kind_of_value (val);
  switch (kd)
    {
    case mo_KNONE:
      gtk_text_buffer_insert_with_tags
        (mom_obtextbuf, piter, "~", 1, valtag, NULL);
      break;
    case mo_KINT:
      {
        char intbuf[32];
        memset (intbuf, 0, sizeof (intbuf));
        snprintf (intbuf, sizeof (intbuf), "%lld",
                  (long long) mo_value_to_int (val, 0));
        gtk_text_buffer_insert_with_tags (mom_obtextbuf, piter, intbuf, 1,
                                          mom_tag_number, valtag, NULL);
      }
      break;
    case mo_KSTRING:
      {
        const char *str = mo_string_cstr (val);
        unsigned siz = mo_string_size (val);
        gtk_text_buffer_insert_with_tags
          (mom_obtextbuf, piter, "\"", 1, valtag, NULL);
        gtk_text_buffer_insert_with_tags
          (mom_obtextbuf, piter, str, siz, mom_tag_string, valtag, NULL);
        gtk_text_buffer_insert_with_tags
          (mom_obtextbuf, piter, "\"", 1, valtag, NULL);
        MOM_DISPLAY_INDENTED_NEWLINE (pdx, depth, valtag);
      }
      break;
    case mo_KSET:
    case mo_KTUPLE:
      {
        bool istuple = kd == mo_KTUPLE;
        unsigned seqsiz = mo_sequence_size (val);
        bool showinside = depth < maxdepth;
        MOM_ASSERTPRINTF (seqsiz <= MOM_SIZE_MAX, "bad seqsiz %u", seqsiz);
        int startoff = 0;
        int endoff = 0;
        if (!istuple && seqsiz == 0 && val == mo_make_empty_set ())
          {                     // empty set, special case
            gtk_text_buffer_insert_with_tags
              (mom_obtextbuf, piter,
               "\342\210\205" /* U+2205 EMPTY SET ∅ */ , 3,
               mom_tag_sequence, valtag, NULL);
            break;
          };
        if (istuple)            // tuple
          MOM_DISPLAY_OPENING (pdx, startoff, "[", mom_tag_sequence, valtag);
        else                    // set
          MOM_DISPLAY_OPENING (pdx, startoff, "{", mom_tag_sequence, valtag);
        if (seqsiz > 0)
          {
            for (unsigned ix = 0; ix < seqsiz; ix++)
              {
                if (showinside || (ix % 5 == 0 && ix > 0))
                  {
                    MOM_DISPLAY_INDENTED_NEWLINE (pdx, depth + 1,
                                                  mom_tag_sequence, valtag);
                  }
                else
                  gtk_text_buffer_insert_with_tags
                    (mom_obtextbuf, piter, " ", 1, mom_tag_sequence, valtag,
                     NULL);
                mo_objref_t cursubobj = mo_sequence_nth (val, ix);
                if (!mo_dyncast_objref (cursubobj))
                  gtk_text_buffer_insert_with_tags
                    (mom_obtextbuf, piter, "~", 1, mom_tag_sequence, valtag,
                     NULL);
                else if (showinside && !mo_objref_namev (cursubobj)
                         && !mo_assoval_get (momgui_displayed_objasso,
                                             cursubobj))
                  {
                    mom_display_full_subobject (cursubobj, pdx, depth + 1);
                  }
                else
                  {
                    mom_display_objref (cursubobj, pdx, valtag);
                  }
              }
          }
        if (showinside)
          {
            MOM_DISPLAY_INDENTED_NEWLINE (pdx, depth + 1,
                                          mom_tag_sequence, valtag);
          }
        if (!istuple)           //set
          MOM_DISPLAY_CLOSING (pdx, endoff, "}", mom_tag_sequence, valtag);
        else                    // tuple
          MOM_DISPLAY_CLOSING (pdx, endoff, "]", mom_tag_sequence, valtag);
      }
      break;
    case mo_KOBJECT:
      {
        mo_objref_t curobj = (mo_objref_t) val;
        bool showinside = depth < maxdepth && !mo_objref_namev (curobj)
          && !mo_assoval_get (momgui_displayed_objasso, curobj);
        if (showinside)
          mom_display_full_subobject (curobj, pdx, depth + 1);
        else
          mom_display_objref (curobj, pdx, valtag);

      }
      break;
    }
}                               /* end of mom_display_value */


static void
mom_display_assoval (mo_assovaldatapayl_ty * asso, momgui_dispctxt_ty * pdx,
                     int depth)
{
  MOM_ASSERTPRINTF (pdx != NULL
                    && pdx->mo_gdx_nmagic == MOMGUI_DISPCTXT_MAGIC,
                    "bad pdx");
  GtkTextIter *piter = &pdx->mo_gdx_iter;
  int maxdepth = pdx->mo_gdx_maxdepth;
  asso = mo_dyncastpayl_assoval (asso);
  if (!asso)
    {
      gtk_text_buffer_insert_with_tags  //
        (mom_obtextbuf, piter, "_", 1, mom_tag_payload, NULL);
      return;
    }
  else if (depth >= maxdepth)
    {
      gtk_text_buffer_insert_with_tags  //
        (mom_obtextbuf, piter, "\342\200\246",  // U+2026 HORIZONTAL ELLIPSIS …
         3, mom_tag_payload, NULL);
      return;
    }
  mo_setvalue_ty *kset = (mo_setvalue_ty *) mo_assoval_keys_set (asso);
  unsigned siz = mo_set_size (kset);
  mo_objref_t *keyarr =
    mom_gc_alloc (mom_prime_above (siz + 1) * sizeof (mo_objref_t));
  memcpy (keyarr, kset->mo_seqobj, siz * sizeof (mo_objref_t));
  if (siz > 1)
    qsort (keyarr, siz, sizeof (mo_objref_t), mom_dispobj_cmp);
  char sizbuf[32];
  memset (sizbuf, 0, sizeof (sizbuf));
  snprintf (sizbuf, sizeof (sizbuf), "[assoval/%d]", siz);
  gtk_text_buffer_insert_with_tags (mom_obtextbuf, piter, sizbuf, -1,
                                    mom_tag_payload, mom_tag_index, NULL);
  for (unsigned ix = 0; ix < siz; ix++)
    {
      MOM_DISPLAY_INDENTED_NEWLINE (pdx, depth + 1, mom_tag_payload);
      gtk_text_buffer_insert_with_tags (mom_obtextbuf, piter,
                                        "\342\226\252 ", 4,
                                        mom_tag_payload, NULL);
      mo_objref_t keyobr = keyarr[ix];
      mom_display_objref (keyobr, pdx, mom_tag_payload);
      gtk_text_buffer_insert_with_tags (mom_obtextbuf, piter,
                                        " \342\226\271 ", 5,
                                        mom_tag_payload, NULL);
      mo_value_t curval = mo_assoval_get (asso, keyobr);
      mom_display_value (curval, pdx, depth + 1, mom_tag_payload);
    }
  MOM_DISPLAY_INDENTED_NEWLINE (pdx, depth, NULL);
}                               /* end mom_display_assoval  */


static void
mom_display_hashset (mo_hashsetpayl_ty * hset, momgui_dispctxt_ty * pdx,
                     int depth)
{
  MOM_ASSERTPRINTF (pdx != NULL
                    && pdx->mo_gdx_nmagic == MOMGUI_DISPCTXT_MAGIC,
                    "bad pdx");
  GtkTextIter *piter = &pdx->mo_gdx_iter;
  int maxdepth = pdx->mo_gdx_maxdepth;
  hset = mo_dyncastpayl_hashset (hset);
  if (!hset)
    {
      gtk_text_buffer_insert_with_tags  //
        (mom_obtextbuf, piter, "_", 1, mom_tag_payload, NULL);
      return;
    }
  else if (depth >= maxdepth)
    {
      gtk_text_buffer_insert_with_tags  //
        (mom_obtextbuf, piter, "\342\200\246",  // U+2026 HORIZONTAL ELLIPSIS …
         3, mom_tag_payload, NULL);
      return;
    }
  mo_setvalue_ty *set = (mo_setvalue_ty *) mo_hashset_elements_set (hset);
  unsigned siz = mo_set_size (set);
  mo_objref_t *elmarr =
    mom_gc_alloc (mom_prime_above (siz + 1) * sizeof (mo_objref_t));
  memcpy (elmarr, set->mo_seqobj, siz * sizeof (mo_objref_t));
  if (siz > 1)
    qsort (elmarr, siz, sizeof (mo_objref_t), mom_dispobj_cmp);
  {
    char sizbuf[32];
    memset (sizbuf, 0, sizeof (sizbuf));
    snprintf (sizbuf, sizeof (sizbuf), "[hashset/%d]", siz);
    gtk_text_buffer_insert_with_tags (mom_obtextbuf, piter, sizbuf, -1,
                                      mom_tag_payload, mom_tag_index, NULL);
  }
  gtk_text_buffer_insert_with_tags (mom_obtextbuf, piter, " \342\235\264",      // U+2774 MEDIUM LEFT CURLY BRACKET ORNAMENT ❴
                                    4, mom_tag_payload, NULL);
  for (unsigned ix = 0; ix < siz; ix++)
    {
      mo_objref_t elemobr = elmarr[ix];
      if (ix % 5 == 0)
        MOM_DISPLAY_INDENTED_NEWLINE (pdx, depth + 1, mom_tag_payload, NULL);
      else
        gtk_text_buffer_insert_with_tags (mom_obtextbuf, piter, " ", 1,
                                          mom_tag_payload, NULL);
      mom_display_objref (elemobr, pdx, mom_tag_payload);
    }
  if (siz > 0)
    gtk_text_buffer_insert_with_tags (mom_obtextbuf, piter, " ", 1,
                                      mom_tag_payload, NULL);
  gtk_text_buffer_insert_with_tags (mom_obtextbuf, piter, "\342\235\265",       // U+2775 MEDIUM RIGHT CURLY BRACKET ORNAMENT ❵
                                    3, mom_tag_payload, NULL);
  MOM_DISPLAY_INDENTED_NEWLINE (pdx, depth, NULL);
}                               /* end mom_display_hashset  */



static void
mom_display_list (mo_listpayl_ty * list, momgui_dispctxt_ty * pdx, int depth)
{
  MOM_ASSERTPRINTF (pdx != NULL
                    && pdx->mo_gdx_nmagic == MOMGUI_DISPCTXT_MAGIC,
                    "bad pdx");
  GtkTextIter *piter = &pdx->mo_gdx_iter;
  int maxdepth = pdx->mo_gdx_maxdepth;
  list = mo_dyncastpayl_list (list);
  if (!list)
    {
      gtk_text_buffer_insert_with_tags  //
        (mom_obtextbuf, piter, "_", 1, mom_tag_payload, NULL);
      return;
    }
  else if (depth >= maxdepth)
    {
      gtk_text_buffer_insert_with_tags  //
        (mom_obtextbuf, piter, "\342\200\246",  // U+2026 HORIZONTAL ELLIPSIS …
         3, mom_tag_payload, NULL);
      return;
    }
  unsigned len = mo_list_length (list);
  {
    char sizbuf[32];
    memset (sizbuf, 0, sizeof (sizbuf));
    snprintf (sizbuf, sizeof (sizbuf), "[list/%d]", len);
    gtk_text_buffer_insert_with_tags (mom_obtextbuf, piter, sizbuf, -1,
                                      mom_tag_payload, mom_tag_index, NULL);
    gtk_text_buffer_insert_with_tags (mom_obtextbuf, piter, "  \342\235\250",   // U+2768 MEDIUM LEFT PARENTHESIS ORNAMENT ❨
                                      4, mom_tag_payload, NULL);
    int cnt = 0;
    for (mo_listelem_ty * el = list->mo_lip_first; el != NULL;
         el = el->mo_lie_next)
      {
        for (int ix = 0; ix < MOM_LISTCHUNK_LEN; ix++)
          if (el->mo_lie_arr[ix])
            {
              MOM_DISPLAY_INDENTED_NEWLINE (pdx, depth + 1, mom_tag_payload);
              mom_display_value (el->mo_lie_arr[ix], pdx, depth + 1,
                                 mom_tag_payload);
              cnt++;
            }
      }
    gtk_text_buffer_insert_with_tags (mom_obtextbuf, piter, "  \342\235\250",   // U+2769 MEDIUM RIGHT PARENTHESIS ORNAMENT ❩
                                      4, mom_tag_payload, NULL);
  }
  MOM_DISPLAY_INDENTED_NEWLINE (pdx, depth, NULL);
}                               /* end mom_display_list  */


static void
mom_display_vectval (mo_vectvaldatapayl_ty * vect, momgui_dispctxt_ty * pdx,
                     int depth)
{
  MOM_ASSERTPRINTF (pdx != NULL
                    && pdx->mo_gdx_nmagic == MOMGUI_DISPCTXT_MAGIC,
                    "bad pdx");
  GtkTextIter *piter = &pdx->mo_gdx_iter;
  int maxdepth = pdx->mo_gdx_maxdepth;

  vect = mo_dyncastpayl_vectval (vect);
  if (!vect)
    {
      gtk_text_buffer_insert_with_tags  //
        (mom_obtextbuf, piter, "_", 1, mom_tag_payload, NULL);
      return;
    }
  else if (depth >= maxdepth)
    {
      gtk_text_buffer_insert_with_tags  //
        (mom_obtextbuf, piter, "\342\200\246",  // U+2026 HORIZONTAL ELLIPSIS …
         3, mom_tag_payload, NULL);
      return;
    }
  unsigned cnt = mo_vectval_count (vect);
  {
    char sizbuf[32];
    memset (sizbuf, 0, sizeof (sizbuf));
    snprintf (sizbuf, sizeof (sizbuf), "[vectval/%d]", cnt);
    gtk_text_buffer_insert_with_tags (mom_obtextbuf, piter, sizbuf, -1,
                                      mom_tag_payload, mom_tag_index, NULL);
  }
  gtk_text_buffer_insert_with_tags      //
    (mom_obtextbuf, piter, " \342\235\256",     // U+276E HEAVY LEFT-POINTING ANGLE QUOTATION MARK ORNAMENT ❮
     4, mom_tag_payload, NULL);
  for (int ix = 0; ix < (int) cnt; ix++)
    {
      {
        char indexbuf[32];
        memset (indexbuf, 0, sizeof (indexbuf));
        MOM_DISPLAY_INDENTED_NEWLINE (pdx, depth + 1, mom_tag_payload);
        snprintf (indexbuf, sizeof (indexbuf), "[%d] ", ix);
        gtk_text_buffer_insert_with_tags (mom_obtextbuf, piter, indexbuf, -1,
                                          mom_tag_payload, mom_tag_index,
                                          NULL);
      }
      mom_display_value (mo_vectval_nth (vect, ix), pdx, depth + 1,
                         mom_tag_payload);
    }
  gtk_text_buffer_insert_with_tags      //
    (mom_obtextbuf, piter, " \342\235\257",     // U+276F HEAVY RIGHT-POINTING ANGLE QUOTATION MARK ORNAMENT ❯
     4, mom_tag_payload, NULL);
  MOM_DISPLAY_INDENTED_NEWLINE (pdx, depth, NULL);
}                               /* end mom_display_vectval  */


static void
mom_display_objpayload (mo_objref_t obr, momgui_dispctxt_ty * pdx, int depth)
{
  MOM_ASSERTPRINTF (mo_dyncast_objref (obr) != NULL, "bad obr");
  MOM_ASSERTPRINTF (pdx != NULL
                    && pdx->mo_gdx_nmagic == MOMGUI_DISPCTXT_MAGIC,
                    "bad pdx");
  GtkTextIter *piter = &pdx->mo_gdx_iter;
  int maxdepth = pdx->mo_gdx_maxdepth;
  mo_objref_t paylkindobr = obr->mo_ob_paylkind;
  void *payldata = obr->mo_ob_payldata;
  MOM_ASSERTPRINTF (paylkindobr != NULL, "no payload");
  gtk_text_buffer_insert_with_tags      //
    (mom_obtextbuf, piter, "\342\200\275",      // U+203D INTERROBANG ‽
     4, mom_tag_payload, NULL);
  mom_display_objref (paylkindobr, pdx, mom_tag_payload);
  gtk_text_buffer_insert_with_tags      //
    (mom_obtextbuf, piter, ":", 1, mom_tag_payload, NULL);
  if (paylkindobr->mo_ob_class == MOM_PREDEF (signature_class))
    {
      char adbuf[32];
      memset (adbuf, 0, sizeof (adbuf));
      Dl_info dif;
      memset (&dif, 0, sizeof (dif));
      if (dladdr (payldata, &dif) && payldata == dif.dli_saddr)
        {
          gtk_text_buffer_insert_with_tags      //
            (mom_obtextbuf, piter, "\342\214\226",      // U+2316 POSITION INDICATOR ⌖
             3, mom_tag_payload, NULL);
          gtk_text_buffer_insert_with_tags      //
            (mom_obtextbuf, piter, dif.dli_sname, -1, mom_tag_payload, NULL);
          snprintf (adbuf, sizeof (adbuf), "@%p°", payldata);
          gtk_text_buffer_insert_with_tags      //
            (mom_obtextbuf, piter, adbuf, -1, mom_tag_payload, NULL);
          gtk_text_buffer_insert_with_tags
            (mom_obtextbuf, piter, dif.dli_fname, -1, mom_tag_payload, NULL);
        }
      else
        {
          snprintf (adbuf, sizeof (adbuf), "@@%p", payldata);
          gtk_text_buffer_insert_with_tags      //
            (mom_obtextbuf, piter, adbuf, -1, mom_tag_payload, NULL);
        }
    }                           // end if signature_class
  else
    {
#define MOM_NBCASE_PAYLOAD 307
#define CASE_PAYLOAD_MOM(Ob) momphash_##Ob % MOM_NBCASE_PAYLOAD:	\
	  if (paylkindobr != MOM_PREDEF(Ob)) goto defaultpayloadcase;	\
	  goto labpayl_##Ob; labpayl_##Ob
      switch (mo_objref_hash (paylkindobr) % MOM_NBCASE_PAYLOAD)
        {
        case CASE_PAYLOAD_MOM (payload_assoval):
          mom_display_assoval ((mo_assovaldatapayl_ty *) payldata,
                               pdx, depth + 1);
          break;
        case CASE_PAYLOAD_MOM (payload_hashset):
          mom_display_hashset ((mo_hashsetpayl_ty *) payldata,
                               pdx, depth + 1);
          break;
        case CASE_PAYLOAD_MOM (payload_list):
          mom_display_list ((mo_listpayl_ty *) payldata, pdx, depth + 1);
          break;
        case CASE_PAYLOAD_MOM (payload_vectval):
          mom_display_vectval ((mo_vectvaldatapayl_ty *) payldata,
                               pdx, depth + 1);
          break;
        case CASE_PAYLOAD_MOM (payload_value):
          mom_display_value ((mo_value_t) payldata, pdx, depth + 1,
                             mom_tag_payload);
          break;
        case CASE_PAYLOAD_MOM (payload_file):
          {
            FILE *fil = (FILE *) payldata;
            if (!fil)
              gtk_text_buffer_insert_with_tags  //
                (mom_obtextbuf, piter, "_", 1, mom_tag_payload, NULL);
            else
              {
                char fbuf[32];
                memset (fbuf, 0, sizeof (fbuf));
                snprintf (fbuf, sizeof (fbuf), "#%d@%p", fileno (fil),
                          (void *) fil);
                gtk_text_buffer_insert_with_tags        //
                  (mom_obtextbuf, piter, fbuf, -1, mom_tag_payload, NULL);
              }
          }
          break;
        case CASE_PAYLOAD_MOM (payload_gobject):
          {
            GObject *gob = (GObject *) payldata;
            if (!gob || gob == MOM_EMPTY_SLOT || !G_IS_OBJECT (gob))
              gtk_text_buffer_insert_with_tags  //
                (mom_obtextbuf, piter, "_", 1, mom_tag_payload, NULL);
            else
              {
                char gbuf[128];
                memset (gbuf, 0, sizeof (gbuf));
                snprintf (gbuf, sizeof (gbuf), "%.50s/%.50s@%p",
                          G_OBJECT_CLASS_NAME (gob), G_OBJECT_TYPE_NAME (gob),
                          (void *) gob);
                gtk_text_buffer_insert_with_tags        //
                  (mom_obtextbuf, piter, gbuf, -1, mom_tag_payload, NULL);
              }
          }
          break;
        case CASE_PAYLOAD_MOM (payload_buffer):
          {
            mo_bufferpayl_ty *bpy = (mo_bufferpayl_ty *) payldata;
            if (!bpy || bpy == MOM_EMPTY_SLOT
                || bpy->mo_buffer_nmagic != MOM_BUFFER_MAGIC)
              gtk_text_buffer_insert_with_tags  //
                (mom_obtextbuf, piter, "_", 1, mom_tag_payload, NULL);
            else
              {
                gtk_text_buffer_insert_with_tags        //
                  (mom_obtextbuf, piter, "\342\200\234",        //U+201C LEFT DOUBLE QUOTATION MARK “
                   3, mom_tag_payload, mom_tag_string, NULL);
                if (MOM_UNLIKELY (bpy->mo_buffer_memstream == NULL))
                  MOM_FATAPRINTF ("invalid closed buffer @%p for obr %s",
                                  bpy, mo_objref_pnamestr (obr));
                gunichar uc = 0;
                long cpos = ftell (bpy->mo_buffer_memstream);
                fflush (bpy->mo_buffer_memstream);
                const char *zone = bpy->mo_buffer_zone;
                const char *pchk = NULL;
                const char *bend = zone + cpos;
                for (const char *pc = zone; pc < bend && *pc;
                     pc = g_utf8_next_char (pc), uc = 0)
                  {
                    uc = g_utf8_get_char (pc);
                    if (uc == '\n' || !pchk || pc[1] == '\0')
                      {
                        MOM_DISPLAY_INDENTED_NEWLINE (pdx, depth,
                                                      mom_tag_payload,
                                                      mom_tag_string);
                        if (pchk)
                          gtk_text_buffer_insert_with_tags (mom_obtextbuf,
                                                            piter, pchk,
                                                            pc - pchk - 1,
                                                            mom_tag_payload,
                                                            mom_tag_string,
                                                            NULL);
                        pchk = (*pc == '\n') ? (pc + 1) : pc;
                      }
                  }
                gtk_text_buffer_insert_with_tags        //
                  (mom_obtextbuf, piter, "\342\200\235",        //U+201D RIGHT DOUBLE QUOTATION MARK ”
                   3, mom_tag_payload, mom_tag_string, NULL);
              }
          }
          break;
        case CASE_PAYLOAD_MOM (payload_json):
          {
            json_t *js = (json_t *) payldata;
            if (!js || js == MOM_EMPTY_SLOT)
              {
                gtk_text_buffer_insert_with_tags        //
                  (mom_obtextbuf, piter, "_", 1, mom_tag_payload,
                   mom_tag_json, NULL);
              }
            else if (depth >= maxdepth)
              {
                gtk_text_buffer_insert_with_tags        //
                  (mom_obtextbuf, piter, "\342\200\246", 3, mom_tag_payload,
                   mom_tag_json, NULL);
              }
            else
              {
                size_t siz = 1024;
                char *buf = calloc (1, siz);
                if (!buf)
                  MOM_FATAPRINTF
                    ("failed to calloc %zd for json payload display", siz);
                FILE *fmem = open_memstream (&buf, &siz);
                if (!fmem)
                  MOM_FATAPRINTF
                    ("failed to openmemstream %zd for json payload display",
                     siz);
                if (json_dumpf (js, fmem, JSON_INDENT (1) | JSON_SORT_KEYS))
                  MOM_FATAPRINTF
                    ("failed to json_dumpf for payload of object %s",
                     mo_objref_pnamestr (obr));
                fputc ('\n', fmem);
                fflush (fmem);
                long len = ftell (fmem);
                char *pchk = buf;
                char *pc = buf;
                while (pc < buf + len)
                  {
                    char *eol = strchr (pc, '\n');
                    if (!eol)
                      break;
                    MOM_DISPLAY_INDENTED_NEWLINE (pdx, depth,
                                                  mom_tag_payload,
                                                  mom_tag_json);
                    gtk_text_buffer_insert_with_tags (mom_obtextbuf,
                                                      piter, pchk,
                                                      eol - pchk - 1,
                                                      mom_tag_payload,
                                                      mom_tag_json, NULL);

                    pc = pchk = eol + 1;
                  }
                free (buf), buf = NULL;
                fclose (fmem);
                MOM_DISPLAY_INDENTED_NEWLINE (pdx, depth,
                                              mom_tag_payload, mom_tag_json);
              }
          }
          break;
        default:
        defaultpayloadcase:
          break;
        }
#undef MOM_NBCASE_PAYLOAD
#undef CASE_PAYLOAD_MOM
    }                           /* end else non-signature payload */
}                               /* end of mom_insert_objpayload_textbuf */



////////////////
void
mom_display_ctx_object (momgui_dispctxt_ty * pdx, int depth)
{
  MOM_ASSERTPRINTF (pdx != NULL
                    && pdx->mo_gdx_nmagic == MOMGUI_DISPCTXT_MAGIC,
                    "bad pdx");
  GtkTextIter *piter = &pdx->mo_gdx_iter;
  mo_objref_t obr = pdx->mo_gdx_obr;
  int maxdepth = pdx->mo_gdx_maxdepth;
  MOM_ASSERTPRINTF (depth >= 0 && depth <= maxdepth
                    && maxdepth <= MOMGUI_MAX_DEPTH,
                    "bad depth:%d maxdepth:%d MOMGUI_MAX_DEPTH:%d", depth,
                    maxdepth, MOMGUI_MAX_DEPTH);
  momgui_dispobjinfo_ty *dinf = pdx->mo_gdx_dispinfo;
  MOM_ASSERTPRINTF (dinf != NULL, "bad dinf");
  MOM_ASSERTPRINTF (mo_dyncast_objref (obr), "bad obr");
  MOM_ASSERTPRINTF (piter != NULL, "bad piter");
  MOM_ASSERTPRINTF (depth >= 0 && depth <= maxdepth
                    && maxdepth <= MOMGUI_MAX_DEPTH,
                    "bad depth %d maxdepth %d MOMGUI_MAX_DEPTH %d", depth,
                    maxdepth, MOMGUI_MAX_DEPTH);
  /// display the title bar
  GtkTextTag *curobjtitletag =
    (depth == 0) ? mom_tag_objtitle : mom_tag_objsubtitle;
  MOM_ASSERTPRINTF (dinf->mo_gdo_startmark == NULL,
                    "dinf got mo_gdo_startmark");
  dinf->mo_gdo_startmark =
    gtk_text_buffer_create_mark (mom_obtextbuf, NULL, piter, FALSE);
  gtk_text_buffer_get_end_iter (mom_obtextbuf, piter);
  MOM_DISPLAY_INDENTED_NEWLINE (pdx, depth, NULL);
  MOM_ASSERTPRINTF (dinf->mo_gdo_hidezoomanchor == NULL,
                    "dinf got mo_gdo_hidezoomanchor");
  {
    dinf->mo_gdo_hidezoomanchor =
      gtk_text_buffer_create_child_anchor (mom_obtextbuf, piter);
    GtkTextIter afterhidit = *piter;
    GtkTextIter beforehidit = afterhidit;
    gtk_text_iter_backward_cursor_position (&beforehidit);
    gtk_text_buffer_apply_tag (mom_obtextbuf, curobjtitletag, &beforehidit,
                               &afterhidit);
  }
  MOM_ASSERTPRINTF (dinf->mo_gdo_hidezoombutton1 == NULL,
                    "dinf got mo_gdo_hidezoombutton1");
  MOM_ASSERTPRINTF (dinf->mo_gdo_hidezoombutton2 == NULL,
                    "dinf got mo_gdo_hidezoombutton2");
  // see https://specifications.freedesktop.org/icon-naming-spec/icon-naming-spec-latest.html
  /** On Linux see also icons in /usr/share/icons/ e.g.
      /usr/share/icons/oxygen/base/16x16/actions/gtk-close.png or
      /usr/share/icons/gnome-colors-common/scalable/actions/gtk-close.svg
   **/
  /// perhaps we might embed some SVG icon in the code???
  dinf->mo_gdo_hidezoombutton1 = gtk_button_new_with_label ("\360\237\221\211");        //U+1F449 WHITE RIGHT POINTING BACKHAND INDEX 👉
  // we probably should show a menu about that
  //gtk_button_new_from_icon_name ("stock_delete", GTK_ICON_SIZE_BUTTON);
  dinf->mo_gdo_hidezoombutton2 =
    gtk_button_new_from_icon_name ("gtk-close", GTK_ICON_SIZE_BUTTON);
  gtk_text_view_add_child_at_anchor (GTK_TEXT_VIEW (mom_tview1),
                                     dinf->mo_gdo_hidezoombutton1,
                                     dinf->mo_gdo_hidezoomanchor);
  gtk_text_view_add_child_at_anchor (GTK_TEXT_VIEW (mom_tview2),
                                     dinf->mo_gdo_hidezoombutton2,
                                     dinf->mo_gdo_hidezoomanchor);
  gtk_widget_show (dinf->mo_gdo_hidezoombutton1);
  gtk_widget_show (dinf->mo_gdo_hidezoombutton2);
  enum mo_space_en spa = mo_objref_space (obr);
  switch (spa)
    {
    case mo_SPACE_NONE:
      gtk_text_buffer_insert_with_tags  //
        (mom_obtextbuf, piter, "\342\227\214",  // U+25CC DOTTED CIRCLE ◌
         3, curobjtitletag, NULL);
      break;
    case mo_SPACE_GLOBAL:
      break;
    case mo_SPACE_PREDEF:
      gtk_text_buffer_insert_with_tags  //
        (mom_obtextbuf, piter, "\342\200\242",  // U+2022 BULLET •
         3, curobjtitletag, NULL);
      break;
    case mo_SPACE_USER:
      gtk_text_buffer_insert_with_tags  //
        (mom_obtextbuf, piter, "\342\200\243",  // U+2023 TRIANGULAR BULLET ‣
         3, curobjtitletag, NULL);
      break;
    default:
      // this should not happen
      {
        MOM_WARNPRINTF ("display_the_object: strange space#%d of object %s",
                        (int) spa, mo_objref_pnamestr (obr));
        char spabuf[32];
        memset (spabuf, 0, sizeof (spabuf));
        snprintf (spabuf, sizeof (spabuf),      //
                  "\342\201\205"        // U+2045 LEFT SQUARE BRACKET WITH QUILL ⁅
                  "?%d" "\342\201\206"  // U+2046 RIGHT SQUARE BRACKET WITH QUILL ⁆
                  , (int) spa);
        gtk_text_buffer_insert_with_tags        //
          (mom_obtextbuf, piter, spabuf, -1, curobjtitletag, NULL);
      }
      break;
    };
  mo_value_t namv = mo_objref_namev (obr);
  if (namv)
    {
      gtk_text_buffer_insert_with_tags (mom_obtextbuf, piter,
                                        mo_string_cstr (namv),
                                        mo_string_size (namv),
                                        curobjtitletag,
                                        mom_tag_objname, NULL);
      gtk_text_buffer_insert_with_tags (mom_obtextbuf, piter,
                                        " = ", -1, curobjtitletag, NULL);
    }
  char idbuf[MOM_CSTRIDSIZ];
  memset (idbuf, 0, sizeof (idbuf));
  mo_objref_idstr (idbuf, obr);
  gtk_text_buffer_insert_with_tags (mom_obtextbuf, piter,
                                    idbuf,
                                    MOMGUI_IDSTART_LEN,
                                    curobjtitletag, mom_tag_idstart, NULL);
  gtk_text_buffer_insert_with_tags (mom_obtextbuf, piter,
                                    idbuf + MOMGUI_IDSTART_LEN,
                                    MOM_CSTRIDLEN - MOMGUI_IDSTART_LEN,
                                    curobjtitletag, mom_tag_idrest, NULL);
  mo_value_t commv = NULL;
  if (!namv
      && (commv = mo_objref_get_attr (obr, MOM_PREDEF (comment))) != NULL
      && mo_dyncast_string (commv))
    {
      char combuf[72];
      memset (combuf, 0, sizeof (combuf));
      const char *pc = NULL;
      const char *pn = NULL;
      char *pb = combuf;
      for (pc = mo_string_cstr (commv);
           pc && *pc
           && (((pn = g_utf8_next_char (pc)),
                pb - combuf < (int) sizeof (combuf) - 10)); pc = pn)
        {
          if (*pc == '\n')
            break;
          if (pb > combuf + 2 * sizeof (combuf) / 3 && isspace (*pc))
            break;
          if (pn == pc + 1)
            *(pb++) = *pc;
          else
            {
              memcpy (pb, pc, pn - pc);
              pb += pn - pc;
            }
        };
      gtk_text_buffer_insert_with_tags (mom_obtextbuf, piter,
                                        ":", -1, curobjtitletag, NULL);
      gtk_text_buffer_insert_with_tags (mom_obtextbuf, piter,
                                        combuf, pb - combuf,
                                        curobjtitletag,
                                        mom_tag_comment, NULL);
      if (pc && *pc)
        gtk_text_buffer_insert_with_tags        //
          (mom_obtextbuf, piter, "\342\200\246",        // U+2026 HORIZONTAL ELLIPSIS …
           3, curobjtitletag, NULL);
    }                           // end if commv is string and anonymous
  MOM_DISPLAY_INDENTED_NEWLINE (pdx, depth, curobjtitletag);
  //// display the mtime
  {
    char tibuf[72];
    memset (tibuf, 0, sizeof (tibuf));
    time_t nowt = 0;
    time (&nowt);
    struct tm nowtm = { };
    struct tm modtm = { };
    localtime_r (&nowt, &nowtm);
    localtime_r (&obr->mo_ob_mtime, &modtm);
    // 64800 seconds is 18 hours, so show mtime as e.g. ⌚ 13:45:12 
    if (obr->mo_ob_mtime > nowt - 64800 && obr->mo_ob_mtime <= nowt)
      strftime (tibuf, sizeof (tibuf), "\342\214\232 " "%T", &modtm);
    // 1728000 seconds is 20 days, so show mtime as e.g. ⌚ Aug 13, 14:25:57
    else if (obr->mo_ob_mtime > nowt - 1728000 && obr->mo_ob_mtime <= nowt)
      strftime (tibuf, sizeof (tibuf), "\342\214\232 " "%b %d, %T", &modtm);
    else if (obr->mo_ob_mtime > 0)
      // otherwise -long ago or in the future- show as ⌚ 2016 Aug 17, 09:45:01
      strftime (tibuf, sizeof (tibuf), "\342\214\232 " "%Y %b %d, %T",
                &modtm);
    else                        // unset time ⌚ ?
      strcpy (tibuf, "\342\214\232 ?");
    gtk_text_buffer_insert_with_tags (mom_obtextbuf, piter,
                                      tibuf, -1, mom_tag_time, NULL);
  }
  MOM_DISPLAY_INDENTED_NEWLINE (pdx, depth, NULL);
  /// display class
  mo_objref_t classobr = obr->mo_ob_class;
  if (classobr)
    {
      gtk_text_buffer_insert_with_tags  //
        (mom_obtextbuf, piter, "\342\254\237 ", // U+2B1F BLACK PENTAGON ⬟
         3, mom_tag_class, NULL);
      mom_display_objref (classobr, pdx, mom_tag_class);
      MOM_DISPLAY_INDENTED_NEWLINE (pdx, depth, NULL);
    }
  /// display attributes
  {
    mo_value_t attrset = mo_assoval_keys_set (obr->mo_ob_attrs);
    unsigned nbattrs = mo_set_size (attrset);
    mo_objref_t *attrarr =
      mom_gc_alloc (mom_prime_above (nbattrs) * sizeof (mo_objref_t));
    for (unsigned ix = 0; ix < nbattrs; ix++)
      attrarr[ix] = mo_set_nth (attrset, ix);
    qsort (attrarr, nbattrs, sizeof (mo_objref_t), mom_dispobj_cmp);
    for (unsigned aix = 0; aix < nbattrs; aix++)
      {
        mo_objref_t curattrobr = attrarr[aix];
        mo_value_t curval = mo_objref_get_attr (obr, curattrobr);
        gtk_text_buffer_insert_with_tags (mom_obtextbuf, piter, "\342\213\206", // U+22C6 STAR OPERATOR ⋆
                                          3, mom_tag_attr, NULL);
        mom_display_objref (curattrobr, pdx, mom_tag_attr);
        gtk_text_buffer_insert_with_tags (mom_obtextbuf, piter, "\342\207\276 ",        // U+21FE RIGHTWARDS OPEN-HEADED ARROW ⇾
                                          3, mom_tag_attr, NULL);
        mom_display_value (curval, pdx, depth + 1, NULL);
        MOM_DISPLAY_INDENTED_NEWLINE (pdx, depth, NULL);
      }
  }
  /// display components
  {
    int nbcomp = mo_vectval_count (obr->mo_ob_comps);
    char indexbuf[16];
    memset (indexbuf, 0, sizeof (indexbuf));
    for (int cix = 0; cix < nbcomp; cix++)
      {
        mo_value_t curcompv = mo_objref_get_comp (obr, cix);
        snprintf (indexbuf, sizeof (indexbuf), "&#%d: ", cix);
        gtk_text_buffer_insert_with_tags
          (mom_obtextbuf, piter, indexbuf, -1, mom_tag_index, NULL);
        mom_display_value (curcompv, pdx, depth + 1, NULL);
        MOM_DISPLAY_INDENTED_NEWLINE (pdx, depth, NULL);
      }
  }
  /// display payload
  if (obr->mo_ob_paylkind != NULL)
    {
      mom_display_objpayload (obr, pdx, depth);
    }
  /// end mark
  if (depth > 0)
    {                           // ending para
      MOM_DISPLAY_INDENTED_NEWLINE (pdx, depth, NULL);
      gtk_text_buffer_insert_with_tags (mom_obtextbuf, piter, " \342\201\226"   //U+2056 THREE DOT PUNCTUATION ⁖
                                        " end ", -1, mom_tag_objsubtitle,
                                        NULL);
      mom_display_objref (obr, pdx, mom_tag_objsubtitle);
      MOM_DISPLAY_INDENTED_NEWLINE (pdx, depth, mom_tag_objsubtitle);
    }
  MOM_DISPLAY_INDENTED_NEWLINE (pdx, depth, NULL);
  MOM_ASSERTPRINTF (dinf->mo_gdo_endmark == NULL, "dinf got mo_gdo_endmark");
  dinf->mo_gdo_endmark =
    gtk_text_buffer_create_mark (mom_obtextbuf, NULL, piter, TRUE);
}                               /* end mom_display_the_object */


// an expensive operation, we regenerate everything. But that might be
// enough for a while, because computer is fast enough to redisplay
// several thousand objects...
void
mo_gui_generate_object_text_buffer (void)
{
  MOM_INFORMPRINTF ("generate_object_text_buffer start");
  mo_setvalue_ty *dispset =
    (mo_setvalue_ty *) mo_assoval_keys_set (momgui_displayed_objasso);
  mo_assovaldatapayl_ty *oldispasso = momgui_displayed_objasso;
  unsigned nbdispob = mo_set_size (dispset);
  g_hash_table_remove_all (mom_dispobjinfo_hashtable);
  g_hash_table_remove_all (mom_shownobjocc_hashtable);
  momgui_displayed_objasso = mo_assoval_reserve (NULL,
                                                 2 * nbdispob + nbdispob / 3 +
                                                 20);
  momgui_shown_obocchset =
    mo_hashset_reserve (NULL, 3 * nbdispob + nbdispob / 2 + 40);
  gtk_text_buffer_set_text (mom_obtextbuf, "", 0);
  MOM_INFORMPRINTF
    ("generate_object_text_buffer cleared text of obtextbuf@%p",
     mom_obtextbuf);
  // sort the dispsetv in alphabetical order, or else obid order
  mo_objref_t *objarr =
    mom_gc_alloc (mom_prime_above (nbdispob + 1) * sizeof (mo_objref_t));
  if (nbdispob > 0)
    memcpy (objarr, dispset->mo_seqobj, nbdispob * sizeof (mo_objref_t));
  if (nbdispob > 1)
    qsort (objarr, nbdispob, sizeof (mo_objref_t), mom_dispobj_cmp);
  // display a common title string
  {
    char titlebuf[64];
    memset (titlebuf, 0, sizeof (titlebuf));
    snprintf (titlebuf, sizeof (titlebuf), "Display of %d objects \n",
              nbdispob);
    GtkTextIter tititer = { };
    gtk_text_buffer_get_start_iter (mom_obtextbuf, &tititer);
    gtk_text_buffer_insert_with_tags (mom_obtextbuf, &tititer,
                                      titlebuf, -1, mom_tag_toptitle, NULL);
    gtk_text_iter_backward_cursor_position (&tititer);
    char datibuf[80];
    memset (datibuf, 0, sizeof (datibuf));
    mom_now_strftime_centi (datibuf, sizeof (datibuf),
                            "(at %Y %b %d, %T.__ %Z)");
    gtk_text_buffer_insert_with_tags (mom_obtextbuf, &tititer, datibuf, -1,
                                      mom_tag_time, NULL);

  }
  // forward-add every displayed object
  for (int ix = 0; ix < (int) nbdispob; ix++)
    {
      mo_objref_t curobj = objarr[ix];
      MOM_ASSERTPRINTF (mo_dyncast_objref (curobj), "bad curobj ix#%d", ix);
      if (!mo_assoval_get (momgui_displayed_objasso, curobj))
        momgui_displayed_objasso =
          mo_assoval_put (momgui_displayed_objasso, curobj,
                          mo_int_to_value (MOMGUI_INITIAL_DEPTH));
    };
  // display each object
  for (int ix = 0; ix < (int) nbdispob; ix++)
    {
      momgui_dispctxt_ty dispctx;
      memset (&dispctx, 0, sizeof (dispctx));
      dispctx.mo_gdx_nmagic = MOMGUI_DISPCTXT_MAGIC;
      gtk_text_buffer_get_end_iter (mom_obtextbuf, &dispctx.mo_gdx_iter);
      gtk_text_buffer_insert (mom_obtextbuf, &dispctx.mo_gdx_iter, "\n", -1);
      mo_objref_t curobj = objarr[ix];
      dispctx.mo_gdx_obr = curobj;
      MOM_ASSERTPRINTF (mo_dyncast_objref (curobj), "bad curobj ix#%d", ix);
      int maxdepth = mo_value_to_int (mo_assoval_get (oldispasso, curobj), 0);
      if (maxdepth <= 0)
        maxdepth = 1;
      else if (maxdepth > MOMGUI_MAX_DEPTH)
        maxdepth = MOMGUI_MAX_DEPTH;
      dispctx.mo_gdx_maxdepth = maxdepth;
      MOM_INFORMPRINTF
        ("generate_object_text_buffer ix#%d curobj=%s maxdepth=%d", ix,
         mo_objref_pnamestr (curobj), maxdepth);
      momgui_displayed_objasso =
        mo_assoval_put (momgui_displayed_objasso, curobj,
                        mo_int_to_value (maxdepth));
      momgui_dispobjinfo_ty *di = calloc (1, sizeof (momgui_dispobjinfo_ty));
      if (!di)
        MOM_FATAPRINTF ("failed to allocate display info for ix#%d", ix);
      di->mo_gdo_dispobr = curobj;
      di->mo_gdo_inobr = NULL;
      g_hash_table_insert (mom_dispobjinfo_hashtable, curobj, di);
      dispctx.mo_gdx_dispinfo = di;
      mom_display_ctx_object (&dispctx, 0);
#warning FIXME: we should have some way to remove one displayed object
      gtk_text_buffer_insert (mom_obtextbuf, &dispctx.mo_gdx_iter, "\n", -1);
    }
  MOM_INFORMPRINTF ("generate_object_text_buffer end");
}                               /* end mo_gui_generate_object_text_buffer */


void
mo_gui_display_object (mo_objref_t ob)
{
  MOM_INFORMPRINTF ("gui_display_object start ob=%s",
                    mo_objref_pnamestr (ob));
  if (!mo_dyncast_objref (ob) || mom_without_gui)
    return;
  if (mo_assoval_get (momgui_displayed_objasso, ob))
    {
      MOM_INFORMPRINTF ("gui_display_object already displayed ob=%s",
                        mo_objref_pnamestr (ob));
      return;
    }
  momgui_displayed_objasso =    //
    mo_assoval_put (momgui_displayed_objasso, ob, mo_int_to_value (1));
  mo_gui_generate_object_text_buffer ();
  MOM_INFORMPRINTF ("gui_display_object end ob=%s", mo_objref_pnamestr (ob));
}                               /* end of mo_gui_display_object */

void
mo_gui_undisplay_object (mo_objref_t ob)
{
  if (!mo_dyncast_objref (ob) || mom_without_gui)
    return;
  if (!mo_assoval_get (momgui_displayed_objasso, ob))
    return;
  momgui_displayed_objasso = mo_assoval_remove (momgui_displayed_objasso, ob);
  mo_gui_generate_object_text_buffer ();
}                               /* end of mo_gui_undisplay_object */

static void
gobject_weak_notify_mom (gpointer data, GObject * oldgobj)
{
  mo_objref_t obr = data;
  MOM_ASSERTPRINTF (mo_dyncast_objref (obr), "bad obr");
  if (obr->mo_ob_paylkind == NULL)
    return;
  if (obr->mo_ob_payldata == NULL)
    {
      obr->mo_ob_paylkind = NULL;
      return;
    }
  MOM_ASSERTPRINTF (obr->mo_ob_paylkind == MOM_PREDEF (payload_gobject),
                    "non gobject obr@%p", obr);
  MOM_ASSERTPRINTF (obr->mo_ob_payldata == (void *) oldgobj, "bad payldata");
  obr->mo_ob_payldata = NULL;
  obr->mo_ob_paylkind = NULL;
}                               /* end gobject_weak_notify_mom */

// called from mo_objref_really_clear_payload
void
mo_objref_cleanup_gobject (mo_objref_t obr)
{
  MOM_ASSERTPRINTF (mo_dyncast_objref (obr), "bad obr");
  MOM_ASSERTPRINTF (obr->mo_ob_paylkind == MOM_PREDEF (payload_gobject),
                    "non gobject obr@%p", obr);
  if (obr->mo_ob_payldata)
    {
      GObject *gob = obr->mo_ob_payldata;
      obr->mo_ob_payldata = NULL;
      g_object_weak_unref (gob, gobject_weak_notify_mom, obr);
    }
}                               /* end mom_object_cleanup_gobject */

void
mo_objref_put_gobject_payload (mo_objref_t obr, GObject * gobj)
{
  if (!mo_dyncast_objref (obr))
    return;
  mo_objref_clear_payload (obr);
  if (!gobj)
    return;
  MOM_ASSERTPRINTF (G_IS_OBJECT (gobj),
                    "objref_put_gobject in obr=%s bad gobj@%p",
                    mo_objref_pnamestr (obr), gobj);
  obr->mo_ob_paylkind = MOM_PREDEF (payload_gobject);
  obr->mo_ob_payldata = gobj;
  g_object_weak_ref (gobj, gobject_weak_notify_mom, obr);
}                               /* end of mo_objref_put_gobject_payload */




/***
 We might have a `the_GUI` predefined object; using
 gtk_builder_new_from_string is not a very good idea.
***/

static void
mom_dumpexit_app (GtkMenuItem * menuitm MOM_UNUSED, gpointer data MOM_UNUSED)
{
  if (mom_dump_dir && !strcmp (mom_dump_dir, "-"))
    mom_dump_dir = ".";
  MOM_INFORMPRINTF ("dumpexit_app");
  g_application_quit (G_APPLICATION (mom_gtkapp));
}                               /* end of mom_dumpexit_app */

static void
mom_dump_app (GtkMenuItem * menuitm MOM_UNUSED, gpointer data MOM_UNUSED)
{
  if (mom_dump_dir && !strcmp (mom_dump_dir, "-"))
    mom_dump_dir = ".";
  MOM_INFORMPRINTF ("dump_app");
  mom_dump_state (NULL);
  MOM_INFORMPRINTF ("done dump_app");
}                               /* end mom_dump_app */

static void
mom_quit_app (GtkMenuItem * menuitm MOM_UNUSED, gpointer data MOM_UNUSED)
{
  MOM_INFORMPRINTF ("quit_app");
  GtkWidget *quitdialog =       //
    gtk_dialog_new_with_buttons ("Quit Monimelt?",
                                 GTK_WINDOW (mom_appwin),
                                 GTK_DIALOG_MODAL |
                                 GTK_DIALOG_DESTROY_WITH_PARENT,
                                 "Quit",
                                 GTK_RESPONSE_CLOSE,
                                 "Cancel",
                                 GTK_RESPONSE_CANCEL,
                                 NULL);
  gtk_window_set_default_size (GTK_WINDOW (quitdialog), 400, 250);
  gtk_widget_show_all (quitdialog);
  int res = gtk_dialog_run (GTK_DIALOG (quitdialog));
  if (res == GTK_RESPONSE_CLOSE)
    {
      mom_dump_dir = "-";
      g_application_quit (G_APPLICATION (mom_gtkapp));
    }
  gtk_widget_destroy (quitdialog);
  quitdialog = NULL;
}                               /* end of mom_quit_app */


static int
mom_obname_cmp (const void *p1, const void *p2)
{
  const char *s1 = mo_string_cstr (*(mo_value_t *) p1);
  const char *s2 = mo_string_cstr (*(mo_value_t *) p2);
  MOM_ASSERTPRINTF (s1 != NULL && s2 != NULL, "s1 or s2 not null");
  return strcmp (s1, s2);
}                               /* end mom_obname_cmp */

/// see http://stackoverflow.com/a/39420466/841108
static void
mom_obcombo_populator (GtkComboBox * combobox, gpointer ptr MOM_UNUSED)
{

  GtkListStore *combolist =     //
    GTK_LIST_STORE (gtk_combo_box_get_model (combobox));
  GtkWidget *entry = gtk_bin_get_child (GTK_BIN (combobox));
  const char *prefix = gtk_entry_get_text (GTK_ENTRY (entry));
  MOM_INFORMPRINTF ("obcombo_populator combobox@%p prefix='%s'",
                    combobox, prefix);
  const size_t preflen = prefix ? strlen (prefix) : 0;
  GtkTreeIter trit = { };
  gtk_list_store_clear (combolist);
  gtk_tree_model_get_iter_first (GTK_TREE_MODEL (combolist), &trit);
  if (preflen == 0 || isalpha (prefix[0]))
    {
      mo_value_t namsetv = (preflen == 0)
        ? mo_named_objects_set () : mo_named_set_of_prefix (prefix);
      int nbnam = mo_set_size (namsetv);
      mo_value_t *namarr = mom_gc_alloc ((1 + nbnam) * sizeof (mo_value_t));
      int cntnam = 0;
      for (int ix = 0; ix < nbnam; ix++)
        {
          mo_objref_t curobr = mo_set_nth (namsetv, ix);
          mo_value_t curnamv = mo_objref_namev (curobr);
          if (mo_dyncast_string (curnamv))
            namarr[cntnam++] = curnamv;
        }
      qsort (namarr, cntnam, sizeof (mo_value_t), mom_obname_cmp);
      for (int ix = 0; ix < cntnam; ix++)
        {
          gtk_list_store_append (combolist, &trit);
          gtk_list_store_set (combolist, &trit, 0,
                              mo_string_cstr (namarr[ix]), -1);
        }
    }
  else if (prefix[0] == '_' && isdigit (prefix[1])
           && isalnum (prefix[2]) && isalnum (prefix[3]))
    {
      mo_value_t anonsetv = mom_set_complete_objectid (prefix);
      int nbanon = mo_set_size (anonsetv);
      for (int ix = 0; ix < nbanon; ix++)
        {
          char bufid[MOM_CSTRIDSIZ];
          memset (bufid, 0, sizeof (bufid));
          mo_objref_idstr (bufid, mo_set_nth (anonsetv, ix));
          if (bufid[0])
            {
              gtk_list_store_append (combolist, &trit);
              gtk_list_store_set (combolist, &trit, 0, bufid, MOM_CSTRIDLEN);
            }
        }
    }
}                               /* end mom_obcombo_populator */

static GtkWidget *
mom_objectcombo (void)
{
  GtkListStore *comblis = gtk_list_store_new (1, G_TYPE_STRING);
  GtkWidget *combobox =
    gtk_combo_box_new_with_model_and_entry (GTK_TREE_MODEL (comblis));
  gtk_widget_set_size_request (combobox, 40, 10);
  gtk_combo_box_set_id_column (GTK_COMBO_BOX (combobox), 0);
  gtk_combo_box_set_entry_text_column (GTK_COMBO_BOX (combobox), 0);
  gtk_combo_box_set_button_sensitivity (GTK_COMBO_BOX (combobox),
                                        GTK_SENSITIVITY_ON);
  g_signal_connect (combobox, "popup", G_CALLBACK (mom_obcombo_populator),
                    NULL);
  return combobox;
}                               /* end mom_objectcombo */



static void
mom_display_edit (GtkMenuItem * menuitm MOM_UNUSED, gpointer data MOM_UNUSED)
{
  MOM_INFORMPRINTF ("display_edit");
  GtkWidget *displaydialog =    //
    gtk_dialog_new_with_buttons ("display:",
                                 GTK_WINDOW (mom_appwin),
                                 GTK_DIALOG_MODAL |
                                 GTK_DIALOG_DESTROY_WITH_PARENT,
                                 "Display",
                                 GTK_RESPONSE_OK,
                                 "Cancel",
                                 GTK_RESPONSE_CANCEL,
                                 NULL);
  gtk_window_set_default_size (GTK_WINDOW (displaydialog), 450, -1);
  GtkWidget *contarea =
    gtk_dialog_get_content_area (GTK_DIALOG (displaydialog));
  GtkWidget *hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 3);
  gtk_container_add (GTK_CONTAINER (contarea), hbox);
  gtk_box_pack_start (GTK_BOX (hbox), gtk_label_new ("object:"),
                      FALSE, FALSE, 1);
  GtkWidget *obcombo = mom_objectcombo ();
  GtkWidget *obentry = gtk_bin_get_child (GTK_BIN (obcombo));
  MOM_ASSERTPRINTF (GTK_IS_ENTRY (obentry), "bad obentry");
  gtk_box_pack_end (GTK_BOX (hbox), obentry, TRUE, TRUE, 1);
  gtk_widget_show_all (displaydialog);
  mo_objref_t displobr = NULL;
  int res = 0;
  do
    {
      displobr = NULL;
      res = gtk_dialog_run (GTK_DIALOG (displaydialog));
      if (res == GTK_RESPONSE_OK)
        {
          mo_hid_t hid = 0;
          mo_loid_t loid = 0;
          gchar *nams = gtk_entry_get_text (obentry);
          if (nams && mom_valid_name (nams))
            displobr = mo_find_named_cstr (nams);
          else if (mo_get_hi_lo_ids_from_cstring (&hid, &loid, nams))
            displobr = mo_objref_find_hid_loid (hid, loid);
          else
            displobr = NULL;
          MOM_INFORMPRINTF ("should display displobr=%s",
                            mo_objref_pnamestr (displobr));
          // display it
          mo_gui_display_object (displobr);
        }
      else
        break;
    }
  while (!displobr);
  gtk_widget_destroy (displaydialog);
  displaydialog = NULL;
}                               /* end mom_display_edit */

static void
mom_newob_edit (GtkMenuItem * menuitm MOM_UNUSED, gpointer data MOM_UNUSED)
{
  MOM_INFORMPRINTF ("newob_edit");
}                               /* end mom_newob_edit */

static bool
mom_stopgui (GtkWidget * w, GdkEvent * ev MOM_UNUSED,
             gpointer data MOM_UNUSED)
{
  MOM_BACKTRACEPRINTF ("stopgui w@%p/%s/%s",
                       w, G_OBJECT_CLASS_NAME (w), G_OBJECT_TYPE_NAME (w));
  GtkWidget *stopdialog =       //
    gtk_dialog_new_with_buttons ("Stop Monimelt?",
                                 GTK_WINDOW (mom_appwin),
                                 GTK_DIALOG_MODAL |
                                 GTK_DIALOG_DESTROY_WITH_PARENT,
                                 "Quit without dumping",
                                 GTK_RESPONSE_CLOSE,
                                 //
                                 "Cancel, so continue",
                                 GTK_RESPONSE_CANCEL,
                                 //
                                 "Dump & exit",
                                 GTK_RESPONSE_APPLY,
                                 NULL);
  gtk_widget_show_all (stopdialog);
  int res = 0;
  for (;;)
    {
      res = gtk_dialog_run (GTK_DIALOG (stopdialog));
      MOM_INFORMPRINTF ("stopgui res#%d", res);
      if (res == GTK_RESPONSE_APPLY)
        {
          if (mom_dump_dir && !strcmp (mom_dump_dir, "-"))
            mom_dump_dir = ".";
          MOM_INFORMPRINTF ("stopgui dump&exit mom_dump_dir=%s",
                            mom_dump_dir);
          g_application_quit (G_APPLICATION (mom_gtkapp));
          break;
        }
      else if (res == GTK_RESPONSE_CLOSE)
        {
          MOM_INFORMPRINTF ("stopgui dump&exit quit");
          mom_dump_dir = "-";
          g_application_quit (G_APPLICATION (mom_gtkapp));
          break;
        }
      else if (res == GTK_RESPONSE_DELETE_EVENT || res == GTK_RESPONSE_CANCEL)
        {
          MOM_INFORMPRINTF ("stopgui dump&exit cancel");
          gtk_widget_destroy (stopdialog);
          return false;
        };
    }
  gtk_widget_destroy (stopdialog);
  return true;                  /// dont propagate
}                               /* end mom_stopgui */

//////////////// create the GUI
static void
mom_initialize_gtk_tags_for_objects (void)
{
  MOM_ASSERTPRINTF (GTK_IS_TEXT_BUFFER (mom_obtextbuf), "bad obtextbuf");
  mom_tag_toptitle =
    gtk_text_buffer_create_tag (mom_obtextbuf,
                                "toptitle",
                                "justification", GTK_JUSTIFY_CENTER,
                                "pixels-above-lines", 2,
                                "pixels-below-lines", 4,
                                "foreground", "navy",
                                "paragraph-background", "lightyellow",
                                "font", "Helvetica Bold", "scale", 1.5, NULL);
  mom_tag_objtitle =
    gtk_text_buffer_create_tag (mom_obtextbuf,
                                "objtitle",
                                "justification", GTK_JUSTIFY_CENTER,
                                "pixels-above-lines", 4,
                                "pixels-below-lines", 2,
                                "foreground", "brown",
                                "font", "Sans Bold",
                                "paragraph-background", "lightcyan",
                                "scale", 1.3, NULL);
  mom_tag_objsubtitle =
    gtk_text_buffer_create_tag (mom_obtextbuf,
                                "objsubtitle",
                                "justification", GTK_JUSTIFY_CENTER,
                                "pixels-above-lines", 1,
                                "pixels-below-lines", 1,
                                "foreground", "chocolate4",
                                "font", "Sans Bold",
                                "paragraph-background", "lightgoldenrod",
                                "scale", 1.15, NULL);
  mom_tag_objname =
    gtk_text_buffer_create_tag (mom_obtextbuf,
                                "objname",
                                "font", "Arial Bold",
                                "foreground", "sienna", NULL);
  mom_tag_idstart =
    gtk_text_buffer_create_tag (mom_obtextbuf,
                                "idstart",
                                "font", "Courier Bold",
                                "foreground", "darkgreen", NULL);
  mom_tag_idrest =
    gtk_text_buffer_create_tag (mom_obtextbuf,
                                "idrest",
                                "family", "Courier",
                                "foreground", "darkolivegreen",
                                "scale", 0.75, NULL);
  mom_tag_number =
    gtk_text_buffer_create_tag (mom_obtextbuf,
                                "number",
                                "family", "Courier New",
                                "foreground", "slateblue", NULL);
  mom_tag_string =
    gtk_text_buffer_create_tag (mom_obtextbuf,
                                "string",
                                "font", "DejaVu Sans Mono, Oblique",
                                "foreground", "saddlebrown", NULL);
  mom_tag_sequence =
    gtk_text_buffer_create_tag (mom_obtextbuf,
                                "sequence",
                                "font", "Luxi Mono, Bold",
                                "foreground", "tomato", NULL);
  mom_tag_attr =
    gtk_text_buffer_create_tag (mom_obtextbuf,
                                "attr",
                                "scale", 1.05,
                                "background", "lavender",
                                "weight", PANGO_WEIGHT_SEMIBOLD, NULL);
  mom_tag_class =
    gtk_text_buffer_create_tag (mom_obtextbuf,
                                "class",
                                "scale", 1.05,
                                "background", "bisque1",
                                "weight", PANGO_WEIGHT_SEMIBOLD, NULL);
  mom_tag_payload =
    gtk_text_buffer_create_tag (mom_obtextbuf,
                                "payload",
                                "scale", 0.95,
                                "background", "pink",
                                "weight", PANGO_WEIGHT_SEMIBOLD, NULL);
  mom_tag_time =
    gtk_text_buffer_create_tag (mom_obtextbuf,
                                "time",
                                "font", "Times, Italic",
                                "scale", 0.85,
                                "foreground", "skyblue4", NULL);
  mom_tag_comment =
    gtk_text_buffer_create_tag (mom_obtextbuf,
                                "comment",
                                "font", "Verdana, Italic",
                                "scale", 0.70,
                                "foreground", "slategrey", NULL);
  mom_tag_index =
    gtk_text_buffer_create_tag (mom_obtextbuf,
                                "index",
                                "font", "DejaVu Sans, Condensed",
                                "scale", 0.85,
                                "foreground", "mediumturquoise", "rise", 3,
                                NULL);
  mom_tag_json =
    gtk_text_buffer_create_tag (mom_obtextbuf, "json", "font",
                                "Inconsolata, Medium", "scale", 0.83,
                                "foreground", "khaki1", NULL);
}                               /* end of mom_initialize_gtk_tags_for_objects */


/* below ten completions, we offer a menu */
#define MOMGUI_COMPLETION_MENU_MAX 10
/* below a thousand completions for a name, we try to insert a common prefix */
#define MOMGUI_COMPLETION_MANY_NAMES 1000
static void
momgui_completecmdix (GtkMenuItem * itm MOM_UNUSED, gpointer ixad)
{
  intptr_t ixl = (intptr_t) ixad;
  if (ixl < 0 || ixl > 2 * MOMGUI_COMPLETION_MENU_MAX
      || ixl >= (intptr_t) mo_set_size (mom_cmdcomplset)
      || mom_cmdcomplstartoff < 0
      || mom_cmdcomplendoff <= mom_cmdcomplstartoff
      || mom_cmdcomplendoff > gtk_text_buffer_get_char_count (mom_cmdtextbuf))
    {
      MOM_WARNPRINTF
        ("completion index ixl=%ld out of bound, or bad start %d & end %d offsets with compl.set size %d",
         (long) ixl, (int) mom_cmdcomplstartoff, (int) mom_cmdcomplendoff,
         mo_set_size (mom_cmdcomplset));
      mom_cmdcomplset = NULL;
      mom_cmdcomplstartoff = 0;
      mom_cmdcomplendoff = 0;
      return;
    }
  mo_objref_t complobj = mo_set_nth (mom_cmdcomplset, (int) ixl);
  GtkTextIter startwit = { };
  GtkTextIter endwit = { };
  gtk_text_buffer_get_iter_at_offset (mom_cmdtextbuf, &startwit,
                                      mom_cmdcomplstartoff);
  gtk_text_buffer_get_iter_at_offset (mom_cmdtextbuf, &endwit,
                                      mom_cmdcomplendoff);
  gtk_text_buffer_begin_user_action (mom_cmdtextbuf);
  gtk_text_buffer_delete (mom_cmdtextbuf, &startwit, &endwit);
  if (mom_cmdcomplwithname)
    {
      gtk_text_buffer_insert (mom_cmdtextbuf, &endwit,
                              mo_objref_pnamestr (complobj), -1);
    }
  else
    {
      char bufid[MOM_CSTRIDSIZ];
      mo_value_t commv = NULL;
      memset (bufid, 0, sizeof (bufid));
      mo_objref_idstr (bufid, complobj);
      gtk_text_buffer_insert (mom_cmdtextbuf, &endwit, bufid, MOM_CSTRIDLEN);
      if ((commv =
           mo_objref_get_attr (complobj,
                               MOM_PREDEF (comment))) != NULL
          && mo_dyncast_string (commv))
        {
          char combuf[72];
          memset (combuf, 0, sizeof (combuf));
          const char *pc = NULL;
          const char *pn = NULL;
          char *pb = combuf;
          for (pc = mo_string_cstr (commv);
               pc && *pc
               && (((pn = g_utf8_next_char (pc)),
                    pb - combuf < (int) sizeof (combuf) - 10)); pc = pn)
            {
              if (*pc == '\n')
                break;
              if (pb > combuf + 2 * sizeof (combuf) / 3 && isspace (*pc))
                break;
              if (pn == pc + 1)
                *(pb++) = *pc;
              else
                {
                  memcpy (pb, pc, pn - pc);
                  pb += pn - pc;
                }
            };
          momgui_cmdstatus_printf ("completion %s: %s", bufid, combuf);
        };
    }
  gtk_text_buffer_place_cursor (mom_cmdtextbuf, &endwit);
  gtk_text_buffer_end_user_action (mom_cmdtextbuf);
  if (mom_cmdcomplmenu)
    {
      gtk_widget_destroy (mom_cmdcomplmenu);
      mom_cmdcomplmenu = NULL;
    }
  mom_cmdcomplset = NULL;
  mom_cmdcomplstartoff = 0;
  mom_cmdcomplendoff = 0;
}                               /* end of momgui_completecmdix */


// for "key-release-event" signal to mom_cmdtview, handle completion with TAB key
static bool
momgui_cmdtextview_keyrelease (GtkWidget * widg MOM_UNUSED, GdkEvent * ev,
                               void *data MOM_UNUSED)
{
  if (ev && ev->type == GDK_KEY_RELEASE
      && ((GdkEventKey *) ev)->keyval == GDK_KEY_Tab)
    {
      if (MOM_UNLIKELY (mom_cmdcomplmenu))
        {
          gtk_widget_destroy (mom_cmdcomplmenu);
          mom_cmdcomplmenu = NULL;
          mom_cmdcomplstartoff = 0;
          mom_cmdcomplendoff = 0;
          mom_cmdcomplwithname = false;
        }
      GtkTextIter itcurs = { };
      gtk_text_buffer_get_iter_at_mark  //
        (mom_cmdtextbuf,
         &itcurs, gtk_text_buffer_get_insert (mom_cmdtextbuf));
      GtkTextIter itbword = itcurs;
      if (!gtk_text_iter_starts_line (&itbword))
        gtk_text_iter_backward_char (&itbword);
      gunichar bwordc = 0;
      int nbackw = 0;
      do
        {
          bwordc = gtk_text_iter_get_char (&itbword);
          MOM_INFORMPRINTF ("itbword C%u'%c' line %d offset %d nbackw#%d",
                            bwordc, (bwordc > ' '
                                     && bwordc < 127) ? (char) bwordc : '?',
                            gtk_text_iter_get_line (&itcurs),
                            gtk_text_iter_get_line_offset (&itcurs), nbackw);
          if (gtk_text_iter_starts_line (&itbword)
              || !gtk_text_iter_backward_char (&itbword))
            break;
          nbackw++;
          bwordc = gtk_text_iter_get_char (&itbword);
        }
      while (bwordc > 0 && bwordc < 127
             && (isalnum (bwordc) || bwordc == '_'));
      if (nbackw > 0 && !gtk_text_iter_starts_line (&itbword))
        gtk_text_iter_forward_char (&itbword);
      char *wordtxt =
        gtk_text_buffer_get_text (mom_cmdtextbuf, &itbword, &itcurs, false);
      gunichar cursc = gtk_text_iter_get_char (&itcurs);        // character just after the cursor, so useless
      MOM_INFORMPRINTF
        ("cmdtextview_keyrelease block TAB, curschar#%u'%c', line %d offset %d bword %d wordtxt '%s'",
         (unsigned) cursc, (cursc >= (unsigned) ' '
                            && cursc < 127U) ? (char) cursc : '?',
         gtk_text_iter_get_line (&itcurs),
         gtk_text_iter_get_line_offset (&itcurs),
         gtk_text_iter_get_line_offset (&itbword), wordtxt);
      MOM_INFORMPRINTF
        ("cmdtextview_keyrelease block TAB bwordc#%u'%c' wordtxt '%s'",
         (unsigned) bwordc, (bwordc >= (unsigned) ' '
                             && bwordc < 127U) ? (char) bwordc : '?',
         wordtxt);
      unsigned complsiz = 0;
      if (isalpha (wordtxt[0]))
        {
          /// should do a name completion
          mom_cmdcomplwithname = true;
          mom_cmdcomplset = (mo_value_t) mo_named_set_of_prefix (wordtxt);
        }
      else if (wordtxt[0] == '_' && isdigit (wordtxt[1])
               && isalnum (wordtxt[2]) && isalnum (wordtxt[3]))
        {
          /// should do an objid completion
          mom_cmdcomplwithname = false;
          mom_cmdcomplset = (mo_value_t) mom_set_complete_objectid (wordtxt);
        }
      if (!mo_dyncast_set (mom_cmdcomplset))
        momgui_cmdstatus_printf ("cannot complete: %s.", wordtxt);
      else if ((complsiz = mo_set_size (mom_cmdcomplset)) == 1)
        {
          /// single completion
          mo_objref_t compl1obj = mo_set_nth (mom_cmdcomplset, 0);
          MOM_ASSERTPRINTF (mo_dyncast_objref (compl1obj), "bad compl1obj");
          gtk_text_buffer_begin_user_action (mom_cmdtextbuf);
          gtk_text_buffer_delete (mom_cmdtextbuf, &itbword, &itcurs);
          if (isalpha (wordtxt[0]))
            gtk_text_buffer_insert (mom_cmdtextbuf, &itcurs,
                                    mo_objref_pnamestr (compl1obj), -1);
          else
            {
              char bufid[MOM_CSTRIDSIZ];
              mo_value_t commv = NULL;
              memset (bufid, 0, sizeof (bufid));
              mo_objref_idstr (bufid, compl1obj);
              gtk_text_buffer_insert (mom_cmdtextbuf, &itcurs, bufid,
                                      MOM_CSTRIDLEN);
              if ((commv =
                   mo_objref_get_attr (compl1obj,
                                       MOM_PREDEF (comment))) != NULL
                  && mo_dyncast_string (commv))
                {
                  char combuf[72];
                  memset (combuf, 0, sizeof (combuf));
                  const char *pc = NULL;
                  const char *pn = NULL;
                  char *pb = combuf;
                  for (pc = mo_string_cstr (commv);
                       pc && *pc
                       && (((pn = g_utf8_next_char (pc)),
                            pb - combuf < (int) sizeof (combuf) - 10));
                       pc = pn)
                    {
                      if (*pc == '\n')
                        break;
                      if (pb > combuf + 2 * sizeof (combuf) / 3
                          && isspace (*pc))
                        break;
                      if (pn == pc + 1)
                        *(pb++) = *pc;
                      else
                        {
                          memcpy (pb, pc, pn - pc);
                          pb += pn - pc;
                        }
                    };
                  momgui_cmdstatus_printf ("completion %s: %s", bufid,
                                           combuf);
                };
              gtk_text_buffer_place_cursor (mom_cmdtextbuf, &itcurs);
            }
          gtk_text_buffer_end_user_action (mom_cmdtextbuf);
        }
      else if (complsiz == 0)
        {
          momgui_cmdstatus_printf ("no completion for: %s", wordtxt);
        }
      else if (complsiz < MOMGUI_COMPLETION_MENU_MAX)
        {
          char complbufid[MOM_CSTRIDSIZ];
          memset (complbufid, 0, sizeof (complbufid));
          if (MOM_UNLIKELY (mom_cmdcomplmenu))
            {
              gtk_widget_destroy (mom_cmdcomplmenu);
              mom_cmdcomplmenu = NULL;
              mom_cmdcomplstartoff = 0;
              mom_cmdcomplendoff = 0;
            }
          mom_cmdcomplmenu = gtk_menu_new ();
          mom_cmdcomplstartoff = gtk_text_iter_get_offset (&itbword);
          mom_cmdcomplendoff = gtk_text_iter_get_offset (&itcurs);
          for (int ix = 0; ix < (int) complsiz; ix++)
            {
              mo_objref_t curcomplobj = mo_set_nth (mom_cmdcomplset, ix);
              GtkWidget *curcomplitem =
                gtk_menu_item_new_with_label
                (mom_cmdcomplwithname
                 ? mo_objref_pnamestr (curcomplobj)
                 : mo_objref_idstr (complbufid, curcomplobj));
              gtk_menu_shell_append (GTK_MENU_SHELL (mom_cmdcomplmenu),
                                     curcomplitem);
              g_signal_connect (curcomplitem, "activate",
                                G_CALLBACK (momgui_completecmdix),
                                (void *) ((intptr_t) ix));
            }
          g_signal_connect (mom_cmdcomplmenu, "deactivate",
                            G_CALLBACK (gtk_widget_destroyed),
                            &mom_cmdcomplmenu);
          gtk_widget_show_all (mom_cmdcomplmenu);
          gtk_menu_popup_at_pointer (GTK_MENU (mom_cmdcomplmenu), NULL);
        }
      else if (mom_cmdcomplwithname && complsiz < MOMGUI_COMPLETION_MANY_NAMES
               && complsiz >= MOMGUI_COMPLETION_MENU_MAX)
        {
          mo_value_t prevnamv =
            mo_objref_namev (mo_set_nth (mom_cmdcomplset, 0));
          MOM_ASSERTPRINTF (mo_dyncast_string (prevnamv), "bad prevnamv");
          int commonlen = (prevnamv ? strlen (mo_string_cstr (prevnamv)) : 0);
          for (int ix = 1; ix < (int) complsiz && commonlen > 0; ix++)
            {
              mo_objref_t curobjv = mo_set_nth (mom_cmdcomplset, ix);
              mo_value_t curnamv = mo_objref_namev (curobjv);
              MOM_ASSERTPRINTF (mo_dyncast_string (curnamv),
                                "bad curnamv for ix#%d", ix);
              const char *prevpc = mo_string_cstr (prevnamv);
              const char *curpc = mo_string_cstr (curnamv);
              int samelen = 0;
              while ((*prevpc) != (char) 0 && (*curpc) != (char) 0
                     && *prevpc == *curpc)
                samelen++;
              if (commonlen > samelen)
                commonlen = samelen;
              prevnamv = curnamv;
            }
          if (commonlen > 1)
            {
              char *commonprefix = mom_gc_alloc_scalar (commonlen + 2);
              memcpy (commonprefix, mo_string_cstr (prevnamv), commonlen);
              gtk_text_buffer_delete (mom_cmdtextbuf, &itbword, &itcurs);
              gtk_text_buffer_insert (mom_cmdtextbuf, &itcurs,
                                      commonprefix, commonlen);
              gtk_text_buffer_place_cursor (mom_cmdtextbuf, &itcurs);
            }
          momgui_cmdstatus_printf
            ("%d completions for %s :: %s ... %s",
             complsiz, wordtxt,
             mo_string_cstr (mo_objref_namev
                             (mo_set_nth (mom_cmdcomplset, 0))),
             mo_string_cstr (mo_objref_namev
                             (mo_set_nth (mom_cmdcomplset, complsiz - 1))));
        }
      else
        {
          momgui_cmdstatus_printf ("too many %d completions for: %s",
                                   (int) complsiz, wordtxt);
        }
      g_free (wordtxt);
      return TRUE;              // don't propagate
    }
  return FALSE;                 // to propagate the event
}                               /* end momgui_cmdtextview_keyrelease */


static mo_objref_t momgui_cmdparse_object (struct momgui_cmdparse_st *,
                                           const char *);
static mo_value_t momgui_cmdparse_value (struct momgui_cmdparse_st *,
                                         const char *);

// register a pair of matching delimiters, e.g. open & closing parenthesis
static void
momgui_cmdparse_delimoffsetpairs (struct momgui_cmdparse_st *cpars,
                                  unsigned leftoff, unsigned rightoff)
{
  MOM_ASSERTPRINTF (cpars && cpars->mo_gcp_nmagic == MOMGUI_CMDPARSE_MAGIC,
                    "bad cpars@%p", cpars);
  MOM_ASSERTPRINTF (leftoff < rightoff, "bad leftoff=%u rightoff=%u",
                    leftoff, rightoff);
  MOM_WARNPRINTF
    ("cmdparse_delimoffsetpairs unimplemented leftoff=%u rightoff=%u",
     leftoff, rightoff);
#warning unimplemented momgui_cmdparse_delimoffsetpairs
}                               /* end momgui_cmdparse_delimoffsetpairs */

// skip spaces, return true if end-of-buffer reached
static bool
momgui_cmdparse_skipspaces (struct momgui_cmdparse_st *cpars)
{
  MOM_ASSERTPRINTF (cpars && cpars->mo_gcp_nmagic == MOMGUI_CMDPARSE_MAGIC,
                    "bad cpars@%p", cpars);
  gunichar uc = 0;
  int nbspaces = 0;
  GtkTextIter itbeg = cpars->mo_gcp_curiter;
  while ((uc = gtk_text_iter_get_char (&cpars->mo_gcp_curiter)) != 0)
    {
      if (!g_unichar_isspace (uc))
        break;
      if (!gtk_text_iter_forward_char (&cpars->mo_gcp_curiter))
        break;
      nbspaces++;
    }
  if (nbspaces > 0)
    {
      gint curoff = gtk_text_iter_get_offset (&cpars->mo_gcp_curiter);
      gtk_text_buffer_remove_all_tags (mom_cmdtextbuf, &itbeg,
                                       &cpars->mo_gcp_curiter);
      gtk_text_buffer_get_iter_at_offset (mom_cmdtextbuf,
                                          &cpars->mo_gcp_curiter, curoff);
    }
  return gtk_text_iter_is_end (&cpars->mo_gcp_curiter);
}                               /* end momgui_cmdparse_skipspaces */

static inline gunichar
momgui_cmdparse_peekchar (struct momgui_cmdparse_st *cpars, int delta)
{
  MOM_ASSERTPRINTF (cpars && cpars->mo_gcp_nmagic == MOMGUI_CMDPARSE_MAGIC,
                    "bad cpars@%p", cpars);
  if (delta == 0)
    return gtk_text_iter_get_char (&cpars->mo_gcp_curiter);
  GtkTextIter it = cpars->mo_gcp_curiter;
  if (delta > 0)
    {
      if (!gtk_text_iter_forward_chars (&it, delta))
        return 0;
      return gtk_text_iter_get_char (&it);
    }
  else if (delta < 0)
    {
      if (!gtk_text_iter_backward_chars (&it, delta))
        return 0;
      return gtk_text_iter_get_char (&it);
    }
  return 0;
}                               /* end of momgui_cmdparse_peekchar */

static inline int
momgui_cmdparse_curline_bytesize (struct momgui_cmdparse_st *cpars)
{
  MOM_ASSERTPRINTF (cpars && cpars->mo_gcp_nmagic == MOMGUI_CMDPARSE_MAGIC,
                    "bad cpars@%p", cpars);
  GtkTextIter it = cpars->mo_gcp_curiter;
  gtk_text_iter_forward_line (&it);
  return gtk_text_iter_get_line_index (&it) -
    gtk_text_iter_get_line_index (&cpars->mo_gcp_curiter);
}                               /* end of momgui_cmdparse_curline_bytesize */


static mo_value_t
momgui_cmdparse_value (struct momgui_cmdparse_st *cpars, const char *msg)
{
  MOM_ASSERTPRINTF (cpars && cpars->mo_gcp_nmagic == MOMGUI_CMDPARSE_MAGIC,
                    "bad cpars@%p", cpars);
  MOM_ASSERTPRINTF (msg != NULL, "missing msg");
  if (momgui_cmdparse_skipspaces (cpars))
    MOMGUI_CMDPARSEFAIL (cpars, "reached end of buffer, expecting value (%s)",
                         msg);
  gunichar curc = momgui_cmdparse_peekchar (cpars, 0);
  gunichar nextc = momgui_cmdparse_peekchar (cpars, 1);
  gunichar digc = 0;
  char sbuf[72];
  memset (sbuf, 0, sizeof (sbuf));
  if (curc == '0' && (nextc == 'x' || nextc == 'X'))
    {
      // parse an hex number
      int ix;
      char *endp = NULL;
      sbuf[0] = curc;
      sbuf[1] = nextc;
      for (ix = 2; ix < (int) sizeof (sbuf) - 2
           && (digc = momgui_cmdparse_peekchar (cpars, ix)) > 0
           && digc < 127 && isxdigit (digc); ix++)
        sbuf[ix] = (char) digc;
      if (ix >= (int) sizeof (sbuf) - 4)
        MOMGUI_CMDPARSEFAIL (cpars, "too long hex number %s (%s)", sbuf, msg);
      long long ll = strtoll (sbuf, &endp, 16);
      if (!endp || *endp || ll < MO_INTMIN || ll > MO_INTMAX)
        MOMGUI_CMDPARSEFAIL (cpars, "bad hex number %s (%s)", sbuf, msg);
      int numlen = endp - sbuf;
      GtkTextIter startnumit = cpars->mo_gcp_curiter;
      GtkTextIter endnumit = startnumit;
      gtk_text_iter_forward_chars (&endnumit, numlen);
      gtk_text_buffer_apply_tag (mom_cmdtextbuf, mom_cmdtag_number,
                                 &startnumit, &endnumit);
      cpars->mo_gcp_curiter = endnumit;
      return mo_int_to_value (ll);
    }                           // end hex number
  else if (curc == '0' && (nextc == 'o' || nextc == 'O'))
    {
      // parse an octal number
      int ix;
      char *endp = NULL;
      sbuf[0] = curc;
      sbuf[1] = nextc;
      for (ix = 2; ix < (int) sizeof (sbuf) - 2
           && (digc = momgui_cmdparse_peekchar (cpars, ix)) > 0
           && digc < 127 && digc >= '0' && digc <= '7'; ix++)
        sbuf[ix] = (char) digc;
      if (ix >= (int) sizeof (sbuf) - 4)
        MOMGUI_CMDPARSEFAIL (cpars, "too long octal number %s (%s)", sbuf,
                             msg);
      long long ll = strtoll (sbuf + 2, &endp, 8);
      if (!endp || *endp || ll < MO_INTMIN || ll > MO_INTMAX)
        MOMGUI_CMDPARSEFAIL (cpars, "bad octal number %s (%s)", sbuf, msg);
      int numlen = endp - sbuf;
      GtkTextIter startnumit = cpars->mo_gcp_curiter;
      GtkTextIter endnumit = startnumit;
      gtk_text_iter_forward_chars (&endnumit, numlen);
      gtk_text_buffer_apply_tag (mom_cmdtextbuf, mom_cmdtag_number,
                                 &startnumit, &endnumit);
      cpars->mo_gcp_curiter = endnumit;
      return mo_int_to_value (ll);
    }                           // end octal number
  else if (curc == '0' && (nextc == 'b' || nextc == 'B'))
    {
      // parse a binary number
      int ix;
      char *endp = NULL;
      sbuf[0] = curc;
      sbuf[1] = nextc;
      for (ix = 2; ix < (int) sizeof (sbuf) - 2
           && (digc = momgui_cmdparse_peekchar (cpars, ix)) > 0
           && (digc == '0' || digc == '1'); ix++)
        sbuf[ix] = (char) digc;
      if (ix >= (int) sizeof (sbuf) - 4)
        MOMGUI_CMDPARSEFAIL (cpars, "too long binary number %s (%s)", sbuf,
                             msg);
      long long ll = strtoll (sbuf + 2, &endp, 8);
      if (!endp || *endp || ll < MO_INTMIN || ll > MO_INTMAX)
        MOMGUI_CMDPARSEFAIL (cpars, "bad binary number %s (%s)", sbuf, msg);
      int numlen = endp - sbuf;
      GtkTextIter startnumit = cpars->mo_gcp_curiter;
      GtkTextIter endnumit = startnumit;
      gtk_text_iter_forward_chars (&endnumit, numlen);
      gtk_text_buffer_apply_tag (mom_cmdtextbuf, mom_cmdtag_number,
                                 &startnumit, &endnumit);
      cpars->mo_gcp_curiter = endnumit;
      return mo_int_to_value (ll);
    }                           // end binary number
  else if ((curc >= '0' && curc <= '9')
           || ((curc == '+' || curc == '-') && nextc >= '0' && nextc <= '9'))
    {                           /* signed decimal number */
      int ix;
      char *endp = NULL;
      sbuf[0] = curc;
      sbuf[1] = nextc;
      for (ix = 1; ix < (int) sizeof (sbuf) - 2
           && (digc = momgui_cmdparse_peekchar (cpars, ix)) > 0
           && digc >= '0' && digc <= '9'; ix++)
        sbuf[ix] = (char) digc;
      if (ix >= (int) sizeof (sbuf) - 4)
        MOMGUI_CMDPARSEFAIL (cpars, "too long number %s (%s)", sbuf, msg);
      long long ll = strtoll (sbuf, &endp, 0);
      if (!endp || *endp || ll < MO_INTMIN || ll > MO_INTMAX)
        MOMGUI_CMDPARSEFAIL (cpars, "bad number %s (%s)", sbuf, msg);
      int numlen = endp - sbuf;
      GtkTextIter startnumit = cpars->mo_gcp_curiter;
      GtkTextIter endnumit = startnumit;
      gtk_text_iter_forward_chars (&endnumit, numlen);
      gtk_text_buffer_apply_tag (mom_cmdtextbuf, mom_cmdtag_number,
                                 &startnumit, &endnumit);
      cpars->mo_gcp_curiter = endnumit;
      return mo_int_to_value (ll);
    }                           // end decimal or signed number
  else if (curc < 127 && (isalpha (curc) || curc == '_'))       // parse objects
    return momgui_cmdparse_object (cpars, msg);
  else if (curc == '~')
    {                           // ~ is for NIL
      GtkTextIter tildit = cpars->mo_gcp_curiter;
      gtk_text_iter_forward_char (&cpars->mo_gcp_curiter);
      gtk_text_buffer_apply_tag (mom_cmdtextbuf, mom_cmdtag_delim,
                                 &tildit, &cpars->mo_gcp_curiter);
      return NULL;
    }
  else if (curc == '\'')
    {                           // ' is for eol-terminated strings
      GtkTextIter strit = cpars->mo_gcp_curiter;
      gtk_text_iter_forward_char (&cpars->mo_gcp_curiter);
      GtkTextIter begit = cpars->mo_gcp_curiter;
      gtk_text_buffer_apply_tag (mom_cmdtextbuf, mom_cmdtag_delim,
                                 &strit, &cpars->mo_gcp_curiter);
      GtkTextIter endlinit = cpars->mo_gcp_curiter;
      gtk_text_iter_forward_line (&endlinit);
      cpars->mo_gcp_curiter = endlinit;
      gtk_text_buffer_apply_tag (mom_cmdtextbuf, mom_cmdtag_string,
                                 &begit, &cpars->mo_gcp_curiter);
      if (cpars->mo_gcp_onlyparse)
        return NULL;
      else
        {
          char *strtxt =
            gtk_text_buffer_get_text (mom_cmdtextbuf, &begit, &endlinit,
                                      false);
          mo_value_t vstr = mo_make_string_cstr (strtxt);
          g_free (strtxt), strtxt = NULL;
          return vstr;
        }
    }
  else if (curc == '{')
    {                           // parse sets of objects
      mo_vectvaldatapayl_ty *vectobj = NULL;
      unsigned leftbraceoff =
        gtk_text_iter_get_offset (&cpars->mo_gcp_curiter);
      unsigned rightbraceoff = 0;
      GtkTextIter bracit = cpars->mo_gcp_curiter;
      gtk_text_iter_forward_char (&cpars->mo_gcp_curiter);
      gtk_text_buffer_apply_tag (mom_cmdtextbuf, mom_cmdtag_delim,
                                 &bracit, &cpars->mo_gcp_curiter);
      while (!momgui_cmdparse_skipspaces (cpars))
        {
          gunichar curc = momgui_cmdparse_peekchar (cpars, 0);
          if (curc == '}')
            {
              rightbraceoff =
                gtk_text_iter_get_offset (&cpars->mo_gcp_curiter);
              GtkTextIter bracit = cpars->mo_gcp_curiter;
              gtk_text_iter_forward_char (&cpars->mo_gcp_curiter);
              gtk_text_buffer_apply_tag (mom_cmdtextbuf, mom_cmdtag_delim,
                                         &bracit, &cpars->mo_gcp_curiter);
              momgui_cmdparse_delimoffsetpairs (cpars, leftbraceoff,
                                                rightbraceoff);
              break;
            }
          mo_objref_t elemob = momgui_cmdparse_object (cpars, "set-elem");
          if (!cpars->mo_gcp_onlyparse)
            {
              if (!elemob)
                MOMGUI_CMDPARSEFAIL (cpars, "nil object as set element (%s)",
                                     msg);
              vectobj = mo_vectval_append (vectobj, elemob);
            }
        };
      if (!cpars->mo_gcp_onlyparse)
        {
          unsigned cnt = mo_vectval_count (vectobj);
          if (!cnt)
            return mo_make_empty_set ();
          return mo_make_set_closeq
            (mo_sequence_filled_allocate (cnt,
                                          (mo_objref_t
                                           *) (vectobj->mo_vect_arr)));
        }
      return NULL;
    }
  else if (curc == 0x2205 /* U+2205 EMPTY SET ∅ */ )
    {
      GtkTextIter delit = cpars->mo_gcp_curiter;
      gtk_text_iter_forward_char (&cpars->mo_gcp_curiter);
      gtk_text_buffer_apply_tag (mom_cmdtextbuf, mom_cmdtag_delim,
                                 &delit, &cpars->mo_gcp_curiter);
      return mo_make_empty_set ();
    }
  else if (curc == '[')
    {                           // parse tuples of objects
      mo_vectvaldatapayl_ty *vectobj = NULL;
      unsigned leftbrackoff =
        gtk_text_iter_get_offset (&cpars->mo_gcp_curiter);
      unsigned rightbrackoff = 0;
      GtkTextIter brackit = cpars->mo_gcp_curiter;
      gtk_text_iter_forward_char (&cpars->mo_gcp_curiter);
      gtk_text_buffer_apply_tag (mom_cmdtextbuf, mom_cmdtag_delim,
                                 &brackit, &cpars->mo_gcp_curiter);
      while (!momgui_cmdparse_skipspaces (cpars))
        {
          gunichar curc = momgui_cmdparse_peekchar (cpars, 0);
          if (curc == ']')
            {
              rightbrackoff =
                gtk_text_iter_get_offset (&cpars->mo_gcp_curiter);
              GtkTextIter bracit = cpars->mo_gcp_curiter;
              gtk_text_iter_forward_char (&cpars->mo_gcp_curiter);
              gtk_text_buffer_apply_tag (mom_cmdtextbuf, mom_cmdtag_delim,
                                         &bracit, &cpars->mo_gcp_curiter);
              momgui_cmdparse_delimoffsetpairs (cpars, leftbrackoff,
                                                rightbrackoff);
              break;
            }
          mo_objref_t elemob = momgui_cmdparse_object (cpars, "tuple-comp");
          if (!cpars->mo_gcp_onlyparse)
            {
              vectobj = mo_vectval_append (vectobj, elemob);
            }
        };
      if (!cpars->mo_gcp_onlyparse)
        {
          unsigned cnt = mo_vectval_count (vectobj);
          if (!cnt)
            return mo_make_empty_tuple ();
          return mo_make_tuple_closeq
            (mo_sequence_filled_allocate (cnt,
                                          (mo_objref_t
                                           *) (vectobj->mo_vect_arr)));
        }
      return NULL;
    }
  MOMGUI_CMDPARSEFAIL (cpars, "bad value (%s)", msg);
}                               /* end momgui_cmdparse_value */

static void
momgui_cmdparse_complement (struct momgui_cmdparse_st *cpars, mo_objref_t obr,
                            const char *msg);
static mo_objref_t
momgui_cmdparse_object (struct momgui_cmdparse_st *cpars, const char *msg)
{
  mo_objref_t objp = NULL;
  bool gotnil = false;
  MOM_ASSERTPRINTF (cpars && cpars->mo_gcp_nmagic == MOMGUI_CMDPARSE_MAGIC,
                    "bad cpars@%p", cpars);
  MOM_ASSERTPRINTF (msg != NULL, "missing msg");
  if (momgui_cmdparse_skipspaces (cpars))
    MOMGUI_CMDPARSEFAIL (cpars,
                         "reached end of buffer, expecting object (%s)", msg);
  gunichar curc = momgui_cmdparse_peekchar (cpars, 0);
  gunichar nextc = momgui_cmdparse_peekchar (cpars, 1);
  // handle objects like "the_GUI" or "_1HW4pIotlYRImRGnL"
  if (curc > 0 && curc < 127
      && (isalpha ((char) curc)
          || (curc == '_' && nextc >= '0' && nextc <= '9')))
    {
      gunichar namc = 0;
      int nlen = 0;
      char smallnambuf[40];
      memset (smallnambuf, 0, sizeof (smallnambuf));
      for (nlen = 0; nlen <= MOM_NAME_MAXLEN + 2
           && (namc = momgui_cmdparse_peekchar (cpars, nlen)) > 0
           && (namc < 127 && (isalnum ((char) namc) || namc == '_')); nlen++);
      if (nlen >= MOM_NAME_MAXLEN)
        MOMGUI_CMDPARSEFAIL (cpars, "too wide name of %d chars (%s)",
                             nlen, msg);
      char *nambuf =
        (nlen < (int) sizeof (smallnambuf) - 2)
        ? smallnambuf
        : mom_gc_alloc_scalar (4 * mom_prime_above (nlen / 4 + 3));
      for (int ix = 0; ix < nlen; ix++)
        nambuf[ix] = (char) momgui_cmdparse_peekchar (cpars, ix);
      if (isalpha (nambuf[0]))
        {
          objp = mo_find_named_cstr (nambuf);
          if (objp)
            {
              GtkTextIter startnamit = cpars->mo_gcp_curiter;
              GtkTextIter endnamit = startnamit;
              gtk_text_iter_forward_chars (&endnamit, nlen);
              gtk_text_buffer_apply_tag (mom_cmdtextbuf, mom_cmdtag_name,
                                         &startnamit, &endnamit);
              cpars->mo_gcp_curiter = endnamit;
            }
        }
      else if (nambuf[0] == '_' && isdigit (nambuf[1]))
        {
          mo_hid_t hid = 0;
          mo_loid_t loid = 0;
          if (nlen == MOM_CSTRIDLEN
              && mo_get_hi_lo_ids_from_cstring (&hid, &loid, nambuf))
            {
              objp = mo_objref_find_hid_loid (hid, loid);
              if (objp)
                {
                  GtkTextIter startidit = cpars->mo_gcp_curiter;
                  GtkTextIter endidit = startidit;
                  gtk_text_iter_forward_chars (&endidit, MOM_CSTRIDLEN);
                  gtk_text_buffer_apply_tag (mom_cmdtextbuf, mom_cmdtag_anon,
                                             &startidit, &endidit);
                  cpars->mo_gcp_curiter = endidit;
                }
            }
        };
      if (!objp)
        {
          GtkTextIter startunit = cpars->mo_gcp_curiter;
          GtkTextIter endunit = startunit;
          gtk_text_iter_forward_chars (&endunit, nlen);
          gtk_text_buffer_apply_tag (mom_cmdtextbuf, mom_cmdtag_unknown,
                                     &startunit, &endunit);
          MOM_WARNPRINTF ("unknown object '%s' (%s) nlen=%d pos#%d",
                          nambuf, msg, nlen,
                          gtk_text_iter_get_offset (&startunit));
          cpars->mo_gcp_curiter = endunit;
          if (!cpars->mo_gcp_onlyparse)
            MOMGUI_CMDPARSEFAIL (cpars, "unknown object %s (%s)",
                                 nambuf, msg);
          cpars->mo_gcp_curiter = endunit;
        }
      else
        cpars->mo_gcp_setparsed =
          mo_hashset_put (cpars->mo_gcp_setparsed, objp);
    }
  /** something like
    _'some comment till eol
    will create an anonymous global object of comment "some comment till eol"
  **/
  else if (curc == '_' && nextc == '\'')
    {
      GtkTextIter startglit = cpars->mo_gcp_curiter;
      GtkTextIter startcomit = startglit;
      gtk_text_iter_forward_chars (&startcomit, 2);
      GtkTextIter endlinit = startcomit;
      gtk_text_iter_forward_line (&endlinit);
      gtk_text_buffer_apply_tag (mom_cmdtextbuf, mom_cmdtag_newglob,
                                 &startglit, &endlinit);
      gtk_text_buffer_apply_tag (mom_cmdtextbuf, mom_cmdtag_newcomm,
                                 &startcomit, &endlinit);
      if (!cpars->mo_gcp_onlyparse)
        {
          mo_value_t commv = NULL;
          char *commbuf =
            gtk_text_buffer_get_text (mom_cmdtextbuf, &startcomit, &endlinit,
                                      false);
          if (commbuf && commbuf[0])
            commv = mo_make_string_cstr (commbuf);
          g_free (commbuf), commbuf = NULL;
          objp = mo_make_global_object ();
          if (commv)
            mo_objref_put_attr (objp, MOM_PREDEF (comment), commv);
          cpars->mo_gcp_setcreated =
            mo_hashset_put (cpars->mo_gcp_setcreated, objp);
        }
      cpars->mo_gcp_curiter = endlinit;
    }
  /** something like
    __'some other comment till eol
    will create an anonymous transient object of comment 
    "some other comment till eol"
  **/
  else if (curc == '_' && nextc == '_'
           && momgui_cmdparse_peekchar (cpars, 2) == '\'')
    {
      GtkTextIter startglit = cpars->mo_gcp_curiter;
      GtkTextIter startcomit = startglit;
      gtk_text_iter_forward_chars (&startcomit, 2);
      GtkTextIter endlinit = startcomit;
      gtk_text_iter_forward_line (&endlinit);
      gtk_text_buffer_apply_tag (mom_cmdtextbuf, mom_cmdtag_newglob,
                                 &startglit, &endlinit);
      gtk_text_buffer_apply_tag (mom_cmdtextbuf, mom_cmdtag_newcomm,
                                 &startcomit, &endlinit);
      if (!cpars->mo_gcp_onlyparse)
        {
          mo_value_t commv = NULL;
          char *commbuf =
            gtk_text_buffer_get_text (mom_cmdtextbuf, &startcomit, &endlinit,
                                      false);
          if (commbuf && commbuf[0])
            commv = mo_make_string_cstr (commbuf);
          g_free (commbuf), commbuf = NULL;
          objp = mo_make_object ();
          if (commv)
            mo_objref_put_attr (objp, MOM_PREDEF (comment), commv);
          cpars->mo_gcp_setcreated =
            mo_hashset_put (cpars->mo_gcp_setcreated, objp);
        }
      cpars->mo_gcp_curiter = endlinit;
    }
  /*** ~ represents the nil object ****/
  else if (curc == '~')
    {
      objp = NULL;
      GtkTextIter startnilit = cpars->mo_gcp_curiter;
      GtkTextIter endnilit = startnilit;
      gtk_text_iter_forward_char (&endnilit);
      gtk_text_buffer_apply_tag (mom_cmdtextbuf, mom_cmdtag_delim,
                                 &startnilit, &endnilit);
      cpars->mo_gcp_curiter = endnilit;
      gotnil = true;
    }
  /** otherwise, parsing error */
  else
    {
      MOMGUI_CMDPARSEFAIL (cpars, "expecting object (%s)", msg);
    }
  if (momgui_cmdparse_skipspaces (cpars))
    return objp;
  if (!gotnil && momgui_cmdparse_peekchar (cpars, 0) == '(')
    momgui_cmdparse_complement (cpars, objp, msg);
  return objp;
}                               /* end momgui_cmdparse_object */

static void
momgui_cmdparse_complement (struct momgui_cmdparse_st *cpars, mo_objref_t obr,
                            const char *msg)
{
  MOM_ASSERTPRINTF (cpars && cpars->mo_gcp_nmagic == MOMGUI_CMDPARSE_MAGIC,
                    "bad cpars@%p", cpars);
  MOM_ASSERTPRINTF (msg != NULL, "missing msg");
  if (momgui_cmdparse_skipspaces (cpars))
    MOMGUI_CMDPARSEFAIL (cpars,
                         "reached end of buffer, expecting complement (%s)",
                         msg);
  gunichar curc = momgui_cmdparse_peekchar (cpars, 0);
  if (curc != '(')
    return;
  GtkTextIter leftpit = cpars->mo_gcp_curiter;
  unsigned leftparenoff = gtk_text_iter_get_offset (&cpars->mo_gcp_curiter);
  gtk_text_iter_forward_char (&cpars->mo_gcp_curiter);
  gtk_text_buffer_apply_tag (mom_cmdtextbuf, mom_cmdtag_delim,
                             &leftpit, &cpars->mo_gcp_curiter);
  while (!momgui_cmdparse_skipspaces (cpars))
    {
      gunichar curc = momgui_cmdparse_peekchar (cpars, 0);
      /// star to add an attribute
      if (curc == '*' || curc == 0x22C6 /*U+22C6 STAR OPERATOR ⋆ */ )
        {
          GtkTextIter starit = cpars->mo_gcp_curiter;
          gtk_text_iter_forward_char (&cpars->mo_gcp_curiter);
          gtk_text_buffer_apply_tag (mom_cmdtextbuf, mom_cmdtag_delim,
                                     &starit, &cpars->mo_gcp_curiter);
          mo_objref_t obattr = momgui_cmdparse_object (cpars, "attr");
          momgui_cmdparse_skipspaces (cpars);
          curc = momgui_cmdparse_peekchar (cpars, 0);
          if (curc == ':' || curc == '>'
              || curc == 0x21FE /*U+21FE RIGHTWARDS OPEN-HEADED ARROW ⇾ */ )
            {
              GtkTextIter colit = cpars->mo_gcp_curiter;
              gtk_text_iter_forward_char (&cpars->mo_gcp_curiter);
              gtk_text_buffer_apply_tag (mom_cmdtextbuf, mom_cmdtag_delim,
                                         &colit, &cpars->mo_gcp_curiter);
            }
          else
            {
              if (obattr && obr)
                MOMGUI_CMDPARSEFAIL (cpars,
                                     "expecting colon after attribute %s of %s (%s)",
                                     mo_objref_pnamestr (obattr),
                                     mo_objref_pnamestr (obr), msg);
              else if (obattr)
                MOMGUI_CMDPARSEFAIL (cpars,
                                     "expecting colon after attribute %s (%s)",
                                     mo_objref_pnamestr (obattr), msg);
              else
                MOMGUI_CMDPARSEFAIL (cpars,
                                     "expecting colon after attribute (%s)",
                                     msg);
            }
          momgui_cmdparse_skipspaces (cpars);
          mo_value_t valat = momgui_cmdparse_value (cpars, "atval");
          if (!cpars->mo_gcp_onlyparse)
            {
              if (!obr && !obattr)
                MOMGUI_CMDPARSEFAIL (cpars,
                                     "missing object to add attribute (%s)",
                                     msg);
              else if (!obr)
                MOMGUI_CMDPARSEFAIL (cpars,
                                     "missing object to add attribute %s (%s)",
                                     mo_objref_pnamestr (obattr), msg);
              else if (!obattr)
                MOMGUI_CMDPARSEFAIL (cpars,
                                     "missing attribute to add into %s (%s)",
                                     mo_objref_pnamestr (obr), msg);
              if (!valat)
                MOMGUI_CMDPARSEFAIL (cpars,
                                     "missing value for adding attribute %s in %s (%s)",
                                     mo_objref_pnamestr (obattr),
                                     mo_objref_pnamestr (obr), msg);
              mo_objref_put_attr (obr, obattr, valat);
            };
        }
      else if (curc == '&')
        {
          // perhaps we might ignore any index info, e.g. &#10: could be parsed as & 
          GtkTextIter ampit = cpars->mo_gcp_curiter;
          gtk_text_iter_forward_char (&cpars->mo_gcp_curiter);
          gtk_text_buffer_apply_tag (mom_cmdtextbuf, mom_cmdtag_delim,
                                     &ampit, &cpars->mo_gcp_curiter);
          momgui_cmdparse_skipspaces (cpars);
          mo_value_t valcomp = momgui_cmdparse_value (cpars, "comp");
          if (!cpars->mo_gcp_onlyparse)
            {
              mo_objref_comp_append (obr, valcomp);
            }
        }
      else if (curc == ')')
        {
          GtkTextIter parit = cpars->mo_gcp_curiter;
          unsigned rightparenoff =
            gtk_text_iter_get_offset (&cpars->mo_gcp_curiter);
          gtk_text_iter_forward_char (&cpars->mo_gcp_curiter);
          gtk_text_buffer_apply_tag (mom_cmdtextbuf, mom_cmdtag_delim,
                                     &parit, &cpars->mo_gcp_curiter);
          momgui_cmdparse_delimoffsetpairs (cpars, leftparenoff,
                                            rightparenoff);
          break;
        }
      else
        {
          char ubuf[8];
          memset (ubuf, 0, sizeof (ubuf));
          if (curc)
            g_unichar_to_utf8 (curc, ubuf);
          else
            strcpy (ubuf, "END");
          if (obr)
            MOMGUI_CMDPARSEFAIL (cpars,
                                 "unexpected character %s in complement of %s (%s)",
                                 ubuf, mo_objref_pnamestr (obr), msg);
          else
            MOMGUI_CMDPARSEFAIL (cpars,
                                 "unexpected character %s in complement (%s)",
                                 ubuf, msg);
        }
    };
}                               /* end of momgui_cmdparse_complement */

static void
momgui_cmdparsefailure (struct momgui_cmdparse_st *cpars, int lineno)
{
  MOM_ASSERTPRINTF (cpars && cpars->mo_gcp_nmagic == MOMGUI_CMDPARSE_MAGIC,
                    "bad cpars@%p", cpars);
  MOM_ASSERTPRINTF (lineno > 0, "bad lineno=%d", lineno);
  MOM_ASSERTPRINTF (mo_dyncast_string (cpars->mo_gcp_errstrv),
                    "bad errstrv in cpars@%p", cpars);
  gint curoff = gtk_text_iter_get_offset (&cpars->mo_gcp_curiter);
  MOM_WARNPRINTF_AT (__FILE__, lineno, "command parse failure (pos#%d): %s",
                     curoff, mo_string_cstr (cpars->mo_gcp_errstrv));
  GtkTextIter itstart = { };
  GtkTextIter itend = { };
  gtk_text_buffer_get_bounds (GTK_TEXT_BUFFER (mom_cmdtextbuf), &itstart,
                              &itend);
  gtk_text_buffer_remove_all_tags (mom_cmdtextbuf,
                                   &cpars->mo_gcp_curiter, &itend);
  gtk_text_buffer_get_iter_at_offset (mom_cmdtextbuf,
                                      &cpars->mo_gcp_curiter, curoff);
  gtk_text_buffer_get_bounds (GTK_TEXT_BUFFER (mom_cmdtextbuf), &itstart,
                              &itend);
  gtk_text_buffer_apply_tag (mom_cmdtextbuf, mom_cmdtag_fail,
                             &cpars->mo_gcp_curiter, &itend);
  gtk_text_buffer_get_iter_at_offset (mom_cmdtextbuf,
                                      &cpars->mo_gcp_curiter, curoff);
  if (cpars->mo_gcp_statusupdate)
    momgui_cmdstatus_printf ("parse failure#%d@%u: %s", lineno,
                             (unsigned) curoff,
                             mo_string_cstr (cpars->mo_gcp_errstrv));
}                               /* end momgui_cmdparsefailure */

static void
momgui_cmdparse_full_buffer (struct momgui_cmdparse_st *cpars)
{
  GtkTextIter itstart = { };
  GtkTextIter itend = { };
  MOM_ASSERTPRINTF (cpars && cpars->mo_gcp_nmagic == MOMGUI_CMDPARSE_MAGIC,
                    "bad cpars@%p", cpars);
  gtk_text_buffer_get_bounds (GTK_TEXT_BUFFER (mom_cmdtextbuf), &itstart,
                              &itend);
  gtk_text_buffer_remove_all_tags (mom_cmdtextbuf, &itstart, &itend);
  gtk_text_buffer_get_bounds (GTK_TEXT_BUFFER (mom_cmdtextbuf), &itstart,
                              &itend);
  cpars->mo_gcp_curiter = itstart;
  int cnt = 0;
#warning this is temporary, we really should do something more fancy
  while (!momgui_cmdparse_skipspaces (cpars))
    {
      char cntbuf[32];
      memset (cntbuf, 0, sizeof (cntbuf));
      cnt++;
      snprintf (cntbuf, sizeof (cntbuf), "count#%d", cnt);
      momgui_cmdparse_value (cpars, cntbuf);
    }
  MOM_BACKTRACEPRINTF ("cmdparse_full_bffer cnt=%d", cnt);
  if (cpars->mo_gcp_statusupdate)
    {
      momgui_cmdstatus_printf ("parsed %d values and %u chars",
                               cnt,
                               gtk_text_buffer_get_char_count
                               (mom_cmdtextbuf));

    }
}                               /* end of momgui_cmdparse_full_buffer */

// for "end-user-action" signal to mom_cmdtextbuf
static void
momgui_cmdtextbuf_enduseraction (GtkTextBuffer * tbuf MOM_UNUSED,
                                 void *data MOM_UNUSED)
{
  MOM_ASSERTPRINTF (tbuf == GTK_TEXT_BUFFER (mom_cmdtextbuf),
                    "cmdtextbuf_enduseraction bad tbuf");
  int curspos = 0;
  g_object_get (G_OBJECT (mom_cmdtextbuf), "cursor-position", &curspos, NULL);
  GtkTextIter itstart = { };
  GtkTextIter itend = { };
  gtk_text_buffer_get_bounds (GTK_TEXT_BUFFER (mom_cmdtextbuf), &itstart,
                              &itend);
  struct momgui_cmdparse_st cmdparse = { };
  memset (&cmdparse, 0, sizeof (cmdparse));
  cmdparse.mo_gcp_nmagic = MOMGUI_CMDPARSE_MAGIC;
  cmdparse.mo_gcp_curiter = itstart;
  cmdparse.mo_gcp_onlyparse = true;
  cmdparse.mo_gcp_statusupdate = true;  /* probably should not be always true */
  int failerr = setjmp (cmdparse.mo_gcp_failjb);
  if (failerr == 0)
    {
      // temporary, always parse... I'm not sure it is a good idea,
      // since we'll get parsing failures on most keystrokes.
      momgui_cmdparse_full_buffer (&cmdparse);
#warning should parse something in cmdtextbuf_enduseraction
    }
  else                          /* failerr != 0 */
    {
      MOM_WARNPRINTF ("parsing of command failed, failerr=%d", failerr);
    };
}                               /* end momgui_cmdtextbuf_enduseraction */


static void
mom_gtkapp_activate (GApplication * app, gpointer user_data MOM_UNUSED)
{
  const int defwinheight = 400;
  const int defwinwidth = 650;
  GdkScreen *screen = gdk_screen_get_default ();
  MOM_ASSERTPRINTF (GDK_IS_SCREEN (screen), "bad screen @%p", screen);
  gtk_style_context_add_provider_for_screen (GDK_SCREEN (screen),
                                             GTK_STYLE_PROVIDER
                                             (mom_gtkcssprov),
                                             GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  mom_appwin = gtk_application_window_new (GTK_APPLICATION (app));
  gtk_window_set_default_size (GTK_WINDOW (mom_appwin), defwinwidth,
                               defwinheight);
  GtkWidget *topvbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
  gtk_container_add (GTK_CONTAINER (mom_appwin), topvbox);
  /////////// create & fill the menubar
  GtkWidget *menubar = gtk_menu_bar_new ();
  /// app menu
  GtkWidget *appmenu = gtk_menu_new ();
  GtkWidget *appitem = gtk_menu_item_new_with_label ("App");
  gtk_menu_shell_append (GTK_MENU_SHELL (menubar), appitem);
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (appitem), appmenu);
  g_signal_connect (appitem, "activate", G_CALLBACK (mom_activate_app_menu),
                    NULL);
  GtkWidget *dumpitem = gtk_menu_item_new_with_label ("Dump");
  GtkWidget *dumpexititem = gtk_menu_item_new_with_label ("dump & eXit");
  GtkWidget *quititem = gtk_menu_item_new_with_label ("Quit");
  gtk_menu_shell_append (GTK_MENU_SHELL (appmenu), dumpitem);
  gtk_menu_shell_append (GTK_MENU_SHELL (appmenu), dumpexititem);
  gtk_menu_shell_append (GTK_MENU_SHELL (appmenu), quititem);
  gtk_menu_shell_append (GTK_MENU_SHELL (appmenu),
                         gtk_separator_menu_item_new ());
  mom_checkitemcmd = gtk_check_menu_item_new_with_label ("show/hide Cmd");
  gtk_menu_shell_append (GTK_MENU_SHELL (appmenu), mom_checkitemcmd);
  g_signal_connect (mom_checkitemcmd, "toggled",
                    G_CALLBACK (mom_toggled_command_shown), NULL);
  g_signal_connect (dumpitem, "activate", G_CALLBACK (mom_dump_app), NULL);
  g_signal_connect (dumpexititem, "activate", G_CALLBACK (mom_dumpexit_app),
                    NULL);
  g_signal_connect (quititem, "activate", G_CALLBACK (mom_quit_app), NULL);
  //// editmenu
  GtkWidget *editmenu = gtk_menu_new ();
  GtkWidget *edititem = gtk_menu_item_new_with_label ("Edit");
  gtk_menu_shell_append (GTK_MENU_SHELL (menubar), edititem);
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (edititem), editmenu);
  GtkWidget *displayitem = gtk_menu_item_new_with_label ("Display...");
  GtkWidget *newobitem = gtk_menu_item_new_with_label ("make New object");
  gtk_menu_shell_append (GTK_MENU_SHELL (editmenu), displayitem);
  gtk_menu_shell_append (GTK_MENU_SHELL (editmenu), newobitem);
  gtk_menu_shell_append (GTK_MENU_SHELL (editmenu),
                         gtk_separator_menu_item_new ());
  g_signal_connect (displayitem, "activate", G_CALLBACK (mom_display_edit),
                    NULL);
  g_signal_connect (newobitem, "activate", G_CALLBACK (mom_newob_edit), NULL);
  gtk_box_pack_start (GTK_BOX (topvbox), menubar, FALSE, FALSE, 2);
  ////
  mom_tagtable = gtk_text_tag_table_new ();
  mom_obtextbuf = gtk_text_buffer_new (mom_tagtable);
  mom_initialize_gtk_tags_for_objects ();
  GtkWidget *paned = gtk_paned_new (GTK_ORIENTATION_VERTICAL);
  mom_tview1 = gtk_text_view_new_with_buffer (mom_obtextbuf);
  mom_tview2 = gtk_text_view_new_with_buffer (mom_obtextbuf);
  gtk_text_view_set_editable (GTK_TEXT_VIEW (mom_tview1), false);
  gtk_text_view_set_editable (GTK_TEXT_VIEW (mom_tview2), false);
  GtkWidget *scrotv1 = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (scrotv1), mom_tview1);
  GtkWidget *scrotv2 = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (scrotv2), mom_tview2);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrotv1),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrotv2),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
  gtk_paned_add1 (GTK_PANED (paned), scrotv1);
  gtk_paned_add2 (GTK_PANED (paned), scrotv2);
  gtk_paned_set_position (GTK_PANED (paned), defwinheight / 3);
  gtk_box_pack_end (GTK_BOX (topvbox), paned, TRUE, TRUE, 2);
  GtkWidget *scrocmd = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrocmd),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  mom_cmdtextbuf = gtk_text_buffer_new (NULL);
  mom_cmdtview = gtk_text_view_new_with_buffer (mom_cmdtextbuf);
  mom_cmdtag_fail =
    gtk_text_buffer_create_tag (mom_cmdtextbuf,
                                "fail",
                                "foreground", "darkred",
                                "background", "lightyellow",
                                "weight", PANGO_WEIGHT_BOLD, NULL);
  mom_cmdtag_number =
    gtk_text_buffer_create_tag (mom_cmdtextbuf,
                                "number",
                                "family", "Courier New",
                                "foreground", "darkgoldenrod", NULL);
  mom_cmdtag_string =
    gtk_text_buffer_create_tag (mom_cmdtextbuf,
                                "string",
                                "family", "Courier New, Italics",
                                "background", "cornsilk2",
                                "foreground", "darkviolet", NULL);
  mom_cmdtag_delim =
    gtk_text_buffer_create_tag (mom_cmdtextbuf,
                                "delim",
                                "scale", 1.07,
                                "family", "FreeMono, Bold",
                                "foreground", "steelblue", NULL);
  mom_cmdtag_name =
    gtk_text_buffer_create_tag (mom_cmdtextbuf,
                                "name",
                                "family", "DejaVu Sans Mono, Bold",
                                "foreground", "mediumpurple", NULL);
  mom_cmdtag_anon =
    gtk_text_buffer_create_tag (mom_cmdtextbuf,
                                "anon",
                                "family", "Courier New",
                                "foreground", "darkgreen", NULL);
  mom_cmdtag_unknown =
    gtk_text_buffer_create_tag (mom_cmdtextbuf,
                                "unknown",
                                "family", "Courier New, Bold Italic",
                                "foreground", "darkred", NULL);
  mom_cmdtag_newglob =
    gtk_text_buffer_create_tag (mom_cmdtextbuf,
                                "newglob",
                                "family", "Liberation Mono, Bold",
                                "foreground", "darkolivegreen",
                                "background", "lavender", NULL);
  mom_cmdtag_newtrans =
    gtk_text_buffer_create_tag (mom_cmdtextbuf,
                                "newtrans",
                                "family", "Liberation Mono, Bold",
                                "foreground", "seagreen",
                                "background", "lightcyan", NULL);
  mom_cmdtag_newcomm =
    gtk_text_buffer_create_tag (mom_cmdtextbuf,
                                "newcomm",
                                "scale", 0.75,
                                "family", "Courier New, Italic", NULL);
  gtk_widget_set_name (mom_cmdtview, "cmdtview");
  gtk_text_view_set_editable (GTK_TEXT_VIEW (mom_cmdtview), true);
  gtk_text_view_set_accepts_tab (GTK_TEXT_VIEW (mom_cmdtview), false);
  g_signal_connect (mom_cmdtview, "key-release-event",
                    G_CALLBACK (momgui_cmdtextview_keyrelease), NULL);
  g_signal_connect (mom_cmdtextbuf, "end-user-action",
                    G_CALLBACK (momgui_cmdtextbuf_enduseraction), NULL);
  gtk_container_add (GTK_CONTAINER (scrocmd), mom_cmdtview);
  mom_cmdwin = gtk_application_window_new (GTK_APPLICATION (app));
  gtk_window_set_title (GTK_WINDOW (mom_cmdwin), "monimelt command");
  gtk_window_set_default_size (GTK_WINDOW (mom_cmdwin), 560, 260);
  GtkWidget *cmdtopvbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
  gtk_container_add (GTK_CONTAINER (mom_cmdwin), cmdtopvbox);
  mom_cmdstatusbar = gtk_statusbar_new ();
  gtk_box_pack_start (GTK_BOX (cmdtopvbox), scrocmd, TRUE, TRUE, 2);
  gtk_box_pack_end (GTK_BOX (cmdtopvbox), mom_cmdstatusbar, FALSE, FALSE, 2);
  gtk_widget_set_name (mom_cmdstatusbar, "cmdstatusbar");
  gtk_style_context_add_provider (gtk_widget_get_style_context (mom_cmdwin),
                                  GTK_STYLE_PROVIDER (mom_gtkcssprov),
                                  GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  gtk_style_context_add_provider (gtk_widget_get_style_context (mom_appwin),
                                  GTK_STYLE_PROVIDER (mom_gtkcssprov),
                                  GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  g_signal_connect (mom_cmdwin, "delete-event",
                    G_CALLBACK (gtk_widget_hide_on_delete), NULL);
  g_signal_connect (mom_appwin, "delete-event", G_CALLBACK (mom_stopgui),
                    NULL);
  mo_gui_generate_object_text_buffer ();
  MOM_INFORMPRINTF ("cmdwin@%p/%s/%s",
                    mom_cmdwin,
                    G_OBJECT_CLASS_NAME (mom_cmdwin),
                    G_OBJECT_TYPE_NAME (mom_cmdwin));
  MOM_INFORMPRINTF ("appwin@%p/%s/%s", mom_appwin,
                    G_OBJECT_CLASS_NAME (mom_appwin),
                    G_OBJECT_TYPE_NAME (mom_appwin));
  momgui_cmdstatus_printf ("loaded %u objects & %u modules",
                           mom_load_nb_objects (), mom_load_nb_modules ());
  gtk_widget_show_all (mom_appwin);
  gtk_widget_show_all (mom_cmdwin);
}                               /* end mom_gtkapp_activate */

static guint
momgui_objhash (const void *ob)
{
  if (ob != NULL)
    return mo_objref_hash ((mo_objref_t) ob);
  return 0;
}                               /* end momgui_objhash */

extern void momgui_begin_running (void);
void
momgui_begin_running (void)
{
  MOM_INFORMPRINTF ("momgui_begin_running");
}                               /* end of momgui_begin_running */

static void
momgui_cssparsingerror (GtkCssProvider * gtkprov MOM_UNUSED,
                        GtkCssSection * sect,
                        GError * err, void *data MOM_UNUSED)
{
  GFile *gfil = sect ? gtk_css_section_get_file (sect) : NULL;
  char *path = gfil ? g_file_get_path (gfil) : "*";
  MOM_WARNPRINTF ("GTK CSS parsing error (file %s, lines %d-%d): (%s#%d)\n"
                  "@!?!@ %s",
                  path,
                  gfil ? gtk_css_section_get_start_line (sect) : 0,
                  gfil ? gtk_css_section_get_end_line (sect) : 0,
                  err ? g_quark_to_string (err->domain) : "-",
                  err ? err->code : 0, err ? err->message : "?");
  MOM_BACKTRACEPRINTF ("GTK CSS parsing error - %s", err ? err->message : 0);
  if (path)
    g_free (path), path = NULL;
}                               /* end momgui_cssparsingerror */


void
mom_run_gtk (int *pargc, char ***pargv, char **dispobjects)
{
  int sta = 0;
  mom_gquark = g_quark_from_static_string ("monimelt");
  momgui_displayed_objasso = mo_assoval_reserve (NULL, 100);
  momgui_shown_obocchset = mo_hashset_reserve (NULL, 1500);
  mom_dispobjinfo_hashtable =   //
    g_hash_table_new_full ((GHashFunc) momgui_objhash, NULL,
                           NULL, (GDestroyNotify) mom_destroy_dispobjinfo);
  mom_shownobjocc_hashtable =   //
    g_hash_table_new_full ((GHashFunc) momgui_objhash, NULL,
                           NULL, (GDestroyNotify) mom_destroy_shownobocc);
  mom_gtkapp =
    gtk_application_new ("org.gcc-melt.monitor", G_APPLICATION_FLAGS_NONE);
  mom_gtkcssprov = gtk_css_provider_get_default ();
  if (!mom_gtk_style_path || !isalpha (mom_gtk_style_path[0])
      || strchr (mom_gtk_style_path, '/'))
    MOM_FATAPRINTF
      ("GTK style path (given by --gtk-style %s ...) is invalid",
       mom_gtk_style_path);
  if (access (mom_gtk_style_path, R_OK))
    MOM_FATAPRINTF ("GTK style %s is not readable", mom_gtk_style_path);
  g_signal_connect (mom_gtkcssprov, "parsing-error",
                    G_CALLBACK (momgui_cssparsingerror), NULL);
  gtk_css_provider_load_from_path (mom_gtkcssprov, mom_gtk_style_path, NULL);
  MOM_INFORMPRINTF ("after loading GTK style %s", mom_gtk_style_path);
  g_signal_connect (mom_gtkapp, "activate",
                    G_CALLBACK (mom_gtkapp_activate), NULL);
  if (dispobjects)
    {
      int nbdisp = 0;
      for (const char **pobn = (const char **)dispobjects; *pobn; pobn++)
        nbdisp++;
      momgui_displayed_objasso =
        mo_assoval_reserve (momgui_displayed_objasso, 2 * nbdisp + 3);
      for (const char **pobn = (const char **)dispobjects; *pobn; pobn++)
        {
          const char *curdispname = *pobn;
          mo_hid_t hid = 0;
          mo_loid_t loid = 0;
          mo_objref_t dispobr = NULL;
          if (mom_valid_name (curdispname))
            {
              dispobr = mo_find_named_cstr (curdispname);
            }
          else if (curdispname[0] == '_'
                   && mo_get_hi_lo_ids_from_cstring (&hid, &loid,
                                                     curdispname))
            {
              dispobr = mo_objref_find_hid_loid (hid, loid);
            }
          else
            MOM_FATAPRINTF ("invalid display name %s", curdispname);
          if (!dispobr)
            MOM_FATAPRINTF ("cannot find displayed object %s", curdispname);
          momgui_displayed_objasso =
            mo_assoval_put (momgui_displayed_objasso, dispobr,
                            mo_int_to_value (MOMGUI_INITIAL_DEPTH));
        }
    }
  MOM_INFORMPRINTF ("Running GTK graphical interface...");
  momgui_begin_running ();
  sta = g_application_run (G_APPLICATION (mom_gtkapp), *pargc, *pargv);
  if (sta)
    MOM_WARNPRINTF ("Running GTK app gave %d", sta);
  g_object_unref (mom_gtkapp);
  mom_gtkapp = NULL;
  MOM_INFORMPRINTF ("Ended GTK graphical interface...");
  return;
}                               /* end mom_run_gtk */

// print to the command statusbar
static void
momgui_cmdstatus_printf (const char *fmt, ...)
{
  static guint statctxid;
  va_list args = { };
  if (MOM_UNLIKELY (statctxid == 0))
    {
      statctxid =
        gtk_statusbar_get_context_id (GTK_STATUSBAR (mom_cmdstatusbar),
                                      "MOMCMDSTATUS");
      MOM_ASSERTPRINTF (statctxid > 0, "bad statctxid %u", statctxid);
    }
  gtk_statusbar_remove_all (GTK_STATUSBAR (mom_cmdstatusbar), statctxid);
  char *buf = NULL;
  int len = 0;
  va_start (args, fmt);
  len = vasprintf (&buf, fmt, args);
  va_end (args);
  if (len < 0 || buf == NULL)
    MOM_FATAPRINTF ("cmdstatus_printf: vasprintf failed fmt=%s", fmt);
  gtk_statusbar_push (GTK_STATUSBAR (mom_cmdstatusbar), statctxid, buf);
  free (buf);
}                               /* end momgui_cmdstatus_printf */

// end of file gui.c
