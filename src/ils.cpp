/***************************************************************************
                           ils.cpp  -  description
                             -------------------
    begin                : Sun Sep 23 2001
    copyright            : (C) 2000-2001 by Damien Sandras
    description          : This file contains all the classes needed 
                           for ILS support (rewrite of the functions
                           existing since 010228)
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


#include <sys/time.h>

#include "../config.h"

#include "ils.h"
#include "main.h"
#include "webcam.h"
#include "common.h"
#include "main_interface.h"

#include "../pixmaps/quickcam.xpm"
#include "../pixmaps/sound.xpm"

extern GnomeMeeting MyApp;
extern GtkWidget *gm;

GMILSClient::GMILSClient (GM_window_widgets *g, options *o)
  :PThread (1000, NoAutoDeleteThread)
{
  gw = g;
  opts = o;

  running = 1;
  in_the_loop = 0;
  has_to_unregister = 0;
  has_to_browse = 0;
  cout<< opts->ldap << endl << flush;

  // if we need to register to the ILS directory
  if (opts->ldap)
    has_to_register = 1;
  else
    has_to_register = 0;

  Resume ();
}


GMILSClient::~GMILSClient ()
{
  running = 0;

  quit_mutex.Wait ();
}


void GMILSClient::Main ()
{
  quit_mutex.Wait ();

  while (running == 1)
    {
     if (has_to_register == 1)
       ils_register (TRUE);

     if (has_to_unregister == 1)
       ils_register (FALSE);

     if (has_to_browse == 1)
       ils_browse ();

      Current ()->Sleep (500);
    }

  quit_mutex.Signal ();
}


void GMILSClient::stop ()
{
  running = 0;
}


void GMILSClient::ils_register ()
{
  has_to_register = 1;
}

void GMILSClient::ils_unregister ()
{
  has_to_unregister = 1;
}

BOOL GMILSClient::ils_register (BOOL reg)
{
  LDAPMessage *res=NULL;
  LDAPMod *mods [15];

  char *firstname_value [2];
  char *surname_value [2];
  char *mail_value [2];
  char *comment_value [2];
  char *location_value [2];
  char *ilsa32833566_value [2];
  char *ilsa32964638_value [2];
  char *ilsa26214430_value [2];
  char *sipaddress_value [2];
  char *sport_value [2];
  char *sappid_value [2];
  char *protid_value [2];
  char *objectclass_value [2];
  char *cn_value [2];

  char *dn = NULL;
  gchar *msg;
  unsigned long sip;
  int rc;
  int error = 0; // no error at the moment

  struct timeval t;

  GtkWidget *msg_box;

  gdk_threads_enter ();
  msg = g_strdup_printf (_("Connecting to ILS directory %s, port %s"), 
			 opts->ldap_server, opts->ldap_port);
  GM_log_insert (gw->log_text, msg);
  g_free (msg);
  gdk_threads_leave ();

  ldap_connection = ldap_open (opts->ldap_server, atoi (opts->ldap_port));

  if ((ldap_connection == NULL) || (ldap_bind_s (ldap_connection, NULL, NULL, LDAP_AUTH_SIMPLE)
				    != LDAP_SUCCESS))
    {
      gdk_threads_enter ();

      msg = g_strdup_printf (_("Error while connecting to ILS directory %s, port %s"), 
			     opts->ldap_server, opts->ldap_port);
      msg_box = gnome_message_box_new (msg, GNOME_MESSAGE_BOX_ERROR,
				       GNOME_STOCK_BUTTON_OK, NULL);
      g_free (msg);
      gtk_widget_show (msg_box);

      error = 1;

      gdk_threads_leave ();
    }

  if (!error)
    {
      // cn
      mods [0] = new (LDAPMod);
      cn_value [0] = g_strdup (opts->mail);
      cn_value [1] = NULL;
      mods [0]->mod_op = LDAP_MOD_ADD | LDAP_MOD_REPLACE;
      mods [0]->mod_type = g_strdup ("cn");
      mods [0]->mod_values = cn_value;

      // objectclass
      mods [1] = new (LDAPMod);
      objectclass_value [0] = g_strdup ("rtperson");
      objectclass_value [1] = NULL;
      mods [1]->mod_op = LDAP_MOD_ADD | LDAP_MOD_REPLACE;
      mods [1]->mod_type = g_strdup ("objectclass");
      mods [1]->mod_values = objectclass_value;

      // Sappid
      mods [2] = new (LDAPMod);
      sappid_value [0] = g_strdup ("gnomemeeting");
      sappid_value [1] = NULL;
      mods [2]->mod_op = LDAP_MOD_ADD | LDAP_MOD_REPLACE;
      mods [2]->mod_type = g_strdup ("sappid");
      mods [2]->mod_values = sappid_value;
      
      // protid
      mods [3] = new (LDAPMod);
      protid_value [0] = g_strdup ("h323");
      protid_value [1] = NULL;
      mods [3]->mod_op = LDAP_MOD_ADD | LDAP_MOD_REPLACE;
      mods [3]->mod_type = g_strdup ("sprotid");
      mods [3]->mod_values = protid_value;
      
      // Sip address
      mods [4] = new (LDAPMod);
      //  sip = inet_addr (ip);
      //  sprintf (ip, "%lu", sip); 
      sipaddress_value [0] = g_strdup ("130.104.1.1");
      sipaddress_value [1] = NULL;
      mods [4]->mod_op = LDAP_MOD_ADD | LDAP_MOD_REPLACE;
      mods [4]->mod_type = g_strdup ("sipaddress");
      mods [4]->mod_values = sipaddress_value;
      
      // the firstname
      mods [5] = new (LDAPMod);
      firstname_value [0] = g_strdup (opts->firstname);
      firstname_value [1] = NULL;
      mods [5]->mod_op = LDAP_MOD_ADD | LDAP_MOD_REPLACE;
      mods [5]->mod_type = g_strdup ("givenname");
      mods [5]->mod_values = firstname_value;
  
      // the surname
      mods [6] = new (LDAPMod);
      surname_value [0] = g_strdup (opts->surname);
      surname_value [1] = NULL;
      mods [6]->mod_op = LDAP_MOD_ADD | LDAP_MOD_REPLACE;
      mods [6]->mod_type = g_strdup ("surname");
      mods [6]->mod_values = surname_value;
  
      // the mail
      mods [7] = new (LDAPMod);
      mail_value [0] = g_strdup (opts->mail);
      mail_value [1] = NULL;
      mods [7]->mod_op = LDAP_MOD_ADD | LDAP_MOD_REPLACE;
      mods [7]->mod_type = g_strdup ("rfc822mailbox");
      mods [7]->mod_values = mail_value;

      // the comment
      mods [8] = new (LDAPMod);
      comment_value [0] = g_strdup (opts->comment);
      comment_value [1] = NULL;
      mods [8]->mod_op = LDAP_MOD_ADD | LDAP_MOD_REPLACE;
      mods [8]->mod_type = g_strdup ("comment");
      mods [8]->mod_values = comment_value;

      // the location
      mods [9] = new (LDAPMod);
      location_value [0] = g_strdup (opts->location);
      location_value [1] = NULL;
      mods [9]->mod_op = LDAP_MOD_ADD | LDAP_MOD_REPLACE;
      mods [9]->mod_type = g_strdup ("location");
      mods [9]->mod_values = location_value;
  
      // Audio Capable ?
      mods [10] = new (LDAPMod);
      ilsa32833566_value [0] = g_strdup ("1");
      ilsa32833566_value [1] = NULL;
      mods [10]->mod_op = LDAP_MOD_ADD | LDAP_MOD_REPLACE;
      mods [10]->mod_type = g_strdup ("ilsa32833566");
      mods [10]->mod_values = ilsa32833566_value;

      // ilsa26214430
      mods [11] = new (LDAPMod);
      ilsa26214430_value [0] = g_strdup ("0");
      ilsa26214430_value [1] = NULL;
      mods [11]->mod_op = LDAP_MOD_ADD | LDAP_MOD_REPLACE;
      mods [11]->mod_type = g_strdup ("ilsa26214430");
      mods [11]->mod_values = ilsa26214430_value;
      
      // Video Capable ?
      mods [12] = new (LDAPMod);
      ilsa32964638_value [0] = g_strdup ("1");
      /*
	if ( (opts.vid_tr) && (GM_cam (opts.video_device, opts.video_channel)) )
	strcpy (ilsa32964638_value [0], "1");
	else*/
      ilsa32964638_value [1] = NULL;
      mods [12]->mod_op = LDAP_MOD_ADD | LDAP_MOD_REPLACE;
      mods [12]->mod_type = g_strdup ("ilsa32964638");
      mods [12]->mod_values = ilsa32964638_value;

      // sport
      mods [13] = new (LDAPMod);
      sport_value [0] = g_strdup ("1720");
      sport_value [1] = NULL;
      mods [13]->mod_op = LDAP_MOD_ADD | LDAP_MOD_REPLACE;
      mods [13]->mod_type = g_strdup ("sport");
      mods [13]->mod_values = sport_value;
      
      mods [14] = NULL;


      dn = g_strdup_printf ("cn=%s,objectclass=rtperson", opts->mail);

      // Asynchronously add or remove the entry
      if (reg)
	msgid = ldap_add (ldap_connection, dn, mods);
      else
	msgid = ldap_delete (ldap_connection, dn);

      // If there is an error, display it...
      if (msgid == -1)
	{
	  cout << ldap_err2string (msgid) << endl << flush;
	  error = 1;
	}
      else
	{
	  // There was no direct error
	  // Block until the result is ok or ko
	  t.tv_sec = 10;
	  t.tv_usec = 0;

	  rc = ldap_result (ldap_connection, msgid, 0, &t, &res);

	  // If the server gave no answer after the timeout has elapsed
	  // then abandon
	  if (rc == 0)
	    {
	      ldap_abandon (ldap_connection, msgid);
	      rc = -1;
	    }

	  if(rc == -1)
	    {
	      gdk_threads_enter ();
	      
	      msg = g_strdup_printf (_("Error while connecting to ILS directory %s, port %s:\nNo answer from server."), opts->ldap_server, opts->ldap_port);

	      msg_box = gnome_message_box_new (msg, GNOME_MESSAGE_BOX_ERROR,
					       GNOME_STOCK_BUTTON_OK, NULL);
	      g_free (msg);
	      gtk_widget_show (msg_box);
	      
	      error = 1;
	      
	      gdk_threads_leave ();
	    }
	  else
	    {
	      gdk_threads_enter ();

	      if (reg)
		msg = g_strdup_printf (_("Sucessfully registered to ILS directory %s, port %s"), opts->ldap_server, opts->ldap_port);
	      else
		msg = g_strdup_printf (_("Sucessfully unregistered from ILS directory %s, port %s"), opts->ldap_server, opts->ldap_port);
	      GM_log_insert (gw->log_text, msg);
	      g_free (msg);

	      gdk_threads_leave ();
	    }
	}

      // We free things
      g_free (ilsa32833566_value [0]);
      g_free (ilsa32964638_value [0]);
      g_free (ilsa26214430_value [0]);
      g_free (objectclass_value [0]);
      g_free (sport_value [0]);
      g_free (sappid_value [0]);
      g_free (protid_value [0]);
      g_free (dn);

      for (int i = 0 ; i < 14 ; i++)
	{
	  g_free (mods [i]->mod_type);
	  delete (mods [i]);
	}
      
      ldap_unbind (ldap_connection);
      
    }

  if (reg)
    has_to_register = 0;
  else
    has_to_unregister = 0;

  return TRUE;
}


