
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
 *                         ils.cpp  -  description
 *                         -----------------------
 *   begin                : Sun Sep 23 2001
 *   copyright            : (C) 2000-2001 by Damien Sandras
 *   description          : The ldap thread.
 *   email                : dsandras@seconix.com
 *
 */
	
#include <sys/time.h>

#include "../config.h"

#include "ils.h"
#include "gnomemeeting.h"
#include "videograbber.h"
#include "common.h"
#include "misc.h"


/* Declarations */
extern GnomeMeeting *MyApp;
extern GtkWidget *gm;

static int gnomemeeting_ldap_window_appbar_update (gpointer); 


/* Callbacks */
int gnomemeeting_ldap_window_appbar_update (gpointer data) 
{
  GtkWidget *statusbar = (GtkWidget *) data;
  
  GtkProgressBar *progress = 
    gnome_appbar_get_progress (GNOME_APPBAR (statusbar));

  gtk_progress_bar_pulse (GTK_PROGRESS_BAR (progress));
   
  return 1;
}


/* The methods */
GMILSClient::GMILSClient ()
  :PThread (1000, NoAutoDeleteThread)
{
  gw = gnomemeeting_get_main_window (gm);
  lw = gnomemeeting_get_ldap_window (gm);

  running = 1;
  in_the_loop = 0;
  has_to_register = 0;
  has_to_unregister = 0;
  has_to_browse = 0;
  registered = 0;

  client = gconf_client_get_default ();

  ldap_server = NULL;
  ldap_port = g_strdup ("389");
  firstname = NULL;
  surname = NULL;
  mail = NULL;
  comment = NULL;
  location = NULL;

  ldap_search_connection = NULL;
  rc_search_connection = -1;

  UpdateConfig ();
  Resume ();
}


GMILSClient::~GMILSClient ()
{
  running = 0;

  quit_mutex.Wait ();

  /* After the thread has stopped,
     we unregister if we were registered */
  if (registered == 1)
    Register (FALSE);
}


void GMILSClient::Main ()
{
  quit_mutex.Wait ();

  while (running == 1) {
  
    /* The most important operation is to unregister */
    if (has_to_unregister == 1) {
     
      /* Unregister the old values */
      Register (FALSE);
      UpdateConfig ();
    }

    if (has_to_register == 1) {

      UpdateConfig ();
      Register (TRUE);
    }

    if (has_to_browse == 1)
      ils_browse ();

    PTimeInterval t = PTime () - starttime;

    /* if there is more than 20 minutes that we are registered,
       we refresh the entry */
    if ((t.GetSeconds () > 1200) && 
	(gconf_client_get_bool (GCONF_CLIENT (client), 
				"/apps/gnomemeeting/ldap/register", 
				NULL))) {

	has_to_register = 1;
	starttime = PTime ();
      }

    Current ()->Sleep (500);
  }

  quit_mutex.Signal ();
}


void GMILSClient::UpdateConfig ()
{
  g_free (ldap_server);
  g_free (firstname);
  g_free (surname);
  g_free (mail);
  g_free (comment);
  g_free (location);

  ldap_server =  
    gconf_client_get_string (GCONF_CLIENT (client),
			     "/apps/gnomemeeting/ldap/ldap_server", 
			     NULL);
  firstname =  
    gconf_client_get_string (GCONF_CLIENT (client),
			     "/apps/gnomemeeting/personal_data/firstname", 
			     NULL);
  surname =  
    gconf_client_get_string (GCONF_CLIENT (client),
			     "/apps/gnomemeeting/personal_data/lastname", 
			     NULL);
  mail =  
    gconf_client_get_string (GCONF_CLIENT (client),
			     "/apps/gnomemeeting/personal_data/mail", 
			     NULL);
  comment =  
    gconf_client_get_string (GCONF_CLIENT (client),
			     "/apps/gnomemeeting/personal_data/comment", 
			     NULL);
  location =  
    gconf_client_get_string (GCONF_CLIENT (client),
			     "/apps/gnomemeeting/personal_data/location", 
			     NULL);
}


void GMILSClient::stop ()
{
  running = 0;
}


void GMILSClient::Register ()
{
  has_to_register = 1;
}


void GMILSClient::Unregister ()
{
  has_to_unregister = 1;
}


