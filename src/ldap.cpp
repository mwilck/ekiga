/***************************************************************************
                          ldap.cxx  -  description
                             -------------------
    begin                : Wed Feb 28 2001
    copyright            : (C) 2000-2001 by Damien Sandras
    description          : This file contains all the functions needed 
                           for ILS support
    email                : dsandras@acm.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "ldap_h.h"
#include "ils.h"
#include "config.h"
#include "main.h"
#include "videograbber.h"
#include "common.h"
#include "main_interface.h"

#include "../pixmaps/quickcam.xpm"
#include "../pixmaps/sound.xpm"
#include "../pixmaps/ldap_add.xpm"
#include "../pixmaps/ldap_refresh.xpm"


/******************************************************************************/
/* Global Variables                                                           */
/******************************************************************************/

extern GtkWidget *gm;
extern GnomeMeeting *MyApp;	

/******************************************************************************/


/******************************************************************************/
/* GTK Callbacks                                                              */
/******************************************************************************/

void ldap_window_clicked (GnomeDialog *widget, int button, gpointer data)
{
  GM_ldap_window_widgets *lw = (GM_ldap_window_widgets *) data;

  if (GTK_WIDGET_VISIBLE (lw->gw->ldap_window))
    gtk_widget_hide_all (lw->gw->ldap_window);
}


void ldap_window_destroyed (GtkWidget *widget, gpointer data)
{
  GM_ldap_window_widgets *lw = (GM_ldap_window_widgets *) data;

  if (GTK_WIDGET_VISIBLE (lw->gw->ldap_window))
    gtk_widget_hide_all (lw->gw->ldap_window);
}


void search_entry_modified (GtkWidget *widget, gpointer data)
{
  GM_ldap_window_widgets *lw = (GM_ldap_window_widgets *) data;
  
  lw->last_selected_row = -1;
}


void ldap_clist_column_clicked (GtkCList *clist, gint column, gpointer data)
{
  GM_ldap_window_widgets *lw = (GM_ldap_window_widgets *) data;

  if (column > 1)
    {
      if (lw->sorted_column != column)
	{
	  gtk_clist_set_sort_column (GTK_CLIST (clist), column);
	  gtk_clist_set_sort_type (GTK_CLIST (clist), GTK_SORT_ASCENDING);
	  gtk_clist_sort (GTK_CLIST (clist));
	  lw->sorted_column = column;
	  lw->sorted_order = 0;
	}
      else
	{
	  if (lw->sorted_order == 0)
	    {
	      lw->sorted_order = 1;
	      gtk_clist_set_sort_type (GTK_CLIST (clist), GTK_SORT_DESCENDING);
	      gtk_clist_sort (GTK_CLIST (clist));
	    }
	  else
	    {
	      lw->sorted_order = 0;
	      gtk_clist_set_sort_type (GTK_CLIST (clist), GTK_SORT_ASCENDING);
	      gtk_clist_sort (GTK_CLIST (clist));
	    }	      
	}
    }

  lw->last_selected_row = 0;
}


void ldap_clist_row_selected (GtkWidget *widget, gint row, 
			      gint column, GdkEventButton *event, gpointer data)
{
  GM_ldap_window_widgets *lw = (GM_ldap_window_widgets *) data;

  lw->last_selected_row = (int) row;		
}


void refresh_button_clicked (GtkButton *button, gpointer data)
{
  GM_ldap_window_widgets *lw = (GM_ldap_window_widgets *) data;
  GMH323EndPoint *endpoint = MyApp->Endpoint ();
  GMILSClient *ils_client = (GMILSClient *) endpoint->get_ils_client ();

  lw->thread_count++;

  if (lw->thread_count == 1)
    {
      gtk_clist_freeze (GTK_CLIST (lw->ldap_users_clist));
      gtk_clist_clear (GTK_CLIST (lw->ldap_users_clist));

      ils_client->ils_browse (lw);

      gtk_clist_thaw (GTK_CLIST (lw->ldap_users_clist));
    }
    
}


