
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

#define GNOMEMEETING_PAD_SMALL 2

/* Declarations */

extern GtkWidget *gm;
extern GnomeMeeting *MyApp;	

static void pref_window_clicked_callback (GnomeDialog *, int, gpointer);
static gint pref_window_destroy_callback (GtkWidget *, gpointer);
static void personal_data_update_button_clicked (GtkWidget *, gpointer);
static void codecs_clist_button_clicked_callback (GtkWidget *, gpointer);
static void codecs_clist_row_selected_callback (GtkWidget *, gint, gint, 
						GdkEventButton *, gpointer);
static void menu_ctree_row_seletected_callback (GtkWidget *, gint, gint, 
						GdkEventButton *, gpointer);

static void gnomemeeting_init_pref_window_general (GtkWidget *);
static void gnomemeeting_init_pref_window_interface (GtkWidget *);
static void gnomemeeting_init_pref_window_directories (GtkWidget *);
static void gnomemeeting_init_pref_window_devices (GtkWidget *);
static void gnomemeeting_init_pref_window_audio_codecs (GtkWidget *);
static void gnomemeeting_init_pref_window_codecs_settings (GtkWidget *);


/* GTK Callbacks */

/* DESCRIPTION  :  This callback is called when the user clicks on "Close"
 * BEHAVIOR     :  Closes the window.
 * PRE          :  gpointer is a valid pointer to a GM_pref_window_widgets.
 */
static void pref_window_clicked_callback (GnomeDialog *widget, int button, 
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
  GConfClient *client = gconf_client_get_default ();
  
  /* if registering is enabled,
     trigger the register notifier */
  if (gconf_client_get_bool (GCONF_CLIENT (client), "/apps/gnomemeeting/ldap/register", 0))
    gconf_client_set_bool (GCONF_CLIENT (client),
			  "/apps/gnomemeeting/ldap/register",
			  1, 0);
}


/* DESCRIPTION  :  This callback is called when the user clicks
 *                 on a button in the Audio Codecs Settings 
 *                 (Add, Del, Up, Down)
 * BEHAVIOR     :  It updates the clist order or the clist data following the
 *                 operation (Up => up, Add => changes row pixmap and set
 *                 row data to 1)
 * PRE          :  gpointer is a valid pointer to a gchar * containing
 *                 the operation (Add / Del / Up / Down)
 */
static void codecs_clist_button_clicked_callback (GtkWidget *widget, 
						  gpointer data)
{ 		
  gchar *codec_name = NULL;
  gchar *gconf_key = 0;

  int i = 0;

  GdkPixmap *yes, *no;
  GdkBitmap *mask_yes, *mask_no;

  GM_pref_window_widgets *pw = (GM_pref_window_widgets *) data;
  GConfClient *client = gconf_client_get_default ();

  gchar *row_data = NULL;   /* Do not free, this is not a copy which is stored,
			       but a copy of the pointer itself */
  gchar *old_row_data = NULL;

  row_data = (gchar *) g_malloc (3);
 
  gnome_stock_pixmap_gdk (GNOME_STOCK_BUTTON_APPLY,
			  NULL, &yes, &mask_yes);

  gnome_stock_pixmap_gdk (GNOME_STOCK_BUTTON_CANCEL,
			  NULL, &no, &mask_no);


  if (!strcmp ((char *) gtk_object_get_data (GTK_OBJECT (widget), "operation"), 
	       "Add")) {
    gtk_clist_set_pixmap (GTK_CLIST (pw->clist_avail), pw->row_avail, 
			  0, yes, mask_yes);
   
    /* First we free the old row data */
    old_row_data = (gchar *) 
      gtk_clist_get_row_data (GTK_CLIST (pw->clist_avail), pw->row_avail);
    g_free (old_row_data);

    /* Second, we set the new data */
    strcpy (row_data, "1");
    gtk_clist_set_row_data (GTK_CLIST (pw->clist_avail), pw->row_avail, 
			    (gpointer) row_data);
  }

  if (!strcmp ((char *) gtk_object_get_data (GTK_OBJECT (widget), "operation"), 
	       "Del")) {
    gtk_clist_set_pixmap (GTK_CLIST (pw->clist_avail), pw->row_avail, 
			  0, no, mask_no);

    /* First we free the old row data */
    old_row_data = (gchar *)
      gtk_clist_get_row_data (GTK_CLIST (pw->clist_avail), pw->row_avail);
    g_free (old_row_data);

    /* Second, we set the new data */
    strcpy (row_data, "0");
    gtk_clist_set_row_data (GTK_CLIST (pw->clist_avail), pw->row_avail, 
			    (gpointer) row_data);
  }
  
  if (!strcmp ((char *) gtk_object_get_data (GTK_OBJECT (widget), "operation"), 
	       "Up")) {
    gtk_clist_row_move (GTK_CLIST (pw->clist_avail), pw->row_avail, 
			pw->row_avail - 1);
    if (pw->row_avail > 0) pw->row_avail--;
  }
  
  if (!strcmp ((char *) gtk_object_get_data (GTK_OBJECT (widget), "operation"), 
	       "Down")) {
    gtk_clist_row_move (GTK_CLIST (pw->clist_avail), pw->row_avail, 
			pw->row_avail + 1);
    if (pw->row_avail < GTK_CLIST (pw->clist_avail)->rows - 1) pw->row_avail++;
  }


  /* We have modified the GTK_CLIST, and we will now read it to
     update the gconf string in the gconf cache */
  while (gtk_clist_get_text  (GTK_CLIST (pw->clist_avail), i, 1, &codec_name)) {
    gchar *entry = g_strconcat (codec_name, "=", gtk_clist_get_row_data (GTK_CLIST (pw->clist_avail),
									 i),
				NULL);
    gchar *temp = g_strjoin ((gconf_key) ? (":") : (""),
			     (gconf_key) ? (gconf_key) : (""), entry, NULL);
    g_free (entry);
    if (gconf_key)
      g_free (gconf_key);
    gconf_key = temp;
    i++;
  }

  gconf_client_set_string (GCONF_CLIENT (client),
			   "/apps/gnomemeeting/audio_codecs/list",
			   gconf_key, NULL);

  g_free (gconf_key);
}


/* DESCRIPTION  :  This callback is called when the user clicks
 *                 on a row of the codecs clist in the Audio Codecs Settings
 * BEHAVIOR     :  It updates the GM_pref_window_widgets * content (row_avail
 *                 field is set to the last selected row)
 * PRE          :  gpointer is a valid pointer to the GM_pref_window_widgets
 */
static void codecs_clist_row_selected_callback (GtkWidget *widget, gint row, 
						gint column, 
						GdkEventButton *event, 
						gpointer data)
{
  GM_pref_window_widgets *pw = (GM_pref_window_widgets *) data;
  pw->row_avail = row;		
}


/* DESCRIPTION  :  This callback is called when the user clicks
 *                 on a row of the ctree containing all the sections.
 * BEHAVIOR     :  It changes the notebook page to the selected one.
 * PRE          :  data is the gtk_notebook.
 */
static void menu_ctree_row_seletected_callback (GtkWidget *widget, gint row, 
						gint column, 
						GdkEventButton *event, 
						gpointer data)
{
  GtkCTreeNode *n;
  n = gtk_ctree_node_nth (GTK_CTREE (widget), row);

  if (atoi ((char *) gtk_ctree_node_get_row_data (GTK_CTREE (widget), n)) != -1)
      gtk_notebook_set_page (GTK_NOTEBOOK (data), 
			     atoi ((char *) gtk_ctree_node_get_row_data 
				   (GTK_CTREE (widget), n)));
}


/* The functions */

