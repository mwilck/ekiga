
/***************************************************************************
                          preferences.cxx  -  description
                             -------------------
    begin                : Tue Dec 26 2000
    copyright            : (C) 2000-2001 by Damien Sandras
    description          : This file contains all the functions needed to
                           create the preferences window and all its callbacks
    email                : dsandras@seconix.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "../config.h"

#include "preferences.h"
#include "videograbber.h"
#include "connection.h"
#include "config.h"
#include "main.h"
#include "common.h"
#include "ils.h"
#include "main_interface.h"
#include "audio.h"

#include <iostream.h>


/******************************************************************************/
/* Global Variables                                                           */
/******************************************************************************/

extern GtkWidget *gm;
extern GnomeMeeting *MyApp;	

/******************************************************************************/


/******************************************************************************/
/* GTK Callbacks                                                              */
/******************************************************************************/

void pref_window_clicked (GnomeDialog *widget, int button, gpointer data)
{
  options *opts;

  switch (button)
    {
      /* The user clicks on OK => save and hide */
    case 2:
      // Save things
      opts = read_config_from_struct ((GM_pref_window_widgets *) data);

      if (check_config_from_struct ((GM_pref_window_widgets *) data))
	{
	  store_config (opts);
	  apply_options (opts, (GM_pref_window_widgets *) data);
	}

      gtk_widget_hide (GTK_WIDGET (widget));
      /* opts' content is destroyed with the widgets. */
      delete (opts);
      break;

      /* The user clicks on apply => only save */
    case 1:
      opts = read_config_from_struct ((GM_pref_window_widgets *) data);

      if (check_config_from_struct ((GM_pref_window_widgets *) data))
	{
	  store_config (opts);
	  apply_options (opts, (GM_pref_window_widgets *) data);
	}

      delete (opts); /* opts' content is destroyed with the widgets */
      break;

      /* the user clicks on cancel => only hide */
    case 0:
      gtk_widget_hide (GTK_WIDGET (widget));
      break;
    }
}


gint pref_window_destroyed (GtkWidget *widget, gpointer data)
{
  gtk_widget_hide (GTK_WIDGET (widget));
  return (TRUE);
}


void button_clicked (GtkWidget *widget, gpointer data)
{ 		
  GdkPixmap *yes, *no;
  GdkBitmap *mask_yes, *mask_no;

  GM_pref_window_widgets *pw = (GM_pref_window_widgets *) data;

  gchar *row_data;   // Do not make free, this is not a copy which is stored,
                     // but the pointer itself
  
  row_data = (gchar *) g_malloc (3);
 
  gnome_stock_pixmap_gdk (GNOME_STOCK_BUTTON_APPLY,
			  NULL, &yes, &mask_yes);

  gnome_stock_pixmap_gdk (GNOME_STOCK_BUTTON_CANCEL,
			  NULL, &no, &mask_no);


  if (!strcmp ((char *) gtk_object_get_data (GTK_OBJECT (widget), "operation"), 
	       "Add"))
    {
      gtk_clist_set_pixmap (GTK_CLIST (pw->clist_avail), pw->row_avail, 
			    0, yes, mask_yes);
      strcpy (row_data, "1");
      gtk_clist_set_row_data (GTK_CLIST (pw->clist_avail), pw->row_avail, 
			      (gpointer) row_data);
    }

  if (!strcmp ((char *) gtk_object_get_data (GTK_OBJECT (widget), "operation"), 
	       "Del"))  
    {
      gtk_clist_set_pixmap (GTK_CLIST (pw->clist_avail), pw->row_avail, 
			    0, no, mask_no);
      strcpy (row_data, "0");
      gtk_clist_set_row_data (GTK_CLIST (pw->clist_avail), pw->row_avail, 
			      (gpointer) row_data);
    }
  
  if (!strcmp ((char *) gtk_object_get_data (GTK_OBJECT (widget), "operation"), 
	       "Up"))  
    {
      gtk_clist_row_move (GTK_CLIST (pw->clist_avail), pw->row_avail, 
			  pw->row_avail - 1);
      if (pw->row_avail > 0) pw->row_avail--;
    }
  
  if (!strcmp ((char *) gtk_object_get_data (GTK_OBJECT (widget), "operation"), 
	       "Down"))  
    {
      gtk_clist_row_move (GTK_CLIST (pw->clist_avail), pw->row_avail, 
			  pw->row_avail + 1);
      if (pw->row_avail < GTK_CLIST (pw->clist_avail)->rows - 1) pw->row_avail++;
    }

  pw->capabilities_changed = 1;
}


void clist_row_selected (GtkWidget *widget, gint row, 
			 gint column, GdkEventButton *event, gpointer data)
{
  GM_pref_window_widgets *pw = (GM_pref_window_widgets *) data;
  pw->row_avail = row;		
}


void ctree_row_selected (GtkWidget *widget, gint row, 
			 gint column, GdkEventButton *event, gpointer data)
{
  GtkCTreeNode *n;
  n = gtk_ctree_node_nth (GTK_CTREE (widget), row);

  if (atoi ((char *) gtk_ctree_node_get_row_data (GTK_CTREE (widget), n)) != -1)
      gtk_notebook_set_page (GTK_NOTEBOOK (data), 
			     atoi ((char *) gtk_ctree_node_get_row_data 
				   (GTK_CTREE (widget), n)));
}


void ldap_option_changed (GtkEditable *editable, gpointer data)
{
  GM_pref_window_widgets *pw = (GM_pref_window_widgets *) data;

  pw->ldap_changed = 1;
}


void audio_mixer_changed (GtkEditable *editable, gpointer data)
{
  GM_pref_window_widgets *pw = (GM_pref_window_widgets *) data;

  pw->audio_mixer_changed = 1;
}


void vid_tr_changed (GtkToggleButton *button, gpointer data)
{
  GM_pref_window_widgets *pw = (GM_pref_window_widgets *) data;

  pw->vid_tr_changed = 1;
}


void gk_option_changed (GtkWidget *widget, gpointer data)
{
  GM_pref_window_widgets *pw = (GM_pref_window_widgets *) data;

  pw->gk_changed = 1;
}
 

void vb_changed (GtkToggleButton *button, gpointer data)
{
  GM_pref_window_widgets *pw = (GM_pref_window_widgets *) data;

  int vb = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (pw->vb));

  if (vb)
    {
      gtk_widget_set_sensitive (GTK_WIDGET (pw->tr_ub_label), FALSE);
      gtk_widget_set_sensitive (GTK_WIDGET (pw->tr_vq_label), FALSE);
      gtk_widget_set_sensitive (GTK_WIDGET (pw->tr_ub), FALSE);
      gtk_widget_set_sensitive (GTK_WIDGET (pw->tr_vq), FALSE);
      gtk_widget_set_sensitive (GTK_WIDGET (pw->video_bandwidth), TRUE);
      gtk_widget_set_sensitive (GTK_WIDGET (pw->video_bandwidth_label), TRUE);

    }
  else
    {
      gtk_widget_set_sensitive (GTK_WIDGET (pw->tr_ub_label), TRUE);
      gtk_widget_set_sensitive (GTK_WIDGET (pw->tr_vq_label), TRUE);
      gtk_widget_set_sensitive (GTK_WIDGET (pw->tr_ub), TRUE);
      gtk_widget_set_sensitive (GTK_WIDGET (pw->tr_vq), TRUE);
      gtk_widget_set_sensitive (GTK_WIDGET (pw->video_bandwidth), FALSE);
      gtk_widget_set_sensitive (GTK_WIDGET (pw->video_bandwidth_label), FALSE);
    }
}
/******************************************************************************/


/******************************************************************************/
/* The functions                                                              */
/******************************************************************************/

