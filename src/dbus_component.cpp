
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
 * programs Opal and Pwlib, and distribute the combination, without
 * applying the requirements of the GNU GPL to the Opal program, as long
 * as you do follow the requirements of the GNU GPL for all the rest of the
 * software thus combined.
 */


/*
 *                         dbus_component.cpp  -  description
 *                         -----------------------------
 *   begin                : Tue Nov 1  2005
 *   copyright            : (C) 2005 by Julien Puydt
 *   description          : This files contains the implementation of the DBUS
 *                          interface of gnomemeeting.
 *
 */

#include <dbus/dbus-glib.h>

#include "dbus_component.h"

#include "gnomemeeting.h"
#include "gm_marshallers.h"
#include "gm_conf.h"
#include "misc.h"

/* all signals understood by this component */
enum {
  ACCOUNT_REGISTER,
  STATE_CHANGED,
  NAME_INFO,
  CLIENT_INFO,
  URL_INFO,
  PROTOCOL_INFO,
  LAST_SIGNAL
};

/* Beginning of a classic GObject declaration */

typedef struct DbusComponent DbusComponent;
typedef struct DbusComponentPrivate DbusComponentPrivate;
typedef struct DbusComponentClass DbusComponentClass;

GType dbus_component_get_type (void);

struct DbusComponent
{
  GObject parent;
};

struct DbusComponentPrivate
{
  gboolean owner;
};

struct DbusComponentClass
{
  GObjectClass parent;
};

static guint signals[LAST_SIGNAL] = { 0 };

#define DBUS_COMPONENT_TYPE_OBJECT (dbus_component_get_type ())
#define DBUS_COMPONENT_OBJECT(object) (G_TYPE_CHECK_INSTANCE_CAST ((object), DBUS_COMPONENT_TYPE_OBJECT, DbusComponent))
#define DBUS_COMPONENT_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), DBUS_COMPONENT_TYPE_OBJECT, DbusComponentClass))
#define DBUS_COMPONENT_IS_OBJECT(object) (G_TYPE_CHECK_INSTANCE_TYPE ((object), DBUS_COMPONENT_TYPE_OBJECT))
#define DBUS_COMPONENT_IS_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), DBUS_COMPONENT_TYPE_OBJECT))
#define DBUS_COMPONENT_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), DBUS_COMPONENT_TYPE_OBJECT, DbusComponentPrivate))

G_DEFINE_TYPE(DbusComponent, dbus_component, G_TYPE_OBJECT);

/* End of a classic GObject declaration */

/* declaration of all the methods of this object */
static gboolean dbus_component_get_accounts_list (DbusComponent *self,
						  char ***accounts,
						  GError **error);
static gboolean dbus_component_register (DbusComponent *self,
					 const char *account,
					 GError **error);
static gboolean dbus_component_unregister (DbusComponent *self,
					   const char *account,
					   GError **error);
static gboolean dbus_component_resignal_account_state (DbusComponent *self,
						       const char *account,
						       GError **error);

static gboolean dbus_component_get_calls_list (DbusComponent *self,
					       char ***calls,
					       GError **error);
static gboolean dbus_component_connect (DbusComponent *self,
					const char *url,
					char **token,
					GError **error);
static gboolean dbus_component_disconnect (DbusComponent *self,
					   const char *token,
					   GError **error);
static gboolean dbus_component_play_pause (DbusComponent *self,
					   const char *token,
					   GError **error);
static gboolean dbus_component_transfer (DbusComponent *self,
					 const char *token,
					 const char *url,
					 GError **error);
static gboolean dbus_component_resignal_call_info (DbusComponent *self,
						   const char *token,
						   GError **error);
static gboolean dbus_component_shutdown (DbusComponent *self,
					 GError **error);
static gboolean dbus_component_get_local_address (DbusComponent *self,
						  const char *protocol,
						  char **url,
						  GError **error);
static gboolean dbus_component_get_name (DbusComponent *self,
					 char **name,
					 GError **error);
