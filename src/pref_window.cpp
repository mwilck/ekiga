
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
 *                         preferences.cxx  -  description
 *                        -------------------
 *   begin                : Tue Dec 26 2000
 *   copyright            : (C) 2000-2001 by Damien Sandras
 *   description          : This file contains all the functions needed to
 *                          create the preferences window and all its callbacks
 *   email                : dsandras@seconix.com
 *
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
#include "docklet.h"
#include "misc.h"


/* Declarations */

extern GtkWidget *gm;
extern GnomeMeeting *MyApp;	

static void pref_window_clicked_callback (GnomeDialog *, int, gpointer);
static gint pref_window_destroy_callback (GtkWidget *, gpointer);
static void codecs_clist_button_clicked_callback (GtkWidget *, gpointer);
static void codecs_clist_row_selected_callback (GtkWidget *, gint, gint, 
						GdkEventButton *, gpointer);
static void menu_ctree_row_seletected_callback (GtkWidget *, gint, gint, 
						GdkEventButton *, gpointer);
static void ldap_option_changed_callback (GtkEditable *, gpointer);
static void audio_option_changed_callback (GtkEditable *, gpointer);
static void video_transmission_option_changed_callback (GtkToggleButton *, 
							gpointer);
static void video_bandwidth_limit_option_changed_callback (GtkToggleButton *, 
							   gpointer);
static void fps_limit_option_changed_callback (GtkToggleButton *, 
					       gpointer);
static void gatekeeper_option_changed (GtkWidget *, gpointer);
static void gatekeeper_option_type_changed_callback (GtkWidget *, gpointer);
static void audio_codecs_option_changed_callback (GtkAdjustment *, gpointer);

static void gnomemeeting_init_pref_window_general (GtkWidget *, 
						   int, options *);
static void gnomemeeting_init_pref_window_interface (GtkWidget *, int, 
						     options *);
static void gnomemeeting_init_pref_window_advanced (GtkWidget *, int, 
						    options *);
static void gnomemeeting_init_pref_window_ldap (GtkWidget *, int, options *);
static void gnomemeeting_init_pref_window_gatekeeper (GtkWidget *, int, 
						      options *);
static void gnomemeeting_init_pref_window_devices (GtkWidget *, int, 
						   options *);
static void gnomemeeting_init_pref_window_audio_codecs (GtkWidget *, int, 
							options *);
static void gnomemeeting_init_pref_window_codecs_settings (GtkWidget *, int, 
							   options *);
static void apply_options (options *);
static void add_codec (GtkWidget *, gchar *, gchar *);


/* GTK Callbacks */

/* DESCRIPTION  :  This callback is called when the user clicks on a button : 
 *                 OK, CANCEL, APPLY.
 * BEHAVIOR     :  OK closes (hides) the window and save settings, APPLY
 *                 applies the settings, and cancel do nothing except hiding
 *                 the window.
 * PRE          :  gpointer is a valid pointer to a GM_pref_window_widgets.
 */
static void pref_window_clicked_callback (GnomeDialog *widget, int button, 
					  gpointer data)
{
  options *opts;

  switch (button) {
    /* The user clicks on OK => save and hide */
  case 2:
    /* Save things */
    opts = gnomemeeting_read_config_from_struct ();
    
    if (gnomemeeting_check_config_from_struct ()) {
      gnomemeeting_store_config (opts);
      apply_options (opts);
    }
    
    gtk_widget_hide (GTK_WIDGET (widget));
    /* opts' content is destroyed with the widgets. */
    delete (opts);
    break;
    
    /* The user clicks on apply => only save */
  case 1:
    opts = gnomemeeting_read_config_from_struct ();
    
    if (gnomemeeting_check_config_from_struct ()) {
      gnomemeeting_store_config (opts);
      apply_options (opts);
    }
    
    delete (opts); /* opts' content is destroyed with the widgets */
    break;

    /* the user clicks on cancel => only hide */
  case 0:
    gtk_widget_hide (GTK_WIDGET (widget));
    break;
  }
}


/* DESCRIPTION  :  This callback is called when the pref window is destroyed.
 * BEHAVIOR     :  Prevents the destroy, only hides the window.
 * PRE          :  //
 */
