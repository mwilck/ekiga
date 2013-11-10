
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2009 Damien Sandras <dsandras@seconix.com>
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
 *                         gm_conf.h  -  description 
 *                         ------------------------------------------
 *   begin                : Fri Oct 17 2003, but based on older code
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : helper notifiers for gnomemeeting's widgets
 *
 */

#include "gmconfwidgets.h"

#include <string.h>

/* GTK Callbacks */
/*
 * There are 2 callbacks, one modifying the config key, ie the GTK callback
 * and the config notifier (_nt) updating the widget back when a config key
 *  changes.
 *
 */


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

  if (gm_conf_get_int (key) != (int) gtk_adjustment_get_value (adj))
    gm_conf_set_int (key, (int) gtk_adjustment_get_value (adj));
}



void
adjustment_changed_nt (G_GNUC_UNUSED gpointer cid, 
		       GmConfEntry *entry,
		       gpointer data)
{
  GtkAdjustment *s = NULL;
  gdouble current_value = 0.0;

  if (gm_conf_entry_get_type (entry) == GM_CONF_INT) {

    s = GTK_ADJUSTMENT (data);

    current_value = gm_conf_entry_get_int (entry);

    g_signal_handlers_block_matched (G_OBJECT (s),
				     G_SIGNAL_MATCH_FUNC,
				     0, 0, NULL,
				     (gpointer) adjustment_changed,
				     NULL);
    if (gtk_adjustment_get_value (GTK_ADJUSTMENT (s)) > current_value
        || gtk_adjustment_get_value (GTK_ADJUSTMENT (s)) < current_value)
      gtk_adjustment_set_value (GTK_ADJUSTMENT (s), current_value);
    g_signal_handlers_unblock_matched (G_OBJECT (s),
				       G_SIGNAL_MATCH_FUNC,
				       0, 0, NULL,
				       (gpointer) adjustment_changed,
				       NULL);
  }
}


/* DESCRIPTION  :  This function is called when an int option menu changes.
 * BEHAVIOR     :  Updates the key given as parameter to the new value of the
 *                 int option menu.  
 * PRE          :  Non-Null data corresponding to int the config key to modify.
 */
void
int_option_menu_changed (GtkWidget *option_menu,
			 gpointer data)
{
  gchar *key = NULL;
  unsigned int current_value = 0;
  unsigned int i = 0;

  key = (gchar *) data;

  i = gtk_combo_box_get_active (GTK_COMBO_BOX (option_menu));
  current_value = gm_conf_get_int (key);

  if (i != current_value)
    gm_conf_set_int (key, i);
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
int_option_menu_changed_nt (G_GNUC_UNUSED gpointer cid, 
			    GmConfEntry *entry,
			    gpointer data)
{
  GtkWidget *e = NULL;
  gint current_value = 0;
  
  if (gm_conf_entry_get_type (entry) == GM_CONF_INT) {
   
    e = GTK_WIDGET (data);
    current_value = gm_conf_entry_get_int (entry);

    g_signal_handlers_block_matched (G_OBJECT (e),
				     G_SIGNAL_MATCH_FUNC,
				     0, 0, NULL,
				     (gpointer) int_option_menu_changed,
				     NULL);
    if (current_value != gtk_combo_box_get_active (GTK_COMBO_BOX (e)))
	gtk_combo_box_set_active (GTK_COMBO_BOX (e), current_value);
    g_signal_handlers_unblock_matched (G_OBJECT (e),
                                       G_SIGNAL_MATCH_FUNC,
                                       0, 0, NULL,
                                       (gpointer) int_option_menu_changed,
                                       NULL);
  }
}


/* DESCRIPTION  :  This function is called when a string option menu changes.
 * BEHAVIOR     :  Updates the key given as parameter to the new value of the
 *                 string option menu.  
 * PRE          :  Non-Null data corresponding to the string config key to
 *                 modify.
 */
void
string_option_menu_changed (GtkWidget *option_menu,
			    gpointer data)
{
  GtkTreeModel *model = NULL;
  GtkTreeIter iter;
  
  gchar *text = NULL;
  gchar *current_value = NULL;
  gchar *key = NULL;

  key = (gchar *) data;

  model = gtk_combo_box_get_model (GTK_COMBO_BOX (option_menu));
  if (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (option_menu), &iter)) {

    gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 0, &text, -1);
    current_value = gm_conf_get_string (key);

    if (text && current_value && g_strcmp0 (text, current_value))
      gm_conf_set_string (key, text);

    g_free (text);
  }
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
string_option_menu_changed_nt (G_GNUC_UNUSED gpointer cid,
			       GmConfEntry *entry,
			       gpointer data)
{
  int cpt = 0;
  int count = 0;

  GtkTreeModel *model = NULL;
  GtkTreeIter iter;

  GtkWidget *e = NULL;

  gchar *text = NULL;
  gchar* txt = NULL;

  if (gm_conf_entry_get_type (entry) == GM_CONF_STRING) {

    e = GTK_WIDGET (data);

    model = gtk_combo_box_get_model (GTK_COMBO_BOX (e));
    count = gtk_tree_model_iter_n_children (model, NULL);
    gtk_tree_model_get_iter_first (model, &iter);

    for (cpt = 0 ; cpt < count ; cpt++) {

      gtk_tree_model_get (model, &iter, 0, &text, -1);
      txt = gm_conf_entry_get_string (entry);
      if (text && !g_strcmp0 (text, txt)) {

        g_free (text);
        g_free (txt);
        break;
      }
      g_free (txt);
      gtk_tree_model_iter_next (model, &iter);

      g_free (text);
    }

    g_signal_handlers_block_matched (G_OBJECT (e),
				     G_SIGNAL_MATCH_FUNC,
				     0, 0, NULL,
				     (gpointer) string_option_menu_changed,
				     NULL);
    if (cpt != count && gtk_combo_box_get_active (GTK_COMBO_BOX (data)) != cpt)
      gtk_combo_box_set_active (GTK_COMBO_BOX (data), cpt);
    g_signal_handlers_unblock_matched (G_OBJECT (e),
				       G_SIGNAL_MATCH_FUNC,
				       0, 0, NULL,
				       (gpointer) string_option_menu_changed,
				       NULL);
  }
}
