
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2003 Damien Sandras
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
 *                         pref_window.cpp  -  description
 *                         -------------------------------
 *   begin                : Tue Dec 26 2000
 *   copyright            : (C) 2000-2003 by Damien Sandras
 *   description          : This file contains all the functions needed to
 *                          create the preferences window and all its callbacks
 *   Additional code      : Miguel Rodríguez Pérez  <migrax@terra.es> 
 */


#include "../config.h"

#include "pref_window.h"
#include "gnomemeeting.h"
#include "ils.h"
#include "sound_handling.h"
#include "misc.h"
#include "urlhandler.h"

/* Declarations */

extern GtkWidget *gm;
extern GnomeMeeting *MyApp;	

static void pref_window_clicked_callback (GtkDialog *,
					  int,
					  gpointer);

static gint pref_window_destroy_callback (GtkWidget *,
					  GdkEvent *,
					  gpointer);

static void personal_data_update_button_clicked (GtkWidget *,
						 gpointer);

static void gatekeeper_update_button_clicked (GtkWidget *,
					      gpointer);

static void codecs_list_button_clicked_callback (GtkWidget *,
						 gpointer);

static void gnomemeeting_codecs_list_add (GtkTreeIter,
					  GtkListStore *, 
					  const gchar *,
					  bool,
					  bool,
					  gchar *);

static GtkWidget *gnomemeeting_pref_window_add_update_button (GtkWidget *,
							      const char *,
							      const char *,
							      GtkSignalFunc,
							      gchar *,  
							      int,
							      int);

static void codecs_list_fixed_toggled (GtkCellRendererToggle *,
				       gchar *, 
				       gpointer);

static void video_image_browse_clicked (GtkWidget *,
					gpointer);

static void file_selector_clicked (GtkFileSelection *,
				   gpointer);

static void gnomemeeting_init_pref_window_general (GtkWidget *);

static void gnomemeeting_init_pref_window_interface (GtkWidget *);

static void gnomemeeting_init_pref_window_directories (GtkWidget *);

static void gnomemeeting_init_pref_window_call_forwarding (GtkWidget *);

static void gnomemeeting_init_pref_window_h323_advanced (GtkWidget *);

static void gnomemeeting_init_pref_window_gatekeeper (GtkWidget *);

static void gnomemeeting_init_pref_window_nat (GtkWidget *);

static void gnomemeeting_init_pref_window_video_devices (GtkWidget *);

static void gnomemeeting_init_pref_window_audio_devices (GtkWidget *);

static void gnomemeeting_init_pref_window_audio_codecs (GtkWidget *);

static void gnomemeeting_init_pref_window_video_codecs (GtkWidget *);


enum {
  
  COLUMN_CODEC_ACTIVE,
  COLUMN_CODEC_NAME,
  COLUMN_CODEC_INFO,
  COLUMN_CODEC_BANDWIDTH,
  COLUMN_CODEC_SELECTABLE,
  COLUMN_CODEC_COLOR,
  COLUMN_CODEC_NUMBER
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
  GmDruidWindow *dw = NULL;

  gw = MyApp->GetMainWindow ();
  pw = MyApp->GetPrefWindow ();
  dw = MyApp->GetDruidWindow ();

  MyApp->DetectDevices ();
  
  /* The player */
  gnomemeeting_update_pstring_option_menu (pw->audio_player,
					   gw->audio_player_devices,
					   DEVICES_KEY "audio_player");
#ifndef DISABLE_GNOME
  gnomemeeting_update_pstring_option_menu (dw->audio_player,
					   gw->audio_player_devices,
					   DEVICES_KEY "audio_player");
#endif

  gnomemeeting_update_pstring_option_menu (pw->audio_player_mixer,
					   gw->audio_mixers,
					   DEVICES_KEY "audio_player_mixer");
#ifndef DISABLE_GNOME
  gnomemeeting_update_pstring_option_menu (dw->audio_player_mixer,
					   gw->audio_mixers,
					   DEVICES_KEY "audio_player_mixer");
#endif
  
  /* The recorder */
  gnomemeeting_update_pstring_option_menu (pw->audio_recorder,
					   gw->audio_recorder_devices,
					   DEVICES_KEY "audio_recorder");
#ifndef DISABLE_GNOME
  gnomemeeting_update_pstring_option_menu (dw->audio_recorder,
					   gw->audio_recorder_devices,
					   DEVICES_KEY "audio_recorder");
#endif

  gnomemeeting_update_pstring_option_menu (pw->audio_recorder_mixer,
					   gw->audio_mixers,
					   DEVICES_KEY "audio_recorder_mixer");
#ifndef DISABLE_GNOME
  gnomemeeting_update_pstring_option_menu (dw->audio_recorder_mixer,
					   gw->audio_mixers,
					   DEVICES_KEY "audio_recorder_mixer");
#endif

  
  /* The Video player */
#ifndef DISABLE_GNOME
  gnomemeeting_update_pstring_option_menu (dw->video_device,
					   gw->video_devices,
					   DEVICES_KEY "video_recorder");
#endif

  gnomemeeting_update_pstring_option_menu (pw->video_device,
					   gw->video_devices,
					   DEVICES_KEY "video_recorder");
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
 * BEHAVIOR     :  Updates the values.
 * PRE          :  /
 */
static void personal_data_update_button_clicked (GtkWidget *widget, 
						  gpointer data)
{
  GMH323EndPoint *endpoint = NULL;
  GConfClient *client = NULL;

  endpoint = MyApp->Endpoint ();
  client = gconf_client_get_default ();

  /* Both are able to not register if the option is not active */
  endpoint->ILSRegister ();
  endpoint->GatekeeperRegister ();
}


/* DESCRIPTION  :  This callback is called when the user clicks
 *                 on the Update button of the gatekeeper Settings.
 * BEHAVIOR     :  Updates the values, and try to register to the gatekeeper.
 * PRE          :  /
 */
static void gatekeeper_update_button_clicked (GtkWidget *widget, 
					      gpointer data)
{
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
		      COLUMN_CODEC_NAME, &selected_codec_name, -1);


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