static gint pref_window_destroy_callback (GtkWidget *widget, gpointer data)
{
  gtk_widget_hide (GTK_WIDGET (widget));
  return (TRUE);
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
	       "Add")) {
    gtk_clist_set_pixmap (GTK_CLIST (pw->clist_avail), pw->row_avail, 
			  0, yes, mask_yes);
    strcpy (row_data, "1");
    gtk_clist_set_row_data (GTK_CLIST (pw->clist_avail), pw->row_avail, 
			    (gpointer) row_data);
  }

  if (!strcmp ((char *) gtk_object_get_data (GTK_OBJECT (widget), "operation"), 
	       "Del")) {
    gtk_clist_set_pixmap (GTK_CLIST (pw->clist_avail), pw->row_avail, 
			  0, no, mask_no);
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

  pw->capabilities_changed = 1;
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


/* DESCRIPTION  :  This callback is called when the user changes
 *                 a ldap related option.
 * BEHAVIOR     :  It sets a flag to say that LDAP has to be reactivated.
 * PRE          :  gpointer is a valid pointer to a GM_pref_window_widgets.
 */
static void ldap_option_changed_callback (GtkEditable *editable, gpointer data)
{
  GM_pref_window_widgets *pw = (GM_pref_window_widgets *) data;

  pw->ldap_changed = 1;
}


/* DESCRIPTION  :  This callback is called when the user changes
 *                 one of the audio related devices.
 * BEHAVIOR     :  It sets a flag.
 * PRE          :  gpointer is a valid pointer to a GM_pref_window_widgets.
 */
static void audio_option_changed_callback (GtkEditable *editable, gpointer data)
{
  GM_pref_window_widgets *pw = (GM_pref_window_widgets *) data;

  pw->audio_mixer_changed = 1;
}


/* DESCRIPTION  :  This callback is called when the user clicks
 *                 on the video transmission toggle button, or change the video
 *                 device, or the video channel or any other video option.
 * BEHAVIOR     :  It sets a flag.
 * PRE          :  gpointer is a valid pointer to a GM_pref_window_widgets.
 */
static void video_transmission_option_changed_callback (GtkToggleButton *button,
							gpointer data)
{
  GM_pref_window_widgets *pw = (GM_pref_window_widgets *) data;
  pw->vid_tr_changed = 1;
}


/* DESCRIPTION  :  This callback is called when the user changes
 *                 any gatekeeper option.
 * BEHAVIOR     :  It sets a flag and changes the state of some widgets
 *                 following the registering method.
 * PRE          :  gpointer is a valid pointer to a GM_pref_window_widgets.
 */
static void gatekeeper_option_changed_callback (GtkWidget *widget, 
						gpointer data)
{
  GM_pref_window_widgets *pw = (GM_pref_window_widgets *) data;

  pw->gk_changed = 1;
}
 

/* DESCRIPTION  :  This callback is called when the user enables or disables
 *                 the video bandwidth limitation.
 * BEHAVIOR     :  It enables/disables other conflicting settings.
 * PRE          :  gpointer is a valid pointer to a GM_pref_window_widgets.
 */
static void video_bandwidth_limit_option_changed_callback (GtkToggleButton *button, gpointer data)
{
  GM_pref_window_widgets *pw = (GM_pref_window_widgets *) data;

  int vb = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (pw->vb));

  if (vb) {
    gtk_widget_set_sensitive (GTK_WIDGET (pw->tr_ub_label), FALSE);
    gtk_widget_set_sensitive (GTK_WIDGET (pw->tr_vq_label), FALSE);
    gtk_widget_set_sensitive (GTK_WIDGET (pw->tr_ub), FALSE);
    gtk_widget_set_sensitive (GTK_WIDGET (pw->tr_vq), FALSE);
    gtk_widget_set_sensitive (GTK_WIDGET (pw->video_bandwidth), TRUE);
    gtk_widget_set_sensitive (GTK_WIDGET (pw->video_bandwidth_label), TRUE);
  }
  else {
    gtk_widget_set_sensitive (GTK_WIDGET (pw->tr_ub_label), TRUE);
    gtk_widget_set_sensitive (GTK_WIDGET (pw->tr_vq_label), TRUE);
    gtk_widget_set_sensitive (GTK_WIDGET (pw->tr_ub), TRUE);
    gtk_widget_set_sensitive (GTK_WIDGET (pw->tr_vq), TRUE);
    gtk_widget_set_sensitive (GTK_WIDGET (pw->video_bandwidth), FALSE);
    gtk_widget_set_sensitive (GTK_WIDGET (pw->video_bandwidth_label), FALSE);
  }
}


/* DESCRIPTION  :  This callback is called when the user enables or disables
 *                 the fps limitation.
 * BEHAVIOR     :  It enables/disables other conflicting settings.
 * PRE          :  gpointer is a valid pointer to a GM_pref_window_widgets.
 */
static void fps_limit_option_changed_callback (GtkToggleButton *button, gpointer data)
{
  GM_pref_window_widgets *pw = (GM_pref_window_widgets *) data;

  int fps = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (pw->fps));

  if (fps) {

    gtk_widget_set_sensitive (GTK_WIDGET (pw->tr_fps_label), TRUE);
    gtk_widget_set_sensitive (GTK_WIDGET (pw->tr_fps), TRUE);
  }
  else {

    gtk_widget_set_sensitive (GTK_WIDGET (pw->tr_fps_label), FALSE);
    gtk_widget_set_sensitive (GTK_WIDGET (pw->tr_fps), FALSE);
  }

  pw->vid_tr_changed = 1;
}


/* DESCRIPTION  :  This callback is called when the user changes the gk
 *                 discover type.
 * BEHAVIOR     :  It enables/disables other conflicting settings.
 * PRE          :  gpointer is a valid pointer to a GM_pref_window_widgets.
 */
static void gatekeeper_option_type_changed_callback (GtkWidget *w, gpointer data)
{
  GM_pref_window_widgets *pw = (GM_pref_window_widgets *) data;

  GtkWidget *active_item;
  gint item_index;

  active_item = gtk_menu_get_active (GTK_MENU 
				     (GTK_OPTION_MENU (pw->gk)->menu));
  item_index = g_list_index (GTK_MENU_SHELL 
			     (GTK_OPTION_MENU (pw->gk)->menu)->children, 
			      active_item);
  
  if (item_index == 0) {
  
    gtk_widget_set_sensitive (GTK_WIDGET (pw->bps_frame), FALSE);
  }
  else {

    gtk_widget_set_sensitive (GTK_WIDGET (pw->bps_frame), TRUE);
  }

  if ((item_index == 0) || (item_index == 3)) {

      gtk_widget_set_sensitive (GTK_WIDGET (pw->gk_host), FALSE);
      gtk_widget_set_sensitive (GTK_WIDGET (pw->gk_host_label), FALSE);
      gtk_widget_set_sensitive (GTK_WIDGET (pw->gk_id), FALSE);
      gtk_widget_set_sensitive (GTK_WIDGET (pw->gk_id_label), FALSE);
  }

  if (item_index == 1) {
    
      gtk_widget_set_sensitive (GTK_WIDGET (pw->gk_host), TRUE);
      gtk_widget_set_sensitive (GTK_WIDGET (pw->gk_host_label), TRUE);
      gtk_widget_set_sensitive (GTK_WIDGET (pw->gk_id), FALSE);
      gtk_widget_set_sensitive (GTK_WIDGET (pw->gk_id_label), FALSE);
  }

  if (item_index == 2) {
    
      gtk_widget_set_sensitive (GTK_WIDGET (pw->gk_host), FALSE);
      gtk_widget_set_sensitive (GTK_WIDGET (pw->gk_host_label), FALSE);
      gtk_widget_set_sensitive (GTK_WIDGET (pw->gk_id), TRUE);
      gtk_widget_set_sensitive (GTK_WIDGET (pw->gk_id_label), TRUE);
  }

  pw->gk_changed = 1;
}


