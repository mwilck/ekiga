
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2006 Damien Sandras
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Ekiga is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OPAL, OpenH323 and PWLIB, and distribute the combination,
 * without applying the requirements of the GNU GPL to the OPAL, OpenH323
 * and PWLIB programs, as long as you do follow the requirements of the
 * GNU GPL for all the rest of the software thus combined.
 */


/*
 *                         log_window.cpp  -  description
 *                         -------------------------
 *   begin                : Sun Sep 1 2002
 *   copyright            : (C) 2000-2006 by Damien Sandras 
 *   description          : This file defines functions to manage the log
 *
 */

#include "../../config.h"


#include "common.h"

#include "history.h"

#include "callbacks.h"
#include "misc.h"

#include <gmstockicons.h>


#ifdef WIN32
#include "winpaths.h"
#endif

struct GmHistoryWindow_ {

  GtkWidget *hw_text_view;		/* The text view of the log window */
};


typedef struct GmHistoryWindow_ GmHistoryWindow;


#define GM_HISTORY_WINDOW(x) (GmHistoryWindow *) (x)


/* Declarations */


/* GUI functions */


/* DESCRIPTION  : / 
 * BEHAVIOR     : Frees a GmHistoryWindow and its content.
 * PRE          : A non-NULL pointer to a GmHistoryWindow.
 */
static void gm_hw_destroy (gpointer);


/* DESCRIPTION  : / 
 * BEHAVIOR     : Returns a pointer to the private GmHistoryWindow
 * 		  used by the address book GMObject.
 * PRE          : The given GtkWidget pointer must be an history book GMObject.
 */
static GmHistoryWindow *gm_hw_get_hw (GtkWidget *);


/* Implementation */

static void
gm_hw_destroy (gpointer hw)
{
  g_return_if_fail (hw != NULL);

  delete ((GmHistoryWindow *) hw);
}


static GmHistoryWindow *
gm_hw_get_hw (GtkWidget *history_window)
{
  g_return_val_if_fail (history_window != NULL, NULL);

  return GM_HISTORY_WINDOW (g_object_get_data (G_OBJECT (history_window), "GMObject"));
}


/* Global functions */

GtkWidget *
gm_history_window_new ()
{
  GmHistoryWindow *hw = NULL;
  
  GdkPixbuf *pixbuf = NULL;
  GtkWidget *window = NULL;
  GtkWidget *scr = NULL;

  GtkTextMark *mark = NULL;
  GtkTextBuffer *buffer = NULL;
  GtkTextIter end;

  window = gtk_dialog_new ();
  gtk_dialog_add_button (GTK_DIALOG (window), GTK_STOCK_CLOSE, GTK_RESPONSE_CANCEL);
  g_object_set_data_full (G_OBJECT (window), "window_name",
			  g_strdup ("log_window"), g_free);
  
  gtk_window_set_title (GTK_WINDOW (window), _("General History"));
  pixbuf = gtk_widget_render_icon (GTK_WIDGET (window),
				   GM_STOCK_16,
				   GTK_ICON_SIZE_MENU, NULL);
  gtk_window_set_icon (GTK_WINDOW (window), pixbuf);
  g_object_unref (pixbuf);

  gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);
  
  
  /* The GMObject data */
  hw = new GmHistoryWindow ();
  g_object_set_data_full (G_OBJECT (window), "GMObject", 
			  hw, (GDestroyNotify) gm_hw_destroy);

  
  hw->hw_text_view = gtk_text_view_new ();
  gtk_text_view_set_editable (GTK_TEXT_VIEW (hw->hw_text_view), FALSE);

  gtk_text_view_set_editable (GTK_TEXT_VIEW (hw->hw_text_view), FALSE);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (hw->hw_text_view), GTK_WRAP_WORD);
  gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (hw->hw_text_view), FALSE);

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (hw->hw_text_view));
  gtk_text_buffer_get_end_iter (buffer, &end);
  mark = gtk_text_buffer_create_mark (GTK_TEXT_BUFFER (buffer), 
				      "current-position", &end, FALSE);

  scr = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scr), 
				       GTK_SHADOW_IN);
  gtk_container_set_border_width (GTK_CONTAINER (scr), 6);
  
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_ALWAYS);

  gtk_container_add (GTK_CONTAINER (scr), hw->hw_text_view);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->vbox), scr,
		      TRUE, TRUE, 0);
  
  g_signal_connect_swapped (GTK_OBJECT (window), 
			    "response", 
			    G_CALLBACK (gnomemeeting_window_hide),
			    (gpointer) window);

  g_signal_connect (GTK_OBJECT (window), "delete-event", 
                    G_CALLBACK (delete_window_cb), NULL);
  
  gtk_widget_show_all (GTK_WIDGET (GTK_DIALOG (window)->vbox));
  
  return window;
}


void 
gm_history_window_insert (GtkWidget *log_window, 
			  const char *format,
			  ...)
{
  va_list args;
  GmHistoryWindow *hw = NULL;

  GtkTextIter end;
  GtkTextMark *mark;
  GtkTextBuffer *buffer;
	
  time_t *timeptr;
  char *time_str;
  gchar *text_buffer = NULL;
  char buf [1025];

  g_return_if_fail (log_window != NULL && format != NULL);
  
  hw = gm_hw_get_hw (log_window);
  
  va_start (args, format);

  vsnprintf (buf, 1024, format, args);

  time_str = (char *) malloc (21);
  timeptr = new (time_t);

  time (timeptr);
  strftime(time_str, 20, "%H:%M:%S", localtime (timeptr));

  text_buffer = g_strdup_printf ("%s %s\n", time_str, buf);

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (hw->hw_text_view));

  gtk_text_buffer_get_iter_at_offset (GTK_TEXT_BUFFER (buffer), &end, -1);
  gtk_text_buffer_insert (GTK_TEXT_BUFFER (buffer), &end, text_buffer, -1);

  mark = gtk_text_buffer_get_mark (GTK_TEXT_BUFFER (buffer), 
				   "current-position");

  if (mark)
    gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (hw->hw_text_view), mark, 
 				  0.0, FALSE, 0,0);
  
  g_free (text_buffer);
  free (time_str);
  delete (timeptr);
}
