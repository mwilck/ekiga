
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
#include "codec_info.h"

#include "dialog.h"
#include "gnome_prefs_window.h"


/* Declarations */

extern GtkWidget *gm;


static void personal_data_update_button_clicked (GtkWidget *,
						 gpointer);

static void gatekeeper_update_button_clicked (GtkWidget *,
					      gpointer);

static void codecs_list_button_clicked_callback (GtkWidget *,
						 gpointer);

static void codecs_list_info_button_clicked_callback (GtkWidget *,
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
							      gfloat);

static void codecs_list_fixed_toggled (GtkCellRendererToggle *,
				       gchar *, 
				       gpointer);

static void video_image_browse_clicked (GtkWidget *,
					gpointer);

static void file_selector_clicked (GtkWidget *,
				   gpointer);

static void gnomemeeting_init_pref_window_general (GtkWidget *,
						   GtkWidget *);

static void gnomemeeting_init_pref_window_interface (GtkWidget *,
						     GtkWidget *);

static void gnomemeeting_init_pref_window_directories (GtkWidget *,
						       GtkWidget *);

static void gnomemeeting_init_pref_window_call_forwarding (GtkWidget *,
							   GtkWidget *);

static void gnomemeeting_init_pref_window_h323_advanced (GtkWidget *,
							 GtkWidget *);

static void gnomemeeting_init_pref_window_gatekeeper (GtkWidget *,
						      GtkWidget *);

static void gnomemeeting_init_pref_window_nat (GtkWidget *,
					       GtkWidget *);

static void gnomemeeting_init_pref_window_video_devices (GtkWidget *,
							 GtkWidget *);

static void gnomemeeting_init_pref_window_audio_devices (GtkWidget *,
							 GtkWidget *);

static void gnomemeeting_init_pref_window_audio_codecs (GtkWidget *,
							GtkWidget *);

static void gnomemeeting_init_pref_window_video_codecs (GtkWidget *,
							GtkWidget *);


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

  endpoint = GnomeMeeting::Process ()->Endpoint ();
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
  GnomeMeeting::Process ()->Endpoint ()->GatekeeperRegister ();
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
			   AUDIO_CODECS_KEY "codecs_list", 
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
			 AUDIO_CODECS_KEY "codecs_list", 
			 GCONF_VALUE_STRING, codecs_data, NULL);

  
  g_slist_free (codecs_data);
}


/* DESCRIPTION  :  This callback is called when the user clicks
 *                 on the info button in the Audio Codecs Settings.
 * BEHAVIOR     :  Displays an information popup about the codec.
 * PRE          :  /
 */
static void codecs_list_info_button_clicked_callback (GtkWidget *widget, 
						      gpointer data)
{ 	
  PString info;
  GMH323CodecInfo codec;

  gchar *selected_codec_name = NULL;

  GmWindow *gw = NULL;

  GtkTreeIter iter;
  GtkTreeView *tree_view = NULL;
  GtkTreeSelection *selection = NULL;


  gw = GnomeMeeting::Process ()->GetMainWindow ();

  /* Get the current selected codec name, there is always one */
  tree_view = GTK_TREE_VIEW (g_object_get_data (G_OBJECT (data), "tree_view"));
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
  
  gtk_tree_selection_get_selected (GTK_TREE_SELECTION (selection), NULL,
				   &iter);
  gtk_tree_model_get (GTK_TREE_MODEL (data), &iter,
		      COLUMN_CODEC_NAME, &selected_codec_name, -1);

  if (selected_codec_name) {

    codec = GMH323CodecInfo (selected_codec_name);
    gnomemeeting_message_dialog (GTK_WINDOW (gw->pref_window), 
				 _("Codec Information"), 
				 codec.GetCodecInfo ());

    g_free (selected_codec_name);
  }
}


/* DESCRIPTION  :  This callback is called when the user clicks
 *                 on a button of the file selector.
 * BEHAVIOR     :  It sets the selected filename in the video_image gconf key.
 * PRE          :  data = the file selector.
 */
