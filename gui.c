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
static GtkTextBuffer *mom_textbuf;
static GtkWidget *mom_appwin;


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
  MOM_INFORMPRINTF ("quitdialog=%p", quitdialog);
  int res = gtk_dialog_run (GTK_DIALOG (quitdialog));
  MOM_INFORMPRINTF ("res=%d", res);
  if (res == GTK_RESPONSE_CLOSE)
    {
      mom_dump_dir = "-";
      g_application_quit (G_APPLICATION (mom_gtkapp));
    }
  gtk_widget_destroy (quitdialog);
  quitdialog = NULL;
}                               /* end of mom_quit_app */


//////////////// create the GUI
static void
mom_gtkapp_activate (GApplication * app, gpointer user_data MOM_UNUSED)
{
  mom_appwin = gtk_application_window_new (GTK_APPLICATION (app));
  gtk_window_set_default_size (GTK_WINDOW (mom_appwin), 520, 460);
  GtkWidget *topvbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
  gtk_container_add (GTK_CONTAINER (mom_appwin), topvbox);
  /// create & fill the menubar
  GtkWidget *menubar = gtk_menu_bar_new ();
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
  g_signal_connect (dumpexititem, "activate", G_CALLBACK (mom_dumpexit_app),
                    NULL);
  g_signal_connect (dumpitem, "activate", G_CALLBACK (mom_dump_app), NULL);
  g_signal_connect (quititem, "activate", G_CALLBACK (mom_quit_app), NULL);
  gtk_box_pack_start (GTK_BOX (topvbox), menubar, FALSE, FALSE, 2);
  ////
  mom_textbuf = gtk_text_buffer_new (NULL);
  GtkWidget *paned = gtk_paned_new (GTK_ORIENTATION_VERTICAL);
  GtkWidget *tview1 = gtk_text_view_new_with_buffer (mom_textbuf);
  GtkWidget *tview2 = gtk_text_view_new_with_buffer (mom_textbuf);
  GtkWidget *scrotv1 = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (scrotv1), tview1);
  GtkWidget *scrotv2 = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (scrotv2), tview2);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrotv1),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrotv2),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
  gtk_paned_add1 (GTK_PANED (paned), scrotv1);
  gtk_paned_add2 (GTK_PANED (paned), scrotv2);
  gtk_box_pack_end (GTK_BOX (topvbox), paned, TRUE, TRUE, 2);
  gtk_widget_show_all (mom_appwin);
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