BOOL GMILSClient::Register (BOOL reg)
{
  LDAPMessage *res=NULL;
  LDAPMod *mods [17];

  char *firstname_value [2];
  char *surname_value [2];
  char *mail_value [2];
  char *comment_value [2];
  char *location_value [2];
  char *ilsa32833566_value [2];
  char *ilsa32964638_value [2];
  char *ilsa26214430_value [2];
  char *ilsa26279966_value [2];
  char *sipaddress_value [2];
  char *sport_value [2];
  char *sappid_value [2];
  char *protid_value [2];
  char *objectclass_value [2];
  char *cn_value [2];
  char *sflags_value [2];

  char *dn = NULL;
  gchar *msg = NULL;
  gchar *ip = NULL;
  unsigned long sip = 0;
  int rc;
  int error = 0; 

  struct timeval t;

  GtkWidget *msg_box;

  /* if it asks to unregister and that we are not registered, 
     exit */
  if (!reg) {
    if (!registered) {
      has_to_unregister = 0;
      return TRUE;
    }
  }

  gnomemeeting_threads_enter ();
  msg = g_strdup_printf (_("Connecting to ILS directory %s, port %s"), 
			 ldap_server, ldap_port);
  gnomemeeting_log_insert (msg);
  g_free (msg);
  gtk_widget_set_sensitive (GTK_WIDGET (lw->refresh_button), FALSE);
  gnomemeeting_threads_leave ();

  ldap_connection = ldap_open (ldap_server, atoi (ldap_port));

  if ((ldap_connection == NULL) || 
      (ldap_bind_s (ldap_connection, NULL, NULL, LDAP_AUTH_SIMPLE)
       != LDAP_SUCCESS)) {
    gnomemeeting_threads_enter ();

    msg_box = gtk_message_dialog_new (GTK_WINDOW (gm),
				      GTK_DIALOG_DESTROY_WITH_PARENT,
				      GTK_MESSAGE_ERROR,
				      GTK_BUTTONS_CLOSE,
				      _("Error while connecting to ILS directory %s, port %s"), ldap_server, ldap_port);

    g_signal_connect_swapped (GTK_OBJECT (msg_box), "response",
			      G_CALLBACK (gtk_widget_destroy),
			      GTK_OBJECT (msg_box));

    gtk_widget_show (msg_box);

    error = 1;

    gnomemeeting_threads_leave ();
  }

  /* if there was no error while connecting */
  if (!error) {
    /* cn */
    mods [0] = new (LDAPMod);
    cn_value [0] = g_convert (mail, strlen (mail), "ISO-8859-1", "UTF8", 
			      0, 0, 0);
    cn_value [1] = NULL;
    mods [0]->mod_op = LDAP_MOD_ADD | LDAP_MOD_REPLACE;
    mods [0]->mod_type = g_strdup ("cn");
    mods [0]->mod_values = cn_value;

    /* objectclass */
    mods [1] = new (LDAPMod);
    objectclass_value [0] = g_strdup ("rtperson");
    objectclass_value [1] = NULL;
    mods [1]->mod_op = LDAP_MOD_ADD | LDAP_MOD_REPLACE;
    mods [1]->mod_type = g_strdup ("objectclass");
    mods [1]->mod_values = objectclass_value;

    /* Sappid */
    mods [2] = new (LDAPMod);
    sappid_value [0] = g_strdup ("GnomeMeeting");
    sappid_value [1] = NULL;
    mods [2]->mod_op = LDAP_MOD_ADD | LDAP_MOD_REPLACE;
    mods [2]->mod_type = g_strdup ("sappid");
    mods [2]->mod_values = sappid_value;
      
    /* protid */
    mods [3] = new (LDAPMod);
    protid_value [0] = g_strdup ("h323");
    protid_value [1] = NULL;
    mods [3]->mod_op = LDAP_MOD_ADD | LDAP_MOD_REPLACE;
    mods [3]->mod_type = g_strdup ("sprotid");
    mods [3]->mod_values = protid_value;
      
    /* Sip address */
    mods [4] = new (LDAPMod);
    ip = MyApp->Endpoint ()->GetCurrentIP ();
    sip = inet_addr (ip);
    g_free (ip); /* we do not need the IP anymore */
    sipaddress_value [0] = g_strdup_printf ("%lu", sip);
    sipaddress_value [1] = NULL;
    mods [4]->mod_op = LDAP_MOD_ADD | LDAP_MOD_REPLACE;
    mods [4]->mod_type = g_strdup ("sipaddress");
    mods [4]->mod_values = sipaddress_value;
      
    /* the firstname */
    mods [5] = new (LDAPMod);
    firstname_value [0] = g_convert (firstname, strlen (firstname), 
				     "ISO-8859-1", "UTF8", 0, 0, 0);
    firstname_value [1] = NULL;
    mods [5]->mod_op = LDAP_MOD_ADD | LDAP_MOD_REPLACE;
    mods [5]->mod_type = g_strdup ("givenname");
    mods [5]->mod_values = firstname_value;
  
    /* the surname */
    mods [6] = new (LDAPMod);
    surname_value [0] = g_convert (surname, strlen (surname), 
				   "ISO-8859-1", "UTF8", 0, 0, 0);
    surname_value [1] = NULL;
    mods [6]->mod_op = LDAP_MOD_ADD | LDAP_MOD_REPLACE;
    mods [6]->mod_type = g_strdup ("surname");
    mods [6]->mod_values = surname_value;
  
    /* the mail */
    mods [7] = new (LDAPMod);
    mail_value [0] = g_strdup (mail);
    mail_value [1] = NULL;
    mods [7]->mod_op = LDAP_MOD_ADD | LDAP_MOD_REPLACE;
    mods [7]->mod_type = g_strdup ("rfc822mailbox");
    mods [7]->mod_values = mail_value;

    /* the comment */
    mods [8] = new (LDAPMod);
    comment_value [0] = g_convert (comment, strlen (comment), 
				   "ISO-8859-1", "UTF8", 0, 0, 0);
    comment_value [1] = NULL;
    mods [8]->mod_op = LDAP_MOD_ADD | LDAP_MOD_REPLACE;
    mods [8]->mod_type = g_strdup ("comment");
    mods [8]->mod_values = comment_value;
    
    /* the location */
    mods [9] = new (LDAPMod);
    location_value [0] = g_convert (location, strlen (location), 
				    "ISO-8859-1", "UTF8", 0, 0, 0);
    location_value [1] = NULL;
    mods [9]->mod_op = LDAP_MOD_ADD | LDAP_MOD_REPLACE;
    mods [9]->mod_type = g_strdup ("location");
    mods [9]->mod_values = location_value;
  
    /* Audio Capable ? */
    mods [10] = new (LDAPMod);
    ilsa32833566_value [0] = g_strdup ("1");
    ilsa32833566_value [1] = NULL;
    mods [10]->mod_op = LDAP_MOD_ADD | LDAP_MOD_REPLACE;
    mods [10]->mod_type = g_strdup ("ilsa32833566");
    mods [10]->mod_values = ilsa32833566_value;

    /* ilsa26214430 */
    mods [11] = new (LDAPMod);
    ilsa26214430_value [0] = g_strdup ("0");
    ilsa26214430_value [1] = NULL;
    mods [11]->mod_op = LDAP_MOD_ADD | LDAP_MOD_REPLACE;
    mods [11]->mod_type = g_strdup ("ilsa26214430");
    mods [11]->mod_values = ilsa26214430_value;
      
    /* Video Capable ? */
    mods [12] = new (LDAPMod);
    ilsa32964638_value [0] = g_strdup ("1");
    ilsa32964638_value [1] = NULL;
    mods [12]->mod_op = LDAP_MOD_ADD | LDAP_MOD_REPLACE;
    mods [12]->mod_type = g_strdup ("ilsa32964638");
    mods [12]->mod_values = ilsa32964638_value;

    /* sport */
    mods [13] = new (LDAPMod);
    sport_value [0] = g_strdup ("1720");
    sport_value [1] = NULL;
    mods [13]->mod_op = LDAP_MOD_ADD | LDAP_MOD_REPLACE;
    mods [13]->mod_type = g_strdup ("sport");
    mods [13]->mod_values = sport_value;

    /* sport */
    mods [14] = new (LDAPMod);
    sflags_value [0] = g_strdup ("1");
    sflags_value [1] = NULL;
    mods [14]->mod_op = LDAP_MOD_ADD | LDAP_MOD_REPLACE;
    mods [14]->mod_type = g_strdup ("sflags");
    mods [14]->mod_values = sflags_value;

    /* ilsa */
    mods [15] = new (LDAPMod);
    ilsa26279966_value [0] = g_strdup_printf ("%u",
					      MAJOR_VERSION << 24 | 
					      MINOR_VERSION << 16 |
					      BUILD_NUMBER);
    ilsa26279966_value [1] = NULL;
    mods [15]->mod_op = LDAP_MOD_ADD | LDAP_MOD_REPLACE;
    mods [15]->mod_type = g_strdup ("ilsa26279966");
    mods [15]->mod_values = ilsa26279966_value;

    mods [16] = NULL;


    dn = g_strdup_printf ("c=-,o=Gnome,cn=%s,objectclass=rtperson", mail);

    /* Asynchronously add or remove the entry */
    if (reg) {
      msgid = ldap_add (ldap_connection, dn, mods);
      registered = 1;
    }
    else {
      msgid = ldap_delete (ldap_connection, dn);
      registered = 0;
    }

    /* If there is an error, display it... */
    if (msgid == -1) {
      cerr << ldap_err2string (msgid) << endl << flush;
      error = 1;
    }
    else {
      /* There was no direct error,
	 Block until the result is ok or ko */
      t.tv_sec = 10;
      t.tv_usec = 0;

      rc = ldap_result (ldap_connection, msgid, 0, &t, &res);

      /* If the server gave no answer after the timeout has elapsed,
	 then abandon */
      if (rc == 0) {
	ldap_abandon (ldap_connection, msgid);
	rc = -1;
      }

      /* There was an error */
      if(rc == -1) {
	gnomemeeting_threads_enter ();
	
	msg_box = gtk_message_dialog_new (GTK_WINDOW (gm),
					  GTK_DIALOG_MODAL,
					  GTK_MESSAGE_ERROR,
					  GTK_BUTTONS_CLOSE,
					  _("Error while connecting to ILS directory %s, port %s"), ldap_server, ldap_port);
	
	g_signal_connect_swapped (GTK_OBJECT (msg_box), "response",
				  G_CALLBACK (gtk_widget_destroy),
				  GTK_OBJECT (msg_box));
	
	gtk_widget_show (msg_box);
	      
	error = 1;
	      
	gnomemeeting_threads_leave ();
      }
      else {
	gnomemeeting_threads_enter ();
	
	if (reg) {

	  msg = g_strdup_printf (_("Sucessfully registered to ILS directory %s, port %s"), ldap_server, ldap_port);
	  gnomemeeting_statusbar_flash (gm, _("Succesfully registered to ILS server"));
	  starttime = PTime ();
	}
	else {

	  msg = g_strdup_printf (_("Sucessfully unregistered from ILS directory %s, port %s"), ldap_server, ldap_port);
	  gnomemeeting_statusbar_flash (gm, _("Succesfully unregistered from ILS server"));
	}
	
	gnomemeeting_log_insert (msg);
	g_free (msg);
	
	gtk_widget_set_sensitive (GTK_WIDGET (lw->refresh_button), TRUE);
	gnomemeeting_threads_leave ();
      }
    }

    /* We free things */
    g_free (ilsa32833566_value [0]);
    g_free (ilsa32964638_value [0]);
    g_free (ilsa26214430_value [0]);
    g_free (ilsa26279966_value [0]);
    g_free (objectclass_value [0]);
    g_free (sport_value [0]);
    g_free (sappid_value [0]);
    g_free (protid_value [0]);
    g_free (dn);
    
    for (int i = 0 ; i < 16 ; i++) {
      g_free (mods [i]->mod_type);
      delete (mods [i]);
    }
    
    /* unbind the connection */
    ldap_unbind (ldap_connection);
      
  } /* if (!error) */

  if (reg) {
    has_to_register = 0;
  }
  else {
    has_to_unregister = 0;
  }

  gnomemeeting_threads_enter ();
  gtk_widget_set_sensitive (GTK_WIDGET (lw->refresh_button), TRUE);
  gnomemeeting_threads_leave ();

  return TRUE;
}