void GM_preferences_init (int calling_state, GM_window_widgets *gw, 
			  GM_pref_window_widgets *pw, options *opts)
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
 
  gw->pref_window = gnome_dialog_new (NULL, GNOME_STOCK_BUTTON_CANCEL,
				      GNOME_STOCK_BUTTON_APPLY,
				      GNOME_STOCK_BUTTON_OK,
				      NULL);
	       
  gtk_signal_connect (GTK_OBJECT(gw->pref_window), "clicked",
		      GTK_SIGNAL_FUNC(pref_window_clicked), (gpointer) pw);

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


  node_txt [0] = g_strdup (_("User Settings"));
  node2 = gtk_ctree_insert_node (GTK_CTREE (ctree), node, 
				 NULL, node_txt, 0,
				 NULL, NULL, NULL, NULL,
				 TRUE, FALSE);
  gtk_ctree_node_set_row_data (GTK_CTREE (ctree),
			       node2, (gpointer) "1");
  g_free (node_txt [0]);
  init_pref_general (notebook, pw, calling_state, opts);	


  node_txt [0] = g_strdup (_("General Settings"));
  node2 = gtk_ctree_insert_node (GTK_CTREE (ctree), node, 
				 NULL, node_txt, 0,
				 NULL, NULL, NULL, NULL,
				 TRUE, FALSE);
  gtk_ctree_node_set_row_data (GTK_CTREE (ctree),
			       node2, (gpointer) "2");
  g_free (node_txt [0]);
  init_pref_interface (notebook, pw, calling_state, opts);


  node_txt [0] = g_strdup (_("Advanced Settings"));
  node3 = gtk_ctree_insert_node (GTK_CTREE (ctree), node, 
				 NULL, node_txt, 0,
				 NULL, NULL, NULL, NULL,
				 TRUE, FALSE);
  gtk_ctree_node_set_row_data (GTK_CTREE (ctree),
			       node3, (gpointer) "3");
  g_free (node_txt [0]);
  init_pref_advanced (notebook, pw, calling_state, opts);


  node_txt [0] = g_strdup (_("ILS Settings"));
  node2 = gtk_ctree_insert_node (GTK_CTREE (ctree), node, 
				 NULL, node_txt, 0,
				 NULL, NULL, NULL, NULL,
				 TRUE, FALSE);
  gtk_ctree_node_set_row_data (GTK_CTREE (ctree),
			       node2, (gpointer) "4");
  g_free (node_txt [0]);
  init_pref_ldap (notebook, pw, calling_state, opts);

  node_txt [0] = g_strdup (_("Gatekeeper Settings"));
  node2 = gtk_ctree_insert_node (GTK_CTREE (ctree), node, 
				 NULL, node_txt, 0,
				 NULL, NULL, NULL, NULL,
				 TRUE, FALSE);
  gtk_ctree_node_set_row_data (GTK_CTREE (ctree),
			       node2, (gpointer) "5");
  g_free (node_txt [0]);
  init_pref_gatekeeper (notebook, pw, calling_state, opts);

  node_txt [0] = g_strdup (_("Device Settings"));
  node2 = gtk_ctree_insert_node (GTK_CTREE (ctree), node, 
				 NULL, node_txt, 0,
				 NULL, NULL, NULL, NULL,
				 TRUE, FALSE);
  gtk_ctree_node_set_row_data (GTK_CTREE (ctree),
			       node2, (gpointer) "6");
  g_free (node_txt [0]);
  init_pref_devices (notebook, pw, calling_state, opts);

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
			       node2, (gpointer) "7");
  g_free (node_txt [0]);
  init_pref_audio_codecs (notebook, pw, calling_state, opts);


  node_txt [0] = g_strdup (_("Codecs Settings"));
  node = gtk_ctree_insert_node (GTK_CTREE (ctree), node, 
				NULL, node_txt, 0,
				NULL, NULL, NULL, NULL,
				TRUE, FALSE);			
  gtk_ctree_node_set_row_data (GTK_CTREE (ctree),
			       node, (gpointer) "8");
  g_free (node_txt [0]);
  init_pref_codecs_settings (notebook, pw, calling_state, opts);

  gtk_signal_connect (GTK_OBJECT (ctree), "select_row",
		      GTK_SIGNAL_FUNC (ctree_row_selected), notebook);

  // Now, add the logo as first page
  pixmap = gnome_pixmap_new_from_file 
    ("/usr/share/pixmaps/gnomemeeting-logo.png");

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
  gtk_clist_set_selectable (clist, 7, FALSE);

  gtk_signal_connect (GTK_OBJECT (gw->pref_window), "close",
		      GTK_SIGNAL_FUNC (pref_window_destroyed), pw);
}


void init_pref_audio_codecs (GtkWidget *notebook, GM_pref_window_widgets *pw,
			     int calling_state, options *opts)
{
  GtkWidget *general_frame;
  GtkWidget *frame, *label;
  GtkWidget *table;			
  GtkWidget *vbox;
  GtkWidget *button;
  GtkTooltips *tip;

  int cpt = 0;
   
  gchar * clist_titles [] = {"", N_("Name"), N_("Info"), N_("Bandwidth")};

  for (int i = 1 ; i < 4 ; i++)
    clist_titles [i] = gettext (clist_titles [i]);
  
  /* A vbox to pack the frames into it */
  vbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);

  general_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (general_frame), GTK_SHADOW_IN);
  gtk_container_add (GTK_CONTAINER (general_frame), vbox);

  /* In this table we put the frame */
  frame = gtk_frame_new (_("Available Codecs"));
  gtk_box_pack_start (GTK_BOX (vbox), frame, 
		      FALSE, FALSE, 0);

  /* Put a table in the first frame */
  table = gtk_table_new (2, 4, FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (frame), GNOME_PAD_SMALL);
    
  
  /* Create the Available Audio Codecs Clist */		 			
  pw->clist_avail = gtk_clist_new_with_titles(4, clist_titles);

  gtk_clist_set_row_height (GTK_CLIST (pw->clist_avail), 18);
  gtk_clist_set_column_width (GTK_CLIST(pw->clist_avail), 0, 20);
  gtk_clist_set_column_width (GTK_CLIST(pw->clist_avail), 1, 100);
  gtk_clist_set_column_width (GTK_CLIST(pw->clist_avail), 2, 100);
  gtk_clist_set_column_width (GTK_CLIST(pw->clist_avail), 3, 100);
  gtk_clist_set_shadow_type (GTK_CLIST(pw->clist_avail), GTK_SHADOW_IN);
  
  /* Here we add the codec buts in the order they are in the config file */
  while (cpt < 5)
    {
      add_codec (pw->clist_avail, opts->audio_codecs [cpt] [0], 
		 opts->audio_codecs [cpt] [1]);
      cpt++;
    }

  /* Callback function when a row is selected */
  gtk_signal_connect(GTK_OBJECT(pw->clist_avail), "select_row",
		     GTK_SIGNAL_FUNC(clist_row_selected), (gpointer) pw);
    
  gtk_table_attach (GTK_TABLE (table), pw->clist_avail, 0, 4, 0, 1,
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);
    
  /* BUTTONS */						
  /* Add */
  button = add_button (_("Add"), 
		       gnome_stock_new_with_icon (GNOME_STOCK_BUTTON_APPLY));  

  gtk_table_attach (GTK_TABLE (table), button, 0, 1, 1, 2,
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (button_clicked), 
		      (gpointer) pw);
  gtk_object_set_data (GTK_OBJECT (button), "operation", (gpointer) "Add");

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, button,
			_("Use this button to add the selected codec to the available codecs list"), NULL);
  

  /* Del */
  button = add_button (_("Delete"), 
		       gnome_stock_new_with_icon (GNOME_STOCK_BUTTON_CANCEL));  

  gtk_table_attach (GTK_TABLE (table), button, 1, 2, 1, 2,
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (button_clicked), 
		      (gpointer) pw);
  gtk_object_set_data (GTK_OBJECT (button), "operation", (gpointer) "Del");  

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, button,
			_("Use this button to remove the selected codec from the available codecs list"), NULL);
  

  /* Up */
  button = add_button (_("Up"), 
		       gnome_stock_new_with_icon (GNOME_STOCK_MENU_UP)); 

  gtk_table_attach (GTK_TABLE (table), button, 2, 3, 1, 2,
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (button_clicked), 
		      (gpointer) pw);
  gtk_object_set_data (GTK_OBJECT (button), "operation", (gpointer) "Up");

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, button,
			_("Use this button to move down the selected codec in the preference order"), NULL);

		
  /* Down */
  button = add_button (_("Down"), 
		       gnome_stock_new_with_icon (GNOME_STOCK_MENU_DOWN)); 

  gtk_table_attach (GTK_TABLE (table), button, 3, 4, 1, 2,
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (button_clicked), 
		      (gpointer) pw);
  gtk_object_set_data (GTK_OBJECT (button), "operation", (gpointer) "Down");

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, button,
			_("Use this button to move up the selected codec in the preference order"), NULL);

  /* Disable things  general_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (general_frame), GTK_SHADOW_IN);

  gtk_container_add (GTK_CONTAINER (general_frame), vbox);
 if we are in call */
  if (calling_state != 0)
      gtk_widget_set_sensitive (GTK_WIDGET (frame), FALSE);


  label = gtk_label_new (_("Audio Codecs")); 		
  gtk_notebook_append_page (GTK_NOTEBOOK(notebook),
			    general_frame, label);		

  if (calling_state != 0)
      gtk_widget_set_sensitive (GTK_WIDGET (label), FALSE);
}