void GMILSClient::ils_browse (GM_ldap_window_widgets *lwi)
{
  lw = lwi;
  has_to_browse = 1;
}

void GMILSClient::ils_browse ()
{
  char *datas [] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
  char *attrs [] = { "surname", "givenname", "comment", "location", 
		     "rfc822mailbox", "sipaddress", "ilsa32833566", 
		     "ilsa32964638", NULL };
  
  LDAPMessage *res = NULL, *e = NULL;

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

  if (!strcmp (opts->ldap_server, ""))
    {
      gdk_threads_enter ();
      gnome_appbar_push (GNOME_APPBAR (lw->statusbar), 
			 _("Please provide an ILS directory in Settings"));
      gdk_threads_leave ();
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

  ldap_connection = ldap_open (opts->ldap_server, atoi (opts->ldap_port));

  if (ldap_connection == NULL)
    {
      gdk_threads_enter ();
      gnome_appbar_push (GNOME_APPBAR (lw->statusbar), 
			 _("Error while connecting to ILS directory"));
      gdk_threads_leave ();
    }

  if (ldap_bind_s (ldap_connection, NULL, NULL, LDAP_AUTH_SIMPLE ) 
      != LDAP_SUCCESS ) 
    {
      gdk_threads_enter ();
      gnome_appbar_push (GNOME_APPBAR (lw->statusbar), 
			 _("Error while connecting to ILS directory"));
      gdk_threads_leave ();
    }


  gdk_threads_enter ();        

  if (lw)
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

  if (lw)
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

      // Check if the window is still present or not
      if (lw)
	gtk_clist_append (GTK_CLIST (lw->ldap_users_clist), (gchar **) datas);
    
      /* Video Capable ? */
      if (ldap_get_values(ldap_connection, e, "ilsa32964638") != NULL)
	nmip = atoi (ldap_get_values(ldap_connection, e, "ilsa32964638") [0]);
      
      if (nmip == 1)
	{
	  if (lw)
	    gtk_clist_set_pixmap (GTK_CLIST (lw->ldap_users_clist), 
				  GTK_CLIST (lw->ldap_users_clist)->rows - 1, 1, 
				  quickcam, quickcam_mask);
	}

      /* Audio Capable ? */
      if (ldap_get_values(ldap_connection, e, "ilsa32833566") != NULL)
	nmip = atoi (ldap_get_values(ldap_connection, e, "ilsa32833566") [0]);
      
      if (nmip == 1)
	{
	  if (lw)
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

  has_to_browse = 0;
}

/******************************************************************************/
