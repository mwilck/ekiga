
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2004 Damien Sandras
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
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : The ILS class thread.
 *
 */

	
#include "../config.h"

#include "ils.h"
#include "gnomemeeting.h"
#include "log_window.h"
#include "misc.h"
#include "gm_conf.h"
#include "stock-icons.h"
#include "dialog.h"

#include <ldap.h>

#include "../pixmaps/inlines.h"


/* Declarations */
extern GtkWidget *gm;


/* XDAP callbacks */
static xmlEntityPtr (*oldgetent) (void *, const xmlChar *);
static xmlEntityPtr xdap_getentity (void *, const xmlChar *);



/* The methods */
GMILSClient::GMILSClient ()
{
  gw = GnomeMeeting::Process ()->GetMainWindow ();
  lw = GnomeMeeting::Process ()->GetLdapWindow ();

  operation = ILS_NONE;

}


GMILSClient::~GMILSClient ()
{
}


BOOL GMILSClient::CheckFieldsConfig (BOOL registering)
{
  gchar *firstname = NULL;
  gchar *surname = NULL;
  gchar *mail = NULL;
  bool no_error = TRUE;

  gnomemeeting_threads_enter ();
  firstname = gm_conf_get_string (PERSONAL_DATA_KEY "firstname");
  surname = gm_conf_get_string (PERSONAL_DATA_KEY "lastname");
  mail = gm_conf_get_string (PERSONAL_DATA_KEY "mail");
  gnomemeeting_threads_leave ();


  if (registering) {
    
    if ((!firstname || PString (firstname).Trim ().IsEmpty ())
        || (!mail || PString (mail).Trim ().IsEmpty () 
            || PString (mail).Find ("@") == P_MAX_INDEX)) {

      /* No need to display that for unregistering */
      gnomemeeting_threads_enter ();
      gnomemeeting_error_dialog (GTK_WINDOW (gm), _("Invalid parameters"), _("Please provide your first name and e-mail in the Personal Data section in order to be able to register to the users directory."));
      gm_conf_set_bool (LDAP_KEY "enable_registering", FALSE);
      gnomemeeting_threads_leave ();
      
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
  ldap_server = gm_conf_get_string (LDAP_KEY "server");
  gnomemeeting_threads_leave ();


  /* We check that there is an ILS server specified */
  if ((ldap_server == NULL) || (!strcmp (ldap_server, ""))) {

    gnomemeeting_threads_enter ();
    gnomemeeting_error_dialog (GTK_WINDOW (gm), _("Invalid users directory"), _("Operation impossible since there is no users directory specified."));
    gnomemeeting_threads_leave ();

    return FALSE;
  }
  
        
  return TRUE;
}


void GMILSClient::Register ()
{
  ILSOperation (ILS_REGISTER);
}


void GMILSClient::Unregister ()
{
  ILSOperation (ILS_UNREGISTER);
}


void GMILSClient::Modify ()
{
  ILSOperation (ILS_UPDATE);
}


void GMILSClient::ILSOperation (Operation operation)
{
  BOOL registering = TRUE;
  bool no_error = TRUE;
  LDAP *ldap = NULL; 
  xmlDocPtr xp = NULL; 
  xmlNodePtr current; 

  char *host = NULL; 
  char *who = NULL;
  char *cred = NULL;
  gchar *msg = NULL;
  gchar *xml_filename = NULL; 
  gchar *ldap_server = NULL; 

  int port = 389; 
  int rc = 0;

  ber_tag_t method;

  struct timeval time_limit = {10, 0};


  if (operation == ILS_REGISTER) {
    
    xml_filename = GNOMEMEETING_DATADIR "/gnomemeeting/xdap/ils_nm_reg.xml";
    registering = TRUE;
  } else if (operation == ILS_UNREGISTER) {
    
    xml_filename = GNOMEMEETING_DATADIR "/gnomemeeting/xdap/ils_nm_unreg.xml";
    registering = FALSE;
  } else if (operation == ILS_UPDATE) {
    
    xml_filename = GNOMEMEETING_DATADIR "/gnomemeeting/xdap/ils_nm_mod.xml";
    registering = TRUE;
  }


  if (CheckServerConfig ()) {

    gnomemeeting_threads_enter ();
    ldap_server = gm_conf_get_string (LDAP_KEY "server");
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
      gnomemeeting_error_dialog (GTK_WINDOW (gm), _("Bad information"), _("Bad LDAP information from XML file: %s."), pferrtostring (rc));
      gnomemeeting_threads_leave ();

      no_error = FALSE;
    }
    /* must be able to reach ldap server */
    else if (!(ldap = ldap_init (ldap_server, 389))) {
      
      gnomemeeting_threads_enter ();
      gnomemeeting_error_dialog (GTK_WINDOW (gm), _("Cannot contact the users directory"), _("Failed to contact the users directory %s:%d. The directory is probably currently overloaded, please try again later."), ldap_server, "389");
      gnomemeeting_threads_leave ();

      no_error = FALSE;
    }
    /* Timeout */
    else if (ldap_set_option (ldap, LDAP_OPT_NETWORK_TIMEOUT, &time_limit)
	     != LDAP_OPT_SUCCESS) {
     
      gnomemeeting_threads_enter ();
      gnomemeeting_error_dialog (GTK_WINDOW (gm), _("Cannot contact the users directory"), _("Failed to set a time limit on operations."));
      gnomemeeting_threads_leave ();

      no_error = FALSE;  
    }
    /* must be able to bind to ldap server */
    else if ((rc = ldap_bind_s (ldap, who, cred, method))) {
      
      gnomemeeting_threads_enter ();
      gnomemeeting_error_dialog (GTK_WINDOW (gm), _("Cannot contact the users directory"), _("Failed to bind to users directory: %s."), ldap_err2string (rc));
      gnomemeeting_threads_leave ();

      no_error = FALSE;
    }
    /* all successful, now process the xml ldap elements */
    else {

      bool ok = CheckFieldsConfig (registering);
     
      /* If all fields are present, then we continue further,
	 otherwise an error dialog will be displayed */
      if (ok) {

	while (current && no_error) 
	  no_error = no_error && XDAPProcess (ldap, xp, &current);

	xmlFree (host);
	xmlFree (who);
	xmlFree (cred);
	
	ldap_unbind_s (ldap);

	if (operation == ILS_REGISTER || operation == ILS_UPDATE) 
	  msg = g_strdup_printf (_("Updated information on the users directory %s."), ldap_server);

	
	if (operation == ILS_UNREGISTER) 
	  msg = g_strdup_printf (_("Unregistered from the users directory %s."), ldap_server);	

	gnomemeeting_threads_enter ();
	gnomemeeting_log_insert (gw->log_window, msg);
	g_free (msg);
	gnomemeeting_threads_leave ();
      }
    }
  }


  xmlFreeDoc (xp);
#if !HAVE_XMLREGISTERNODEDEFAULT
  xdapfree();
#endif
#if XDAPLEAKCHECK
  xdapleakcheck();
#endif

  if (!no_error) {

    gnomemeeting_threads_enter ();
    msg = g_strdup_printf (_("Error while registering to %s"),
			   ldap_server);
    gnomemeeting_log_insert (gw->log_window, msg);
    gnomemeeting_statusbar_flash (gw->statusbar, msg);
    g_free (msg);
    gnomemeeting_threads_leave ();
  }

  g_free (ldap_server);
}


BOOL
GMILSClient::XDAPProcess (LDAP * ldap, xmlDocPtr xp, xmlNodePtr * curp)
{
  int msgid = 0;
  struct timeval t;
  int rc = 0;
  int mt = 0;
  LDAPMessage *res = NULL;
  int op = 0;
  unsigned int ignoremask = 0;

  /* process the current node and update to next node */
  msgid = ldaprun (ldap, xp, curp, &op, &ignoremask, 1);  /* 0 == async */

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
  }

  return TRUE; /* FIX ME */
}


