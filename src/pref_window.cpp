
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
 *                         preferences.cpp  -  description
 *                         -------------------------------
 *   begin                : Tue Dec 26 2000
 *   copyright            : (C) 2000-2002 by Damien Sandras
 *   description          : This file contains all the functions needed to
 *                          create the preferences window and all its callbacks
 *   Additional code      : Miguel Rodríguez Pérez  <migrax@terra.es> 
 */


#include "../config.h"

#include "pref_window.h"
#include "videograbber.h"
#include "connection.h"
#include "config.h"
#include "gnomemeeting.h"
#include "common.h"
#include "ils.h"
#include "misc.h"

#include <gconf/gconf-client.h>
#ifndef DISABLE_GNOME
#include <gnome.h>
#endif


/* Declarations */

extern GtkWidget *gm;
extern GnomeMeeting *MyApp;	

static void pref_window_clicked_callback (GtkDialog *, int, gpointer);
static gint pref_window_destroy_callback (GtkWidget *, GdkEvent *, gpointer);
static void personal_data_update_button_clicked (GtkWidget *, gpointer);
static void gatekeeper_update_button_clicked (GtkWidget *, gpointer);
static void codecs_list_button_clicked_callback (GtkWidget *, gpointer);
static void gnomemeeting_codecs_list_add (GtkTreeIter, GtkListStore *, 
					  const gchar *, bool);

static void codecs_list_fixed_toggled (GtkCellRendererToggle *, gchar *, 
				       gpointer);
static void video_image_browse_clicked (GtkWidget *, gpointer);
static void file_selector_clicked (GtkFileSelection *, gpointer);


static void gnomemeeting_init_pref_window_general (GtkWidget *);
static void gnomemeeting_init_pref_window_interface (GtkWidget *);
static void gnomemeeting_init_pref_window_directories (GtkWidget *);
static void gnomemeeting_init_pref_window_video_devices (GtkWidget *);
static void gnomemeeting_init_pref_window_audio_devices (GtkWidget *);
static void gnomemeeting_init_pref_window_audio_codecs (GtkWidget *);
static void gnomemeeting_init_pref_window_video_codecs (GtkWidget *);


enum {
  
  COLUMN_ACTIVE,
  COLUMN_NAME,
  COLUMN_INFO,
  COLUMN_BANDWIDTH,
  COLUMN_NUMBER
};


/* GTK Callbacks */

/* DESCRIPTION  :  /
 * BEHAVIOR     :  Refreshes the devices list.
 * PRE          :  /
 */
static void refresh_devices (GtkWidget *widget, gpointer data)
{
  GmPrefWindow *pw = NULL;
  GmWindow *gw = NULL;
  char *gconf_string = NULL;
  GConfClient *client = NULL;
  GtkWidget *item = NULL;
  GtkWidget *menu = NULL;
  int i = 0;
  int index = 0;
  int cpt = 0;

  gw = gnomemeeting_get_main_window (gm);
  pw = gnomemeeting_get_pref_window (gm);
  client = gconf_client_get_default ();

  /* The player */
  gconf_string =  
    gconf_client_get_string (GCONF_CLIENT (client), 
			     "/apps/gnomemeeting/devices/audio_player", 
			     NULL);

  gw->audio_player_devices =
    PSoundChannel::GetDeviceNames (PSoundChannel::Player);

  i = gw->audio_player_devices.GetSize ();
  index = gw->audio_player_devices.GetValuesIndex(PString (gconf_string));

  gtk_option_menu_remove_menu (GTK_OPTION_MENU (pw->audio_player));
  menu = gtk_menu_new ();

  cpt = 0;
  while (cpt < i) {

    gchar *string = g_strdup (gw->audio_player_devices [cpt]);

    if (strcmp (string, "loopback")) {

      item = gtk_menu_item_new_with_label (string);
      gtk_widget_show (item);
      gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
    }

    cpt++;
    g_free (string);
  }

  if (index == P_MAX_INDEX)
    index = 0;

  gtk_option_menu_set_menu (GTK_OPTION_MENU (pw->audio_player), menu);
  gtk_option_menu_set_history (GTK_OPTION_MENU (pw->audio_player), index);

  g_signal_connect (G_OBJECT (GTK_OPTION_MENU (pw->audio_player)->menu), 
		    "deactivate", G_CALLBACK (string_option_menu_changed),
  		    (gpointer) "/apps/gnomemeeting/devices/audio_player");    


  /* The recorder */
  gconf_string =  
    gconf_client_get_string (GCONF_CLIENT (client), 
			     "/apps/gnomemeeting/devices/audio_recorder", 
			     NULL);

  gw->audio_recorder_devices =
    PSoundChannel::GetDeviceNames (PSoundChannel::Recorder);

  i = gw->audio_recorder_devices.GetSize ();
  index = gw->audio_recorder_devices.GetValuesIndex(PString (gconf_string));

  gtk_option_menu_remove_menu (GTK_OPTION_MENU (pw->audio_recorder));
  menu = gtk_menu_new ();

  cpt = 0;
  while (cpt < i) {

    gchar *string = g_strdup (gw->audio_recorder_devices [cpt]);

    if (strcmp (string, "loopback")) {

      item = gtk_menu_item_new_with_label (string);
      gtk_widget_show (item);
      gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
    }

    g_free (string);
    cpt++;
  }

  if (index == P_MAX_INDEX)
    index = 0;

  gtk_option_menu_set_menu (GTK_OPTION_MENU (pw->audio_recorder), menu);
  gtk_option_menu_set_history (GTK_OPTION_MENU (pw->audio_recorder), index);

  g_signal_connect (G_OBJECT (GTK_OPTION_MENU (pw->audio_recorder)->menu), 
		    "deactivate", G_CALLBACK (string_option_menu_changed),
  		    (gpointer) "/apps/gnomemeeting/devices/audio_recorder");


  /* The Video player */
  gconf_string =  
    gconf_client_get_string (GCONF_CLIENT (client), 
			     "/apps/gnomemeeting/devices/video_recorder", 
			     NULL);

  gw->video_devices = PVideoInputDevice::GetInputDeviceNames ();

  i = gw->video_devices.GetSize ();
  index = gw->video_devices.GetValuesIndex(PString (gconf_string));

  gtk_option_menu_remove_menu (GTK_OPTION_MENU (pw->video_device));
  menu = gtk_menu_new ();

  cpt = 0;
  while (cpt < i) {

    gchar *string = g_strdup (gw->video_devices [cpt]);
    item = gtk_menu_item_new_with_label (string);
    gtk_widget_show (item);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
    g_free (string);
    cpt++;
  }
  item = gtk_menu_item_new_with_label (_("Picture"));
  gtk_widget_show (item);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

  if (index == P_MAX_INDEX)
    index = 0;

  gtk_option_menu_set_menu (GTK_OPTION_MENU (pw->video_device), menu);
  gtk_option_menu_set_history (GTK_OPTION_MENU (pw->video_device), index);

  g_signal_connect (G_OBJECT (GTK_OPTION_MENU (pw->video_device)->menu), 
		    "deactivate", G_CALLBACK (string_option_menu_changed),
  		    (gpointer) "/apps/gnomemeeting/devices/video_recorder");
}


/* DESCRIPTION  :  This callback is called when the user clicks on "Close"
 * BEHAVIOR     :  Closes the window.
 * PRE          :  gpointer is a valid pointer to a GmPrefWindow.
 */
static void pref_window_clicked_callback (GtkDialog *widget, int button, 
					  gpointer data)
{
  switch (button) {

  case 0:
  
    if (widget)
      gtk_widget_hide (GTK_WIDGET (widget));
    
    break;
  }
}


/* DESCRIPTION  :  This callback is called when the pref window is deleted.
 * BEHAVIOR     :  Prevents the destroy, only hides the window.
 * PRE          :  /
 */
static gint pref_window_destroy_callback (GtkWidget *widget, GdkEvent *ev,
					  gpointer data)
{
  gtk_widget_hide (GTK_WIDGET (widget));
  return (TRUE);
}