/* DESCRIPTION  :  This callback is called when the user changes any
 *                 audio codec related option that needs to reinit
 *                 the capablities.
 * BEHAVIOR     :  It enables/disables other conflicting settings.
 * PRE          :  gpointer is a valid pointer to a GM_pref_window_widgets.
 */
static void audio_codecs_option_changed_callback (GtkAdjustment *w, 
						 gpointer data)
{
  GM_pref_window_widgets *pw = (GM_pref_window_widgets *) data;

  pw->audio_codecs_changed = 1;
}


/* The functions */

void gnomemeeting_init_pref_window (int calling_state, options *opts)
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

  gw->pref_window = gnome_dialog_new (NULL, GNOME_STOCK_BUTTON_CANCEL,
				      GNOME_STOCK_BUTTON_APPLY,
				      GNOME_STOCK_BUTTON_OK,
				      NULL);
	       
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


  node_txt [0] = g_strdup (_("User Settings"));
  node2 = gtk_ctree_insert_node (GTK_CTREE (ctree), node, 
				 NULL, node_txt, 0,
				 NULL, NULL, NULL, NULL,
				 TRUE, FALSE);
  gtk_ctree_node_set_row_data (GTK_CTREE (ctree),
			       node2, (gpointer) "1");
  g_free (node_txt [0]);
  gnomemeeting_init_pref_window_general (notebook, calling_state, opts);


  node_txt [0] = g_strdup (_("General Settings"));
  node2 = gtk_ctree_insert_node (GTK_CTREE (ctree), node, 
				 NULL, node_txt, 0,
				 NULL, NULL, NULL, NULL,
				 TRUE, FALSE);
  gtk_ctree_node_set_row_data (GTK_CTREE (ctree),
			       node2, (gpointer) "2");
  g_free (node_txt [0]);
  gnomemeeting_init_pref_window_interface (notebook, calling_state, opts);


  node_txt [0] = g_strdup (_("Advanced Settings"));
  node3 = gtk_ctree_insert_node (GTK_CTREE (ctree), node, 
				 NULL, node_txt, 0,
				 NULL, NULL, NULL, NULL,
				 TRUE, FALSE);
  gtk_ctree_node_set_row_data (GTK_CTREE (ctree),
			       node3, (gpointer) "3");
  g_free (node_txt [0]);
  gnomemeeting_init_pref_window_advanced (notebook, calling_state, opts);


  node_txt [0] = g_strdup (_("ILS Settings"));
  node2 = gtk_ctree_insert_node (GTK_CTREE (ctree), node, 
				 NULL, node_txt, 0,
				 NULL, NULL, NULL, NULL,
				 TRUE, FALSE);
  gtk_ctree_node_set_row_data (GTK_CTREE (ctree),
			       node2, (gpointer) "4");
  g_free (node_txt [0]);
  gnomemeeting_init_pref_window_ldap (notebook, calling_state, opts);


  node_txt [0] = g_strdup (_("Gatekeeper Settings"));
  node2 = gtk_ctree_insert_node (GTK_CTREE (ctree), node, 
				 NULL, node_txt, 0,
				 NULL, NULL, NULL, NULL,
				 TRUE, FALSE);
  gtk_ctree_node_set_row_data (GTK_CTREE (ctree),
			       node2, (gpointer) "5");
  g_free (node_txt [0]);
  gnomemeeting_init_pref_window_gatekeeper (notebook, calling_state, opts);


  node_txt [0] = g_strdup (_("Device Settings"));
  node2 = gtk_ctree_insert_node (GTK_CTREE (ctree), node, 
				 NULL, node_txt, 0,
				 NULL, NULL, NULL, NULL,
				 TRUE, FALSE);
  gtk_ctree_node_set_row_data (GTK_CTREE (ctree),
			       node2, (gpointer) "6");
  g_free (node_txt [0]);
  gnomemeeting_init_pref_window_devices (notebook, calling_state, opts);


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
  gnomemeeting_init_pref_window_audio_codecs (notebook, calling_state, opts);


  node_txt [0] = g_strdup (_("Codecs Settings"));
  node = gtk_ctree_insert_node (GTK_CTREE (ctree), node, 
				NULL, node_txt, 0,
				NULL, NULL, NULL, NULL,
				TRUE, FALSE);			
  gtk_ctree_node_set_row_data (GTK_CTREE (ctree),
			       node, (gpointer) "8");
  g_free (node_txt [0]);
  gnomemeeting_init_pref_window_codecs_settings (notebook, calling_state, 
						 opts);


  gtk_signal_connect (GTK_OBJECT (ctree), "select_row",
		      GTK_SIGNAL_FUNC (menu_ctree_row_seletected_callback), 
		      notebook);

  /* Now, add the logo as first page */
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
		      GTK_SIGNAL_FUNC (pref_window_destroy_callback), pw);
}


/* BEHAVIOR     :  It builds the notebook page for audio codecs settings and
 *                 add it to the notebook, default values are set from the
 *                 options struct given as parameter.
 * PRE          :  parameters have to be valid
 *                 * 1 : pointer to the notebook
 *                 * 2 : calling_state such as when creating the pref window
 *                 * 3 : pointer to valid options read in the config file
 */
