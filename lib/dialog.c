/*  dialog.c
 *
 *  GnomeMeeting -- A Video-Conferencing application
 *  Copyright (C) 2000-2002 Damien Sandras
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *
 *  Authors: Damien Sandras <dsandras@seconix.com>
 *           Jorn Baayen <jorn@nl.linux.com>
 *           Kenneth Christiansen <kenneth@gnu.org>
 */

/*
 *                         dialog.c  -  description
 *                         ------------------------
 *   begin                : Mon Jun 17 2002
 *   copyright            : (C) 2000-2002 by Damien Sandras
 *   description          : This file contains all the functions needed to
 *                          to create dialogs for GnomeMeeting.
 */

#include "../config.h"
#include <gtk/gtk.h>
#include <glib.h>

#ifndef DISABLE_GNOME
#include <gnome.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "dialog.h"

#ifdef WIN32
#define vsnprintf _vsnprintf
#endif

#ifndef _
#ifdef DISABLE_GNOME
#include <libintl.h>
#define _(x) gettext(x)
#ifdef gettext_noop
#define N_(String) gettext_noop (String)
#else
#define N_(String) (String)
#endif
#endif
#endif


static GtkWidget *
gnomemeeting_dialog (GtkWindow *parent,
		     const char *primary_text,
		     const char *format, 
		     va_list args,
		     GtkMessageType type);


GtkWidget *
gnomemeeting_error_dialog (GtkWindow *parent,
			   const char *primary_text,
			   const char *format,
			   ...)
{
  GtkWidget *dialog = NULL;
  va_list args;
  
  va_start (args, format);

  dialog =
    gnomemeeting_dialog (parent, primary_text, format, args,
			 GTK_MESSAGE_ERROR);
  
  va_end (args);
}


GtkWidget *
gnomemeeting_warning_dialog (GtkWindow *parent,
			     const char *primary_text,
			     const char *format,
			     ...)
{
  GtkWidget *dialog = NULL;
  va_list args;
  
  va_start (args, format);

  dialog =
    gnomemeeting_dialog (parent, primary_text, format, args,
			 GTK_MESSAGE_WARNING);
  
  va_end (args);

  return dialog;
}


GtkWidget *
gnomemeeting_message_dialog (GtkWindow *parent,
			     const char *primary_text,
			     const char *format,
			     ...)
{
  GtkWidget *dialog = NULL;
  va_list args;
  
  va_start (args, format);
  
  dialog =
    gnomemeeting_dialog (parent, primary_text, format, args, GTK_MESSAGE_INFO);
  
  va_end (args);

  return dialog;
}


static void 
popup_toggle_changed (GtkCheckButton *button,
		      gpointer data)
{
  if (GTK_TOGGLE_BUTTON (button)->active) {

    /* changed to 'hide' */
    g_object_set_data (G_OBJECT (data), "widget_data", (gpointer) "1" );
  }
  else  {

    /* changed to 'show' */
    g_object_set_data (G_OBJECT (data), "widget_data", (gpointer) "0");
  }
}


GtkWidget *
gnomemeeting_warning_dialog_on_widget (GtkWindow *parent, 
                                       GtkWidget *widget,
				       const char *primary_text,
                                       const char *format,
				       ...)
{
  va_list args;
  GtkWidget *button = NULL;
  GtkWidget *dialog = NULL;
  char buffer[1025];
  gchar *do_not_show = NULL;
  gchar *prim_text = NULL;
  gchar *dialog_text = NULL;
  
  va_start (args, format);
  
  g_return_if_fail (widget != NULL);


  /* if not set, do_not_show will get the value of 0 */
  do_not_show = (gchar *) g_object_get_data (G_OBJECT (widget), "widget_data");


  if ((do_not_show)&&(!strcmp (do_not_show, "1")))
  {
    /* doesn't show warning dialog as state is 'hide' */
    return;
  }
  
  button = 
    gtk_check_button_new_with_label (_("Do not show this dialog again"));
  
  g_signal_connect (G_OBJECT (button), "toggled",
                    G_CALLBACK (popup_toggle_changed),
                    widget);

  if ((do_not_show == NULL)||(!strcmp (do_not_show, "1"))) {

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
    g_object_set_data (G_OBJECT (widget), "widget_data", "1");
  }
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), FALSE);

    
  
  vsnprintf (buffer, 1024, format, args);

  prim_text =
    g_strdup_printf ("<span weight=\"bold\" size=\"larger\">%s</span>",
		     primary_text);
  vsnprintf (buffer, 1024, format, args);
  
  dialog_text =
    g_strdup_printf ("%s\n\n%s", prim_text, buffer);

  dialog = gtk_message_dialog_new (parent, 
                                   GTK_DIALOG_MODAL,
                                   GTK_MESSAGE_WARNING,
                                   GTK_BUTTONS_OK,
                                   NULL);

  gtk_label_set_markup (GTK_LABEL (GTK_MESSAGE_DIALOG (dialog)->label),
			dialog_text);

  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), 
                     button);
  
  g_signal_connect_swapped (GTK_OBJECT (dialog), "response",
                            G_CALLBACK (gtk_widget_destroy),
                            GTK_OBJECT (dialog));
  
  gtk_widget_show_all (dialog);
  
  va_end (args);

  g_free (prim_text);
  g_free (dialog_text);

  return dialog;
}


/**
 * gnomemeeting_dialog
 *
 * @parent: The parent window of the dialog.
 * @format: a char * including printf formats
 * @args  : va_list that the @format char * uses.
 * @type  : specifies the kind of GtkMessageType dialogs to use. 
 *
 * Creates and runs a dialog and destroys it afterward. 
 **/
static GtkWidget *
gnomemeeting_dialog (GtkWindow *parent,
		     const char *prim_text,
                     const char *format, 
                     va_list args, 
                     GtkMessageType type)
{
  GtkWidget *dialog;
  gchar *primary_text = NULL;
  gchar *dialog_text = NULL;
  char buffer [1025];

  primary_text =
    g_strdup_printf ("<span weight=\"bold\" size=\"larger\">%s</span>",
		     prim_text);
  
  vsnprintf (buffer, 1024, format, args);

  dialog_text =
    g_strdup_printf ("%s\n\n%s", primary_text, buffer);
  
  dialog =
    gtk_message_dialog_new (parent, GTK_DIALOG_MODAL, type,
			    GTK_BUTTONS_OK, NULL);

  gtk_label_set_markup (GTK_LABEL (GTK_MESSAGE_DIALOG (dialog)->label),
			dialog_text);
  
  g_signal_connect_swapped (GTK_OBJECT (dialog), "response",
                            G_CALLBACK (gtk_widget_destroy),
                            GTK_OBJECT (dialog));
  
  gtk_widget_show_all (dialog);

  g_free (dialog_text);
  g_free (primary_text);

  return dialog;
}
