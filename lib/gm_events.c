
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
 *                         gm_events.c  -  description
 *                         -------------------------------------
 *   begin                : Tue, 31 Aug 2004
 *   copyright            : (C) 2004 by Julien Puydt and Damien Sandras
 *   description          : New signals for GObjects, and an event dispatcher
 *
 */

#include "gm_events.h"
#include "gm_events_marshalers.h"

/* declaration of the event dispatcher GObject type */

#define GM_EVENTS_DISPATCHER_TYPE gm_events_dispatcher_get_type()


#define GM_EVENTS_DISPATCHER(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), \
                                   GM_EVENTS_DISPATCHER_TYPE, \
                                   GMEventsDispatcher))


#define GM_EVENTS_DISPATCHER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), \
                                          GM_EVENTS_DISPATCHER_TYPE, \
                                          GMEventsDispatcherClass))


#define IS_TEST(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), \
                      GM_EVENTS_DISPATCHER_TYPE))


#define GM_EVENTS_DISPATCHER_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS ((o), \
                                            GM_EVENTS_DISPATCHER_TYPE, \
                                            GMEventsDispatcherClass))


typedef struct _GMEventsDispatcher GMEventsDispatcher;


typedef struct _GMEventsDispatcherClass GMEventsDispatcherClass;


struct _GMEventsDispatcher {
  GObject parent;

  GSList *observers;
};


struct _GMEventsDispatcherClass {
  GObjectClass parent;
};


static GType gm_events_dispatcher_get_type ();


static void gm_events_dispatcher_class_init (GMEventsDispatcherClass *klass);


static void gm_events_dispatcher_init (GMEventsDispatcher *self);


static void gm_events_dispatcher_finalize (GObject *self);


/* declaration of the various signals callbacks, their name is
 * usually <signal_name>_cb for a signal named "signal-name"
 */


static void call_begin_cb (GObject *self,
			   gchar *call_token,
			   gpointer user_data);


static void call_end_cb (GObject *self,
			 gchar *call_token,
			 gpointer user_data);


static void endpoint_state_changed_cb (GObject *self,
				       gint new_state,
				       gpointer user_data);


/* implementation of the event dispatcher GObject type */


static GType
gm_events_dispatcher_get_type ()
{
  static GType my_type = 0;

  if (my_type == 0) {
 
    static const GTypeInfo my_info = {

      sizeof (GMEventsDispatcherClass),
      NULL,
      NULL,
      (GClassInitFunc)gm_events_dispatcher_class_init,
      NULL,
      NULL,
      sizeof(GMEventsDispatcher),
      0,
      (GInstanceInitFunc)gm_events_dispatcher_init,
    };
    my_type = g_type_register_static(G_TYPE_OBJECT ,
				     "GMEventsDispatcher", &my_info, 0);
  }
  
  return my_type;
}


static void
gm_events_dispatcher_class_init (GMEventsDispatcherClass *klass)
{
  GObjectClass *object_klass = NULL;

  object_klass = G_OBJECT_CLASS (klass);
  object_klass->finalize = gm_events_dispatcher_finalize;
}


static void
gm_events_dispatcher_init (GMEventsDispatcher *self)
{
  self->observers = NULL;
  g_signal_connect (G_OBJECT (self), "call-begin",
                    G_CALLBACK (call_begin_cb), NULL);
  g_signal_connect (G_OBJECT (self), "call-end",
                    G_CALLBACK (call_end_cb), NULL);
  g_signal_connect (G_OBJECT (self), "endpoint-state-changed",
                    G_CALLBACK (endpoint_state_changed_cb), NULL);
}


static void
gm_events_dispatcher_finalize (GObject *self)
{
  GObjectClass *parent_class = NULL;
  GMEventsDispatcher *dispatcher = NULL;
  GObject *obj = NULL;
  
  dispatcher = GM_EVENTS_DISPATCHER (self);
  
  g_slist_foreach (dispatcher->observers, (GFunc) g_object_unref, NULL);
  g_slist_free (dispatcher->observers);
  
  parent_class = G_OBJECT_CLASS (g_type_class_peek_parent (GM_EVENTS_DISPATCHER_GET_CLASS (self)));
  if (parent_class->finalize)
    parent_class->finalize (G_OBJECT (self));
}


/* implementation of the signal callbacks: they all basically loop over the
 * observers and send the signals to each, without any treatment
 */


static void
call_begin_cb (GObject *self,
	       gchar *call_token,
	       gpointer user_data)
{
  GMEventsDispatcher *dispatcher = NULL;
  GObject *obj = NULL;
  GSList *list = NULL;
  
  dispatcher = GM_EVENTS_DISPATCHER (self);

  for (list = dispatcher->observers;
       list != NULL;
       list = g_slist_next (list)) {

    obj = G_OBJECT (list->data);
    g_signal_emit_by_name (obj, "call-begin", call_token);
  }
}


static void call_end_cb (GObject *self,
			 gchar *call_token,
			 gpointer user_data)
{
  GMEventsDispatcher *dispatcher = NULL;
  GObject *obj = NULL;
  GSList *list = NULL;
  
  dispatcher = GM_EVENTS_DISPATCHER (self);

  for (list = dispatcher->observers;
       list != NULL;
       list = g_slist_next (list)) {

    obj = G_OBJECT (list->data);
    g_signal_emit_by_name (obj, "call-end", call_token);
  }
}


static void endpoint_state_changed_cb (GObject *self,
				       gint new_state,
				       gpointer user_data)
{
  GMEventsDispatcher *dispatcher = NULL;
  GObject *obj = NULL;
  GSList *list = NULL;
  
  dispatcher = GM_EVENTS_DISPATCHER (self);

  for (list = dispatcher->observers;
       list != NULL;
       list = g_slist_next (list)) {

    obj = G_OBJECT (list->data);
    g_signal_emit_by_name (obj, "endpoint-state-changed", new_state);
  }
}


/* implementation of the external api */


void
gm_events_init ()
{
  g_signal_new ("call-begin",
		G_TYPE_OBJECT,
		G_SIGNAL_RUN_LAST,
		0, NULL, NULL,
		gm_event_VOID__STRING,
		G_TYPE_NONE, 1,
		G_TYPE_STRING);

  g_signal_new ("call-end",
		G_TYPE_OBJECT,
		G_SIGNAL_RUN_LAST,
		0, NULL, NULL,
		gm_event_VOID__STRING,
		G_TYPE_NONE, 1,
		G_TYPE_STRING);

  g_signal_new ("endpoint-state-changed",
		G_TYPE_OBJECT,
		G_SIGNAL_RUN_LAST,
		0, NULL, NULL,
		gm_event_VOID__INT,
		G_TYPE_NONE, 1,
		G_TYPE_INT);
}


GObject*
gm_events_dispatcher_new ()
{
  return g_object_new (GM_EVENTS_DISPATCHER_TYPE, NULL);
}


void
gm_events_dispatcher_add_observer (GObject *dispatcher,
				   GObject *observer)
{
  GMEventsDispatcher *self = NULL;

  g_return_if_fail (dispatcher != NULL);
  g_return_if_fail (observer != NULL);

  self = GM_EVENTS_DISPATCHER (dispatcher);
  g_object_ref (observer);
  self->observers = g_slist_prepend (self->observers, (gpointer)observer);
}