static void gnomemeeting_init_pref_window_audio_codecs (GtkWidget *notebook, 
							int calling_state, 
							options *opts)
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
  
  /* Get the data */
  GM_pref_window_widgets *pw = gnomemeeting_get_pref_window (gm);

  /* A vbox to pack the frames into it */
  vbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);

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
  while (cpt < 5) {
    add_codec (pw->clist_avail, opts->audio_codecs [cpt] [0], 
	       opts->audio_codecs [cpt] [1]);
    cpt++;
  }

  /* Callback function when a row is selected */
  gtk_signal_connect(GTK_OBJECT(pw->clist_avail), "select_row",
		     GTK_SIGNAL_FUNC(codecs_clist_row_selected_callback), 
		     (gpointer) pw);
    
  gtk_table_attach (GTK_TABLE (table), pw->clist_avail, 0, 4, 0, 1,
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);
    
  /* BUTTONS */						
  /* Add */
  button = gnomemeeting_button (_("Add"), 
		       gnome_stock_new_with_icon (GNOME_STOCK_BUTTON_APPLY));  

  gtk_table_attach (GTK_TABLE (table), button, 0, 1, 1, 2,
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);
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
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);
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
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);
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
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (codecs_clist_button_clicked_callback), 
		      (gpointer) pw);
  gtk_object_set_data (GTK_OBJECT (button), "operation", (gpointer) "Down");

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, button,
			_("Use this button to move up the selected codec in the preference order"), NULL);


  if (calling_state != 0)
      gtk_widget_set_sensitive (GTK_WIDGET (frame), FALSE);


  label = gtk_label_new (_("Audio Codecs")); 		
  gtk_notebook_append_page (GTK_NOTEBOOK(notebook),
			    general_frame, label);		

  if (calling_state != 0)
      gtk_widget_set_sensitive (GTK_WIDGET (label), FALSE);
}


/* BEHAVIOR     :  It builds the notebook page for interface settings
 *                 add it to the notebook, default values are set from the
 *                 options struct given as parameter.
 * PRE          :  See init_pref_audio_codecs.
 */
static void gnomemeeting_init_pref_window_interface (GtkWidget *notebook, 
						     int calling_state, 
						     options *opts)
{
  GtkWidget *frame, *label;
  GtkWidget *general_frame;

  GtkWidget *vbox;
  GtkWidget *table;
 
  GtkTooltips *tip;


  /* Get the data */
  GM_pref_window_widgets *pw = gnomemeeting_get_pref_window (gm);

  vbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);

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

  table = gtk_table_new (3, 2, TRUE);
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
  gtk_tooltips_set_tip (tip, pw->show_quickbar,
			_("If enabled, the quick access bar is displayed"), 
			NULL);

  /* Show / hide the docklet */
  pw->show_docklet = 
    gtk_check_button_new_with_label (_("Show Docklet"));
  gtk_table_attach (GTK_TABLE (table), pw->show_docklet, 0, 1, 2, 3,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);	
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pw->show_docklet), 
				opts->show_docklet);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->show_docklet,
			_("If enabled, there is support for a docklet in the Gnome or KDE panel"), 
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
  pw->incoming_call_sound = 
    gtk_check_button_new_with_label (_("Incoming Call"));
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


/* BEHAVIOR     :  It builds the notebook page for the codecs settings and
 *                 add it to the notebook, default values are set from the
 *                 options struct given as parameter.
 * PRE          :  See init_pref_audio_codecs.
 */
static void gnomemeeting_init_pref_window_codecs_settings (GtkWidget *notebook,
							   int calling_state, 
							   options *opts)
{
  GtkWidget *frame, *label;
  GtkWidget *general_frame;

  GtkWidget *menu1;  /* For the Transmitted Video Size */
  GtkWidget *menu2;  /* For the Transmitted Video Format */
  GtkWidget *re_vq;
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

  /* Get the data */
  GM_pref_window_widgets *pw = gnomemeeting_get_pref_window (gm);
		
  vbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);

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
  gtk_container_set_border_width (GTK_CONTAINER (frame), GNOME_PAD_SMALL);
  gtk_container_set_border_width (GTK_CONTAINER (audio_codecs_notebook), 
				  GNOME_PAD_SMALL);

  /* Create a page for each audio codecs having settings */
  /* General Settings */
  table = gtk_table_new (2, 4, TRUE);

  label = gtk_label_new (_("Jitter Buffer Delay:"));
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
	
  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, jitter_buffer,
			_("The jitter buffer delay to buffer audio calls (in ms)"), NULL);


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
		       1.0, 7.0, 
		       1.0, 1.0, 1.0);

  gsm_frames = gtk_spin_button_new (pw->gsm_frames_spin_adj, 1.0, 0);
  
  gtk_table_attach (GTK_TABLE (table), gsm_frames, 1, 2, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, gsm_frames,
			_("The number of frames in each transmitted GSM packet"), NULL);


  /* if it changes, we have to reinit the capabilities */
  gtk_signal_connect (GTK_OBJECT (pw->gsm_frames_spin_adj), "value-changed",
		      GTK_SIGNAL_FUNC (audio_codecs_option_changed_callback), 
		      (gpointer) pw);

  pw->gsm_sd = gtk_check_button_new_with_label (_("Silence Detection"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pw->gsm_sd),
				opts->gsm_sd);
  gtk_table_attach (GTK_TABLE (table), pw->gsm_sd, 0, 2, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);			

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
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);			

  pw->g711_frames_spin_adj = (GtkAdjustment *) 
    gtk_adjustment_new(opts->g711_frames, 
		       11.0, 240.0, 
		       1.0, 1.0, 1.0);

  g711_frames = gtk_spin_button_new (pw->g711_frames_spin_adj, 1.0, 0);
  
  gtk_table_attach (GTK_TABLE (table), g711_frames, 1, 2, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);			

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, g711_frames,
			_("The number of frames in each transmitted G.711 packet"), NULL);


  /* If it changes, we have to reinit the capabilities */
  gtk_signal_connect (GTK_OBJECT (pw->gsm_frames_spin_adj), "value-changed",
		      GTK_SIGNAL_FUNC (audio_codecs_option_changed_callback), 
		      (gpointer) pw);

  pw->g711_sd = gtk_check_button_new_with_label (_("Silence Detection"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pw->g711_sd),
				opts->g711_sd);
  gtk_table_attach (GTK_TABLE (table), pw->g711_sd, 0, 2, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);			

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->g711_sd,
			_("Enable silence detection for the G.711 based codecs"), NULL);


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

  gtk_signal_connect (GTK_OBJECT (GTK_OPTION_MENU (pw->opt1)->menu), 
		      "deactivate",
 		      GTK_SIGNAL_FUNC (video_transmission_option_changed_callback), (gpointer) pw);

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

  gtk_signal_connect (GTK_OBJECT (GTK_OPTION_MENU (pw->opt2)->menu), 
		      "deactivate",
 		      GTK_SIGNAL_FUNC (video_transmission_option_changed_callback), (gpointer) pw);
    
  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->opt2,
			_("Here you can choose the transmitted video format"), NULL);


  /* Transmitted Frames per Second */
  pw->tr_fps_label = gtk_label_new (_("Transmitted FPS:"));
  gtk_table_attach (GTK_TABLE (table), pw->tr_fps_label, 2, 3, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);			

  pw->tr_fps_spin_adj = (GtkAdjustment *) 
    gtk_adjustment_new(opts->tr_fps, 
		       1.0, 40.0, 
		       1.0, 1.0, 1.0);

  pw->tr_fps = gtk_spin_button_new (pw->tr_fps_spin_adj, 1.0, 0);
  
  gtk_table_attach (GTK_TABLE (table), pw->tr_fps, 3, 4, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);	

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->tr_fps,
			_("Here you can set a limit to the number of frames that will be transmitted each second. This limit is set on the Video Grabber."), NULL);

  /* Enable Transmitted FPS Limitation */
  pw->fps = 
    gtk_check_button_new_with_label (_("Transmitted FPS Limit"));

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pw->fps), 
				(opts->fps == 1));

  gtk_table_attach (GTK_TABLE (table), pw->fps, 2, 4, 2, 3,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);		

  gtk_signal_connect (GTK_OBJECT (pw->fps), "toggled",
		      GTK_SIGNAL_FUNC (fps_limit_option_changed_callback), 
		      (gpointer) pw);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->fps,
			_("Here you can enable or disable the limit on the number of transmitted frames per second."), NULL);


  /* Init the buttons */
  fps_limit_option_changed_callback (GTK_TOGGLE_BUTTON (pw->fps), pw);
  
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
		      GTK_SIGNAL_FUNC (video_transmission_option_changed_callback), (gpointer) pw);

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
			_("Here you can choose the maximum bandwidth that can be used by the H.261 video codec (in kbytes/s)"), NULL);

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
 		      GTK_SIGNAL_FUNC (video_bandwidth_limit_option_changed_callback), 
		      (gpointer) pw);

  /* Update the sensitivity */
  video_bandwidth_limit_option_changed_callback (NULL, pw);

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


