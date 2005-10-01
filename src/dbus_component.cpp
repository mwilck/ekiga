
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
 *                         dbus_component.cpp  -  description
 *                         --------------------------
 *   begin                : Tue Oct 26 2004
 *   copyright            : (C) 2004 by Julien Puydt
 *   description          : Implementation of the DBUS component.
 *
 */

#include "dbus_component.h"
#define DBUS_API_SUBJECT_TO_CHANGE
#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h>

#include "gm_conf.h"

#include "common.h"
#include "gnomemeeting.h"
#include "endpoint.h"

/* declaration of the GObject 
 * this is pretty stupid/standard code
 */


#define DBUS_COMPONENT_TYPE dbus_component_get_type ()


#define DBUS_COMPONENT(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), \
                             DBUS_COMPONENT_TYPE, DBusComponent))


#define DBUS_COMPONENT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), \
                                     DBUS_COMPONENT_TYPE, DBusComponentClass))


#define IS_DBUS_COMPONENT(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), \
			        DBUS_COMPONENT_TYPE))


#define DBUS_COMPONENT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                                       DBUS_COMPONENT_TYPE, \
                                       DBusComponentClass)) 


typedef struct _DBusComponent DBusComponent;


typedef struct _DBusComponentClass DBusComponentClass;


struct _DBusComponent
{
  GObject parent;

  GMEndPoint *endpoint; /* gnomemeeting's end of the bridge */
  DBusConnection *connection; /* DBUS' end of the bridge */
  gboolean is_registered; /* are we the first gnomemeeting known to DBUS? */
  gboolean owns_the_service; /* did we manage to own the DBUS service? */
};


struct _DBusComponentClass
{
  GObjectClass parent;
};


static GType dbus_component_get_type();


static void dbus_component_finalize (GObject *self);


static void dbus_component_init (DBusComponent *self);


static void dbus_component_class_init (DBusComponentClass *klass);


/* declaration of various helpers */

/* turns the endpoint state into a string, easier to move around */
static const gchar *state_to_string (GMEndPoint::CallingState);

/* this function connects/reconnects the component to DBUS
 * it has that signature because it is also called through a timer, when
 * we lose the connection
 */
static gboolean connect_component (gpointer user_data);


/* declaration of the signal callbacks
 * those are for the signals that the endpoint emits on the component, and that
 * it will then broadcast on DBUS.
 */


static void call_begin_cb (GObject *self,
			   gchar *call_token,
			   gpointer user_data);


static void call_end_cb (GObject *self,
			 gchar *call_token,
			 gpointer user_data);


static void endpoint_state_changed_cb (GObject *self,
				       GMEndPoint::CallingState new_state,
				       gpointer user_data);


/* declaration of the DBUS helper functions */


/* this function is called by DBUS when a watched type of message
 * arrives ; it is only used to know if we're still connected to the bus
 */
static DBusHandlerResult filter_func (DBusConnection *connection,
				      DBusMessage *message,
				      void *user_data);



/* this function is called by DBUS when a message directed at the
 * GM_DBUS_OBJECT_PATH arrives (provided we're the registered instance!)
 * it routes the message to the correct handler
 */
static DBusHandlerResult path_message_func (DBusConnection *connection,
					    DBusMessage *message,
					    void *user_data);

/* the rest of those DBUS helpers, with name "handle_<method call>_message",
 * are used to handle the various method calls. They get their arguments
 * directly from path_message_func.
 */
static void handle_connect_to_message (DBusConnection *connection,
				       DBusMessage *message);


static void handle_get_state_message (DBusConnection *connection,
				      DBusMessage *message);


static void handle_disconnect_message (DBusConnection *connection,
				       DBusMessage *message);


static void handle_get_calls_list_message (DBusConnection *connection,
					   DBusMessage *message);


static void handle_get_call_info_message (DBusConnection *connection,
					  DBusMessage *message);


/* definition of some helper DBUS-related data */


static DBusObjectPathVTable call_vtable = {
  NULL,
  path_message_func,
  NULL,
};


/* Implementation of the GObject */


static GType
dbus_component_get_type()
{
  static GType my_type = 0; 
  
  if (!my_type) {

    static const GTypeInfo my_info = {
      sizeof (DBusComponentClass),
      NULL,
      NULL,
      (GClassInitFunc) dbus_component_class_init,
      NULL,
      NULL,
      sizeof(DBusComponent),
      0,
      (GInstanceInitFunc) dbus_component_init
    };
    my_type = g_type_register_static (G_TYPE_OBJECT ,
				      "DBusComponent", &my_info, 
				      (GTypeFlags)0);
  }
  
  return my_type;
}