  gw = MyApp->GetMainWindow ();

  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {

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

  pw = MyApp->GetPrefWindow ();
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
gnomemeeting_codecs_list_add (GtkTreeIter iter, GtkListStore *store, 
			      const gchar *codec_name, bool enabled,
			      bool possible, gchar *color)
{
  gchar *data [3];

  data [0] = g_strdup (codec_name);
  data [1] = NULL;
  data [2] = NULL;

  if (!strcmp (codec_name, "LPC-10")) {
    data [1] = g_strdup (_("Okay"));
    data [2] = g_strdup ("3.46 kbits");
  }

  if (!strcmp (codec_name, "SpeexNarrow-15k")) {
    data [1] = g_strdup (_("Excellent"));
    data [2] = g_strdup ("15 Kbps");
  }

  if (!strcmp (codec_name, "SpeexNarrow-8k")) {
    data [1] = g_strdup (_("Good Quality"));
    data [2] = g_strdup ("8 Kbps");
  }

  if (!strcmp (codec_name, "MS-GSM")) {
    data [1] = g_strdup (_("Good Quality"));
    data [2] = g_strdup ("13 Kbps");
  }

  if (!strcmp (codec_name, "G.711-ALaw-64k")) {
    data [1] = g_strdup (_("Excellent"));
    data [2] = g_strdup ("64 Kbps");
  }

  if (!strcmp (codec_name, "G.711-uLaw-64k")) {
    data [1] = g_strdup (_("Excellent"));
    data [2] = g_strdup ("64 Kbps");
  }

  if (!strcmp (codec_name, "GSM-06.10")) {
    data [1] = g_strdup (_("Good Quality"));
    data [2] = g_strdup ("16.5 Kbps");
  }

  if (!strcmp (codec_name, "G.726-32k")) {
    data [1] = g_strdup (_("Good Quality"));
    data [2] = g_strdup ("32 Kbps");
  }

  if (!strcmp (codec_name, "G.723.1")) {
    data [1] = g_strdup (_("Excellent Quality"));
    data [2] = g_strdup ("6.3 Kbps");
  }

  if (data [1] && data [2]) {
 
    gtk_list_store_append (store, &iter);
    gtk_list_store_set (store, &iter,
			COLUMN_CODEC_ACTIVE, enabled,
			COLUMN_CODEC_NAME, data [0],
			COLUMN_CODEC_INFO, data [1],
			COLUMN_CODEC_BANDWIDTH, data [2],
			COLUMN_CODEC_SELECTABLE, possible,
			COLUMN_CODEC_COLOR, color,
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
  gtk_tree_model_get (model, &iter, COLUMN_CODEC_ACTIVE, &fixed, -1);
  gtk_tree_model_get (model, &iter, COLUMN_CODEC_NAME, &selected_codec_name, -1);
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
  gchar *cselect_row = NULL;
  gchar *selected_codec = NULL;
  gchar *quicknet = NULL;
    
  PString dev;
  PString codec;
    
  GSList *codecs_data = NULL;
  GConfClient *client = gconf_client_get_default ();

  codecs_data = 
    gconf_client_get_list (client, 
			   "/apps/gnomemeeting/audio_codecs/codecs_list", 
			   GCONF_VALUE_STRING, NULL);

  selected_codec =
    (gchar *) g_object_get_data (G_OBJECT (codecs_list_store), 
				 "selected_codec");

  gtk_list_store_clear (GTK_LIST_STORE (codecs_list_store));

  /* We are adding the codecs */
  while (codecs_data) {

    gchar **couple = g_strsplit ((gchar *) codecs_data->data, "=", 0);

    if ((couple [0]) && (couple [1])) {

      quicknet =
	gconf_client_get_string (client, DEVICES_KEY "audio_player", NULL);

      codec = PString (couple [0]);
      if (quicknet)
	dev = PString (quicknet);
      
      if (codec.Find ("G.723.1") != P_MAX_INDEX &&
	  dev.Find ("phone") == P_MAX_INDEX) {
	
	gnomemeeting_codecs_list_add (list_iter, codecs_list_store, 
				      couple [0], 0, false, "darkgray");

      }
      else {

	gnomemeeting_codecs_list_add (list_iter, codecs_list_store, 
				      couple [0], atoi (couple [1]),
				      true, "black");
      }
      
      g_free (quicknet);
    }

    
    if ((selected_codec) && (!strcmp (selected_codec, couple [0]))) 
      selected_row = current_row;

    g_strfreev (couple);
    codecs_data = codecs_data->next;
    current_row++;
  }

      
  cselect_row = g_strdup_printf("%d", selected_row);
  tree_path = gtk_tree_path_new_from_string (cselect_row);
  tree_view =
    GTK_TREE_VIEW (g_object_get_data (G_OBJECT (codecs_list_store), 
				      "tree_view"));
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
  
  gtk_tree_selection_select_path (GTK_TREE_SELECTION (selection),
				  tree_path);
  
  g_free (cselect_row);
  g_slist_free (codecs_data);

  gtk_tree_path_free (tree_path);
}
                                                                  
                                                                               
static GtkWidget *
gnomemeeting_pref_window_add_update_button (GtkWidget *table,
					    const char *stock_id,
					    const char *label,
					    GtkSignalFunc func,
					    gchar *tooltip,  
					    int row, int col)
{
  GtkWidget *image = NULL;
  GtkWidget *button = NULL;                                                    
  GmPrefWindow *pw = NULL;                                           

  
  pw = MyApp->GetPrefWindow ();                                      

  image = gtk_image_new_from_stock (stock_id, GTK_ICON_SIZE_BUTTON);
  button = gnomemeeting_button_new (label, image);

  gtk_table_attach (GTK_TABLE (table),  button, col, col+1, row, row+1,
                    (GtkAttachOptions) (NULL),                           
                    (GtkAttachOptions) (NULL),                           
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
static void
gnomemeeting_init_pref_window_general (GtkWidget *notebook)
{
  GtkWidget *vbox = NULL;
  GtkWidget *table = NULL;

  /* Get the data */
  GmPrefWindow *pw = MyApp->GetPrefWindow ();

  /* Packing widgets */                                                        
  vbox = gtk_vbox_new (FALSE, 4);
  table = gnomemeeting_vbox_add_table (vbox, _("Personal Information"), 5, 3);

  /* Add all the fields */
  pw->firstname =
    gnomemeeting_table_add_entry (table, _("_First name:"), PERSONAL_DATA_KEY "firstname", _("Enter your first name."), 0);
  gtk_widget_set_size_request (GTK_WIDGET (pw->firstname), 250, -1);
  gtk_entry_set_max_length (GTK_ENTRY (pw->firstname), 65);

  pw->surname = 
    gnomemeeting_table_add_entry (table, _("Sur_name:"), PERSONAL_DATA_KEY "lastname", _("Enter your last name."), 1);
  gtk_widget_set_size_request (GTK_WIDGET (pw->surname), 250, -1);
  gtk_entry_set_max_length (GTK_ENTRY (pw->surname), 65);
                                                                               
  pw->mail =
    gnomemeeting_table_add_entry (table, _("E-_mail address:"), PERSONAL_DATA_KEY "mail", _("Enter your e-mail address."), 2);
  gtk_widget_set_size_request (GTK_WIDGET (pw->mail), 250, -1);
  gtk_entry_set_max_length (GTK_ENTRY (pw->mail), 65);
  
  pw->comment =
    gnomemeeting_table_add_entry (table, _("_Comment:"), PERSONAL_DATA_KEY "comment", _("Enter a comment about yourself for the user directory."), 3);
  gtk_widget_set_size_request (GTK_WIDGET (pw->comment), 250, -1);
  gtk_entry_set_max_length (GTK_ENTRY (pw->comment), 65);
  
  pw->location =
    gnomemeeting_table_add_entry (table, _("_Location:"), PERSONAL_DATA_KEY "location", _("Enter your location (country or city) for the user directory."), 4);
  gtk_widget_set_size_request (GTK_WIDGET (pw->location), 250, -1);
  gtk_entry_set_max_length (GTK_ENTRY (pw->location), 65);

  /* Add the try button */
  pw->directory_update_button =
    gnomemeeting_pref_window_add_update_button (table, GTK_STOCK_APPLY, _("_Apply"), GTK_SIGNAL_FUNC (personal_data_update_button_clicked), _("Click here to update the user directory you are registered to with the new First Name, Last Name, E-Mail, Comment and Location or to update your alias on the Gatekeeper."), 5, 2);
  gtk_container_set_border_width (GTK_CONTAINER (pw->directory_update_button),
				  GNOMEMEETING_PAD_SMALL*2);

  gtk_notebook_append_page (GTK_NOTEBOOK(notebook), vbox, NULL);
}                                                                              
                                                                               

/* BEHAVIOR     :  It builds the notebook page for interface settings
 *                 add it to the notebook, default values are set from the
 *                 options struct given as parameter.
 * PRE          :  See init_pref_audio_codecs.
 */
static void
gnomemeeting_init_pref_window_interface (GtkWidget *notebook)
{
  GtkWidget *vbox = NULL;
  GtkWidget *table = NULL;

  /* Get the data */
  GmPrefWindow *pw = MyApp->GetPrefWindow ();

  /* Packing widgets */
  vbox = gtk_vbox_new (FALSE, 4);
  gtk_notebook_append_page (GTK_NOTEBOOK(notebook), vbox, NULL);
  table = gnomemeeting_vbox_add_table (vbox, _("GnomeMeeting GUI"), 2, 2);

  pw->show_splash =
    gnomemeeting_table_add_toggle (table, _("_Show splash screen"), VIEW_KEY "show_splash", _("If enabled, the splash screen will be displayed when GnomeMeeting starts."), 0);

  pw->start_hidden =
    gnomemeeting_table_add_toggle (table, _("Start _hidden"), VIEW_KEY "start_docked", _("If enabled, GnomeMeeting will start hidden. The docklet must be enabled."), 1);

  
  /* Packing widget */
  table = gnomemeeting_vbox_add_table (vbox, _("Behavior"), 3, 2);

  /* The toggles */
  pw->aa =
    gnomemeeting_table_add_toggle (table, _("_Automatically answer calls"), GENERAL_KEY "auto_answer", _("If enabled, incoming calls will be automatically answered."), 1);
  
  pw->dnd =
    gnomemeeting_table_add_toggle (table, _("Do _not disturb"), GENERAL_KEY "do_not_disturb", _("If enabled, incoming calls will be automatically refused."), 0);
                                                                               
  pw->incoming_call_popup =
    gnomemeeting_table_add_toggle (table, _("Display a po_pup window when receiving a call"), VIEW_KEY "show_popup", _("If enabled, a popup window will be displayed when receiving an incoming call."), 2);

  
  /* Packing widget */
  table = gnomemeeting_vbox_add_table (vbox, _("Video Display"), 2, 1);

#ifdef HAS_SDL
  /* Translators: the full sentence is Use a fullscreen size 
     of X by Y pixels */
  gnomemeeting_table_add_spin_range (table, _("Use a fullscreen size of"),
				     &pw->fullscreen_width,
				     _("by"),
				     &pw->fullscreen_height,
				     _("pixels"),
				     VIDEO_DISPLAY_KEY "fullscreen_width",
				     VIDEO_DISPLAY_KEY "fullscreen_height",
				     _("The image width for fullscreen."),
				     _("The image height for fullscreen."),
				     10.0, 10.0, 640.0, 480.0, 10.0, 0);
#endif
  
  pw->bilinear_filtering =
    gnomemeeting_table_add_toggle (table, _("Enable bilinear _filtering on displayed video"), VIDEO_DISPLAY_KEY "bilinear_filtering", _("Enable or disable bilinear interpolation when rendering video images (this has no effect in fullscreen mode)."), 2);

  /* Text Chat */
  table = gnomemeeting_vbox_add_table (vbox, _("Text Chat"), 1, 1);
  
  pw->auto_clear_text_chat = gnomemeeting_table_add_toggle (table, _("Automatically clear the text chat at the end of calls"), GENERAL_KEY "auto_clear_text_chat", _("If enabled, the text chat will automatically be cleared at the end of calls."), 0);
}


/* BEHAVIOR     :  It builds the notebook page for XDAP directories,
 *                 it adds it to the notebook.
 * PRE          :  The notebook.
 */
static void
gnomemeeting_init_pref_window_directories (GtkWidget *notebook)
{
  GtkWidget *vbox = NULL;
  GtkWidget *table = NULL;

  /* Get the data */
  GmPrefWindow *pw = MyApp->GetPrefWindow ();


  /* Packing widgets for the XDAP directory */
  vbox = gtk_vbox_new (FALSE, 4);
  gtk_notebook_append_page (GTK_NOTEBOOK(notebook), vbox, NULL);
  table = gnomemeeting_vbox_add_table (vbox, _("User Directory"), 3, 2);


  /* Add all the fields */                                                     
  pw->ldap_server =
    gnomemeeting_table_add_entry (table, _("User directory:"), LDAP_KEY "ldap_server", _("The user directory server to register with."), 0, true);

  pw->ldap =
    gnomemeeting_table_add_toggle (table, _("Enable _registering"), LDAP_KEY "register", _("If enabled, register with the selected user directory."), 1);

  pw->ldap_visible =
    gnomemeeting_table_add_toggle (table, _("_Publish my details in the users directory"), LDAP_KEY "visible", _("If enabled, your details are shown to people browsing the user directory. If disabled, you are not visible to users browsing the user directory, but they can still use the callto URL to call you."), 2);
}


/* BEHAVIOR     :  It builds the notebook page for call forwarding,
 *                 it adds it to the notebook.                                
 * PRE          :  The notebook.                                               
 */                                                                            
static void
gnomemeeting_init_pref_window_call_forwarding (GtkWidget *notebook)
{
  GtkWidget *vbox = NULL;
  GtkWidget *table = NULL;

  
  /* Get the data */
  GmPrefWindow *pw = MyApp->GetPrefWindow ();

  
  /* Packing widgets */
  vbox = gtk_vbox_new (FALSE, 4);
  gtk_notebook_append_page (GTK_NOTEBOOK(notebook), vbox, NULL);
  table = gnomemeeting_vbox_add_table (vbox, _("Call Forwarding"), 4, 2);


  /* Add all the fields */                                                     
  pw->forward_host = 
    gnomemeeting_table_add_entry (table, _("Forward calls to _host:"), CALL_FORWARDING_KEY "forward_host", _("The host where calls should be forwarded to in the cases selected above."), 0, true);
  if (!strcmp (gtk_entry_get_text (GTK_ENTRY (pw->forward_host)), ""))
    gtk_entry_set_text (GTK_ENTRY (pw->forward_host),
			GMURL ().GetDefaultURL ());
  gtk_widget_set_size_request (GTK_WIDGET (pw->forward_host), 250, -1);  
  
  pw->always_forward =
    gnomemeeting_table_add_toggle (table, _("_Always forward calls to the given host"), CALL_FORWARDING_KEY "always_forward", _("If enabled, all incoming calls will be forwarded to the host that is specified in the field below."), 1);

  pw->no_answer_forward =
    gnomemeeting_table_add_toggle (table, _("Forward calls to the given host if _no answer"), CALL_FORWARDING_KEY "no_answer_forward", _("If enabled, all incoming calls will be forwarded to the host that is specified in the field below if you do not answer the call."), 2);

  pw->busy_forward =
    gnomemeeting_table_add_toggle (table, _("Forward calls to the given host if _busy"), CALL_FORWARDING_KEY "busy_forward", _("If enabled, all incoming calls will be forwarded to the host that is specified in the field below if you already are in a call or if you are in Do Not Disturb mode."), 3);
}


/* BEHAVIOR     :  It builds the notebook page for H.323 advanced settings
 *                 and add it to the notebook.
 * PRE          :  The notebook.
 */
static void
gnomemeeting_init_pref_window_h323_advanced (GtkWidget *notebook)
{
  GtkWidget *vbox = NULL;
  GtkWidget *table = NULL;

  gchar *capabilities [] = {_("All"),
			    _("None"),
			    _("rfc2833"),
			    _("Signal"),
			    _("String"),
			    NULL};
                                                                               
  /* Get the data */                                                           
  GmPrefWindow *pw = MyApp->GetPrefWindow ();

  
  /* Packing widget */
  vbox = gtk_vbox_new (FALSE, 4);
  table =
    gnomemeeting_vbox_add_table (vbox, _("H.323 Version 2 Settings"), 2, 1);

  /* The toggles */                                                            
  pw->ht =
    gnomemeeting_table_add_toggle (table, _("Enable H.245 _tunnelling"), GENERAL_KEY "h245_tunneling", _("This enables H.245 Tunnelling mode. In H.245 Tunnelling mode H.245 messages are encapsulated into the the H.225 channel (port 1720). This saves one TCP connection during calls. H.245 Tunnelling was introduced in H.323v2 and Netmeeting does not support it. Using both Fast Start and H.245 Tunnelling can crash some versions of Netmeeting."), 0);

  pw->fs =
    gnomemeeting_table_add_toggle (table, _("Enable fast _start procedure"), GENERAL_KEY "fast_start", _("Connection will be established in Fast Start mode. Fast Start is a new way to start calls faster that was introduced in H.323v2. It is not supported by Netmeeting and using both Fast Start and H.245 Tunnelling can crash some versions of Netmeeting."), 1);

  
  /* Packing widget */                                                         
  table =
    gnomemeeting_vbox_add_table (vbox, _("User Input Capabilities"), 1, 1);
  
  pw->uic =
    gnomemeeting_table_add_int_option_menu (table, _("User Input Capabilities _type:"), capabilities, GENERAL_KEY "user_input_capability", _("This permits to set the mode for User Input Capabilities. The values can be \"All\", \"None\", \"rfc2833\", \"Signal\" or \"String\" (default is \"All\"). Choosing other values than \"All\", \"String\" or \"rfc2833\" disables the Text Chat."), 0);
  
  gtk_notebook_append_page (GTK_NOTEBOOK(notebook), vbox, NULL);
}                               


/* BEHAVIOR     :  It builds the notebook page for the gatekeeper
 *                 and add it to the notebook.
 * PRE          :  The notebook.
 */
static void
gnomemeeting_init_pref_window_gatekeeper (GtkWidget *notebook)
{
  GtkWidget *vbox = NULL;
  GtkWidget *table = NULL;

  GtkWidget *button = NULL;
  gchar *options [] = {_("Do not register"), 
		       _("Gatekeeper host"), 
		       _("Gatekeeper ID"), 
		       _("Automatically discover"), 
		       NULL};

  
  /* Get the data */
  GmPrefWindow *pw = MyApp->GetPrefWindow ();

  
  /* Add fields for the gatekeeper */
  vbox = gtk_vbox_new (FALSE, 4);
  table = gnomemeeting_vbox_add_table (vbox, _("Gatekeeper"), 5, 3);

  pw->gk_id = 
    gnomemeeting_table_add_entry (table, _("Gatekeeper _ID:"), GATEKEEPER_KEY "gk_id", _("The Gatekeeper identifier to register with."), 1);

  pw->gk_host = 
    gnomemeeting_table_add_entry (table, _("Gatekeeper _host:"), GATEKEEPER_KEY "gk_host", _("The Gatekeeper host to register with."), 2);

  pw->gk_alias = 
    gnomemeeting_table_add_entry (table, _("Gatekeeper _alias:"), GATEKEEPER_KEY "gk_alias", _("The Gatekeeper alias to use when registering (string, or E164 ID if only 0123456789#)."), 3);

  pw->gk_password = 
    gnomemeeting_table_add_entry (table, _("Gatekeeper _password:"), GATEKEEPER_KEY "gk_password", _("The Gatekeeper password to use for H.235 authentication to the Gatekeeper."), 4);
  gtk_entry_set_visibility (GTK_ENTRY (pw->gk_password), FALSE);

  pw->gk = 
    gnomemeeting_table_add_int_option_menu (table, _("Registering method:"), options, GATEKEEPER_KEY "registering_method", _("Registering method to use"), 0);

  button =
    gnomemeeting_pref_window_add_update_button (table, GTK_STOCK_APPLY, _("_Apply"), GTK_SIGNAL_FUNC (gatekeeper_update_button_clicked), _("Click here to update your Gatekeeper settings."), 5, 2);

  gtk_notebook_append_page (GTK_NOTEBOOK(notebook), vbox, NULL);
}


/* BEHAVIOR     :  It builds the notebook page for NAT support
 *                 and add it to the notebook.
 * PRE          :  The notebook.
 */
static void
gnomemeeting_init_pref_window_nat (GtkWidget *notebook)
{
  GtkWidget *vbox = NULL;
  GtkWidget *table = NULL;


  /* Get the data */
  GmPrefWindow *pw = MyApp->GetPrefWindow ();

  /* IP translation */
  vbox = gtk_vbox_new (FALSE, 4);
  table =
    gnomemeeting_vbox_add_table (vbox, _("NAT/PAT Router Support"), 2, 1);

  pw->ip_translation = 
    gnomemeeting_table_add_toggle (table, _("Enable IP _translation"), GENERAL_KEY "ip_translation", _("This enables IP translation. IP translation is useful if GnomeMeeting is running behind a NAT/PAT router. You have to put the public IP of the router in the field below. If you are registered to ils.seconix.com, GnomeMeeting will automatically fetch the public IP using the ILS service. If your router natively supports H.323, you can disable this."), 1);

  pw->public_ip = 
    gnomemeeting_table_add_entry (table, _("Public _IP of the NAT/PAT router:"), GENERAL_KEY "public_ip", _("Enter the public IP of your NAT/PAT router if you want to use IP translation. If you are registered to ils.seconix.com, GnomeMeeting will automatically fetch the public IP using the ILS service."), 2);

  gtk_notebook_append_page (GTK_NOTEBOOK(notebook), vbox, NULL);
}


/* BEHAVIOR     :  It builds the notebook page for the audio devices 
 *                 settings and add it to the notebook.
 * PRE          :  See init_pref_audio_codecs.
 */
static void
gnomemeeting_init_pref_window_audio_devices (GtkWidget *notebook)
{
  GConfClient *client = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *table = NULL;

  gchar *aec [] = {_("Off"),
		   _("Low"),
		   _("Medium"),
		   _("High"),
		   _("AGC"),
		   NULL};

  /* Get the data */                                             
  GmWindow *gw = MyApp->GetMainWindow ();
  GmPrefWindow *pw = MyApp->GetPrefWindow ();
  
  client = gconf_client_get_default ();

  /* Packing widgets for the XDAP directory */
  vbox = gtk_vbox_new (FALSE, 4);
  gtk_notebook_append_page (GTK_NOTEBOOK(notebook), vbox, NULL);
  table = gnomemeeting_vbox_add_table (vbox, _("Audio Devices"), 5, 3);
                                                                               
  /* Add all the fields */
  /* The player */
  pw->audio_player =
    gnomemeeting_table_add_pstring_option_menu (table, _("Playing device:"), gw->audio_player_devices, DEVICES_KEY "audio_player", _("Select the audio player device to use."), 0);

  pw->audio_player_mixer = 
    gnomemeeting_table_add_pstring_option_menu (table, _("Playing mixer:"), gw->audio_mixers, DEVICES_KEY "audio_player_mixer", _("Select the mixer to use to change the volume of the audio player."), 1);
  
  /* The recorder */
  pw->audio_recorder = 
    gnomemeeting_table_add_pstring_option_menu (table, _("Recording device:"), gw->audio_recorder_devices, DEVICES_KEY "audio_recorder", _("Select the audio recorder device to use."), 2);

  pw->audio_recorder_mixer = 
    gnomemeeting_table_add_pstring_option_menu (table, _("Recording mixer:"), gw->audio_mixers, DEVICES_KEY "audio_recorder_mixer", _("Select the mixer to use to change the volume of the audio recorder."), 3);

  /* That button will refresh the devices list */
  gnomemeeting_pref_window_add_update_button (table, GTK_STOCK_REFRESH, _("_Detect devices"), GTK_SIGNAL_FUNC (refresh_devices), _("Click here to refresh the devices list."), 4, 2);
  
#ifdef HAS_IXJ
  /* The Quicknet devices related options */
  table = gnomemeeting_vbox_add_table (vbox, _("Quicknet Device"), 2, 2);

  pw->lid_aec =
    gnomemeeting_table_add_int_option_menu (table, _("Echo _cancellation:"), aec, DEVICES_KEY "lid_aec", _("The Automatic Echo Cancellation level: Off, Low, Medium, High, Automatic Gain Compensation. Choosing Automatic Gain Compensation modulates the volume for best quality."), 0);

  pw->lid_country =
    gnomemeeting_table_add_entry (table, _("Country _code:"), DEVICES_KEY "lid_country", _("The two-letter country code of your country (e.g.: BE, UK, FR, DE, ...)."), 1, false);
  gtk_entry_set_max_length (GTK_ENTRY (pw->lid_country), 2);
  gtk_widget_set_size_request (GTK_WIDGET (pw->lid_country), 100, -1);  
#endif
}


/* BEHAVIOR     :  It builds the notebook page for the video devices 
 *                 settings and add it to the notebook.
 * PRE          :  See init_pref_audio_codecs.
 */
static void
gnomemeeting_init_pref_window_video_devices (GtkWidget *notebook)
{
  GConfClient *client = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *table = NULL;

  GtkWidget *button = NULL;

  gchar *video_size [] = {_("Small"),
			  _("Large"), 
			  NULL};
  gchar *video_format [] = {_("PAL (Europe)"), 
			    _("NTSC (America)"), 
			    _("SECAM (France)"), 
			    _("Auto"), 
			    NULL};


  /* Get the data */                                             
  GmWindow *gw = MyApp->GetMainWindow ();
  GmPrefWindow *pw = MyApp->GetPrefWindow ();
  
  client = gconf_client_get_default ();

  /* Packing widgets for the XDAP directory */
  vbox = gtk_vbox_new (FALSE, 4);
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), vbox, NULL);

  /* The video devices related options */
  table = gnomemeeting_vbox_add_table (vbox, _("Video Devices"), 6, 4);

  /* The video device */
  pw->video_device =
    gnomemeeting_table_add_pstring_option_menu (table, _("Video device:"), gw->video_devices, DEVICES_KEY "video_recorder", _("Select the video device to use. Using an invalid video device or \"Picture\" for video transmission will transmit a test picture."), 0);

  /* Video Channel */
  pw->video_channel =
    gnomemeeting_table_add_spin (table, _("Video channel:"), DEVICES_KEY "video_channel", _("The video channel number to use (to select camera, tv or other sources)."), 0.0, 10.0, 1.0, 3);
  
  pw->opt1 =
    gnomemeeting_table_add_int_option_menu (table, _("Video size:"), video_size, DEVICES_KEY "video_size", _("Select the transmitted video size: Small (QCIF 176x144) or Large (CIF 352x288)."), 1);

  pw->opt2 =
    gnomemeeting_table_add_int_option_menu (table, _("Video format:"), video_format, DEVICES_KEY "video_format", _("Select the format for video cameras. (Does not apply to most USB cameras)."), 2);

  pw->video_image =
    gnomemeeting_table_add_entry (table, _("Video image:"), DEVICES_KEY "video_image", _("The image to transmit if \"Picture\" is selected for the video device or if the opening of the device fails. Leave blank to use the default GnomeMeeting logo."), 4);


  /* The file selector button */
  button = gtk_button_new_with_label (_("Choose a picture"));
  gtk_table_attach (GTK_TABLE (table), button, 2, 3, 4, 5,
                    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
                    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
                    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);

  g_signal_connect (G_OBJECT (button), "clicked",
		    G_CALLBACK (video_image_browse_clicked),
		    NULL);


  /* That button will refresh the devices list */
  gnomemeeting_pref_window_add_update_button (table, GTK_STOCK_REFRESH, _("_Detect devices"), GTK_SIGNAL_FUNC (refresh_devices), _("Click here to refresh the devices list."), 5, 2);
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
  GmPrefWindow *pw = MyApp->GetPrefWindow ();


  /* Packing widgets */
  vbox =  gtk_vbox_new (FALSE, 4);
  gtk_notebook_append_page (GTK_NOTEBOOK(notebook), vbox, NULL);  
  table =
    gnomemeeting_vbox_add_table (vbox, _("Available Audio Codecs"), 8, 2);

  pw->codecs_list_store = gtk_list_store_new (COLUMN_CODEC_NUMBER,
					      G_TYPE_BOOLEAN,
					      G_TYPE_STRING,
					      G_TYPE_STRING,
					      G_TYPE_STRING,
					      G_TYPE_BOOLEAN,
					      G_TYPE_STRING);

  tree_view = 
    gtk_tree_view_new_with_model (GTK_TREE_MODEL (pw->codecs_list_store));
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (tree_view), TRUE);
  gtk_tree_view_set_search_column (GTK_TREE_VIEW (tree_view),0);
  
  frame = gtk_frame_new (NULL);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 
				  2 * GNOMEMEETING_PAD_SMALL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_container_add (GTK_CONTAINER (frame), tree_view);
  gtk_container_set_border_width (GTK_CONTAINER (tree_view), 0);


  /* Set all Colums */
  renderer = gtk_cell_renderer_toggle_new ();
  column = gtk_tree_view_column_new_with_attributes (_("A"),
						     renderer,
						     "active", 
						     COLUMN_CODEC_ACTIVE,
						     NULL);
  gtk_tree_view_column_add_attribute (column, renderer, "activatable", 
				      COLUMN_CODEC_SELECTABLE);
  gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 25);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
  g_signal_connect (G_OBJECT (renderer), "toggled",
		    G_CALLBACK (codecs_list_fixed_toggled), 
		    GTK_TREE_MODEL (pw->codecs_list_store));

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Name"),
						     renderer,
						     "text", 
						     COLUMN_CODEC_NAME,
						     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
  gtk_tree_view_column_add_attribute (column, renderer, "foreground", 
				      COLUMN_CODEC_COLOR);
  g_object_set (G_OBJECT (renderer), "weight", "bold", NULL);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Info"),
						     renderer,
						     "text", 
						     COLUMN_CODEC_INFO,
						     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
  gtk_tree_view_column_add_attribute (column, renderer, "foreground", 
				      COLUMN_CODEC_COLOR);
  g_object_set (G_OBJECT (renderer), "style", PANGO_STYLE_ITALIC, NULL);


  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Bandwidth"),
						     renderer,
						     "text", 
						     COLUMN_CODEC_BANDWIDTH,
						     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
  gtk_tree_view_column_add_attribute (column, renderer, "foreground", 
				      COLUMN_CODEC_COLOR);

  g_object_set_data (G_OBJECT (pw->codecs_list_store), "tree_view",
		     (gpointer) tree_view);


  /* Here we add the codec but in the order they are in the config file */
  gtk_table_attach (GTK_TABLE (table), frame, 0, 1, 0, 8,        
                    (GtkAttachOptions) (NULL),
                    (GtkAttachOptions) (NULL),
                    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);

  gnomemeeting_codecs_list_build (pw->codecs_list_store);

  button1 = gtk_button_new_from_stock (GTK_STOCK_GO_UP);
  gtk_table_attach (GTK_TABLE (table),  button1, 1, 2, 3, 4,
                    (GtkAttachOptions) NULL,                           
                    (GtkAttachOptions) NULL,        
                    5 * GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
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
                    5 * GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);  
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
  table = gnomemeeting_vbox_add_table (vbox, _("Audio Codecs Settings"), 2, 1);

  /* Translators: the full sentence is Automatically adjust jitter buffer
     between X and Y ms */
  gnomemeeting_table_add_spin_range (table,
				     _("Automatically adjust _jitter buffer between"),
				     &pw->min_jitter_buffer,
				     _("and"),
				     &pw->max_jitter_buffer,
				     _("ms"),
				     AUDIO_SETTINGS_KEY "min_jitter_buffer",
				     AUDIO_SETTINGS_KEY "max_jitter_buffer",
				     _("The minimum jitter buffer size for audio reception (in ms)."),
				     _("The maximum jitter buffer size for audio reception (in ms)."),
				     20.0, 20.0, 1000.0, 1000.0, 1.0, 0);
  
  pw->sd = 
    gnomemeeting_table_add_toggle (table, _("Enable silence _detection"), AUDIO_SETTINGS_KEY "sd", _("If enabled, use silence detection with the GSM and G.711 codecs."), 1);
}
                                                                               

/* BEHAVIOR     :  It builds the notebook page for video codecs settings.
 *                 it adds it to the notebook.
 * PRE          :  The notebook.
 */
static void
gnomemeeting_init_pref_window_video_codecs (GtkWidget *notebook)
{
  GtkWidget *vbox = NULL;
  GtkWidget *table = NULL;


  /* Get the data */
  GmPrefWindow *pw = MyApp->GetPrefWindow ();


  /* Packing widgets for the XDAP directory */ 
  vbox =  gtk_vbox_new (FALSE, 4);
  gtk_notebook_append_page (GTK_NOTEBOOK(notebook), vbox, NULL);
  table = gnomemeeting_vbox_add_table (vbox, _("General Settings"), 2, 1);

  
  /* Add fields */
  pw->vid_tr = 
    gnomemeeting_table_add_toggle (table, _("Enable video _transmission"), VIDEO_SETTINGS_KEY "enable_video_transmission", _("If enabled, video is transmitted during a call."), 0);

  pw->vid_re = 
    gnomemeeting_table_add_toggle (table, _("Enable video _reception"), VIDEO_SETTINGS_KEY "enable_video_reception", _("If enabled, allows video to be received during a call."), 1);


  /* H.261 Settings */
  table = gnomemeeting_vbox_add_table (vbox, _("Bandwidth Control"), 1, 1);

  /* Translators: the full sentence is Maximum video bandwidth of X KB/s */
  pw->maximum_video_bandwidth =
    gnomemeeting_table_add_spin (table, _("Maximum video _bandwidth of"), VIDEO_SETTINGS_KEY "maximum_video_bandwidth", _("The maximum video bandwidth in kbytes/s. The video quality and the number of transmitted frames per second will be dynamically adjusted above their minimum during calls to try to minimize the bandwidth to the given value."), 2.0, 100.0, 1.0, 0, _("KB/s"), true);
  
  
  table =
    gnomemeeting_vbox_add_table (vbox, _("Advanced Quality Settings"), 3, 1);
  gtk_table_set_row_spacings (GTK_TABLE (table), 2 * GNOMEMEETING_PAD_SMALL);
  
  /* Translators: the full sentence is Keep a minimum video quality of X % */
  pw->tr_vq =
    gnomemeeting_table_add_spin (table, _("Keep a minimum video _quality of"), VIDEO_SETTINGS_KEY "tr_vq", _("The minimum transmitted video quality to keep when trying to minimize the used bandwidth:  choose 100% on a LAN for the best quality, 1% being the worst quality."), 1.0, 100.0, 1.0, 0, _("%"), true);

  /* Translators: the full sentence is Transmit at least X frames per second */
  pw->tr_fps =
    gnomemeeting_table_add_spin (table, _("Transmit at least"), VIDEO_SETTINGS_KEY "tr_fps", _("The minimum number of video frames to transmit each second when trying to minimize the bandwidth."), 1.0, 30.0, 1.0, 1, _("_frames per second"), true);
				 
  /* Translators: the full sentence is Transmit X background blocks with each
     frame */
  pw->tr_ub =
    gnomemeeting_table_add_spin (table, _("Transmit"), VIDEO_SETTINGS_KEY "tr_ub", _("Choose the number of blocks (that have not changed) transmitted with each frame. These blocks fill in the background."), 1.0, 99.0, 1.0, 2, _("background _blocks with each frame"), true);
}


GtkWidget *gnomemeeting_pref_window_new (GmPrefWindow *pw)
{
  GtkTreeSelection *selection = NULL;
  GtkCellRenderer *cell = NULL;
  GtkWidget *tree_view = NULL;
  GtkTreeViewColumn *column = NULL;
  GtkTreeStore *model = NULL;
  GtkTreeIter iter;
  GtkTreeIter child_iter;

  GtkWidget *window = NULL;
  GtkWidget *event_box = NULL, *hbox = NULL, *vbox = NULL;
  GtkWidget *frame = NULL;
  GtkWidget *pixmap = NULL;
  GtkWidget *label = NULL;
  GtkWidget *hsep = NULL;

  PangoAttrList *attrs = NULL; 
  PangoAttribute *attr = NULL; 

  /* The notebook on the right */
  GtkWidget *notebook = NULL;

  /* Box inside the prefs window */
  GtkWidget *dialog_vbox = NULL;
 
  window = gtk_dialog_new ();
  gtk_dialog_add_button (GTK_DIALOG (window), GTK_STOCK_CLOSE, 0);

  g_signal_connect (G_OBJECT (window), "response",
 		    G_CALLBACK (pref_window_clicked_callback), 
 		    (gpointer) pw);

  g_signal_connect (G_OBJECT (window), "delete_event",
 		    G_CALLBACK (pref_window_destroy_callback), 
 		    (gpointer) pw);

  gtk_window_set_title (GTK_WINDOW (window), _("GnomeMeeting Settings"));


  /* Construct the window */
  notebook = gtk_notebook_new ();

  dialog_vbox = GTK_DIALOG (window)->vbox;

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
  gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_box_pack_start (GTK_BOX (hbox), frame, TRUE, TRUE, 0);
  gtk_widget_show (frame);
  
  vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_widget_show (vbox);
  
  
  label = gtk_label_new (NULL);
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 4);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_container_add (GTK_CONTAINER (frame), label);
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
  attrs = pango_attr_list_new ();
  attr = pango_attr_scale_new (PANGO_SCALE_LARGE);
  attr->start_index = 0;
  attr->end_index = G_MAXUINT; 
  pango_attr_list_insert (attrs, attr); 
  attr = pango_attr_weight_new (PANGO_WEIGHT_HEAVY);
  attr->start_index = 0;
  attr->end_index = G_MAXUINT;
  pango_attr_list_insert (attrs, attr);
  gtk_label_set_attributes (GTK_LABEL (label), attrs);
  pango_attr_list_unref (attrs);
  gtk_widget_show (label);

  hsep = gtk_hseparator_new ();
  gtk_box_pack_start (GTK_BOX (vbox), hsep, FALSE, FALSE, 0); 
  gtk_box_pack_start (GTK_BOX (vbox), notebook, FALSE, FALSE, 0);

  g_object_set_data (G_OBJECT (window), "section_label", label);


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
		      &child_iter, 0, _("Directory Settings"), 1, 3, -1);
  gnomemeeting_init_pref_window_directories (notebook);