/* DESCRIPTION  :  This callback is called when the user clicks
 *                 on the Update button of the Personal data Settings.
 * BEHAVIOR     :  Updates the values
 * PRE          :  /
 */
static void personal_data_update_button_clicked (GtkWidget *widget, 
						  gpointer data)
{
  GConfClient *client = gconf_client_get_default ();

  
  /* 1 */
  /* if registering is enabled for LDAP,
     modify the values */
  if (gconf_client_get_bool (GCONF_CLIENT (client), 
			     "/apps/gnomemeeting/ldap/register", 0)) {

    GMILSClient *ils_client = 
      (GMILSClient *) MyApp->Endpoint ()->GetILSClientThread ();
    ils_client->Modify ();
  }


  /* 2 */
  /* Set the local User name */
  MyApp->Endpoint ()->SetUserNameAndAlias ();

  /* Remove the current Gatekeeper */
  MyApp->Endpoint ()->RemoveGatekeeper(0);
    
  /* Register the current Endpoint to the Gatekeeper */
  MyApp->Endpoint ()->GatekeeperRegister ();
}


/* DESCRIPTION  :  This callback is called when the user clicks
 *                 on the Update button of the gatekeeper Settings.
 * BEHAVIOR     :  Updates the values
 * PRE          :  /
 */
static void gatekeeper_update_button_clicked (GtkWidget *widget, 
					      gpointer data)
{
  /* Remove the current Gatekeeper */
  MyApp->Endpoint ()->RemoveGatekeeper(0);
    
  /* Register the current Endpoint to the Gatekeeper */
  MyApp->Endpoint ()->GatekeeperRegister ();
}


/* DESCRIPTION  :  This callback is called when the user clicks
 *                 on a button in the Audio Codecs Settings 
 *                 (Up, Down)
 * BEHAVIOR     :  It updates the list order.
 * PRE          :  /
 */
static void codecs_list_button_clicked_callback (GtkWidget *widget, 
						 gpointer data)
{ 	
  GConfClient *client = NULL;
  GtkTreeIter iter;
  GtkTreeView *tree_view = NULL;
  GtkTreeSelection *selection = NULL;
  GSList *codecs_data = NULL;
  GSList *codecs_data_element = NULL;
  GSList *codecs_data_iter = NULL;
  gchar *selected_codec_name = NULL;
  gchar **couple;
  int codec_pos = 0;
  int operation = 0;

  client = gconf_client_get_default ();


  /* Get the current selected codec name, there is always one */
  tree_view = GTK_TREE_VIEW (g_object_get_data (G_OBJECT (data), "tree_view"));
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
  
  gtk_tree_selection_get_selected (GTK_TREE_SELECTION (selection), NULL,
				   &iter);
  gtk_tree_model_get (GTK_TREE_MODEL (data), &iter,
		      COLUMN_NAME, &selected_codec_name, -1);


  /* We set the selected codec name as data of the list store, to select 
     it again once the codecs list has been rebuilt */
  g_object_set_data (G_OBJECT (data), "selected_codec", 
		     (gpointer) selected_codec_name); 

  /* the gchar * must not be freed,
     it points to the internal
     element of the list_store */
			  
  /* Read all codecs, build the gconf data for the key, after having 
     set the selected codec one row above its current plance */
  codecs_data =
    gconf_client_get_list (client, 
			   "/apps/gnomemeeting/audio_codecs/codecs_list", 
			   GCONF_VALUE_STRING, NULL);

  codecs_data_iter = codecs_data;
  while (codecs_data_iter) {
    
    couple = g_strsplit ((gchar *) codecs_data_iter->data, "=", 0);

    if (couple [0]) {

      if (!strcmp (couple [0], selected_codec_name)) {

	g_strfreev (couple);

	break;
      }

      codec_pos++;
    }

    codecs_data_iter = codecs_data_iter->next;
  }

  
  if (!strcmp ((gchar *) g_object_get_data (G_OBJECT (widget), "operation"), 
	       "up"))
    operation = 1;


  /* The selected codec is at pos codec_pos, we will build the gconf key data,
     and set that codec one pos up or one pos down */
  if (((codec_pos == 0)&&(operation == 1))||
      ((codec_pos == GM_AUDIO_CODECS_NUMBER - 1)&&(operation == 0))) {

    g_slist_free (codecs_data);

    return;
  }

  
  if (operation == 1) {

    
    codecs_data_element = g_slist_nth (codecs_data, codec_pos);
    codecs_data = g_slist_remove_link (codecs_data, codecs_data_element);
    codecs_data = 
      g_slist_insert (codecs_data, (gchar *) codecs_data_element->data, 
		      codec_pos - 1);
    g_slist_free (codecs_data_element);
  }
  else {
    
    codecs_data_element = g_slist_nth (codecs_data, codec_pos);
    codecs_data = g_slist_remove_link (codecs_data, codecs_data_element);
    codecs_data = 
      g_slist_insert (codecs_data, (gchar *) codecs_data_element->data, 
		      codec_pos + 1);
    g_slist_free (codecs_data_element);    
  }


  gconf_client_set_list (client, 
			 "/apps/gnomemeeting/audio_codecs/codecs_list", 
			 GCONF_VALUE_STRING, codecs_data, NULL);

  
  g_slist_free (codecs_data);
}


static void
tree_selection_changed_cb (GtkTreeSelection *selection,
			   gpointer data)
{
  int page = 0;
  gchar *name = NULL;
  GtkTreeIter iter;
  GtkWidget *label;
  GtkTreeModel *model;
  GmWindow *gw = NULL;

  gw = gnomemeeting_get_main_window (gm);

  gtk_tree_selection_get_selected (selection, &model, &iter);
  
  gtk_tree_model_get (GTK_TREE_MODEL (model),
		      &iter, 1, &page, -1);
  gtk_tree_model_get (GTK_TREE_MODEL (model),
		      &iter, 0, &name, -1);

  label = 
    (GtkWidget *) g_object_get_data (G_OBJECT (gw->pref_window), 
				     "section_label");

  gtk_label_set_text (GTK_LABEL (label), name);

  gtk_notebook_set_current_page (GTK_NOTEBOOK (data), page);
}


/* DESCRIPTION  :  This callback is called when the user clicks
 *                 on a button of the file selector.
 * BEHAVIOR     :  It sets the selected filename in the video_image entry.
 * PRE          :  data = the file selector.
 */
static void  
file_selector_clicked (GtkFileSelection *selector, gpointer data) 
{
  GmPrefWindow *pw = NULL;
  gchar *filename = NULL;

  pw = gnomemeeting_get_pref_window (gm);
  filename = (gchar *)
    gtk_file_selection_get_filename (GTK_FILE_SELECTION (data));
  
  gtk_entry_set_text (GTK_ENTRY (pw->video_image), filename);
}


/* DESCRIPTION  :  This callback is called when the user clicks
 *                 on the browse button.
 * BEHAVIOR     :  It displays the file selector widget.
 * PRE          :  /
 */
static void
video_image_browse_clicked (GtkWidget *b, gpointer data)
{
  GtkWidget *selector = NULL;

  selector = gtk_file_selection_new (_("Please choose the video image"));

  gtk_widget_show (selector);

  g_signal_connect (G_OBJECT (GTK_FILE_SELECTION (selector)->ok_button),
		    "clicked",
		    G_CALLBACK (file_selector_clicked),
		    (gpointer) selector);
     
  /* Ensure that the dialog box is destroyed when the user clicks a button. */
  g_signal_connect_swapped (G_OBJECT (GTK_FILE_SELECTION (selector)->ok_button),
			    "clicked",
			    G_CALLBACK (gtk_widget_destroy),
			    (gpointer) selector);

  g_signal_connect_swapped (G_OBJECT (GTK_FILE_SELECTION (selector)->cancel_button),
			    "clicked",
			    G_CALLBACK (gtk_widget_destroy),
			    (gpointer) selector);
}


static void 
notebook_toggle_changed (GtkCheckButton *but, gpointer data)
{
  GConfClient *client = gconf_client_get_default ();
  gchar *key = (gchar *) data;

  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (but)))
    gconf_client_set_int (GCONF_CLIENT (client),
			  key, 0, NULL);
  else
    gconf_client_set_int (GCONF_CLIENT (client),
			  key, 3, NULL);
}