/* BEHAVIOR     :  It builds the notebook page for general settings and
 *                 add it to the notebook, default values are set from the
 *                 options struct given as parameter.
 * PRE          :  See init_pref_audio_codecs.
 */
void gnomemeeting_init_pref_window_general (GtkWidget *notebook, 
					    int calling_state, options *opts)
{
  GtkWidget *frame, *label;
  GtkWidget *general_frame;

  GtkWidget *vbox;
  GtkWidget *table;
 
  GtkTooltips *tip;


  /* Get the data */
  GM_pref_window_widgets *pw = gnomemeeting_get_pref_window (gm);

  vbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);

  general_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (general_frame), GTK_SHADOW_IN);

  gtk_container_add (GTK_CONTAINER (general_frame), vbox);

  /* The title of the notebook page */
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_OUT);
  gtk_box_pack_start (GTK_BOX (vbox), frame, 
		      FALSE, TRUE, 0);

  label = gtk_label_new (_("User Settings"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label), 2, 1);
  gtk_container_add (GTK_CONTAINER (frame), label);


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
		      GTK_SIGNAL_FUNC (ldap_option_changed_callback), (gpointer) pw);


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
		      GTK_SIGNAL_FUNC (ldap_option_changed_callback), (gpointer) pw);


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
		      GTK_SIGNAL_FUNC (ldap_option_changed_callback), (gpointer) pw);


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
		      GTK_SIGNAL_FUNC (ldap_option_changed_callback), (gpointer) pw);

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
		      GTK_SIGNAL_FUNC (ldap_option_changed_callback), (gpointer) pw);

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


/* BEHAVIOR     :  It builds the notebook page for advanced settings and
 *                 add it to the notebook, default values are set from the
 *                 options struct given as parameter.
 * PRE          :  See init_pref_audio_codecs.
 */
static void gnomemeeting_init_pref_window_advanced (GtkWidget *notebook, 
						    int calling_state, 
						    options *opts)
{
  GtkWidget *frame;
  GtkWidget *general_frame;
  GtkWidget *vbox;
  GtkWidget *table;
  GtkWidget *label;

  GtkTooltips *tip;


  /* Get the data */
  GM_pref_window_widgets *pw = gnomemeeting_get_pref_window (gm);

  vbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);

  general_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (general_frame), GTK_SHADOW_IN);

  gtk_container_add (GTK_CONTAINER (general_frame), vbox);

  /* The title of the notebook page */
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_OUT);
  gtk_box_pack_start (GTK_BOX (vbox), frame, 
		      FALSE, TRUE, 0);

  label = gtk_label_new (_("Advanced Settings"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label), 2, 1);
  gtk_container_add (GTK_CONTAINER (frame), label);

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


  /* The End */
  label = gtk_label_new (_("Advanced Settings"));

  gtk_notebook_append_page (GTK_NOTEBOOK(notebook), general_frame, label);

}


/* BEHAVIOR     :  It builds the notebook page for ILS settings and
 *                 add it to the notebook, default values are set from the
 *                 options struct given as parameter.
 * PRE          :  See init_pref_audio_codecs.
 */
