
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
#include "audio.h"
#include "misc.h"

#include <gconf/gconf-client.h>

#include "../pixmaps/tb_jump_to.xpm"

#define GNOMEMEETING_PAD_SMALL 1

/* Declarations */

extern GtkWidget *gm;
extern GnomeMeeting *MyApp;	

static void pref_window_clicked_callback (GtkDialog *, int, gpointer);
static gint pref_window_destroy_callback (GtkWidget *, gpointer);
static void personal_data_update_button_clicked (GtkWidget *, gpointer);
static void codecs_list_button_clicked_callback (GtkWidget *, gpointer);
static void gnomemeeting_codecs_list_add (GtkTreeIter, GtkListStore *, const gchar *, bool);

static void codecs_list_fixed_toggled (GtkCellRendererToggle *, gchar *, gpointer);
static void menu_ctree_row_seletected_callback (GtkWidget *, gint, gint, 
						GdkEventButton *, gpointer);


static void gnomemeeting_init_pref_window_general (GtkWidget *);
static void gnomemeeting_init_pref_window_interface (GtkWidget *);
static void gnomemeeting_init_pref_window_directories (GtkWidget *);
static void gnomemeeting_init_pref_window_devices (GtkWidget *);
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

/* DESCRIPTION  :  This callback is called when the user clicks on "Close"
 * BEHAVIOR     :  Closes the window.
 * PRE          :  gpointer is a valid pointer to a GM_pref_window_widgets.
 */
static void pref_window_clicked_callback (GtkDialog *widget, int button, 
					  gpointer data)
{
  switch (button) {

  case 0:
  
    gtk_widget_hide (GTK_WIDGET (widget));
    
    break;
  }
}


/* DESCRIPTION  :  This callback is called when the pref window is destroyed.
 * BEHAVIOR     :  Prevents the destroy, only hides the window.
 * PRE          :  /
 */
static gint pref_window_destroy_callback (GtkWidget *widget, gpointer data)
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
  gchar *firstname = NULL;
  gchar *lastname = NULL;
  gchar *local_name = NULL;
  gchar *alias_ = NULL;
  PString alias;
  GConfClient *client = gconf_client_get_default ();
  
  /* 1 */
  /* if registering is enabled for LDAP,
     trigger the register notifier */
  if (gconf_client_get_bool (GCONF_CLIENT (client), "/apps/gnomemeeting/ldap/register", 0))
    gconf_client_set_bool (GCONF_CLIENT (client),
			  "/apps/gnomemeeting/ldap/register",
			  1, 0);

  /* 2 */
  /* Set the local User name */
  firstname =
    gconf_client_get_string (client, 
			     "/apps/gnomemeeting/personal_data/firstname", 
			     0);
  lastname =
    gconf_client_get_string (client, 
			     "/apps/gnomemeeting/personal_data/lastname", 0);

  if ((firstname) && (lastname)) {
    
    local_name = g_strdup ("");
    local_name = g_strconcat (local_name, firstname, " ", lastname, NULL);
  }
  else 
    local_name = g_strdup ((const char *) MyApp->Endpoint ()->GetLocalUserName ());
  
  alias_ = gconf_client_get_string (client, 
				      "/apps/gnomemeeting/gatekeeper/gk_alias", 0);
  if (alias_ != NULL)
    alias = PString (alias_);
  

  /* It is the first alias for the gatekeeper */
  MyApp->Endpoint ()->SetLocalUserName (local_name);
  
  /* Add the old alias (SetLocalUserName removes it) */
  if (!alias.IsEmpty ()) {

    MyApp->Endpoint ()->AddAliasName (alias);
  }


  /* 3 */
  /* Register to the Gatekeeper */
  int method = gconf_client_get_int (GCONF_CLIENT (client), "/apps/gnomemeeting/gatekeeper/registering_method", 0);

  /* We do that through the notifier */
  if (method)
    gconf_client_set_int (GCONF_CLIENT (client),
			  "/apps/gnomemeeting/gatekeeper/registering_method",
			  method, 0);
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
  gchar *codec_name = NULL;
  gchar *gconf_data = NULL;
  gchar *selected_codec_name = NULL;
  gchar *codecs_data = NULL;
  gchar **codecs;
  gchar *temp = NULL;
  gchar *tmp =NULL;
  int codec_pos = 0;
  int cpt = 0;
  int operation = 0;

  client = gconf_client_get_default ();
  gconf_data = g_strdup ("");

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
  codecs_data = gconf_client_get_string (client, 
					 "/apps/gnomemeeting/audio_codecs/list", NULL);

  /* We are adding the codecs */
  codecs = g_strsplit (codecs_data, ":", 0);

  for (cpt = 0 ; ((codecs [cpt] != NULL) && (cpt < GM_AUDIO_CODECS_NUMBER)) ; cpt++) {

    gchar **couple = g_strsplit (codecs [cpt], "=", 0);

    if (couple [0])
      if (!strcmp (couple [0], selected_codec_name)) codec_pos = cpt;

    g_strfreev (couple);
  }

  if (!strcmp ((gchar *) g_object_get_data (G_OBJECT (widget), "operation"), "up"))
    operation = 1;

  /* The selected codec is at pos codec_pos, we will build the gconf key data,
     and set that codec one pos up or one pos down */
  if (((codec_pos == 0)&&(operation == 1))||
      ((codec_pos == GM_AUDIO_CODECS_NUMBER - 1)&&(operation == 0))) {

    g_strfreev (codecs);
    g_free (codecs_data);
    g_free (gconf_data);

    return;
  }

  if (operation == 1) {

    temp = codecs [codec_pos - 1];
    codecs [codec_pos - 1] = codecs [codec_pos];
    codecs [codec_pos] = temp;
  }
  else {

    temp = codecs [codec_pos + 1];
    codecs [codec_pos + 1] = codecs [codec_pos];
    codecs [codec_pos] = temp;
  }

  for (cpt = 0 ; cpt < GM_AUDIO_CODECS_NUMBER ; cpt++) {

    tmp = g_strconcat (gconf_data, codecs [cpt], ":",  NULL);

    if (gconf_data)
      g_free (gconf_data);

    gconf_data = tmp;
    /* do not free codecs, they are pointers to the list_store fields */
  }
  g_strfreev (codecs);
  g_free (codecs_data);

  gconf_client_set_string (client, "/apps/gnomemeeting/audio_codecs/list", 
			   gconf_data, NULL);

  g_free (gconf_data);
}