static void 
gnomemeeting_codecs_list_add (GtkTreeIter iter, GtkListStore *store, 
			      const gchar *codec_name, bool enabled)
{
  gchar *data [3];

  data [0] = g_strdup (codec_name);
  data [1] = NULL;
  data [2] = NULL;

  if (!strcmp (codec_name, "LPC10")) {
    data [1] = g_strdup (_("Okay"));
    data [2] = g_strdup ("3.46 kbits");
  }

#ifdef SPEEX_CODEC
  if (!strcmp (codec_name, "Speex-15k")) {
    data [1] = g_strdup (_("Excellent"));
    data [2] = g_strdup ("15 kbits");
  }

  if (!strcmp (codec_name, "Speex-8k")) {
    data [1] = g_strdup (_("Good Quality"));
    data [2] = g_strdup ("8 kbits");
  }
#endif

  if (!strcmp (codec_name, "MS-GSM")) {
    data [1] = g_strdup (_("Good Quality"));
    data [2] = g_strdup ("13 kbits");
  }

  if (!strcmp (codec_name, "G.711-ALaw-64k")) {
    data [1] = g_strdup (_("Excellent"));
    data [2] = g_strdup ("64 kbits");
  }

  if (!strcmp (codec_name, "G.711-uLaw-64k")) {
    data [1] = g_strdup (_("Excellent"));
    data [2] = g_strdup ("64 kbits");
  }

  if (!strcmp (codec_name, "GSM-06.10")) {
    data [1] = g_strdup (_("Good Quality"));
    data [2] = g_strdup ("16.5 kbits");
  }

  if (!strcmp (codec_name, "G.726-32k")) {
    data [1] = g_strdup (_("Good Quality"));
    data [2] = g_strdup ("32 kbits");
  }

  if (data [1] && data [2]) {
 
    gtk_list_store_append (store, &iter);
    gtk_list_store_set (store, &iter,
			COLUMN_ACTIVE, enabled,
			COLUMN_NAME, data [0],
			COLUMN_INFO, data [1],
			COLUMN_BANDWIDTH, data [2],
			-1);
  }

  g_free (data [0]);
  g_free (data [1]);
  g_free (data [2]);
}


static void
codecs_list_fixed_toggled (GtkCellRendererToggle *cell, gchar *path_str, gpointer data)
{
  GtkTreeModel *model = (GtkTreeModel *) data;
  GtkTreePath *path = gtk_tree_path_new_from_string (path_str);
  GtkTreeIter iter;
  gchar *codec_new = NULL, **couple;
  GSList *codecs_data = NULL, *codecs_data_iter = NULL;
  GSList *codecs_data_element = NULL;
  GConfClient *client = NULL;
  gboolean fixed;
  gchar *selected_codec_name = NULL;
  int current_row = 0;

  client = gconf_client_get_default ();

  /* get toggled iter */
  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_model_get (model, &iter, COLUMN_ACTIVE, &fixed, -1);
  gtk_tree_model_get (model, &iter, COLUMN_NAME, &selected_codec_name, -1);
  fixed ^= 1;
  gtk_tree_path_free (path);

  /* We set the selected codec name as data of the list store, 
     to select it again once the codecs list has been rebuilt */
  g_object_set_data (G_OBJECT (data), "selected_codec", 
		     (gpointer) selected_codec_name); 
  /* Stores a copy of the pointer,
     the gchar * must not be freed,
     as it points to the list_store
     element */

  /* Read all codecs, build the gconf data for the key, 
     after having set the selected codec
     one row above its current plance */
  codecs_data = 
    gconf_client_get_list (client, 
			   "/apps/gnomemeeting/audio_codecs/codecs_list", 
			   GCONF_VALUE_STRING, NULL);

  /* We are reading the codecs */
  codecs_data_iter = codecs_data;
  while (codecs_data_iter) {
    
    couple = g_strsplit ((gchar *) codecs_data_iter->data, "=", 0);

    if (couple [0]) {

      if (!strcmp (couple [0], selected_codec_name)) {

	gchar *v = g_strdup_printf ("%d", (int) fixed);
	codec_new = g_strconcat (couple [0], "=", v,  NULL);
	g_free (v);
	g_strfreev (couple);

	break;
      }

      current_row++;
    }

    g_strfreev (couple);

    codecs_data_iter = codecs_data_iter->next;
  }  


  /* Rebuilt the gconf_key with the update values */
  codecs_data_element = g_slist_nth (codecs_data, current_row); 
  codecs_data = g_slist_remove_link (codecs_data, codecs_data_element);
  codecs_data = g_slist_insert (codecs_data, codec_new, current_row);
  
  g_slist_free (codecs_data_element);
  
  gconf_client_set_list (client, "/apps/gnomemeeting/audio_codecs/codecs_list",
			 GCONF_VALUE_STRING, codecs_data, NULL);

  g_slist_free (codecs_data);
  g_free (codec_new);
}


/* Misc functions */
void gnomemeeting_codecs_list_build (GtkListStore *codecs_list_store) 
{
  GtkTreeView *tree_view = NULL;
  GtkTreeSelection *selection = NULL;
  GtkTreePath *tree_path = NULL;
  GtkTreeIter list_iter;
  int selected_row = 0;
  int current_row = 0;
  gchar *cselect_row;
  gchar *selected_codec = NULL;

  GSList *codecs_data = NULL;
  GConfClient *client = gconf_client_get_default ();

  codecs_data = 
    gconf_client_get_list (client, 
			   "/apps/gnomemeeting/audio_codecs/codecs_list", 
			   GCONF_VALUE_STRING, NULL);

  selected_codec = (gchar *) g_object_get_data (G_OBJECT (codecs_list_store), 
						"selected_codec");

  gtk_list_store_clear (GTK_LIST_STORE (codecs_list_store));

  /* We are adding the codecs */
  while (codecs_data) {

    gchar **couple = g_strsplit ((gchar *) codecs_data->data, "=", 0);

    if ((couple [0]) && (couple [1]))
      gnomemeeting_codecs_list_add (list_iter, codecs_list_store, 
				    couple [0], atoi (couple [1]));     

    if ((selected_codec) && (!strcmp (selected_codec, couple [0]))) 
      selected_row = current_row;

    g_strfreev (couple);
    codecs_data = codecs_data->next;
    current_row++;
  }

      
  cselect_row = g_strdup_printf("%d", selected_row);
  tree_path = gtk_tree_path_new_from_string (cselect_row);
  tree_view = GTK_TREE_VIEW (g_object_get_data (G_OBJECT (codecs_list_store), 
						"tree_view"));
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
  
  gtk_tree_selection_select_path (GTK_TREE_SELECTION (selection), tree_path);
  
  g_free (cselect_row);
  g_slist_free (codecs_data);

  gtk_tree_path_free (tree_path);
}
                                                                  
                                                                               
static GtkWidget *
gnomemeeting_pref_window_add_update_button (GtkWidget *table,
					    GtkSignalFunc func,
					    gchar *tooltip,  
					    int row, int col)
{
  GtkWidget *button = NULL;                                                    

  GmPrefWindow *pw = NULL;                                           
                                                                               
  pw = gnomemeeting_get_pref_window (gm);                                      
                                                                               

  button = gtk_button_new_from_stock (GTK_STOCK_APPLY);
                                                                               
  gtk_table_attach (GTK_TABLE (table),  button, col, col+1, row, row+1,        
                    (GtkAttachOptions) (GTK_EXPAND),                           
                    (GtkAttachOptions) (GTK_EXPAND),                           
                    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);           
                                                                               
  g_signal_connect (G_OBJECT (button), "clicked",                          
		    G_CALLBACK (func), (gpointer) pw);

  
  gtk_tooltips_set_tip (pw->tips, button, tooltip, NULL);

                                                
  return button;                                                               
}                                                                              
                                                                               
                                                                               
/* BEHAVIOR     :  It builds the notebook page for general settings and        
 *                 add it to the notebook.                                     
 * PRE          :  The notebook.                                               
 */                                                                            