void init_pref_interface (GtkWidget *notebook, GM_pref_window_widgets *pw,
			  int calling_state, options *opts)
{
  GtkWidget *frame, *label;
  GtkWidget *general_frame;

  GtkWidget *vbox;
  GtkWidget *table;
 
  GtkTooltips *tip;


  vbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);

  general_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (general_frame), GTK_SHADOW_IN);

  gtk_container_add (GTK_CONTAINER (general_frame), vbox);

  
  /* In this table we put the frame */
  frame = gtk_frame_new (_("GnomeMeeting Preferences"));
  gtk_box_pack_start (GTK_BOX (vbox), frame, 
		      FALSE, FALSE, 0);

  table = gtk_table_new (2, 2, TRUE);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (frame), GNOME_PAD_SMALL);
  

  /* Show / hide splash screen at startup */
  pw->show_splash = gtk_check_button_new_with_label (_("Show Splash Screen"));
  gtk_table_attach (GTK_TABLE (table), pw->show_splash, 1, 2, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);	
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pw->show_splash), 
				opts->show_splash);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->show_splash,
			_("If enabled, the splash screen will be displayed at startup time"), NULL);


  /* Show / hide the notebook at startup */
  pw->show_notebook = gtk_check_button_new_with_label (_("Show Control Panel"));
  gtk_table_attach (GTK_TABLE (table), pw->show_notebook, 0, 1, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);	
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pw->show_notebook), 
				opts->show_notebook);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->show_notebook,
			_("If enabled, the control panel is displayed"), NULL);


  /* Show / hide the statusbar at startup */
  pw->show_statusbar = gtk_check_button_new_with_label (_("Show Status Bar"));
  gtk_table_attach (GTK_TABLE (table), pw->show_statusbar, 0, 1, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);	
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pw->show_statusbar), 
				opts->show_statusbar);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->show_statusbar,
			_("If enabled, the statusbar is displayed"), NULL);


  /* Show / hide the quickbar at startup */
  pw->show_quickbar = 
    gtk_check_button_new_with_label (_("Show Quick Access Bar"));
  gtk_table_attach (GTK_TABLE (table), pw->show_quickbar, 1, 2, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);	
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pw->show_quickbar), 
				opts->show_quickbar);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->show_statusbar,
			_("If enabled, the quick access bar is displayed"), 
			NULL);


  /* Behavior */
  frame = gtk_frame_new (_("Behavior"));
  gtk_box_pack_start (GTK_BOX (vbox), frame, 
		      FALSE, FALSE, 0);

  /* Put a table in the first frame */
  table = gtk_table_new (2, 2, TRUE);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (frame), GNOME_PAD_SMALL);


  /* Auto Answer toggle button */						
  pw->aa = gtk_check_button_new_with_label (_("Auto Answer"));
  gtk_table_attach (GTK_TABLE (table), pw->aa, 0, 1, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);	
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pw->aa), opts->aa);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->aa,
			_("If enabled, incoming calls will be automatically answered"), NULL);

  
  /* DND toggle button */
  pw->dnd = gtk_check_button_new_with_label (_("Do Not Disturb"));
  gtk_table_attach (GTK_TABLE (table), pw->dnd, 0, 1, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);	
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pw->dnd), opts->dnd);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->dnd,
			_("If enabled, incoming calls will be automatically refused"), NULL);
  

  /* Popup display */
  pw->popup = gtk_check_button_new_with_label (_("Popup window"));
  gtk_table_attach (GTK_TABLE (table), pw->popup, 1, 2, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);	
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pw->popup), opts->popup);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->popup,
			_("If enabled, a popup will be displayed when receiving an incoming call"), NULL);


  /* Enable / disable video preview */
  pw->video_preview = gtk_check_button_new_with_label (_("Video Preview"));
  gtk_table_attach (GTK_TABLE (table), pw->video_preview, 1, 2, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);	
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pw->video_preview), 
				opts->video_preview);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->video_preview,
			_("If enabled, the video preview mode will be set at startup"), 
			NULL);


  /* Play Sound */
  frame = gtk_frame_new (_("Sound"));
  gtk_box_pack_start (GTK_BOX (vbox), frame, 
		      FALSE, FALSE, 0);

  /* Put a table in the first frame */
  table = gtk_table_new (1, 2, TRUE);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (frame), GNOME_PAD_SMALL);


  /* Auto Answer toggle button */						
  pw->incoming_call_sound = gtk_check_button_new_with_label (_("Incoming Call"));
  gtk_table_attach (GTK_TABLE (table), pw->incoming_call_sound, 0, 1, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);	
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pw->incoming_call_sound), 
				opts->incoming_call_sound);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->incoming_call_sound,
			_("If enabled, GnomeMeeting will play a sound when receiving an incoming call (the sound to play is chosen in the Gnome Control Center)"), NULL);



  label = gtk_label_new (_("General Settings"));

  gtk_notebook_append_page (GTK_NOTEBOOK(notebook), general_frame, label);
}