void apply_filter_button_clicked (GtkButton *button, gpointer data)
{
  GM_ldap_window_widgets *lw = (GM_ldap_window_widgets *) data;
  // should not be freed : entry is a pointer to the text part of an entry
  // and text is a pointer to the text part of a clist
  gchar *entry = NULL, *text = NULL;
  GtkWidget *active_item;

  int cpt = 0, col = 0;

  active_item = gtk_menu_get_active (GTK_MENU (GTK_OPTION_MENU 
					       (lw->option_menu)->menu));

  col = g_list_index (GTK_MENU_SHELL (GTK_OPTION_MENU (lw->option_menu)
				      ->menu)->children, 
		      active_item)
        + 2;

  if (col != lw->last_selected_col)
    lw->last_selected_row = -1;


  entry = gtk_entry_get_text (GTK_ENTRY (lw->search_entry));


  for (cpt = lw->last_selected_row + 1 ; 
       cpt < GTK_CLIST (lw->ldap_users_clist)->rows ; 
       cpt++)
    {
      gtk_clist_get_text (GTK_CLIST (lw->ldap_users_clist), cpt, 
			  col, &text);

      if (!strcasecmp (entry, text))
	{
	  gtk_clist_select_row (GTK_CLIST (lw->ldap_users_clist), 
				cpt, col);

	  lw->last_selected_row = cpt;
	  lw->last_selected_col = col;

	  gtk_clist_moveto (GTK_CLIST (lw->ldap_users_clist), cpt, 
			    0, 0, 0);

	  break;
	}
    }
}


void user_add_button_clicked (GtkButton *button, gpointer data)
{
  char ip [15];
  char *ip_text = &ip [0];
  GM_ldap_window_widgets *lw = (GM_ldap_window_widgets *) data;

  if (lw->last_selected_row != -1)
    {
      gtk_clist_get_text (GTK_CLIST (lw->ldap_users_clist),
			  lw->last_selected_row, 7,
			  &ip_text);

      MyApp->AddContactIP (ip_text);
    }
}

/******************************************************************************/


/******************************************************************************/
/* The functions                                                              */
/******************************************************************************/