void gnomemeeting_init_pref_window_general (GtkWidget *notebook)               
{                                                                              
  GtkWidget *vbox = NULL;                                                      
  GtkWidget *table = NULL;                                                     
                                                                               
  /* Get the data */                                                           
  GmPrefWindow *pw = gnomemeeting_get_pref_window (gm);              
                                                                               
                                                                               
  /* Packing widgets */                                                        
  vbox = gtk_vbox_new (FALSE, 4);
  table = gnomemeeting_vbox_add_table (vbox, _("Personal Information"),
				       5, 3);                           

                                                                               
  /* Add all the fields */                                                     
  pw->firstname = 
    gnomemeeting_table_add_entry (table, _("First Name:"), "/apps/gnomemeeting/personal_data/firstname", _("Enter your first name."), 0);

  pw->surname = 
    gnomemeeting_table_add_entry (table, _("Last Name:"), "/apps/gnomemeeting/personal_data/lastname", _("Enter your last name."), 1);
                                                                               
  pw->mail = gnomemeeting_table_add_entry (table, _("E-mail Address:"), "/apps/gnomemeeting/personal_data/mail", _("Enter your e-mail address."), 2);
                                                                               
  pw->comment = gnomemeeting_table_add_entry (table, _("Comment:"), "/apps/gnomemeeting/personal_data/comment", _("Here you can fill in a comment about yourself for ILS directories."), 3);
                                                                               
  pw->location = gnomemeeting_table_add_entry (table, _("Location:"), "/apps/gnomemeeting/personal_data/location", _("Where do you call from?"), 4);
                                                                               
                                                                               
  /* Add the try button */                                                     
  pw->directory_update_button =                                                
    gnomemeeting_pref_window_add_update_button (table, GTK_SIGNAL_FUNC (personal_data_update_button_clicked), _("Click here to update the LDAP server you are registered to with the new First Name, Last Name, E-Mail, Comment and Location or to update your alias on the Gatekeeper."), 5, 2);
  gtk_container_set_border_width (GTK_CONTAINER (pw->directory_update_button),
				  GNOMEMEETING_PAD_SMALL*2);

  gtk_notebook_append_page (GTK_NOTEBOOK(notebook), vbox, NULL);
}                                                                              
                                                                               

/* BEHAVIOR     :  It builds the notebook page for interface settings          
 *                 add it to the notebook, default values are set from the     
 *                 options struct given as parameter.                          
 * PRE          :  See init_pref_audio_codecs.                                 
 */                                                                            
static void gnomemeeting_init_pref_window_interface (GtkWidget *notebook)      
{                                                                              
  GtkWidget *vbox = NULL;                                                      
  GtkWidget *table = NULL;                                                     
                                                                               
  /* Get the data */                                                           
  GmPrefWindow *pw = gnomemeeting_get_pref_window (gm);              
                                                                               
                                                                               
  /* Packing widgets */                                                        
  vbox = gtk_vbox_new (FALSE, 4);
  gtk_notebook_append_page (GTK_NOTEBOOK(notebook), vbox, NULL);    
  table = gnomemeeting_vbox_add_table (vbox,
					      _("GnomeMeeting GUI"), 2, 2);

  pw->show_splash = gnomemeeting_table_add_toggle (table, _("Show Splash Screen"), "/apps/gnomemeeting/view/show_splash", _("If enabled, the splash screen will be displayed at startup time."), 0, 1);
                                                                                          
  pw->start_hidden = gnomemeeting_table_add_toggle (table, _("Start Hidden"), "/apps/gnomemeeting/view/start_docked", _("If enabled, GnomeMeeting will start hidden. The docklet must be enabled."), 1, 1);

                                                                               
  /* Packing widget */                                                         
  table = gnomemeeting_vbox_add_table (vbox, _("Behavior"), 3, 1);      
                                                                               
                                                                               
  /* The toggles */
  pw->aa = gnomemeeting_table_add_toggle (table, _("Auto Answer"), "/apps/gnomemeeting/general/auto_answer", _("If enabled, incoming calls will be automatically answered."), 1, 0);
                                                                               
  pw->dnd = gnomemeeting_table_add_toggle (table, _("Do Not Disturb"), "/apps/gnomemeeting/general/do_not_disturb", _("If enabled, incoming calls will be automatically refused."), 0, 0);
                                                                               
  pw->incoming_call_popup = gnomemeeting_table_add_toggle (table, _("Popup window"), "/apps/gnomemeeting/view/show_popup", _("If enabled, a popup will be displayed when receiving an incoming call"), 2, 0);



  /* Packing widget */                                                         
  table = gnomemeeting_vbox_add_table (vbox, _("Video Display"), 
					      3, 1);  
                                                                               
#ifdef HAS_SDL  
  pw->fullscreen_width =
    gnomemeeting_table_add_spin (table, _("Fullscreen Width:"),       
				       "/apps/gnomemeeting/video_display/fullscreen_width",
				       _("The image width for fullscreeen."),
				       10.0, 640.0, 10.0, 0);

  pw->fullscreen_height =
    gnomemeeting_table_add_spin (table, _("Fullscreen Height:"),       
				       "/apps/gnomemeeting/video_display/fullscreen_height",
				       _("The image height for fullscreeen."),
				       10.0, 480.0, 10.0, 1);
#endif

  pw->bilinear_filtering =
    gnomemeeting_table_add_toggle (table, _("Enable bilinear filtering"), "/apps/gnomemeeting/video_display/bilinear_filtering", _("Enable or disable bilinear interpolation when rendering video images (it has no effect on fullscreen)."), 3, 0);


  /* Packing widget */                                                         
  table = gnomemeeting_vbox_add_table (vbox, _("Sound"),                
                                              1, 1);                           
                                                                               
  /* The toggles */                                                            
  pw->incoming_call_sound = gnomemeeting_table_add_toggle (table, _("Incoming Call"), "/apps/gnomemeeting/general/incoming_call_sound", _("If enabled, GnomeMeeting will play a sound when receiving an incoming call (the sound to play is chosen in the Gnome Control Center)."), 0, 0);
}


/* BEHAVIOR     :  It builds the notebook page for XDAP directories and gk,
 *                 it adds it to the notebook.                                
 * PRE          :  The notebook.                                               
 */                                                                            
void gnomemeeting_init_pref_window_directories (GtkWidget *notebook)        
{                                                                              
  GtkWidget *vbox = NULL;                                                      
  GtkWidget *table = NULL;                                                     
  GtkWidget *button = NULL;
  gchar *options [] = {_("Do not register"), 
		       _("Gatekeeper host"), 
		       _("Gatekeeper ID"), 
		       _("Automatic Discover"), 
		       NULL};
                                                                               
  /* Get the data */                                                           
  GmPrefWindow *pw = gnomemeeting_get_pref_window (gm);              
                                                                               
                                                                               
  /* Packing widgets for the XDAP directory */                            
  vbox = gtk_vbox_new (FALSE, 4);
  gtk_notebook_append_page (GTK_NOTEBOOK(notebook), vbox, NULL);    
  table = gnomemeeting_vbox_add_table (vbox, _("XDAP Directory"),
                                              2, 1);                           
                                                                               
  /* Add all the fields */                                                     
  pw->ldap_server = 
    gnomemeeting_table_add_entry (table, _("XDAP Directory:"), "/apps/gnomemeeting/ldap/ldap_server", _("The XDAP server to register to."), 0);

  pw->ldap =
    gnomemeeting_table_add_toggle (table, _("Enable Registering"), "/apps/gnomemeeting/ldap/register", _("If enabled, permit to register to the selected LDAP directory"), 1, 0);


  /* Add fields for the gatekeeper */
  table = gnomemeeting_vbox_add_table (vbox, _("Gatekeeper"), 5, 3);

  pw->gk_id = 
    gnomemeeting_table_add_entry (table, _("Gatekeeper ID:"), "/apps/gnomemeeting/gatekeeper/gk_id", _("The Gatekeeper identifier to register to."), 1);

  pw->gk_host = 
    gnomemeeting_table_add_entry (table, _("Gatekeeper host:"), "/apps/gnomemeeting/gatekeeper/gk_host", _("The Gatekeeper host to register to."), 2);

  pw->gk_alias = 
    gnomemeeting_table_add_entry (table, _("Gatekeeper alias: "), "/apps/gnomemeeting/gatekeeper/gk_alias", _("The Gatekeeper Alias to use when registering (string, or E164 ID if only 0123456789#)."), 3);

  pw->gk_password = 
    gnomemeeting_table_add_entry (table, _("Gatekeeper password:"), "/apps/gnomemeeting/gatekeeper/gk_password", _("The Gatekeeper password to use for H.235 authentication to the Gatekeeper."), 4);
  gtk_entry_set_visibility (GTK_ENTRY (pw->gk_password), FALSE);

  pw->gk = 
    gnomemeeting_table_add_int_option_menu (table, _("Registering method:"), options, "/apps/gnomemeeting/gatekeeper/registering_method", _("Registering method to use"), 0);

  button =
    gnomemeeting_pref_window_add_update_button (table, GTK_SIGNAL_FUNC (gatekeeper_update_button_clicked), _("Click here to update your Gatekeeper settings."), 5, 2);
}                                                                              