void init_pref_codecs_settings (GtkWidget *notebook, 
				GM_pref_window_widgets *pw,
				int calling_state, options *opts)
{
  GtkWidget *frame, *label;
  GtkWidget *general_frame;

  GtkWidget *menu1;  // For the Transmitted Video Size
  GtkWidget *menu2;  // For the Transmitted Video Format
  GtkWidget *re_vq;
  GtkWidget *tr_fps;		
  GtkWidget *item;
  GtkWidget *audio_codecs_notebook;
  GtkWidget *video_codecs_notebook;
  GtkWidget *jitter_buffer;
  GtkWidget *gsm_frames;
  GtkWidget *g711_frames;

  GtkTooltips *tip;

  GtkWidget *table;
  GtkWidget *table2;
  GtkWidget *vbox;
		
  vbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);

  general_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (general_frame), GTK_SHADOW_IN);

  gtk_container_add (GTK_CONTAINER (general_frame), vbox);


  /*** Audio Codecs Settings ***/

  frame = gtk_frame_new (_("Audio Codecs Settings"));
  gtk_box_pack_start (GTK_BOX (vbox), frame, 
		      FALSE, FALSE, 0);

  /* Put a notebook in the frame */
  audio_codecs_notebook = gtk_notebook_new ();
  gtk_container_add (GTK_CONTAINER (frame), audio_codecs_notebook);
  gtk_container_set_border_width (GTK_CONTAINER (frame), GNOME_PAD_SMALL);
  gtk_container_set_border_width (GTK_CONTAINER (audio_codecs_notebook), 
				  GNOME_PAD_SMALL);

  /* Create a page for each audio codecs having settings */
  /* General Settings */
  table = gtk_table_new (2, 4, TRUE);

  label = gtk_label_new (_("Jitter Buffer Delay"));
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);			

  pw->jitter_buffer_spin_adj = (GtkAdjustment *) 
    gtk_adjustment_new(opts->jitter_buffer, 
		       20.0, 5000.0, 
		       1.0, 1.0, 1.0);

  jitter_buffer = gtk_spin_button_new (pw->jitter_buffer_spin_adj, 1.0, 0);
  
  gtk_table_attach (GTK_TABLE (table), jitter_buffer, 1, 2, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);			
	

  label = gtk_label_new (_("General Settings"));

  gtk_notebook_append_page (GTK_NOTEBOOK (audio_codecs_notebook),
			    table, label);

  /* GSM codec */
  table = gtk_table_new (2, 4, TRUE);

  label = gtk_label_new (_("GSM Frames"));
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);			

  pw->gsm_frames_spin_adj = (GtkAdjustment *) 
    gtk_adjustment_new(opts->gsm_frames, 
		       1.0, 20.0, 
		       1.0, 1.0, 1.0);

  gsm_frames = gtk_spin_button_new (pw->gsm_frames_spin_adj, 1.0, 0);
  
  gtk_table_attach (GTK_TABLE (table), gsm_frames, 1, 2, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);			

  pw->gsm_sd = gtk_check_button_new_with_label (_("Silence Detection"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pw->gsm_sd),
				opts->gsm_sd);
  gtk_table_attach (GTK_TABLE (table), pw->gsm_sd, 0, 2, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);			

  label = gtk_label_new (_("GSM Codec Settings"));

  gtk_notebook_append_page (GTK_NOTEBOOK (audio_codecs_notebook),
			    table, label);

  /* G.711 codec */
  table = gtk_table_new (2, 4, TRUE);

  label = gtk_label_new (_("G.711 Frames"));
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);			

  pw->g711_frames_spin_adj = (GtkAdjustment *) 
    gtk_adjustment_new(opts->g711_frames, 
		       1.0, 40.0, 
		       1.0, 1.0, 1.0);

  g711_frames = gtk_spin_button_new (pw->g711_frames_spin_adj, 1.0, 0);
  
  gtk_table_attach (GTK_TABLE (table), g711_frames, 1, 2, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);			

  pw->g711_sd = gtk_check_button_new_with_label (_("Silence Detection"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pw->g711_sd),
				opts->g711_sd);
  gtk_table_attach (GTK_TABLE (table), pw->g711_sd, 0, 2, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);			

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
  gtk_container_set_border_width (GTK_CONTAINER (frame), GNOME_PAD_SMALL);
  gtk_container_set_border_width (GTK_CONTAINER (video_codecs_notebook), 
				  GNOME_PAD_SMALL);

  /* Create a page for each video codecs having settings */
  /* General Settings */
  table = gtk_table_new (3, 4, TRUE);

  /* Video Size Option Menu */
  label = gtk_label_new (_("Video Size:"));
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);			
  
  menu1 = gtk_menu_new ();
  pw->opt1 = gtk_option_menu_new ();
  item = gtk_menu_item_new_with_label (_("Small"));
  gtk_menu_append (GTK_MENU (menu1), item);
  item = gtk_menu_item_new_with_label (_("Large"));
  gtk_menu_append (GTK_MENU (menu1), item);
  gtk_option_menu_set_menu (GTK_OPTION_MENU (pw->opt1), menu1);
  gtk_option_menu_set_history (GTK_OPTION_MENU (pw->opt1), opts->video_size);	

  gtk_table_attach (GTK_TABLE (table), pw->opt1, 1, 2, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);		

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->opt1,
			_("Here you can choose the transmitted video size"), NULL);
	
  /* Video Format Option Menu */
  label = gtk_label_new (_("Video Format:"));
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);			
  
  menu2 = gtk_menu_new ();
  pw->opt2 = gtk_option_menu_new ();
  item = gtk_menu_item_new_with_label ("PAL");
  gtk_menu_append (GTK_MENU (menu2), item);
  item = gtk_menu_item_new_with_label ("NTSC");
  gtk_menu_append (GTK_MENU (menu2), item);
  gtk_option_menu_set_menu (GTK_OPTION_MENU (pw->opt2), menu2);
  gtk_option_menu_set_history (GTK_OPTION_MENU (pw->opt2), opts->video_format);
  
  gtk_table_attach (GTK_TABLE (table), pw->opt2, 1, 2, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);		

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->opt2,
			_("Here you can choose the transmitted video format"), NULL);

  /* Enable Video Transmission */
  pw->vid_tr = 
    gtk_check_button_new_with_label (_("Video Transmission"));

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pw->vid_tr), 
				(opts->vid_tr == 1));

  gtk_table_attach (GTK_TABLE (table), pw->vid_tr, 2, 4, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);		

  gtk_signal_connect (GTK_OBJECT (pw->vid_tr), "toggled",
		      GTK_SIGNAL_FUNC (vid_tr_changed), (gpointer) pw);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->vid_tr,
			_("Here you can choose to enable or disable video transmission"), NULL);


  label = gtk_label_new (_("General Settings"));

  gtk_notebook_append_page (GTK_NOTEBOOK (video_codecs_notebook),
			    table, label);

  /* H.261 codec */
  table = gtk_table_new (2, 4, FALSE);

  /* Transmitted Video Quality */
  pw->tr_vq_label = gtk_label_new (_("Transmitted Quality:"));
  gtk_table_attach (GTK_TABLE (table), pw->tr_vq_label, 0, 1, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);			
  
  pw->tr_vq_spin_adj = (GtkAdjustment *) gtk_adjustment_new(opts->tr_vq, 
							1.0, 31.0, 
							1.0, 1.0, 1.0);
  pw->tr_vq = gtk_spin_button_new (pw->tr_vq_spin_adj, 1.0, 0);
  
  gtk_table_attach (GTK_TABLE (table), pw->tr_vq, 1, 2, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);			
										
  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->tr_vq,
			_("Here you can choose the transmitted video quality:  choose 1 on a LAN for the best quality, 31 being the worst quality"), NULL);


  /* Updated blocks / frame */
  pw->tr_ub_label = gtk_label_new (_("Updated blocks:"));
  gtk_table_attach (GTK_TABLE (table), pw->tr_ub_label, 0, 1, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);			
  
  pw->tr_ub_spin_adj = (GtkAdjustment *) gtk_adjustment_new(opts->tr_ub, 
							2.0, 99.0, 1.0, 
							1.0, 1.0);
  pw->tr_ub = gtk_spin_button_new (pw->tr_ub_spin_adj, 2.0, 0);
  
  gtk_table_attach (GTK_TABLE (table), pw->tr_ub, 1, 2, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);			
 
  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->tr_ub,
			_("Here you can choose the number of blocks (that haven't changed) transmitted with each frame. These blocks fill in the background."), NULL);

  /* Received Video Quality */
  label = gtk_label_new (_("Received Quality:"));
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);			
  
  pw->re_vq_spin_adj = (GtkAdjustment *) gtk_adjustment_new(opts->re_vq, 
							    1.0, 31.0, 1.0, 
							    1.0, 1.0);
  re_vq = gtk_spin_button_new (pw->re_vq_spin_adj, 1.0, 0);

  gtk_table_attach (GTK_TABLE (table), re_vq, 1, 2, 2, 3,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, re_vq,
			_("The video quality to request from the remote party"), NULL);

  /* Max. video Bandwidth */
  pw->video_bandwidth_label = gtk_label_new (_("Max. Video Bandwidth:"));
  gtk_table_attach (GTK_TABLE (table), pw->video_bandwidth_label, 2, 3, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);			
  
  pw->video_bandwidth_spin_adj = 
    (GtkAdjustment *) gtk_adjustment_new(opts->video_bandwidth, 
					 2.0, 256.0, 1.0, 
					 1.0, 1.0);
  pw->video_bandwidth = gtk_spin_button_new (pw->video_bandwidth_spin_adj, 8.0, 0);
  
  gtk_table_attach (GTK_TABLE (table), pw->video_bandwidth, 3, 4, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);			
 
  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->video_bandwidth,
			_("Here you can choose the maximum bandwidth that can be used by the H.261 video codec. Choose 0 to disable it."), NULL);

  /* Enable Bandwidth Limitation */
  pw->vb = 
    gtk_check_button_new_with_label (_("Bandwidth Limitation"));

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pw->vb), 
				(opts->vb == 1));

  gtk_table_attach (GTK_TABLE (table), pw->vb, 2, 4, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);		

  gtk_signal_connect (GTK_OBJECT (pw->vb), "toggled",
 		      GTK_SIGNAL_FUNC (vb_changed), (gpointer) pw);

  if (opts->vb)
    {
      gtk_widget_set_sensitive (GTK_WIDGET (pw->tr_ub_label), FALSE);
      gtk_widget_set_sensitive (GTK_WIDGET (pw->tr_vq_label), FALSE);
      gtk_widget_set_sensitive (GTK_WIDGET (pw->tr_ub), FALSE);
      gtk_widget_set_sensitive (GTK_WIDGET (pw->tr_vq), FALSE);
    }
  else
    {
      gtk_widget_set_sensitive (GTK_WIDGET (pw->video_bandwidth), FALSE);
      gtk_widget_set_sensitive (GTK_WIDGET (pw->video_bandwidth_label), FALSE);
    }

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->vb,
			_("Here you can choose to enable or disable video bandwidth limitation"), NULL);


  label = gtk_label_new (_("H.261 Codec Settings"));

  gtk_notebook_append_page (GTK_NOTEBOOK (video_codecs_notebook),
			    table, label);

  label = gtk_label_new (_("Video Codecs Settings"));
  gtk_notebook_append_page (GTK_NOTEBOOK(notebook),
			    general_frame, label);

}