static void gnomemeeting_init_pref_window_ldap (GtkWidget *notebook,
						int calling_state, 
						options *opts)
{
  GtkWidget *general_frame;
  GtkWidget *frame;
  GtkWidget *vbox;
  GtkWidget *table;
  GtkWidget *label;

  GtkTooltips *tip;


  /* Get the data */
  GM_pref_window_widgets *pw = gnomemeeting_get_pref_window (gm);

  vbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);

  general_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (general_frame), GTK_SHADOW_IN);

  gtk_container_add (GTK_CONTAINER (general_frame), vbox);

  /* The title of the notebook page */
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_OUT);
  gtk_box_pack_start (GTK_BOX (vbox), frame, 
		      FALSE, TRUE, 0);

  label = gtk_label_new (_("ILS Settings"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label), 2, 1);
  gtk_container_add (GTK_CONTAINER (frame), label);

  /* ILS settings */
  frame = gtk_frame_new (_("ILS Directory to register to"));
  gtk_box_pack_start (GTK_BOX (vbox), frame, 
		      FALSE, FALSE, 0);


  /* Put a table in the first frame */
  table = gtk_table_new (2, 4, FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (frame), GNOME_PAD_SMALL);


  /* ILS directory */
  label = gtk_label_new (_("ILS Directory:"));
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);
  
  pw->ldap_server = gtk_entry_new();
  gtk_table_attach (GTK_TABLE (table), pw->ldap_server, 1, 4, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);
  gtk_entry_set_text (GTK_ENTRY (pw->ldap_server), opts->ldap_server);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->ldap_server,
			_("The ILS directory to register to"), NULL);

  gtk_signal_connect (GTK_OBJECT (pw->ldap_server), "changed",
		      GTK_SIGNAL_FUNC (ldap_option_changed_callback), 
		      (gpointer) pw);


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
		      GTK_SIGNAL_FUNC (ldap_option_changed_callback), 
		      (gpointer) pw);


  /* Use ILS */ 
  pw->ldap = gtk_check_button_new_with_label (_("Register"));
  gtk_table_attach (GTK_TABLE (table), pw->ldap, 3, 4, 1, 2,
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pw->ldap), (opts->ldap == 1));

  gtk_signal_connect (GTK_OBJECT (pw->ldap), "toggled",
		      GTK_SIGNAL_FUNC (ldap_option_changed_callback), 
		      (gpointer) pw);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->ldap,
			_("If enabled, permit to register to the selected ILS directory"), NULL);


  /* The End */									
  label = gtk_label_new (_("ILS Settings"));

  gtk_notebook_append_page (GTK_NOTEBOOK(notebook), general_frame, label);
}


/* BEHAVIOR     :  It builds the notebook page for GateKeeper settings and
 *                 add it to the notebook, default values are set from the
 *                 options struct given as parameter.
 * PRE          :  See init_pref_audio_codecs.
 */
static void gnomemeeting_init_pref_window_gatekeeper (GtkWidget *notebook, 
						      int calling_state, 
						      options *opts)
{
  GtkWidget *general_frame;
  GtkWidget *frame;
  GtkWidget *vbox;
  GtkWidget *table;
  GtkWidget *label;
  GtkWidget *menu;
  GtkWidget *item;

  GtkWidget *bps;

  GtkTooltips *tip;


  /* Get the data */
  GM_pref_window_widgets *pw = gnomemeeting_get_pref_window (gm);

  vbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);

  general_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (general_frame), GTK_SHADOW_IN);

  gtk_container_add (GTK_CONTAINER (general_frame), vbox);

  /* The title of the notebook page */
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_OUT);
  gtk_box_pack_start (GTK_BOX (vbox), frame, 
		      FALSE, TRUE, 0);

  label = gtk_label_new (_("Gatekeeper Settings"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label), 2, 1);
  gtk_container_add (GTK_CONTAINER (frame), label);

  /* Gatekeeper settings */
  frame = gtk_frame_new (_("Gatekeeper Registering Method"));
  gtk_box_pack_start (GTK_BOX (vbox), frame, 
		      FALSE, FALSE, 0);


  /* Put a table in the first frame */
  table = gtk_table_new (3, 2, FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (frame), GNOME_PAD_SMALL);

  /* Gatekeeper ID */
  pw->gk_id_label = gtk_label_new (_("Gatekeeper ID:"));
  gtk_table_attach (GTK_TABLE (table), pw->gk_id_label, 0, 1, 1, 2,
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
		      GTK_SIGNAL_FUNC (gatekeeper_option_changed_callback), (gpointer) pw);


  /* Gatekeeper Host */
  pw->gk_host_label = gtk_label_new (_("Gatekeeper host:"));
  gtk_table_attach (GTK_TABLE (table), pw->gk_host_label, 0, 1, 0, 1,
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
		      GTK_SIGNAL_FUNC (gatekeeper_option_changed_callback), (gpointer) pw);


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

  gtk_signal_connect (GTK_OBJECT (GTK_OPTION_MENU (pw->gk)->menu), 
		      "deactivate",
		      GTK_SIGNAL_FUNC (gatekeeper_option_type_changed_callback), 
		      (gpointer) pw);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, pw->gk,
			_("Registering method to use"), NULL);


  /* Gatekeeper settings */
  pw->bps_frame = gtk_frame_new (_("Gatekeeper Bandwidth"));
  gtk_box_pack_start (GTK_BOX (vbox), pw->bps_frame, 
		      FALSE, FALSE, 0);


  /* Put a table in the first frame */
  table = gtk_table_new (1, 4, TRUE);
  gtk_container_add (GTK_CONTAINER (pw->bps_frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (pw->bps_frame), 
				  GNOME_PAD_SMALL);


  /* Max Used Bandwidth spin button */					
  label = gtk_label_new (_("Maximum Bandwidth:"));
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);	
  
  pw->bps_spin_adj = (GtkAdjustment *) gtk_adjustment_new (opts->bps, 
							   1000.0, 40000.0, 
							   1.0, 100.0, 1.0);
  bps = gtk_spin_button_new (pw->bps_spin_adj, 100.0, 0);
  
  gtk_table_attach (GTK_TABLE (table), bps, 1, 2, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);	

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, bps,
			_("The maximum bandwidth that should be used for the communication. This bandwidth limitation will be transmitted to the gatekeeper."), NULL);


  gatekeeper_option_type_changed_callback (NULL, pw);

  /* The End */					      			
  label = gtk_label_new (_("Gatekeeper Settings"));

  gtk_notebook_append_page (GTK_NOTEBOOK(notebook), general_frame, label);
}