/* BEHAVIOR     :  It builds the notebook page for H.323 advanced settings and        
 *                 add it to the notebook.                                     
 * PRE          :  The notebook.                                               
 */                                                                            
void gnomemeeting_init_pref_window_h323_advanced (GtkWidget *notebook)               
{                                                                              
  GtkWidget *vbox = NULL;                                                      
  GtkWidget *table = NULL;                                                     
                                                                               
  /* Get the data */                                                           
  GmPrefWindow *pw = gnomemeeting_get_pref_window (gm);              
                                                                               
                                                                               
  /* Packing widgets */                                                        
  vbox = gtk_vbox_new (FALSE, 4);
  table = gnomemeeting_vbox_add_table (vbox, _("H.323 Call Forwarding"),
                                              4, 2);                           

                                                                               
  /* Add all the fields */                                                     
  pw->forward_host = 
    gnomemeeting_table_add_entry (table, _("Forward calls to host:"), "/apps/gnomemeeting/call_forwarding/forward_host", _("Enter here the host where calls should be forwarded in the cases selected above"), 3);

  pw->always_forward =
    gnomemeeting_table_add_toggle (table, _("Always forward calls to the given host"), "/apps/gnomemeeting/call_forwarding/always_forward", _("If enabled, all incoming calls will always be forwarded to the host that is specified in the field below."), 0, 0);

  pw->no_answer_forward =
    gnomemeeting_table_add_toggle (table, _("Forward calls to the given host if no answer"), "/apps/gnomemeeting/call_forwarding/no_answer_forward", _("If enabled, all incoming calls will be forwarded to the host that is specified in the field below if you don't answer the call."), 1, 0);

  pw->busy_forward =
    gnomemeeting_table_add_toggle (table, _("Forward calls to the given host if busy"), "/apps/gnomemeeting/call_forwarding/busy_forward", _("If enabled, all incoming calls will be forwarded to the host that is specified in the field below if you already are in a call or if you are in Do Not Disturb mode."), 2, 0);


  /* Packing widget */                                                         
  table = gnomemeeting_vbox_add_table (vbox, 
					      _("H.323 V2 Settings"),
                                              2, 1);                           

                                                                               
  /* The toggles */                                                            
  pw->ht = 
    gnomemeeting_table_add_toggle (table, _("Enable H.245 Tunnelling"), "/apps/gnomemeeting/general/h245_tunneling", _("This enables H.245 Tunnelling mode. In H.245 Tunnelling mode H.245 messages are encapsulated into the the H.225 channel (port 1720). This permits sparing one random TCP port during calls. H.245 Tunnelling was introduced in H.323v2 and Netmeeting doesn't support it. Using both Fast Start and H.245 Tunnelling can crash some versions of Netmeeting."), 0, 0);
                                                                               
  pw->fs = 
    gnomemeeting_table_add_toggle (table, _("Enable Fast Start"), "/apps/gnomemeeting/general/fast_start", _("Connection will be established in Fast Start mode. Fast Start is a new way to start calls faster that was introduced in H.323v2. It is not supported by Netmeeting and using both Fast Start and H.245 Tunnelling can crash some versions of Netmeeting."), 1, 0);


  /* IP translation */
  table = gnomemeeting_vbox_add_table (vbox, 
					      _("NAT/PAT Router Support"),
                                              2, 1);     

  pw->ip_translation = 
    gnomemeeting_table_add_toggle (table, _("Enable IP Translation"), "/apps/gnomemeeting/general/ip_translation", _("This enables IP translation. IP translation is useful if GnomeMeeting is running behind a NAT/PAT router. You have to put the public IP of the router in the field below. If you are registered to ils.seconix.com, GnomeMeeting will automatically fetch the public IP using the ILS service. If your router natively supports H.323, you can disable this."), 1, 0);

  pw->public_ip = 
    gnomemeeting_table_add_entry (table, _("Public IP of the NAT/PAT router:"), "/apps/gnomemeeting/general/public_ip", _("You can put here the public IP of your NAT/PAT router if you want to use IP translation. If you are registered to ils.seconix.com, GnomeMeeting will automatically fetch the public IP using the ILS service."), 2);

  gtk_notebook_append_page (GTK_NOTEBOOK(notebook), vbox, NULL);
}                                                                              


/* BEHAVIOR     :  It builds the notebook page for the audio devices 
 *                 settings and add it to the notebook.
 * PRE          :  See init_pref_audio_codecs.
 */
static void gnomemeeting_init_pref_window_audio_devices (GtkWidget *notebook)
{
  GConfClient *client = NULL;
  GtkWidget *vbox = NULL;                                                      
  GtkWidget *table = NULL; 

  GtkWidget *button = NULL;
  GtkWidget *table2 = NULL;

  int i = 0;

  gchar *aec [] = {_("Off"),
		   _("Low"),
		   _("Medium"),
		   _("High"),
		   _("Automatic Gain Compensation"),
		   NULL};

  gchar *audio_player_devices_list [20];
  gchar *audio_recorder_devices_list [20];
                

  /* Get the data */                                             
  GmWindow *gw = gnomemeeting_get_main_window (gm);              
  GmPrefWindow *pw = gnomemeeting_get_pref_window (gm);
  
  client = gconf_client_get_default ();
                                                                               
  /* Packing widgets for the XDAP directory */                                
  vbox = gtk_vbox_new (FALSE, 4);
  gtk_notebook_append_page (GTK_NOTEBOOK(notebook), vbox, NULL);    
  table = gnomemeeting_vbox_add_table (vbox, _("Audio Devices"),
                                              4, 2);                           
                                                                               
  /* Add all the fields */                 
  /* The player */
  i = gw->audio_player_devices.GetSize () - 1;
  if (i >= 20) i = 19;

  for (int j = i ; j >= 0; j--) 
    if (strcmp (gw->audio_player_devices [j], "loopback"))
      audio_player_devices_list [j] = g_strdup (gw->audio_player_devices [j]);
    else
      audio_player_devices_list [j] = NULL;

  
  audio_player_devices_list [i+1] = NULL;

  pw->audio_player = 
    gnomemeeting_table_add_string_option_menu (table, _("Audio Player:"), audio_player_devices_list, "/apps/gnomemeeting/devices/audio_player", _("Enter the audio player device to use."), 0);

  for (int j = i ; j >= 0; j--) 
    g_free (audio_player_devices_list [j]);

  
  /* The recorder */
  i = gw->audio_recorder_devices.GetSize () - 1;
  if (i >= 20) i = 19;

  for (int j = i ; j >= 0; j--) 
    if (strcmp (gw->audio_recorder_devices [j], "loopback"))
      audio_recorder_devices_list [j] = 
	g_strdup (gw->audio_recorder_devices [j]);
    else
      audio_recorder_devices_list [j] = NULL;
  
  audio_recorder_devices_list [i + 1] = NULL;

  pw->audio_recorder = 
    gnomemeeting_table_add_string_option_menu (table, _("Audio Recorder:"), audio_recorder_devices_list, "/apps/gnomemeeting/devices/audio_recorder", _("Enter the audio recorder device to use."), 2);

  for (int j = i ; j >= 0; j--) 
    g_free (audio_recorder_devices_list [j]);


#ifdef HAS_IXJ
  /* The Quicknet devices related options */
  table = gnomemeeting_vbox_add_table (vbox, _("Quicknet Device"), 
					      3, 2);

  pw->lid_aec =
    gnomemeeting_table_add_int_option_menu (table, _("Automatic Echo Cancellation:"), aec, "/apps/gnomemeeting/devices/lid_aec", _("The Automatic Echo Cancellation level: Off, Low, Medium, High, Automatic Gain Compensation. Choosing Automatic Gain Compensation modulates the volume for best quality."), 0);

  pw->lid_country =
    gnomemeeting_table_add_entry (table, _("Country Code:"), "/apps/gnomemeeting/devices/lid_country", _("The two-letter country code of your country (e.g.: BE, UK, FR, DE, ...)."), 1);

  pw->lid =
    gnomemeeting_table_add_toggle (table, _("Use the Quicknet Device"), "/apps/gnomemeeting/devices/lid", _("If enabled, GnomeMeeting will use the Quicknet device instead of the regular soundcard during calls."), 2, 0);
#endif


  /* That button will refresh the devices list */
  button = gtk_button_new_from_stock (GTK_STOCK_REFRESH);
  table2 = gtk_table_new (1, 6, TRUE);
  gtk_box_pack_start (GTK_BOX (vbox), table2, FALSE, FALSE, 0);
  gtk_table_attach (GTK_TABLE (table2), button, 5, 6, 0, 1,         
                    (GtkAttachOptions) (GTK_SHRINK | GTK_FILL),                
                    (GtkAttachOptions) (GTK_SHRINK | GTK_FILL),                
                    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);  

  g_signal_connect (G_OBJECT (button), "clicked",
		    G_CALLBACK (refresh_devices), NULL);
}


