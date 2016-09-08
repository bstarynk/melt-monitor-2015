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
static GQuark mom_gquark;
static GtkTextBuffer *mom_obtextbuf;
static GtkTextTagTable *mom_tagtable;
static GtkTextTag *mom_tag_toptitle;    // tag for top text
static GtkTextTag *mom_tag_objtitle;    // tag for object title line
static GtkTextTag *mom_tag_objsubtitle; // tag for object subtitle line
static GtkTextTag *mom_tag_objname;     // tag for object names
static GtkTextTag *mom_tag_attr;        // tag for attributes
static GtkTextTag *mom_tag_idstart;     // tag for first 6 characters of objids
static GtkTextTag *mom_tag_idrest;      // tag for rest of objids
static GtkTextTag *mom_tag_number;      // tag for numbers
static GtkTextTag *mom_tag_string;      // tag for strings
static GtkTextTag *mom_tag_sequence;    // tag for sequences (tuples & sets)
static GtkTextTag *mom_tag_time;        // tag for time
static GtkTextTag *mom_tag_comment;     // tag for comment
static GtkWidget *mom_appwin;
static GtkWidget *mom_tview1;
static GtkWidget *mom_tview2;
#define MOMGUI_IDSTART_LEN 6

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
#define MOMGUI_INITIAL_DEPTH 1
// the hashset of shown object occurrences
static mo_hashsetpayl_ty *momgui_shown_obocchset;

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
};
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


// destructor for momgui_dispobjinfo_ty, for g_hash_table_new_full
static void
mom_destroy_dispobjinfo (momgui_dispobjinfo_ty * dispobi)
{
  momgui_displayed_objasso =    //
    mo_assoval_remove (momgui_displayed_objasso, dispobi->mo_gdo_dispobr);
  g_clear_object (&dispobi->mo_gdo_startmark);
  g_clear_object (&dispobi->mo_gdo_endmark);
  memset (dispobi, 0, sizeof (*dispobi));
  free (dispobi);
}                               /* end of mom_destroy_dispobjinfo */

static void
mom_destroy_shownobocc (momgui_shownobocc_ty * shoboc)
{
  momgui_shown_obocchset =      //
    mo_hashset_remove (momgui_shown_obocchset, shoboc->mo_gso_showobr);
  g_clear_object (&shoboc->mo_gso_txtag);
  memset (shoboc, 0, sizeof (*shoboc));
  free (shoboc);
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



#define MOM_DISPLAY_INDENTED_NEWLINE(Piter,Depth,...) do {	\
  gtk_text_buffer_insert_with_tags				\
  (mom_obtextbuf, (Piter),					\
   "                \n"+(16-(Depth)%16), 1+(Depth)%16,		\
  ##__VA_ARGS__, NULL);						\
} while(0)

#define MOM_DISPLAY_OPENING(Piter,VarOff,Str,...) do {	\
    VarOff = gtk_text_iter_get_offset ((Piter));	\
    gtk_text_buffer_insert_with_tags			\
      (mom_obtextbuf, (Piter), (Str), strlen((Str)),	\
       ##__VA_ARGS__, NULL);				\
  } while(0)

#define MOM_DISPLAY_CLOSING(Piter,VarOff,Str,...) do {	\
    gtk_text_buffer_insert_with_tags			\
      (mom_obtextbuf, (Piter), (Str), strlen((Str)),	\
       ##__VA_ARGS__, NULL);				\
    VarOff = gtk_text_iter_get_offset ((Piter));	\
} while(0)


static void
mom_display_the_object (mo_objref_t obr, GtkTextIter * piter, int depth,
                        int maxdepth, momgui_dispobjinfo_ty * pardisp);

static void
mom_insert_objref_textbuf (mo_objref_t obr, GtkTextIter * piter)
{
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
          MOM_FATAPRINTF ("failed to allocate shownobocc_ty for obr=%s",
                          mo_objref_pnamestr (obr));
        shoc->mo_gso_showobr = obr;
        objtag = shoc->mo_gso_txtag
          = gtk_text_buffer_create_tag (mom_obtextbuf, idbuf,
                                        "font", "DejaVu Serif, Book",
                                        "background", "ivory", NULL);
      }
    else
      objtag = shoc->mo_gso_txtag;
  }
  MOM_ASSERTPRINTF (objtag != NULL, "no obtag for %s",
                    mo_objref_pnamestr (obr));
  if (namv)
    {
      gtk_text_buffer_insert_with_tags (mom_obtextbuf, piter,
                                        mo_string_cstr (namv),
                                        mo_string_size (namv),
                                        objtag, mom_tag_objname, NULL);
      return;
    }
  gtk_text_buffer_insert_with_tags (mom_obtextbuf, piter,
                                    idbuf,
                                    MOMGUI_IDSTART_LEN,
                                    objtag, mom_tag_idstart, NULL);
  gtk_text_buffer_insert_with_tags (mom_obtextbuf, piter,
                                    idbuf + MOMGUI_IDSTART_LEN,
                                    MOM_CSTRIDLEN - MOMGUI_IDSTART_LEN,
                                    objtag, mom_tag_idrest, NULL);
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
                                        ":", -1, objtag, NULL);
      gtk_text_buffer_insert_with_tags (mom_obtextbuf, piter,
                                        combuf, pb - combuf,
                                        objtag, mom_tag_comment, NULL);
      if (pc && *pc)
        gtk_text_buffer_insert_with_tags        //
          (mom_obtextbuf, piter, "\342\200\246",        // U+2026 HORIZONTAL ELLIPSIS …
           3, objtag, NULL);
    }                           // end if commv is string and anonymous
}                               /* end mom_insert_objref_textbuf */

