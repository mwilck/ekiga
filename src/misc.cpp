
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2002 Damien Sandras
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/*
 *                         misc.cpp  -  description
 *                         ------------------------
 *   begin                : Thu Nov 22 2001
 *   copyright            : (C) 2000-2002 by Damien Sandras
 *   description          : This file contains miscellaneous functions.
 *   Additional Code      : De Michele Cristiano, Miguel Rodríguez 
 *
 */


#include "../config.h"

#include <ptlib.h>
#include <gnome.h>
#include "config.h"
#include "main_window.h"
#include "common.h"
#include "pref_window.h"

#include "misc.h"

#include "../pixmaps/text_logo.xpm"

/* Declarations */
extern GtkWidget *gm;

/* The functions */


/* DESCRIPTION   :  /
 * BEHAVIOR      : Frees data in a double linked list
 * PRE           : the list must have dinamically alocated data
 */
void gnomemeeting_free_glist_data (gpointer user_data)
{
  GList *list = (GList *) user_data;

  while (list) {
    g_free (list->data);
    list = list->next;
  }

  g_list_free (list);
}


void gnomemeeting_threads_enter () {

  if (PThread::Current ()->GetThreadName () != "gnomemeeting") {
    
    //    cout << "Will take GDK Lock" << endl << flush;
    PTRACE(1, "Will Take GDK Lock");
    gdk_threads_enter ();
    PTRACE(1, "GDK Lock Taken");
    //    cout << "GDK Lock Taken" << endl << flush;
  }
  else {

    PTRACE(1, "Ignore GDK Lock request : Main Thread");
  }
    
}


void gnomemeeting_threads_leave () {

  if (PThread::Current ()->GetThreadName () != "gnomemeeting") {

    //    cout << "Will Release GDK Lock" << endl << flush;
    //    cout << PThread::Current ()->GetThreadName () << endl << flush;
    PTRACE(1, "Will Release GDK Lock");
    gdk_threads_leave ();
    PTRACE(1, "GDK Lock Released");
    //    cout << "GDK Lock Released" << endl << flush;
  }
  else {

    PTRACE(1, "Ignore GDK UnLock request : Main Thread");
  }
    
}


GtkWidget *gnomemeeting_button (gchar *lbl, GtkWidget *pixmap)
{
  GtkWidget *button;
  GtkWidget *hbox2;
  GtkWidget *label;
  
  button = gtk_button_new ();
  label = gtk_label_new (N_(lbl));
  hbox2 = gtk_hbox_new (FALSE, 0);

  gtk_box_pack_start(GTK_BOX (hbox2), pixmap, TRUE, TRUE, GNOME_PAD_SMALL);  
  gtk_box_pack_start(GTK_BOX (hbox2), label, TRUE, TRUE, GNOME_PAD_SMALL);
  
  gtk_container_add (GTK_CONTAINER (button), hbox2);
    
  gtk_widget_show (pixmap);
  gtk_widget_show (label);
  gtk_widget_show (hbox2);
		
  return button;
}


GM_window_widgets *gnomemeeting_get_main_window (GtkWidget *gm)
{
  GM_window_widgets *gw = (GM_window_widgets *) 
    g_object_get_data (G_OBJECT (gm), "gw");

  return gw;
}


GM_pref_window_widgets *gnomemeeting_get_pref_window (GtkWidget *gm)
{
  GM_pref_window_widgets *pw = (GM_pref_window_widgets *) 
    g_object_get_data (G_OBJECT (gm), "pw");

  return pw;
}


GM_ldap_window_widgets *gnomemeeting_get_ldap_window (GtkWidget *gm)
{
  GM_ldap_window_widgets *lw = (GM_ldap_window_widgets *) 
    g_object_get_data (G_OBJECT (gm), "lw");

  return lw;
}


GmTextChat *gnomemeeting_get_chat_window (GtkWidget *gm)
{
  GmTextChat *chat = (GmTextChat *) 
    g_object_get_data (G_OBJECT (gm), "chat");

  return chat;
}


void gnomemeeting_log_insert (gchar *text)
{
  GtkTextIter start, end;
  GtkTextMark *mark;

  time_t *timeptr;
  char *time_str;

  time_str = (char *) malloc (21);
  timeptr = new (time_t);

  GM_window_widgets *gw = gnomemeeting_get_main_window (gm);

  time (timeptr);
  strftime(time_str, 20, "%H:%M:%S ", localtime (timeptr));

  gtk_text_buffer_get_bounds (GTK_TEXT_BUFFER (gw->history), 
			      &start, &end);
  gtk_text_buffer_insert (GTK_TEXT_BUFFER (gw->history), 
			  &end, time_str, -1);
  gtk_text_buffer_get_bounds (GTK_TEXT_BUFFER (gw->history), 
			      &start, &end);
  gtk_text_buffer_insert (GTK_TEXT_BUFFER (gw->history), &end, text, -1);
  gtk_text_buffer_get_bounds (GTK_TEXT_BUFFER (gw->history), 
			      &start, &end);
  gtk_text_buffer_insert (GTK_TEXT_BUFFER (gw->history), &end, "\n", -1);

  mark = gtk_text_buffer_create_mark (GTK_TEXT_BUFFER (gw->history), 
				      "current-position", &end, FALSE);

  gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (gw->history_view), mark, 
				0.0, FALSE, 0,0);
  
  free (time_str);
  delete (timeptr);
}


