
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
 *                         ils.cpp  -  description
 *                         -----------------------
 *   begin                : Sun Sep 23 2001
 *   copyright            : (C) 2000-2003 by Damien Sandras
 *   description          : The ILS class thread.
 *
 */

	
#include "../config.h"

#include "ils.h"
#include "gnomemeeting.h"
#include "ldap_window.h"
#include "misc.h"
#include "stock-icons.h"
#include "dialog.h"

#include <ldap.h>

#include "../pixmaps/inlines.h"


/* Declarations */
extern GnomeMeeting *MyApp;
extern GtkWidget *gm;

static void process (LDAP *, xmlDocPtr, xmlNodePtr *);
static xmlEntityPtr (*oldgetent) (void *, const xmlChar *);
static xmlEntityPtr xdap_getentity (void *, const xmlChar *);


void
process (LDAP * ldap, xmlDocPtr xp, xmlNodePtr * curp)
{
  int msgid;
  struct timeval t;
  int rc;
  int mt;
  LDAPMessage *res;
  int op;
  unsigned int ignoremask;

  /* process the current node and update to next node */
  msgid = ldaprun (ldap, xp, curp, &op, &ignoremask, 0);  /* 0 == async */

  if (msgid > 0) {
    do {

      t.tv_sec = 10;
      t.tv_usec = 0;

      rc = mt = ldap_result (ldap, msgid, 1, &t, &res);
      /* returns -1 == fail, 0 == timeout, > 0 == msg */


      /*
       * This is a bit complicated. It might appear there is
       * no error from ldap_result, but there still might be
       * an error from ldap_result2error()
       * So need to handle all error cases
       */

      if (rc > 0) {
	rc = ldap_result2error (ldap, res, 0);

	if (((ignoremask & IGN_NO) &&
	     (rc == LDAP_NO_SUCH_OBJECT)) ||
	    ((ignoremask & IGN_NA)
	     && (rc == LDAP_INSUFFICIENT_ACCESS)) ||
	    ((ignoremask & IGN_NL) &&
	     (rc == LDAP_NOT_ALLOWED_ON_NONLEAF)) ||
	    ((ignoremask & IGN_EX) &&
	     (rc == LDAP_ALREADY_EXISTS)))
	  rc = 0;

	if (!rc || (rc == LDAP_NO_RESULTS_RETURNED)) {

	  ldap_msgfree (res);

	} else if (rc == 0)

	  ldap_abandon (ldap, msgid);
      }
    } while (mt == LDAP_RES_SEARCH_ENTRY);
    
  } else if (msgid > 0) {
    
    gnomemeeting_threads_enter ();
    gnomemeeting_error_dialog (GTK_WINDOW (gm), _("Parse error"), _("There was an error while processing the registering to the user directory: %d %s."), msgid, pferrtostring (msgid));
    gnomemeeting_threads_leave ();
  }
}
 

/* The methods */
GMILSClient::GMILSClient ()
  :PThread (1000, NoAutoDeleteThread)
{
  gw = MyApp->GetMainWindow ();
  lw = MyApp->GetLdapWindow ();

  running = 1;
  has_to_register = 0;
  has_to_unregister = 0;
  has_to_modify = 0;
  registered = 0;

  client = gconf_client_get_default ();

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

  while (running == 1) {
  
    /* The most important operation is to unregister */
    if (has_to_unregister == 1) {
     
      if (registered)
	Register (0);
    }

    if (has_to_register == 1) {

      if (!registered)
	Register (1);
    }

    if (has_to_modify == 1) {

      if (registered)
	Register (2);
    }


    PTimeInterval t = PTime () - starttime;

    /* if there is more than 20 minutes that we are registered,
       we refresh the entry */
    gnomemeeting_threads_enter ();
    if ((t.GetSeconds () > 1200) && 
	(gconf_client_get_bool (GCONF_CLIENT (client), 
				"/apps/gnomemeeting/ldap/register", 
				NULL))) {

	has_to_unregister = 1;
	has_to_register = 1;
	starttime = PTime ();
      }
    gnomemeeting_threads_leave ();

    Current ()->Sleep (100);
  }

  quit_mutex.Signal ();
}


BOOL GMILSClient::CheckFieldsConfig ()
{
  gchar *firstname = NULL;
  gchar *surname = NULL;
  gchar *mail = NULL;
  bool registering = TRUE;
  bool no_error = TRUE;

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
  
  registering =
    gconf_client_get_bool (GCONF_CLIENT (client),
			   "/apps/gnomemeeting/ldap/register", 
			   NULL);

  if (registering) {

    if ((firstname == NULL) || (!strcmp (firstname, ""))
	|| (mail == NULL) || (!strcmp (mail, ""))) {
      
      /* No need to display that for unregistering */
      gnomemeeting_error_dialog (GTK_WINDOW (gm), _("Invalid parameters"), _("Please provide your first name and e-mail in the Personal Data section in order to be able to register to the user directory."));
      
      no_error = FALSE;
    }
  }
  else
    if ((mail == NULL) || (!strcmp (mail, "")))
      no_error = FALSE;


  g_free (firstname);
  g_free (surname);
  g_free (mail);
      
  return no_error;
}


