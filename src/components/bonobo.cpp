
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2006 Damien Sandras
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
 * GnomeMeeting is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OpenH323 and Pwlib, and distribute the combination, without
 * applying the requirements of the GNU GPL to the OpenH323 program, as long
 * as you do follow the requirements of the GNU GPL for all the rest of the
 * software thus combined.
 */


/*
 *                         bonobo_component.h  -  description
 *                         -------------------------------
 *   begin                : Mon Mar 26 2001
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : Implementation of the bonobo component
 */

#include "../../config.h"


#include "common.h"
#include "callbacks.h"
#include "gnomemeeting.h"

#include "gmdialog.h"

#include <libgnomeui/gnome-window-icon.h>
#include <bonobo-activation/bonobo-activation-activate.h>
#include <bonobo-activation/bonobo-activation-register.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-listener.h>
#include <gdk/gdkx.h>


#define ACT_IID "OAFIID:GNOME_gnomemeeting_Factory"


static void bonobo_component_handle_new_event (BonoboListener *,
					       const char *, 
					       const CORBA_any *,
					       CORBA_Environment *,
					       gpointer);

static Bonobo_RegistrationResult bonobo_component_register_as_factory (void);


/* Factory stuff */

/* DESCRIPTION  :  /
 * BEHAVIOR     :  Invoked remotely to instantiate GnomeMeeting 
 *                 with the given URL.
 * PRE          :  /
 */
static void
bonobo_component_handle_new_event (BonoboListener    *listener,
				   const char        *event_name, 
				   const CORBA_any   *any,
				   CORBA_Environment *ev,
				   gpointer           user_data)
{
  GtkWidget *main_window = NULL;

  int i;
  int argc;
  char **argv;
  CORBA_sequence_CORBA_string *args;
  
  args = (CORBA_sequence_CORBA_string *) any->_value;
  argc = args->_length;
  argv = args->_buffer;

  main_window = GnomeMeeting::Process ()->GetMainWindow ();

  if (strcmp (event_name, "new_gnomemeeting")) {

      g_warning ("Unknown event '%s' on GnomeMeeting", event_name);
      return;
  }

  
  for (i = 1; i < argc; i++) {
    if (!strcmp (argv [i], "-c") || !strcmp (argv [i], "--callto")) 
      break;
  } 


  if ((i < argc) && (i + 1 < argc) && (argv [i+1])) {

    gdk_threads_enter ();
    GnomeMeeting::Process ()->Connect (argv [i+1]);
    gdk_threads_leave ();
  }
  else {

    gdk_threads_enter ();
    gnomemeeting_warning_dialog (GTK_WINDOW (main_window), _("Cannot run GnomeMeeting"), _("GnomeMeeting is already running, if you want it to call a given callto or h323 URL, please use \"gnomemeeting -c URL\"."));
    gdk_threads_leave ();
  }
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Registers GnomeMeeting as a factory.
 * PRE          :  Returns the registration result.
 */
static Bonobo_RegistrationResult
bonobo_component_register_as_factory (void)
{
  char *per_display_iid;
  BonoboListener *listener;
  Bonobo_RegistrationResult result;

  listener = bonobo_listener_new (bonobo_component_handle_new_event, NULL);

  per_display_iid = 
    bonobo_activation_make_registration_id (ACT_IID, 
					    DisplayString (gdk_display));

  result = 
    bonobo_activation_active_server_register (per_display_iid, 
					      BONOBO_OBJREF (listener));

  if (result != Bonobo_ACTIVATION_REG_SUCCESS)
    bonobo_object_unref (BONOBO_OBJECT (listener));

  g_free (per_display_iid);

  return result;
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Invoke the factory.
 * PRE          :  Registers the new factory, or use the already registered 
 *                 factory, or displays an error in the terminal.
 */
gboolean
bonobo_component_init (int argc, char *argv[])
{
  Bonobo_Listener listener;

  switch (bonobo_component_register_as_factory ())
    {
    case Bonobo_ACTIVATION_REG_SUCCESS:
      /* we were the first GnomeMeeting to register */
      return FALSE;

    case Bonobo_ACTIVATION_REG_NOT_LISTED:
      g_printerr (_("It appears that you do not have gnomemeeting.server installed in a valid location. Factory mode disabled.\n"));
      return FALSE;
      
    case Bonobo_ACTIVATION_REG_ERROR:
      g_printerr (_("Error registering GnomeMeeting with the activation service; factory mode disabled.\n"));
      return FALSE;

    case Bonobo_ACTIVATION_REG_ALREADY_ACTIVE:
      /* lets use it then */
      break;
    }


  listener = 
    bonobo_activation_activate_from_id (ACT_IID, 
					Bonobo_ACTIVATION_FLAG_EXISTING_ONLY, 
					NULL, NULL);
  
  if (listener != CORBA_OBJECT_NIL) {

    int i;
    CORBA_any any;
    CORBA_sequence_CORBA_string args;
    CORBA_Environment ev;
    
    CORBA_exception_init (&ev);

    any._type = TC_CORBA_sequence_CORBA_string;
    any._value = &args;

    args._length = argc;
    args._buffer = g_newa (CORBA_char *, args._length);
    for (i = 0; i < (signed) (args._length); i++)
      args._buffer [i] = argv [i];
      
    Bonobo_Listener_event (listener, "new_gnomemeeting", &any, &ev);
    CORBA_Object_release (listener, &ev);

    if (!BONOBO_EX (&ev))
      return TRUE;

    CORBA_exception_free (&ev);
  } 
  else {    
    g_printerr (_("Failed to retrieve gnomemeeting server from activation server\n"));
  }
  
  return FALSE;
}
