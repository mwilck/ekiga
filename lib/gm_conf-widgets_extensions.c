
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2004 Damien Sandras
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
 *
 *
 * GnomeMeting is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OpenH323 and Pwlib, and distribute the combination, without
 * applying the requirements of the GNU GPL to the OpenH323 program, as long
 * as you do follow the requirements of the GNU GPL for all the rest of the
 * software thus combined.
 */


/*
 *                         gm_conf.h  -  description 
 *                         ------------------------------------------
 *   begin                : Fri Oct 17 2003, but based on older code
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : helper notifiers for gnomemeeting's widgets
 *
 */

#include "gm_conf-widgets_extensions.h"


/* GTK Callbacks */
/*
 * There are 2 callbacks, one modifying the config key, ie the GTK callback
 * and the config notifier (_nt) updating the widget back when a config key
 *  changes.
 *
 */

/* DESCRIPTION  :  This function is called when an entry is activated.
 * BEHAVIOR     :  Updates the key given as parameter to the new value of the
 *                 entry.  
 * PRE          :  Non-Null data corresponding to the string config key
 *                 to modify.
 */
void
entry_activate_changed (GtkWidget *w,
                        gpointer data)
{
  entry_focus_changed (w, NULL, data);
}


/* DESCRIPTION  :  This function is called when the focus of an entry changes.
 * BEHAVIOR     :  Updates the key given as parameter to the new value of the
 *                 entry.  
 * PRE          :  Non-Null data corresponding to the string config key
 *                 to modify.
 */
gboolean
entry_focus_changed (GtkWidget  *w,
                     GdkEventFocus *ev,
                     gpointer data)
{
  gchar *key = NULL;
  gchar *current_value = NULL;
  
  key = (gchar *) data;

  current_value = gm_conf_get_string (key);

  if (!current_value 
      || strcmp (current_value, gtk_entry_get_text (GTK_ENTRY (w)))) {

    gm_conf_set_string (key, (gchar *)gtk_entry_get_text (GTK_ENTRY (w)));
  }
  g_free (current_value);

  return FALSE;
}

/* DESCRIPTION  :  Generic notifiers for entries.
 *                 This callback is called when a specific key of
 *                 the config database associated with an entry changes.
 * BEHAVIOR     :  It updates the widget.
 * PRE          :  The config key triggering that notifier on modification
 *                 should be of type string.
 */
void
entry_changed_nt (gpointer cid, 
		  GmConfEntry *entry,
		  gpointer data)
{
  GtkWidget *e = NULL;
  gchar *current_value = NULL;
  
  if (gm_conf_entry_get_type(entry) == GM_CONF_STRING) {

    gdk_threads_enter ();
  
    e = GTK_WIDGET (data);
    current_value = (gchar *) gm_conf_entry_get_string (entry);

    if (current_value
	&& strcmp (current_value, gtk_entry_get_text (GTK_ENTRY (e)))) {

      g_signal_handlers_block_matched (G_OBJECT (e),
				       G_SIGNAL_MATCH_FUNC,
				       0, 0, NULL,
				       (gpointer) entry_focus_changed,
				       NULL);
      g_signal_handlers_block_matched (G_OBJECT (e),
				       G_SIGNAL_MATCH_FUNC,
				       0, 0, NULL,
				       (gpointer) entry_activate_changed,
				       NULL);
      gtk_entry_set_text (GTK_ENTRY (e), current_value);
      g_signal_handlers_unblock_matched (G_OBJECT (e),
					 G_SIGNAL_MATCH_FUNC,
					 0, 0, NULL,
					 (gpointer) entry_activate_changed,
					 NULL);
      g_signal_handlers_unblock_matched (G_OBJECT (e),
					 G_SIGNAL_MATCH_FUNC,
					 0, 0, NULL,
					 (gpointer) entry_focus_changed,
					 NULL);
    }

    gdk_threads_leave (); 
  }
}

/* DESCRIPTION  :  This function is called when a toggle changes.
 * BEHAVIOR     :  Updates the key given as parameter to the new value of the
 *                 toggle.  
 * PRE          :  Non-Null data corresponding to the boolean config key to
 *                 modify.
 */