void init_pref_general (GtkWidget *notebook, GM_pref_window_widgets *pw,
			int calling_state, options *opts)
{
  GtkWidget *frame, *label;
  GtkWidget *general_frame;

  GtkWidget *vbox;
  GtkWidget *table;
 
  GtkTooltips *tip;


  vbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);

  general_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (general_frame), GTK_SHADOW_IN);

  gtk_container_add (GTK_CONTAINER (general_frame), vbox);


  /* In this table we put the frame */
  frame = gtk_frame_new (_("GnomeMeeting"));
  gtk_box_pack_start (GTK_BOX (vbox), frame, 
		      FALSE, FALSE, 0);

  table = gtk_table_new (6, 2, FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (frame), GNOME_PAD_SMALL);
  
  
  /* User Name entry */
  label = gtk_label_new (_("First Name:"));
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);
  
  pw->firstname = gtk_entry_new ();
  gtk_table_attach (GTK_TABLE (table), pw->firstname, 1, 2, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);

  gtk_entry_set_text (GTK_ENTRY (pw->firstname), opts->firstname);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->firstname,
			_("Enter your first name"), NULL);

  gtk_signal_connect (GTK_OBJECT (pw->firstname), "changed",
		      GTK_SIGNAL_FUNC (ldap_option_changed), (gpointer) pw);


  /* Surname entry (LDAP) */
  label = gtk_label_new (_("Last name:"));
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);
  
  pw->surname = gtk_entry_new ();
  gtk_table_attach (GTK_TABLE (table), pw->surname, 1, 2, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);

  gtk_entry_set_text (GTK_ENTRY (pw->surname), opts->surname);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->surname,
			_("Enter your last name"), NULL);

  gtk_signal_connect (GTK_OBJECT (pw->surname), "changed",
		      GTK_SIGNAL_FUNC (ldap_option_changed), (gpointer) pw);


  /* E-mail (LDAP) */
  label = gtk_label_new (_("E-mail address:"));
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);
  
  pw->mail = gtk_entry_new();
  gtk_table_attach (GTK_TABLE (table), pw->mail, 1, 2, 2, 3,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);

  gtk_entry_set_text (GTK_ENTRY (pw->mail), opts->mail);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->mail,
			_("Enter your e-mail address"), NULL);

  gtk_signal_connect (GTK_OBJECT (pw->mail), "changed",
		      GTK_SIGNAL_FUNC (ldap_option_changed), (gpointer) pw);


  /* Comment (LDAP) */
  label = gtk_label_new (_("Comment:"));
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 3, 4,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);
  
  pw->comment = gtk_entry_new();
  gtk_table_attach (GTK_TABLE (table), pw->comment, 1, 2, 3, 4,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);

  gtk_entry_set_text (GTK_ENTRY (pw->comment), opts->comment);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->comment,
			_("Here you can fill in a comment about yourself for ILS directories"), NULL);

  gtk_signal_connect (GTK_OBJECT (pw->comment), "changed",
		      GTK_SIGNAL_FUNC (ldap_option_changed), (gpointer) pw);

  /* Location */
  label = gtk_label_new (_("Location:"));
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 4, 5,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);
  
  pw->location = gtk_entry_new();
  gtk_table_attach (GTK_TABLE (table), pw->location, 1, 2, 4, 5,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);

  gtk_entry_set_text (GTK_ENTRY (pw->location), opts->location);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->location,
			_("Where do you call from?"), NULL);

  gtk_signal_connect (GTK_OBJECT (pw->location), "changed",
		      GTK_SIGNAL_FUNC (ldap_option_changed), (gpointer) pw);

  /* Port Entry */
  label = gtk_label_new (_("Listen Port:"));
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 5, 6,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);
  
  pw->entry_port = gtk_entry_new();
  gtk_table_attach (GTK_TABLE (table), pw->entry_port, 1, 2, 5, 6,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);
  gtk_entry_set_text (GTK_ENTRY (pw->entry_port), opts->listen_port);
    
  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->entry_port,
			_("The port that GnomeMeeting should listen on"), NULL);


  /* The End */									
  label = gtk_label_new (_("User Settings"));

  gtk_notebook_append_page (GTK_NOTEBOOK(notebook), general_frame, label);
}