/* BEHAVIOR     :  It builds the notebook page for the device settings and
 *                 add it to the notebook, default values are set from the
 *                 options struct given as parameter.
 * PRE          :  See init_pref_audio_codecs.
 */
static void gnomemeeting_init_pref_window_devices (GtkWidget *notebook, 
						   int calling_state, 
						   options *opts)
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


  /* Get the data */
  GM_pref_window_widgets *pw = gnomemeeting_get_pref_window (gm);

  vbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);

  general_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (general_frame), GTK_SHADOW_IN);

  gtk_container_add (GTK_CONTAINER (general_frame), vbox);

  /* The title of the notebook page */
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_OUT);
  gtk_box_pack_start (GTK_BOX (vbox), frame, 
		      FALSE, TRUE, 0);

  label = gtk_label_new (_("Device Settings"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label), 2, 1);
  gtk_container_add (GTK_CONTAINER (frame), label);

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

  for (int i = pw->gw->audio_player_devices.GetSize () - 1 ; i >= 0; i--) {
    if (pw->gw->audio_player_devices [i] != PString (opts->audio_player)) {
      audio_player_devices_list = g_list_prepend 
	(audio_player_devices_list, 
	 g_strdup (pw->gw->audio_player_devices [i]));
    }  
  }

  audio_player_devices_list = 
    g_list_prepend (audio_player_devices_list, opts->audio_player);
  gtk_combo_set_popdown_strings (GTK_COMBO (pw->audio_player), 
				 audio_player_devices_list);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, GTK_COMBO (pw->audio_player)->entry, 
			_("Enter the audio player device to use"), NULL);

  gtk_signal_connect (GTK_OBJECT(GTK_COMBO (pw->audio_player)->entry), "changed",
		      GTK_SIGNAL_FUNC (audio_option_changed_callback), (gpointer) pw);


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

  for (int i = pw->gw->audio_recorder_devices.GetSize () - 1; i >= 0; i--) {
    if (pw->gw->audio_recorder_devices [i] != PString (opts->audio_recorder)) {
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
		      GTK_SIGNAL_FUNC (audio_option_changed_callback), (gpointer) pw);
    

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
		      GTK_SIGNAL_FUNC (audio_option_changed_callback), (gpointer) pw);


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
		      GTK_SIGNAL_FUNC (audio_option_changed_callback), (gpointer) pw);


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

   for (int i = pw->gw->video_devices.GetSize () - 1; i >= 0; i--) {
     if (pw->gw->video_devices [i] != PString (opts->video_device))
       video_devices_list = 
	 g_list_prepend (video_devices_list, 
			 g_strdup (pw->gw->video_devices [i]));
   }
   
   video_devices_list = g_list_prepend (video_devices_list, 
					opts->video_device);
   gtk_combo_set_popdown_strings (GTK_COMBO (pw->video_device), 
				  video_devices_list);
   
   tip = gtk_tooltips_new ();

   gtk_tooltips_set_tip (tip, GTK_COMBO (pw->video_device)->entry, 
 			_("Enter the video device to use"), NULL);
 

  gtk_signal_connect (GTK_OBJECT (GTK_COMBO (pw->video_device)->entry), 
		      "changed",
 		      GTK_SIGNAL_FUNC (video_transmission_option_changed_callback), (gpointer) pw);

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
			_("The video channel number to use (camera, tv, ...)"), 
			NULL);

  gtk_signal_connect (GTK_OBJECT (pw->video_channel_spin_adj), "value-changed",
		      GTK_SIGNAL_FUNC (video_transmission_option_changed_callback), (gpointer) pw);


  /* The End */									
  label = gtk_label_new (_("Device Settings"));

  gtk_notebook_append_page (GTK_NOTEBOOK(notebook), general_frame, label);
}


/* Miscellaneous functions */

/* DESCRIPTION  :  / 
 * BEHAVIOR     :  Add the codec (second parameter) to the codecs clist 
 *                 and the right pixmap (Enabled/Disabled) following the 3rd
 *                 parameter. Also sets row data (1 for Enabled, O for not).
 * PRE          :  First parameter should be a valid clist
 */
static void add_codec (GtkWidget *list, gchar *CodecName, gchar * Enabled)
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


/* BEHAVIOR     :  Apply the options given as parameter to the endpoint
 *                 and soundcard and video4linux device and LDAP.
 * PRE          :  A valid pointer to valid options and a valid pointer
 *                 to GM_pref_window_widgets.
 */