  gtk_tree_store_append (GTK_TREE_STORE (model), &child_iter, &iter);
  gtk_tree_store_set (GTK_TREE_STORE (model),
		      &child_iter, 0, _("Call Forwarding"), 1, 4, -1);
  gnomemeeting_init_pref_window_call_forwarding (notebook);


  /* H.323 Settings */
  gtk_tree_store_append (GTK_TREE_STORE (model), &iter, NULL);
  gtk_tree_store_set (GTK_TREE_STORE (model),
		      &iter, 0, _("H.323 Settings"), 1, 0, -1);
  
  gtk_tree_store_append (GTK_TREE_STORE (model), &child_iter, &iter);
  gtk_tree_store_set (GTK_TREE_STORE (model),
		      &child_iter, 0, _("Advanced Settings"), 1, 5, -1);
  gnomemeeting_init_pref_window_h323_advanced (notebook);          

  gtk_tree_store_append (GTK_TREE_STORE (model), &child_iter, &iter);
  gtk_tree_store_set (GTK_TREE_STORE (model),
		      &child_iter, 0, _("Gatekeeper Settings"), 1, 6, -1);
  gnomemeeting_init_pref_window_gatekeeper (notebook);          

  gtk_tree_store_append (GTK_TREE_STORE (model), &child_iter, &iter);
  gtk_tree_store_set (GTK_TREE_STORE (model),
		      &child_iter, 0, _("NAT Settings"), 1, 7, -1);
  gnomemeeting_init_pref_window_nat (notebook);          

  
  /* Another section */
  gtk_tree_store_append (GTK_TREE_STORE (model), &iter, NULL);
  gtk_tree_store_set (GTK_TREE_STORE (model),
		      &iter, 0, _("Codecs"), 1, 0, -1);