void init_pref_advanced (GtkWidget *notebook, GM_pref_window_widgets *pw,
			 int calling_state, options *opts)
{
  GtkWidget *frame;
  GtkWidget *general_frame;
  GtkWidget *vbox;
  GtkWidget *table;
  GtkWidget *label;

  GtkTooltips *tip;

  GtkWidget *bps;
		
  vbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);

  general_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (general_frame), GTK_SHADOW_IN);

  gtk_container_add (GTK_CONTAINER (general_frame), vbox);


  /* Advanced Settings */
  frame = gtk_frame_new (_("Advanced Settings"));
  gtk_box_pack_start (GTK_BOX (vbox), frame, 
		      FALSE, FALSE, 0);


  /* Put a table in this second frame */
  table = gtk_table_new (2, 4, FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (frame), GNOME_PAD_SMALL);


  /* H245 Tunnelling button */				
  pw->ht = gtk_check_button_new_with_label (_("Enable H.245 Tunnelling"));
  gtk_table_attach (GTK_TABLE (table), pw->ht, 0, 2, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);	
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pw->ht), opts->ht);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->ht,
			_("This enables H.245 Tunnelling mode"), NULL);


  /* Fast Start button */							
  pw->fs = gtk_check_button_new_with_label (_("Enable Fast Start"));
  gtk_table_attach (GTK_TABLE (table), pw->fs, 0, 2, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);	
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pw->fs), opts->fs);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->fs,
			_("Connection will be established in Fast Start mode"), NULL);


  /* Max Used Bandwidth spin button */					
  label = gtk_label_new (_("Maximum Bandwidth:"));
  gtk_table_attach (GTK_TABLE (table), label, 2, 3, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);	
  
  pw->bps_spin_adj = (GtkAdjustment *) gtk_adjustment_new (opts->bps, 
							   1000.0, 40000.0, 
							   1.0, 100.0, 1.0);
  bps = gtk_spin_button_new (pw->bps_spin_adj, 100.0, 0);
  
  gtk_table_attach (GTK_TABLE (table), bps, 3, 4, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);	

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, bps,
			_("The maximum bandwidth that should be used for the communication"), NULL);


  /* Silence Detection */ 
  pw->sd = gtk_check_button_new_with_label (_("Enable Silence Detection"));
  gtk_table_attach (GTK_TABLE (table), pw->sd, 2, 4, 1, 2,
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pw->sd), (opts->sd == 1));

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->sd,
			_("Enable or disable silence detection.  If silence detection is enabled, silences will not be transmitted over the Internet."), NULL);

  /* The End */
  label = gtk_label_new (_("Advanced Settings"));

  gtk_notebook_append_page (GTK_NOTEBOOK(notebook), general_frame, label);

}


void init_pref_ldap (GtkWidget *notebook, GM_pref_window_widgets *pw,
		     int calling_state, options *opts)
{
  GtkWidget *general_frame;
  GtkWidget *frame;
  GtkWidget *vbox;
  GtkWidget *table;
  GtkWidget *label;

  GtkTooltips *tip;

  vbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);

  general_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (general_frame), GTK_SHADOW_IN);

  gtk_container_add (GTK_CONTAINER (general_frame), vbox);


  /* ILS settings */
  frame = gtk_frame_new (_("ILS Directory to register to"));
  gtk_box_pack_start (GTK_BOX (vbox), frame, 
		      FALSE, FALSE, 0);


  /* Put a table in the first frame */
  table = gtk_table_new (2, 2, FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (frame), GNOME_PAD_SMALL);


  /* ILS directory */
  label = gtk_label_new (_("ILS Directory:"));
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);
  
  pw->ldap_server = gtk_entry_new();
  gtk_table_attach (GTK_TABLE (table), pw->ldap_server, 1, 2, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);
  gtk_entry_set_text (GTK_ENTRY (pw->ldap_server), opts->ldap_server);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->ldap_server,
			_("The ILS directory to register to"), NULL);

  gtk_signal_connect (GTK_OBJECT (pw->ldap_server), "changed",
		      GTK_SIGNAL_FUNC (ldap_option_changed), (gpointer) pw);


  /* ILS port */
  label = gtk_label_new (_("ILS Port:"));
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);
  
  pw->ldap_port = gtk_entry_new();
  gtk_table_attach (GTK_TABLE (table), pw->ldap_port, 1, 2, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);
  gtk_entry_set_text (GTK_ENTRY (pw->ldap_port), opts->ldap_port);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->ldap_port,
			_("The corresponding port: 389 is the standard port"), NULL);

  gtk_signal_connect (GTK_OBJECT (pw->ldap_port), "changed",
		      GTK_SIGNAL_FUNC (ldap_option_changed), (gpointer) pw);


  /* Use ILS */ 
  pw->ldap = gtk_check_button_new_with_label (_("Register"));
  gtk_table_attach (GTK_TABLE (table), pw->ldap, 1, 2, 1, 2,
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pw->ldap), (opts->ldap == 1));

  gtk_signal_connect (GTK_OBJECT (pw->ldap), "toggled",
		      GTK_SIGNAL_FUNC (ldap_option_changed), (gpointer) pw);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->ldap,
			_("If enabled, permit to register to the selected ILS directory"), NULL);


  /* The End */									
  label = gtk_label_new (_("ILS Settings"));

  gtk_notebook_append_page (GTK_NOTEBOOK(notebook), general_frame, label);
}


void init_pref_gatekeeper (GtkWidget *notebook, GM_pref_window_widgets *pw,
			   int calling_state, options *opts)
{
  GtkWidget *general_frame;
  GtkWidget *frame;
  GtkWidget *vbox;
  GtkWidget *table;
  GtkWidget *label;
  GtkWidget *menu;
  GtkWidget *item;

  GtkTooltips *tip;

  vbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);

  general_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (general_frame), GTK_SHADOW_IN);

  gtk_container_add (GTK_CONTAINER (general_frame), vbox);


  /* Gatekeeper settings */
  frame = gtk_frame_new (_("Gatekeeper Settings"));
  gtk_box_pack_start (GTK_BOX (vbox), frame, 
		      FALSE, FALSE, 0);


  /* Put a table in the first frame */
  table = gtk_table_new (3, 2, FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (frame), GNOME_PAD_SMALL);

  /* Gatekeeper ID */
  label = gtk_label_new (_("Gatekeeper ID:"));
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);
  
  pw->gk_id = gtk_entry_new();
  gtk_table_attach (GTK_TABLE (table), pw->gk_id, 1, 2, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);
  gtk_entry_set_text (GTK_ENTRY (pw->gk_id), opts->gk_id);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->gk_id,
			_("The Gatekeeper identifier."), NULL);

  gtk_signal_connect (GTK_OBJECT (pw->gk_id), "changed",
		      GTK_SIGNAL_FUNC (gk_option_changed), (gpointer) pw);


  /* Gatekeeper Host */
  label = gtk_label_new (_("Gatekeeper host:"));
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);
  
  pw->gk_host = gtk_entry_new();
  gtk_table_attach (GTK_TABLE (table), pw->gk_host, 1, 2, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);
  gtk_entry_set_text (GTK_ENTRY (pw->gk_host), opts->gk_host);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->gk_host,
			_("The Gatekeeper host to register to."), NULL);

  gtk_signal_connect (GTK_OBJECT (pw->gk_host), "changed",
		      GTK_SIGNAL_FUNC (gk_option_changed), (gpointer) pw);


  /* GK registering method */ 
  label = gtk_label_new (_("Registering method:"));
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);

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
  gtk_option_menu_set_history (GTK_OPTION_MENU (pw->gk), opts->gk);	

  gtk_table_attach (GTK_TABLE (table), pw->gk, 1, 2, 2, 3,
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);

  gtk_signal_connect (GTK_OBJECT (GTK_OPTION_MENU (pw->gk)->menu), "deactivate",
		      GTK_SIGNAL_FUNC (gk_option_changed), (gpointer) pw);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->gk,
			_("Registering method to use"), NULL);

  /* The End */									
  label = gtk_label_new (_("Gatekeeper Settings"));

  gtk_notebook_append_page (GTK_NOTEBOOK(notebook), general_frame, label);
}