void GMILSClient::ils_browse (int page)
{
  has_to_browse = 1;
  page_num = page;
}


gchar *GMILSClient::Search (gchar *ldap_server, gchar *ldap_port, gchar *mail)
{
  char *attrs [] = { "rfc822mailbox", "sipaddress", NULL };
  char **ldv = NULL;
  unsigned long int nmip = 0;
  int part1;
  int part2;
  int part3;
  int part4;
  struct timeval t = {10, 0};

  gchar *ip = NULL;
  gchar *cn = NULL;

  LDAPMessage *res = NULL, *e = NULL;


  if (((!strcmp (ldap_server, ""))&&(ldap_server)) 
      ||((!strcmp (ldap_port, ""))&&(ldap_port)) 
      ||((!strcmp (mail, ""))&&(mail)))
    return NULL;
 

  if ((ldap_search_connection != NULL)||(rc_search_connection != -1)) {
    
    ldap_abandon (ldap_search_connection, rc_search_connection);
	
  }

  /* Opens the connection to the ILS directory */
  ldap_search_connection = ldap_open (ldap_server, atoi (ldap_port));


  if ((ldap_search_connection == NULL) || 
      (rc_search_connection = 
       ldap_bind_s (ldap_search_connection, NULL, NULL, LDAP_AUTH_SIMPLE ) 
      != LDAP_SUCCESS)) {
    
    return NULL;
  }
  

  cn = g_strdup_printf ("(&(cn=%s))", mail);
  rc_search_connection = 
    ldap_search_st (ldap_search_connection, "objectClass=RTPerson", 
		    LDAP_SCOPE_BASE,
		    cn, attrs, 0, &t, &res); 
  g_free (cn);

  if (rc_search_connection != 0)
    return ip;

  /* We only take the first entry */
  e = ldap_first_entry (ldap_search_connection, res); 
  if (e)
    ldv = ldap_get_values (ldap_search_connection, e, "sipaddress");

  if (ldv != NULL) {

    nmip = strtoul (ldv [0], NULL, 10);
    ldap_value_free (ldv);
  }
      
  part1 = (int) (nmip/(256*256*256));
  part2 = (int) ((nmip - part1 * (256 * 256 * 256)) / (256 * 256));
  part3 = (int) ((nmip - part1 * (256 * 256 * 256) - part2 * (256 * 256)) 
		 / 256);
  part4 = (int) ((nmip - part1 * (256 * 256 * 256) - part2 * (256 * 256) 
		  - part3 * 256));
  
  ip = g_strdup_printf ("%d.%d.%d.%d", part4, part3, part2, part1);

  ldap_msgfree (res);
  ldap_unbind (ldap_search_connection);

  rc_search_connection = -1;
  ldap_search_connection = NULL;

  return ip;
}