void GM_ldap_init (GM_window_widgets *gw)
{
  GtkWidget *table, *entry_table;
  GtkWidget *vbox;
  GtkWidget *dialog_vbox;
  GtkWidget *frame;
  GtkWidget *scroll;
  GtkWidget *refresh_button;
  GtkWidget *user_add_button;
  GtkWidget *apply_filter_button;
  GtkWidget *who_pixmap;
  GtkWidget *user_add_pixmap;
  GtkWidget *menu;
  GtkWidget *menu_item;


  GM_ldap_window_widgets *lw = NULL;

  lw = new (GM_ldap_window_widgets);

  lw->last_selected_row = -1;
  lw->last_selected_row = 2;

  lw->thread_count = 0;
  lw->gw = gw;

  gchar * clist_titles [] = 
    {
     /* Translators: This is as in "Audio". */
     N_("A"),
     /* Translators: This is as in "Video". */
     N_("V"),
     N_("First Name"), N_("Last name"), N_("E-mail"), 
     N_("Location"), N_("Comment"), N_("IP")};

  who_pixmap =  gnome_pixmap_new_from_xpm_d ((char **) ldap_refresh_xpm);
  user_add_pixmap =  gnome_pixmap_new_from_xpm_d ((char **) ldap_add_xpm);

  lw->gw->ldap_window = gnome_dialog_new (NULL, GNOME_STOCK_BUTTON_OK, NULL);
  dialog_vbox = GNOME_DIALOG (gw->ldap_window)->vbox;

  /* a vbox to put the entry frame and the user list */
  vbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
  gtk_box_pack_start (GTK_BOX (dialog_vbox), vbox, TRUE, TRUE, 0);

  // Intl
  for (int i = 0 ; i < 8 ; i++)
    clist_titles [i] = gettext (clist_titles [i]);

  /* Search filter entry */
  frame = gtk_frame_new (_("Search filter"));
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_OUT);

  entry_table = gtk_table_new (2, 10, FALSE);

  // option menu
  menu = gtk_menu_new ();
  menu_item = gtk_menu_item_new_with_label (_("first name"));
  gtk_menu_append (GTK_MENU (menu), menu_item);

  menu_item = gtk_menu_item_new_with_label (_("last name"));
  gtk_menu_append (GTK_MENU (menu), menu_item);

  menu_item = gtk_menu_item_new_with_label (_("e-mail"));
  gtk_menu_append (GTK_MENU (menu), menu_item);

  menu_item = gtk_menu_item_new_with_label (_("location"));
  gtk_menu_append (GTK_MENU (menu), menu_item);

  lw->option_menu = gtk_option_menu_new ();
  gtk_option_menu_set_menu (GTK_OPTION_MENU (lw->option_menu),
			    menu);
  gtk_option_menu_set_history (GTK_OPTION_MENU (lw->option_menu),
			       1);

  gtk_table_attach (GTK_TABLE (entry_table), lw->option_menu, 1, 4, 0, 2,
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);

  // entry
  lw->search_entry = gtk_entry_new ();
  gtk_container_add (GTK_CONTAINER (frame), entry_table);
  gtk_container_set_border_width (GTK_CONTAINER (frame), GNOME_PAD_SMALL);

  gtk_table_attach (GTK_TABLE (entry_table), lw->search_entry, 4, 8, 0, 2,
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);

  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);

  // the filter button
  apply_filter_button = gtk_button_new_with_label (_("Apply filter on"));

  gtk_table_attach (GTK_TABLE (entry_table), apply_filter_button, 0, 1, 0, 2,
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);

  // the refresh and user_add buttons
  refresh_button = gtk_button_new ();
  gtk_widget_set_usize (GTK_WIDGET (refresh_button), 30, 30);
  gtk_container_add (GTK_CONTAINER (refresh_button), who_pixmap);
  gtk_table_attach (GTK_TABLE (entry_table), refresh_button, 8, 9, 0, 2,
		    (GtkAttachOptions) NULL, 
		    (GtkAttachOptions) NULL,
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);

  user_add_button = gtk_button_new ();
  gtk_widget_set_usize (GTK_WIDGET (user_add_button), 30, 30);
  gtk_container_add (GTK_CONTAINER (user_add_button), user_add_pixmap);
  gtk_table_attach (GTK_TABLE (entry_table), user_add_button, 9, 10, 0, 2,
		    (GtkAttachOptions) NULL, 
		    (GtkAttachOptions) NULL,
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);


  /* Status Bar */
  lw->statusbar = gnome_appbar_new (FALSE, TRUE, GNOME_PREFERENCES_NEVER);
  gtk_container_add (GTK_CONTAINER (dialog_vbox), 
		     lw->statusbar);
  gtk_container_set_border_width (GTK_CONTAINER (lw->statusbar), 
				  GNOME_PAD_SMALL);


  /* Ldap users list */
  frame = gtk_frame_new (_("ILS Users List"));
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_OUT);

  gtk_box_pack_start (GTK_BOX (vbox), frame, 
		      FALSE, FALSE, 0);
  
  table = gtk_table_new (2, 10, FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (frame), GNOME_PAD_SMALL);

  scroll = gtk_scrolled_window_new (NULL, NULL);

  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll), 
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_AUTOMATIC);

  lw->ldap_users_clist = gtk_clist_new_with_titles (8, clist_titles);

  gtk_clist_set_column_auto_resize (GTK_CLIST (lw->ldap_users_clist), 0, TRUE);
  gtk_clist_set_column_auto_resize (GTK_CLIST (lw->ldap_users_clist), 1, TRUE);
  gtk_clist_set_column_auto_resize (GTK_CLIST (lw->ldap_users_clist), 2, TRUE);
  gtk_clist_set_column_auto_resize (GTK_CLIST (lw->ldap_users_clist), 3, TRUE);
  gtk_clist_set_column_auto_resize (GTK_CLIST (lw->ldap_users_clist), 4, TRUE);
  gtk_clist_set_column_auto_resize (GTK_CLIST (lw->ldap_users_clist), 5, TRUE);
  gtk_clist_set_column_auto_resize (GTK_CLIST (lw->ldap_users_clist), 6, TRUE);
  gtk_clist_set_column_auto_resize (GTK_CLIST (lw->ldap_users_clist), 7, TRUE);
  gtk_clist_set_column_auto_resize (GTK_CLIST (lw->ldap_users_clist), 8, TRUE);

  gtk_clist_set_shadow_type (GTK_CLIST (lw->ldap_users_clist), GTK_SHADOW_IN);

  gtk_widget_set_usize (GTK_WIDGET (lw->ldap_users_clist), 520, 180);
  
  gtk_container_add (GTK_CONTAINER (scroll), lw->ldap_users_clist);
  gtk_table_attach (GTK_TABLE (table), scroll, 0, 10, 0, 2,
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		    GNOME_PAD_BIG, GNOME_PAD_BIG);

  gtk_signal_connect (GTK_OBJECT (refresh_button), "pressed",
		      GTK_SIGNAL_FUNC (refresh_button_clicked), 
		      (gpointer) lw);

  gtk_signal_connect (GTK_OBJECT (user_add_button), "pressed",
		      GTK_SIGNAL_FUNC (user_add_button_clicked), 
		      (gpointer) lw);

  gtk_signal_connect (GTK_OBJECT (apply_filter_button), "pressed",
		      GTK_SIGNAL_FUNC (apply_filter_button_clicked), 
		      (gpointer) lw);

  gtk_signal_connect (GTK_OBJECT (lw->ldap_users_clist), "select_row",
		      GTK_SIGNAL_FUNC (ldap_clist_row_selected), (gpointer) lw);

  gtk_signal_connect (GTK_OBJECT (lw->ldap_users_clist), "click-column",
		      GTK_SIGNAL_FUNC (ldap_clist_column_clicked), 
		      (gpointer) lw);

  gtk_signal_connect (GTK_OBJECT(lw->search_entry), "changed",
		      GTK_SIGNAL_FUNC(search_entry_modified), (gpointer) lw);

  gtk_signal_connect (GTK_OBJECT(gw->ldap_window), "clicked",
		     GTK_SIGNAL_FUNC(ldap_window_clicked), (gpointer) lw);

  gtk_signal_connect (GTK_OBJECT (gw->ldap_window), "close",
		      GTK_SIGNAL_FUNC (ldap_window_destroyed), (gpointer) lw);
}