xmlEntityPtr xdap_getentity (void *ctx, const xmlChar * name)
{
  xmlEntityPtr entity;
  xmlChar *entval;
  xmlChar *encentval;
  xmlParserCtxtPtr ctxt = (xmlParserCtxtPtr) ctx;

  PString pip;
  
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
  gchar *sflags = NULL;
  
  unsigned long int sip = 0;

  gnomemeeting_threads_enter ();

  firstname = gm_conf_get_string (PERSONAL_DATA_KEY "firstname");
  surname = gm_conf_get_string (PERSONAL_DATA_KEY "lastname");

  if (!surname || !strcmp (surname, ""))
    surname = g_strdup ("-");
  tmp = g_strdup_printf ("%.65s", surname);
  g_free (surname);
  surname = tmp;
  
  mail = gm_conf_get_string (PERSONAL_DATA_KEY "mail");
  tmp = g_strdup_printf ("%.65s", mail);
  g_free (mail);
  mail = tmp;

  comment = gm_conf_get_string (PERSONAL_DATA_KEY "comment");
  if (!comment || !strcmp (comment, ""))
    comment = g_strdup ("-");
  tmp = g_strdup_printf ("%.65s", comment);
  g_free (comment);
  comment = tmp;

  location = gm_conf_get_string (PERSONAL_DATA_KEY "location");
  if (!location || !strcmp (location, ""))
    location = g_strdup ("-");
  tmp = g_strdup_printf ("%.65s", location);
  g_free (location);
  location = tmp;

  port = g_strdup_printf ("%d", gm_conf_get_int (PORTS_KEY "listen_port"));

  version =  g_strdup_printf ("%u", MAJOR_VERSION << 24 | 
			            MINOR_VERSION << 16 |
			            BUILD_NUMBER);

  if ((GnomeMeeting::Process ()->Endpoint ()->GetCallingState () != GMH323EndPoint::Standby)
      || (gm_conf_get_int (CALL_OPTIONS_KEY "incoming_call_mode") == BUSY))
    busy = g_strdup ("1");
  else
    busy = g_strdup ("0");

  if (gm_conf_get_bool (LDAP_KEY "show_details"))
    sflags = g_strdup ("1");
  else
    sflags = g_strdup ("0");
  
  if (gm_conf_get_bool (VIDEO_CODECS_KEY "enable_video_transmission"))
    ilsa32964638 = g_strdup ("1");
  else
    ilsa32964638 = g_strdup ("0");
  gnomemeeting_threads_leave ();

  pip = GnomeMeeting::Process ()->Endpoint ()->GetCurrentIP ();
  sip = inet_addr ((const char *) pip);
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
  else if (!strcmp ((char *) name, "sflags"))
    entval = xmlStrdup (BAD_CAST sflags);
  else if (!strcmp ((char *) name, "sappid"))
    entval = xmlStrdup (BAD_CAST "GnomeMeeting");
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
  g_free (sflags);
  
  return entity;
}