void gnomemeeting_init_pref_window ()
{
  gchar *node_txt [1]; 
  gchar * ctree_titles [] = {N_("Settings")};

  GtkWidget *pixmap, *event_box, *hpaned;

  GtkWidget *frame;

  /* The ctree on the left and its corresponding clist */
  GtkWidget *ctree;
  GtkCList *clist;

  /* The ctree's nodes */
  GtkCTreeNode *node, *node2, *node3;

  /* The notebook on the right */
  GtkWidget *notebook;

  /* Box inside the prefs window */
  GtkWidget *dialog_vbox;
 
  /* Get the data */
  GM_window_widgets *gw = gnomemeeting_get_main_window (gm);
  GM_pref_window_widgets *pw = gnomemeeting_get_pref_window (gm);

  gw->pref_window = gnome_dialog_new (NULL, GNOME_STOCK_BUTTON_CLOSE, NULL);
	       
  gtk_signal_connect (GTK_OBJECT(gw->pref_window), "clicked",
		      GTK_SIGNAL_FUNC(pref_window_clicked_callback), 
		      (gpointer) pw);

  gtk_window_set_title (GTK_WINDOW (gw->pref_window), 
			_("GnomeMeeting Preferences"));	

  ctree_titles [0] = gettext (ctree_titles [0]);
  ctree = gtk_ctree_new_with_titles (1, 0, ctree_titles);
  clist = GTK_CLIST (ctree);
  notebook = gtk_notebook_new ();

  dialog_vbox = GNOME_DIALOG (gw->pref_window)->vbox;

  hpaned = gtk_hpaned_new ();
  gtk_paned_pack1 (GTK_PANED (hpaned), ctree, TRUE, TRUE);
  gtk_paned_pack2 (GTK_PANED (hpaned), notebook, TRUE, TRUE);
  gtk_box_pack_start (GTK_BOX (dialog_vbox), hpaned, FALSE, FALSE, 0);

  gtk_clist_set_column_auto_resize (GTK_CLIST (clist), 0, TRUE);
  
  gtk_window_set_policy (GTK_WINDOW (gw->pref_window), FALSE, FALSE, TRUE);

  /* All the notebook pages */
  node_txt [0] = g_strdup (_("General"));
  node = gtk_ctree_insert_node (GTK_CTREE (ctree), NULL,
				NULL, node_txt, 0,
				NULL, NULL, NULL, NULL,
				FALSE, TRUE);
  gtk_ctree_node_set_row_data (GTK_CTREE (ctree),
			       node, (gpointer) "-1");
  g_free (node_txt [0]);


  node_txt [0] = g_strdup (_("Personal Data"));
  node2 = gtk_ctree_insert_node (GTK_CTREE (ctree), node, 
				 NULL, node_txt, 0,
				 NULL, NULL, NULL, NULL,
				 TRUE, FALSE);
  gtk_ctree_node_set_row_data (GTK_CTREE (ctree),
			       node2, (gpointer) "1");
  g_free (node_txt [0]);
  gnomemeeting_init_pref_window_general (notebook);


  node_txt [0] = g_strdup (_("General Settings"));
  node2 = gtk_ctree_insert_node (GTK_CTREE (ctree), node, 
				 NULL, node_txt, 0,
				 NULL, NULL, NULL, NULL,
				 TRUE, FALSE);
  gtk_ctree_node_set_row_data (GTK_CTREE (ctree),
			       node2, (gpointer) "2");
  g_free (node_txt [0]);
  gnomemeeting_init_pref_window_interface (notebook);


  node_txt [0] = g_strdup (_("Directory Settings"));
  node2 = gtk_ctree_insert_node (GTK_CTREE (ctree), node, 
				 NULL, node_txt, 0,
				 NULL, NULL, NULL, NULL,
				 TRUE, FALSE);
  gtk_ctree_node_set_row_data (GTK_CTREE (ctree),
			       node2, (gpointer) "3");
  g_free (node_txt [0]);
  gnomemeeting_init_pref_window_directories (notebook);


  node_txt [0] = g_strdup (_("Device Settings"));
  node2 = gtk_ctree_insert_node (GTK_CTREE (ctree), node, 
				 NULL, node_txt, 0,
				 NULL, NULL, NULL, NULL,
				 TRUE, FALSE);
  gtk_ctree_node_set_row_data (GTK_CTREE (ctree),
			       node2, (gpointer) "4");
  g_free (node_txt [0]);
  gnomemeeting_init_pref_window_devices (notebook);


  node_txt [0] = g_strdup (_("Codecs"));
  node = gtk_ctree_insert_node (GTK_CTREE (ctree), NULL, 
				NULL, node_txt, 0,
				NULL, NULL, NULL, NULL,
				FALSE, TRUE);
  g_free (node_txt [0]);
  gtk_ctree_node_set_row_data (GTK_CTREE (ctree),
			       node, (gpointer) "-1");
 

  node_txt [0] = g_strdup (_("Audio Codecs"));
  /* We use another node, otherwise, Video Codecs will
     have this node as parent which is not correct */
  node2 = gtk_ctree_insert_node (GTK_CTREE (ctree), node, 
				 NULL, node_txt, 0,
				 NULL, NULL, NULL, NULL,
				 TRUE, FALSE);			
  gtk_ctree_node_set_row_data (GTK_CTREE (ctree),
			       node2, (gpointer) "5");
  g_free (node_txt [0]);
  gnomemeeting_init_pref_window_audio_codecs (notebook);


  node_txt [0] = g_strdup (_("Codecs Settings"));
  node = gtk_ctree_insert_node (GTK_CTREE (ctree), node, 
				NULL, node_txt, 0,
				NULL, NULL, NULL, NULL,
				TRUE, FALSE);			
  gtk_ctree_node_set_row_data (GTK_CTREE (ctree),
			       node, (gpointer) "6");
  g_free (node_txt [0]);
  gnomemeeting_init_pref_window_codecs_settings (notebook);


  gtk_signal_connect (GTK_OBJECT (ctree), "select_row",
		      GTK_SIGNAL_FUNC (menu_ctree_row_seletected_callback), 
		      notebook);


  /* Now, add the logo as first page */
  pixmap = gnome_pixmap_new_from_file 
    (GNOMEMEETING_IMAGES "/gnomemeeting-logo.png");

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  
  event_box = gtk_event_box_new ();
  gtk_container_add (GTK_CONTAINER (frame), event_box);
  gtk_container_add (GTK_CONTAINER (event_box), 
	  	     GTK_WIDGET (pixmap));

  gtk_notebook_prepend_page (GTK_NOTEBOOK (notebook), frame, NULL);
 
  gtk_notebook_set_scrollable (GTK_NOTEBOOK (notebook), TRUE);
  gtk_notebook_popup_enable (GTK_NOTEBOOK (notebook));
  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (notebook), FALSE);
  gtk_notebook_set_page (GTK_NOTEBOOK (notebook), 0);

  gtk_clist_set_selectable (clist, 0, FALSE);
  gtk_clist_set_selectable (clist, 5, FALSE);

  gtk_signal_connect (GTK_OBJECT (gw->pref_window), "close",
		      GTK_SIGNAL_FUNC (pref_window_destroy_callback), pw);
}


/* BEHAVIOR     :  It builds the notebook page for audio codecs settings and
 *                 add it to the notebook, default values are set from the
 *                 options struct given as parameter.
 * PRE          :  parameters has to be valid
 *                 (pointer to the notebook)
 */
static void gnomemeeting_init_pref_window_audio_codecs (GtkWidget *notebook) 
{
  GtkWidget *general_frame;
  GtkWidget *frame, *label;
  GtkWidget *table;			
  GtkWidget *vbox;
  GtkWidget *button;
  GtkTooltips *tip;
  gchar *clist_data;

  gchar * clist_titles [] = {"", N_("Name"), N_("Info"), N_("Bandwidth")};

  for (int i = 1 ; i < 4 ; i++)
    clist_titles [i] = gettext (clist_titles [i]);
  
  /* Get the data */
  GM_pref_window_widgets *pw = gnomemeeting_get_pref_window (gm);
  GConfClient *client = gconf_client_get_default ();

  /* A vbox to pack the frames into it */
  vbox = gtk_vbox_new (FALSE, GNOMEMEETING_PAD_SMALL);

  general_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (general_frame), GTK_SHADOW_IN);
  gtk_container_add (GTK_CONTAINER (general_frame), vbox);

  /* The title of the notebook page */
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_OUT);
  gtk_box_pack_start (GTK_BOX (vbox), frame, 
		      FALSE, TRUE, 0);

  label = gtk_label_new (_("Audio Codecs"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label), 2, 1);
  gtk_container_add (GTK_CONTAINER (frame), label);

  /* In this table we put the frame */
  frame = gtk_frame_new (_("Available Codecs"));
  gtk_box_pack_start (GTK_BOX (vbox), frame, 
		      FALSE, FALSE, 0);

  /* Put a table in the first frame */
  table = gtk_table_new (2, 4, FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (frame), GNOMEMEETING_PAD_SMALL);
    
  
  /* Create the Available Audio Codecs Clist */	
  pw->clist_avail = gtk_clist_new_with_titles(4, clist_titles);

  gtk_clist_set_row_height (GTK_CLIST (pw->clist_avail), 18);
  gtk_clist_set_column_width (GTK_CLIST(pw->clist_avail), 0, 20);
  gtk_clist_set_column_width (GTK_CLIST(pw->clist_avail), 1, 100);
  gtk_clist_set_column_width (GTK_CLIST(pw->clist_avail), 2, 100);
  gtk_clist_set_column_width (GTK_CLIST(pw->clist_avail), 3, 100);
  gtk_clist_set_shadow_type (GTK_CLIST(pw->clist_avail), GTK_SHADOW_IN);
  
  /* Here we add the codec buts in the order they are in the config file */
  clist_data = gconf_client_get_string (client, "/apps/gnomemeeting/audio_codecs/list", NULL);

  gchar **codecs;
  codecs = g_strsplit (clist_data, ":", 0);

  for (int i = 0 ; codecs [i] != NULL ; i++) {

    gchar **couple = g_strsplit (codecs [i], "=", 0);
    gnomemeeting_codecs_list_add (pw->clist_avail, couple [0], couple [1]);
    g_strfreev (couple);
  }
  g_strfreev (codecs);
  g_free (clist_data);

  /* Callback function when a row is selected */
  gtk_signal_connect(GTK_OBJECT(pw->clist_avail), "select_row",
		     GTK_SIGNAL_FUNC(codecs_clist_row_selected_callback), 
		     (gpointer) pw);
    
  gtk_table_attach (GTK_TABLE (table), pw->clist_avail, 0, 4, 0, 1,
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
    
  /* BUTTONS */						
  /* Add */
  button = gnomemeeting_button (_("Add"), 
		       gnome_stock_new_with_icon (GNOME_STOCK_BUTTON_APPLY));  

  gtk_table_attach (GTK_TABLE (table), button, 0, 1, 1, 2,
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (codecs_clist_button_clicked_callback), 
		      (gpointer) pw);
  gtk_object_set_data (GTK_OBJECT (button), "operation", (gpointer) "Add");

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, button,
			_("Use this button to add the selected codec to the available codecs list"), NULL);
  

  /* Del */
  button = gnomemeeting_button (_("Delete"), 
		       gnome_stock_new_with_icon (GNOME_STOCK_BUTTON_CANCEL));  

  gtk_table_attach (GTK_TABLE (table), button, 1, 2, 1, 2,
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (codecs_clist_button_clicked_callback), 
		      (gpointer) pw);
  gtk_object_set_data (GTK_OBJECT (button), "operation", (gpointer) "Del");  

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, button,
			_("Use this button to remove the selected codec from the available codecs list"), NULL);
  

  /* Up */
  button = gnomemeeting_button (_("Up"), 
		       gnome_stock_new_with_icon (GNOME_STOCK_MENU_UP)); 

  gtk_table_attach (GTK_TABLE (table), button, 2, 3, 1, 2,
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (codecs_clist_button_clicked_callback), 
		      (gpointer) pw);
  gtk_object_set_data (GTK_OBJECT (button), "operation", (gpointer) "Up");

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, button,
			_("Use this button to move down the selected codec in the preference order"), NULL);

		
  /* Down */
  button = gnomemeeting_button (_("Down"), 
				gnome_stock_new_with_icon (GNOME_STOCK_MENU_DOWN)); 

  gtk_table_attach (GTK_TABLE (table), button, 3, 4, 1, 2,
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (codecs_clist_button_clicked_callback), 
		      (gpointer) pw);
  gtk_object_set_data (GTK_OBJECT (button), "operation", (gpointer) "Down");

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, button,
			_("Use this button to move up the selected codec in the preference order"), NULL);


  label = gtk_label_new (_("Audio Codecs")); 		
  gtk_notebook_append_page (GTK_NOTEBOOK(notebook),
			    general_frame, label);		
}