static void
tree_selection_changed_cb (GtkTreeSelection *selection,
			   gpointer data)
{
  int page = 0;
  gchar *name = NULL;
  GtkTreeIter iter;
  GtkWidget *window;
  GtkWidget *label;
  GtkTreeModel *model;
  GM_window_widgets *gw = NULL;

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


void entry_changed (GtkEditable  *e, gpointer data)
{
  GM_pref_window_widgets *pw = gnomemeeting_get_pref_window (gm);
  GConfClient *client = gconf_client_get_default ();
  gchar *key = (gchar *) data;

  gconf_client_set_string (GCONF_CLIENT (client),
                           key,
                           gtk_entry_get_text (GTK_ENTRY (e)),
                           NULL);
}


void adjustment_changed (GtkAdjustment *adj, gpointer data)
{
  GConfClient *client = gconf_client_get_default ();
  gchar *key = (gchar *) data;

  gconf_client_set_int (GCONF_CLIENT (client),
                        key,
                        (int) adj->value, NULL);
}


void toggle_changed (GtkCheckButton *but, gpointer data)
{
  GConfClient *client = gconf_client_get_default ();
  gchar *key = (gchar *) data;

  gconf_client_set_bool (GCONF_CLIENT (client),
                         key,
                         gtk_toggle_button_get_active
                         (GTK_TOGGLE_BUTTON (but)),
                         NULL);
}


void int_option_menu_changed (GtkWidget *menu, gpointer data)
{
  GConfClient *client = gconf_client_get_default ();
  gchar *key = (gchar *) data;
  guint item_index;
  GtkWidget *active_item;

  active_item = gtk_menu_get_active (GTK_MENU (menu));
  item_index = g_list_index (GTK_MENU_SHELL (GTK_MENU (menu))->children, 
			     active_item);
 
  gconf_client_set_int (GCONF_CLIENT (client),
			key, item_index, NULL);
}


void string_option_menu_changed (GtkWidget *menu, gpointer data)
{
  GtkWidget *active_item;
  const gchar *text;
  GConfClient *client = gconf_client_get_default ();

  gchar *key = (gchar *) data;
  guint item_index;

  active_item = gtk_menu_get_active (GTK_MENU (menu));
  if (active_item == NULL)
    text = "";
  else
    text = gtk_label_get_text (GTK_LABEL (GTK_BIN (active_item)->child));

  gconf_client_set_string (GCONF_CLIENT (client),
			   key, text, NULL);
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

  if (!strcmp (codec_name, "LPC10")) {
      data [1] = g_strdup (_("Okay"));
      data [2] = g_strdup ("3.46 kb");
  }

  if (!strcmp (codec_name, "MS-GSM")) {
      data [1] = g_strdup (_("Good Quality"));
      data [2] = g_strdup ("13 kbits");
  }

  if (!strcmp (codec_name, "G.711-ALaw-64k")) {
    data [1] = g_strdup (_("Good Quality"));
    data [2] = g_strdup ("64 kbits");
  }

  if (!strcmp (codec_name, "G.711-uLaw-64k")) {
    data [1] = g_strdup (_("Good Quality"));
    data [2] = g_strdup ("64 kbits");
  }

  if (!strcmp (codec_name, "GSM-06.10")) {
    data [1] = g_strdup (_("Good Quality"));
    data [2] = g_strdup ("16.5 kbits");
  }

  if (!strcmp (codec_name, "GSM-06.10")) {
    data [1] = g_strdup (_("Good Quality"));
    data [2] = g_strdup ("16.5 kbits");
  }

  if (!strcmp (codec_name, "G.726-16k")) {
    data [1] = g_strdup (_("Good Quality"));
    data [2] = g_strdup ("16 kbits");
  }

  if (!strcmp (codec_name, "G.726-32k")) {
    data [1] = g_strdup (_("OKay"));
    data [2] = g_strdup ("32 kbits");
  }

  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter,
		      COLUMN_ACTIVE, enabled,
		      COLUMN_NAME, data [0],
		      COLUMN_INFO, data [1],
		      COLUMN_BANDWIDTH, data [2],
		      -1);

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
  gchar **codecs, **couple;
  gchar *codecs_data = NULL;
  gchar *tmp = NULL;
  gchar *gconf_data = NULL;
  GConfClient *client = NULL;
  gboolean fixed;
  gchar *selected_codec_name = NULL;
  int cpt = 0;

  gconf_data = g_strdup ("");
  client = gconf_client_get_default ();

  /* get toggled iter */
  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_model_get (model, &iter, COLUMN_ACTIVE, &fixed, -1);
  gtk_tree_model_get (model, &iter, COLUMN_NAME, &selected_codec_name, -1);
  fixed ^= 1;
  gtk_tree_path_free (path);

  /* We set the selected codec name as data of the list store, to select it again
     once the codecs list has been rebuilt */
  g_object_set_data (G_OBJECT (data), "selected_codec", 
		     (gpointer) selected_codec_name); /* Stores a copy of the pointer,
							 the gchar * must not be freed,
							 as it points to the list_store
							 element */

  /* Read all codecs, build the gconf data for the key, after having set the selected codec
     one row above its current plance */
  codecs_data = gconf_client_get_string (client, 
					 "/apps/gnomemeeting/audio_codecs/list", NULL);

  /* We are reading the codecs */
  codecs = g_strsplit (codecs_data, ":", 0);

  for (cpt = 0 ; ((codecs [cpt] != NULL) && (cpt < GM_AUDIO_CODECS_NUMBER)) ; cpt++) {

    couple = g_strsplit (codecs [cpt], "=", 0);

    if (couple [0])
      if (!strcmp (couple [0], selected_codec_name)) {

	g_free (couple [1]);
	couple [1] = g_strdup_printf ("%d", (int) fixed);
	codecs [cpt] = g_strconcat (couple [0], "=", couple [1],  NULL);
	g_strfreev (couple);
      }
  }  

  /* Rebuilt the gconf_key with the update values */
  for (cpt = 0 ; cpt < GM_AUDIO_CODECS_NUMBER ; cpt++) {

    tmp = g_strconcat (gconf_data, codecs [cpt], ":",  NULL);

    if (gconf_data)
      g_free (gconf_data);

    gconf_data = tmp;
    /* do not free codecs, they are pointers to the list_store fields */
  }
  g_strfreev (codecs);
  g_free (codecs_data);

  gconf_client_set_string (client, "/apps/gnomemeeting/audio_codecs/list", 
			   gconf_data, NULL);

  g_free (gconf_data);
}