static gboolean dbus_component_get_location (DbusComponent *self,
					     char **location,
					     GError **error);
static gboolean dbus_component_get_comment (DbusComponent *self,
					    char **comment,
					    GError **error);

static gboolean dbus_component_claim_ownership (DbusComponent *self);

/* get the code to make the GObject accessible through dbus
 * (this is especially where we get dbus_glib_dbus_component_object_info !)
 */
#include "dbus_component_stub.h"

/* implementation of the GObject's methods */

static void
dbus_component_init (DbusComponent *self)
{
  /* nothing to do */
}

static void
dbus_component_class_init (DbusComponentClass *klass)
{
  /* register our private structure */
  g_type_class_add_private (klass, sizeof (DbusComponentPrivate));

  /* creation of all the signals */
  signals[ACCOUNT_REGISTER] = g_signal_new ("account-register",
					    G_OBJECT_CLASS_TYPE (klass),
					    G_SIGNAL_RUN_LAST,
					    0,
					    NULL, NULL,
					    gm_marshal_VOID__STRING_UINT,
					    G_TYPE_NONE,
					    2, G_TYPE_STRING, G_TYPE_UINT);

  signals[STATE_CHANGED] = g_signal_new ("state-changed",
					 G_OBJECT_CLASS_TYPE (klass),
					 G_SIGNAL_RUN_LAST,
					 0,
					 NULL, NULL,
					 gm_marshal_VOID__STRING_UINT,
					 G_TYPE_NONE,
					 2, G_TYPE_STRING, G_TYPE_UINT);

  signals[NAME_INFO] = g_signal_new ("name-info",
				     G_OBJECT_CLASS_TYPE (klass),
				     G_SIGNAL_RUN_LAST,
				     0,
				     NULL, NULL,
				     gm_marshal_VOID__STRING_STRING,
				     G_TYPE_NONE,
				     2, G_TYPE_STRING, G_TYPE_STRING);
  signals[CLIENT_INFO] = g_signal_new ("client-info",
				       G_OBJECT_CLASS_TYPE (klass),
				       G_SIGNAL_RUN_LAST,
				       0,
				       NULL, NULL,
				       gm_marshal_VOID__STRING_STRING,
				       G_TYPE_NONE,
				       2, G_TYPE_STRING, G_TYPE_STRING);
  signals[URL_INFO] = g_signal_new ("url-info",
				    G_OBJECT_CLASS_TYPE (klass),
				    G_SIGNAL_RUN_LAST,
				    0,
				    NULL, NULL,
				    gm_marshal_VOID__STRING_STRING,
				    G_TYPE_NONE,
				    2, G_TYPE_STRING, G_TYPE_STRING);
  signals[PROTOCOL_INFO] = g_signal_new ("protocol-info",
					 G_OBJECT_CLASS_TYPE (klass),
					 G_SIGNAL_RUN_LAST,
					 0,
					 NULL, NULL,
					 gm_marshal_VOID__STRING_STRING,
					 G_TYPE_NONE,
					 2, G_TYPE_STRING, G_TYPE_STRING);

  /* initializing as dbus object */
  dbus_g_object_type_install_info (DBUS_COMPONENT_TYPE_OBJECT,
				   &dbus_glib_dbus_component_object_info);

}

static gboolean
dbus_component_get_accounts_list (DbusComponent *self,
				  char ***accounts,
				  GError **error)
{
  /* FIXME: really should get the list of accounts
   *
   * notice that the g_strdup is important !
   */
  *accounts = g_new (char *, 2);
  (*accounts)[0] = g_strdup ("sample call token");
  (*accounts)[1] = NULL;

  return TRUE;
}

static gboolean
dbus_component_register (DbusComponent *self,
			 const char *account,
			 GError **error)
{
  /* FIXME: really should call a function to trigger the registration
   *
   * notice that the account-register signal shouldn't be sent from here !
   */

  g_print ("Registering account %s\n", account);

  g_signal_emit (self, signals[ACCOUNT_REGISTER], 0,
		 account, REGISTERED);

  return TRUE;
}