/* BEHAVIOR     :  It builds the notebook page for interface settings
 *                 add it to the notebook, default values are set from the
 *                 options struct given as parameter.
 * PRE          :  See init_pref_audio_codecs.
 */
static void gnomemeeting_init_pref_window_interface (GtkWidget *notebook)
{
  GtkWidget *frame, *label;
  GtkWidget *general_frame;

  GtkWidget *vbox;
  GtkWidget *table;
 
  GtkTooltips *tip;
  GConfClient *client = gconf_client_get_default ();


  /* Get the data */
  GM_pref_window_widgets *pw = gnomemeeting_get_pref_window (gm);

  vbox = gtk_vbox_new (FALSE, GNOMEMEETING_PAD_SMALL);

  general_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (general_frame), GTK_SHADOW_IN);

  gtk_container_add (GTK_CONTAINER (general_frame), vbox);

  /* The title of the notebook page */
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_OUT);
  gtk_box_pack_start (GTK_BOX (vbox), frame, 
		      FALSE, TRUE, 0);

  label = gtk_label_new (_("General Settings"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label), 2, 1);
  gtk_container_add (GTK_CONTAINER (frame), label);

  
  /* In this frame we put a vbox*/
  frame = gtk_frame_new (_("GnomeMeeting GUI"));
  gtk_box_pack_start (GTK_BOX (vbox), frame, 
		      FALSE, FALSE, 0);

  table = gtk_table_new (3, 2, FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (frame), GNOMEMEETING_PAD_SMALL);
  

  /* Show / hide splash screen at startup */
  pw->show_chat_window = gtk_check_button_new_with_label (_("Show Chat Window"));
  gtk_table_attach (GTK_TABLE (table), pw->show_chat_window, 1, 2, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);	
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pw->show_chat_window), 
				gconf_client_get_bool (client, "/apps/gnomemeeting/"
						       "view/show_chat_window",
						       0));

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->show_chat_window,
			_("If enabled, the chat window will be displayed at startup time"), NULL);
  gtk_signal_connect (GTK_OBJECT (pw->show_chat_window), "toggled",
		      GTK_SIGNAL_FUNC (toggle_changed),
		      (gpointer) "/apps/gnomemeeting/view/show_chat_window");


  /* Show / hide splash screen at startup */
  pw->show_splash = gtk_check_button_new_with_label (_("Show Splash Screen"));
  gtk_table_attach (GTK_TABLE (table), pw->show_splash, 1, 2, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);	
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pw->show_splash), 
				gconf_client_get_bool (client, "/apps/gnomemeeting/"
						       "view/show_splash",
						       0));

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->show_splash,
			_("If enabled, the splash screen will be displayed at startup time"), NULL);
  gtk_signal_connect (GTK_OBJECT (pw->show_splash), "toggled",
		      GTK_SIGNAL_FUNC (toggle_changed),
		      (gpointer) "/apps/gnomemeeting/view/show_splash");

  /* Show / hide the notebook at startup */
  pw->show_notebook = gtk_check_button_new_with_label (_("Show Control Panel"));
  gtk_table_attach (GTK_TABLE (table), pw->show_notebook, 0, 1, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);	
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pw->show_notebook), 
				gconf_client_get_bool (client, "/apps/gnomemeeting/view/show_control_panel", NULL));


  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->show_notebook,
			_("If enabled, the control panel is displayed"), NULL);

  gtk_signal_connect (GTK_OBJECT (pw->show_notebook), "toggled",
		      GTK_SIGNAL_FUNC (toggle_changed),
		      (gpointer) "/apps/gnomemeeting/view/show_control_panel");


  /* Show / hide the statusbar at startup */
  pw->show_statusbar = gtk_check_button_new_with_label (_("Show Status Bar"));
  gtk_table_attach (GTK_TABLE (table), pw->show_statusbar, 0, 1, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);	
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pw->show_statusbar), 
				gconf_client_get_bool (client, "/apps/gnomemeeting/view/show_status_bar", NULL));

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->show_statusbar,
			_("If enabled, the statusbar is displayed"), NULL);

  gtk_signal_connect (GTK_OBJECT (pw->show_statusbar), "toggled",
		      GTK_SIGNAL_FUNC (toggle_changed),
		      (gpointer) "/apps/gnomemeeting/view/show_status_bar");


  /* Show / hide the docklet */
  pw->show_docklet = 
    gtk_check_button_new_with_label (_("Show Docklet"));
  gtk_table_attach (GTK_TABLE (table), pw->show_docklet, 0, 1, 2, 3,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);	
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pw->show_docklet), 
				gconf_client_get_bool (client, "/apps/gnomemeeting/view/show_docklet", 0));

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->show_docklet,
			_("If enabled, there is support for a docklet in the Gnome or KDE panel"), NULL);

  gtk_signal_connect (GTK_OBJECT(pw->show_docklet), "toggled",
		      GTK_SIGNAL_FUNC (toggle_changed), 
		      (gpointer) "/apps/gnomemeeting/view/show_docklet");


  /* Behavior */
  frame = gtk_frame_new (_("Behavior"));
  gtk_box_pack_start (GTK_BOX (vbox), frame, 
		      FALSE, FALSE, 0);

  /* Put a table in the first frame */
  table = gtk_table_new (3, 1, FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (frame), GNOMEMEETING_PAD_SMALL);


  /* Auto Answer toggle button */   
  pw->aa = gtk_check_button_new_with_label (_("Auto Answer"));
  gtk_table_attach (GTK_TABLE (table), pw->aa, 0, 1, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);	
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pw->aa), gconf_client_get_bool (client, "/apps/gnomemeeting/general/auto_answer", 0));

  gtk_signal_connect (GTK_OBJECT(pw->aa), "toggled",
		      GTK_SIGNAL_FUNC (toggle_changed), 
		      (gpointer) "/apps/gnomemeeting/general/auto_answer");

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->aa,
			_("If enabled, incoming calls will be automatically answered"), NULL);

  
  /* DND toggle button */
  pw->dnd = gtk_check_button_new_with_label (_("Do Not Disturb"));
  gtk_table_attach (GTK_TABLE (table), pw->dnd, 0, 1, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);	
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pw->dnd), gconf_client_get_bool (client, "/apps/gnomemeeting/general/do_not_disturb", 0));

  gtk_signal_connect (GTK_OBJECT(pw->dnd), "toggled",
		      GTK_SIGNAL_FUNC (toggle_changed), 
		      (gpointer) "/apps/gnomemeeting/general/do_not_disturb");

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->dnd,
			_("If enabled, incoming calls will be automatically refused"), NULL);
  

  /* Popup display */
  pw->incoming_call_popup = gtk_check_button_new_with_label (_("Popup window"));
  gtk_table_attach (GTK_TABLE (table), pw->incoming_call_popup, 0, 1, 2, 3,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pw->incoming_call_popup), gconf_client_get_bool (client, "/apps/gnomemeeting/view/show_popup", 0));
  
  gtk_signal_connect (GTK_OBJECT(pw->incoming_call_popup), "toggled",
		      GTK_SIGNAL_FUNC (toggle_changed), 
		      (gpointer) "/apps/gnomemeeting/view/show_popup");
	
  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->incoming_call_popup,
			_("If enabled, a popup will be displayed when receiving an incoming call"), NULL);

  /* Advanced settings */
  frame = gtk_frame_new (_("H.323 Advanced Settings"));
  gtk_box_pack_start (GTK_BOX (vbox), frame, 
		      FALSE, FALSE, 0);


  /* Put a table in this frame */
  table = gtk_table_new (2, 4, FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (frame), GNOMEMEETING_PAD_SMALL);


  /* H245 Tunnelling button */				
  pw->ht = gtk_check_button_new_with_label (_("Enable H.245 Tunnelling"));
  gtk_table_attach (GTK_TABLE (table), pw->ht, 0, 2, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);	

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pw->ht), gconf_client_get_bool (client, "/apps/gnomemeeting/general/h245_tunneling", 0));
  
  gtk_signal_connect (GTK_OBJECT(pw->ht), "toggled",
		      GTK_SIGNAL_FUNC (toggle_changed), 
		      (gpointer) "/apps/gnomemeeting/general/h245_tunneling");

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->ht,
			_("This enables H.245 Tunnelling mode"), NULL);


  /* Fast Start button */	
  pw->fs = gtk_check_button_new_with_label (_("Enable Fast Start"));
  gtk_table_attach (GTK_TABLE (table), pw->fs, 0, 2, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);	

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pw->fs), gconf_client_get_bool (client, "/apps/gnomemeeting/general/fast_start", 0));
  
  gtk_signal_connect (GTK_OBJECT(pw->fs), "toggled",
		      GTK_SIGNAL_FUNC (toggle_changed), 
		      (gpointer) "/apps/gnomemeeting/general/fast_start");
  

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->fs,
			_("Connection will be established in Fast Start mode"), NULL);

  /* Play Sound */
  frame = gtk_frame_new (_("Sound"));
  gtk_box_pack_start (GTK_BOX (vbox), frame, 
		      FALSE, FALSE, 0);

  /* Put a table in the first frame */
  table = gtk_table_new (1, 2, FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (frame), GNOMEMEETING_PAD_SMALL);


  /* Incoming call button */						
  pw->incoming_call_sound = 
    gtk_check_button_new_with_label (_("Incoming Call"));
  gtk_table_attach (GTK_TABLE (table), pw->incoming_call_sound, 0, 1, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);	

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pw->incoming_call_sound), gconf_client_get_bool (client, "/apps/gnomemeeting/general/h245_tunneling", 0));
  
  gtk_signal_connect (GTK_OBJECT(pw->incoming_call_sound), "toggled",
		      GTK_SIGNAL_FUNC (toggle_changed), 
		      (gpointer) "/apps/gnomemeeting/general/incoming_call_sound");

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->incoming_call_sound,
			_("If enabled, GnomeMeeting will play a sound when receiving an incoming call (the sound to play is chosen in the Gnome Control Center)"), NULL);


  label = gtk_label_new (_("General Settings"));

  gtk_notebook_append_page (GTK_NOTEBOOK(notebook), general_frame, label);
}


