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
 *  Started: Mon 17 June 2002, includes code from ?
 *
 *  Authors: Damien Sandras <dsandras@seconix.com>
 *           Jorn Baayen <jorn@nl.linux.com>
 *           Kenneth Christiansen <kenneth@gnu.org>
 */

#include <gtk/gtk.h>
#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "dialog.h"

static void gnomemeeting_dialog (GtkWindow *parent, const char *format, 
                                 va_list args, GtkMessageType type);

void
gnomemeeting_error_dialog (GtkWindow *parent, const char *format, ...)
{
  va_list args;
  
  va_start (args, format);
  
  gnomemeeting_dialog (parent, format, args, GTK_MESSAGE_ERROR);
  
  va_end (args);
}

void
gnomemeeting_warning_dialog (GtkWindow *parent, const char *format, ...)
{
  va_list args;
  
  va_start (args, format);
  
  gnomemeeting_dialog (parent, format, args, GTK_MESSAGE_WARNING);
  
  va_end (args);
}

void
gnomemeeting_message_dialog (GtkWindow *parent, const char *format, ...)
{
  va_list args;
  
  va_start (args, format);
  
  gnomemeeting_dialog (parent, format, args, GTK_MESSAGE_INFO);
  
  va_end (args);
}


static void 
popup_toggle_changed (GtkCheckButton *button, gpointer data)
{
  if (GTK_TOGGLE_BUTTON (button)->active)
  {
    /* changed to 'hide' */
    g_object_set_data (G_OBJECT (data), "widget_data", GINT_TO_POINTER (1));
  }
  else
  {
    /* changed to 'show' */
    g_object_set_data (G_OBJECT (data), "widget_data", GINT_TO_POINTER (0));
  }
}

/**
 * gnomemeeting_warning_dialog_on_widget:
 *
 * @parent: The parent of the dialog
 * @widget: The widget to associate the warning with.
 * @format: String containing printf like syntax.
 * @ ...  : Variables of different kinds called from the 
 *          format line.
 *
 * Only shows a dialog if the users has not clicked on
 * 'Do not show this dialog again' when associated with 
 * the same widget. 
 *
 * This can be useful in certain situations. For instance
 * you might have a toggle button for a setting that is not
 * allowed to change while the app is in a certain state.
 * When you change the toggle it checks if the change is 
 * allowed or else it calls this function associating it  
 * with the toggle button. If the user chooses to ignore 
 * the dialog in the rest of the session, then this dialog 
 * will not popup with new calls to this function when 
 * associating with the same toggle button.
 *
 * This function only works in the current session.
 **/
void 
gnomemeeting_warning_dialog_on_widget (GtkWindow *parent, 
                                       GtkWidget *widget,
                                       const char *format,...)
{
  va_list    args;
  GtkWidget *button = NULL;
  GtkWidget *dialog;
  char       buffer[1025];
  gint       allow_warnings;
  
  va_start (args, format);
  
  g_return_if_fail (widget != NULL);
  
  /* if not set, allow_warnings will get the value of 0 */
  allow_warnings = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (widget), 
                                                       "widget_data"));
  
  if (allow_warnings != 0)
  {
    /* doesn't show warning dialog as state is 'hide' */
    return;
  }
  
  button = gtk_check_button_new_with_label ("Do not show this dialog again");
  
  g_signal_connect (G_OBJECT (button), "toggled",
                    G_CALLBACK (popup_toggle_changed),
                    widget);
  
  /* automatically calls the popup_toggle_changed callback function 
   * and is get set to 'hide', so no reason to hide it manually. */
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
  
  vsnprintf (buffer, 1024, format, args);
  
  dialog = gtk_message_dialog_new (parent, 
                                   GTK_DIALOG_DESTROY_WITH_PARENT,
                                   GTK_MESSAGE_WARNING,
                                   GTK_BUTTONS_OK,
                                   buffer);
  
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), 
                     button);
  
  g_signal_connect_swapped (GTK_OBJECT (dialog), "response",
                            G_CALLBACK (gtk_widget_destroy),
                            GTK_OBJECT (dialog));
  
  gtk_widget_show_all (dialog);
  
  va_end (args);
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
static void
gnomemeeting_dialog (GtkWindow *parent, 
                     const char *format, 
                     va_list args, 
                     GtkMessageType type)
{
  GtkWidget *dialog;
  char buffer[1025];
  
  vsnprintf (buffer, 1024, format, args);
  
  dialog = gtk_message_dialog_new (parent, 
                                   GTK_DIALOG_DESTROY_WITH_PARENT,
                                   type,
                                   GTK_BUTTONS_OK,
                                   buffer);
  
  gtk_dialog_run (GTK_DIALOG (dialog));
  
  gtk_widget_destroy (dialog);
}