static void
mom_insert_value_textbuf (mo_value_t val, GtkTextIter * piter,
                          int depth, int maxdepth, GtkTextTag * valtag)
{
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
        MOM_DISPLAY_INDENTED_NEWLINE (piter, depth, valtag);
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
          MOM_DISPLAY_OPENING (piter, startoff, "[", mom_tag_sequence,
                               valtag);
        else                    // set
          MOM_DISPLAY_OPENING (piter, startoff, "{", mom_tag_sequence,
                               valtag);
        if (seqsiz > 0)
          {
            for (unsigned ix = 0; ix < seqsiz; ix++)
              {
                if (showinside || (ix % 5 == 0 && ix > 0))
                  {
                    MOM_DISPLAY_INDENTED_NEWLINE (piter, depth + 1,
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
                    momgui_dispobjinfo_ty *curdisp = NULL;
#warning FIXME: should probably make a fresh curdisp
                    mom_display_the_object (cursubobj, piter, depth + 1,
                                            maxdepth, curdisp);
                  }
                else
                  {
                    mom_insert_objref_textbuf (cursubobj, piter);
                  }
              }
          }
        if (showinside)
          {
            MOM_DISPLAY_INDENTED_NEWLINE (piter, depth + 1,
                                          mom_tag_sequence, valtag);
          }
        if (!istuple)           //set
          MOM_DISPLAY_CLOSING (piter, endoff, "}", mom_tag_sequence, valtag);
        else                    // tuple
          MOM_DISPLAY_CLOSING (piter, endoff, "]", mom_tag_sequence, valtag);
      }
      break;
    case mo_KOBJECT:
      {
        mo_objref_t curobj = (mo_objref_t) val;
        bool showinside = depth < maxdepth && !mo_objref_namev (curobj)
          && !mo_assoval_get (momgui_displayed_objasso, curobj);
        if (showinside)
          {
            momgui_dispobjinfo_ty *curdisp = NULL;
#warning FIXME: should probably make a fresh curdisp
            mom_display_the_object (curobj, piter, depth + 1, maxdepth,
                                    curdisp);
          }
        else
          mom_insert_objref_textbuf (curobj, piter);

      }
      break;
    }
}                               /* end of mom_insert_value_textbuf */