/* BEHAVIOR     :  It builds the notebook page for the video devices 
 *                 settings and add it to the notebook.
 * PRE          :  See init_pref_audio_codecs.
 */
static void gnomemeeting_init_pref_window_video_devices (GtkWidget *notebook)
{
  GConfClient *client = NULL;
  gchar *gconf_string = NULL;
  GtkWidget *vbox = NULL;                                                      
  GtkWidget *table = NULL; 

  GtkWidget *button = NULL;
  GtkWidget *table2 = NULL;

  int i = 0;
  gchar *video_size [] = {_("Small"), 
			  _("Large"), 
			  NULL};
  gchar *video_format [] = {_("PAL"), 
			    _("NTSC"), 
			    _("SECAM"), 
			    _("auto"), 
			    NULL};

  gchar *video_devices_list [20];
                

  /* Get the data */                                             
  GmWindow *gw = gnomemeeting_get_main_window (gm);              
  GmPrefWindow *pw = gnomemeeting_get_pref_window (gm);
  
  client = gconf_client_get_default ();
                                                                               
  /* Packing widgets for the XDAP directory */                                
  vbox = gtk_vbox_new (FALSE, 4);
  gtk_notebook_append_page (GTK_NOTEBOOK(notebook), vbox, NULL);    


  /* The video devices related options */
  table = gnomemeeting_vbox_add_table (vbox, _("Video Devices"), 6, 3);

  /* The video device */
  gconf_string =  gconf_client_get_string (GCONF_CLIENT (client), "/apps/gnomemeeting/devices/video_recorder", NULL);

  i = gw->video_devices.GetSize () - 1;
  if (i >= 20) i = 19;

  for (int j = i ; j >= 0; j--) 
    video_devices_list [j] = 
      g_strdup (gw->video_devices [j]);
  video_devices_list [i + 1] = g_strdup (_("Picture"));
  video_devices_list [i + 2] = NULL;

  pw->video_device = 
    gnomemeeting_table_add_string_option_menu (table, _("Video Device:"), video_devices_list, "/apps/gnomemeeting/devices/video_recorder", _("Enter the video device to use. Using an invalid video device for video transmission will transmit a test picture."), 1);

  for (int j = i + 1 ; j >= 0; j--) 
    g_free (video_devices_list [j]);
  g_free (gconf_string);


  /* Video Channel */
  pw->video_channel =
    gnomemeeting_table_add_spin (table, _("Video Channel:"),       
				       "/apps/gnomemeeting/devices/video_channel",       
				       _("The video channel number to use (camera, tv, ...)."),
				       0.0, 10.0, 1.0, 2);
  
  pw->opt1 =
    gnomemeeting_table_add_int_option_menu (table, _("Video Size:"), video_size, "/apps/gnomemeeting/devices/video_size", _("Choose the transmitted video size: QCIF (small) or CIF (large)."), 3);

  pw->opt2 =
    gnomemeeting_table_add_int_option_menu (table, _("Video Format:"), video_format, "/apps/gnomemeeting/devices/video_format", _("Here you can choose the transmitted video format."), 4);


  pw->video_image =
    gnomemeeting_table_add_entry (table, _("Video Image:"), "/apps/gnomemeeting/devices/video_image", _("The image to transmit if the Picture device is selected as video device or if the opening of the device fails (none specified = default GnomeMeeting logo)."), 5);


  /* The file selector button */
  button = gtk_button_new_with_label (_("Browse"));
  gtk_table_attach (GTK_TABLE (table), button, 2, 3, 5, 6,         
                    (GtkAttachOptions) (GTK_SHRINK | GTK_FILL),                
                    (GtkAttachOptions) (GTK_SHRINK | GTK_FILL),                
                    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);  

  g_signal_connect (G_OBJECT (button), "clicked",
		    G_CALLBACK (video_image_browse_clicked),
		    NULL);


  /* That button will refresh the devices list */
  button = gtk_button_new_from_stock (GTK_STOCK_REFRESH);
  table2 = gtk_table_new (1, 6, TRUE);
  gtk_box_pack_start (GTK_BOX (vbox), table2, FALSE, FALSE, 0);
  gtk_table_attach (GTK_TABLE (table2), button, 5, 6, 0, 1,         
                    (GtkAttachOptions) (GTK_SHRINK | GTK_FILL),                
                    (GtkAttachOptions) (GTK_SHRINK | GTK_FILL),                
                    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  
  g_signal_connect (G_OBJECT (button), "clicked",
		    G_CALLBACK (refresh_devices), NULL);
}


/* BEHAVIOR     :  It builds the notebook page for audio codecs settings and  
 *                 add it to the notebook.                                     
 * PRE          :  The notebook.                                               
 */                                                                            
void gnomemeeting_init_pref_window_audio_codecs (GtkWidget *notebook)         
{                                                                              
  GtkWidget *vbox = NULL;                                                      
  GtkWidget *table = NULL;                                                     
  GtkWidget *button1 = NULL;
  GtkWidget *button2 = NULL;
  GtkWidget *frame = NULL;

  int width = 80;
  GtkRequisition size_request1, size_request2;

  /* For the GTK TreeView */
  GtkWidget *tree_view;
  GtkTreeViewColumn *column;
  GtkCellRenderer *renderer;                        
                                                       

  /* Get the data */                                                           
  GmPrefWindow *pw = gnomemeeting_get_pref_window (gm);


  /* Packing widgets */                                                        
  vbox =  gtk_vbox_new (FALSE, 4);
  gtk_notebook_append_page (GTK_NOTEBOOK(notebook), vbox, NULL);  
  table = gnomemeeting_vbox_add_table (vbox, 
				       _("Available Audio Codecs"), 
				       8, 2);

  pw->codecs_list_store = gtk_list_store_new (COLUMN_NUMBER,
					      G_TYPE_BOOLEAN,
					      G_TYPE_STRING,
					      G_TYPE_STRING,
					      G_TYPE_STRING);

  tree_view = 
    gtk_tree_view_new_with_model (GTK_TREE_MODEL (pw->codecs_list_store));
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (tree_view), TRUE);
  gtk_tree_view_set_search_column (GTK_TREE_VIEW (tree_view),0);
  
  frame = gtk_frame_new (NULL);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 
				  2*GNOMEMEETING_PAD_SMALL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_container_add (GTK_CONTAINER (frame), tree_view);
  gtk_container_set_border_width (GTK_CONTAINER (tree_view), 0);


  /* Set all Colums */
  renderer = gtk_cell_renderer_toggle_new ();
  column = gtk_tree_view_column_new_with_attributes (_("A"),
						     renderer,
						     "active", 
						     COLUMN_ACTIVE,
						     NULL);
  gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 25);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
  g_signal_connect (G_OBJECT (renderer), "toggled",
		    G_CALLBACK (codecs_list_fixed_toggled), 
		    GTK_TREE_MODEL (pw->codecs_list_store));

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Name"),
						     renderer,
						     "text", 
						     COLUMN_NAME,
						     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Info"),
						     renderer,
						     "text", 
						     COLUMN_INFO,
						     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Bandwidth"),
						     renderer,
						     "text", 
						     COLUMN_BANDWIDTH,
						     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);

  g_object_set_data (G_OBJECT (pw->codecs_list_store), "tree_view", (gpointer) tree_view);


  /* Here we add the codec buts in the order they are in the config file */
  gtk_table_attach (GTK_TABLE (table),  frame, 0, 1, 0, 8,        
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);           

  gnomemeeting_codecs_list_build (pw->codecs_list_store);

  button1 = gtk_button_new_from_stock (GTK_STOCK_GO_UP);
  gtk_table_attach (GTK_TABLE (table),  button1, 1, 2, 3, 4,        
                    (GtkAttachOptions) NULL,                           
                    (GtkAttachOptions) NULL,        
                    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);           
  g_object_set_data (G_OBJECT (button1), "operation", (gpointer) "up");
  gtk_widget_size_request (GTK_WIDGET (button1), &size_request1);
  gtk_container_set_border_width (GTK_CONTAINER (button1), 
				  GNOMEMEETING_PAD_SMALL);

  g_signal_connect (G_OBJECT (button1), "clicked",
		    G_CALLBACK (codecs_list_button_clicked_callback), 
		    GTK_TREE_MODEL (pw->codecs_list_store));

  button2 = gtk_button_new_from_stock (GTK_STOCK_GO_DOWN);
  gtk_table_attach (GTK_TABLE (table),  button2, 1, 2, 4, 5,        
                    (GtkAttachOptions) NULL,                           
                    (GtkAttachOptions) NULL,
                    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);  
  g_object_set_data (G_OBJECT (button2), "operation", (gpointer) "down");
  gtk_widget_size_request (GTK_WIDGET (button2), &size_request2);
  gtk_container_set_border_width (GTK_CONTAINER (button2), 
				  GNOMEMEETING_PAD_SMALL);
  g_signal_connect (G_OBJECT (button2), "clicked",
		    G_CALLBACK (codecs_list_button_clicked_callback), 
		    GTK_TREE_MODEL (pw->codecs_list_store));
  
  /* Set the buttons to the same one (the largest needed one) */
  if ( (size_request1.width >= size_request2.width) &&
       (size_request1.width >= width) )
    width = size_request1.width;
  else
    if (size_request2.width >= width)
      width = size_request2.width;

  gtk_widget_set_size_request (GTK_WIDGET (button1), width, 30);
  gtk_widget_set_size_request (GTK_WIDGET (button2), width, 30);

  gtk_widget_show_all (frame);


  /* Here we add the audio codecs options */
  table = gnomemeeting_vbox_add_table (vbox, _("Audio Codecs Settings"), 3, 2);

  pw->gsm_frames =
     gnomemeeting_table_add_spin (table, _("GSM Frames per packet:"),       
 				       "/apps/gnomemeeting/audio_settings/gsm_frames",
					_("The number of frames in each transmitted GSM packet."),
 				       1.0, 7.0, 1.0, 0);

  pw->g711_frames =
     gnomemeeting_table_add_spin (table, _("G.711 Frames per packet:"),       
 				       "/apps/gnomemeeting/audio_settings/g711_frames",
					_("The number of frames in each transmitted G.711 packet."),
 				       11.0, 240.0, 1.0, 1);

  pw->sd = 
    gnomemeeting_table_add_toggle (table, _("Enable Silence Detection"),       
 				       "/apps/gnomemeeting/audio_settings/sd",
					_("Enable/disable the silence detection for the GSM and G.711 codecs."), 2, 0);


  /* The jitter buffer */
  table = gnomemeeting_vbox_add_table (vbox, _("Dynamic Jitter Buffer"), 2, 2);

  pw->min_jitter_buffer =
    gnomemeeting_table_add_spin (table, _("Minimal Jitter Buffer:"), AUDIO_SETTINGS_KEY "min_jitter_buffer", _("The minimal jitter buffer size for audio reception (in ms)."), 20.0, 1000.0, 10.0, 0);

  pw->max_jitter_buffer =
    gnomemeeting_table_add_spin (table, _("Maximal Jitter Buffer:"), AUDIO_SETTINGS_KEY "max_jitter_buffer", _("The maximal jitter buffer size for audio reception (in ms)."), 500.0, 1000.0, 10.0, 1);
}
                                                                               

