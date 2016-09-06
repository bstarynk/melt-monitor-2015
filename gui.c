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
                    mo_object_pnamestr (obr), gobj);
  obr->mo_ob_paylkind = MOM_PREDEF (payload_gobject);
  obr->mo_ob_payldata = gobj;
  g_object_weak_ref (gobj, gobject_weak_notify_mom, obr);
}                               /* end of mo_objref_put_gobject_payload */




/***
 We might have a `the_GUI` predefined object, with a `ui_xml`
 attribute giving the long string to be passed to
 gtk_builder_new_from_string then use gtk_builder_connect_signals_full
 and/or gtk_builder_add_callback_symbol
***/

static void
mom_gtkbuilder_connect (GtkBuilder * builder MOM_UNUSED,
                        GObject * obj,
                        const gchar * signam,
                        const gchar * hdlnam,
                        GObject * connectobj,
                        GConnectFlags flags, gpointer data)
{
  mo_objref_t obr = data;
  MOM_ASSERTPRINTF (mo_dyncast_objref (obr), "bad obr");
  MOM_INFORMPRINTF ("gtkbuilder_connect obr:%s obj@%p(gtyp=%s,gcla=%s),\n"
                    ".. signam=%s, hdlnam=%s"
                    " connectobj@%p(gtyp=%s,gcla=%s) flags=%#x\n",
                    mo_object_pnamestr (obr),
                    (void *) obj,
                    G_OBJECT_TYPE_NAME (obj), G_OBJECT_CLASS_NAME (obj),
                    signam, hdlnam,
                    (void *) connectobj,
                    connectobj ? G_OBJECT_TYPE_NAME (connectobj) : "*nil*",
                    connectobj ? G_OBJECT_CLASS_NAME (connectobj) : "*Nil*",
                    (unsigned) flags);
}                               /* end mom_gtkbuilder_connect */

static void
quitapp_mom (GSimpleAction * action, GVariant * param, gpointer app)
{
  MOM_INFORMPRINTF ("quitapp action@%p (gtyp=%s,gcla=%s) app@%p",
                    action, G_OBJECT_TYPE_NAME (action),
                    G_OBJECT_CLASS_NAME (action), (void *) app);
  MOM_INFORMPRINTF ("quitapp param@%p vty=%s", param,
                    g_variant_get_type_string (param));
}                               /* end quitapp_mom */

static void
dumpexitapp_mom (GSimpleAction * action, GVariant * param, gpointer app)
{
  MOM_INFORMPRINTF ("dumpexitapp action@%p (gtyp=%s,gcla=%s) app@%p",
                    action, G_OBJECT_TYPE_NAME (action),
                    G_OBJECT_CLASS_NAME (action), (void *) app);
  MOM_INFORMPRINTF ("dumpexitapp param@%p vty=%s", param,
                    g_variant_get_type_string (param));
}                               /* end dumpexitapp_mom */

static GActionEntry momapp_actionentries[] =
{
  // see http://stackoverflow.com/a/27539131
  { "$mom.quit", quitapp_mom, NULL, NULL, NULL, {0, 0, 0} },
  { "$mom.dumpexit", dumpexitapp_mom, NULL, NULL, NULL, {0, 0, 0} }
};

static void
mom_gtkapp_activate (GApplication * app, gpointer user_data MOM_UNUSED)
{
  GtkWidget *widget = NULL;
  widget = gtk_application_window_new (GTK_APPLICATION (app));
  mo_value_t bldstrv =
    mo_objref_get_attr (MOM_PREDEF (the_GUI), MOM_PREDEF (xml_gtkbuild));
  if (!mo_dyncast_string (bldstrv))
    MOM_FATAPRINTF ("gtkapp_activate: bad bldstrv");
  MOM_INFORMPRINTF ("gtkapp_activate: bldstrv=%s\n",
                    mo_string_cstr (bldstrv));
  GtkBuilder *builder = gtk_builder_new_from_string (mo_string_cstr (bldstrv),
                                                     mo_string_size
                                                     (bldstrv));
  MOM_INFORMPRINTF ("gtkapp_activate: adding callbacks...");
  gtk_builder_add_callback_symbol (builder, "$mom.quit",
                                   G_CALLBACK (quitapp_mom));
  gtk_builder_add_callback_symbol (builder, "$mom.dumpexit",
                                   G_CALLBACK (dumpexitapp_mom));
  MOM_INFORMPRINTF ("gtkapp_activate: adding actions...");
  g_action_map_add_action_entries (G_ACTION_MAP (app),
                                   momapp_actionentries, G_N_ELEMENTS (momapp_actionentries),
                                   app);
  MOM_INFORMPRINTF ("gtkapp_activate: connecting signals...");
  gtk_builder_connect_signals_full (builder, mom_gtkbuilder_connect,
                                    MOM_PREDEF (the_GUI));
  GMenuModel *app_menu =
    G_MENU_MODEL (gtk_builder_get_object (builder, "appmenu"));
  gtk_application_set_app_menu (GTK_APPLICATION (app), app_menu);
  gtk_widget_show (widget);
  g_object_unref (builder);
}                               /* end mom_gtkapp_activate */


void
mom_run_gtk (int *pargc, char ***pargv)
{
  int sta = 0;
  mom_gquark = g_quark_from_static_string ("monimelt");
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