static void
mom_display_the_object (mo_objref_t obr, GtkTextIter * piter, int depth,
                        int maxdepth, momgui_dispobjinfo_ty * pardisp)
{
  MOM_ASSERTPRINTF (mo_dyncast_objref (obr), "bad obr");
  MOM_ASSERTPRINTF (piter != NULL, "bad piter");
  MOM_ASSERTPRINTF (depth >= 0 && depth <= maxdepth
                    && maxdepth <= MOMGUI_MAX_DEPTH,
                    "bad depth %d maxdepth %d MOMGUI_MAX_DEPTH %d", depth,
                    maxdepth, MOMGUI_MAX_DEPTH);
  MOM_INFORMPRINTF ("display_the_object %s parent %s",
                    mo_objref_pnamestr (obr),
                    pardisp ? mo_objref_pnamestr (pardisp->mo_gdo_dispobr) :
                    "*none*");
  GtkTextTag *curobjtitletag =
    (depth == 0) ? mom_tag_objtitle : mom_tag_objsubtitle;
  gtk_text_buffer_get_end_iter (mom_obtextbuf, piter);
  MOM_DISPLAY_INDENTED_NEWLINE (piter, depth, NULL);
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
  MOM_DISPLAY_INDENTED_NEWLINE (piter, depth, curobjtitletag);
  char tibuf[72];
  memset (tibuf, 0, sizeof (tibuf));
  time_t nowt = 0;
  time (&nowt);
  struct tm nowtm = { };
  localtime_r (&nowt, &nowtm);
  // 64800 seconds is 18 hours, so show mtime as e.g. ⌚ 13:45:12 
  if (obr->mo_ob_mtime > nowt - 64800 && obr->mo_ob_mtime <= nowt)
    strftime (tibuf, sizeof (tibuf), "\342\214\232 " "%T", &nowtm);
  // 1728000 seconds is 20 days, so show mtime as e.g. ⌚ Aug 13, 14:25:57
  else if (obr->mo_ob_mtime > nowt - 1728000 && obr->mo_ob_mtime <= nowt)
    strftime (tibuf, sizeof (tibuf), "\342\214\232 " "%b %d, %T", &nowtm);
  else if (obr->mo_ob_mtime > 0)
    // otherwise -long ago or in the future- show as ⌚ 2016 Aug 17, 09:45:01
    strftime (tibuf, sizeof (tibuf), "\342\214\232 " "%Y %b %d, %T", &nowtm);
  else                          // unset time ⌚ ?
    strcpy (tibuf, "\342\214\232 ?");
  gtk_text_buffer_insert_with_tags (mom_obtextbuf, piter,
                                    tibuf, -1, mom_tag_time, NULL);
  mo_value_t classobr = obr->mo_ob_class;
  if (classobr)
    {
#warning FIXME: should shadow-show the class
    }
#warning FIXME: should get the set of attributes, properly sort them, and display each attribute
  mo_value_t attrset = mo_assoval_keys_set (obr->mo_ob_attrs);
#warning FIXME: should display the components
  int nbcomp = mo_vectval_count (obr->mo_ob_comps);
#warning FIXME: should display the payload
  MOM_DISPLAY_INDENTED_NEWLINE (piter, depth, NULL);
}                               /* end mom_display_the_object */


// an expensive operation, we regenerate everything. But that might be
// enough for a while, because computer is fast enough to redisplay
// several thousand objects...
void
mo_gui_generate_object_text_buffer (void)
{
  mo_value_t dispsetv = mo_assoval_keys_set (momgui_displayed_objasso);
  mo_assovaldatapayl_ty *oldispasso = momgui_displayed_objasso;
  unsigned nbdispob = mo_set_size (dispsetv);
  g_hash_table_remove_all (mom_dispobjinfo_hashtable);
  g_hash_table_remove_all (mom_shownobjocc_hashtable);
  momgui_displayed_objasso = mo_assoval_reserve (NULL,
                                                 2 * nbdispob + nbdispob / 3 +
                                                 20);
  momgui_shown_obocchset =
    mo_hashset_reserve (NULL, 3 * nbdispob + nbdispob / 2 + 40);
  gtk_text_buffer_set_text (mom_obtextbuf, "", 0);
  // sort the dispsetv in alphabetical order, or else obid order
  mo_objref_t *objarr =
    mom_gc_alloc (mom_prime_above (nbdispob + 1) * sizeof (mo_objref_t));
  if (nbdispob > 0)
    memcpy (objarr, ((mo_sequencevalue_ty *) dispsetv)->mo_seqobj,
            nbdispob * sizeof (mo_objref_t));
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
      GtkTextIter iter = { };
      gtk_text_buffer_get_end_iter (mom_obtextbuf, &iter);
      mo_objref_t curobj = objarr[ix];
      gtk_text_buffer_insert_with_tags (mom_obtextbuf, &iter,
                                        "\n", -1, NULL, NULL);
      MOM_ASSERTPRINTF (mo_dyncast_objref (curobj), "bad curobj ix#%d", ix);
      int maxdepth = mo_value_to_int (mo_assoval_get (oldispasso, curobj), 0);
      if (maxdepth <= 0)
        maxdepth = 1;
      else if (maxdepth > MOMGUI_MAX_DEPTH)
        maxdepth = MOMGUI_MAX_DEPTH;
      momgui_displayed_objasso =
        mo_assoval_put (momgui_displayed_objasso, curobj,
                        mo_int_to_value (maxdepth));
      mom_display_the_object (objarr[ix], &iter, 0, maxdepth, NULL);
      gtk_text_buffer_insert_with_tags (mom_obtextbuf, &iter,
                                        "\n", -1, NULL, NULL);
    }
}                               /* end mo_gui_generate_object_text_buffer */