BOOL GMILSClient::CheckServerConfig ()
{
  gchar *ldap_server = NULL;


  gnomemeeting_threads_enter ();
  ldap_server =  
    gconf_client_get_string (GCONF_CLIENT (client),
			     "/apps/gnomemeeting/ldap/ldap_server", 
			     NULL);
  gnomemeeting_threads_leave ();


  /* We check that there is an ILS server specified */
  if ((ldap_server == NULL) || (!strcmp (ldap_server, ""))) {

    gnomemeeting_threads_enter ();
    gnomemeeting_error_dialog (GTK_WINDOW (gm), _("Invalid user directory"), _("Operation impossible since there is no user directory specified."));
    gnomemeeting_threads_leave ();

    return FALSE;
  }
  
        
  return TRUE;
}


void GMILSClient::Register ()
{
  has_to_register = 1;
}


void GMILSClient::Unregister ()
{
  has_to_unregister = 1;
}


void GMILSClient::Modify ()
{
  has_to_modify = 1;
}


BOOL GMILSClient::Register (int reg)
{
  bool no_error = TRUE;
  LDAP *ldap = NULL; /* For the LDAP connection */
  xmlDocPtr xp = NULL; /* Pointer to the XML Document */
  xmlNodePtr current; /* Pointer to a node */
  gchar *ldap_server = NULL; /* ldap server */
  char *host = NULL; /* Room for the default host */
  gchar * xml_filename = NULL; /* Filename of template */
  int port; 
  char *who;
  char *cred;
  ber_tag_t method;
  int rc;
  struct timeval time_limit;
  gchar *msg = NULL;
  
  time_limit.tv_sec = 10;
  time_limit.tv_usec = 0;


  if (reg == 1)
    xml_filename = DATADIR "/gnomemeeting/xdap/ils_nm_reg.xml";
  if (reg == 0)
    xml_filename = DATADIR "/gnomemeeting/xdap/ils_nm_unreg.xml";
  if (reg == 2)
    xml_filename = DATADIR "/gnomemeeting/xdap/ils_nm_mod.xml";


  if (CheckServerConfig ()) {

    gnomemeeting_threads_enter ();
    ldap_server =  
      gconf_client_get_string (GCONF_CLIENT (client),
			       "/apps/gnomemeeting/ldap/ldap_server", 
			       NULL);

    msg = g_strdup_printf (_("Contacting %s..."), ldap_server);

    if (reg != 2)
      gnomemeeting_statusbar_flash (gw->statusbar, msg);
    gnomemeeting_log_insert (gw->history_text_view, msg);    
    g_free (msg);
    gnomemeeting_threads_leave ();

    /* xml file must parse */
    if (!(xp = parseonly (xml_filename,
			  xdap_getentity, &oldgetent, 1))) {
      
      gnomemeeting_threads_enter ();
      gnomemeeting_error_dialog (GTK_WINDOW (gm), _("Failed to parse XML file"), _("There was an error while parsing the XML file. Please make sure that it is correctly installed in your system."));
      gnomemeeting_threads_leave ();

      no_error = FALSE;
    }
    /* xml file or entities must list ldap server info */
    /* this also updates a pointer to the first node to process */
    else if ((rc = getldapinfo (xp, &current, &host, &port, &who,
				&cred, &method))) {

      gnomemeeting_threads_enter ();      
      gnomemeeting_error_dialog (GTK_WINDOW (gm), _("Bad information"), _("Bad ldap information from XML file: %s."), pferrtostring (rc));
      gnomemeeting_threads_leave ();

      no_error = FALSE;
    }
    /* must be able to reach ldap server */
    else if (!(ldap = ldap_init (ldap_server, 389))) {
      
      gnomemeeting_threads_enter ();
      gnomemeeting_error_dialog (GTK_WINDOW (gm), _("Cannot contact the user directory"), _("Failed to contact the user directory %s:%d."), ldap_server, "389");
      gnomemeeting_threads_leave ();

      no_error = FALSE;
    }
    /* Timeout */
    else if (ldap_set_option (ldap, LDAP_OPT_NETWORK_TIMEOUT, &time_limit)
	     != LDAP_OPT_SUCCESS) {
     
      gnomemeeting_threads_enter ();
      gnomemeeting_error_dialog (GTK_WINDOW (gm), _("Cannot contact the user directory"), _("Failed to set a time limit on operations."));
      gnomemeeting_threads_leave ();

      no_error = FALSE;  
    }
    /* must be able to bind to ldap server */
    else if ((rc = ldap_bind_s (ldap, who, cred, method))) {
      
      gnomemeeting_threads_enter ();
      gnomemeeting_error_dialog (GTK_WINDOW (gm), _("Cannot contact the user directory"), _("Failed to bind to user directory: %s."), ldap_err2string (rc));
      gnomemeeting_threads_leave ();

      no_error = FALSE;
    }
    /* all successful, now process the xml ldap elements */
    else {

      gnomemeeting_threads_enter ();
      bool ok = CheckFieldsConfig ();
      gnomemeeting_threads_leave ();
      
      if (ok) {
	while (current) {

	  process (ldap, xp, &current);
	}
      }
      else
	no_error = FALSE; /* There are missing fields */

      xmlFree (host);
      xmlFree (who);
      xmlFree (cred);

      ldap_unbind_s (ldap);

      if ((reg == 1) || (reg == 2)) {

	if (reg == 1) {

	  msg = g_strdup_printf (_("Successfully registered to %s."), 
				 ldap_server);

	  /* if we registered to ILS, let's update the IP from ILS 
	     to the gateway IP of the translation */
	  
	  gchar *ip = NULL;
	  gnomemeeting_threads_enter ();
	  gchar *mail =  
	    gconf_client_get_string (GCONF_CLIENT (client), 
				     "/apps/gnomemeeting/personal_data/mail", 
				     NULL);
	  gnomemeeting_threads_leave ();

	  if ((mail)&&(strcmp (mail, ""))
	      &&(!strcmp (ldap_server, "ils.seconix.com")))
	    ip = Search (ldap_server, "389", mail);

	  if (ip) {

	    PString IP = PString (ip);
	    PINDEX prt = IP.Find (':');
	    
	    if (prt != P_MAX_INDEX)
	      IP = IP.Left (prt);
	    
	    gnomemeeting_threads_enter ();
	    if (PString ("0.0.0.0") != IP)
	      gconf_client_set_string (client, "/apps/gnomemeeting/general/public_ip", (const char *) IP, NULL);
	    gnomemeeting_threads_leave ();

	    g_free (ip);
	    g_free (mail);
	  }
	}
	else
	  msg = g_strdup_printf (_("Updated information on %s."), 
				 ldap_server);
	
	has_to_register = 0;
	has_to_modify = 0;
	registered = 1;
      }

      if (reg == 0) {

	msg = g_strdup_printf (_("Successfully unregistered from %s."),
			       ldap_server);	
	has_to_unregister = 0;
	registered = 0;
      }

      gnomemeeting_threads_enter ();
      
      if (reg != 2)
	gnomemeeting_statusbar_flash (gw->statusbar, msg);
      gnomemeeting_log_insert (gw->history_text_view, msg);
      g_free (msg);

      gnomemeeting_threads_leave ();
    }
  }
  else
    no_error = FALSE; /* CheckServerConfig failed */


  xmlFreeDoc (xp);
#if !HAVE_XMLREGISTERNODEDEFAULT
  xdapfree();
#endif
#if XDAPLEAKCHECK
  xdapleakcheck();
#endif

  if (!no_error) {

    gnomemeeting_threads_enter ();
    if ((reg == 1) || (reg == 2))
      msg = g_strdup_printf (_("Error while registering to %s."),
			     ldap_server);
    if (reg == 0)
      msg = g_strdup_printf (_("Error while unregistering from %s."),
			     ldap_server);
    
    gnomemeeting_log_insert (gw->history_text_view, msg);
    gnomemeeting_statusbar_flash (gw->statusbar, msg);
    g_free (msg);

    has_to_register = 0;
    has_to_unregister = 0;
    has_to_modify = 0;
    gnomemeeting_threads_leave ();
  }

  g_free (ldap_server);


  return no_error;
}