static void
dbus_component_finalize (GObject *object)
{
  DBusComponent *self = NULL;
  GObjectClass *parent_class = NULL;

  g_return_if_fail (IS_DBUS_COMPONENT (object));

  self = DBUS_COMPONENT (object);

  if (self->connection != NULL) {

    dbus_connection_disconnect (self->connection);
    dbus_connection_unref (self->connection);
  }
 
  parent_class = G_OBJECT_CLASS (g_type_class_peek_parent (DBUS_COMPONENT_GET_CLASS (self)));

  parent_class->finalize (G_OBJECT(self));
}


static void
dbus_component_init (DBusComponent *self)
{
  self->endpoint = NULL;
  self->connection = NULL;
  self->is_registered = FALSE;
  self->owns_the_service = FALSE;

  g_signal_connect (G_OBJECT (self),
		    "call-begin",
		    G_CALLBACK (call_begin_cb),
		    NULL);
  g_signal_connect (G_OBJECT (self),
		    "call-end",
		    G_CALLBACK (call_end_cb),
		    NULL);
  g_signal_connect (G_OBJECT (self),
		    "endpoint-state-changed",
		    G_CALLBACK (endpoint_state_changed_cb),
		    NULL);
}


static void dbus_component_class_init (DBusComponentClass *klass)
{
  GObjectClass *object_klass = G_OBJECT_CLASS (klass);

  object_klass->finalize = dbus_component_finalize;
}


/* implementation of various helpers */


static const gchar*
state_to_string (GMEndPoint::CallingState state)
{
  static const gchar *standby = "Standby";
  static const gchar *calling = "Calling";
  static const gchar *connected = "Connected";
  static const gchar *called = "Called";
  static const gchar *bogus = "Bogus";
  const gchar *result;

  switch (state) {

  case GMEndPoint::Standby:
    result = standby;
    break;
  case GMEndPoint::Calling:
    result = calling;
    break;
  case GMEndPoint::Connected:
    result  = connected;
    break;
  case GMEndPoint::Called:
    result = called;
    break;
  default:
    result = bogus;
  }
  
  return result;
} 


static gboolean
connect_component (gpointer user_data)
{
  DBusComponent *self = NULL;
  
  g_return_val_if_fail (IS_DBUS_COMPONENT (user_data), TRUE);

  self = DBUS_COMPONENT (user_data);

  if (self->connection == NULL) { /* we lost contact with the server */

    self->connection = dbus_bus_get (DBUS_BUS_SESSION, NULL);
    if (self->connection != NULL) {

      if (dbus_connection_add_filter (self->connection, 
				      filter_func,
				      self, NULL))
 	dbus_connection_setup_with_g_main (self->connection, NULL);
      else {

	dbus_connection_disconnect (self->connection);
	self->connection = NULL;
      }
    }
  }

  if (self->connection != NULL) {  
    /* we have a contact with the server, check the rest */
    if (self->is_registered == FALSE)
      self->is_registered 
	= dbus_connection_register_object_path (self->connection,
						GM_DBUS_OBJECT_PATH,
						&call_vtable, self);
    
    if (self->owns_the_service == FALSE)
      self->owns_the_service 
	= (dbus_bus_request_name (self->connection, 
				     GM_DBUS_SERVICE, 0, NULL) >= 0);
    
 
  }

  return self->connection != NULL && self->is_registered && self->owns_the_service;
}


/* implementation of the signal callbacks */


static void
call_begin_cb (GObject *object,
	       gchar *call_token,
	       gpointer user_data)
{
  DBusComponent *self = NULL;
  DBusMessage *message = NULL;

  g_return_if_fail (IS_DBUS_COMPONENT (object));

  self = DBUS_COMPONENT (object);

  if (self->connection == NULL)
    return;

  message = dbus_message_new_signal (GM_DBUS_OBJECT_PATH,
				     GM_DBUS_INTERFACE, "AddCall");

  if (dbus_message_append_args (message,
				DBUS_TYPE_STRING, call_token,
				DBUS_TYPE_INVALID)) {

    (void)dbus_connection_send (self->connection, message, NULL);
    dbus_connection_flush (self->connection);
  }
  dbus_message_unref (message);
}