void init_pref_devices (GtkWidget *notebook, GM_pref_window_widgets *pw,
			int calling_state, options *opts)
{
  GtkWidget *general_frame;
  GtkWidget *frame;
  GtkWidget *vbox;
  GtkWidget *table;
  GtkWidget *label;
  GtkWidget *video_channel;
  GList *audio_player_devices_list = NULL;
  GList *audio_recorder_devices_list = NULL;
  GList *video_devices_list = NULL;
  GtkTooltips *tip;

  vbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);

  general_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (general_frame), GTK_SHADOW_IN);

  gtk_container_add (GTK_CONTAINER (general_frame), vbox);


  /* Audio device */
  frame = gtk_frame_new (_("Audio Devices"));
  gtk_box_pack_start (GTK_BOX (vbox), frame, 
		      FALSE, FALSE, 0);


  /* Put a table in the first frame */
  table = gtk_table_new (4, 2, FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (frame), GNOME_PAD_SMALL);


  /* Audio Device */
  label = gtk_label_new (_("Player:"));
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);

  pw->audio_player = gtk_combo_new ();
  gtk_table_attach (GTK_TABLE (table), pw->audio_player, 1, 2, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);

  for (int i = pw->gw->audio_player_devices.GetSize () - 1 ; i >= 0; i--) 
    {
      if (pw->gw->audio_player_devices [i] != PString (opts->audio_player))
	{
	  audio_player_devices_list = g_list_prepend 
	    (audio_player_devices_list, g_strdup (pw->gw->audio_player_devices [i]));
	}
     
    }

  audio_player_devices_list = g_list_prepend (audio_player_devices_list, opts->audio_player);
  gtk_combo_set_popdown_strings (GTK_COMBO (pw->audio_player), 
				 audio_player_devices_list);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, GTK_COMBO (pw->audio_player)->entry, 
			_("Enter the audio player device to use"), NULL);

  gtk_signal_connect (GTK_OBJECT(GTK_COMBO (pw->audio_player)->entry), "changed",
		      GTK_SIGNAL_FUNC (audio_mixer_changed), (gpointer) pw);


  /* Audio Recorder Device */
  label = gtk_label_new (_("Recorder:"));
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);

  pw->audio_recorder = gtk_combo_new ();
  gtk_table_attach (GTK_TABLE (table), pw->audio_recorder, 1, 2, 2, 3,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);

  for (int i = pw->gw->audio_recorder_devices.GetSize () - 1; i >= 0; i--) 
    {
      if (pw->gw->audio_recorder_devices [i] != PString (opts->audio_recorder))
	{
	  audio_recorder_devices_list = g_list_prepend 
	    (audio_recorder_devices_list, 
	     g_strdup (pw->gw->audio_recorder_devices [i]));
	}
     
    }

  audio_recorder_devices_list = g_list_prepend (audio_recorder_devices_list, 
						opts->audio_recorder);
  gtk_combo_set_popdown_strings (GTK_COMBO (pw->audio_recorder), 
				 audio_recorder_devices_list);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, GTK_COMBO (pw->audio_recorder)->entry, 
			_("Enter the audio recorder device to use"), NULL);

  gtk_signal_connect (GTK_OBJECT(GTK_COMBO (pw->audio_recorder)->entry), "changed",
		      GTK_SIGNAL_FUNC (audio_mixer_changed), (gpointer) pw);
    

  /* Audio Mixers */
  label = gtk_label_new (_("Player Mixer:"));
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);

  pw->audio_player_mixer = gtk_entry_new ();
  gtk_table_attach (GTK_TABLE (table), pw->audio_player_mixer, 1, 2, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);

  gtk_entry_set_text (GTK_ENTRY (pw->audio_player_mixer), 
		      opts->audio_player_mixer);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->audio_player_mixer,
			_("The audio mixer to use for player settings"), NULL);

  gtk_signal_connect (GTK_OBJECT (pw->audio_player_mixer), "changed",
		      GTK_SIGNAL_FUNC (audio_mixer_changed), (gpointer) pw);


  label = gtk_label_new (_("Recorder Mixer:"));
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 3, 4,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);

  pw->audio_recorder_mixer = gtk_entry_new ();
  gtk_table_attach (GTK_TABLE (table), pw->audio_recorder_mixer, 1, 2, 3, 4,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);

  gtk_entry_set_text (GTK_ENTRY (pw->audio_recorder_mixer), 
		      opts->audio_recorder_mixer);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->audio_recorder_mixer,
			_("The audio mixer to use for recorder settings"), NULL);

  gtk_signal_connect (GTK_OBJECT (pw->audio_recorder_mixer), "changed",
		      GTK_SIGNAL_FUNC (audio_mixer_changed), (gpointer) pw);


  /* Video device */
  frame = gtk_frame_new (_("Video Device"));
  gtk_box_pack_start (GTK_BOX (vbox), frame, 
		      FALSE, FALSE, 0);

  /* Put a table in the first frame */
  table = gtk_table_new (2, 2, FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (frame), GNOME_PAD_SMALL);

  /* Video Device */
  label = gtk_label_new (_("Video Device:"));
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);
  
  pw->video_device = gtk_combo_new ();
  gtk_table_attach (GTK_TABLE (table), pw->video_device, 1, 2, 0, 1,
 		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
 		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
 		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);
  
   for (int i = pw->gw->video_devices.GetSize () - 1; i >= 0; i--) 
     {
       if (pw->gw->video_devices [i] != PString (opts->video_device))
	 video_devices_list = 
	   g_list_prepend (video_devices_list, g_strdup (pw->gw->video_devices [i]));
    }
   
   video_devices_list = g_list_prepend (video_devices_list, opts->video_device);
   gtk_combo_set_popdown_strings (GTK_COMBO (pw->video_device), video_devices_list);
   
   tip = gtk_tooltips_new ();

   gtk_tooltips_set_tip (tip, GTK_COMBO (pw->video_device)->entry, 
 			_("Enter the video device to use"), NULL);
 

  gtk_signal_connect (GTK_OBJECT (GTK_COMBO (pw->video_device)->entry), "changed",
 		      GTK_SIGNAL_FUNC (vid_tr_changed), (gpointer) pw);

  /* Video channel spin button */					
  label = gtk_label_new (_("Video Channel:"));
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);	
  
  pw->video_channel_spin_adj = (GtkAdjustment *) 
    gtk_adjustment_new (opts->video_channel, 
			0.0, 10.0, 
			1.0, 1.0, 
			1.0);
  video_channel = gtk_spin_button_new (pw->video_channel_spin_adj, 100.0, 0);
  
  gtk_table_attach (GTK_TABLE (table), video_channel, 1, 2, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);	

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, video_channel,
			_("The video channel number to use (camera, tv, ...)"), NULL);

  gtk_signal_connect (GTK_OBJECT (pw->video_channel_spin_adj), "value-changed",
		      GTK_SIGNAL_FUNC (vid_tr_changed), (gpointer) pw);


  /* The End */									
  label = gtk_label_new (_("Device Settings"));

  gtk_notebook_append_page (GTK_NOTEBOOK(notebook), general_frame, label);
}

/******************************************************************************/


/******************************************************************************/
/* Miscellaneous functions                                                    */
/******************************************************************************/

GtkWidget * add_button (gchar *lbl, GtkWidget *pixmap)
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
  
  gtk_widget_set_usize (GTK_WIDGET (button), 40, 25);
  
  gtk_widget_show (pixmap);
  gtk_widget_show (label);
  gtk_widget_show (hbox2);
		
  return button;
}