gchar *GMILSClient::Search (gchar *ldap_server, gchar *ldap_port, gchar *mail)
{
  LDAP *ldap_search_connection = NULL;
  int rc_search_connection = -1;

  char *attrs [] = { "rfc822mailbox", "sappid", "sipaddress", "sport", NULL };
  char **ldv = NULL;
 
  unsigned long int nmip = 0;
  int part1;
  int part2;
  int part3;
  int part4;
  int port = 1720;
  struct timeval t = {10, 0};
  struct timeval t2 = {7, 0};

  bool no_error = TRUE;
  gchar *ip = NULL;
  gchar *cn = NULL;
  gchar *app = NULL;

  LDAPMessage *res = NULL, *e = NULL;

  if (((!strcmp (ldap_server, ""))&&(ldap_server)) 
      ||((!strcmp (ldap_port, ""))&&(ldap_port)) 
      ||((!strcmp (mail, ""))&&(mail)))
    return NULL;
 

  /* must be able to reach ldap server */
  if (!(ldap_search_connection = ldap_init (ldap_server, atoi (ldap_port)))) {
      
    no_error = FALSE;
  }
  /* Timeout */
  else if (ldap_set_option (ldap_search_connection, LDAP_OPT_NETWORK_TIMEOUT, 
			    &t)
	   != LDAP_OPT_SUCCESS) {
     
    no_error = FALSE;  
  }
  /* must be able to bind to ldap server */
  else if ((rc_search_connection 
	    = ldap_bind_s (ldap_search_connection, NULL, NULL, 
			   LDAP_AUTH_SIMPLE))) {
        
    no_error = FALSE;
  }
  /* all successful */
  else {
    
    cn = g_strdup_printf ("(&(cn=%s))", mail);
    rc_search_connection = 
      ldap_search_st (ldap_search_connection, "objectClass=RTPerson", 
		      LDAP_SCOPE_BASE,
		      cn, attrs, 0, &t2, &res); 
    g_free (cn);
    
    if (rc_search_connection != 0)
      return ip;
    
    /* We only take the first entry */
    e = ldap_first_entry (ldap_search_connection, res); 
    if (e) {
      
      ldv = ldap_get_values (ldap_search_connection, e, "sipaddress");
      if ((ldv != NULL)&&(ldv [0] != NULL)) {
	
	nmip = strtoul (ldv [0], NULL, 10);
	ldap_value_free (ldv);
      }


      ldv = ldap_get_values (ldap_search_connection, e, "sappid");
      if ((ldv != NULL)&&(ldv [0] != NULL)) {
	
	app = g_strdup (ldv [0]);
	ldap_value_free (ldv);
      }


      ldv = ldap_get_values (ldap_search_connection, e, "sport");
      if ((ldv != NULL)&&(ldv [0] != NULL)) {
	
	port = atoi (ldv [0]);
	
	/* Ugly hack because Netmeeting sucks, it registers
	   random ports to ILS, but it only supports 1720 */
	if (app&&!strcmp (app, "ms-netmeeting")&&port == 1503) {

	  port = 1720;
	  g_free (app);
	}

	ldap_value_free (ldv);
      }

    }

    part1 = (nmip & 0xff000000) >> 24;
    part2 = (nmip & 0x00ff0000) >> 16;
    part3 = (nmip & 0x0000ff00) >> 8;
    part4 = nmip & 0x000000ff;
    
    ip = g_strdup_printf ("%d.%d.%d.%d:%d", part4, part3, part2, part1, port);

    ldap_msgfree (res);
    ldap_unbind (ldap_search_connection);
  }


  rc_search_connection = -1;
  ldap_search_connection = NULL;
  
  return ip;
}