/* BEHAVIOR     :  It builds the notebook page for the codecs settings and
 *                 add it to the notebook, default values are set from the
 *                 options struct given as parameter.
 * PRE          :  See init_pref_audio_codecs.
 */
static void gnomemeeting_init_pref_window_codecs_settings (GtkWidget *notebook)
{
  GtkWidget *frame, *label;
  GtkWidget *general_frame;
  GtkWidget *audio_codecs_notebook;
  GtkWidget *video_codecs_notebook;

  GtkTooltips *tip;

  GtkWidget *table;
  GtkWidget *vbox;

  /* Get the data */
  GM_pref_window_widgets *pw = gnomemeeting_get_pref_window (gm);
  GConfClient *client = gconf_client_get_default ();
		
  vbox = gtk_vbox_new (FALSE, GNOMEMEETING_PAD_SMALL);

  general_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (general_frame), GTK_SHADOW_IN);

  gtk_container_add (GTK_CONTAINER (general_frame), vbox);


  /* The title of the notebook page */
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_OUT);
  gtk_box_pack_start (GTK_BOX (vbox), frame, 
		      FALSE, TRUE, 0);

  label = gtk_label_new (_("Codecs Settings"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label), 2, 1);
  gtk_container_add (GTK_CONTAINER (frame), label);


  /*** Audio Codecs Settings ***/

  frame = gtk_frame_new (_("Audio Codecs Settings"));
  gtk_box_pack_start (GTK_BOX (vbox), frame, 
		      FALSE, FALSE, 0);

  /* Put a notebook in the frame */
  audio_codecs_notebook = gtk_notebook_new ();
  gtk_container_add (GTK_CONTAINER (frame), audio_codecs_notebook);
  gtk_container_set_border_width (GTK_CONTAINER (frame), GNOMEMEETING_PAD_SMALL);
  gtk_container_set_border_width (GTK_CONTAINER (audio_codecs_notebook), 
				  GNOMEMEETING_PAD_SMALL);

  /* Create a page for each audio codecs having settings */
  /* General Settings */
  table = gtk_table_new (2, 4, TRUE);

  label = gtk_label_new (_("Jitter Buffer Delay:"));
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);			

  int jitter_buffer_value = gconf_client_get_int (client, 
						  "/apps/gnomemeeting/audio_settings/jitter_buffer", NULL);
  pw->jitter_buffer_spin_adj = (GtkAdjustment *) 
    gtk_adjustment_new(jitter_buffer_value, 
		       20.0, 5000.0, 
		       1.0, 1.0, 1.0);

  pw->jitter_buffer = gtk_spin_button_new (pw->jitter_buffer_spin_adj, 1.0, 0);
  
  gtk_table_attach (GTK_TABLE (table), pw->jitter_buffer, 1, 2, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);			
	
  /* Connect the signal that updates the gconf cache */
  gtk_signal_connect (GTK_OBJECT (pw->jitter_buffer_spin_adj), "value-changed",
		      GTK_SIGNAL_FUNC (adjustment_changed), 
		      (gpointer) "/apps/gnomemeeting/audio_settings/jitter_buffer");

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->jitter_buffer,
			_("The jitter buffer delay to buffer audio calls (in ms)"), NULL);


  label = gtk_label_new (_("General Settings"));

  gtk_notebook_append_page (GTK_NOTEBOOK (audio_codecs_notebook),
			    table, label);

  /* GSM codec */
  table = gtk_table_new (2, 4, TRUE);

  label = gtk_label_new (_("GSM Frames"));
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);			

  int gsm_frames = gconf_client_get_int (client, 
					 "/apps/gnomemeeting/audio_settings/gsm_frames", NULL);
  pw->gsm_frames_spin_adj = (GtkAdjustment *) 
    gtk_adjustment_new(gsm_frames, 
		       1.0, 7.0, 
		       1.0, 1.0, 1.0);

  pw->gsm_frames = gtk_spin_button_new (pw->gsm_frames_spin_adj, 1.0, 0);
  
  gtk_table_attach (GTK_TABLE (table), pw->gsm_frames, 1, 2, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);

  gtk_signal_connect (GTK_OBJECT (pw->gsm_frames_spin_adj), "value-changed",
		      GTK_SIGNAL_FUNC (adjustment_changed), 
		      (gpointer) "/apps/gnomemeeting/audio_settings/gsm_frames");

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->gsm_frames,
			_("The number of frames in each transmitted GSM packet"), NULL);


  pw->gsm_sd = gtk_check_button_new_with_label (_("Silence Detection"));

  bool gsm_sd = gconf_client_get_bool (client, "/apps/gnomemeeting/audio_settings/gsm_sd", NULL);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pw->gsm_sd),
				gsm_sd);
  gtk_table_attach (GTK_TABLE (table), pw->gsm_sd, 0, 2, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);			
  gtk_signal_connect (GTK_OBJECT (pw->gsm_sd), "toggled",
		      GTK_SIGNAL_FUNC (toggle_changed),
		      (gpointer) "/apps/gnomemeeting/audio_settings/gsm_sd");

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->gsm_sd,
			_("Enable silence detection for the GSM based codecs"), NULL);


  label = gtk_label_new (_("GSM Codec Settings"));

  gtk_notebook_append_page (GTK_NOTEBOOK (audio_codecs_notebook),
			    table, label);

  /* G.711 codec */
  table = gtk_table_new (2, 4, TRUE);

  label = gtk_label_new (_("G.711 Frames"));
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);			

  int g711_frames = gconf_client_get_int (client, 
					  "/apps/gnomemeeting/audio_settings/g711_frames", NULL);
  pw->g711_frames_spin_adj = (GtkAdjustment *) 
    gtk_adjustment_new(g711_frames, 
		       11.0, 240.0, 
		       1.0, 1.0, 1.0);

  pw->g711_frames = gtk_spin_button_new (pw->g711_frames_spin_adj, 1.0, 0);
  
  gtk_table_attach (GTK_TABLE (table), pw->g711_frames, 1, 2, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);			
  gtk_signal_connect (GTK_OBJECT (pw->g711_frames_spin_adj), "value-changed",
		      GTK_SIGNAL_FUNC (adjustment_changed), 
		      (gpointer) "/apps/gnomemeeting/audio_settings/g711_frames");
 
  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->g711_frames,
			_("The number of frames in each transmitted G.711 packet"), NULL);


  pw->g711_sd = gtk_check_button_new_with_label (_("Silence Detection"));

  bool g711_sd = gconf_client_get_bool (client, "/apps/gnomemeeting/audio_settings/g711_sd", NULL);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pw->g711_sd),
				g711_sd);
  gtk_table_attach (GTK_TABLE (table), pw->g711_sd, 0, 2, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);			

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->g711_sd,
			_("Enable silence detection for the G.711 based codecs"), NULL);

  gtk_signal_connect (GTK_OBJECT (pw->g711_sd), "toggled",
		      GTK_SIGNAL_FUNC (toggle_changed),
		      (gpointer) "/apps/gnomemeeting/audio_settings/g711_sd");

  label = gtk_label_new (_("G.711 Codec Settings"));

  gtk_notebook_append_page (GTK_NOTEBOOK (audio_codecs_notebook),
			    table, label);

  /*** Video Codecs Settings ***/
  frame = gtk_frame_new (_("Video Codecs Settings"));

  gtk_box_pack_start (GTK_BOX (vbox), frame, 
		      FALSE, FALSE, 0);


  /* Put a notebook in the frame */
  video_codecs_notebook = gtk_notebook_new ();
  gtk_container_add (GTK_CONTAINER (frame), video_codecs_notebook);
  gtk_container_set_border_width (GTK_CONTAINER (frame), GNOMEMEETING_PAD_SMALL);
  gtk_container_set_border_width (GTK_CONTAINER (video_codecs_notebook), 
				  GNOMEMEETING_PAD_SMALL);

  /* Create a page for each video codecs having settings */
  /* General Settings */
  table = gtk_table_new (2, 3, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), GNOMEMEETING_PAD_SMALL);
  gtk_table_set_col_spacings (GTK_TABLE (table), GNOMEMEETING_PAD_SMALL);


  /* Enable Transmitted FPS Limitation */
  pw->fps = 
    gtk_check_button_new_with_label (_("Limit the video transmission to"));

  gtk_table_attach (GTK_TABLE (table), pw->fps, 0, 1, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);		

  GTK_TOGGLE_BUTTON (pw->fps)->active = 
    gconf_client_get_bool (client, 
			   "/apps/gnomemeeting/video_settings/enable_fps", 0);

  gtk_signal_connect (GTK_OBJECT (pw->fps),
		      "toggled",
		      GTK_SIGNAL_FUNC (toggle_changed),
		      (gpointer) "/apps/gnomemeeting/video_settings/enable_fps");

  pw->tr_fps_spin_adj = (GtkAdjustment *) 
    gtk_adjustment_new (gconf_client_get_int (client, "/apps/gnomemeeting/video_settings/tr_fps", 0), 1.0, 30.0, 1.0, 1.0, 1.0);

  pw->tr_fps = gtk_spin_button_new (pw->tr_fps_spin_adj, 1.0, 0);

  gtk_table_attach (GTK_TABLE (table), pw->tr_fps, 1, 2, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);

  label = gtk_label_new (_("frame(s) per second"));
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_RIGHT);

  gtk_table_attach (GTK_TABLE (table), label, 2, 3, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);	

  /* Connect the signal that updates the gconf cache */
  gtk_signal_connect (GTK_OBJECT (pw->tr_fps_spin_adj), "value-changed",
		      GTK_SIGNAL_FUNC (adjustment_changed), 
		      (gpointer) "/apps/gnomemeeting/video_settings/tr_fps");

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->tr_fps,
			_("Here you can set a limit to the number of frames that will be transmitted each second. This limit is set on the Video Grabber."), NULL);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->fps,
			_("Here you can enable or disable the limit on the number of transmitted frames per second."), NULL);

  
  /* Enable Video Transmission */
  pw->vid_tr = 
    gtk_check_button_new_with_label (_("Video Transmission"));

  gtk_table_attach (GTK_TABLE (table), pw->vid_tr, 0, 3, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);		

  GTK_TOGGLE_BUTTON (pw->vid_tr)->active =
    gconf_client_get_bool (client, "/apps/gnomemeeting/video_settings/enable_video_transmission", NULL);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->vid_tr,
			_("Here you can choose to enable or disable video transmission"), NULL);

  gtk_signal_connect (GTK_OBJECT (pw->vid_tr),
		      "toggled",
		      GTK_SIGNAL_FUNC (toggle_changed),
		      (gpointer) "/apps/gnomemeeting/video_settings/enable_video_transmission");

  label = gtk_label_new (_("General Settings"));

  gtk_notebook_append_page (GTK_NOTEBOOK (video_codecs_notebook),
			    table, label);


  /**** H.261 codec ****/
  table = gtk_table_new (3, 6, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), GNOMEMEETING_PAD_SMALL);
  gtk_table_set_col_spacings (GTK_TABLE (table), GNOMEMEETING_PAD_SMALL);


  /* Transmitted Video Quality */
  label = gtk_label_new (_("Transmitted Quality of"));
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_RIGHT);

  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);			
  
  pw->tr_vq_spin_adj = (GtkAdjustment *) 
    gtk_adjustment_new(gconf_client_get_int (client, "/apps/gnomemeeting/video_settings/tr_vq", 0), 1.0, 100.0, 1.0, 5.0, 1.0);

  pw->tr_vq = gtk_spin_button_new (pw->tr_vq_spin_adj, 1.0, 0);

  gtk_table_attach (GTK_TABLE (table), pw->tr_vq, 1, 2, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);			

  /* xgettext:no-c-format */
  label = gtk_label_new (_("%"));
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);

  gtk_table_attach (GTK_TABLE (table), label, 2, 3, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);			

  /* Connect the signal that updates the gconf cache */
  gtk_signal_connect (GTK_OBJECT (pw->tr_vq_spin_adj), "value-changed",
		      GTK_SIGNAL_FUNC (adjustment_changed), 
		      (gpointer) "/apps/gnomemeeting/video_settings/tr_vq");	

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->tr_vq,
			_("Here you can choose the transmitted video quality:  choose 100% on a LAN for the best quality, 1% being the worst quality"), NULL);


  /* Updated blocks / frame */
  label = gtk_label_new (_("Transmitted Background Blocks:"));
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);

  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);			
  
  pw->tr_ub_spin_adj = (GtkAdjustment *) 
    gtk_adjustment_new(gconf_client_get_int (client, "/apps/gnomemeeting/video_settings/tr_ub", 0), 2.0, 99.0, 1.0, 5.0, 1.0);

  pw->tr_ub = gtk_spin_button_new (pw->tr_ub_spin_adj, 2.0, 0);
  
  gtk_table_attach (GTK_TABLE (table), pw->tr_ub, 1, 2, 2, 3,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);			
 
  /* Connect the signal that updates the gconf cache */
  gtk_signal_connect (GTK_OBJECT (pw->tr_ub_spin_adj), "value-changed",
		      GTK_SIGNAL_FUNC (adjustment_changed), 
		      (gpointer) "/apps/gnomemeeting/video_settings/tr_ub");

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->tr_ub,
			_("Here you can choose the number of blocks (that haven't changed) transmitted with each frame. These blocks fill in the background."), NULL);


  /* Received Video Quality */
  label = gtk_label_new (_("Received Video Quality should be"));
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_RIGHT);

  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);			
  
  pw->re_vq_spin_adj = (GtkAdjustment *) 
    gtk_adjustment_new(gconf_client_get_int (client, "/apps/gnomemeeting/video_settings/re_vq", 0), 1.0, 100.0, 1.0, 5.0, 1.0);

  pw->re_vq = gtk_spin_button_new (pw->re_vq_spin_adj, 1.0, 0);


  gtk_table_attach (GTK_TABLE (table), pw->re_vq, 1, 2, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);		   

  /* xgettext:no-c-format */
  label = gtk_label_new (_("%"));
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);

  gtk_table_attach (GTK_TABLE (table), label, 2, 3, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);		    

  /* Connect the signal that updates the gconf cache */
  gtk_signal_connect (GTK_OBJECT (pw->re_vq_spin_adj), "value-changed",
		      GTK_SIGNAL_FUNC (adjustment_changed), 
		      (gpointer) "/apps/gnomemeeting/video_settings/re_vq");

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->re_vq,
			_("The video quality hint to request to the remote"), NULL);


  /* Max. video Bandwidth */
  pw->vb = 
    gtk_check_button_new_with_label (_("Limit the bandwidth to"));

  GTK_TOGGLE_BUTTON (pw->vb)->active = 
    gconf_client_get_bool (client, "/apps/gnomemeeting/video_settings/enable_vb", NULL);

  gtk_table_attach (GTK_TABLE (table), pw->vb, 3, 4, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | 0),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);		

  gtk_signal_connect (GTK_OBJECT (pw->vb), "toggled",
 		      GTK_SIGNAL_FUNC (toggle_changed), 
		      (gpointer) "/apps/gnomemeeting/video_settings/enable_vb");
  
  pw->video_bandwidth_spin_adj = 
    (GtkAdjustment *) gtk_adjustment_new(gconf_client_get_int (client, "/apps/gnomemeeting/video_settings/video_bandwidth", NULL), 2.0, 100.0, 1.0, 1.0, 1.0);

  pw->video_bandwidth = 
    gtk_spin_button_new (pw->video_bandwidth_spin_adj, 8.0, 0);
  
  gtk_table_attach (GTK_TABLE (table), pw->video_bandwidth, 4, 5, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);			

  /* Connect the signal that updates the gconf cache */
  gtk_signal_connect (GTK_OBJECT (pw->video_bandwidth_spin_adj), 
		      "value-changed",
		      GTK_SIGNAL_FUNC (adjustment_changed), 
		      (gpointer) "/apps/gnomemeeting/video_settings/video_bandwidth");

  label = gtk_label_new (_("kb/s"));
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);

  gtk_table_attach (GTK_TABLE (table), label, 5, 6, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);			
 
  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->video_bandwidth,
			_("Here you can choose the maximum bandwidth that can be used by the H.261 video codec (in kbytes/s)"), NULL);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->vb,
			_("Here you can choose to enable or disable video bandwidth limitation"), NULL);


  /* The End */
  label = gtk_label_new (_("H.261 Codec Settings"));

  gtk_notebook_append_page (GTK_NOTEBOOK (video_codecs_notebook),
			    table, label);

  label = gtk_label_new (_("Video Codecs Settings"));
  gtk_notebook_append_page (GTK_NOTEBOOK(notebook),
			    general_frame, label);

}