void
toggle_changed (GtkCheckButton *but,
		gpointer data)
{
  gchar *key = NULL; 

  key = (gchar *) data;

  if (gm_conf_get_bool (key)
      != gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (but)))
    gm_conf_set_bool (key, 
		      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (but)));
}


/* DESCRIPTION  :  Generic notifiers for toggles.
 *                 This callback is called when a specific key of
 *                 the config database associated with a toggle changes, this
 *                 only updates the toggle.
 * BEHAVIOR     :  It only updates the widget.
 * PRE          :  The config key triggering that notifier on modification
 *"                should be of type boolean.
 */
void
toggle_changed_nt (gpointer cid, 
		   GmConfEntry *entry,
		   gpointer data)
{
  GtkWidget *e = NULL;
  gboolean current_value = FALSE;
  
  if (gm_conf_entry_get_type (entry) == GM_CONF_BOOL) {
   
    gdk_threads_enter ();
  
    e = GTK_WIDGET (data);

    /* We set the new value for the widget */
    current_value = gm_conf_entry_get_bool (entry);

    g_signal_handlers_block_matched (G_OBJECT (e),
				     G_SIGNAL_MATCH_FUNC,
				     0, 0, NULL,
				     (gpointer) toggle_changed,
				     NULL);
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (e)) != current_value)
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (e), current_value);
    g_signal_handlers_unblock_matched (G_OBJECT (e),
				       G_SIGNAL_MATCH_FUNC,
				       0, 0, NULL,
				       (gpointer) toggle_changed,
				       NULL);
    gdk_threads_leave (); 
  }
}

/* DESCRIPTION  :  This function is called when an adjustment changes.
 * BEHAVIOR     :  Updates the key given as parameter to the new value of the
 *                 adjustment.  
 * PRE          :  Non-Null data corresponding to the int config key to modify.
 */
void
adjustment_changed (GtkAdjustment *adj,
		    gpointer data)
{
  gchar *key = NULL;

  key = (gchar *) data;

  if (gm_conf_get_int (key) != (int) adj->value)
    gm_conf_set_int (key, (int) adj->value);
}


/* DESCRIPTION  :  Generic notifiers for adjustments.
 *                 This callback is called when a specific key of
 *                 the config database associated with an adjustment changes.
 * BEHAVIOR     :  It only updates the widget.
 * PRE          :  The config key triggering that notifier on modification
 *                 should be of type integer.
 */
void
adjustment_changed_nt (gpointer cid, 
		       GmConfEntry *entry,
		       gpointer data)
{
  GtkWidget *e = NULL;
  GtkAdjustment *s = NULL;
  gdouble current_value = 0.0;
  
  if (gm_conf_entry_get_type (entry) == GM_CONF_INT) {
    
    gdk_threads_enter ();

    e = GTK_WIDGET (data);
    s = gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON (e));
      
    current_value = gm_conf_entry_get_int (entry);

    g_signal_handlers_block_matched (G_OBJECT (e),
				     G_SIGNAL_MATCH_FUNC,
				     0, 0, NULL,
				     (gpointer) adjustment_changed,
				     NULL);
    if (gtk_adjustment_get_value (GTK_ADJUSTMENT (s)) != current_value)
      gtk_adjustment_set_value (GTK_ADJUSTMENT (s), current_value);
    g_signal_handlers_unblock_matched (G_OBJECT (e),
				       G_SIGNAL_MATCH_FUNC,
				       0, 0, NULL,
				       (gpointer) adjustment_changed,
				       NULL);
      
    gdk_threads_leave ();
  }
}

/* DESCRIPTION  :  This function is called when an int option menu changes.
 * BEHAVIOR     :  Updates the key given as parameter to the new value of the
 *                 int option menu.  
 * PRE          :  Non-Null data corresponding to int the config key to modify.
 */
void
int_option_menu_changed (GtkWidget *menu,
			 gpointer data)
{
  gchar *key = NULL;
  GtkWidget *active_item = NULL;
  guint item_index = -1;
  
  key = (gchar *) data;
  
  active_item = gtk_menu_get_active (GTK_MENU (menu));
  item_index = g_list_index (GTK_MENU_SHELL (GTK_MENU (menu))->children, 
			     active_item);

  if (gm_conf_get_int (key) != item_index)
    gm_conf_set_int (key, item_index);
}