static gboolean
dbus_component_unregister (DbusComponent *self,
			   const char *account,
			   GError **error)
{
  /* FIXME: really should call a function to trigger the unregistration
   *
   * notice that the account-register signal shouldn't be sent from here !
   */

  g_print ("Unregistering account %s\n", account);

  g_signal_emit (self, signals[ACCOUNT_REGISTER], 0,
		 account, UNREGISTERED);

  return TRUE;
}

static gboolean
dbus_component_resignal_account_state (DbusComponent *self,
				       const char *account,
				       GError **error)
{
  /* FIXME: should query the state of the account and set what to send as
   * new state accordingly
   */

  g_print ("Resignalling about account %s\n", account);

  g_signal_emit (self, signals[ACCOUNT_REGISTER], 0,
		 "sample account name", REGISTERED);

  return TRUE;
}

static gboolean
dbus_component_get_calls_list (DbusComponent *self,
			       char ***calls,
			       GError **error)
{
  /* FIXME: really should get the list of calls
   *
   * notice that the g_strdup is important !
   */
  *calls = g_new (char *, 2);
  (*calls)[0] = g_strdup ("sample call token");
  (*calls)[1] = NULL;

  return TRUE;
}

static gboolean
dbus_component_connect (DbusComponent *self,
			const char *url,
			char **token,
			GError **error)
{
  /* FIXME: should really ask the endpoint to place the call and get the
   * token
   *
   * notice that the g_strdup is important
   *
   * probably we can send the state-changed signal with a new state of calling
   */

  g_print ("Connecting to %s\n", url);

  *token = g_strdup (url);
  g_signal_emit (self, signals[STATE_CHANGED], 0,
		 url, CALLING);
  g_signal_emit (self, signals[URL_INFO], 0,
		 url, url);

  return TRUE;
}

static gboolean
dbus_component_disconnect (DbusComponent *self,
			   const char *token,
			   GError **error)
{
  /* FIXME: should ask the endpoint to disconnect the call
   *
   * sending the signal should happen independantly
   */

  g_print ("Disconnecting call %s\n", token);

  g_signal_emit (self, signals[STATE_CHANGED], 0,
		 token, INVALID_CALL);

  return TRUE;
}

static gboolean
dbus_component_play_pause (DbusComponent *self,
			   const char *token,
			   GError **error)
{
  /* FIXME: ask the endpoint the current state and switch it
   */

  g_print ("Playing/pausing call %s\n", token);

  g_signal_emit (self, signals[STATE_CHANGED], 0,
		 token, CALLED);

  return TRUE;
}

static gboolean
dbus_component_transfer (DbusComponent *self,
			 const char *token,
			 const char *url,
			 GError **error)
{
  /* FIXME: ask the endpoint to do it
   */

  g_print ("Transferring token %s to %s\n", token, url);

  return TRUE;
}

static gboolean
dbus_component_resignal_call_info (DbusComponent *self,
				   const char *token,
				   GError **error)
{
  /* FIXME: should get a hold of the call, and check that each information is
   * available, and only then send the appropriate signal
   */

  g_print ("Resignalling about call %s\n", token);

  g_signal_emit (self, signals[STATE_CHANGED], 0,
		 "sample call token",
		 CONNECTED);
  g_signal_emit (self, signals[NAME_INFO], 0,
		 "sample call token",
		 "sample name");
  g_signal_emit (self, signals[CLIENT_INFO], 0,
		 "sample call token",
		 "sample client");
  g_signal_emit (self, signals[URL_INFO], 0,
		 "sample call token",
		 "sample url");
  g_signal_emit (self, signals[PROTOCOL_INFO], 0,
		 "sample call token",
		 "sample protocol");

  return TRUE;
}