void gnomemeeting_init_main_window_logo ()
{
  GdkPixbuf *text_logo_pix = NULL;
  GtkRequisition size_request;

  GM_window_widgets *gw = gnomemeeting_get_main_window (gm);

  gtk_widget_size_request (GTK_WIDGET (gw->video_frame), &size_request);

  if ((size_request.width - GM_FRAME_SIZE != 176) || 
      (size_request.height != 144)) {

     gtk_widget_set_size_request (GTK_WIDGET (gw->video_frame),
				  176 + GM_FRAME_SIZE, 144);
  }

  text_logo_pix = gdk_pixbuf_new_from_xpm_data ((const char **) text_logo_xpm);
  gtk_image_set_from_pixbuf (GTK_IMAGE (gw->video_image),
			     GDK_PIXBUF (text_logo_pix));

  g_object_unref (text_logo_pix);
}


gint PlaySound (GtkWidget *widget)
{
  void *object = NULL;

  if (widget != NULL) {

    /* First we check if it is the phone or the globe that is displayed.
       We can't call gnomemeeting_threads_enter as idles and timers
       are executed in the main thread */
    gdk_threads_enter ();
    object = g_object_get_data (G_OBJECT (widget), "pixmapg");
    gdk_threads_leave ();
  }

  /* If the applet contents the phone pixmap */
  if (object == NULL) {

    gnome_triggers_do ("", "program", "GnomeMeeting", 
		       "incoming_call", NULL);
  }

  return TRUE;
}


GtkWidget*
gnomemeeting_history_combo_box_new (const gchar *key)
{
  GtkWidget* combo;
  gchar **contacts;
  int i;		
  combo = gtk_combo_new ();
  gchar *stored_contacts;
  GList *contacts_list;
  GConfClient *client = gconf_client_get_default ();
  stored_contacts = gconf_client_get_string (client,
					     key,
					     0);
  contacts_list = NULL;
  /* We read the history on the hard disk */
  
  contacts = g_strsplit (stored_contacts ? (stored_contacts) : (""), "|", 0);
  if (stored_contacts)
    g_free (stored_contacts);
  for (i = 0 ; contacts [i] != NULL ; i++)
    contacts_list = g_list_append (contacts_list, 
				   contacts [i]);
     
  if (contacts_list != NULL)
    gtk_combo_set_popdown_strings (GTK_COMBO (combo), 
				   contacts_list);
  gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(combo)->entry), ""); 
  if (contacts_list)
    g_object_set_data_full (G_OBJECT (combo), "history",
			    contacts_list, gnomemeeting_free_glist_data);


  return combo; 	
}


/* DESCRIPTION   :  /
 * BEHAVIOR      : Add a new entry to the history combo and saves it
 *                 in the gconf db.
 * PRE           : key is the gconf key used to store the history.
 */