/* Misc functions */
void gnomemeeting_codecs_list_build (GtkListStore *codecs_list_store, 
				     gchar *codecs_data)
{
  GtkTreeView *tree_view = NULL;
  GtkTreeSelection *selection = NULL;
  GtkTreePath *tree_path = NULL;
  GtkTreeIter list_iter;
  int selected_row = 0;
  gchar *cselect_row;

  static const gchar * const available_codecs[] = {
    "GSM-06.10",
    "MS-GSM",
    "G.726-16k",
    "G.726-32k",
    "G.711-uLaw-64k",
    "G.711-ALaw-64k",
    "LPC10",
    NULL
  };

  gchar *selected_codec = NULL;
  gchar **codecs;

  selected_codec = (gchar *) g_object_get_data (G_OBJECT (codecs_list_store), 
						"selected_codec");

  gtk_list_store_clear (GTK_LIST_STORE (codecs_list_store));

  /* We are adding the codecs */
  codecs = g_strsplit (codecs_data, ":", 0);

  for (int i = 0 ; ((codecs [i] != NULL) && (i < GM_AUDIO_CODECS_NUMBER)) ; i++) {

    gchar **couple = g_strsplit (codecs [i], "=", 0);

    if ((couple [0] != NULL) && (couple [1] != NULL)) {

      gnomemeeting_codecs_list_add (list_iter, codecs_list_store, 
				    couple [0], atoi (couple [1])); 

      /* Select the iter for the row corresponding to the last selected codec */
      if ((selected_codec) && (!strcmp (couple [0], selected_codec))) {
      
	selected_row = i;
      }
    }

    g_strfreev (couple);
  }

  /* This algo needs to be improved */
  for (int i = 0; available_codecs[i] != NULL; i++) {
    bool found = false;

    for (int j = 0; ((codecs[j] != NULL) && (j < GM_AUDIO_CODECS_NUMBER)) ; j++) {
      
      gchar **couple = g_strsplit (codecs[j], "=", 0);

      if ((couple [0] != NULL) && (couple [1] != NULL))
	if (!strcmp (available_codecs[i], couple[0])) {

	  found = true;
	  g_strfreev (couple);
	  
	  break;
	}

      g_strfreev (couple);
    }
    
    if (!found) 
      gnomemeeting_codecs_list_add (list_iter, codecs_list_store, available_codecs [i], 0); 

  }

  g_strfreev (codecs);

  cselect_row = g_strdup_printf("%d", selected_row);
  tree_path = gtk_tree_path_new_from_string (cselect_row);
  tree_view = GTK_TREE_VIEW (g_object_get_data (G_OBJECT (codecs_list_store), 
						      "tree_view"));
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
	
  gtk_tree_selection_select_path (GTK_TREE_SELECTION (selection), tree_path);

  g_free (cselect_row);
  gtk_tree_path_free (tree_path);
}


static GtkWidget *
gnomemeeting_pref_window_add_table (GtkWidget *vbox,         
				    gchar *frame_name,       
				    int rows, int cols)      
{                                                                              
  GtkWidget *frame;                                                            
  GtkWidget *table;                                                            
                                                                               
  frame = gtk_frame_new (frame_name);                                          
                                                                               
  gtk_box_pack_start (GTK_BOX (vbox), frame,                                   
                      FALSE, FALSE, 0);                                        
                                                                               
  table = gtk_table_new (rows, cols, FALSE);                                   
                                                                               
  gtk_container_add (GTK_CONTAINER (frame), table);                            
  gtk_container_set_border_width (GTK_CONTAINER (frame), 
				  GNOMEMEETING_PAD_SMALL * 2);
  gtk_container_set_border_width (GTK_CONTAINER (table), 
				  GNOMEMEETING_PAD_SMALL * 3);
                                                                               
  gtk_table_set_row_spacings (GTK_TABLE (table), GNOMEMEETING_PAD_SMALL);      
  gtk_table_set_col_spacings (GTK_TABLE (table), GNOMEMEETING_PAD_SMALL);      
                                                                               
  return table;                                                                
}                                                                              

                                                                               
static GtkWidget *
gnomemeeting_pref_window_add_entry (GtkWidget *table,        
				    gchar *label_txt,        
				    gchar *gconf_key,        
				    gchar *tooltip,          
				    int row)                 
{                                                                              
  GtkWidget *entry = NULL;                                                     
  GtkWidget *label = NULL;                                                     
  GtkTooltips *tip = NULL;                                                     
  gchar *gconf_string = NULL;                                                  
  GConfClient *client = NULL;                                                  
                                                                               
  client = gconf_client_get_default ();                                        
                                                                               

  label = gtk_label_new (label_txt);                                           

  gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1,                
                    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),                
                    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),                
                    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);           
                                                                               
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);                         
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_RIGHT);                

                                                                               
  entry = gtk_entry_new ();                                                    
  gtk_table_attach (GTK_TABLE (table), entry, 1, 2, row, row+1,                
                    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),                
                    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),                
                    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);           
                                                                               
  gconf_string =  gconf_client_get_string (GCONF_CLIENT (client),              
                                           gconf_key, NULL);                   
                                                                               
  if (gconf_string != NULL)                                                    
    gtk_entry_set_text (GTK_ENTRY (entry), gconf_string);                      
                                                                               
  g_free (gconf_string);                                                       
                                                                               
                                                                               
  /* We set the key as data to be able to get the data in order to block       
     the signal in the gconf notifier */                             
  g_object_set_data (G_OBJECT (entry), "gconf_key", (void *) gconf_key);

  g_signal_connect (G_OBJECT (entry), "changed",                           
		    G_CALLBACK (entry_changed),                         
		    (gpointer) g_object_get_data (G_OBJECT (entry),
						  "gconf_key"));


  tip = gtk_tooltips_new ();                                                   
  gtk_tooltips_set_tip (tip, entry, tooltip, NULL);                            
                                                                               
  return entry;                                                                
}                                                                              
                                                                               
                                                                               
static GtkWidget *
gnomemeeting_pref_window_add_toggle (GtkWidget *table,       
				     gchar *label_txt,       
				     gchar *gconf_key,       
				     gchar *tooltip,         
				     int row, int col)       
{
  GtkWidget *label = NULL;                                                     
  GtkWidget *toggle = NULL;  
  GtkTooltips *tip = NULL;                                                     
                                                                               
                                                                               
  GConfClient *client = NULL;                                                  
                                                                               
  client = gconf_client_get_default ();                                        

                                                                               
  toggle = gtk_check_button_new_with_label (label_txt);                        
  gtk_table_attach (GTK_TABLE (table), toggle, col, col+1, row, row+1,         
                    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),                
                    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),                
                    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);           
                                                                               
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle), 
				gconf_client_get_bool (client, 
						       gconf_key, NULL));

                                                                               
  tip = gtk_tooltips_new ();                                                   
  gtk_tooltips_set_tip (tip, toggle, tooltip, NULL);                           

  /* We set the key as data to be able to get the data in order to block       
     the signal in the gconf notifier */                             
  g_object_set_data (G_OBJECT (toggle), "gconf_key", (void *) gconf_key);
                                                                               
  g_signal_connect (G_OBJECT (toggle), "toggled", G_CALLBACK (toggle_changed),
		    (gpointer) gconf_key);                                   
                                                                               
  return toggle;
}                                                                              