/* BEHAVIOR     :  It builds the notebook page for video codecs settings.
 *                 it adds it to the notebook.          
 * PRE          :  The notebook.                                               
 */                                                                            
void gnomemeeting_init_pref_window_video_codecs (GtkWidget *notebook) 
{                                                                              
  GtkWidget *vbox = NULL;                                                      
  GtkWidget *table = NULL;                                                     

                                                                               
  /* Get the data */                                                           
  GmPrefWindow *pw = gnomemeeting_get_pref_window (gm);              
                                                                               
                                                                               
  /* Packing widgets for the XDAP directory */ 
  vbox =  gtk_vbox_new (FALSE, 4);
  gtk_notebook_append_page (GTK_NOTEBOOK(notebook), vbox, NULL);  
  table = gnomemeeting_vbox_add_table (vbox, _("General Settings"),
                                              3, 1);                           

  /* Add fields */
  pw->tr_fps =
    gnomemeeting_table_add_spin (table, _("Maximum Transmitted FPS:"),       
 				       "/apps/gnomemeeting/video_settings/tr_fps",
				       _("The number of video frames transmitted each second."),
 				       1.0, 30.0, 1.0, 1);

  pw->fps = 
    gnomemeeting_table_add_toggle (table, _("Enable FPS Limitation"), "/apps/gnomemeeting/video_settings/enable_fps", _("Enable/disable the limit on the transmitted FPS."), 0, 0);

  pw->vid_tr = 
    gnomemeeting_table_add_toggle (table, _("Enable Video Transmission"), "/apps/gnomemeeting/video_settings/enable_video_transmission", _("Enable/disable the video transmission."), 2, 0);


  /* H.261 Settings */
  table = gnomemeeting_vbox_add_table (vbox, _("H.261 Settings"),
                                              3, 1);                           

  pw->tr_vq =
    gnomemeeting_table_add_spin (table, _("Transmitted Video Quality:"),       
 				       "/apps/gnomemeeting/video_settings/tr_vq",
				       _("The transmitted video quality:  choose 100% on a LAN for the best quality, 1% being the worst quality."),
 				       1.0, 100.0, 1.0, 0);

  pw->re_vq =
    gnomemeeting_table_add_spin (table, _("Received Video Quality:"),       
 				       "/apps/gnomemeeting/video_settings/re_vq",
				       _("The received video quality:  choose 100% on a LAN for the best quality, 1% being the worst quality."),
 				       1.0, 100.0, 1.0, 1);

  pw->tr_ub =
    gnomemeeting_table_add_spin (table, _("Transmitted Background Blocks:"),       
 				       "/apps/gnomemeeting/video_settings/tr_ub",
				       _("Here you can choose the number of blocks (that haven't changed) transmitted with each frame. These blocks fill in the background."),
 				       1.0, 99.0, 1.0, 2);
}                                                                              

            
void gnomemeeting_init_pref_window ()
{
  GtkTreeSelection *selection;
  GtkCellRenderer *cell;
  GtkWidget *tree_view;
  GtkTreeViewColumn *column;
  GtkTreeStore *model;
  GtkTreeIter iter;
  GtkTreeIter child_iter;

  GtkWidget *event_box, *hbox, *vbox;
  GtkWidget *frame;
  GtkWidget *pixmap;
  GtkWidget *label;

  PangoAttrList     *attrs; 
  PangoAttribute    *attr; 

  /* The notebook on the right */
  GtkWidget *notebook;

  /* Box inside the prefs window */
  GtkWidget *dialog_vbox;
 
  /* Get the data */
  GmWindow *gw = gnomemeeting_get_main_window (gm);
  GmPrefWindow *pw = gnomemeeting_get_pref_window (gm);

  gw->pref_window = gtk_dialog_new ();
  gtk_dialog_add_button (GTK_DIALOG (gw->pref_window), GTK_STOCK_CLOSE, 0);

  g_signal_connect (G_OBJECT (gw->pref_window), "response",
 		    G_CALLBACK (pref_window_clicked_callback), 
 		    (gpointer) pw);

  g_signal_connect (G_OBJECT (gw->pref_window), "delete_event",
 		    G_CALLBACK (pref_window_destroy_callback), 
 		    (gpointer) pw);

  gtk_window_set_title (GTK_WINDOW (gw->pref_window), 
			_("GnomeMeeting Settings"));	


  /* Construct the window */
  notebook = gtk_notebook_new ();

  dialog_vbox = GTK_DIALOG (gw->pref_window)->vbox;

  hbox = gtk_hbox_new (FALSE, 6);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 6);
  gtk_container_add (GTK_CONTAINER (dialog_vbox), hbox);

  pw->tips = gtk_tooltips_new ();


  /* Build the TreeView on the left */
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_box_pack_start (GTK_BOX (hbox), frame, FALSE, FALSE, 0);
  model = gtk_tree_store_new (2, G_TYPE_STRING, G_TYPE_INT);
  tree_view = gtk_tree_view_new ();
  gtk_tree_view_set_model (GTK_TREE_VIEW (tree_view), GTK_TREE_MODEL (model));
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
  gtk_container_add (GTK_CONTAINER (frame), tree_view);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (tree_view), FALSE);

  gtk_tree_selection_set_mode (GTK_TREE_SELECTION (selection),
			       GTK_SELECTION_BROWSE);


  /* Some design stuff to put the notebook pages in it */
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_box_pack_start (GTK_BOX (hbox), frame, TRUE, TRUE, 0);
  gtk_widget_show (frame);
  
  vbox = gtk_vbox_new (FALSE, 4);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_widget_show (vbox);

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_OUT);
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, TRUE, 0);
  gtk_widget_show (frame);
    
  hbox = gtk_hbox_new (FALSE, 4);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 4);
  gtk_container_add (GTK_CONTAINER (frame), hbox);
  gtk_widget_show (hbox);
    
  label = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  attrs = pango_attr_list_new ();
  attr = pango_attr_scale_new (PANGO_SCALE_LARGE);
  attr->start_index = 0;
  attr->end_index = -1; 
  pango_attr_list_insert (attrs, attr); 
  attr = pango_attr_weight_new (PANGO_WEIGHT_HEAVY);
  attr->start_index = 0;
  attr->end_index = -1;
  pango_attr_list_insert (attrs, attr);
  gtk_label_set_attributes (GTK_LABEL (label), attrs);
  pango_attr_list_unref (attrs);
  gtk_widget_show (label);

  gtk_box_pack_start (GTK_BOX (vbox), notebook, FALSE, FALSE, 0);

  g_object_set_data (G_OBJECT (gw->pref_window), "section_label", label);


  /* All the notebook pages */
  /* General Section */
  gtk_tree_store_append (GTK_TREE_STORE (model), &iter, NULL);
  gtk_tree_store_set (GTK_TREE_STORE (model),
		      &iter, 0, _("General"), 1, 0, -1);

  gtk_tree_store_append (GTK_TREE_STORE (model), &child_iter, &iter);
  gtk_tree_store_set (GTK_TREE_STORE (model),
		      &child_iter, 0, _("Personal Data"), 1, 1, -1);
  gnomemeeting_init_pref_window_general (notebook);

  gtk_tree_store_append (GTK_TREE_STORE (model), &child_iter, &iter);
  gtk_tree_store_set (GTK_TREE_STORE (model),
		      &child_iter, 0, _("General Settings"), 1, 2, -1);
  gnomemeeting_init_pref_window_interface (notebook);

  gtk_tree_store_append (GTK_TREE_STORE (model), &child_iter, &iter);
  gtk_tree_store_set (GTK_TREE_STORE (model),
		      &child_iter, 0, _("H.323 Advanced"), 1, 3, -1);
  gnomemeeting_init_pref_window_h323_advanced (notebook);          

  gtk_tree_store_append (GTK_TREE_STORE (model), &child_iter, &iter);
  gtk_tree_store_set (GTK_TREE_STORE (model),
		      &child_iter, 0, _("Directory Settings"), 1, 4, -1);
  gnomemeeting_init_pref_window_directories (notebook);

 
  /* Another section */
  gtk_tree_store_append (GTK_TREE_STORE (model), &iter, NULL);
  gtk_tree_store_set (GTK_TREE_STORE (model),
		      &iter, 0, _("Codecs"), 1, 0, -1);

  gtk_tree_store_append (GTK_TREE_STORE (model), &child_iter, &iter);
  gtk_tree_store_set (GTK_TREE_STORE (model),
		      &child_iter, 0, _("Audio Codecs"), 1, 5, -1);
  gnomemeeting_init_pref_window_audio_codecs (notebook);


  gtk_tree_store_append (GTK_TREE_STORE (model), &child_iter, &iter);
  gtk_tree_store_set (GTK_TREE_STORE (model),
		      &child_iter, 0, _("Video Codecs"), 1, 6, -1);
  gnomemeeting_init_pref_window_video_codecs (notebook);


  /* Another section */
  gtk_tree_store_append (GTK_TREE_STORE (model), &iter, NULL);
  gtk_tree_store_set (GTK_TREE_STORE (model),
		      &iter, 0, _("Devices"), 1, 0, -1);

  gtk_tree_store_append (GTK_TREE_STORE (model), &child_iter, &iter);
  gtk_tree_store_set (GTK_TREE_STORE (model),
		      &child_iter, 0, _("Audio Devices"), 1, 7, -1);
  gnomemeeting_init_pref_window_audio_devices (notebook);

  gtk_tree_store_append (GTK_TREE_STORE (model), &child_iter, &iter);
  gtk_tree_store_set (GTK_TREE_STORE (model),
		      &child_iter, 0, _("Video Devices"), 1, 8, -1);
  gnomemeeting_init_pref_window_video_devices (notebook);


  cell = gtk_cell_renderer_text_new ();
  
  column = gtk_tree_view_column_new_with_attributes (_("Section"),
						     cell, "text", 0, NULL);
  
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view),
			       GTK_TREE_VIEW_COLUMN (column));

  gtk_tree_view_expand_all (GTK_TREE_VIEW (tree_view));


  /* Now, add the logo as first page */
  pixmap =  gtk_image_new_from_file 
    (GNOMEMEETING_IMAGES "/gnomemeeting-logo.png");

  event_box = gtk_event_box_new ();
  gtk_container_add (GTK_CONTAINER (event_box), 
	  	     GTK_WIDGET (pixmap));

  gtk_notebook_prepend_page (GTK_NOTEBOOK (notebook), event_box, NULL);
 
  gtk_notebook_set_show_border (GTK_NOTEBOOK (notebook), FALSE);
  gtk_notebook_popup_enable (GTK_NOTEBOOK (notebook));
  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (notebook), FALSE);
  gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), 0);

  g_signal_connect (selection, "changed",
		    G_CALLBACK (tree_selection_changed_cb), 
		    notebook);
}

