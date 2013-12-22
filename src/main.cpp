
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2012 Damien Sandras <dsandras@seconix.com>
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
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Ekiga is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OPAL, OpenH323 and PWLIB, and distribute the combination,
 * without applying the requirements of the GNU GPL to the OPAL, OpenH323
 * and PWLIB programs, as long as you do follow the requirements of the
 * GNU GPL for all the rest of the software thus combined.
 */


/*
 *                         main.cpp  -  description
 *                         -------------------------------
 *   begin                : Sun Apr 29 2012
 *   copyright            : (C) 2000-2012 by Damien Sandras
 *   description          : This file contains the main method.
 */

#include "revision.h"
#include "config.h"

#include "platform/platform.h"

#include <glib/gi18n.h>

#include <opal/buildopts.h> // only for OPAL_VERSION!

#ifdef HAVE_DBUS
#include "dbus-helper/dbus.h"
#endif

#ifndef WIN32
#include <signal.h>
#include <gdk/gdkx.h>
#else
#include "platform/winpaths.h"
#include <gdk/gdkwin32.h>
#include <cstdio>
#endif

#include "ekiga-settings.h"

#include "engine.h"
#include "runtime.h"

#include "call-core.h"

#include "gtk-frontend.h"
#include "ekiga.h"

#ifdef WIN32
// the linker must not find main
#define main(c,v,e) ekigas_real_main(c,v,e)
#endif