static GtkWidget *
gnomemeeting_pref_window_add_spin (GtkWidget *table,       
				   gchar *label_txt,       
				   gchar *gconf_key,       
				   gchar *tooltip,         
				   double min, double max, double step, int row)       
{
  GtkAdjustment *adj = NULL;
  GtkWidget *label = NULL;
  GtkWidget *spin_button = NULL;
  GtkTooltips *tip = NULL;                                                     
                                                                               
                                                                               
  GConfClient *client = NULL;                                                  
                                                                               
  client = gconf_client_get_default ();                                        


  label = gtk_label_new (label_txt);                                           

  gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1,                
                    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),                
                    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),                
                    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);           
                                                                               
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);                         
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_RIGHT);                


  adj = (GtkAdjustment *) 
    gtk_adjustment_new (gconf_client_get_int (client, gconf_key, 0), min, max, step, 
			2.0, 1.0);

  spin_button =
    gtk_spin_button_new (adj, 1.0, 0);
                                                                               
  gtk_table_attach (GTK_TABLE (table), spin_button, 1, 2, row, row+1,         
                    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),                
                    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),                
                    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);           
                                                                               
  gtk_adjustment_set_value (GTK_ADJUSTMENT (adj), 
			    gconf_client_get_int (client, gconf_key, NULL));

                                                                               
  tip = gtk_tooltips_new ();                                                   
  gtk_tooltips_set_tip (tip, spin_button, tooltip, NULL);                           

  /* We set the key as data to be able to get the data in order to block       
     the signal in the gconf notifier */                             
  g_object_set_data (G_OBJECT (adj), "gconf_key", (void *) gconf_key);
                                                                               
  g_signal_connect (G_OBJECT (adj), "value-changed", G_CALLBACK (adjustment_changed),
		    (gpointer) gconf_key);                                   
                                                                               
  return spin_button;
}                                                                              


static GtkWidget *
gnomemeeting_pref_window_add_int_option_menu (GtkWidget *table,       
					      gchar *label_txt, 
					      gchar **options,
					      gchar *gconf_key,       
					      gchar *tooltip,         
					      int row)       
{
  GtkWidget *item = NULL;
  GtkWidget *label = NULL;                                                     
  GtkWidget *option_menu = NULL;
  GtkWidget *menu = NULL;
  GtkTooltips *tip = NULL;                                                     
  int cpt = 0;                                                   

  GConfClient *client = NULL;                                                  
                                                                               
  client = gconf_client_get_default ();                                        


  label = gtk_label_new (label_txt);                                           

  gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1,                
                    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),                
                    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),                
                    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);           
                                                                               
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);                         
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_RIGHT);                


  menu = gtk_menu_new ();
  option_menu = gtk_option_menu_new ();

  while (options [cpt]) {

    item = gtk_menu_item_new_with_label (options [cpt]);
    gtk_widget_show (item);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
    cpt++;
  }

  gtk_option_menu_set_menu (GTK_OPTION_MENU (option_menu), menu);
  gtk_option_menu_set_history (GTK_OPTION_MENU (option_menu), 
 			       gconf_client_get_int (client, gconf_key, NULL));

  gtk_table_attach (GTK_TABLE (table), option_menu, 1, 2, row, row+1,         
                    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),                
                    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),                
                    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);           
                                                                               
                                                                               
  tip = gtk_tooltips_new ();                                                   
  gtk_tooltips_set_tip (tip, option_menu, tooltip, NULL);                           

  /* We set the key as data to be able to get the data in order to block       
     the signal in the gconf notifier */                             
  g_object_set_data (G_OBJECT (option_menu), "gconf_key", (void *) gconf_key);
                                                                               
  g_signal_connect (G_OBJECT (GTK_OPTION_MENU (option_menu)->menu), 
		    "deactivate", G_CALLBACK (int_option_menu_changed),
  		    (gpointer) gconf_key);                                   
                                                                               
  return option_menu;
}                                                                              