static void  
file_selector_clicked (GtkWidget *b, gpointer data) 
{
  GConfClient *client = NULL;
  gchar *filename = NULL;

  client = gconf_client_get_default ();
  
  filename = (gchar *)
    gtk_file_selection_get_filename (GTK_FILE_SELECTION (data));
  
  gconf_client_set_string (client, DEVICES_KEY "video_image",
			   filename, NULL);
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
  GMH323CodecInfo codec;

  PString codec_quality;
  PString codec_bitrate;

  if (!codec_name || !store || !color)
    return;

  codec = GMH323CodecInfo (codec_name);
  codec_quality = codec.GetCodecQuality ();
  codec_bitrate = codec.GetCodecBitRate ();
    
  if (!codec_quality.IsEmpty () && !codec_bitrate.IsEmpty ()) {

    gtk_list_store_append (store, &iter);
    gtk_list_store_set (store, &iter,
			COLUMN_CODEC_ACTIVE, enabled,
			COLUMN_CODEC_NAME, codec_name,
			COLUMN_CODEC_INFO, (const char *) codec_quality,
			COLUMN_CODEC_BANDWIDTH, (const char *) codec_bitrate,
			COLUMN_CODEC_SELECTABLE, possible,
			COLUMN_CODEC_COLOR, color,
			-1);
  }
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
			   AUDIO_CODECS_KEY "codecs_list", 
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
  
  gconf_client_set_list (client, AUDIO_CODECS_KEY "codecs_list",
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

  GdkEvent *event = NULL;

  int selected_row = 0;
  int current_row = 0;

  gchar *cselect_row = NULL;
  gchar *selected_codec = NULL;
  gchar *quicknet = NULL;
    
  GMH323CodecInfo cdec;

  PString dev;
  PString codec;
    
  GSList *codecs_data = NULL;
  GConfClient *client = gconf_client_get_default ();

  codecs_data = 
    gconf_client_get_list (client, 
			   AUDIO_CODECS_KEY "codecs_list", 
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


  /* Select the right row, and disable if needed the properties button */
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

  event = gdk_event_new (GDK_BUTTON_PRESS);
  g_signal_emit_by_name (G_OBJECT (tree_view), "event-after", event);
  gdk_event_free (event);
}
                                                                  
                                                                               
static GtkWidget *
gnomemeeting_pref_window_add_update_button (GtkWidget *box,
					    const char *stock_id,
					    const char *label,
					    GtkSignalFunc func,
					    gchar *tooltip,
					    gfloat valign)  
{
  GtkWidget *alignment = NULL;
  GtkWidget *image = NULL;
  GtkWidget *button = NULL;                                                    
  GmPrefWindow *pw = NULL;                                           

  
  pw = GnomeMeeting::Process ()->GetPrefWindow ();                                      

  image = gtk_image_new_from_stock (stock_id, GTK_ICON_SIZE_BUTTON);
  button = gnomemeeting_button_new (label, image);

  alignment = gtk_alignment_new (1, valign, 0, 0);
  gtk_container_add (GTK_CONTAINER (alignment), button);
  gtk_container_set_border_width (GTK_CONTAINER (button), 6);

  gtk_box_pack_start (GTK_BOX (box), alignment, TRUE, TRUE, 0);
                                                                               
  g_signal_connect (G_OBJECT (button), "clicked",                          
		    G_CALLBACK (func), (gpointer) pw);

  
  return button;                                                               
}                                                                              
                                                                               
                                                                               
/* BEHAVIOR     :  It builds the container for general settings and
 *                 returns it.
 * PRE          :  /
 */
static void
gnomemeeting_init_pref_window_general (GtkWidget *window,
				       GtkWidget *container)
{
  GtkWidget *subsection = NULL;
  GtkWidget *entry = NULL;

  subsection = gnome_prefs_subsection_new (window, container,
					   _("Personal Information"), 4, 2);
  
  /* Add all the fields */
  entry =
    gnome_prefs_entry_new (subsection, _("_First name:"),
			   PERSONAL_DATA_KEY "firstname",
			   _("Enter your first name."), 0, false);
  gtk_widget_set_size_request (GTK_WIDGET (entry), 250, -1);
  gtk_entry_set_max_length (GTK_ENTRY (entry), 65);

  entry =
    gnome_prefs_entry_new (subsection, _("Sur_name:"),
			   PERSONAL_DATA_KEY "lastname",
			   _("Enter your last name."), 1, false);
  gtk_widget_set_size_request (GTK_WIDGET (entry), 250, -1);
  gtk_entry_set_max_length (GTK_ENTRY (entry), 65);

  entry =
    gnome_prefs_entry_new (subsection, _("E-_mail address:"),
			   PERSONAL_DATA_KEY "mail",
			   _("Enter your e-mail address."), 2, false);
  gtk_widget_set_size_request (GTK_WIDGET (entry), 250, -1);
  gtk_entry_set_max_length (GTK_ENTRY (entry), 65);

  entry =
    gnome_prefs_entry_new (subsection, _("_Comment:"),
			   PERSONAL_DATA_KEY "comment",
			   _("Enter a comment about yourself."), 3, false);
  gtk_widget_set_size_request (GTK_WIDGET (entry), 250, -1);
  gtk_entry_set_max_length (GTK_ENTRY (entry), 65);

  entry =
    gnome_prefs_entry_new (subsection, _("_Location:"),
			   PERSONAL_DATA_KEY "location",
			   _("Enter your country or city."), 4, false);
  gtk_widget_set_size_request (GTK_WIDGET (entry), 250, -1);
  gtk_entry_set_max_length (GTK_ENTRY (entry), 65);

  
  /* Add the update button */
  gnomemeeting_pref_window_add_update_button (container, GTK_STOCK_APPLY, _("_Apply"), GTK_SIGNAL_FUNC (personal_data_update_button_clicked), _("Click here to update the user directory you are registered to with the new First Name, Last Name, E-Mail, Comment and Location or to update your alias on the Gatekeeper."), 0);
}                                                                              
                                                                               

/* BEHAVIOR     :  It builds the container for interface settings
 *                 add returns it.
 * PRE          :  /
 */
static void
gnomemeeting_init_pref_window_interface (GtkWidget *window,
					 GtkWidget *container)
{
  GtkWidget *subsection = NULL;

  
  /* GnomeMeeting GUI */
  subsection =
    gnome_prefs_subsection_new (window, container,
				_("GnomeMeeting GUI"), 2, 2);

  gnome_prefs_toggle_new (subsection, _("_Show splash screen"), VIEW_KEY "show_splash", _("If enabled, the splash screen will be displayed when GnomeMeeting starts."), 0);

  gnome_prefs_toggle_new (subsection, _("Start _hidden"), VIEW_KEY "start_docked", _("If enabled, GnomeMeeting will start hidden. The notification area must be present in the panel."), 1);

  
  /* Packing widget */
  subsection =
    gnome_prefs_subsection_new (window, container, _("Video Display"), 2, 1);

#ifdef HAS_SDL
  /* Translators: the full sentence is Use a fullscreen size 
     of X by Y pixels */
  gnome_prefs_range_new (subsection, _("Use a fullscreen size of"), NULL, _("by"), NULL, _("pixels"), VIDEO_DISPLAY_KEY "fullscreen_width", VIDEO_DISPLAY_KEY "fullscreen_height", _("The image width for fullscreen."), _("The image height for fullscreen."), 10.0, 10.0, 640.0, 480.0, 10.0, 0);
#endif
  
  gnome_prefs_toggle_new (subsection, _("Place windows displaying video _above other windows"), VIDEO_DISPLAY_KEY "stay_on_top", _("Place windows displaying video above other windows during calls."), 2);

  /* Text Chat */
  subsection =
    gnome_prefs_subsection_new (window, container, _("Text Chat"), 1, 1);
  
  gnome_prefs_toggle_new (subsection, _("Automatically clear the text chat at the end of calls"), GENERAL_KEY "auto_clear_text_chat", _("If enabled, the text chat will automatically be cleared at the end of calls."), 0);
}


/* BEHAVIOR     :  It builds the container for XDAP directories,
 *                 and returns it.
 * PRE          :  /
 */
static void
gnomemeeting_init_pref_window_directories (GtkWidget *window,
					   GtkWidget *container)
{
  GtkWidget *subsection = NULL;


  /* Packing widgets for the XDAP directory */
  
  subsection = gnome_prefs_subsection_new (window, container,
					   _("User Directory"), 3, 2);


  /* Add all the fields */                                                     
  gnome_prefs_entry_new (subsection, _("User directory:"), LDAP_KEY "ldap_server", _("The user directory server to register with."), 0, true);

  gnome_prefs_toggle_new (subsection, _("Enable _registering"), LDAP_KEY "register", _("If enabled, register with the selected user directory."), 1);

  gnome_prefs_toggle_new (subsection, _("_Publish my details in the users directory when registering"), LDAP_KEY "visible", _("If enabled, your details are shown to people browsing the user directory. If disabled, you are not visible to users browsing the user directory, but they can still use the callto URL to call you."), 2);
}


/* BEHAVIOR     :  It builds the container for call forwarding,
 *                 and returns it.
 * PRE          :  /                                             
 */                                                                            
static void
gnomemeeting_init_pref_window_call_forwarding (GtkWidget *window,
					       GtkWidget *container)
{
  GtkWidget *entry = NULL;
  GtkWidget *subsection = NULL;

  
  subsection = gnome_prefs_subsection_new (window, container,
					   _("Call Forwarding"), 4, 2);


  /* Add all the fields */                                                     
  entry =
    gnome_prefs_entry_new (subsection, _("Forward calls to _host:"), CALL_FORWARDING_KEY "forward_host", _("The host where calls should be forwarded to in the cases selected above."), 0, true);
  if (!strcmp (gtk_entry_get_text (GTK_ENTRY (entry)), ""))
    gtk_entry_set_text (GTK_ENTRY (entry), GMURL ().GetDefaultURL ());
  gtk_widget_set_size_request (GTK_WIDGET (entry), 250, -1);  
  
  gnome_prefs_toggle_new (subsection, _("_Always forward calls to the given host"), CALL_FORWARDING_KEY "always_forward", _("If enabled, all incoming calls will be forwarded to the host that is specified in the field above."), 1);

  gnome_prefs_toggle_new (subsection, _("Forward calls to the given host if _no answer"), CALL_FORWARDING_KEY "no_answer_forward", _("If enabled, all incoming calls will be forwarded to the host that is specified in the field above if you do not answer the call."), 2);

  gnome_prefs_toggle_new (subsection, _("Forward calls to the given host if _busy"), CALL_FORWARDING_KEY "busy_forward", _("If enabled, all incoming calls will be forwarded to the host that is specified in the field above if you already are in a call or if you are in Do Not Disturb mode."), 3);
}


/* BEHAVIOR     :  It builds the container for the H.323 advanced settings
 *                 and returns it.
 * PRE          :  /
 */
static void
gnomemeeting_init_pref_window_h323_advanced (GtkWidget *window,
					     GtkWidget *container)
{
  GmPrefWindow *pw = NULL;
  
  GtkWidget *subsection = NULL;

  gchar *capabilities [] = {_("All"),
			    _("None"),
			    _("rfc2833"),
			    _("Signal"),
			    _("String"),
			    NULL};

  pw = GnomeMeeting::Process ()->GetPrefWindow ();

  
  /* Packing widget */
  subsection =
    gnome_prefs_subsection_new (window, container,
				_("H.323 Version 2 Settings"), 2, 1);

  /* The toggles */
  pw->ht =
    gnome_prefs_toggle_new (subsection, _("Enable H.245 _tunnelling"), GENERAL_KEY "h245_tunneling", _("This enables H.245 Tunnelling mode. In H.245 Tunnelling mode H.245 messages are encapsulated into the the H.225 channel (port 1720). This saves one TCP connection during calls. H.245 Tunnelling was introduced in H.323v2 and Netmeeting does not support it. Using both Fast Start and H.245 Tunnelling can crash some versions of Netmeeting."), 0);

  pw->fs =
    gnome_prefs_toggle_new (subsection, _("Enable fast _start procedure"), GENERAL_KEY "fast_start", _("Connection will be established in Fast Start mode. Fast Start is a new way to start calls faster that was introduced in H.323v2. It is not supported by Netmeeting and using both Fast Start and H.245 Tunnelling can crash some versions of Netmeeting."), 1);

  
  /* Packing widget */                                                         
  subsection =
    gnome_prefs_subsection_new (window, container,
				_("User Input Capabilities"), 1, 1);

  pw->uic =
    gnome_prefs_int_option_menu_new (subsection, _("T_ype:"), capabilities, GENERAL_KEY "user_input_capability", _("This permits to set the mode for User Input Capabilities. The values can be \"All\", \"None\", \"rfc2833\", \"Signal\" or \"String\" (default is \"All\"). Choosing other values than \"All\", \"String\" or \"rfc2833\" disables the Text Chat."), 0);
}                               


/* BEHAVIOR     :  It builds the container for the gatekeeper settings
 *                 and returns it.
 * PRE          :  /
 */
static void
gnomemeeting_init_pref_window_gatekeeper (GtkWidget *window,
					  GtkWidget *container)
{
  GtkWidget *entry = NULL;
  GtkWidget *subsection = NULL;

  gchar *options [] = {_("Do not register"), 
		       _("Gatekeeper host"), 
		       _("Gatekeeper ID"), 
		       _("Automatically discover"), 
		       NULL};

  
  /* Add fields for the gatekeeper */
  subsection = gnome_prefs_subsection_new (window, container,
					   _("Gatekeeper"), 5, 3);

  gnome_prefs_entry_new (subsection, _("Gatekeeper _ID:"), GATEKEEPER_KEY "gk_id", _("The Gatekeeper identifier to register with."), 1, false);

  gnome_prefs_entry_new (subsection, _("Gatekeeper _host:"), GATEKEEPER_KEY "gk_host", _("The Gatekeeper host to register with."), 2, false);

  gnome_prefs_entry_new (subsection, _("Gatekeeper _alias:"), GATEKEEPER_KEY "gk_alias", _("The Gatekeeper alias to use when registering (string, or E164 ID if only 0123456789#)."), 3, false);

  entry =
    gnome_prefs_entry_new (subsection, _("Gatekeeper _password:"), GATEKEEPER_KEY "gk_password", _("The Gatekeeper password to use for H.235 authentication to the Gatekeeper."), 4, false);
  gtk_entry_set_visibility (GTK_ENTRY (entry), FALSE);

  gnome_prefs_int_option_menu_new (subsection, _("Registering method:"), options, GATEKEEPER_KEY "registering_method", _("Registering method to use"), 0);

  /* Translators: the full sentence is Registration Time-To-Live of X s */
  gnome_prefs_spin_new (subsection, _("Registration Time-To-Live of "), GATEKEEPER_KEY "time_to_live", _("The gatekeeper TTL for the registration, expressed in seconds."), 60.0, 3600.0, 1.0, 5, _("s"), true);
  
  gnomemeeting_pref_window_add_update_button (container, GTK_STOCK_APPLY, _("_Apply"), GTK_SIGNAL_FUNC (gatekeeper_update_button_clicked), _("Click here to update your Gatekeeper settings."), 0);
}


/* BEHAVIOR     :  It builds the container for NAT support
 *                 and returns it.
 * PRE          :  /
 */
static void
gnomemeeting_init_pref_window_nat (GtkWidget *window,
				   GtkWidget *container)
{
  GtkWidget *subsection = NULL;


  /* IP translation */
  subsection =
    gnome_prefs_subsection_new (window, container,
				_("NAT/PAT Router Support"), 2, 1);

  gnome_prefs_toggle_new (subsection, _("Enable IP _translation"), NAT_KEY "ip_translation", _("This enables IP translation. IP translation is useful if GnomeMeeting is running behind a NAT/PAT router. You have to put the public IP of the router in the field below. If you are registered to ils.seconix.com, GnomeMeeting will automatically fetch the public IP using the ILS service. If your router natively supports H.323, you can disable this."), 1);

  gnome_prefs_entry_new (subsection, _("Public _IP of the NAT/PAT router:"), NAT_KEY "public_ip", _("Enter the public IP of your NAT/PAT router if you want to use IP translation. If you are registered to ils.seconix.com, GnomeMeeting will automatically fetch the public IP using the ILS service."), 2, false);
}


/* BEHAVIOR     :  It builds the container for the audio devices 
 *                 settings and returns it.
 * PRE          :  /
 */
static void
gnomemeeting_init_pref_window_audio_devices (GtkWidget *window,
					     GtkWidget *container)
{
  GmWindow *gw = NULL;
  GmPrefWindow *pw = NULL;
  
  GtkWidget *entry = NULL;
  GtkWidget *subsection = NULL;

  gchar **array = NULL;
  gchar *aec [] = {_("Off"),
		   _("Low"),
		   _("Medium"),
		   _("High"),
		   _("AGC"),
		   NULL};


  gw = GnomeMeeting::Process ()->GetMainWindow ();
  pw = GnomeMeeting::Process ()->GetPrefWindow ();
  

#ifdef TRY_PLUGINS
  subsection = gnome_prefs_subsection_new (window, container,
					   _("Audio Manager"), 1, 2);
                                                                               
  /* Add all the fields for the audio manager */
  array = gw->audio_managers.ToCharArray ();
  gnome_prefs_string_option_menu_new (subsection, _("Audio manager:"), array, DEVICES_KEY "audio_manager", _("The audio manager that will be used to detect the devices and manage them."), 0);
  free (array);
#endif


  /* Add all the fields */
  subsection = gnome_prefs_subsection_new (window, container,
					   _("Audio Devices"), 4, 2);
                                                                               

  /* The player */
  array = gw->audio_player_devices.ToCharArray ();
  pw->audio_player =
    gnome_prefs_string_option_menu_new (subsection, _("Playing device:"), array, DEVICES_KEY "audio_player", _("Select the audio player device to use."), 0);
  free (array);
  
  /* The recorder */
  array = gw->audio_recorder_devices.ToCharArray ();
  pw->audio_recorder =
    gnome_prefs_string_option_menu_new (subsection, _("Recording device:"), array, DEVICES_KEY "audio_recorder", _("Select the audio recorder device to use."), 2);
  free (array);

#ifdef HAS_IXJ
  /* The Quicknet devices related options */
  subsection = gnome_prefs_subsection_new (window, container,
					   _("Quicknet Device"), 2, 2);

  gnome_prefs_int_option_menu_new (subsection, _("Echo _cancellation:"), aec, DEVICES_KEY "lid_aec", _("The Automatic Echo Cancellation level: Off, Low, Medium, High, Automatic Gain Compensation. Choosing Automatic Gain Compensation modulates the volume for best quality."), 0);

  entry =
    gnome_prefs_entry_new (subsection, _("Country _code:"), DEVICES_KEY "lid_country", _("The two-letter country code of your country (e.g.: BE, UK, FR, DE, ...)."), 1, false);
  gtk_entry_set_max_length (GTK_ENTRY (entry), 2);
  gtk_widget_set_size_request (GTK_WIDGET (entry), 100, -1);  
#endif

  
  /* That button will refresh the devices list */
  gnomemeeting_pref_window_add_update_button (container, GTK_STOCK_REFRESH, _("_Detect devices"), GTK_SIGNAL_FUNC (gnomemeeting_pref_window_refresh_devices_list), _("Click here to refresh the devices list."), 1);
}


/* BEHAVIOR     :  It builds the container for the video devices 
 *                 settings and returns it.
 * PRE          :  /
 */
static void
gnomemeeting_init_pref_window_video_devices (GtkWidget *window,
					     GtkWidget *container)
{
  GmWindow *gw = NULL;
  GmPrefWindow *pw = NULL;
  
  GtkWidget *entry = NULL;
  GtkWidget *subsection = NULL;

  GtkWidget *button = NULL;

  gchar **array = NULL;
  gchar *video_size [] = {_("Small"),
			  _("Large"), 
			  NULL};
  gchar *video_format [] = {_("PAL (Europe)"), 
			    _("NTSC (America)"), 
			    _("SECAM (France)"), 
			    _("Auto"), 
			    NULL};


  gw = GnomeMeeting::Process ()->GetMainWindow ();
  pw = GnomeMeeting::Process ()->GetPrefWindow ();
  

#ifdef TRY_PLUGINS
  /* The video manager */
  subsection = gnome_prefs_subsection_new (window, container,
					   _("Video Manager"), 1, 2);

  array = gw->video_managers.ToCharArray ();
  gnome_prefs_string_option_menu_new (subsection, _("Video manager:"), array, DEVICES_KEY "video_manager", _("The video manager that will be used to detect the devices and manage them."), 0);
  free (array);
#endif

  /* The video devices related options */
  subsection = gnome_prefs_subsection_new (window, container,
					   _("Video Devices"), 5, 3);

  /* The video device */
  array = gw->video_devices.ToCharArray ();
  pw->video_device =
    gnome_prefs_string_option_menu_new (subsection, _("Video device:"), array, DEVICES_KEY "video_recorder", _("Select the video device to use. Using an invalid video device or \"Picture\" for video transmission will transmit a test picture."), 0);
  free (array);
  
  /* Video Channel */
  gnome_prefs_spin_new (subsection, _("Video channel:"), DEVICES_KEY "video_channel", _("The video channel number to use (to select camera, tv or other sources)."), 0.0, 10.0, 1.0, 3, NULL, false);
  
  gnome_prefs_int_option_menu_new (subsection, _("Video size:"), video_size, DEVICES_KEY "video_size", _("Select the transmitted video size: Small (QCIF 176x144) or Large (CIF 352x288)."), 1);

  gnome_prefs_int_option_menu_new (subsection, _("Video format:"), video_format, DEVICES_KEY "video_format", _("Select the format for video cameras. (Does not apply to most USB cameras)."), 2);

  entry =
    gnome_prefs_entry_new (subsection, _("Video image:"), DEVICES_KEY "video_image", _("The image to transmit if \"Picture\" is selected for the video device or if the opening of the device fails. Leave blank to use the default GnomeMeeting logo."), 4, false);


  /* The file selector button */
  button = gtk_button_new_with_label (_("Choose a picture"));
  gtk_table_attach (GTK_TABLE (subsection), button, 2, 3, 4, 5,
                    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
                    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
                    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);

  g_signal_connect (G_OBJECT (button), "clicked",
		    G_CALLBACK (video_image_browse_clicked),
		    entry);


  /* That button will refresh the devices list */
  gnomemeeting_pref_window_add_update_button (container, GTK_STOCK_REFRESH, _("_Detect devices"), GTK_SIGNAL_FUNC (gnomemeeting_pref_window_refresh_devices_list), _("Click here to refresh the devices list."), 1);
}


/* BEHAVIOR     :  It builds the container for audio codecs settings and
 *                 returns it.
 * PRE          :  /
 */
static void
gnomemeeting_init_pref_window_audio_codecs (GtkWidget *window,
					    GtkWidget *container)
{
  GtkWidget *subsection = NULL;
  
  GtkWidget *alignment = NULL;
  GtkWidget *buttons_vbox = NULL;
  GtkWidget *hbox = NULL;
    
  GtkWidget *button = NULL;
  GtkWidget *frame = NULL;

  GtkWidget *tree_view = NULL;
  GtkTreeViewColumn *column = NULL;
  GtkCellRenderer *renderer = NULL;                        
                                                       

  /* Get the data */
  GmPrefWindow *pw = GnomeMeeting::Process ()->GetPrefWindow ();


  /* Packing widgets */
  subsection =
    gnome_prefs_subsection_new (window, container,
				_("Available Audio Codecs"), 1, 1);

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_table_attach (GTK_TABLE (subsection), hbox, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_SHRINK), 
		    (GtkAttachOptions) (GTK_SHRINK),
                    0, 0);

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
  gtk_box_pack_start (GTK_BOX (hbox), frame, FALSE, FALSE, 0);


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


  /* The buttons */
  alignment = gtk_alignment_new (1, 0.5, 0, 0);
  buttons_vbox = gtk_vbutton_box_new ();
  
  gtk_box_set_spacing (GTK_BOX (buttons_vbox), 2 * GNOMEMEETING_PAD_SMALL);

  gtk_container_add (GTK_CONTAINER (alignment), buttons_vbox);
  gtk_box_pack_start (GTK_BOX (hbox), alignment, 
		      TRUE, TRUE, 2 * GNOMEMEETING_PAD_SMALL);

  button = gtk_button_new_from_stock (GTK_STOCK_GO_UP);
  gtk_box_pack_start (GTK_BOX (buttons_vbox), button, TRUE, TRUE, 0);
  g_object_set_data (G_OBJECT (button), "operation", (gpointer) "up");
  g_signal_connect (G_OBJECT (button), "clicked",
		    G_CALLBACK (codecs_list_button_clicked_callback), 
		    GTK_TREE_MODEL (pw->codecs_list_store));

  button = gtk_button_new_from_stock (GTK_STOCK_GO_DOWN);
  gtk_box_pack_start (GTK_BOX (buttons_vbox), button, TRUE, TRUE, 0);
  g_object_set_data (G_OBJECT (button), "operation", (gpointer) "down");
  g_signal_connect (G_OBJECT (button), "clicked",
		    G_CALLBACK (codecs_list_button_clicked_callback), 
		    GTK_TREE_MODEL (pw->codecs_list_store));

  button = gtk_button_new_from_stock (GTK_STOCK_DIALOG_INFO);
  gtk_box_pack_start (GTK_BOX (buttons_vbox), button, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button), "clicked",
		    G_CALLBACK (codecs_list_info_button_clicked_callback), 
		    GTK_TREE_MODEL (pw->codecs_list_store));  
  gtk_widget_show_all (frame);


  /* Here we finally add the codecs in the order they are in the config */
  gnomemeeting_codecs_list_build (pw->codecs_list_store);


  /* Here we add the audio codecs options */
  subsection = 
    gnome_prefs_subsection_new (window, container,
				_("Audio Codecs Settings"), 2, 1);

  /* Translators: the full sentence is Automatically adjust jitter buffer
     between X and Y ms */
  gnome_prefs_range_new (subsection, _("Automatically adjust _jitter buffer between"), NULL, _("and"), NULL, _("ms"), AUDIO_SETTINGS_KEY "min_jitter_buffer", AUDIO_SETTINGS_KEY "max_jitter_buffer", _("The minimum jitter buffer size for audio reception (in ms)."), _("The maximum jitter buffer size for audio reception (in ms)."), 20.0, 20.0, 1000.0, 1000.0, 1.0, 0);
  
  gnome_prefs_toggle_new (subsection, _("Enable silence _detection"), AUDIO_SETTINGS_KEY "sd", _("If enabled, use silence detection with the GSM and G.711 codecs."), 1);
}
                                                                               