/* BEHAVIOR     :  It builds the notebook page for general settings and
 *                 add it to the notebook, default values are set from the
 *                 options struct given as parameter.
 * PRE          :  See init_pref_audio_codecs.
 */
void gnomemeeting_init_pref_window_general (GtkWidget *notebook)
{
  GtkWidget *frame, *label;
  GtkWidget *general_frame;

  GtkWidget *pixmap;

  GtkWidget *vbox;
  GtkWidget *table;
 
  GtkTooltips *tip;
  gchar *gconf_string;


  /* Get the data */
  GM_pref_window_widgets *pw = gnomemeeting_get_pref_window (gm);
  GConfClient *client = gconf_client_get_default ();


  vbox = gtk_vbox_new (FALSE, GNOMEMEETING_PAD_SMALL);

  general_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (general_frame), GTK_SHADOW_IN);

  gtk_container_add (GTK_CONTAINER (general_frame), vbox);

  /* The title of the notebook page */
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_OUT);
  gtk_box_pack_start (GTK_BOX (vbox), frame, 
		      FALSE, TRUE, 0);

  label = gtk_label_new (_("Personal Data"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label), 2, 1);
  
  gtk_container_add (GTK_CONTAINER (frame), label);


  /* In this table we put the frame */
  frame = gtk_frame_new (_("GnomeMeeting"));
  gtk_box_pack_start (GTK_BOX (vbox), frame, 
		      FALSE, FALSE, 0);

  table = gtk_table_new (6, 3, FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (frame), GNOMEMEETING_PAD_SMALL);
  gtk_table_set_row_spacings (GTK_TABLE (table), GNOMEMEETING_PAD_SMALL);
  gtk_table_set_col_spacings (GTK_TABLE (table), GNOMEMEETING_PAD_SMALL);

  
  /* User Name entry */
  label = gtk_label_new (_("First Name:"));
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_RIGHT);

  pw->firstname = gtk_entry_new ();
  gtk_table_attach (GTK_TABLE (table), pw->firstname, 1, 2, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK | GTK_EXPAND),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
 
  gconf_string =  gconf_client_get_string (GCONF_CLIENT (client),
					   "/apps/gnomemeeting/personal_data/firstname", NULL);
  if (gconf_string != NULL)
    gtk_entry_set_text (GTK_ENTRY (pw->firstname), gconf_string); 
  g_free (gconf_string);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->firstname,
			_("Enter your first name"), NULL);

  /* We set the key as data to be able to get the data in order to block 
     the signal in the gconf notifier */
  gtk_object_set_data (GTK_OBJECT (pw->firstname), "gconf_key",
		       (void *) "/apps/gnomemeeting/personal_data/firstname");
  gtk_signal_connect (GTK_OBJECT (pw->firstname), "changed",
		      GTK_SIGNAL_FUNC (entry_changed), 
		      (gpointer) gtk_object_get_data (GTK_OBJECT (pw->firstname),
						      "gconf_key"));


  /* Surname entry (LDAP) */
  label = gtk_label_new (_("Last Name:"));
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_RIGHT);

  pw->surname = gtk_entry_new ();
  gtk_table_attach (GTK_TABLE (table), pw->surname, 1, 2, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK | GTK_EXPAND),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);

  gconf_string =  gconf_client_get_string (GCONF_CLIENT (client),
					   "/apps/gnomemeeting/personal_data/lastname",  NULL);
  if (gconf_string != NULL)
    gtk_entry_set_text (GTK_ENTRY (pw->surname), gconf_string); 
  g_free (gconf_string);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->surname,
			_("Enter your last name"), NULL);

  /* We set the key as data to be able to get the data in order to block 
     the signal in the gconf notifier */
  gtk_object_set_data (GTK_OBJECT (pw->surname), "gconf_key",
		       (void *) "/apps/gnomemeeting/personal_data/lastname");
  gtk_signal_connect (GTK_OBJECT (pw->surname), "changed",
		      GTK_SIGNAL_FUNC (entry_changed), 
		      (gpointer) gtk_object_get_data (GTK_OBJECT (pw->surname),
						      "gconf_key"));


  /* E-mail (LDAP) */
  label = gtk_label_new (_("E-mail address:"));
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_RIGHT);

  pw->mail = gtk_entry_new();
  gtk_table_attach (GTK_TABLE (table), pw->mail, 1, 2, 2, 3,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK | GTK_EXPAND),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);

  gconf_string =  gconf_client_get_string (GCONF_CLIENT (client),
					   "/apps/gnomemeeting/personal_data/mail", NULL);
  if (gconf_string != NULL)
    gtk_entry_set_text (GTK_ENTRY (pw->mail), gconf_string); 
  g_free (gconf_string);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->mail,
			_("Enter your e-mail address"), NULL);

  /* We set the key as data to be able to get the data in order to block 
     the signal in the gconf notifier */
  gtk_object_set_data (GTK_OBJECT (pw->mail), "gconf_key",
		       (void *) "/apps/gnomemeeting/personal_data/mail");
  gtk_signal_connect (GTK_OBJECT (pw->mail), "changed",
		      GTK_SIGNAL_FUNC (entry_changed), 
		      (gpointer) gtk_object_get_data (GTK_OBJECT (pw->mail),
						      "gconf_key"));


  /* Comment (LDAP) */
  label = gtk_label_new (_("Comment:"));
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 3, 4,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_RIGHT);

  pw->comment = gtk_entry_new();
  gtk_table_attach (GTK_TABLE (table), pw->comment, 1, 2, 3, 4,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK | GTK_EXPAND),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);

  gconf_string =  gconf_client_get_string (GCONF_CLIENT (client),
					   "/apps/gnomemeeting/personal_data/comment", NULL);
  if (gconf_string != NULL)
    gtk_entry_set_text (GTK_ENTRY (pw->comment), gconf_string); 
  g_free (gconf_string);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->comment,
			_("Here you can fill in a comment about yourself for ILS directories"), NULL);

  /* We set the key as data to be able to get the data in order to block 
     the signal in the gconf notifier */
  gtk_object_set_data (GTK_OBJECT (pw->comment), "gconf_key",
		       (void *) "/apps/gnomemeeting/personal_data/comment");
  gtk_signal_connect (GTK_OBJECT (pw->comment), "changed",
		      GTK_SIGNAL_FUNC (entry_changed), 
		      (gpointer) gtk_object_get_data (GTK_OBJECT (pw->comment),
						      "gconf_key"));


  /* Location */
  label = gtk_label_new (_("Location:"));
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 4, 5,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_RIGHT);

  pw->location = gtk_entry_new();
  gtk_table_attach (GTK_TABLE (table), pw->location, 1, 2, 4, 5,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK | GTK_EXPAND),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);

  gconf_string =  gconf_client_get_string (GCONF_CLIENT (client),
					   "/apps/gnomemeeting/personal_data/location", NULL);
  if (gconf_string != NULL)
    gtk_entry_set_text (GTK_ENTRY (pw->location), gconf_string); 
  g_free (gconf_string);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->location,
			_("Where do you call from?"), NULL);

  /* We set the key as data to be able to get the data in order to block 
     the signal in the gconf notifier */
  gtk_object_set_data (GTK_OBJECT (pw->location), "gconf_key",
		       (void *) "/apps/gnomemeeting/personal_data/location");
  gtk_signal_connect (GTK_OBJECT (pw->location), "changed",
		      GTK_SIGNAL_FUNC (entry_changed), 
		      (gpointer) gtk_object_get_data (GTK_OBJECT (pw->location),
						      "gconf_key"));


  /* Try button */
  pixmap =  gnome_pixmap_new_from_xpm_d ((char **) tb_jump_to_xpm);
  pw->directory_update_button = gnomemeeting_button (_("Update"), pixmap);

  gtk_table_attach (GTK_TABLE (table),  pw->directory_update_button, 2, 3, 5, 6,
		    (GtkAttachOptions) (GTK_EXPAND),
		    (GtkAttachOptions) (GTK_EXPAND),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);

  gtk_widget_set_usize (GTK_WIDGET (pw->directory_update_button), 85, 28);

  gtk_signal_connect (GTK_OBJECT (pw->directory_update_button), "clicked",
		      GTK_SIGNAL_FUNC (personal_data_update_button_clicked), 
		      (gpointer) pw);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->directory_update_button,
			_("Click here to try your new settings and update the LDAP server you are registered to"), NULL);


  /* The End */									
  label = gtk_label_new (_("User Settings"));

  gtk_notebook_append_page (GTK_NOTEBOOK(notebook), general_frame, label);
}


