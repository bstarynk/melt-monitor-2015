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

static void
mom_gtkapp_activate (GApplication * app, gpointer user_data MOM_UNUSED)
{
  GtkWidget *widget;

  widget = gtk_application_window_new (GTK_APPLICATION (app));
  gtk_widget_show (widget);
}                               /* end mom_gtkapp_activate */

void
mom_run_gtk (int *pargc, char ***pargv)
{
  int sta = 0;
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