void
mo_gui_display_object (mo_objref_t ob)
{
  if (!mo_dyncast_objref (ob) || mom_without_gui)
    return;
  if (mo_assoval_get (momgui_displayed_objasso, ob))
    return;
  momgui_displayed_objasso =    //
    mo_assoval_put (momgui_displayed_objasso, ob, mo_int_to_value (1));
  mo_gui_generate_object_text_buffer ();
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

static GtkWidget *
mom_objectentry (void)
{
  GtkWidget *obent = gtk_combo_box_text_new_with_entry ();
  gtk_widget_set_size_request (obent, 30, 10);
  mo_value_t namsetv = mo_named_objects_set ();
  int nbnam = mo_set_size (namsetv);
  MOM_ASSERTPRINTF (nbnam > 0, "bad nbnam");
  mo_value_t *namarr = mom_gc_alloc (nbnam * sizeof (mo_value_t));
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
    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (obent),
                                    mo_string_cstr (namarr[ix]));
  GtkWidget *combtextent = gtk_bin_get_child (GTK_BIN (obent));
  MOM_ASSERTPRINTF (GTK_IS_ENTRY (combtextent), "bad combtextent");
  MOM_ASSERTPRINTF (gtk_entry_get_completion (GTK_ENTRY (combtextent)) ==
                    NULL, "got completion in combtextent");
  // if the entered text starts with a letter, I want it to be
  // completed with the appended text above if the entered text starts
  // with an undersore, then a digit, then two alphanum (like _0BV or
  // _6S3 for example), I want to call a completion function.
#warning objectentry: what should I code here?
  return obent;
}                               /* end mom_objectentry */



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
  GtkWidget *contarea =
    gtk_dialog_get_content_area (GTK_DIALOG (displaydialog));
  GtkWidget *hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 3);
  gtk_container_add (GTK_CONTAINER (contarea), hbox);
  gtk_box_pack_start (GTK_BOX (hbox), gtk_label_new ("object:"),
                      FALSE, FALSE, 1);
  GtkWidget *obent = mom_objectentry ();
  gtk_box_pack_end (GTK_BOX (hbox), obent, TRUE, TRUE, 1);
  gtk_widget_show_all (displaydialog);
  mo_objref_t displobr = NULL;
  int res = 0;
  do
    {
      displobr = NULL;
      res = gtk_dialog_run (GTK_DIALOG (displaydialog));
      if (res == GTK_RESPONSE_OK)
        {
          gchar *nams =
            gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT (obent));
          if (nams && mom_valid_name (nams))
            displobr = mo_find_named_cstr (nams);
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

static void
mom_copy_edit (GtkMenuItem * menuitm MOM_UNUSED, gpointer data MOM_UNUSED)
{
  MOM_INFORMPRINTF ("copy_edit");
}                               /* end mom_copy_edit */

static void
mom_paste_edit (GtkMenuItem * menuitm MOM_UNUSED, gpointer data MOM_UNUSED)
{
  MOM_INFORMPRINTF ("paste_edit");
}                               /* end mom_paste_edit */

static void
mom_cut_edit (GtkMenuItem * menuitm MOM_UNUSED, gpointer data MOM_UNUSED)
{
  MOM_INFORMPRINTF ("cut_edit");
}                               /* end mom_cut_edit */


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
                                "scale", 0.8, NULL);
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
  mom_tag_time =
    gtk_text_buffer_create_tag (mom_obtextbuf,
                                "time",
                                "font", "Times, Italic",
                                "scale", 0.85,
                                "foreground", "skyblue4", NULL);
  mom_tag_comment =
    gtk_text_buffer_create_tag (mom_obtextbuf,
                                "time",
                                "font", "Verdana, Italic",
                                "scale", 0.60,
                                "foreground", "slategrey", NULL);
  // we should fill the mom_tagtable