/* BEHAVIOR     :  It builds the notebook page for Directories settings and
 *                 add it to the notebook, default values are set from the
 *                 options struct given as parameter.
 * PRE          :  See init_pref_audio_codecs.
 */
static void gnomemeeting_init_pref_window_directories (GtkWidget *notebook)
{
  GtkWidget *general_frame;
  GtkWidget *frame;
  GtkWidget *vbox;
  GtkWidget *table;
  GtkWidget *label;

  GtkWidget *menu, *item, *bps;

  GtkTooltips *tip;
  gchar *gconf_string;

  /* Get the data */
  GM_pref_window_widgets *pw = gnomemeeting_get_pref_window (gm);
  GConfClient *client = gconf_client_get_default ();


  vbox = gtk_vbox_new (FALSE, GNOMEMEETING_PAD_SMALL);

  general_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (general_frame), GTK_SHADOW_IN);

  gtk_container_add (GTK_CONTAINER (general_frame), vbox);

  /* The title of the notebook page */
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_OUT);
  gtk_box_pack_start (GTK_BOX (vbox), frame, 
		      FALSE, TRUE, 0);

  label = gtk_label_new (_("Directories Settings"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label), 2, 1);
  gtk_container_add (GTK_CONTAINER (frame), label);

  /* ILS settings */
  frame = gtk_frame_new (_("LDAP Directory"));
  gtk_box_pack_start (GTK_BOX (vbox), frame, 
		      FALSE, FALSE, 0);


  /* Put a table in the first frame */
  table = gtk_table_new (2, 2, FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (frame), GNOMEMEETING_PAD_SMALL);


  /* ILS directory */
  label = gtk_label_new (_("ILS Directory:"));
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_RIGHT);

  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  
  pw->ldap_server = gtk_entry_new();
  gtk_table_attach (GTK_TABLE (table), pw->ldap_server, 1, 2, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);

  gconf_string =  gconf_client_get_string (GCONF_CLIENT (client),
					   "/apps/gnomemeeting/ldap/ldap_server",
					   NULL);
  if (gconf_string != NULL)
    gtk_entry_set_text (GTK_ENTRY (pw->ldap_server), gconf_string); 

  g_free (gconf_string);

  gtk_signal_connect (GTK_OBJECT (pw->ldap_server), "changed",
		      GTK_SIGNAL_FUNC (entry_changed), 
		      (gpointer) "/apps/gnomemeeting/ldap/ldap_server");

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->ldap_server,
			_("The ILS directory to register to"), NULL);


  /* ILS port */
  /*
  label = gtk_label_new (_("LDAP Port:"));
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  
  pw->ldap_port = gtk_entry_new();
  gtk_table_attach (GTK_TABLE (table), pw->ldap_port, 1, 2, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_entry_set_text (GTK_ENTRY (pw->ldap_port), "389");

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->ldap_port,
			_("The corresponding port: 389 is the standard port"), NULL);
  */

  /* Use ILS */ 
  pw->ldap = gtk_check_button_new_with_label (_("Register"));
  gtk_table_attach (GTK_TABLE (table), pw->ldap, 0, 1, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pw->ldap), 
				gconf_client_get_bool (GCONF_CLIENT (client),
						       "/apps/gnomemeeting/ldap/register", NULL));

  gtk_signal_connect (GTK_OBJECT (pw->ldap), "toggled",
		      GTK_SIGNAL_FUNC (toggle_changed), 
		      (gpointer) "/apps/gnomemeeting/ldap/register");

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->ldap,
			_("If enabled, permit to register to the selected LDAP directory"), NULL);


  /* Gatekeeper settings */
  frame = gtk_frame_new (_("H.323 Gatekeeper Settings"));
  gtk_box_pack_start (GTK_BOX (vbox), frame, 
		      FALSE, FALSE, 0);


  /* Put a table in the first frame */
  table = gtk_table_new (4, 2, FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (frame), GNOMEMEETING_PAD_SMALL);

  /* Gatekeeper ID */
  label = gtk_label_new (_("Gatekeeper ID:"));
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_RIGHT);

  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  
  pw->gk_id = gtk_entry_new();
  gtk_table_attach (GTK_TABLE (table), pw->gk_id, 1, 2, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_entry_set_text (GTK_ENTRY (pw->gk_id), "");

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->gk_id,
			_("The Gatekeeper identifier."), NULL);


  /* Gatekeeper Host */
  label = gtk_label_new (_("Gatekeeper host:"));
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_RIGHT);

  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  
  pw->gk_host = gtk_entry_new();
  gtk_table_attach (GTK_TABLE (table), pw->gk_host, 1, 2, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_entry_set_text (GTK_ENTRY (pw->gk_host), "");

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->gk_host,
			_("The Gatekeeper host to register to."), NULL);


  /* GK registering method */ 
  label = gtk_label_new (_("Registering method:"));
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_RIGHT);

  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);

  menu = gtk_menu_new ();
  pw->gk = gtk_option_menu_new ();
  item = gtk_menu_item_new_with_label (_("Do not register to a Gatekeeper"));
  gtk_menu_append (GTK_MENU (menu), item);
  item = gtk_menu_item_new_with_label (_("Register using the Gatekeeper host"));
  gtk_menu_append (GTK_MENU (menu), item);
  item = gtk_menu_item_new_with_label (_("Register using the Gatekeeper ID"));
  gtk_menu_append (GTK_MENU (menu), item);
  item = gtk_menu_item_new_with_label (_("Try to discover the Gatekeeper"));
  gtk_menu_append (GTK_MENU (menu), item);
  gtk_option_menu_set_menu (GTK_OPTION_MENU (pw->gk), menu);
  gtk_option_menu_set_history (GTK_OPTION_MENU (pw->gk), 0);	

  gtk_table_attach (GTK_TABLE (table), pw->gk, 1, 2, 2, 3,
		    (GtkAttachOptions) (GTK_SHRINK | GTK_FILL),
		    (GtkAttachOptions) (GTK_SHRINK | GTK_FILL),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->gk,
			_("Registering method to use"), NULL);


  /* Max Used Bandwidth spin button */					
  label = gtk_label_new (_("Maximum Bandwidth:"));
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_RIGHT);

  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 3, 4,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);	
  
  pw->bps_spin_adj = (GtkAdjustment *) gtk_adjustment_new (10, 
							   1000.0, 40000.0, 
							   1.0, 100.0, 1.0);
  bps = gtk_spin_button_new (pw->bps_spin_adj, 100.0, 0);
  
  gtk_table_attach (GTK_TABLE (table), bps, 1, 2, 3, 4,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);	

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, bps,
			_("The maximum bandwidth that should be used for the communication. This bandwidth limitation will be transmitted to the gatekeeper."), NULL);

  cout << "FIX ME: GateKeeper" << endl << flush;
  /* The End */									
  label = gtk_label_new (_("ILS Settings"));

  gtk_notebook_append_page (GTK_NOTEBOOK(notebook), general_frame, label);
}