/* BEHAVIOR     :  It builds the container for video codecs settings and
 *                 returns it.
 * PRE          :  /
 */
static void
gnomemeeting_init_pref_window_video_codecs (GtkWidget *window,
					    GtkWidget *container)
{
  GtkWidget *subsection = NULL;
  GmPrefWindow *pw = NULL;

  pw = GnomeMeeting::Process ()->GetPrefWindow ();

  subsection = gnome_prefs_subsection_new (window, container,
					   _("General Settings"), 2, 1);

  
  /* Add fields */
  gnome_prefs_toggle_new (subsection, _("Enable video _transmission"), VIDEO_SETTINGS_KEY "enable_video_transmission", _("If enabled, video is transmitted during a call."), 0);

  pw->vid_re =
    gnome_prefs_toggle_new (subsection, _("Enable video _reception"), VIDEO_SETTINGS_KEY "enable_video_reception", _("If enabled, allows video to be received during a call."), 1);


  /* H.261 Settings */
  subsection = gnome_prefs_subsection_new (window, container,
					   _("Bandwidth Control"), 1, 1);

  /* Translators: the full sentence is Maximum video bandwidth of X KB/s */
  gnome_prefs_spin_new (subsection, _("Maximum video _bandwidth of"), VIDEO_SETTINGS_KEY "maximum_video_bandwidth", _("The maximum video bandwidth in kbytes/s. The video quality and the number of transmitted frames per second will be dynamically adjusted above their minimum during calls to try to minimize the bandwidth to the given value."), 2.0, 100.0, 1.0, 0, _("KB/s"), true);
  

  /* Advanced quality settings */
  subsection =
    gnome_prefs_subsection_new (window, container,
				_("Advanced Quality Settings"), 3, 1);
  
  /* Translators: the full sentence is Keep a minimum video quality of X % */
  gnome_prefs_spin_new (subsection, _("Keep a minimum video _quality of"), VIDEO_SETTINGS_KEY "tr_vq", _("The minimum transmitted video quality to keep when trying to minimize the used bandwidth:  choose 100% on a LAN for the best quality, 1% being the worst quality."), 1.0, 100.0, 1.0, 0, _("%"), true);

  /* Translators: the full sentence is Transmit at least X frames per second */
  gnome_prefs_spin_new (subsection, _("Transmit at least"), VIDEO_SETTINGS_KEY "tr_fps", _("The minimum number of video frames to transmit each second when trying to minimize the bandwidth."), 1.0, 30.0, 1.0, 1, _("_frames per second"), true);
				 
  /* Translators: the full sentence is Transmit X background blocks with each
     frame */
  gnome_prefs_spin_new (subsection, _("Transmit"), VIDEO_SETTINGS_KEY "tr_ub", _("Choose the number of blocks (that have not changed) transmitted with each frame. These blocks fill in the background."), 1.0, 99.0, 1.0, 2, _("background _blocks with each frame"), true);
}