static gboolean
dbus_component_shutdown (DbusComponent *self,
			 GError **error)
{
  /* FIXME: look how to do it properly
   */

  g_print ("Shutting down... or at least should\n");

  return TRUE;
}

static gboolean
dbus_component_get_local_address (DbusComponent *self,
				  const char *protocol,
				  char **url,
				  GError **error)
{
  /* FIXME: ask the endpoint
   *
   * notice that the g_strdup is important !
   */

  g_print ("Giving away address as protocol %s\n", protocol);

  *url = g_strdup ("slurp:who_you_know@where_you_know");

  return TRUE;
}

static gboolean
dbus_component_get_name (DbusComponent *self,
			 char **name,
			 GError **error)
{
  gchar *firstname = NULL;
  gchar *lastname = NULL;

  firstname = gm_conf_get_string (PERSONAL_DATA_KEY "firstname");
  lastname = gm_conf_get_string (PERSONAL_DATA_KEY "lastname");
  *name = gnomemeeting_create_fullname (firstname, lastname);

  g_free (firstname);
  g_free (lastname);
  /* not freeing the full name is not a leak : dbus will do it for us ! */

  return TRUE;
}
static gboolean
dbus_component_get_location (DbusComponent *self,
			     char **location,
			     GError **error)
{
  /* FIXME: get through gmconf
   *
   * notice that the g_strdup is important !
   */

  *location = g_strdup ("Sample Location");
  return TRUE;
}

static gboolean
dbus_component_get_comment (DbusComponent *self,
			    char **comment,
			    GError **error)
{
  /* FIXME: get through gmconf
    *
   * notice that the g_strdup is important !
  */

  *comment = g_strdup ("Sample Comment");

  return TRUE;
}

static gboolean
dbus_component_claim_ownership (DbusComponent *self)
{
  DbusComponentPrivate *data = DBUS_COMPONENT_GET_PRIVATE (self);
  DBusGConnection *bus = NULL;
  DBusGProxy *bus_proxy = NULL;
  guint request_name_result;
  GError *error = NULL;

  /* in case we are called automatically */
  if (data->owner)
    return TRUE;

  bus = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
  if (!bus) {

    g_error ("Couldn't connect to session bus : %s\n", error->message);
    return FALSE;
  }

  bus_proxy = dbus_g_proxy_new_for_name (bus, "org.freedesktop.DBus",
                                         "/org/freedesktop/DBus",
                                         "org.freedesktop.DBus");

  if (!dbus_g_proxy_call (bus_proxy, "RequestName", &error,
                          G_TYPE_STRING, "net.gnomemeeting.instance",
                          G_TYPE_UINT, DBUS_NAME_FLAG_PROHIBIT_REPLACEMENT,
                          G_TYPE_INVALID,
                          G_TYPE_UINT, &request_name_result,
                          G_TYPE_INVALID)) {

    g_error ("Couldn't get the net.gnomemeeting.instance name : %s\n",
	     error->message);
    return FALSE;
  }

  dbus_g_connection_register_g_object (bus, "/net/gnomemeeting/instance",
				       G_OBJECT (self));

  data->owner = TRUE;
  return TRUE;
}

/* implementation of the externally-visible api */

/* first a little helper function */

GObject *
gnomemeeting_dbus_component_new ()
{
  /* FIXME: this function does too many things :
   * 1. it creates one instance of it ;
   * 2. it makes it available on the bus.
   *
   * TODO:
   * 1. should be done here
   * 2. should be done in a separate function, that sets a boolean property
   * of the dbus component : is it available on the bus or not ?
   */
  DbusComponent *result = NULL;

  result = DBUS_COMPONENT_OBJECT (g_object_new (DBUS_COMPONENT_TYPE_OBJECT,
						NULL));

  (void)dbus_component_claim_ownership (result);

  return G_OBJECT (result);
}

gboolean
gnomemeeting_dbus_component_is_first_instance (GObject *self)
{
  DbusComponentPrivate *data = DBUS_COMPONENT_GET_PRIVATE (self);

  return data->owner;
}