/* BEHAVIOR     :  It builds the notebook page for the device settings and
 *                 add it to the notebook, default values are set from the
 *                 options struct given as parameter.
 * PRE          :  See init_pref_audio_codecs.
 */
static void gnomemeeting_init_pref_window_devices (GtkWidget *notebook)
{
  GtkWidget *general_frame;
  GtkWidget *frame;
  GtkWidget *vbox;
  GtkWidget *table;
  GtkWidget *label;
  GtkWidget *menu1, *menu2;
  GtkWidget *item;
  GList *audio_player_devices_list = NULL;
  GList *audio_recorder_devices_list = NULL;
  GList *video_devices_list = NULL;
  gchar *gconf_string = NULL;
  GtkTooltips *tip;
  int default_present = 0;

  /* Get the data */
  GM_pref_window_widgets *pw = gnomemeeting_get_pref_window (gm);
  GM_window_widgets *gw = gnomemeeting_get_main_window (gm);
  GConfClient *client = gconf_client_get_default ();

  vbox = gtk_vbox_new (FALSE, GNOMEMEETING_PAD_SMALL);

  general_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (general_frame), GTK_SHADOW_IN);

  gtk_container_add (GTK_CONTAINER (general_frame), vbox);

  /* The title of the notebook page */
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_OUT);
  gtk_box_pack_start (GTK_BOX (vbox), frame, 
		      FALSE, FALSE, 0);

  label = gtk_label_new (_("Device Settings"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label), 2, 1);
  gtk_container_add (GTK_CONTAINER (frame), label);

  /**** Audio devices ****/
  frame = gtk_frame_new (_("Audio Devices"));
  gtk_box_pack_start (GTK_BOX (vbox), frame, 
		      FALSE, FALSE, 0);


  /* Put a table in the first frame */
  table = gtk_table_new (4, 2, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), GNOMEMEETING_PAD_SMALL);
  gtk_table_set_col_spacings (GTK_TABLE (table), GNOMEMEETING_PAD_SMALL);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (frame), GNOMEMEETING_PAD_SMALL);


  /* Audio Device */
  label = gtk_label_new (_("Player:"));
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_RIGHT);

  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);

  pw->audio_player = gtk_combo_new ();
  gtk_table_attach (GTK_TABLE (table), pw->audio_player, 1, 2, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);

  /* Build the list from the auto-detected devices */
  gconf_string =  gconf_client_get_string (GCONF_CLIENT (client), "/apps/gnomemeeting/devices/audio_player", NULL);

  for (int i = gw->audio_player_devices.GetSize () - 1; i >= 0; i--) {
  
    if (gconf_string != NULL)
      if (!strcmp (gconf_string, gw->audio_player_devices [i]))
	default_present = 1;

    audio_player_devices_list = g_list_prepend 
	(audio_player_devices_list, 
	 g_strdup (gw->audio_player_devices [i]));
  }

  gtk_combo_set_popdown_strings (GTK_COMBO (pw->audio_player), 
				 audio_player_devices_list);
  gtk_entry_set_editable (GTK_ENTRY (GTK_COMBO (pw->audio_player)->entry),
			  FALSE);

  if (default_present)
    gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (pw->audio_player)->entry),
			gconf_string);    
  g_free (gconf_string);

  gtk_object_set_data (GTK_OBJECT (pw->audio_player), "gconf_key",
		       (void *) "/apps/gnomemeeting/devices/audio_player");

  gtk_signal_connect (GTK_OBJECT (GTK_COMBO (pw->audio_player)->entry), 
		      "changed",
		      GTK_SIGNAL_FUNC (entry_changed), 
		      (gpointer) gtk_object_get_data (GTK_OBJECT (pw->audio_player), "gconf_key"));

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, GTK_COMBO (pw->audio_player)->entry, 
			_("Enter the audio player device to use"), NULL);


  /* Audio Recorder Device */
  label = gtk_label_new (_("Recorder:"));
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_RIGHT);

  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);

  pw->audio_recorder = gtk_combo_new ();
  gtk_table_attach (GTK_TABLE (table), pw->audio_recorder, 1, 2, 2, 3,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);

  gconf_string =  gconf_client_get_string (GCONF_CLIENT (client), "/apps/gnomemeeting/devices/audio_recorder", NULL);
  
  default_present = 0;

  for (int i = gw->audio_recorder_devices.GetSize () - 1; i >= 0; i--) {
  
    if (gconf_string != NULL)
      if (!strcmp (gconf_string, gw->audio_recorder_devices [i]))
	default_present = 1;

    audio_recorder_devices_list = g_list_prepend 
	(audio_recorder_devices_list, 
	 g_strdup (gw->audio_recorder_devices [i]));
  }

  gtk_combo_set_popdown_strings (GTK_COMBO (pw->audio_recorder), 
				 audio_recorder_devices_list);
  gtk_entry_set_editable (GTK_ENTRY (GTK_COMBO (pw->audio_recorder)->entry),
			  FALSE);

  if (default_present)
    gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (pw->audio_recorder)->entry),
			gconf_string);
  g_free (gconf_string);

  gtk_object_set_data (GTK_OBJECT (pw->audio_recorder), "gconf_key",
		       (void *) "/apps/gnomemeeting/devices/audio_recorder");

  gtk_signal_connect (GTK_OBJECT (GTK_COMBO (pw->audio_recorder)->entry), 
		      "changed",
		      GTK_SIGNAL_FUNC (entry_changed), 
		      (gpointer) gtk_object_get_data (GTK_OBJECT (pw->audio_recorder), "gconf_key"));

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, GTK_COMBO (pw->audio_recorder)->entry, 
			_("Enter the audio recorder device to use"), NULL);


  /* Audio Mixers */
  label = gtk_label_new (_("Player Mixer:"));
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_RIGHT);

  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);

  pw->audio_player_mixer = gtk_entry_new ();
  gtk_table_attach (GTK_TABLE (table), pw->audio_player_mixer, 1, 2, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);

  gconf_string =  gconf_client_get_string (GCONF_CLIENT (client), "/apps/gnomemeeting/devices/audio_player_mixer", NULL);
  if (gconf_string != NULL)
    gtk_entry_set_text (GTK_ENTRY (pw->audio_player_mixer), 
			gconf_string);
  g_free (gconf_string);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->audio_player_mixer,
			_("The audio mixer to use for player settings"), NULL);

  gtk_object_set_data (GTK_OBJECT (pw->audio_player_mixer), "gconf_key",
		       (void *) "/apps/gnomemeeting/devices/audio_player_mixer");
  gtk_signal_connect (GTK_OBJECT (pw->audio_player_mixer), "changed",
		      GTK_SIGNAL_FUNC (entry_changed), 
		      (gpointer) gtk_object_get_data (GTK_OBJECT (pw->audio_player_mixer), "gconf_key"));


  /* Recorder Mixer */
  label = gtk_label_new (_("Recorder Mixer:"));
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_RIGHT);

  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 3, 4,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);

  pw->audio_recorder_mixer = gtk_entry_new ();
  gtk_table_attach (GTK_TABLE (table), pw->audio_recorder_mixer, 1, 2, 3, 4,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);

  gconf_string =  gconf_client_get_string (GCONF_CLIENT (client), "/apps/gnomemeeting/devices/audio_recorder_mixer", NULL);
  if (gconf_string != NULL)
    gtk_entry_set_text (GTK_ENTRY (pw->audio_recorder_mixer), 
			gconf_string);

  g_free (gconf_string);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->audio_recorder_mixer,
			_("The audio mixer to use for recorder settings"), NULL);

  gtk_object_set_data (GTK_OBJECT (pw->audio_recorder_mixer), "gconf_key",
		       (void *) "/apps/gnomemeeting/devices/audio_recorder_mixer");
  gtk_signal_connect (GTK_OBJECT (pw->audio_recorder_mixer), "changed",
		      GTK_SIGNAL_FUNC (entry_changed), 
		      (gpointer) gtk_object_get_data (GTK_OBJECT (pw->audio_recorder_mixer), "gconf_key"));


  /**** Video device ****/
  frame = gtk_frame_new (_("Video Device"));
  gtk_box_pack_start (GTK_BOX (vbox), frame, 
		      FALSE, TRUE, 0);


  /* Put a table in the first frame */
  table = gtk_table_new (5, 2, FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (frame), GNOMEMEETING_PAD_SMALL);
  gtk_table_set_row_spacings (GTK_TABLE (table), GNOMEMEETING_PAD_SMALL);
  gtk_table_set_col_spacings (GTK_TABLE (table), GNOMEMEETING_PAD_SMALL);


  /* Video Device */
  label = gtk_label_new (_("Video Device:"));
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_RIGHT);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  
  pw->video_device = gtk_combo_new ();
  gtk_table_attach (GTK_TABLE (table), pw->video_device, 1, 2, 0, 1,
 		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
 		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
 		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
 
  /* Set the correct value from the gconf database, if the device still exists */
  gconf_string =  gconf_client_get_string (GCONF_CLIENT (client), "/apps/gnomemeeting/devices/video_recorder", NULL);
  default_present = 0;
  
  for (int i = gw->video_devices.GetSize () - 1; i >= 0; i--) {

    if (gconf_string != NULL)
      if (!strcmp (gconf_string, gw->video_devices [i]))
	default_present = 1;

    video_devices_list = 
      g_list_prepend (video_devices_list, 
		      g_strdup (gw->video_devices [i]));
  }
  
  if (default_present)
    gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (pw->video_device)->entry),
			gconf_string);
  else { /* We must fix the default, as the one already set won't work */
    gconf_client_set_string (GCONF_CLIENT (client), "/apps/gnomemeeting/devices/video_recorder",
			     gw->video_devices [0], 0);
  }

  g_free (gconf_string);

  gtk_combo_set_popdown_strings (GTK_COMBO (pw->video_device), 
				 video_devices_list);
  gtk_entry_set_editable (GTK_ENTRY (GTK_COMBO (pw->video_device)->entry),
			  FALSE);
  
  /* Set the key as data to be able to use it to block the signal */
  gtk_object_set_data (GTK_OBJECT (pw->video_device), "gconf_key",
		       (void *) "/apps/gnomemeeting/devices/video_recorder");
  
  gtk_signal_connect (GTK_OBJECT (GTK_COMBO (pw->video_device)->entry), 
		      "changed",
		      GTK_SIGNAL_FUNC (entry_changed), 
		      (gpointer) gtk_object_get_data (GTK_OBJECT (pw->video_device), "gconf_key"));
  
  
  tip = gtk_tooltips_new ();
   
  gtk_tooltips_set_tip (tip, GTK_COMBO (pw->video_device)->entry, 
 			_("Enter the video device to use"), NULL);

 
  /* Video channel spin button */					
  label = gtk_label_new (_("Video Channel:"));
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_RIGHT);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);	
  
  pw->video_channel_spin_adj = (GtkAdjustment *) 
    gtk_adjustment_new (gconf_client_get_int (client, "/apps/gnomemeeting/devices/video_channel", 0), 0.0, 10.0, 1.0, 1.0, 1.0);

  pw->video_channel = 
    gtk_spin_button_new (pw->video_channel_spin_adj, 100.0, 0);
  
  gtk_table_attach (GTK_TABLE (table), pw->video_channel, 1, 2, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->video_channel,
			_("The video channel number to use (camera, tv, ...)"), 
			NULL);

  /* Set the key as data to be able to use it to block the signal */
  gtk_object_set_data (GTK_OBJECT (pw->video_channel_spin_adj), "gconf_key",
		       (void *) "/apps/gnomemeeting/devices/video_channel");
  
  gtk_signal_connect (GTK_OBJECT (pw->video_channel_spin_adj), 
		      "value_changed",
		      GTK_SIGNAL_FUNC (adjustment_changed), 
		      (gpointer) gtk_object_get_data (GTK_OBJECT (pw->video_channel_spin_adj), "gconf_key"));
  

  /* Video Size Option Menu */
  label = gtk_label_new (_("Video Size:"));
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_RIGHT);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);			
  
  menu1 = gtk_menu_new ();
  pw->opt1 = gtk_option_menu_new ();
  item = gtk_menu_item_new_with_label (_("Small"));
  gtk_menu_append (GTK_MENU (menu1), item);
  item = gtk_menu_item_new_with_label (_("Large"));
  gtk_menu_append (GTK_MENU (menu1), item);
  gtk_option_menu_set_menu (GTK_OPTION_MENU (pw->opt1), menu1);
  gtk_option_menu_set_history (GTK_OPTION_MENU (pw->opt1), 
			       gconf_client_get_int (client, "/apps/gnomemeeting/devices/video_size", NULL));

  gtk_table_attach (GTK_TABLE (table), pw->opt1, 1, 2, 2, 3,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);

  /* We set the key as data to be able to get the data in order to block 
     the signal in the gconf notifier */
  gtk_object_set_data (GTK_OBJECT (pw->opt1), "gconf_key",
		       (void *) "/apps/gnomemeeting/devices/video_size");

  gtk_signal_connect (GTK_OBJECT (GTK_OPTION_MENU (pw->opt1)->menu), 
		      "deactivate",
 		      GTK_SIGNAL_FUNC (option_menu_changed), 
		      (gpointer) gtk_object_get_data (GTK_OBJECT (pw->opt1),
						      "gconf_key"));

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->opt1,
			_("Here you can choose the transmitted video size"), NULL);
	

  /* Video Format Option Menu */
  label = gtk_label_new (_("Video Format:"));
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_RIGHT);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 3, 4,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);			
  
  menu2 = gtk_menu_new ();
  pw->opt2 = gtk_option_menu_new ();
  item = gtk_menu_item_new_with_label ("PAL");
  gtk_menu_append (GTK_MENU (menu2), item);
  item = gtk_menu_item_new_with_label ("NTSC");
  gtk_menu_append (GTK_MENU (menu2), item);
  item = gtk_menu_item_new_with_label ("SECAM");
  gtk_menu_append (GTK_MENU (menu2), item);
  item = gtk_menu_item_new_with_label (_("Auto"));
  gtk_menu_append (GTK_MENU (menu2), item);
  gtk_option_menu_set_menu (GTK_OPTION_MENU (pw->opt2), menu2);
  gtk_option_menu_set_history (GTK_OPTION_MENU (pw->opt2), 
			       gconf_client_get_int (client, "/apps/gnomemeeting/devices/video_size", NULL));
  
  gtk_table_attach (GTK_TABLE (table), pw->opt2, 1, 2, 3, 4,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);

  /* Set the data to be able to block the signal later */
  gtk_object_set_data (GTK_OBJECT (pw->opt2), "gconf_key",
		       (void *) "/apps/gnomemeeting/devices/video_format");
  
  gtk_signal_connect (GTK_OBJECT (GTK_OPTION_MENU (pw->opt2)->menu), 
		      "deactivate",
 		      GTK_SIGNAL_FUNC (option_menu_changed), 
		      (gpointer) gtk_object_get_data (GTK_OBJECT (pw->opt2),
						      "gconf_key"));

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->opt2,
			_("Here you can choose the transmitted video format"), NULL);


  /* Enable / disable video preview */
  pw->video_preview = gtk_check_button_new_with_label (_("Video Preview"));

  gtk_table_attach (GTK_TABLE (table), pw->video_preview, 0, 1, 4, 5,
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);	
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pw->video_preview), 
				gconf_client_get_bool (client, "/apps/gnomemeeting/devices/video_preview", NULL));
 
  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->video_preview,
			_("If enabled, the video preview mode will be set at startup"),	NULL);

  gtk_signal_connect (GTK_OBJECT (pw->video_preview),
		      "toggled",
		      GTK_SIGNAL_FUNC (toggle_changed),
		      (gpointer) "/apps/gnomemeeting/devices/video_preview");


  /* The End */									
  label = gtk_label_new (_("Device Settings"));

  gtk_notebook_append_page (GTK_NOTEBOOK(notebook), general_frame, label);
}