static GtkWidget *
gnomemeeting_pref_window_add_string_option_menu (GtkWidget *table,       
						 gchar *label_txt, 
						 gchar **options,
						 gchar *gconf_key,       
						 gchar *tooltip,         
						 int row)       
{
  GtkWidget *item = NULL;
  GtkWidget *label = NULL;                                                     
  GtkWidget *option_menu = NULL;
  GtkWidget *menu = NULL;
  GtkTooltips *tip = NULL;        
  gchar *gconf_string = NULL;
  int history = 0;

  int cpt = 0;                                                   

  GConfClient *client = NULL;                                                  
                                                                               
  client = gconf_client_get_default ();                                        


  label = gtk_label_new (label_txt);                                           

  gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1,                
                    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),                
                    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),                
                    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);           
                                                                               
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);                         
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_RIGHT);                


  menu = gtk_menu_new ();
  option_menu = gtk_option_menu_new ();

  gconf_string = gconf_client_get_string (client, gconf_key, NULL);

  while (options [cpt]) {

    if (gconf_string != NULL)
      if (!strcmp (gconf_string, options [cpt]))
	history = cpt;

    item = gtk_menu_item_new_with_label (options [cpt]);
    gtk_widget_show (item);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
    cpt++;
  }

  gtk_option_menu_set_menu (GTK_OPTION_MENU (option_menu), menu);
  gtk_option_menu_set_history (GTK_OPTION_MENU (option_menu), 
 			       history);

  gtk_table_attach (GTK_TABLE (table), option_menu, 1, 2, row, row+1,         
                    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),                
                    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),                
                    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);           
                                                                               
                                                                               
  tip = gtk_tooltips_new ();                                                   
  gtk_tooltips_set_tip (tip, option_menu, tooltip, NULL);                  

  /* We set the key as data to be able to get the data in order to block       
     the signal in the gconf notifier */                             
  g_object_set_data (G_OBJECT (option_menu), "gconf_key", (void *) gconf_key);
                                                                               
  g_signal_connect (G_OBJECT (GTK_OPTION_MENU (option_menu)->menu), 
		    "deactivate", G_CALLBACK (string_option_menu_changed),
  		    (gpointer) gconf_key);                                   
                                                                               
  return option_menu;
}

                                                                               
static GtkWidget *
gnomemeeting_pref_window_add_update_button (GtkWidget *table,
					    GtkSignalFunc func,
					    gchar *tooltip,  
					    int row, int col)
{
  GtkTooltips *tip = NULL;
  GtkWidget *button = NULL;                                                    

  GM_pref_window_widgets *pw = NULL;                                           
                                                                               
  pw = gnomemeeting_get_pref_window (gm);                                      
                                                                               

  button = gtk_button_new_from_stock (GTK_STOCK_APPLY);
                                                                               
  gtk_table_attach (GTK_TABLE (table),  button, col, col+1, row, row+1,        
                    (GtkAttachOptions) (GTK_EXPAND),                           
                    (GtkAttachOptions) (GTK_EXPAND),                           
                    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);           
                                                                               
  g_signal_connect (G_OBJECT (button), "clicked",                          
		    G_CALLBACK (func), (gpointer) pw);

  tip = gtk_tooltips_new ();                                                   
  gtk_tooltips_set_tip (tip, button, tooltip, NULL);                           
                                                                               
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
  GM_pref_window_widgets *pw = gnomemeeting_get_pref_window (gm);              
                                                                               
                                                                               
  /* Packing widgets */                                                        
  vbox = gtk_vbox_new (FALSE, 4);
  table = gnomemeeting_pref_window_add_table (vbox, _("Personnal Information"),
                                              7, 3);                           

                                                                               
  /* Add all the fields */                                                     
  pw->firstname = 
    gnomemeeting_pref_window_add_entry (table, _("First Name:"), "/apps/gnomemeeting/personal_data/firstname", _("Enter your first name."), 0);

  pw->surname = 
    gnomemeeting_pref_window_add_entry (table, _("Last Name:"), "/apps/gnomemeeting/personal_data/lastname", _("Enter your last name."), 1);
                                                                               
  pw->gk_alias = 
    gnomemeeting_pref_window_add_entry (table, _("Gatekeeper alias: "), "/apps/gnomemeeting/gatekeeper/gk_alias", _("The Gatekeeper Alias to use when registering (string, or E164 ID if only 0123456789#)."), 2);

  pw->mail = gnomemeeting_pref_window_add_entry (table, _("E-mail Address:"), "/apps/gnomemeeting/personal_data/mail", _("Enter your e-mail address."), 3);
                                                                               
  pw->comment = gnomemeeting_pref_window_add_entry (table, _("Comment:"), "/apps/gnomemeeting/personal_data/comment", _("Here you can fill in a comment about yourself for ILS directories."), 4);
                                                                               
  pw->location = gnomemeeting_pref_window_add_entry (table, _("Location:"), "/apps/gnomemeeting/personal_data/location", _("Where do you call from?"), 5);
                                                                               
                                                                               
  /* Add the try button */                                                     
  pw->directory_update_button =                                                
    gnomemeeting_pref_window_add_update_button (table, GTK_SIGNAL_FUNC (personal_data_update_button_clicked), _("Click here to update the LDAP server you are registered to with the new First Name, Last Name, E-Mail, Comment and Location or to update your alias on the Gatekeeper."), 6, 2);
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
  GConfClient *client = gconf_client_get_default ();
                                                                               
  /* Get the data */                                                           
  GM_pref_window_widgets *pw = gnomemeeting_get_pref_window (gm);              
                                                                               
                                                                               
  /* Packing widgets */                                                        
  vbox = gtk_vbox_new (FALSE, 4);
  gtk_notebook_append_page (GTK_NOTEBOOK(notebook), vbox, NULL);    
  table = gnomemeeting_pref_window_add_table (vbox,
					      _("GnomeMeeting GUI"), 2, 2);

  pw->show_splash = gnomemeeting_pref_window_add_toggle (table, _("Show Splash Screen"), "/apps/gnomemeeting/view/show_splash", _("If enabled, the splash screen will be displayed at startup time."), 0, 1);
                                                                                          
  pw->start_hidden = gnomemeeting_pref_window_add_toggle (table, _("Start Hidden"), "/apps/gnomemeeting/view/start_docked", _("If enabled, GnomeMeeting will start hidden. The docklet must be enabled."), 1, 1);

                                                                               
  /* Packing widget */                                                         
  table = gnomemeeting_pref_window_add_table (vbox, _("Behavior"), 3, 1);      
                                                                               
                                                                               
  /* The toggles */
  pw->aa = gnomemeeting_pref_window_add_toggle (table, _("Auto Answer"), "/apps/gnomemeeting/general/auto_answer", _("If enabled, incoming calls will be automatically answered."), 1, 0);
                                                                               
  pw->dnd = gnomemeeting_pref_window_add_toggle (table, _("Do Not Disturb"), "/apps/gnomemeeting/general/do_not_disturb", _("If enabled, incoming calls will be automatically refused."), 0, 0);
                                                                               
  pw->incoming_call_popup = gnomemeeting_pref_window_add_toggle (table, _("Popup window"), "/apps/gnomemeeting/view/show_popup", _("If enabled, a popup will be displayed when receiving an incoming call"), 2, 0);
                                                                                
                                                                               
  /* Packing widget */                                                         
  table = gnomemeeting_pref_window_add_table (vbox, _("Sound"),                
                                              1, 1);                           
                                                                               
  /* The toggles */                                                            
  pw->incoming_call_sound = gnomemeeting_pref_window_add_toggle (table, _("Incoming Call"), "/apps/gnomemeeting/general/incoming_call_sound", _("If enabled, GnomeMeeting will play a sound when receiving an incoming call (the sound to play is chosen in the Gnome Control Center)."), 0, 0);
}