xmlEntityPtr xdap_getentity (void *ctx, const xmlChar * name)
{
  xmlEntityPtr entity;
  xmlChar *entval;
  xmlChar *encentval;
  xmlParserCtxtPtr ctxt = (xmlParserCtxtPtr) ctx;

  gchar *firstname = NULL;
  gchar *surname = NULL;
  gchar *mail = NULL;
  gchar *comment = NULL;
  gchar *location = NULL;
  gchar *tmp = NULL;
  gchar *version = NULL;
  gchar *busy = NULL;
  gchar *ip = NULL;
  gchar *port = NULL;
  gchar *ilsa32964638 = NULL;
  gchar *ilsa39321630 = NULL;
  
  unsigned long int sip = 0;

  gnomemeeting_threads_enter ();
  GConfClient *client = gconf_client_get_default ();


  firstname = 
    gconf_client_get_string (GCONF_CLIENT (client),
			     "/apps/gnomemeeting/personal_data/firstname", 
			     NULL);

  surname =  
    gconf_client_get_string (GCONF_CLIENT (client),
			     "/apps/gnomemeeting/personal_data/lastname", 
			     NULL);
  if (!surname || !strcmp (surname, ""))
    surname = g_strdup ("-");
  tmp = g_strdup_printf ("%.65s", surname);
  g_free (surname);
  surname = tmp;
  
  mail =  
    gconf_client_get_string (GCONF_CLIENT (client),
			     "/apps/gnomemeeting/personal_data/mail", 
			     NULL);
  tmp = g_strdup_printf ("%.65s", mail);
  g_free (mail);
  mail = tmp;

  comment =  
    gconf_client_get_string (GCONF_CLIENT (client),
			     "/apps/gnomemeeting/personal_data/comment", 
			     NULL);
  if (!comment || !strcmp (comment, ""))
    comment = g_strdup ("-");
  tmp = g_strdup_printf ("%.65s", comment);
  g_free (comment);
  comment = tmp;

  location =  
    gconf_client_get_string (GCONF_CLIENT (client),
			     "/apps/gnomemeeting/personal_data/location", 
			     NULL);
  if (!location || !strcmp (location, ""))
    location = g_strdup ("-");
  tmp = g_strdup_printf ("%.65s", location);
  g_free (location);
  location = tmp;

  port = 
    g_strdup_printf ("%d", 
		     gconf_client_get_int (GCONF_CLIENT (client),
					   PORTS_KEY "listen_port",
					   NULL));

  version =  g_strdup_printf ("%u", MAJOR_VERSION << 24 | 
			            MINOR_VERSION << 16 |
			            BUILD_NUMBER);

  if ((MyApp->Endpoint ()->GetCallingState () != 0)
      || (gconf_client_get_bool (client, 
				 GENERAL_KEY "do_not_disturb", 
				 NULL)))
    busy = g_strdup ("1");
  else
    busy = g_strdup ("0");

  if (gconf_client_get_bool (client, LDAP_KEY "visible", NULL))
    ilsa39321630 = g_strdup ("1");
  else
    ilsa39321630 = g_strdup ("8");
  
  if (gconf_client_get_bool (client, "/apps/gnomemeeting/video_settings/enable_video_transmission", NULL))
    ilsa32964638 = g_strdup ("1");
  else
    ilsa32964638 = g_strdup ("0");
  gnomemeeting_threads_leave ();

  ip = MyApp->Endpoint ()->GetCurrentIP ();
  sip = inet_addr (ip);
  g_free (ip);
  ip = g_strdup_printf ("%lu", sip);

  if (!strcmp ((char *) name, "comment"))
    entval = xmlStrdup (BAD_CAST comment);
  else if (!strcmp ((char *) name, "location"))
    entval = xmlStrdup (BAD_CAST location);
  else if (!strcmp ((char *) name, "org"))
    entval = xmlStrdup (BAD_CAST "Gnome");
  else if (!strcmp ((char *) name, "country"))
    entval = xmlStrdup (BAD_CAST "-");
  else if (!strcmp ((char *) name, "class"))
    entval = xmlStrdup (BAD_CAST "RTPerson");
  else if (!strcmp ((char *) name, "host"))
    entval = xmlStrdup (BAD_CAST "ils.seconix.com");
  else if (!strcmp ((char *) name, "ilsa26214430"))
    entval = xmlStrdup (BAD_CAST busy);
 else if (!strcmp ((char *) name, "ilsa32964638"))
    entval = xmlStrdup (BAD_CAST ilsa32964638);
  else if (!strcmp ((char *) name, "port"))
    entval = xmlStrdup (BAD_CAST port);
  else if (!strcmp ((char *) name, "decip"))
    entval = xmlStrdup (BAD_CAST ip);
  else if (!strcmp ((char *) name, "email"))
    entval = xmlStrdup (BAD_CAST mail);
  else if (!strcmp ((char *) name, "givenname"))
    entval = xmlStrdup (BAD_CAST firstname);
  else if (!strcmp ((char *) name, "surname"))
    entval = xmlStrdup (BAD_CAST surname);
  else if (!strcmp ((char *) name, "sappid"))
    entval = xmlStrdup (BAD_CAST "GnomeMeeting");
  else if (!strcmp ((char *) name, "ilsa39321630"))
      entval = xmlStrdup (BAD_CAST ilsa39321630);
  else if (!strcmp ((char *) name, "ilsa26279966"))
    entval = xmlStrdup (BAD_CAST version);
  else
    return 0;

  encentval = xmlEncodeSpecialChars(((xmlParserCtxtPtr) ctx)->myDoc, entval);

  if (!ctxt->myDoc->intSubset) {
    (void) xmlCreateIntSubset (ctxt->myDoc, BAD_CAST "ils",
			       BAD_CAST "SYSTEM",
			       BAD_CAST "ils.dtd");
  } else {
    D (D_TRACE, fprintf (stderr, "iss %s\n",
			 ctxt->myDoc->intSubset->name));
  }


  entity = xmlGetDocEntity(((xmlParserCtxtPtr) ctx)->myDoc, name);
  if (entity) {
    if (strcmp((char *)entity->content, (char *)encentval))
      fprintf(stderr,
	      "Warning: New entity value will be ignored %s %s %s\n",
	      name, encentval, entity->content);
  } else
    entity = xmlAddDocEntity (((xmlParserCtxtPtr) ctx)->myDoc,
			      name, XML_INTERNAL_GENERAL_ENTITY, 0, 0, 
			      encentval);

  xmlFree(entval);
  xmlFree(encentval);

  g_free (firstname);
  g_free (surname);
  g_free (mail);
  g_free (comment);
  g_free (location);
  g_free (version);
  g_free (busy);
  g_free (ip);
  g_free (port);
  g_free (ilsa32964638);
  g_free (ilsa39321630);
  
  return entity;
}