/* Miscellaneous functions */

void 
gnomemeeting_codecs_list_add (GtkWidget *list, gchar *CodecName, 
			      gchar * Enabled)
{
  GdkPixmap *yes, *no;
  GdkBitmap *mask_yes, *mask_no;
  gchar *row_data;   /* Do not free, this is not a copy which is stored */ 

  
  row_data = (gchar *) g_malloc (3);
		
  gchar *data [4];

  gnome_stock_pixmap_gdk (GNOME_STOCK_BUTTON_APPLY,
			  NULL, &yes, &mask_yes);

  gnome_stock_pixmap_gdk (GNOME_STOCK_BUTTON_CANCEL,
			  NULL, &no, &mask_no);


  data [0] = NULL;
  data [1] = CodecName;

  if (!strcmp (CodecName, "LPC10")) {
      data [2] = g_strdup (_("Okay"));
      data [3] = g_strdup ("3.46 kb");
  }

  if (!strcmp (CodecName, "MS-GSM")) {
      data [2] = g_strdup (_("Good Quality"));
      data [3] = g_strdup ("13 kbits");
  }

  if (!strcmp (CodecName, "G.711-ALaw-64k")) {
    data [2] = g_strdup (_("Good Quality"));
    data [3] = g_strdup ("64 kbits");
  }

  if (!strcmp (CodecName, "G.711-uLaw-64k")) {
    data [2] = g_strdup (_("Good Quality"));
    data [3] = g_strdup ("64 kbits");
  }

  if (!strcmp (CodecName, "GSM-06.10")) {
    data [2] = g_strdup (_("Good Quality"));
    data [3] = g_strdup ("16.5 kbits");
  }


  gtk_clist_append (GTK_CLIST (list), (gchar **) data);
  
  /* Set the appropriate pixmap */
  if (strcmp (Enabled, "1") == 0) {
    gtk_clist_set_pixmap (GTK_CLIST (list), 
			  GTK_CLIST (list)->rows - 1, 
			  0, yes, mask_yes);
    strcpy (row_data, "1");
    gtk_clist_set_row_data (GTK_CLIST (list), 
			    GTK_CLIST (list)->rows - 1, 
			    (gpointer) row_data);
  }
  else {
    gtk_clist_set_pixmap (GTK_CLIST (list), 
			  GTK_CLIST (list)->rows - 1, 
			  0, no, mask_no);
    strcpy (row_data, "0");
    gtk_clist_set_row_data (GTK_CLIST (list), 
			    GTK_CLIST (list)->rows - 1, 
			    (gpointer) row_data);
  }
  
  g_free (data [2]);
  g_free (data [3]);
}