/* BEHAVIOR     :  It builds the notebook page for XDAP directories and gk,        
 *                 it adds it to the notebook.                                     
 * PRE          :  The notebook.                                               
 */                                                                            
void gnomemeeting_init_pref_window_directories (GtkWidget *notebook)               
{                                                                              
  GtkWidget *vbox = NULL;                                                      
  GtkWidget *table = NULL;                                                     
  gchar *options [] = {_("Do not register"), 
		       _("Gatekeeper host"), 
		       _("Gatekeeper ID"), 
		       _("Automatic Discover"), 
		       NULL};
                                                                               
  /* Get the data */                                                           
  GM_pref_window_widgets *pw = gnomemeeting_get_pref_window (gm);              
                                                                               
                                                                               
  /* Packing widgets for the XDAP directory */                                               
  vbox = gtk_vbox_new (FALSE, 4);
  gtk_notebook_append_page (GTK_NOTEBOOK(notebook), vbox, NULL);    
  table = gnomemeeting_pref_window_add_table (vbox, _("XDAP Directory"),
                                              2, 1);                           
                                                                               
  /* Add all the fields */                                                     
  pw->ldap_server = 
    gnomemeeting_pref_window_add_entry (table, _("XDAP Directory:"), "/apps/gnomemeeting/ldap/ldap_server", _("The XDAP server to register to."), 0);

  pw->ldap =
    gnomemeeting_pref_window_add_toggle (table, _("Enable Registering"), "/apps/gnomemeeting/ldap/register", _("If enabled, permit to register to the selected LDAP directory"), 1, 0);


  /* Add fields for the gatekeeper */
  table = gnomemeeting_pref_window_add_table (vbox, _("Gatekeeper"), 4, 1);                 

  pw->gk_id = 
    gnomemeeting_pref_window_add_entry (table, _("Gatekeeper ID:"), "/apps/gnomemeeting/gatekeeper/gk_id", _("The Gatekeeper identifier to register to."), 1);

  pw->gk_host = 
    gnomemeeting_pref_window_add_entry (table, _("Gatekeeper host:"), "/apps/gnomemeeting/gatekeeper/gk_host", _("The Gatekeeper host to register to."), 2);

  pw->gk = 
    gnomemeeting_pref_window_add_int_option_menu (table, _("Registering method:"), options, "/apps/gnomemeeting/gatekeeper/registering_method", _("Registering method to use"), 0);
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
  GM_pref_window_widgets *pw = gnomemeeting_get_pref_window (gm);              
                                                                               
                                                                               
  /* Packing widgets */                                                        
  vbox = gtk_vbox_new (FALSE, 4);
  table = gnomemeeting_pref_window_add_table (vbox, _("H.323 Call Forwarding"),
                                              4, 2);                           

                                                                               
  /* Add all the fields */                                                     
  pw->forward_host = 
    gnomemeeting_pref_window_add_entry (table, _("Forward calls to host:"), "/apps/gnomemeeting/call_forwarding/host", _("Enter here the host where calls should be forwarded in the cases selected below"), 3);

  pw->always_forward =
    gnomemeeting_pref_window_add_toggle (table, _("Always forward calls to the given host"), "/apps/gnomemeeting/call_forwarding/always_forward", _("If enabled, all incoming calls will always be forwarded to the host that is specified in the field below."), 0, 0);

  pw->no_answer_forward =
    gnomemeeting_pref_window_add_toggle (table, _("Forward calls to the given host if no answer"), "/apps/gnomemeeting/call_forwarding/no_answer_forward", _("If enabled, all incoming calls will be forwarded to the host that is specified in the field below if you don't answer the call."), 1, 0);

  pw->busy_forward =
    gnomemeeting_pref_window_add_toggle (table, _("Forward calls to the given host if busy"), "/apps/gnomemeeting/call_forwarding/busy_forward", _("If enabled, all incoming calls will be forwarded to the host that is specified in the field below if you already are in a call or if you are in Do Not Disturb mode."), 2, 0);
  cout << "FIX ME; Call forwarding + Call forwarding in DND mode" << endl << flush;


  /* Packing widget */                                                         
  table = gnomemeeting_pref_window_add_table (vbox, 
					      _("H.323 V2 Settings"),
                                              2, 1);                           

                                                                               
  /* The toggles */                                                            
  pw->ht = gnomemeeting_pref_window_add_toggle (table, _("Enable H.245 Tunnelling"), "/apps/gnomemeeting/general/h245_tunneling", _("This enables H.245 Tunnelling mode."), 0, 0);
                                                                               
  pw->fs = gnomemeeting_pref_window_add_toggle (table, _("Enable Fast Start"), "/apps/gnomemeeting/general/fast_start", _("Connection will be established in Fast Start mode."), 1, 0);


  gtk_notebook_append_page (GTK_NOTEBOOK(notebook), vbox, NULL);
}                                                                              


/* BEHAVIOR     :  It builds the notebook page for the device settings and
 *                 add it to the notebook.
 * PRE          :  See init_pref_audio_codecs.
 */