void 
gnomemeeting_pref_window_refresh_devices_list (GtkWidget *widget, 
					       gpointer data)
{
  GmPrefWindow *pw = NULL;
  GmWindow *gw = NULL;

  gchar **array = NULL;
  
  gw = GnomeMeeting::Process ()->GetMainWindow ();
  pw = GnomeMeeting::Process ()->GetPrefWindow ();

  GnomeMeeting::Process ()->DetectDevices ();

  /* The player */
  array = gw->audio_player_devices.ToCharArray ();
  gnome_prefs_string_option_menu_update (pw->audio_player,
					 array,
					 DEVICES_KEY "audio_player");
  free (array);
  
  /* The recorder */
  array = gw->audio_recorder_devices.ToCharArray ();
  gnome_prefs_string_option_menu_update (pw->audio_recorder,
					 array,
					 DEVICES_KEY "audio_recorder");
  free (array);
  
  
  /* The Video player */
  array = gw->video_devices.ToCharArray ();

  gnome_prefs_string_option_menu_update (pw->video_device,
					 array,
					 DEVICES_KEY "video_recorder");
  free (array);
}


GtkWidget *gnomemeeting_pref_window_new (GmPrefWindow *pw)
{
  GtkWidget *window = NULL;
  GtkWidget *container = NULL;
  
  window = 
    gnome_prefs_window_new (GNOMEMEETING_IMAGES "/gnomemeeting-logo.png");
  
  gnome_prefs_window_section_new (window, _("General"));
  container = gnome_prefs_window_subsection_new (window, _("Personal Data"));
  gnomemeeting_init_pref_window_general (window, container);

  container = gnome_prefs_window_subsection_new (window,
						 _("General Settings"));
  gnomemeeting_init_pref_window_interface (window, container);
  
  container = gnome_prefs_window_subsection_new (window,
						 _("Directory Settings"));
  gnomemeeting_init_pref_window_directories (window, container);

  container = gnome_prefs_window_subsection_new (window, _("Call Forwarding"));
  gnomemeeting_init_pref_window_call_forwarding (window, container);
  

  gnome_prefs_window_section_new (window, _("H.323 Settings"));
  container = gnome_prefs_window_subsection_new (window,
						 _("Advanced Settings"));
  gnomemeeting_init_pref_window_h323_advanced (window, container);          

  container = gnome_prefs_window_subsection_new (window,
						 _("Gatekeeper Settings"));
  gnomemeeting_init_pref_window_gatekeeper (window, container);          

  container = gnome_prefs_window_subsection_new (window, _("NAT Settings"));
  gnomemeeting_init_pref_window_nat (window, container);

  
  gnome_prefs_window_section_new (window, _("Codecs"));

  container = gnome_prefs_window_subsection_new (window, _("Audio Codecs"));
  gnomemeeting_init_pref_window_audio_codecs (window, container);

  container = gnome_prefs_window_subsection_new (window, _("Video Codecs"));
  gnomemeeting_init_pref_window_video_codecs (window, container);


  gnome_prefs_window_section_new (window, _("Devices"));
  container = gnome_prefs_window_subsection_new (window, _("Audio Devices"));
  gnomemeeting_init_pref_window_audio_devices (window, container);

  container = gnome_prefs_window_subsection_new (window, _("Video Devices"));
  gnomemeeting_init_pref_window_video_devices (window, container);

  return window;
}