void * GM_ldap_populate_ldap_users_clist (void *lwi)
{
  // FREEING MEMORY  :  checked   STATUS  :  OK
  GM_ldap_window_widgets *lw = (GM_ldap_window_widgets *) lwi;
  LDAP *ldap_connection = NULL;
  char *datas [] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
  char *attrs [] = { "surname", "givenname", "comment", "location", 
		     "rfc822mailbox", "sipaddress", "ilsa32833566", 
		     "ilsa32964638", NULL };
  
  LDAPMessage *res = NULL, *e = NULL;

  options opts;

  int rc = 0;
  unsigned long int nmip = 0;
  int part1;
  int part2;
  int part3;
  int part4;
  char ip [15];

  GdkPixmap *quickcam;
  GdkBitmap *quickcam_mask;
  GdkPixmap *sound;
  GdkBitmap *sound_mask;

  read_config (&opts);

  if (!strcmp (opts.ldap_server, ""))
    {
      gdk_threads_enter ();
      gnome_appbar_push (GNOME_APPBAR (lw->statusbar), 
			 _("Please provide an ILS directory in Settings"));
      gdk_threads_leave ();
      return (0);
    }


  gdk_threads_enter ();
  
  quickcam = gdk_pixmap_create_from_xpm_d (gm->window, &quickcam_mask,
					   NULL,
					   (gchar **) quickcam_xpm);

  sound = gdk_pixmap_create_from_xpm_d (gm->window, &sound_mask,
					NULL,
					(gchar **) sound_xpm); 

  gnome_appbar_push (GNOME_APPBAR (lw->statusbar), 
		     _("Connecting to ILS directory... Please Wait."));

  gdk_threads_leave ();

  ldap_connection = ldap_open (opts.ldap_server, atoi (opts.ldap_port));

  if (ldap_connection == NULL)
    {
      gdk_threads_enter ();
      gnome_appbar_push (GNOME_APPBAR (lw->statusbar), 
			 _("Error while connecting to ILS directory"));
      gdk_threads_leave ();

      return (0);
    }

  if (ldap_bind_s (ldap_connection, NULL, NULL, LDAP_AUTH_SIMPLE ) 
      != LDAP_SUCCESS ) 
    {
      gdk_threads_enter ();
      gnome_appbar_push (GNOME_APPBAR (lw->statusbar), 
			 _("Error while connecting to ILS directory"));
      gdk_threads_leave ();

      return (0);
    }


  gdk_threads_enter ();        
  gnome_appbar_push (GNOME_APPBAR (lw->statusbar), 
		     _("Searching for users... Please Wait."));
  gdk_threads_leave ();

  rc = ldap_search_s (ldap_connection, "objectClass=RTPerson", 
		      LDAP_SCOPE_BASE,
		      "(&(cn=%))",
		      attrs, 0, &res); 

  for (int i = 0 ; i < 8 ; i++)
    datas [i] = NULL;


  gdk_threads_enter ();
  gnome_appbar_push (GNOME_APPBAR (lw->statusbar), 
		     _("Search completed!"));


  for(e = ldap_first_entry(ldap_connection, res); 
      e != NULL; e = ldap_next_entry(ldap_connection, e)) 
    {
      if (ldap_get_values (ldap_connection, e, "surname") != NULL)
	datas [3] = g_strdup (ldap_get_values (ldap_connection, e, "surname") [0]);
      
      if (ldap_get_values(ldap_connection, e, "givenname") != NULL)
	datas [2] = g_strdup (ldap_get_values 
			      (ldap_connection, e, "givenname") [0]);
      
      if (ldap_get_values(ldap_connection, e, "location") != NULL)
	datas [5] = g_strdup (ldap_get_values 
			      (ldap_connection, e, "location") [0]);
      
      if (ldap_get_values(ldap_connection, e, "comment") != NULL)
	datas [6] = g_strdup (ldap_get_values (ldap_connection, e, "comment") [0]);

      if (ldap_get_values(ldap_connection, e, "rfc822mailbox") != NULL)
	datas [4] = g_strdup (ldap_get_values 
			      (ldap_connection, e, "rfc822mailbox") [0]);

      if (ldap_get_values(ldap_connection, e, "sipaddress") != NULL)
	nmip = strtoul (ldap_get_values(ldap_connection, e, "sipaddress") [0], 
			NULL, 10);
      
      part1 = (int) (nmip/(256*256*256));
      part2 = (int) ((nmip - part1 * (256 * 256 * 256)) / (256 * 256));
      part3 = (int) ((nmip - part1 * (256 * 256 * 256) - part2 * (256 * 256)) 
		     / 256);
      part4 = (int) ((nmip - part1 * (256 * 256 * 256) - part2 * (256 * 256) 
		      - part3 * 256));
      

      sprintf (ip, "%d.%d.%d.%d", part4, part3, part2, part1);
      // ip will be freed (char ip [15]), so we make a copy in datas [7]
     
      datas [7] = g_strdup ((char *) ip);
      gtk_clist_append (GTK_CLIST (lw->ldap_users_clist), (gchar **) datas);
    
      /* Video Capable ? */
      if (ldap_get_values(ldap_connection, e, "ilsa32964638") != NULL)
	nmip = atoi (ldap_get_values(ldap_connection, e, "ilsa32964638") [0]);
      
      if (nmip == 1)
	{
	  gtk_clist_set_pixmap (GTK_CLIST (lw->ldap_users_clist), 
				GTK_CLIST (lw->ldap_users_clist)->rows - 1, 1, 
				quickcam, quickcam_mask);
	}

      /* Audio Capable ? */
      if (ldap_get_values(ldap_connection, e, "ilsa32833566") != NULL)
	nmip = atoi (ldap_get_values(ldap_connection, e, "ilsa32833566") [0]);
      
      if (nmip == 1)
	{
	  gtk_clist_set_pixmap (GTK_CLIST (lw->ldap_users_clist), 
				GTK_CLIST (lw->ldap_users_clist)->rows - 1, 0, 
				sound, sound_mask);
	}

      for (int j = 0 ; j < 7 ; j++)
	g_free (datas [j]);

    } // end of for
  
  gdk_threads_leave ();
  
  ldap_msgfree (res);
  ldap_unbind (ldap_connection);

  lw->thread_count = 0;

  g_options_free (&opts);

  return (0);
}


void *GM_ldap_register (char *ip, GM_window_widgets *gw)
{
  return (0);
}


void *GM_ldap_refresh (void)
{
  options opts;
  char filter [500];
  LDAPMessage *res = NULL;
  LDAP *ldap_connection = NULL;
  int rc = 0;
  char *attrs [] = { "*", NULL}; 

  read_config (&opts);

  ldap_connection = ldap_open(opts.ldap_server, atoi (opts.ldap_port));

  if (ldap_connection == NULL)
    {
      return (0);
    }

  if (ldap_bind_s (ldap_connection, NULL, NULL, LDAP_AUTH_SIMPLE ) 
      != LDAP_SUCCESS ) 
    {
      return (0);
    }

  strcpy (filter, "(&(objectClass=RTPerson)(cn=");
  strcat (filter, opts.mail);
  strcat (filter, ")(sttl=10))");

  rc = ldap_search_s (ldap_connection, "objectClass=RTPerson", 
		      LDAP_SCOPE_BASE, filter,
		      attrs, 0, &res); 


  g_options_free (&opts);

  ldap_msgfree (res);
  ldap_unbind (ldap_connection);

  return (0);
}