/* DESCRIPTION  :  Generic notifiers for int-based option menus.
 *                 This callback is called when a specific key of
 *                 the config database associated with an option menu changes,
 *                 it only updates the menu.
 * BEHAVIOR     :  It only updates the widget.
 * PRE          :  The config key triggering that notifier on modifiction
 *                 should be of type integer.
 */
void
int_option_menu_changed_nt (gpointer cid, 
			    GmConfEntry *entry,
			    gpointer data)
{
  GtkWidget *e = NULL;
  gint current_value = 0;
  
  if (gm_conf_entry_get_type (entry) == GM_CONF_INT) {
   
    gdk_threads_enter ();

    e = GTK_WIDGET (data);
    current_value = gm_conf_entry_get_int (entry);

    g_signal_handlers_block_matched (G_OBJECT (e),
				     G_SIGNAL_MATCH_FUNC,
				     0, 0, NULL,
				     (gpointer) int_option_menu_changed,
				     NULL);
    if (current_value != gtk_option_menu_get_history (GTK_OPTION_MENU (e)))
	gtk_option_menu_set_history (GTK_OPTION_MENU (e), current_value);
    g_signal_handlers_block_matched (G_OBJECT (e),
				     G_SIGNAL_MATCH_FUNC,
				     0, 0, NULL,
				     (gpointer) int_option_menu_changed,
				     NULL);
    gdk_threads_leave ();
  }
}

/* DESCRIPTION  :  This function is called when a string option menu changes.
 * BEHAVIOR     :  Updates the key given as parameter to the new value of the
 *                 string option menu.  
 * PRE          :  Non-Null data corresponding to the string config key to
 *                 modify.
 */
void
string_option_menu_changed (GtkWidget *menu,
			    gpointer data)
{
  GtkWidget *active_item = NULL;
  const gchar *text = NULL;

  gchar *current_value = NULL;
  gchar *key = NULL;

  key = (gchar *) data;


  active_item = gtk_menu_get_active (GTK_MENU (menu));
  if (!active_item)
    text = "";
  else
    text = gtk_label_get_text (GTK_LABEL (GTK_BIN (active_item)->child));

  current_value = gm_conf_get_string (key);

  if (text && current_value && strcmp (text, current_value))
    gm_conf_set_string (key, text);
}


/* DESCRIPTION  :  Generic notifiers for string-based option_menus.
 *                 This callback is called when a specific key of
 *                 the config database associated with an option menu changes,
 *                 this only updates the menu.
 * BEHAVIOR     :  It only updates the widget.
 * PRE          :  The config key triggering that notifier on modifiction
 *                 should be of type string.
 */
void
string_option_menu_changed_nt (gpointer cid, 
			       GmConfEntry *entry,
			       gpointer data)
{
  int cpt = 0;
  GtkWidget *e = NULL;
  GtkWidget *label = NULL;
  GList *glist = NULL;
  gpointer mydata;

  if (gm_conf_entry_get_type (entry) == GM_CONF_STRING) {
   
    gdk_threads_enter ();

    e = GTK_WIDGET (data);
    
    /* We set the new value for the widget */
    glist = 
      g_list_first (GTK_MENU_SHELL (GTK_MENU (GTK_OPTION_MENU (e)->menu))->children);
    
    while ((mydata = g_list_nth_data (glist, cpt)) != NULL) {

      label = GTK_BIN (mydata)->child;
      if (label && !strcmp (gtk_label_get_text (GTK_LABEL (label)), 
			    gm_conf_entry_get_string (entry)))
	break;
      cpt++; 
    } 

    g_signal_handlers_block_matched (G_OBJECT (e),
				     G_SIGNAL_MATCH_FUNC,
				     0, 0, NULL,
				     (gpointer) string_option_menu_changed,
				     NULL);
    if (gtk_option_menu_get_history (GTK_OPTION_MENU (data)) != cpt)
      gtk_option_menu_set_history (GTK_OPTION_MENU (data), cpt);
    g_signal_handlers_unblock_matched (G_OBJECT (e),
				       G_SIGNAL_MATCH_FUNC,
				       0, 0, NULL,
				       (gpointer) string_option_menu_changed,
				       NULL);
	 
    gdk_threads_leave ();
  }
}