void GMILSClient::ils_browse ()
{
  char *datas [] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
  char *attrs [] = { "surname", "givenname", "comment", "location", 
		     "rfc822mailbox", "sipaddress", "ilsa32833566", 
		     "ilsa32964638", "ilsa26279966", "sappid", NULL };
  char **ldv;

  LDAPMessage *res = NULL, *e = NULL;

  int rc = 0;
  unsigned long int nmip = 0;
  int part1;
  int part2;
  int part3;
  int part4;
  int cj = 0;
  int page_exists = 0; /* flag to see if the page still exists 
			  when filling the list */
  char ip [16];
  gchar *ldap_server = NULL;
  gchar *text_label = NULL;

  GtkProgressBar *progress;
  guint ils_timeout;
  GtkWidget *page = NULL, *label = NULL;
  GtkListStore *xdap_users_list_store = NULL;
  GtkTreeIter list_iter;
  int curr_page;
  bool audio = false, video = false;

  gnomemeeting_threads_enter ();
  gtk_widget_set_sensitive (GTK_WIDGET (lw->refresh_button), FALSE);
  ldap_server = (gchar *) gtk_entry_get_text 
    (GTK_ENTRY (GTK_COMBO (lw->ils_server_combo)->entry));
  gnomemeeting_threads_leave ();

  if (!strcmp (ldap_server, "")) {

    gnomemeeting_threads_enter ();
    gnome_appbar_push (GNOME_APPBAR (lw->statusbar), _("Please provide an ILS directory in the window"));
    gnomemeeting_threads_leave ();
    
    lw->thread_count--;
    has_to_browse = 0;
    
    gnomemeeting_threads_enter ();
    gtk_widget_set_sensitive (GTK_WIDGET (lw->refresh_button), TRUE);
    gnomemeeting_threads_leave ();
    
    return;
  }

  gnomemeeting_threads_enter ();

  progress = gnome_appbar_get_progress (GNOME_APPBAR (lw->statusbar));
  
  gnome_appbar_push (GNOME_APPBAR (lw->statusbar), 
		     _("Connecting to ILS directory... Please Wait."));


  /* We add a timeout to make the status indicator move in activity mode */
  ils_timeout = gtk_timeout_add (50, gnomemeeting_ldap_window_appbar_update, 
				 lw->statusbar);

  gnomemeeting_threads_leave ();


  /* Opens the connection to the ILS directory */
  ldap_connection = ldap_open (ldap_server, 389);

  if (ldap_connection == NULL) {

    gnomemeeting_threads_enter ();
    gnome_appbar_push (GNOME_APPBAR (lw->statusbar), 
		       _("Error while connecting to ILS directory"));
    gtk_widget_set_sensitive (GTK_WIDGET (lw->refresh_button), TRUE);
    
    /* Remove the timeout */
    gtk_timeout_remove (ils_timeout);
    
    gnomemeeting_threads_leave ();
    
    lw->thread_count--;
    has_to_browse = 0;

    return;
  }
  
  if (ldap_bind_s (ldap_connection, NULL, NULL, LDAP_AUTH_SIMPLE ) 
      != LDAP_SUCCESS)  {

    gnomemeeting_threads_enter ();
    gnome_appbar_push (GNOME_APPBAR (lw->statusbar), 
		       _("Error while connecting to ILS directory"));
    gtk_widget_set_sensitive (GTK_WIDGET (lw->refresh_button), TRUE);
    
    /* Remove the timeout */
    gtk_timeout_remove (ils_timeout);
        
    /* Remove the current page */
    gtk_notebook_remove_page (GTK_NOTEBOOK (lw->notebook), page_num);
    if (page_num == 1)
      gtk_widget_show (gtk_notebook_get_nth_page 
		       (GTK_NOTEBOOK (lw->notebook),
			0));
    
    gnomemeeting_threads_leave ();

    lw->thread_count--;
    has_to_browse = 0;
    
    return;
  }

  gnomemeeting_threads_enter ();        

  gnome_appbar_push (GNOME_APPBAR (lw->statusbar), 
		     _("Fetching users' information... Please Wait."));

  gnomemeeting_threads_leave ();

  rc = ldap_search_s (ldap_connection, "objectClass=RTPerson", 
		      LDAP_SCOPE_BASE,
		      "(&(cn=%))",
		      attrs, 0, &res); 

  for (int i = 0 ; i < 9 ; i++)
    datas [i] = NULL;

  gnomemeeting_threads_enter ();

  gnome_appbar_push (GNOME_APPBAR (lw->statusbar), 
		     _("Search completed!"));

  curr_page = gtk_notebook_get_current_page (GTK_NOTEBOOK (lw->notebook));
  page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (lw->notebook), curr_page);
  ldap_server = (gchar *) gtk_entry_get_text 
    (GTK_ENTRY (GTK_COMBO (lw->ils_server_combo)->entry));


  /* Check if the page with the ldap_server we are browsing still exists */
  while ((page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (lw->notebook), 
					    cj))) {
     
    label = gtk_notebook_get_tab_label (GTK_NOTEBOOK (lw->notebook), page);
    label = (GtkWidget *) g_object_get_data (G_OBJECT (label), "label");
    text_label = (gchar *) gtk_label_get_text (GTK_LABEL (label));
    if (!(strcasecmp (text_label, ldap_server))) {

      page_exists = 1;
      break;
    }

    cj++;
  }

  if (page != NULL)
    xdap_users_list_store = 
      GTK_LIST_STORE (g_object_get_data (G_OBJECT (page), "list_store"));

  /*Maybe the user closed the tab while we were waiting */
  if ((xdap_users_list_store != NULL) && (page_exists)) { 
    
    for (e = ldap_first_entry(ldap_connection, res); 
	 e != NULL; e = ldap_next_entry(ldap_connection, e)) {
      
      
      ldv = ldap_get_values (ldap_connection, e, "surname");
      if (ldv != NULL) {
	
	datas [3] = 
	  g_strdup (ldv [0]);
	ldap_value_free (ldv);
      }
      
      ldv = ldap_get_values(ldap_connection, e, "givenname");
      if (ldv != NULL) {
	
	datas [2] = g_strdup (ldv [0]);
	ldap_value_free (ldv);
      }
      
      ldv = ldap_get_values(ldap_connection, e, "location");
      if (ldv != NULL) {
	
	datas [5] = g_strdup (ldv [0]);
	ldap_value_free (ldv);
      }
      
      ldv = ldap_get_values(ldap_connection, e, "comment");
      if (ldv != NULL)	{
	
	datas [6] = 
	  g_strdup (ldv [0]);
	ldap_value_free (ldv);
      }

      ldv = ldap_get_values(ldap_connection, e, "sappid");
      if (ldv != NULL)	{

	datas [7]  =
	  g_strdup (ldv [0]);
	ldap_value_free (ldv);
      }

      ldv = ldap_get_values(ldap_connection, e, "ilsa26279966");
      if (ldv != NULL)	{
	gchar *value =
	  g_strdup (ldv [0]);
	
	unsigned int v, a, b, c;
	v = atoi (value);
	a = (v & 0xff000000) >> 24;
	b = (v & 0x00ff0000) >> 16;
	c = v & 0x0000ffff;
	g_free (value);

	if (datas [7] != NULL)
	  datas [7] = g_strdup_printf ("%s %d.%d.%d", datas [7], a, b, c); 
	else
	  datas [7] = g_strdup_printf ("%d.%d.%d", a, b, c); 

	ldap_value_free (ldv);
      }

      ldv = ldap_get_values(ldap_connection, e, "rfc822mailbox");
      if (ldv != NULL) {
	
	datas [4] = g_strdup (ldv [0]);
	ldap_value_free (ldv);
      }
      
      ldv = ldap_get_values(ldap_connection, e, "sipaddress");
      if (ldv != NULL) {
	
	nmip = strtoul (ldv [0], NULL, 10);
	ldap_value_free (ldv);
      }
      
      part1 = (int) (nmip/(256*256*256));
      part2 = (int) ((nmip - part1 * (256 * 256 * 256)) / (256 * 256));
      part3 = (int) ((nmip - part1 * (256 * 256 * 256) - part2 * (256 * 256)) 
		     / 256);
      part4 = (int) ((nmip - part1 * (256 * 256 * 256) - part2 * (256 * 256) 
		      - part3 * 256));
      
      sprintf (ip, "%d.%d.%d.%d", part4, part3, part2, part1);
      /* ip will be freed (char ip [16]), so we make a copy in datas [7] */
      
      datas [8] = g_strdup ((char *) ip);
      

      /* Video Capable ? */
      ldv = ldap_get_values(ldap_connection, e, "ilsa32964638");
      if (ldv != NULL) {
	
 	video = 
	  (bool)atoi (ldv [0]);
 	ldap_value_free (ldv);
      }
      

      /* Audio Capable ? */
      ldv = ldap_get_values(ldap_connection, e, "ilsa32833566");
      if (ldv != NULL) {
	
 	audio = 
	  (bool)atoi (ldv [0]);
 	ldap_value_free (ldv);
      }
      

      /* Check if the window is still present or not */
      if (lw) {

	gchar *utf8_data [7] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL};

 	for (int j = 0 ; j < 7 ; j++) 

	  /* Strings are already UTF8-encoded for LDAP v3,
	     but Microsoft ILS doesn't take care of that */
	  if (datas [j + 2])
	    utf8_data [j] = g_convert (datas [j+2], strlen (datas [j + 2]),
				       "UTF8", "ISO-8859-1", NULL, NULL, NULL);

	gtk_list_store_append (xdap_users_list_store, &list_iter);
	gtk_list_store_set (xdap_users_list_store, &list_iter,
			    COLUMN_AUDIO, audio,
			    COLUMN_VIDEO, video,
			    COLUMN_FIRSTNAME, utf8_data [0],
 			    COLUMN_LASTNAME, utf8_data [1],
 			    COLUMN_EMAIL, utf8_data [2],
			    COLUMN_LOCATION, utf8_data [3],
 			    COLUMN_COMMENT, utf8_data [4],
 			    COLUMN_VERSION, utf8_data [5],
 			    COLUMN_IP, utf8_data [6],
			    -1);

	for (int j = 0 ; j < 7 ; j++)
	  if (utf8_data [j])
	    g_free (utf8_data [j]);

      }   
      
      for (int j = 2 ; j <= 8 ; j++) {
	
	if (datas [j] != NULL)
	  g_free (datas [j]);
	
	datas [j] = NULL;
      }

    } /* end of for */
  }

  ldap_msgfree (res);
  ldap_unbind (ldap_connection);

  lw->thread_count = 0;

  has_to_browse = 0;

  /* Remove the timeout */
  gtk_timeout_remove (ils_timeout);


  /* Enable sensitivity */
  gtk_widget_set_sensitive (GTK_WIDGET (lw->refresh_button), TRUE);
  gnomemeeting_threads_leave ();
}