  gtk_tree_store_append (GTK_TREE_STORE (model), &child_iter, &iter);
  gtk_tree_store_set (GTK_TREE_STORE (model),
		      &child_iter, 0, _("Audio Codecs"), 1, 8, -1);
  gnomemeeting_init_pref_window_audio_codecs (notebook);


  gtk_tree_store_append (GTK_TREE_STORE (model), &child_iter, &iter);
  gtk_tree_store_set (GTK_TREE_STORE (model),
		      &child_iter, 0, _("Video Codecs"), 1, 9, -1);
  gnomemeeting_init_pref_window_video_codecs (notebook);


  /* Another section */
  gtk_tree_store_append (GTK_TREE_STORE (model), &iter, NULL);
  gtk_tree_store_set (GTK_TREE_STORE (model),
		      &iter, 0, _("Devices"), 1, 0, -1);

  gtk_tree_store_append (GTK_TREE_STORE (model), &child_iter, &iter);
  gtk_tree_store_set (GTK_TREE_STORE (model),
		      &child_iter, 0, _("Audio Devices"), 1, 10, -1);
  gnomemeeting_init_pref_window_audio_devices (notebook);

  gtk_tree_store_append (GTK_TREE_STORE (model), &child_iter, &iter);
  gtk_tree_store_set (GTK_TREE_STORE (model),
		      &child_iter, 0, _("Video Devices"), 1, 11, -1);
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

  return window;
}

