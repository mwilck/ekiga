
/*
 * Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2007 Damien Sandras

 * This program is free software; you can  redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version. This program is distributed in the hope
 * that it will be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Ekiga is licensed under the GPL license and as a special exception, you
 * have permission to link or otherwise combine this program with the
 * programs OPAL, OpenH323 and PWLIB, and distribute the combination, without
 * applying the requirements of the GNU GPL to the OPAL, OpenH323 and PWLIB
 * programs, as long as you do follow the requirements of the GNU GPL for all
 * the rest of the software thus combined.
 */


/*
 *                         runtime.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : declaration of a service object
 *
 */

#include "runtime.h"

struct thread_data
{
  thread_data (sigc::slot<void> _action): action(_action) {}

  sigc::slot<void> action;
};

static void
common_helper (struct thread_data *data)
{
  data->action ();
  delete data;
}

static gboolean
run_later_or_back_in_main_helper (gpointer data)
{
  common_helper ((struct thread_data *)data);
  return FALSE;
}

static gpointer
run_in_thread_helper (gpointer data)
{
  common_helper ((struct thread_data *)data);
  return NULL;
}

Ekiga::Runtime::Runtime ()
{
  loop = g_main_loop_new (NULL, FALSE);
}

Ekiga::Runtime::~Runtime ()
{
  quit ();
  g_main_loop_unref (loop);
}

void
Ekiga::Runtime::run ()
{
  g_main_loop_run (loop);
}

void
Ekiga::Runtime::quit ()
{
  g_main_loop_quit (loop);
}

void
Ekiga::Runtime::run_later (sigc::slot<void> action,
			   unsigned int seconds)
{
  g_timeout_add (1000*seconds, run_later_or_back_in_main_helper,
		 (gpointer)(new struct thread_data (action)));
}

void
Ekiga::Runtime::run_in_thread (sigc::slot<void> action)
{
  g_thread_create (run_in_thread_helper,
		   (gpointer)(new struct thread_data (action)),
		   FALSE, NULL);
}

void
Ekiga::Runtime::run_back_in_main (sigc::slot<void> action)
{
  g_idle_add (run_later_or_back_in_main_helper,
	      (gpointer)(new struct thread_data (action)));
}