/* The browser methods */

GMILSBrowser::GMILSBrowser (GmLdapWindowPage *lwpage,
			    gchar *server,
			    gchar *filter)
  :PThread (1000, AutoDeleteThread)
{
  lw = MyApp->GetLdapWindow ();
  lwp = lwpage;
  
  if (server)
    ldap_server = g_strdup (server);
  else
    ldap_server = NULL;

  if (filter)
    search_filter = g_convert (filter, strlen (filter),
			       "ISO-8859-1", "UTF-8",
			       NULL, NULL, NULL);
  else
    search_filter = NULL;

  this->Resume ();
}


GMILSBrowser::~GMILSBrowser ()
{
  if (ldap_server) 
    g_free (ldap_server);

  if (search_filter)
    g_free (search_filter);

  if (lwp) {

    lwp->ils_browser = NULL;
    lwp->search_quit_mutex.Signal ();
  }
}


void GMILSBrowser::Main ()
{
  char *attrs [] = { "surname", "givenname", "comment", "location", 
		     "rfc822mailbox", "sipaddress", "ilsa32833566", 
		     "ilsa32964638", "ilsa26279966", "ilsa26214430", 
		     "sport", "sappid", "xstatus", NULL};

  char **char_ldap_data [] = {NULL, NULL, NULL, NULL, NULL, NULL};
  char **bool_ldap_data [] = {NULL, NULL, NULL};
  bool b_ldap_data [] = {false, false, false};
  char * utf8_char_ldap_data [] = {NULL, NULL, NULL, NULL, NULL, NULL};

  gchar **num_users = NULL;
  char **sipaddress = NULL;
  char **ilsa26279966 = NULL;
  char **sport = NULL;
  char **xstatus = NULL;
  gchar *utf8_username = NULL;
  gchar *utf8_remote_app = NULL;
  gchar *utf8_callto = NULL;
    
  int rc = -1;
  LDAPMessage *res = NULL, *e = NULL;

  unsigned int v = 0, a = 0, b = 0, c = 0;
  unsigned int part1 = 0, part2 = 0, part3 = 0, part4 = 0;
  unsigned long int nmip = 0;
  int port = 1720;
  int users_nbr = 0;
  int retry = 0;

  gchar *color = NULL;
  gchar *filter = NULL;
  gchar *ip = NULL;

  LDAP *ldap_connection = NULL;
  GdkPixbuf *status_icon = NULL;
  GtkListStore *users_list_store = NULL;
  GtkTreeIter list_iter;

  GmWindow *gw = NULL;

  struct timeval time_limit;
  time_limit.tv_sec = 10;
  time_limit.tv_usec = 0;
  bool no_error = TRUE;

  if (!lwp)
    return;

  lwp->search_quit_mutex.Wait ();

  gnomemeeting_threads_enter ();
  gw = MyApp->GetMainWindow ();  
  users_list_store =
    GTK_LIST_STORE (gtk_tree_view_get_model (GTK_TREE_VIEW (lwp->tree_view)));
  gnomemeeting_threads_leave ();

  do {

    gnomemeeting_threads_enter ();
    gnomemeeting_statusbar_push (lwp->statusbar,
				 _("Contacting %s..."), ldap_server);
    gnomemeeting_threads_leave ();

    /* must be able to reach ldap server */
    if (!(ldap_connection = ldap_init (ldap_server, 389))) {
      
      gnomemeeting_threads_enter ();
      gnomemeeting_statusbar_push (lwp->statusbar,
				   _("Failed to contact LDAP server %s:%d."),
				   ldap_server, "389");
      gnomemeeting_threads_leave ();
      
      no_error = FALSE;
    }
    /* Timeout */
    else if (ldap_set_option (ldap_connection, LDAP_OPT_NETWORK_TIMEOUT, 
			      &time_limit)
	     != LDAP_OPT_SUCCESS) {
     
      gnomemeeting_threads_enter ();
      gnomemeeting_statusbar_push (lwp->statusbar,
				   _("Failed to set time limit on LDAP operations."));
      gnomemeeting_threads_leave ();
    
      no_error = FALSE;  
    }
    /* must be able to bind to ldap server */
    else if ((rc = ldap_bind_s (ldap_connection, NULL, NULL, 
				LDAP_AUTH_SIMPLE))) {
      
      gnomemeeting_threads_enter ();
      gnomemeeting_statusbar_push (lwp->statusbar, _("Failed to contact to LDAP server %s: %s."), ldap_server, ldap_err2string (rc));
      gnomemeeting_threads_leave ();
    
      no_error = FALSE;
    }
    /* all successful */
    else {
    
      gnomemeeting_threads_enter ();        
      gnomemeeting_statusbar_push (lwp->statusbar,
				   _("Fetching online users list from %s."),
				   ldap_server);
      gnomemeeting_threads_leave ();
      
      if (search_filter)
	filter = g_strdup_printf ("(&(cn=%%)%s)", search_filter);
      else
	filter = g_strdup ("(&(cn=%))");

      rc =
	ldap_search_s (ldap_connection, "objectClass=RTPerson", 
		       LDAP_SCOPE_BASE,
		       filter, attrs, 0, &res); 
      g_free (filter);

      gnomemeeting_threads_enter ();
      if (rc != 0)
	if (rc == LDAP_SERVER_DOWN) 
	  gnomemeeting_statusbar_push (lwp->statusbar,
				       _("Connection to %s lost..."),
				       ldap_server);
	else
	  gnomemeeting_statusbar_push (lwp->statusbar, _("Could not fetch online users list."));
      gnomemeeting_threads_leave ();
  
      retry++;
    }
  } while ((rc == LDAP_SERVER_DOWN) && (retry <= 5));


  if (rc == 0 && users_list_store != NULL && res) { 

    gnomemeeting_threads_enter ();
    gtk_tree_view_set_model (GTK_TREE_VIEW (lwp->tree_view), NULL);
    gtk_list_store_clear (GTK_LIST_STORE (users_list_store));
    gnomemeeting_threads_leave ();
	
    for (e = ldap_first_entry (ldap_connection, res); 
	 e != NULL;
	 e = ldap_next_entry (ldap_connection, e)) {
	
      users_nbr++;

      char_ldap_data [0] = ldap_get_values(ldap_connection, e, "givenname");
      char_ldap_data [1] = ldap_get_values (ldap_connection, e, "surname");
      char_ldap_data [2] = ldap_get_values(ldap_connection, e, "location");
      char_ldap_data [3] = ldap_get_values(ldap_connection, e, "comment");
      char_ldap_data [4] = ldap_get_values(ldap_connection, e, "sappid");
      char_ldap_data [5] =
	ldap_get_values(ldap_connection, e, "rfc822mailbox");
      bool_ldap_data [0] =
	ldap_get_values(ldap_connection, e, "ilsa32964638"); /* video */
      bool_ldap_data [1] =
	ldap_get_values(ldap_connection, e, "ilsa32833566"); /* audio */
      bool_ldap_data [2] =
	ldap_get_values(ldap_connection, e, "ilsa26214430"); /* available */

      sport = ldap_get_values(ldap_connection, e, "sport");
      xstatus = ldap_get_values(ldap_connection, e, "xstatus");
      ilsa26279966 = ldap_get_values(ldap_connection, e, "ilsa26279966");
      sipaddress = ldap_get_values(ldap_connection, e, "sipaddress");


      /* Free it here because we need to keep the last value to use it
	 after the for */
      if (num_users)
	g_strfreev (num_users);
	

      /* char LDAP data, to be converted to UTF-8 if not valid */
      for (int i = 0 ; i < 6 ; i++) {
	  
	if (char_ldap_data [i] && char_ldap_data [i] [0]) {
	  
	  if (!g_utf8_validate (char_ldap_data [i] [0], -1, NULL))
	    utf8_char_ldap_data [i] =
	      gnomemeeting_from_iso88591_to_utf8 (PString (char_ldap_data [i] [0]));
	  else
	    utf8_char_ldap_data [i] =
	      g_strdup (char_ldap_data [i] [0]);
	    
	  ldap_value_free (char_ldap_data [i]);
	  char_ldap_data [i] = NULL;
	}
      }


      /* bool LDAP data */
      for (int i = 0 ; i < 3 ; i++) {

	if (bool_ldap_data [i] && bool_ldap_data [i] [0]) {
	    
	  b_ldap_data [i] = (bool) atoi (bool_ldap_data [i] [0]);
	  ldap_value_free (bool_ldap_data [i]);
	  bool_ldap_data [i] = NULL;
	}
      }


      /* Number of users: seconix.com specific */
      if (xstatus && xstatus [0]) {

	num_users = g_strsplit (xstatus [0], ",", 0);
	ldap_value_free (xstatus);
	xstatus = NULL;
      }


      /* Remote application port */
      if (sport && sport [0]) {

	port = atoi (sport [0]);

	/* Ignore data port */
	if (port == 1503 && sport [1])
	  port = atoi (sport [1]);
	  
	ldap_value_free (sport);
	sport = NULL;
      }


      /* Remote IP */
      if (sipaddress && sipaddress [0]) {

	nmip = strtoul (sipaddress [0], NULL, 10);
	part1 = (nmip & 0xff000000) >> 24;
	part2 = (nmip & 0x00ff0000) >> 16;
	part3 = (nmip & 0x0000ff00) >> 8;
	part4 = nmip & 0x000000ff;

	ip =
	  g_strdup_printf ("%d.%d.%d.%d:%d",
			   part4, part3, part2, part1, port);

	ldap_value_free (sipaddress);
	sipaddress = NULL;
      }


      /* Remote application name and version */
      if (ilsa26279966 && ilsa26279966 [0]) {
	  
	v = atoi (ilsa26279966 [0]);
	a = (v & 0xff000000) >> 24;
	b = (v & 0x00ff0000) >> 16;
	c = v & 0x0000ffff;

	utf8_remote_app =
	  g_strdup_printf ("%s %d.%d.%d",
			   utf8_char_ldap_data [4] ?
			   utf8_char_ldap_data [4] : "",
			   a, b, c); 

	ldap_value_free (ilsa26279966);
	ilsa26279966 = NULL;
      }
	

      /* Status icon */
      gnomemeeting_threads_enter ();
      if (!b_ldap_data [2]) {

	status_icon =
	  gtk_widget_render_icon (lwp->tree_view, 
				  GM_STOCK_STATUS_AVAILABLE,
				  GTK_ICON_SIZE_MENU, NULL);
	color = g_strdup ("black");
      } 
      else {
	 
	status_icon =
	  gtk_widget_render_icon (lwp->tree_view, 
				  GM_STOCK_STATUS_OCCUPIED,
				  GTK_ICON_SIZE_MENU, NULL);
	color = g_strdup ("#8a8a8a");
      }        
      gnomemeeting_threads_leave ();

      utf8_username =
	g_strdup_printf ("%s %s",
			 utf8_char_ldap_data [0] ?
			 utf8_char_ldap_data [0] : "",
			 utf8_char_ldap_data [1] ?
			 utf8_char_ldap_data [1] : "");
      utf8_callto =
	g_strdup_printf ("callto:%s/%s",
			 ldap_server,
			 utf8_char_ldap_data [5] ?
			 utf8_char_ldap_data [5] : "");

      gnomemeeting_threads_enter ();
      gtk_list_store_append (users_list_store, &list_iter);
      gtk_list_store_set (users_list_store, &list_iter,
			  COLUMN_ILS_STATUS, status_icon,
			  COLUMN_ILS_AUDIO, b_ldap_data [1],
			  COLUMN_ILS_VIDEO, b_ldap_data [0],
			  COLUMN_ILS_NAME, utf8_username,
			  COLUMN_ILS_URL, utf8_callto,
			  COLUMN_ILS_LOCATION, utf8_char_ldap_data [2],
			  COLUMN_ILS_COMMENT, utf8_char_ldap_data [3],
			  COLUMN_ILS_VERSION, utf8_remote_app,
			  COLUMN_ILS_IP, ip ? ip : "",
			  COLUMN_ILS_COLOR, color,
			  -1);
      gnomemeeting_threads_leave ();


      /* Free some memory and put the variables in their initial state */
      for (int i = 0 ; i < 6 ; i++) {
	  
	if (utf8_char_ldap_data [i])
	  free (utf8_char_ldap_data [i]);

	utf8_char_ldap_data [i] = NULL;
      }

      for (int i = 0 ; i < 3 ; i++) {

	b_ldap_data [i] = false;
      }
	
      g_free (ip);
      g_free (color);
      g_free (utf8_username);
      g_free (utf8_remote_app);
      g_free (utf8_callto);
      ip = NULL;
      color = NULL;
      utf8_username = NULL;
      utf8_remote_app = NULL;
      utf8_callto = NULL;

      gnomemeeting_threads_enter ();
      g_object_unref (status_icon);
      gnomemeeting_threads_leave ();
    } /* end of for */

    gnomemeeting_threads_enter ();
    gtk_tree_view_set_model (GTK_TREE_VIEW (lwp->tree_view),
			     GTK_TREE_MODEL (users_list_store));

    if (num_users && num_users [1]) {

      gnomemeeting_statusbar_push (lwp->statusbar, _("Search completed: %d user(s) listed on a total of %d user(s) from %s."), users_nbr, PMAX (atoi (num_users [1]), users_nbr), ldap_server);

      g_strfreev (num_users);
    }
    else
      gnomemeeting_statusbar_push (lwp->statusbar, _("Search completed: %d user(s) found on %s."), users_nbr, ldap_server);

    gnomemeeting_threads_leave ();
  }


  if (no_error) {

    ldap_msgfree (res);
    ldap_unbind (ldap_connection);
  }
}
