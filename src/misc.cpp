
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2001 Damien Sandras
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

#include <esd.h>

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
    
    
    PTRACE(1, "Will Take GDK Lock");
    gdk_threads_enter ();
    PTRACE(1, "GDK Lock Taken");
  }
  else {

    PTRACE(1, "Ignore GDK Lock request : Main Thread");
  }
    
}


void gnomemeeting_threads_leave () {

  if (PThread::Current ()->GetThreadName () != "gnomemeeting") {

  
    PTRACE(1, "Will Release GDK Lock");
    gdk_threads_leave ();
    PTRACE(1, "GDK Lock Released");
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
  
  gtk_widget_set_usize (GTK_WIDGET (button), 80, 25);
  
  gtk_widget_show (pixmap);
  gtk_widget_show (label);
  gtk_widget_show (hbox2);
		
  return button;
}


GM_window_widgets *gnomemeeting_get_main_window (GtkWidget *gm)
{
  GM_window_widgets *gw = (GM_window_widgets *) 
    gtk_object_get_data (GTK_OBJECT (gm), "gw");

  return gw;
}


GM_pref_window_widgets *gnomemeeting_get_pref_window (GtkWidget *gm)
{
  GM_pref_window_widgets *pw = (GM_pref_window_widgets *) 
    gtk_object_get_data (GTK_OBJECT (gm), "pw");

  return pw;
}


GM_ldap_window_widgets *gnomemeeting_get_ldap_window (GtkWidget *gm)
{
  GM_ldap_window_widgets *lw = (GM_ldap_window_widgets *) 
    gtk_object_get_data (GTK_OBJECT (gm), "lw");

  return lw;
}


void gnomemeeting_log_insert (gchar *text)
{
  time_t *timeptr;
  char *time_str;

  time_str = (char *) malloc (21);
  timeptr = new (time_t);

  GM_window_widgets *gw = gnomemeeting_get_main_window (gm);

  time (timeptr);
  strftime(time_str, 20, "%H:%M:%S :  ", localtime (timeptr));

  gtk_text_insert (GTK_TEXT (gw->log_text), NULL, NULL, NULL, time_str, -1);
  gtk_text_insert (GTK_TEXT (gw->log_text), NULL, NULL, NULL, text, -1);
  gtk_text_insert (GTK_TEXT (gw->log_text), NULL, NULL, NULL, "\n", -1);

  free (time_str);
  delete (timeptr);
}


void gnomemeeting_init_main_window_logo ()
{
  GdkPixmap *text_logo;
  GdkBitmap *text_logo_mask;
  GdkRectangle update_rec;
  
  update_rec.x = 0;
  update_rec.y = 0;
  update_rec.width = GM_QCIF_WIDTH;
  update_rec.height = GM_QCIF_HEIGHT;

  GM_window_widgets *gw = gnomemeeting_get_main_window (gm);

  if (gw->drawing_area->allocation.width != GM_QCIF_WIDTH &&
      gw->drawing_area->allocation.height != GM_QCIF_HEIGHT) {
    
    gtk_drawing_area_size (GTK_DRAWING_AREA (gw->drawing_area), 
			   GM_QCIF_WIDTH, GM_QCIF_HEIGHT);
    gtk_widget_set_usize (GTK_WIDGET (gw->video_frame),
			  GM_QCIF_WIDTH + GM_FRAME_SIZE, GM_QCIF_HEIGHT);
  }

  gdk_draw_rectangle (gw->pixmap, gw->drawing_area->style->black_gc, TRUE,
		      0, 0, GM_QCIF_WIDTH, GM_QCIF_HEIGHT);

  text_logo = gdk_pixmap_create_from_xpm_d (gm->window, &text_logo_mask,
					    NULL,
					    (gchar **) text_logo_xpm);

  gdk_draw_pixmap (gw->pixmap, gw->drawing_area->style->black_gc, 
		   text_logo, 0, 0, 
		   (GM_QCIF_WIDTH - 150) / 2,
		   (GM_QCIF_HEIGHT - 62) / 2, -1, -1);

  gtk_widget_draw (gw->drawing_area, &update_rec);   
}


gint PlaySound (GtkWidget *widget)
{
  GtkWidget *object = NULL;
  int esd_client = 0;

  /* Put ESD into Resume mode */
  esd_client = esd_open_sound (NULL);

  if (widget != NULL) {

    /* First we check if it is the phone or the globe that is displayed.
       We can't call gnomemeeting_threads_enter as idles and timers
       are executed in the main thread */
    gdk_threads_enter ();
    object = (GtkWidget *) gtk_object_get_data (GTK_OBJECT (widget),
						"pixmapg");
    gdk_threads_leave ();
  }

  /* If the applet contents the phone pixmap */
  if (object == NULL) {
    esd_resume (esd_client);
    gnome_triggers_do ("", "program", "GnomeMeeting", 
		       "incoming_call", NULL);
  }
  else
    esd_standby (esd_client);

  esd_close (esd_client);

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
    gtk_object_set_data_full (GTK_OBJECT (combo), "history",
			      contacts_list, gnomemeeting_free_glist_data);

  g_free (contacts);

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
  if (!g_strcasecmp (entry_content, "")) {
    g_free (entry_content);
    return;
  }
  
  /* We read the max_contacs setting */
  client = gconf_client_get_default ();
  max_contacts = gconf_client_get_int (client,
				       "/apps/gnomemeeting/history/entries",
				       0);
  /* Put the current entry in the history of the combo */
  gtk_list_clear_items (GTK_LIST (GTK_COMBO(combo)->list), 
			0, -1);
  /* if the entry is not in the list */
  contacts_list = (GList *) (gtk_object_get_data (GTK_OBJECT (combo), "history"));
  if (contacts_list) {
    for (GList *temp = contacts_list; temp != 0; temp = g_list_next (temp)) {
      if (!g_strcasecmp ((gchar *)temp->data, entry_content)) {
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
      contacts_list = g_list_remove(contacts_list, last_item->data);
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
  
  gtk_object_remove_no_notify (GTK_OBJECT (combo), "history");
  if (contacts_list)
    gtk_object_set_data_full (GTK_OBJECT (combo), "history", 
			      contacts_list, gnomemeeting_free_glist_data);
}


/* This function overrides from a pwlib function */
void PAssertFunc (const char * file, int line, const char * msg)

{
  static BOOL inAssert;

  if (inAssert)
    return;

  inAssert = TRUE;

  ostream & trace = PTrace::Begin(0, file, line);
  trace << "PWLib\tAssertion fail";
  if (msg != NULL)
    trace << ": " << msg;
  trace << PTrace::End;

  if (&trace != &PError) {
    PError << "Assertion fail: File " << file << ", Line " << line << endl;
    if (msg != NULL)
      PError << msg << endl;
  }


  for(;;) {
    PError << "\nAbort, Core dump, Ignore"

           << "? " << flush;
    int c = getchar();

    switch (c) {
      case 'a' :
      case 'A' :
        PError << "\nAborting.\n";
        _exit(1);

      case 'c' :
      case 'C' :
        PError << "\nDumping core.\n";
        kill(getpid(), SIGABRT);

      case 'i' :
      case 'I' :
      case EOF :
        PError << "\nIgnoring.\n";
        inAssert = FALSE;
        return;
    }
  }
}


static void popup_toggle_changed (GtkCheckButton *but, gpointer data)
{
  if (GTK_TOGGLE_BUTTON (but)->active) 
    gtk_object_set_data (GTK_OBJECT (data), "widget_data", (gpointer) "1");
  else
    gtk_object_set_data (GTK_OBJECT (data), "widget_data", (gpointer) "0");
}


void gnomemeeting_warning_popup (GtkWidget *w, gchar *m)
{
  gchar *msg = NULL;
  gchar *widget_data = NULL;
  GtkWidget *msg_box = NULL;
  GtkWidget *toggle_button = NULL;

  msg = g_strdup (m);
     
  widget_data = (gchar *) gtk_object_get_data (GTK_OBJECT (w), "widget_data");

  toggle_button = 
    gtk_check_button_new_with_label (_("Do not show this dialog again"));
  
  gtk_signal_connect (GTK_OBJECT (toggle_button), "toggled",
		      GTK_SIGNAL_FUNC (popup_toggle_changed),
		      w);
		 
  /* If it is the first time that we are called OR if data is != 0 */
  if (!gtk_object_get_data (GTK_OBJECT (w), "widget_data")||
      (strcmp ((gchar *) gtk_object_get_data (GTK_OBJECT (w), "widget_data"), "0"))) {

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle_button), TRUE);
    gtk_object_set_data (GTK_OBJECT (w), "widget_data", (gpointer) "1");
  }

  
  if ((widget_data == NULL)||(!strcmp ((gchar *) gtk_object_get_data (GTK_OBJECT (w), "widget_data"), "0"))) {

    msg_box = gnome_message_box_new (msg, GNOME_MESSAGE_BOX_WARNING, 
				   "OK", NULL);

    gtk_container_add (GTK_CONTAINER (GNOME_DIALOG (msg_box)->vbox), 
		       toggle_button);
    
    gtk_widget_show_all (msg_box);
  }
  else
    gtk_widget_destroy (GTK_WIDGET (toggle_button));

  g_free (msg);
}