void 
gnomemeeting_history_combo_box_add_entry (GtkCombo *combo, const gchar *key,
					  const gchar *new_entry)
{
  bool found = false;
  unsigned int max_contacts;
  gchar *entry_content;
  GList *contacts_list;
  GConfClient *client;
  
  /* we make a dup, because the entry text will change */
  entry_content = g_strdup (gtk_entry_get_text 
			    (GTK_ENTRY (GTK_COMBO (combo)->entry)));  
  
  /* if it is an empty entry_content, return */
  if (!strcmp (entry_content, "")) {
    g_free (entry_content);
    return;
  }
  
  /* We read the max_contacs setting */
  client = gconf_client_get_default ();
  max_contacts = gconf_client_get_int (client,
				       "/apps/gnomemeeting/history/entries",
				       0);

  /* if the entry is not in the list */
  contacts_list = (GList *) (g_object_get_data (G_OBJECT (combo), "history"));
  if (contacts_list) {
    for (GList *temp = contacts_list; temp != 0; temp = g_list_next (temp)) {

      if (!strcasecmp ((gchar *)temp->data, entry_content)) {
	found = true;
	break;
      }
    }
  }
  
  if (!found) {
    /* this will not store a copy of entry_content, but entry_content itself */
    contacts_list = 
      g_list_prepend (contacts_list, entry_content);

    if (g_list_length(contacts_list) > max_contacts ) {

      GList *last_item = g_list_last(contacts_list);
      contacts_list = g_list_remove (contacts_list, last_item->data);
      g_free (last_item->data);
    }

    /* well, time to store the list in gconf */
    gchar *history = 0;

    /* FIXME: This can be heavily improved */
    for (GList *item = contacts_list; item != 0; item = g_list_next (item)) {

      gchar *temp = g_strjoin ((history) ? ("|") : (""), 
			       (history) ? (history) : (""), item->data, 0);
      if (history)
	g_free (history);
      history = temp;
    }

    gconf_client_set_string (client, key,
			     history, 0);
    g_free (history);
  }   
  
  gtk_combo_set_popdown_strings (GTK_COMBO (combo), 
				 contacts_list);
  gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (combo)->entry), 
  		      entry_content);
  
  /* if found, it is not added in the GList, we can free it */
  if (found)
    g_free (entry_content);
  
  g_object_steal_data (G_OBJECT (combo), "history");
  if (contacts_list)
    g_object_set_data_full (G_OBJECT (combo), "history", 
			    contacts_list, gnomemeeting_free_glist_data);
}


/* Helper functions por the PAssert dialog */
static void passert_close_cb (GtkDialog *dialog, gpointer data)
{
  _exit (1);
}


/* This function overrides from a pwlib function */
void PAssertFunc (const char * file, int line, const char * msg)

{
  static bool inAssert;
  gchar *mesg = NULL;

  if (inAssert)
    return;

  inAssert = true;

  gnomemeeting_threads_enter ();
  mesg = g_strdup_printf (_("Error: %s\n"), msg);

  GtkWidget *dialog = 
    gtk_dialog_new_with_buttons (_("GnomeMeeting Error"),
				 GTK_WINDOW (gm),
				 GTK_DIALOG_MODAL,
				 _("Ignore"), 0,
				 NULL);

  GtkWidget *label = gtk_label_new (mesg);
  
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), label, TRUE, TRUE, 0);
  
  gtk_widget_show_all (dialog);
  gnomemeeting_threads_leave ();

  gnomemeeting_threads_enter ();

  g_signal_connect_swapped (GTK_OBJECT (dialog),
			    "response",
			    G_CALLBACK (gtk_widget_destroy),
			    GTK_OBJECT (dialog));  

  gnomemeeting_threads_leave ();

  inAssert = FALSE;

  return;
}


static void popup_toggle_changed (GtkCheckButton *but, gpointer data)
{
  if (GTK_TOGGLE_BUTTON (but)->active) 
    g_object_set_data (G_OBJECT (data), "widget_data", GINT_TO_POINTER (1));
  else
    g_object_set_data (G_OBJECT (data), "widget_data", GINT_TO_POINTER (0));
}


void gnomemeeting_warning_popup (GtkWidget *w, gchar *m)
{
  gchar *msg = NULL;
  gint widget_data;
  GtkWidget *msg_box = NULL;
  GtkWidget *toggle_button = NULL;

  msg = g_strdup (m);
     
  if (w)
    widget_data = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (w), 
						      "widget_data"));
  else
    widget_data = 0;

  if (w) {

    toggle_button = 
      gtk_check_button_new_with_label (_("Do not show this dialog again"));

    g_signal_connect (G_OBJECT (toggle_button), "toggled",
		      G_CALLBACK (popup_toggle_changed),
		      w);
  }
		 
  /* If it is the first time that we are called OR if data is != 0 */
  if (w)
    if (GPOINTER_TO_INT (g_object_get_data (G_OBJECT (w), "widget_data"))==0) {

      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle_button), TRUE);
      g_object_set_data (G_OBJECT (w), "widget_data", GINT_TO_POINTER (1));
    }

  
  if (widget_data == 0) {

    msg_box = gtk_message_dialog_new (GTK_WINDOW (gm),
				      GTK_DIALOG_DESTROY_WITH_PARENT,
				      GTK_MESSAGE_ERROR,
				      GTK_BUTTONS_CLOSE,
				      msg);

    g_signal_connect_swapped (GTK_OBJECT (msg_box), "response",
			      G_CALLBACK (gtk_widget_destroy),
			      GTK_OBJECT (msg_box));
    
    gtk_widget_show (msg_box);

    
    if (w)
      gtk_container_add (GTK_CONTAINER (GTK_DIALOG (msg_box)->vbox), 
			 toggle_button);
    
    gtk_widget_show_all (msg_box);
  }
  else
    if (w)
      gtk_widget_destroy (GTK_WIDGET (toggle_button));

  g_free (msg);
}