static void
call_end_cb (GObject *object,
	     gchar *call_token,
	     gpointer user_data)
{
  DBusComponent *self = NULL;
  DBusMessage *message = NULL;

  g_return_if_fail (IS_DBUS_COMPONENT (object));

  self = DBUS_COMPONENT (object);

  if (self->connection == NULL)
    return;

  message = dbus_message_new_signal (GM_DBUS_OBJECT_PATH,
				     GM_DBUS_INTERFACE, "DeleteCall");

  if (dbus_message_append_args (message,
				DBUS_TYPE_STRING, call_token,
				DBUS_TYPE_INVALID)) {

    (void)dbus_connection_send (self->connection, message, NULL);
    dbus_connection_flush (self->connection);
  }
  dbus_message_unref (message);
}


static void
endpoint_state_changed_cb (GObject *object,
			   GMEndPoint::CallingState new_state,
			   gpointer user_data)
{
  DBusComponent *self = NULL;
  DBusMessage *message = NULL;

  g_return_if_fail (IS_DBUS_COMPONENT (object));

  self = DBUS_COMPONENT (object);

  if (self->connection == NULL)
    return;

  message = dbus_message_new_signal (GM_DBUS_OBJECT_PATH,
				     GM_DBUS_INTERFACE, "StateChanged");

  if (dbus_message_append_args (message,
				DBUS_TYPE_STRING, state_to_string (new_state),
				DBUS_TYPE_INVALID)) {

    (void)dbus_connection_send (self->connection, message, NULL);
    dbus_connection_flush (self->connection);
  }
  dbus_message_unref (message);
}


/* implementation of the DBUS helpers */


static DBusHandlerResult
filter_func (DBusConnection *connection,
	     DBusMessage *message,
	     void *user_data)
{
  DBusComponent *self = NULL;

  g_return_val_if_fail (user_data != NULL,
			DBUS_HANDLER_RESULT_NOT_YET_HANDLED);

  self = DBUS_COMPONENT (user_data);

  g_return_val_if_fail (self->connection == connection,
			DBUS_HANDLER_RESULT_NOT_YET_HANDLED);

  if (dbus_message_is_signal (message,
                              DBUS_INTERFACE_LOCAL,
                              "Disconnected"))
    {

      dbus_connection_unref (self->connection);
      self->connection = NULL;
      self->is_registered = FALSE;
      self->owns_the_service = FALSE;
      g_timeout_add (3000, connect_component, (gpointer)self);

      return DBUS_HANDLER_RESULT_HANDLED;
    }

  return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}


static DBusHandlerResult
path_message_func (DBusConnection *connection,
                   DBusMessage *message,
                   void *user_data)
{
  DBusComponent *self = NULL;
  DBusHandlerResult result = DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

  self = DBUS_COMPONENT (user_data);
  if (dbus_message_is_method_call (message,
                                   GM_DBUS_SERVICE,
                                   "ConnectTo")) {

    handle_connect_to_message (connection, message);
    result = DBUS_HANDLER_RESULT_HANDLED;
  }
  else if (dbus_message_is_method_call (message,
					GM_DBUS_SERVICE,
					"GetState")) {

    handle_get_state_message (connection, message);
    result = DBUS_HANDLER_RESULT_HANDLED;
  }
  else if (dbus_message_is_method_call (message,
					GM_DBUS_SERVICE,
					"Disconnect")) {

    handle_disconnect_message (connection, message);
    result = DBUS_HANDLER_RESULT_HANDLED;
  }
  else if (dbus_message_is_method_call (message,
					GM_DBUS_SERVICE,
					"GetCallsList")) {

    handle_get_calls_list_message (connection, message);
    result = DBUS_HANDLER_RESULT_HANDLED;
  }
  else if (dbus_message_is_method_call (message,
					GM_DBUS_SERVICE,
					"GetCallInfo")) {

    handle_get_call_info_message (connection, message);
    result = DBUS_HANDLER_RESULT_HANDLED;
  }
  
  return result;
}


static void
handle_connect_to_message (DBusConnection *connection,
			   DBusMessage *message)
{
  gchar *address = NULL;

  if (dbus_message_get_args (message, NULL,
			     DBUS_TYPE_STRING, &address,
			     DBUS_TYPE_INVALID)) {

    GnomeMeeting::Process ()->Connect (address);
    g_free (address);
  }
}