static void apply_options (options *opts)
{
  /* Get the data */
  GM_pref_window_widgets *pw = gnomemeeting_get_pref_window (gm);

  /* opts has been updated when this function is called */

  GMH323EndPoint *endpoint;
  int vol_play = 0, vol_rec = 0;

  endpoint = MyApp->Endpoint ();

  /* Reinitialise the endpoint settings
     so that the opts structure of the EndPoint class
     is up to date. */
  endpoint->Reset ();

  /* ILS is enabled and an option has changed : register */
  if ((opts->ldap) && (pw->ldap_changed)) {

    /* We register to the new ILS directory */
    GMILSClient *ils_client = (GMILSClient *) endpoint->GetILSClient ();
    
    ils_client->Register ();
    
    pw->ldap_changed = 0;
  }

  /* ILS is disabled, and an option has changed : unregister */
  if ((!opts->ldap) && (pw->ldap_changed)) {
    /* We unregister to the new ILS directory */
    GMILSClient *ils_client = (GMILSClient *) endpoint->GetILSClient ();
    
    ils_client->Unregister ();
    
    pw->ldap_changed = 0;
  }

  /* Change the audio mixer source */
  if (pw->audio_mixer_changed) {
    gchar *text;
    
    /* We set the new mixer in the object as data, because me do not want
       to keep it in memory */
    gtk_object_remove_data (GTK_OBJECT (pw->gw->adj_play), 
			    "audio_player_mixer");
    gtk_object_set_data (GTK_OBJECT (pw->gw->adj_play), "audio_player_mixer", 
			 g_strdup (opts->audio_player_mixer));

    gtk_object_remove_data (GTK_OBJECT (pw->gw->adj_rec), 
			    "audio_recorder_mixer");
    gtk_object_set_data (GTK_OBJECT (pw->gw->adj_rec), "audio_recorder_mixer", 
			 g_strdup (opts->audio_recorder_mixer));

    /* We are sure that those mixers are ok, it has been checked */
    gnomemeeting_volume_get (opts->audio_player_mixer, 0, &vol_play);
    gnomemeeting_volume_get (opts->audio_recorder_mixer, 1, &vol_rec);

    gtk_adjustment_set_value (GTK_ADJUSTMENT (pw->gw->adj_play),
			      vol_play / 257);
    gtk_adjustment_set_value (GTK_ADJUSTMENT (pw->gw->adj_rec),
			      vol_rec / 257);
       
    /* Set recording source and set micro to record */
    MyApp->Endpoint()->SetSoundChannelPlayDevice(opts->audio_player);
    MyApp->Endpoint()->SetSoundChannelRecordDevice(opts->audio_recorder);

    /* Translators: This is shown in the history. */
    text = g_strdup_printf (_("Set Audio player device to %s"), 
			    opts->audio_player);
    gnomemeeting_log_insert (text);
    g_free (text);

    /* Translators: This is shown in the history. */
    text = g_strdup_printf (_("Set Audio recorder device to %s"), 
			    opts->audio_recorder);
    gnomemeeting_log_insert (text);
    g_free (text);
    
    gnomemeeting_set_recording_source (opts->audio_recorder_mixer, 0); 
    
    pw->audio_mixer_changed = 0;
  }


  /* Change video settings if vid_tr is enables */
  if (pw->vid_tr_changed)
    {
      GMVideoGrabber *vg = (GMVideoGrabber *) 
	MyApp->Endpoint ()->GetVideoGrabber ();

      vg->Reset ();

      /* Video Size has changed */
      pw->capabilities_changed = 1;
      pw->vid_tr_changed = 0;
    }


  /* Unregister from the Gatekeeper, if any, and if
     it is needed */
  if ((MyApp->Endpoint()->GetGatekeeper () != NULL) 
      && (pw->gk_changed) && (!opts->gk))
    MyApp->Endpoint()->RemoveGatekeeper ();

  /* Register to the gatekeeper */
  if ((opts->gk) && (pw->gk_changed)) {
    MyApp->Endpoint()->GatekeeperRegister ();
    pw->gk_changed = 0;
  }

  /* Reinitialise the capabilities */
  if (pw->capabilities_changed) {
    MyApp->Endpoint ()->RemoveAllCapabilities ();
    MyApp->Endpoint ()->AddAudioCapabilities ();
    MyApp->Endpoint ()->AddVideoCapabilities (opts->video_size);
    
    pw->capabilities_changed = 0;
  }

  /* Reinitialise the audio codecs capabilities */
  if (pw->audio_codecs_changed) {
    MyApp->Endpoint ()->RemoveAllCapabilities ();
    MyApp->Endpoint ()->AddAudioCapabilities ();

    pw->audio_codecs_changed = 0;
  }

  if (opts->video_preview) {
    /* it will only be set to active, if it is not already and start the
       preview */
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pw->gw->preview_button), 
				  TRUE);
  }
  else {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pw->gw->preview_button), 
				  FALSE);
  }


  /* Show / Hide notebook and / or statusbar and / or docklet */
  GtkWidget *object = (GtkWidget *) 
    gtk_object_get_data (GTK_OBJECT (gm),
			 "view_menu_uiinfo");

  GnomeUIInfo *view_menu_uiinfo = (GnomeUIInfo *) object;

  if (!(opts->show_notebook)) {

    gtk_widget_hide_all (pw->gw->main_notebook);
    GTK_CHECK_MENU_ITEM (view_menu_uiinfo [2].widget)->active = FALSE;
  }
  else {

    gtk_widget_show_all (pw->gw->main_notebook);  
    GTK_CHECK_MENU_ITEM (view_menu_uiinfo [2].widget)->active = TRUE;
  }


 if (!(opts->show_statusbar)) {

    gtk_widget_hide_all (pw->gw->statusbar);
    GTK_CHECK_MENU_ITEM (view_menu_uiinfo [3].widget)->active = FALSE;
  }
  else {

    gtk_widget_show_all (pw->gw->statusbar);  
    GTK_CHECK_MENU_ITEM (view_menu_uiinfo [3].widget)->active = TRUE;
  }


 if (!(opts->show_quickbar)) {

    gtk_widget_hide_all (pw->gw->quickbar_frame);
    GTK_CHECK_MENU_ITEM (view_menu_uiinfo [4].widget)->active = FALSE;
  }
  else {

    gtk_widget_show_all (pw->gw->quickbar_frame);  
    GTK_CHECK_MENU_ITEM (view_menu_uiinfo [4].widget)->active = TRUE;
  }
 
 if (!opts->show_docklet) {

   GTK_CHECK_MENU_ITEM (view_menu_uiinfo [5].widget)->active = FALSE;
   gnomemeeting_docklet_hide (pw->gw->docklet);
 }
 else {
   GTK_CHECK_MENU_ITEM (view_menu_uiinfo [5].widget)->active = TRUE;
   gnomemeeting_docklet_show (pw->gw->docklet);
 }
}