static void gnomemeeting_init_pref_window_devices (GtkWidget *notebook)
{
  GConfClient *client = NULL;
  gchar *gconf_string = NULL;
  GtkWidget *vbox = NULL;                                                      
  GtkWidget *table = NULL; 
  int default_present = -1;
  int i = 0;
  gchar *video_size [] = {_("Small"), 
			  _("Large"), 
			  NULL};
  gchar *video_format [] = {_("PAL"), 
			    _("NTSC"), 
			    _("SECAM"), 
			    _("auto"), 
			    NULL};                                                  
  gchar *audio_player_devices_list [20];
  gchar *audio_recorder_devices_list [20];
  gchar *video_devices_list [20];
                

  /* Get the data */                                             
  GM_window_widgets *gw = gnomemeeting_get_main_window (gm);              
  GM_pref_window_widgets *pw = gnomemeeting_get_pref_window (gm);
  
  client = gconf_client_get_default ();
                                                                               
  /* Packing widgets for the XDAP directory */                                
  vbox = gtk_vbox_new (FALSE, 4);
  gtk_notebook_append_page (GTK_NOTEBOOK(notebook), vbox, NULL);    
  table = gnomemeeting_pref_window_add_table (vbox, _("Audio Devices"),
                                              4, 2);                           
                                                                               
  /* Add all the fields */                 
  /* The player */
  gconf_string =  gconf_client_get_string (GCONF_CLIENT (client), "/apps/gnomemeeting/devices/audio_player", NULL);

  i = gw->audio_player_devices.GetSize () - 1;
  if (i >= 20) i = 19;

  for (int j = i ; j >= 0; j--) 
    audio_player_devices_list [j] = g_strdup (gw->audio_player_devices [j]);

  
  audio_player_devices_list [i+1] = NULL;

  pw->audio_player = 
    gnomemeeting_pref_window_add_string_option_menu (table, _("Audio Player:"), audio_player_devices_list, "/apps/gnomemeeting/devices/audio_player", _("Enter the audio player device to use."), 0);

  for (int j = i ; j >= 0; j--) 
    g_free (audio_player_devices_list [j]);
  g_free (gconf_string); 

  
  pw->audio_player_mixer =
    gnomemeeting_pref_window_add_entry (table, _("Player Mixer:"), "/apps/gnomemeeting/devices/audio_player_mixer", _("The audio mixer to use to setup the volume of the audio player device."), 1);


  /* The recorder */
  gconf_string =  gconf_client_get_string (GCONF_CLIENT (client), "/apps/gnomemeeting/devices/audio_recorder", NULL);

  i = gw->audio_recorder_devices.GetSize () - 1;
  if (i >= 20) i = 19;

  for (int j = i ; j >= 0; j--) 
    audio_recorder_devices_list [j] = 
      g_strdup (gw->audio_recorder_devices [j]);
  
  audio_recorder_devices_list [i+1] = NULL;

  pw->audio_recorder = 
    gnomemeeting_pref_window_add_string_option_menu (table, _("Audio Recorder:"), audio_recorder_devices_list, "/apps/gnomemeeting/devices/audio_recorder", _("Enter the audio recorder device to use."), 2);

  for (int j = i ; j >= 0; j--) 
    g_free (audio_recorder_devices_list [j]);
  g_free (gconf_string);


  pw->audio_recorder_mixer =
    gnomemeeting_pref_window_add_entry (table, _("Recorder Mixer:"), "/apps/gnomemeeting/devices/audio_recorder_mixer", _("The audio mixer to use to setup the volume of the audio recorder device."), 3);

  /* The video devices related options */
  table = gnomemeeting_pref_window_add_table (vbox, _("Video Devices"), 5, 2);

  /* The video device */
  gconf_string =  gconf_client_get_string (GCONF_CLIENT (client), "/apps/gnomemeeting/devices/video_recorder", NULL);

  i = gw->video_devices.GetSize () - 1;
  if (i >= 20) i = 19;

  for (int j = i ; j >= 0; j--) 
    video_devices_list [j] = 
      g_strdup (gw->video_devices [j]);
  
  video_devices_list [i+1] = NULL;

  pw->video_device = 
    gnomemeeting_pref_window_add_string_option_menu (table, _("Video Device:"), video_devices_list, "/apps/gnomemeeting/devices/video_recorder", _("Enter the video device to use, using a wrong video device for video transmission will transmit a test picture."), 1);

  for (int j = i ; j >= 0; j--) 
    g_free (video_devices_list [j]);
  g_free (gconf_string);


  /* Video Channel */
  pw->video_channel =
    gnomemeeting_pref_window_add_spin (table, _("Video Channel:"),       
				       "/apps/gnomemeeting/devices/video_channel",       
				       _("The video channel number to use (camera, tv, ...)."),
				       0.0, 10.0, 1.0, 2);
  
  pw->opt1 =
    gnomemeeting_pref_window_add_int_option_menu (table, _("Video Size:"), video_size, "/apps/gnomemeeting/devices/video_size", _("Choose the transmitted video size : QCIF (Small) or CIF (Large)."), 3);

  pw->opt2 =
    gnomemeeting_pref_window_add_int_option_menu (table, _("Video Format:"), video_format, "/apps/gnomemeeting/devices/video_format", _("Here you can choose the transmitted video format."), 4);

  pw->video_preview =
    gnomemeeting_pref_window_add_toggle (table, _("Video Preview"), "/apps/gnomemeeting/devices/video_preview", _("If enabled, the video preview mode will be set activated and you will be able to see yourself without being in a call."), 5, 0);

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

  gchar *codecs_data = NULL;

  int width = 80;
  GtkRequisition size_request1, size_request2;

  /* For the GTK TreeView */
  GtkWidget *tree_view;
  GtkTreePath *tree_path;
  GtkTreeViewColumn *column;
  GtkCellRenderer *renderer;                        
                                                       

  /* Get the data */                                                           
  GM_pref_window_widgets *pw = gnomemeeting_get_pref_window (gm);
  GConfClient *client = gconf_client_get_default ();


  /* Packing widgets */                                                        
  vbox =  gtk_vbox_new (FALSE, 4);
  gtk_notebook_append_page (GTK_NOTEBOOK(notebook), vbox, NULL);  
  table = gnomemeeting_pref_window_add_table (vbox, 
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
  gtk_tree_view_set_search_column (GTK_TREE_VIEW (tree_view),
				   COLUMN_FIRSTNAME);
  
  frame = gtk_frame_new (NULL);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 2*GNOMEMEETING_PAD_SMALL);
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
  codecs_data = gconf_client_get_string (client, 
					 "/apps/gnomemeeting/audio_codecs/list", NULL);

  gtk_table_attach (GTK_TABLE (table),  frame, 0, 1, 0, 8,        
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);           

  gnomemeeting_codecs_list_build (pw->codecs_list_store, codecs_data);


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
  table = gnomemeeting_pref_window_add_table (vbox, _("Audio Codecs Settings"), 4, 2);

  /* Jitter Buffer */
  pw->jitter_buffer =
     gnomemeeting_pref_window_add_spin (table, _("Jitter Buffer:"),       
 				       "/apps/gnomemeeting/audio_settings/jitter_buffer",
					_("The jitter buffer delay to buffer audio calls (in ms)."),
 				       20.0, 5000.0, 1.0, 1);

  pw->gsm_frames =
     gnomemeeting_pref_window_add_spin (table, _("GSM Frames per packet:"),       
 				       "/apps/gnomemeeting/audio_settings/gsm_frames",
					_("The number of frames in each transmitted GSM packet."),
 				       1.0, 7.0, 1.0, 2);

  pw->g711_frames =
     gnomemeeting_pref_window_add_spin (table, _("G.711 Frames per packet:"),       
 				       "/apps/gnomemeeting/audio_settings/g711_frames",
					_("The number of frames in each transmitted G.711 packet."),
 				       11.0, 240.0, 1.0, 3);

  pw->sd = 
    gnomemeeting_pref_window_add_toggle (table, _("Enable Silence Detection"),       
 				       "/apps/gnomemeeting/audio_settings/sd",
					_("Enable or not the silence detection for the GSM and G.711 codecs."), 4, 0);
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
  GM_pref_window_widgets *pw = gnomemeeting_get_pref_window (gm);              
                                                                               
                                                                               
  /* Packing widgets for the XDAP directory */ 
  vbox =  gtk_vbox_new (FALSE, 4);
  gtk_notebook_append_page (GTK_NOTEBOOK(notebook), vbox, NULL);  
  table = gnomemeeting_pref_window_add_table (vbox, _("General Settings"),
                                              3, 1);                           

  /* Add fields */
  pw->tr_fps =
    gnomemeeting_pref_window_add_spin (table, _("Maximum Transmitted FPS:"),       
 				       "/apps/gnomemeeting/video_settings/tr_fps",
				       _("The number of video frames transmitted each second."),
 				       1.0, 30.0, 1.0, 1);

  pw->fps = 
    gnomemeeting_pref_window_add_toggle (table, _("Enable FPS Limitation"), "/apps/gnomemeeting/video_settings/enable_fps", _("Enable/disable the limit on the transmitted FPS."), 0, 0);

  pw->vid_tr = 
    gnomemeeting_pref_window_add_toggle (table, _("Enable Video Transmission"), "/apps/gnomemeeting/video_settings/enable_video_transmission", _("Enable/disable the video transmission."), 2, 0);


  /* H.261 Settings */
  table = gnomemeeting_pref_window_add_table (vbox, _("H.261 Settings"),
                                              3, 1);                           

  pw->tr_vq =
    gnomemeeting_pref_window_add_spin (table, _("Transmitted Video Quality:"),       
 				       "/apps/gnomemeeting/video_settings/tr_vq",
				       _("The transmitted video quality:  choose 100% on a LAN for the best quality, 1% being the worst quality."),
 				       1.0, 100.0, 1.0, 0);

  pw->re_vq =
    gnomemeeting_pref_window_add_spin (table, _("Received Video Quality:"),       
 				       "/apps/gnomemeeting/video_settings/re_vq",
				       _("The received video quality:  choose 100% on a LAN for the best quality, 1% being the worst quality."),
 				       1.0, 100.0, 1.0, 1);

  pw->tr_ub =
    gnomemeeting_pref_window_add_spin (table, _("Transmitted Background Blocks:"),       
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
  GM_window_widgets *gw = gnomemeeting_get_main_window (gm);
  GM_pref_window_widgets *pw = gnomemeeting_get_pref_window (gm);

  gw->pref_window = gtk_dialog_new ();
  gtk_dialog_add_button (GTK_DIALOG (gw->pref_window), GTK_STOCK_CLOSE, 0);

  g_signal_connect (G_OBJECT (gw->pref_window), "response",
 		    G_CALLBACK (pref_window_clicked_callback), 
 		    (gpointer) pw);

  gtk_window_set_title (GTK_WINDOW (gw->pref_window), 
			_("GnomeMeeting Settings"));	


  /* Construct the window */
  notebook = gtk_notebook_new ();

  dialog_vbox = GTK_DIALOG (gw->pref_window)->vbox;

  hbox = gtk_hbox_new (FALSE, 6);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 6);
  gtk_container_add (GTK_CONTAINER (dialog_vbox), hbox);


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
  attr = pango_attr_scale_new (PANGO_SCALE_X_LARGE);
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

  gtk_tree_store_append (GTK_TREE_STORE (model), &child_iter, &iter);
  gtk_tree_store_set (GTK_TREE_STORE (model),
		      &child_iter, 0, _("Device Settings"), 1, 5, -1);
  gnomemeeting_init_pref_window_devices (notebook);

  /* Another section */
  gtk_tree_store_append (GTK_TREE_STORE (model), &iter, NULL);
  gtk_tree_store_set (GTK_TREE_STORE (model),
		      &iter, 0, _("Codecs"), 1, 0, -1);

  gtk_tree_store_append (GTK_TREE_STORE (model), &child_iter, &iter);
  gtk_tree_store_set (GTK_TREE_STORE (model),
		      &child_iter, 0, _("Audio Codecs"), 1, 6, -1);
  gnomemeeting_init_pref_window_audio_codecs (notebook);


  gtk_tree_store_append (GTK_TREE_STORE (model), &child_iter, &iter);
  gtk_tree_store_set (GTK_TREE_STORE (model),
		      &child_iter, 0, _("Video Codecs"), 1, 7, -1);
  gnomemeeting_init_pref_window_video_codecs (notebook);


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