static void
handle_get_state_message (DBusConnection *connection,
			  DBusMessage *message)
{
  DBusMessage *reply = NULL;
  const gchar *state = NULL;
  GMEndPoint *ep = NULL;

  ep = GnomeMeeting::Process ()->Endpoint ();
  
  reply = dbus_message_new_method_return (message);
  state = state_to_string (ep->GetCallingState ());
  if (dbus_message_append_args (reply,
				DBUS_TYPE_STRING, state,
				DBUS_TYPE_INVALID)) {

    (void)dbus_connection_send (connection, reply, NULL);
    dbus_connection_flush (connection);
  }
  dbus_message_unref (reply);
}


static void
handle_disconnect_message (DBusConnection *connection,
			   DBusMessage *message)
{
  gchar *call_token = NULL;
  if (dbus_message_get_args (message, NULL,
			     DBUS_TYPE_STRING, &call_token,
			     DBUS_TYPE_INVALID)) {
  
    /* FIXME: should use call_token, when gnomemeeting will support it! */
    GnomeMeeting::Process ()->Disconnect ();
    g_free (call_token);
  }
}


static void
handle_get_calls_list_message (DBusConnection *connection,
			       DBusMessage *message)
{
  DBusMessage *reply = NULL;
  const char *call_token = NULL;
  GMEndPoint *ep = NULL;

  ep = GnomeMeeting::Process ()->Endpoint ();
  
  reply = dbus_message_new_method_return (message);
  call_token = (const char *)ep->GetCurrentCallToken ();
  if (dbus_message_append_args (reply,
				DBUS_TYPE_STRING, call_token,
				DBUS_TYPE_INVALID)) {

    (void)dbus_connection_send (connection, reply, NULL);
    dbus_connection_flush (connection);
  }
  dbus_message_unref (reply);
}


static void
handle_get_call_info_message (DBusConnection *connection,
			      DBusMessage *message)
{
  DBusMessage *reply = NULL;
  const char *call_token = NULL;
  GMEndPoint *ep = NULL;
  PSafePtr<OpalCall> call = NULL;
  PSafePtr<OpalConnection> gmconnection = NULL;
  gchar *name = NULL;
  gchar *url = NULL;
  gchar *app = NULL;

  ep = GnomeMeeting::Process ()->Endpoint ();

  if (dbus_message_get_args (message, NULL,
			     DBUS_TYPE_STRING, &call_token,
			     DBUS_TYPE_INVALID)) {
  
    call = ep->FindCallWithLock ((PString)call_token);
    gmconnection = ep->GetConnection (call, TRUE);
    if (gmconnection != NULL)
      ep->GetRemoteConnectionInfo (*gmconnection, name, app, url);

    reply = dbus_message_new_method_return (message);
    if (dbus_message_append_args (reply,
				  DBUS_TYPE_STRING, name,
				  DBUS_TYPE_STRING, url,
				  DBUS_TYPE_STRING, app,
				  DBUS_TYPE_INVALID)) {

      (void)dbus_connection_send (connection, reply, NULL);
      dbus_connection_flush (connection);
    }
    dbus_message_unref (reply);
    g_free (name);
    g_free (app);
    g_free (url);
  }
}


/* implementation of the externally-visible api */


GObject*
dbus_component_new(GMEndPoint *endpoint)
{
  DBusComponent *result = NULL;

  result = DBUS_COMPONENT (g_object_new (DBUS_COMPONENT_TYPE, NULL));

  result->endpoint = endpoint;
  result->endpoint->AddObserver (G_OBJECT (result));

  (void)connect_component ((gpointer)result);

  return G_OBJECT (result);
}


gboolean
dbus_component_is_first_instance (GObject *object)
{
  g_return_val_if_fail (IS_DBUS_COMPONENT (object), FALSE);

  return DBUS_COMPONENT (object)->is_registered;
}


void
dbus_component_call_address (GObject *object, 
			     const gchar *address)
{
  DBusMessage *message = NULL;
  DBusComponent *self = NULL;

  g_return_if_fail (IS_DBUS_COMPONENT (object));
 
  self = DBUS_COMPONENT (object);
 
  if (self->connection == NULL)
    return;

  message = dbus_message_new_method_call (GM_DBUS_SERVICE,
					  GM_DBUS_OBJECT_PATH,
					  GM_DBUS_INTERFACE, "Call");

  dbus_message_set_no_reply (message, TRUE);
  
  if (dbus_message_append_args (message,
				DBUS_TYPE_STRING, address,
				DBUS_TYPE_INVALID)) {
    
    (void)dbus_connection_send (self->connection,
				message, NULL);
    dbus_connection_flush (self->connection);
  }
  dbus_message_unref (message);
}