#warning mom_initialize_gtk_tags_for_objects unimplemented
  MOM_WARNPRINTF ("mom_initialize_gtk_tags_for_objects unimplemented");
}                               /* end of mom_initialize_gtk_tags_for_objects */

static void
mom_gtkapp_activate (GApplication * app, gpointer user_data MOM_UNUSED)
{
  mom_appwin = gtk_application_window_new (GTK_APPLICATION (app));
  gtk_window_set_default_size (GTK_WINDOW (mom_appwin), 520, 460);
  GtkWidget *topvbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
  gtk_container_add (GTK_CONTAINER (mom_appwin), topvbox);
  /////////// create & fill the menubar
  GtkWidget *menubar = gtk_menu_bar_new ();
  /// app menu
  GtkWidget *appmenu = gtk_menu_new ();
  GtkWidget *appitem = gtk_menu_item_new_with_label ("App");
  gtk_menu_shell_append (GTK_MENU_SHELL (menubar), appitem);
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (appitem), appmenu);
  GtkWidget *dumpitem = gtk_menu_item_new_with_label ("Dump");
  GtkWidget *dumpexititem = gtk_menu_item_new_with_label ("dump & eXit");
  GtkWidget *quititem = gtk_menu_item_new_with_label ("Quit");
  gtk_menu_shell_append (GTK_MENU_SHELL (appmenu), dumpitem);
  gtk_menu_shell_append (GTK_MENU_SHELL (appmenu), dumpexititem);
  gtk_menu_shell_append (GTK_MENU_SHELL (appmenu), quititem);
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
  GtkWidget *copyitem = gtk_menu_item_new_with_label ("Copy");
  GtkWidget *pasteitem = gtk_menu_item_new_with_label ("Paste");
  GtkWidget *cutitem = gtk_menu_item_new_with_label ("Cut");
  gtk_menu_shell_append (GTK_MENU_SHELL (editmenu), displayitem);
  gtk_menu_shell_append (GTK_MENU_SHELL (editmenu), newobitem);
  gtk_menu_shell_append (GTK_MENU_SHELL (editmenu),
                         gtk_separator_menu_item_new ());
  gtk_menu_shell_append (GTK_MENU_SHELL (editmenu), copyitem);
  gtk_menu_shell_append (GTK_MENU_SHELL (editmenu), pasteitem);
  gtk_menu_shell_append (GTK_MENU_SHELL (editmenu), cutitem);
  g_signal_connect (displayitem, "activate", G_CALLBACK (mom_display_edit),
                    NULL);
  g_signal_connect (newobitem, "activate", G_CALLBACK (mom_newob_edit), NULL);
  g_signal_connect (copyitem, "activate", G_CALLBACK (mom_copy_edit), NULL);
  g_signal_connect (pasteitem, "activate", G_CALLBACK (mom_paste_edit), NULL);
  g_signal_connect (cutitem, "activate", G_CALLBACK (mom_cut_edit), NULL);
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
  gtk_box_pack_end (GTK_BOX (topvbox), paned, TRUE, TRUE, 2);
  mo_gui_generate_object_text_buffer ();
  gtk_widget_show_all (mom_appwin);
}                               /* end mom_gtkapp_activate */

static guint
momgui_objhash (const void *ob)
{
  if (ob != NULL)
    return mo_objref_hash ((mo_objref_t) ob);
  return 0;
}                               /* end momgui_objhash */

void
mom_run_gtk (int *pargc, char ***pargv)
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
  g_signal_connect (mom_gtkapp, "activate", G_CALLBACK (mom_gtkapp_activate),
                    NULL);
  MOM_INFORMPRINTF ("Running GTK graphical interface...");
  sta = g_application_run (G_APPLICATION (mom_gtkapp), *pargc, *pargv);
  if (sta)
    MOM_WARNPRINTF ("Running GTK app gave %d", sta);
  g_object_unref (mom_gtkapp);
  mom_gtkapp = NULL;
  MOM_INFORMPRINTF ("Ended GTK graphical interface...");
  return;
}                               /* end mom_run_gtk */