void add_codec (GtkWidget *list, gchar *CodecName, gchar * Enabled)
{
  GdkPixmap *yes, *no;
  GdkBitmap *mask_yes, *mask_no;
  gchar *row_data;   // Do not make free, this is not a copy which is stored 

  
  row_data = (gchar *) g_malloc (3);
		
  gchar *data [4];

  gnome_stock_pixmap_gdk (GNOME_STOCK_BUTTON_APPLY,
			  NULL, &yes, &mask_yes);

  gnome_stock_pixmap_gdk (GNOME_STOCK_BUTTON_CANCEL,
			  NULL, &no, &mask_no);


  data [0] = NULL;
  data [1] = CodecName;

  if (!strcmp (CodecName, "LPC10"))
    {
      data [2] = g_strdup (_("Okay"));
      data [3] = g_strdup ("3.46 kb");
    }

  if (!strcmp (CodecName, "MS-GSM"))
    {
      data [2] = g_strdup (_("Good Quality"));
      data [3] = g_strdup ("13 kbits");
    }

  if (!strcmp (CodecName, "G.711-ALaw-64k"))
    {
      data [2] = g_strdup (_("Good Quality"));
      data [3] = g_strdup ("64 kbits");
    }

  if (!strcmp (CodecName, "G.711-uLaw-64k"))
    {
      data [2] = g_strdup (_("Good Quality"));
      data [3] = g_strdup ("64 kbits");
    }

  if (!strcmp (CodecName, "GSM-06.10"))
    {
      data [2] = g_strdup (_("Good Quality"));
      data [3] = g_strdup ("16.5 kbits");
    }


  gtk_clist_append (GTK_CLIST (list), (gchar **) data);
  
  // Set the appropriate pixmap
  if (strcmp (Enabled, "1") == 0)
    {
      gtk_clist_set_pixmap (GTK_CLIST (list), 
			    GTK_CLIST (list)->rows - 1, 
			    0, yes, mask_yes);
      strcpy (row_data, "1");
      gtk_clist_set_row_data (GTK_CLIST (list), 
			      GTK_CLIST (list)->rows - 1, 
			      (gpointer) row_data);
    }
  else
    {
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


void apply_options (options *opts, GM_pref_window_widgets *pw)
{
  // opts has been updated when you call this function

  GMH323EndPoint *endpoint;
  int vol_play = 0, vol_rec = 0;

  endpoint = MyApp->Endpoint ();

  // Reinitialise the endpoint settings
  // so that the opts structure of the EndPoint class
  // is up to date.
  endpoint->ReInitialise ();

  // ILS is enabled and an option has changed : register
  if ((opts->ldap) && (pw->ldap_changed))
    {
      // We register to the new ILS directory
      GMILSClient *ils_client = (GMILSClient *) endpoint->get_ils_client ();

      ils_client->ils_register ();

      pw->ldap_changed = 0;
    }

  // ILS is disabled, and an option has changed : unregister
  if ((!opts->ldap) && (pw->ldap_changed))
    {
      // We unregister to the new ILS directory
      GMILSClient *ils_client = (GMILSClient *) endpoint->get_ils_client ();

      ils_client->ils_unregister ();

      pw->ldap_changed = 0;
    }

  // Change the audio mixer source
  if (pw->audio_mixer_changed)
    {
      gchar *text;

      // We set the new mixer in the object as data, because me do not want
      // to keep it in memory
      gtk_object_remove_data (GTK_OBJECT (pw->gw->adj_play), "audio_player_mixer");
      gtk_object_set_data (GTK_OBJECT (pw->gw->adj_play), "audio_player_mixer", 
			   g_strdup (opts->audio_player_mixer));

      gtk_object_remove_data (GTK_OBJECT (pw->gw->adj_rec), "audio_recorder_mixer");
      gtk_object_set_data (GTK_OBJECT (pw->gw->adj_rec), "audio_recorder_mixer", 
			   g_strdup (opts->audio_recorder_mixer));

      // We are sure that those mixers are ok, it has been checked
      GM_volume_get (opts->audio_player_mixer, 0, &vol_play);
      GM_volume_get (opts->audio_recorder_mixer, 1, &vol_rec);

      gtk_adjustment_set_value (GTK_ADJUSTMENT (pw->gw->adj_play),
				 vol_play / 257);
      gtk_adjustment_set_value (GTK_ADJUSTMENT (pw->gw->adj_rec),
				 vol_rec / 257);
       
      // Set recording source and set micro to record
      MyApp->Endpoint()->SetSoundChannelPlayDevice(opts->audio_player);
      MyApp->Endpoint()->SetSoundChannelRecordDevice(opts->audio_recorder);

      /* Translators: This is shown in the history. */
      text = g_strdup_printf (_("Set Audio player device to %s"), opts->audio_player);
      GM_log_insert (pw->gw->log_text, text);
      g_free (text);

      /* Translators: This is shown in the history. */
      text = g_strdup_printf (_("Set Audio recorder device to %s"), 
				     opts->audio_recorder);
      GM_log_insert (pw->gw->log_text, text);
      g_free (text);

      GM_set_recording_source (opts->audio_recorder_mixer, 0); 
  
      pw->audio_mixer_changed = 0;
    }


  if (pw->vid_tr_changed)
    {
      // kept for future changements in GM.
      pw->vid_tr_changed = 0;
    }


  // Unregister from the Gatekeeper, if any, and if
  // it is needed
  if ((MyApp->Endpoint()->Gatekeeper () != NULL) && (pw->gk_changed) && (!opts->gk))
    MyApp->Endpoint()->RemoveGatekeeper ();

  // Register to the gatekeeper
  if ((opts->gk) && (pw->gk_changed))
    {
      MyApp->Endpoint()->GatekeeperRegister ();
      pw->gk_changed = 0;
    }

  // Reinitialise the capabilities
  if (pw->capabilities_changed)
    {
      MyApp->Endpoint ()->RemoveAllCapabilities ();
      MyApp->Endpoint ()->AddAudioCapabilities ();
      MyApp->Endpoint ()->AddVideoCapabilities (opts->video_size);

      pw->capabilities_changed = 0;
    }

  if (opts->video_preview)
    // it will only be set to active, if it is not already
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pw->gw->preview_button), TRUE);
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pw->gw->preview_button), FALSE);

  // Show / Hide notebook and / or statusbar
  if ( (!(opts->show_notebook)) 
       && (GTK_WIDGET_VISIBLE (GTK_WIDGET (pw->gw->main_notebook))) )
    gtk_widget_hide_all (pw->gw->main_notebook);

  if ( (opts->show_notebook)
       && (!(GTK_WIDGET_VISIBLE (GTK_WIDGET (pw->gw->main_notebook)))) )
    gtk_widget_show_all (pw->gw->main_notebook);

  if ( (!(opts->show_statusbar)) 
       && (GTK_WIDGET_VISIBLE (GTK_WIDGET (pw->gw->statusbar))) )
    gtk_widget_hide (pw->gw->statusbar);

  if ( (opts->show_statusbar)
       && (!(GTK_WIDGET_VISIBLE (GTK_WIDGET (pw->gw->statusbar)))) )
    gtk_widget_show (pw->gw->statusbar);

  if ( (!(opts->show_quickbar)) 
       && (GTK_WIDGET_VISIBLE (GTK_WIDGET (pw->gw->quickbar_frame))) )
    gtk_widget_hide_all (pw->gw->quickbar_frame);

  if ( (opts->show_quickbar)
       && (!(GTK_WIDGET_VISIBLE (GTK_WIDGET (pw->gw->quickbar_frame)))) )
    gtk_widget_show_all (pw->gw->quickbar_frame);

}

/******************************************************************************/
