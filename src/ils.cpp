
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

#include "../pixmaps/quickcam.xpm"
#include "../pixmaps/sound.xpm"


/* Declarations */
extern GnomeMeeting *MyApp;
extern GtkWidget *gm;

static int gnomemeeting_ldap_window_appbar_update (gpointer); 


/* Callbacks */
int gnomemeeting_ldap_window_appbar_update (gpointer data) 
{
  float val;
  GtkWidget *statusbar = (GtkWidget *) data;
  
  GtkProgress *progress = gnome_appbar_get_progress (GNOME_APPBAR (statusbar));

  val = gtk_progress_get_value(GTK_PROGRESS (progress));
  
  val += 0.5;
  
  if (val > 100) 
    val = 0;
   
  gtk_progress_set_value(GTK_PROGRESS(progress), val);
   
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
	(gconf_client_get_bool (GCONF_CLIENT (client), "/apps/gnomemeeting/ldap/register", NULL))) {

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

  ldap_server =  gconf_client_get_string (GCONF_CLIENT (client),
					  "/apps/gnomemeeting/ldap/ldap_server", 
					  NULL);
  firstname =  gconf_client_get_string (GCONF_CLIENT (client),
					"/apps/gnomemeeting/personal_data/firstname", 
					NULL);
  surname =  gconf_client_get_string (GCONF_CLIENT (client),
				      "/apps/gnomemeeting/personal_data/lastname", 
				      NULL);
  mail =  gconf_client_get_string (GCONF_CLIENT (client),
				   "/apps/gnomemeeting/personal_data/mail", 
				   NULL);
  comment =  gconf_client_get_string (GCONF_CLIENT (client),
				      "/apps/gnomemeeting/personal_data/comment", 
				      NULL);
  location =  gconf_client_get_string (GCONF_CLIENT (client),
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

    msg = g_strdup_printf (_("Error while connecting to ILS directory %s, port %s"), ldap_server, ldap_port);
    msg_box = gnome_message_box_new (msg, GNOME_MESSAGE_BOX_ERROR,
				     GNOME_STOCK_BUTTON_OK, NULL);
    g_free (msg);
    gtk_widget_show (msg_box);

    error = 1;

    gnomemeeting_threads_leave ();
  }

  /* if there was no error while connecting */
  if (!error) {
    /* cn */
    mods [0] = new (LDAPMod);
    cn_value [0] = g_strdup (mail);
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
    firstname_value [0] = g_strdup (firstname);
    firstname_value [1] = NULL;
    mods [5]->mod_op = LDAP_MOD_ADD | LDAP_MOD_REPLACE;
    mods [5]->mod_type = g_strdup ("givenname");
    mods [5]->mod_values = firstname_value;
  
    /* the surname */
    mods [6] = new (LDAPMod);
    surname_value [0] = g_strdup (surname);
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
    comment_value [0] = g_strdup (comment);
    comment_value [1] = NULL;
    mods [8]->mod_op = LDAP_MOD_ADD | LDAP_MOD_REPLACE;
    mods [8]->mod_type = g_strdup ("comment");
    mods [8]->mod_values = comment_value;
    
    /* the location */
    mods [9] = new (LDAPMod);
    location_value [0] = g_strdup (location);
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
    ilsa26279966_value [0] = g_strdup ("5505024");
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
	
	msg = g_strdup_printf (_("Error while connecting to ILS directory %s, port %s:\nNo answer from server."), ldap_server, ldap_port);
	
	msg_box = gnome_message_box_new (msg, GNOME_MESSAGE_BOX_ERROR,
					 GNOME_STOCK_BUTTON_OK, NULL);
	g_free (msg);
	gtk_widget_show (msg_box);
	      
	error = 1;
	      
	gnomemeeting_threads_leave ();
      }
      else {
	gnomemeeting_threads_enter ();
	
	if (reg) {
	  msg = g_strdup_printf (_("Sucessfully registered to ILS directory %s, port %s"), ldap_server, ldap_port);
	  gnome_appbar_push (GNOME_APPBAR (gw->statusbar), _("Succesfully registered to ILS server"));
	  starttime = PTime ();
	}
	else {
	  msg = g_strdup_printf (_("Sucessfully unregistered from ILS directory %s, port %s"), ldap_server, ldap_port);
	  gnome_appbar_push (GNOME_APPBAR (gw->statusbar), _("Succesfully unregistered from ILS server"));
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


void GMILSClient::ils_browse ()
{
  char *datas [] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
  char *attrs [] = { "surname", "givenname", "comment", "location", 
		     "rfc822mailbox", "sipaddress", "ilsa32833566", 
		     "ilsa32964638", "ilsa26279966", "sappid", NULL };
  
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

  GdkPixmap *quickcam;
  GdkBitmap *quickcam_mask;
  GdkPixmap *sound;
  GdkBitmap *sound_mask;
  GtkProgress *progress;
  guint ils_timeout;
  GtkWidget *page = NULL, *clist = NULL, *label = NULL;
  int curr_page;

  gnomemeeting_threads_enter ();
  gtk_widget_set_sensitive (GTK_WIDGET (lw->refresh_button), FALSE);
  ldap_server = gtk_entry_get_text 
    (GTK_ENTRY (GTK_COMBO (lw->ils_server_combo)->entry));
  gnomemeeting_threads_leave ();

  if (!strcmp (ldap_server, "")) {

    gnomemeeting_threads_enter ();
    gnome_appbar_push (GNOME_APPBAR (lw->statusbar), 
		       _("Please provide an ILS directory in the window"));
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
  
  quickcam = gdk_pixmap_create_from_xpm_d (gm->window, &quickcam_mask,
					   NULL,
					   (gchar **) quickcam_xpm);

  sound = gdk_pixmap_create_from_xpm_d (gm->window, &sound_mask,
					NULL,
					(gchar **) sound_xpm); 

  gnome_appbar_push (GNOME_APPBAR (lw->statusbar), 
		     _("Connecting to ILS directory... Please Wait."));

  /* We add a timeout to make the status indicator move in activity mode */
  gtk_progress_set_activity_mode (GTK_PROGRESS (progress), TRUE);
  gtk_progress_bar_set_activity_step (GTK_PROGRESS_BAR (progress), 4);
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
    gtk_progress_set_activity_mode (GTK_PROGRESS (progress), FALSE);
    gtk_progress_set_value(GTK_PROGRESS(progress), 0);
    
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
    gtk_progress_set_activity_mode (GTK_PROGRESS (progress), FALSE);
    gtk_progress_set_value(GTK_PROGRESS(progress), 0);
    
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

  /* Make the progress bar in activity mode go faster */
  gtk_progress_bar_set_activity_step (GTK_PROGRESS_BAR (progress), 20);

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
  ldap_server = gtk_entry_get_text 
    (GTK_ENTRY (GTK_COMBO (lw->ils_server_combo)->entry));


  /* Check if the page with the ldap_server we are browsing still exists */
  while ((page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (lw->notebook), cj))) {
     
    label = GTK_WIDGET 
      (g_list_first (gtk_container_children (GTK_CONTAINER
					     (gtk_notebook_get_tab_label 
					      (GTK_NOTEBOOK (lw->notebook), 
					       page))))->data);
    gtk_label_get (GTK_LABEL (label), &text_label);
    if (!(g_strcasecmp (text_label, ldap_server))) {

      page_exists = 1;
      break;
    }

    cj++;
  }

  if (page != NULL)
    clist = GTK_WIDGET (gtk_object_get_data (GTK_OBJECT (page), 
					     "ldap_users_clist"));

  /*Maybe the user closed the tab while we were waiting */
  if ((clist != NULL)&&(page_exists)) { 
    
    gtk_clist_freeze (GTK_CLIST (clist));
    for (e = ldap_first_entry(ldap_connection, res); 
	 e != NULL; e = ldap_next_entry(ldap_connection, e)) {
      
      if (ldap_get_values (ldap_connection, e, "surname") != NULL) {
	
	datas [3] = 
	  g_strdup (ldap_get_values (ldap_connection, e, "surname") [0]);
	ldap_value_free (ldap_get_values (ldap_connection, e, "surname"));
      }
      
      if (ldap_get_values(ldap_connection, e, "givenname") != NULL) {
	
	datas [2] = g_strdup (ldap_get_values 
			      (ldap_connection, e, "givenname") [0]);
	ldap_value_free (ldap_get_values (ldap_connection, e, "givenname"));
      }
      
      if (ldap_get_values(ldap_connection, e, "location") != NULL) {
	
	datas [5] = g_strdup (ldap_get_values 
			      (ldap_connection, e, "location") [0]);
	ldap_value_free (ldap_get_values (ldap_connection, e, "location"));
      }
      
      if (ldap_get_values(ldap_connection, e, "comment") != NULL)	{
	
	datas [6] = 
	  g_strdup (ldap_get_values (ldap_connection, e, "comment") [0]);
	ldap_value_free (ldap_get_values (ldap_connection, e, "comment"));
      }

      if (ldap_get_values(ldap_connection, e, "sappid") != NULL)	{

	datas [7]  =
	  g_strdup (ldap_get_values (ldap_connection, e, "sappid") [0]);
	ldap_value_free (ldap_get_values (ldap_connection, e, "sappid"));
      }

      if (ldap_get_values(ldap_connection, e, "ilsa26279966") != NULL)	{
	gchar *value =
	  g_strdup (ldap_get_values (ldap_connection, e, "ilsa26279966") [0]);
	
	int v,a,b,c;
	v = atoi (value); 
	a=v / (16777216);
	b=(v-a*16777216)/65536;
	c=(v-a*16777216-b*65536);
	g_free (value);

	datas [7] = g_strdup_printf ("%s %d.%d.%d", datas [7], a, b, c); 
	ldap_value_free (ldap_get_values (ldap_connection, e, "ilsa26279966"));
      }

      if (ldap_get_values(ldap_connection, e, "rfc822mailbox") != NULL) {
	
	datas [4] = g_strdup (ldap_get_values 
			      (ldap_connection, e, "rfc822mailbox") [0]);
	ldap_value_free (ldap_get_values (ldap_connection, e, "rfc822mailbox"));
      }
      
      if (ldap_get_values(ldap_connection, e, "sipaddress") != NULL) {
	
	nmip = strtoul (ldap_get_values(ldap_connection, e, "sipaddress") [0], 
			NULL, 10);
	ldap_value_free (ldap_get_values (ldap_connection, e, "sipaddress"));
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
      
      /* Check if the window is still present or not */
      if (lw)
	gtk_clist_append (GTK_CLIST (clist), 
			  (gchar **) datas);
          
      /* Video Capable ? */
      if (ldap_get_values(ldap_connection, e, "ilsa32964638") != NULL) {
	
	nmip = atoi (ldap_get_values(ldap_connection, e, "ilsa32964638") [0]);
	ldap_value_free (ldap_get_values (ldap_connection, e, "ilsa32964638"));
      }
      
      if (nmip == 1) {
	
	if (lw)
	  gtk_clist_set_pixmap (GTK_CLIST (clist), 
				GTK_CLIST (clist)->rows - 1, 1, 
				quickcam, quickcam_mask);
      }
      
      /* Audio Capable ? */
      if (ldap_get_values(ldap_connection, e, "ilsa32833566") != NULL) {
	
	nmip = atoi (ldap_get_values(ldap_connection, e, "ilsa32833566") [0]);
	ldap_value_free (ldap_get_values (ldap_connection, e, "ilsa32833566"));
      }
      
      if (nmip == 1) {
	
	if (lw)
	  gtk_clist_set_pixmap (GTK_CLIST (clist), 
				GTK_CLIST (clist)->rows - 1, 0, 
				sound, sound_mask);
      }
      
      for (int j = 2 ; j <= 8 ; j++) {
	
	if (datas [j] != NULL)
	  g_free (datas [j]);
	
	datas [j] = NULL;
      }
    } /* end of for */
    
    gtk_clist_thaw (GTK_CLIST (clist));
  }

  /* Make the progress bar in activity mode go faster */
  gtk_progress_bar_set_activity_step (GTK_PROGRESS_BAR (progress), 5);
  
  ldap_msgfree (res);
  ldap_unbind (ldap_connection);

  lw->thread_count = 0;

  has_to_browse = 0;

  /* Remove the timeout */
  gtk_timeout_remove (ils_timeout);
  gtk_progress_set_activity_mode (GTK_PROGRESS (progress), FALSE);
  gtk_progress_set_value(GTK_PROGRESS(progress), 0);

  // Enable sensitivity
  gtk_widget_set_sensitive (GTK_WIDGET (lw->refresh_button), TRUE);
  gnomemeeting_threads_leave ();
}