/* The main () */
int
main (int argc,
      char ** argv,
      char ** /*envp*/)
{
  GOptionContext *context = NULL;

  Ekiga::ServiceCorePtr service_core(new Ekiga::ServiceCore);

  gchar *path = NULL;
  gchar *url = NULL;

  int debug_level = 0;

  /* Globals */
#ifndef WIN32
  if (!XInitThreads ())
    exit (1);
#endif

#if !GLIB_CHECK_VERSION(2,36,0)
  g_type_init ();
#endif
#if !GLIB_CHECK_VERSION(2,32,0)
  g_thread_init();
#endif

  /* GTK+ initialization */
  gtk_init (&argc, &argv);
#ifndef WIN32
  signal (SIGPIPE, SIG_IGN);
#endif

  /* initialize platform-specific code */
  gm_platform_init ();
#ifdef WIN32
  // plugins (i.e. the audio/video ptlib/opal codecs) are searched in ./plugins
  chdir (win32_datadir ());
#endif

  /* Gettext initialization */
  path = g_build_filename (DATA_DIR, "locale", NULL);
  textdomain (GETTEXT_PACKAGE);
  bindtextdomain (GETTEXT_PACKAGE, path);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  g_free (path);

  /* Application name */
  g_set_application_name (_("Ekiga Softphone"));
#ifndef WIN32
  setenv ("PULSE_PROP_application.name", _("Ekiga Softphone"), true);
#endif

  /* Arguments initialization */
  GOptionEntry arguments [] =
    {
      {
	"debug", 'd', 0, G_OPTION_ARG_INT, &debug_level,
       N_("Prints debug messages in the console (level between 1 and 8)"),
       NULL
      },
      {
	"call", 'c', 0, G_OPTION_ARG_STRING, &url,
	N_("Makes Ekiga call the given URI"),
	NULL
      },
      {
	NULL, 0, 0, (GOptionArg)0, NULL,
	NULL,
	NULL
      }
    };
  context = g_option_context_new (NULL);
  g_option_context_add_main_entries (context, arguments, PACKAGE_NAME);
  g_option_context_set_help_enabled (context, TRUE);

  g_option_context_add_group (context, gtk_get_option_group (TRUE));
  g_option_context_parse (context, &argc, &argv, NULL);
  g_option_context_free (context);

#ifndef WIN32
  char* text_label =  g_strdup_printf ("%d", debug_level);
  setenv ("PTLIB_TRACE_CODECS", text_label, TRUE);
  g_free (text_label);
#else
  char* text_label =  g_strdup_printf ("PTLIB_TRACE_CODECS=%d", debug_level);
  _putenv (text_label);
  g_free (text_label);
  if (debug_level != 0) {
    std::string desk_path = g_get_user_special_dir (G_USER_DIRECTORY_DESKTOP);
    if (!desk_path.empty ())
      std::freopen((desk_path + "\\ekiga-stderr.txt").c_str (), "w", stderr);
  }
#endif

#if PTRACING
  if (debug_level != 0)
    PTrace::Initialise (PMAX (PMIN (8, debug_level), 0), NULL,
			PTrace::Timestamp | PTrace::Thread
			| PTrace::Blocks | PTrace::DateAndTime);
#endif

#ifdef HAVE_DBUS
  if (!ekiga_dbus_claim_ownership ()) {
    ekiga_dbus_client_show ();
    if (url != NULL)
      ekiga_dbus_client_connect (url);
    exit (0);
  }
#endif

  /* Ekiga initialisation */
  // should come *after* ptrace initialisation, to track codec loading for ex.
  GnomeMeeting instance;

  Ekiga::Runtime::init ();
  engine_init (service_core, argc, argv);

  PTRACE (1, "Ekiga version "
          << MAJOR_VERSION << "." << MINOR_VERSION << "." << BUILD_NUMBER);
#ifdef EKIGA_REVISION
  PTRACE (1, "Ekiga git revision: " << EKIGA_REVISION);
#endif
  PTRACE (1, "PTLIB version " << PTLIB_VERSION);
  PTRACE (1, "OPAL version " << OPAL_VERSION);
#if defined HAVE_XV || defined HAVE_DX
  PTRACE (1, "Accelerated rendering support enabled");
#else
  PTRACE (1, "Accelerated rendering support disabled");
#endif
#ifdef HAVE_DBUS
  PTRACE (1, "DBUS support enabled");
#else
  PTRACE (1, "DBUS support disabled");
#endif

  boost::shared_ptr<GtkFrontend> gtk_frontend
    = service_core->get<GtkFrontend>("gtk-frontend");
  boost::shared_ptr<Ekiga::Settings> general_settings (new Ekiga::Settings (GENERAL_SCHEMA));

  GtkWidget *main_window = GTK_WIDGET (gtk_frontend->get_main_window ());

  const int schema_version = MAJOR_VERSION * 1000
                             + MINOR_VERSION * 10
                             + BUILD_NUMBER;
  int crt_version = general_settings->get_int ("version");
  if (crt_version < schema_version) {

    // show the assistant if there is no config file
    if (crt_version == 0)
      gtk_widget_show_all (GTK_WIDGET (gtk_frontend->get_assistant_window ()));

    /* Update the version number */
    general_settings->set_int ("version", schema_version);
  }

  /* Show the main window */
  gtk_widget_show (main_window);

  /* Call the given host if needed */
  if (url) {

    boost::shared_ptr<Ekiga::CallCore> call_core = service_core->get<Ekiga::CallCore> ("call-core");
    call_core->dial (url);
  }

#ifdef HAVE_DBUS
  EkigaDBusComponent *dbus_component = ekiga_dbus_component_new (*service_core);
#endif

  // from now on, things should have taken their final place
  service_core->close ();

  /* The GTK loop */
  gtk_main ();

#ifdef HAVE_DBUS
  g_object_unref (dbus_component);
#endif

  /* Exit Ekiga */
  GnomeMeeting::Process ()->Exit ();
  service_core.reset ();
  Ekiga::Runtime::quit ();

  /* deinitialize platform-specific code */
  gm_platform_shutdown ();

  return 0;
}


#ifdef WIN32

typedef struct {
  int newmode;
} _startupinfo;

extern "C" void __getmainargs (int *argcp, char ***argvp, char ***envp, int glob, _startupinfo *sinfo);
int
APIENTRY WinMain (HINSTANCE hInstance,
		  HINSTANCE hPrevInstance,
		  LPSTR     lpCmdLine,
		  int       nCmdShow)
{
  HANDLE ekr_mutex;
  int iresult = 0;
  char **env;
  char **argv;
  int argc;
  _startupinfo info = {0};

  ekr_mutex = CreateMutex (NULL, FALSE, "EkigaIsRunning");
  if (GetLastError () == ERROR_ALREADY_EXISTS)
    MessageBox (NULL, "Ekiga is running already !", "Ekiga - 2nd instance", MB_ICONEXCLAMATION | MB_OK);
  else {

    /* use msvcrt.dll to parse command line */
    __getmainargs (&argc, &argv, &env, 0, &info);

    iresult = main (argc, argv, env);
  }
  CloseHandle (ekr_mutex);
  return iresult;
}
#endif

