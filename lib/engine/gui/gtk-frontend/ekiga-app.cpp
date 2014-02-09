
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2014 Damien Sandras <dsandras@seconix.com>
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
 *                         ekiga-app.cpp  -  description
 *                         -----------------------------
 *   begin                : written in Feb 2014 by Damien Sandras
 *   copyright            : (c) 2014 by Damien Sandras
 *   description          : main Ekiga GtkApplication
 *
 */

#include <gtk/gtk.h>

#include "config.h"
#include "ekiga-settings.h"

#include "trigger.h"
#include "ekiga-app.h"
#include "gmstockicons.h"
#include "account-core.h"
#include "chat-core.h"
#include "contact-core.h"
#include "presence-core.h"
#include "addressbook-window.h"
#include "accounts-window.h"
#include "assistant-window.h"
#include "preferences-window.h"
#include "call-window.h"
#include "chat-window.h"
#include "main_window.h"
#include "statusicon.h"
#include "roster-view-gtk.h"
#include "history-source.h"
#include "opal-bank.h"
#include "book-view-gtk.h"
#include "notification-core.h"
#include "personal-details.h"
#include "audioinput-core.h"
#include "audiooutput-core.h"
#include "videoinput-core.h"
#include "videooutput-core.h"
#include "call-core.h"
#include "engine.h"
#include "runtime.h"

#include "gmwindow.h"

#ifdef WIN32
#include "platform/winpaths.h"
#include <windows.h>
#include <shellapi.h>
#define WIN32_HELP_DIR "help"
#define WIN32_HELP_FILE "index.html"
#endif

#ifdef HAVE_DBUS
#include "../../../../src/dbus-helper/dbus.h"
#endif


#include <glib/gi18n.h>

/*
 * The GmApplication
 */
struct _GmApplicationPrivate
{
  Ekiga::ServiceCorePtr core;

  GtkWidget *main_window;
  GtkWidget *chat_window;

  EkigaDBusComponent *dbus_component;
};

G_DEFINE_TYPE (GmApplication, gm_application, GTK_TYPE_APPLICATION);

/* Private helpers */
static gboolean
option_context_parse (GOptionContext *context,
                      gchar **arguments,
                      GError **error)
{
  gint argc;
  gchar **argv;
  gint i;
  gboolean ret;

  /* We have to make an extra copy of the array, since g_option_context_parse()
   * assumes that it can remove strings from the array without freeing them.
   */
  argc = g_strv_length (arguments);
  argv = g_new (gchar *, argc);
  for (i = 0; i < argc; i++)
    argv[i] = arguments[i];

  ret = g_option_context_parse (context, &argc, &argv, error);

  g_free (argv);

  return ret;
}

static void
quit_activated (G_GNUC_UNUSED GSimpleAction *action,
                G_GNUC_UNUSED GVariant *parameter,
                gpointer app)
{
  g_application_quit (G_APPLICATION (app));
}


static void
about_activated (G_GNUC_UNUSED GSimpleAction *action,
                 G_GNUC_UNUSED GVariant *parameter,
                 gpointer app)
{
  gm_application_show_about (GM_APPLICATION (app));
}


static void
help_activated (G_GNUC_UNUSED GSimpleAction *action,
                G_GNUC_UNUSED GVariant *parameter,
                gpointer app)
{
  gm_application_show_help (GM_APPLICATION (app), NULL);
}


static void
window_activated (GSimpleAction *action,
                  G_GNUC_UNUSED GVariant *parameter,
                  gpointer app)
{
  GmApplication *self = GM_APPLICATION (app);

  g_return_if_fail (self && self->priv->core);

  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GM_TYPE_APPLICATION, GmApplicationPrivate);

  boost::shared_ptr<Ekiga::AudioInputCore> audio_input_core =
    self->priv->core->get<Ekiga::AudioInputCore> ("audioinput-core");
  boost::shared_ptr<Ekiga::AudioOutputCore> audio_output_core =
    self->priv->core->get<Ekiga::AudioOutputCore> ("audiooutput-core");
  boost::shared_ptr<Ekiga::VideoInputCore> video_input_core =
    self->priv->core->get<Ekiga::VideoInputCore> ("videoinput-core");
  boost::shared_ptr<Ekiga::ContactCore> contact_core =
    self->priv->core->get<Ekiga::ContactCore> ("contact-core");

  parent = GM_WINDOW (gtk_application_get_active_window (GTK_APPLICATION (self)));

  if (!g_strcmp0 (g_action_get_name (G_ACTION (action)), "preferences"))
    gm_application_show_preferences_window (self);

  else if (!g_strcmp0 (g_action_get_name (G_ACTION (action)), "addressbook"))
    gm_application_show_addressbook_window (self);

  else if (!g_strcmp0 (g_action_get_name (G_ACTION (action)), "accounts"))
    gm_application_show_accounts_window (self);

  else if (!g_strcmp0 (g_action_get_name (G_ACTION (action)), "assistant"))
    gm_application_show_assistant_window (self);
}

static GActionEntry app_entries[] =
{
    { "preferences", window_activated, NULL, NULL, NULL, 0 },
    { "assistant", window_activated, NULL, NULL, NULL, 0 },
    { "addressbook", window_activated, NULL, NULL, NULL, 0 },
    { "accounts", window_activated, NULL, NULL, NULL, 0 },
    { "help", help_activated, NULL, NULL, NULL, 0 },
    { "about", about_activated, NULL, NULL, NULL, 0 },
    { "quit", quit_activated, NULL, NULL, NULL, 0 }
};


/* Public api */
void
ekiga_main (int argc,
            char **argv)
{
  GmApplication *app = gm_application_new ();
  g_application_set_inactivity_timeout (G_APPLICATION (app), 10000);

  if (g_application_get_is_remote (G_APPLICATION (app))) {
    g_application_run (G_APPLICATION (app), argc, argv);
    return;
  }

  Ekiga::ServiceCorePtr core(new Ekiga::ServiceCore);

  Ekiga::Runtime::init ();
  engine_init (core, argc, argv);

  gm_application_set_core (app, core);

  app->priv->main_window = gm_main_window_new (app);
  gm_application_show_main_window (app);

  app->priv->chat_window = chat_window_new (app);
  status_icon_new (app);

#ifdef HAVE_DBUS
  app->priv->dbus_component = ekiga_dbus_component_new (app);
#endif

  boost::shared_ptr<Ekiga::Settings> general_settings (new Ekiga::Settings (GENERAL_SCHEMA));
  const int schema_version = MAJOR_VERSION * 1000 + MINOR_VERSION * 10 + BUILD_NUMBER;
  if (general_settings->get_int ("version") < schema_version) {
    gm_application_show_assistant_window (app);
    general_settings->set_int ("version", schema_version);
  }

  core->close ();
  g_application_run (G_APPLICATION (app), argc, argv);

  g_object_unref (app);
}


/* GObject stuff */
static void
gm_application_activate (GApplication *self)
{
  GmApplication *app = GM_APPLICATION (self);

  gm_application_show_main_window (app);
}

static void
gm_application_startup (GApplication *app)
{
  GmApplication *self = GM_APPLICATION (app);

  GtkBuilder *builder = NULL;
  GMenuModel *app_menu = NULL;

  G_APPLICATION_CLASS (gm_application_parent_class)->startup (app);

  const gchar *menu =
    "<?xml version=\"1.0\"?>"
    "<interface>"
    "  <menu id=\"appmenu\">"
    "    <section>"
    "      <item>"
    "        <attribute name=\"label\" translatable=\"yes\">Address _Book</attribute>"
    "        <attribute name=\"action\">app.addressbook</attribute>"
    "      </item>"
    "      <item>"
    "        <attribute name=\"label\" translatable=\"yes\">_Accounts</attribute>"
    "        <attribute name=\"action\">app.accounts</attribute>"
    "      </item>"
    "    </section>"
    "    <section>"
    "      <item>"
    "        <attribute name=\"label\" translatable=\"yes\">_Preferences</attribute>"
    "        <attribute name=\"action\">app.preferences</attribute>"
    "      </item>"
    "      <item>"
    "        <attribute name=\"label\" translatable=\"yes\">Configuration _Assistant</attribute>"
    "        <attribute name=\"action\">app.assistant</attribute>"
    "      </item>"
    "    </section>"
    "    <section>"
    "      <item>"
    "        <attribute name=\"label\" translatable=\"yes\">_Help</attribute>"
    "        <attribute name=\"action\">app.help</attribute>"
    "        <attribute name=\"accel\">F1</attribute>"
    "      </item>"
    "      <item>"
    "        <attribute name=\"label\" translatable=\"yes\">_About</attribute>"
    "        <attribute name=\"action\">app.about</attribute>"
    "      </item>"
    "    </section>"
    "    <section>"
    "      <item>"
    "        <attribute name=\"label\" translatable=\"yes\">_Quit</attribute>"
    "        <attribute name=\"action\">app.quit</attribute>"
    "        <attribute name=\"accel\">&lt;Primary&gt;q</attribute>"
    "      </item>"
    "    </section>"
    "  </menu>"
    "</interface>";

  g_action_map_add_action_entries (G_ACTION_MAP (self),
                                   app_entries, G_N_ELEMENTS (app_entries),
                                   self);

  builder = gtk_builder_new ();
  gtk_builder_add_from_string (builder, menu, -1, NULL);
  app_menu = G_MENU_MODEL (gtk_builder_get_object (builder, "appmenu"));
  gtk_application_set_app_menu (GTK_APPLICATION (self), app_menu);
  g_object_unref (builder);
}


static void
gm_application_dispose (GObject *obj)
{
  GmApplication *self = NULL;

  self = GM_APPLICATION (obj);

#ifdef HAVE_DBUS
  g_object_unref (self->priv->dbus_component);
#endif


  G_OBJECT_CLASS (gm_application_parent_class)->dispose (obj);
}


static void
gm_application_shutdown (GApplication *app)
{
  GmApplication *self = GM_APPLICATION (app);

  g_return_if_fail (self);

  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GM_TYPE_APPLICATION, GmApplicationPrivate);
  PThread::Current()->Sleep (2000); // FIXME, This allows all threads to start and quit. Sucks.

  gtk_widget_hide (GTK_WIDGET (self->priv->main_window));

  self->priv->core.reset ();
  Ekiga::Runtime::quit ();

  G_APPLICATION_CLASS (gm_application_parent_class)->shutdown (app);
}

static gint
gm_application_command_line (GApplication *app,
                             GApplicationCommandLine *cl)
{
  gchar **arguments = NULL;
  GOptionContext *context = NULL;
  GError *error = NULL;

  GmApplication *self = GM_APPLICATION (app);

  g_return_val_if_fail (self && self->priv->core, -1);

  static gchar *url = NULL;
  static int debug_level = 0;
  static gboolean hangup = FALSE;
  static gboolean help = FALSE;
  static gboolean version = FALSE;

  static GOptionEntry options [] =
    {
        {
          "help", '?', 0, G_OPTION_ARG_NONE, &help,
          N_("Show the application's help"), NULL
        },

        /* Version */
        {
          "version", 'V', 0, G_OPTION_ARG_NONE, &version,
          N_("Show the application's version"), NULL
        },
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
          "hangup", '\0', 0, G_OPTION_ARG_NONE, &hangup,
          N_("Hangup the current call (if any)"),
          NULL
        },
        {
          NULL, 0, 0, (GOptionArg)0, NULL,
          NULL,
          NULL
        }
    };

  self->priv =
    G_TYPE_INSTANCE_GET_PRIVATE (self,
                                 GM_TYPE_APPLICATION,
                                 GmApplicationPrivate);

  context = g_option_context_new (NULL);
  g_option_context_add_main_entries (context, options, PACKAGE_NAME);
  g_option_context_add_group (context, gtk_get_option_group (FALSE));

  arguments = g_application_command_line_get_arguments (cl, NULL);

  /* Avoid exit() on the main instance */
  g_option_context_set_help_enabled (context, FALSE);

  if (!option_context_parse (context, arguments, &error)) {
    /* We should never get here since parsing would have
     * failed on the client side... */
    g_application_command_line_printerr (cl,
                                         _("%s\nRun '%s --help' to see a full list of available command line options.\n"),
                                         error->message, arguments[0]);

    g_error_free (error);
    g_application_command_line_set_exit_status (cl, 1);
    g_application_quit (app);
  }
  else if (url) {
    boost::shared_ptr<Ekiga::CallCore> call_core =
      self->priv->core->get<Ekiga::CallCore> ("call-core");
    call_core->dial (url);
  }
  else if (hangup) {
    boost::shared_ptr<Ekiga::CallCore> call_core =
      self->priv->core->get<Ekiga::CallCore> ("call-core");
    call_core->hang_up ();
  }
  else if (version) {
    g_print ("%s - Version %d.%d.%d\n",
             g_get_application_name (),
             MAJOR_VERSION, MINOR_VERSION, BUILD_NUMBER);
    g_application_quit (app);
  }
  else if (help) {
    g_print (g_option_context_get_help (context, TRUE, NULL));
    g_application_quit (app);
  }

  g_strfreev (arguments);
  g_option_context_free (context);

  g_application_activate (app);

  g_free (url);
  url = NULL;
  hangup = FALSE;
  version = FALSE;
  help = FALSE;

  return 0;
}

static void
gm_application_class_init (GmApplicationClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GApplicationClass *app_class = G_APPLICATION_CLASS (klass);

  object_class->dispose = gm_application_dispose;

  app_class->startup = gm_application_startup;
  app_class->activate = gm_application_activate;
  app_class->command_line = gm_application_command_line;
  app_class->shutdown = gm_application_shutdown;

  g_type_class_add_private (object_class, sizeof (GmApplicationPrivate));
}


static void
gm_application_init (GmApplication *self)
{
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GM_TYPE_APPLICATION, GmApplicationPrivate);
}


GmApplication *
gm_application_new ()
{
  GmApplication *self =
    GM_APPLICATION (g_object_new (GM_TYPE_APPLICATION,
                                  "application-id", "org.gnome.Ekiga",
                                  "flags", G_APPLICATION_HANDLES_COMMAND_LINE,
                                  NULL));
  g_application_register (G_APPLICATION (self), NULL, NULL);

  return self;
}


void
gm_application_set_core (GmApplication *self,
                         Ekiga::ServiceCorePtr core)
{
  g_return_if_fail (GM_IS_APPLICATION (self));
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GM_TYPE_APPLICATION, GmApplicationPrivate);

  self->priv->core = core;
}


Ekiga::ServiceCorePtr
gm_application_get_core (GmApplication *self)
{
  g_return_val_if_fail (GM_IS_APPLICATION (self), boost::shared_ptr<Ekiga::ServiceCore> ());
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GM_TYPE_APPLICATION, GmApplicationPrivate);

  return self->priv->core;
}


void
gm_application_show_main_window (GmApplication *self)
{
  g_return_if_fail (GM_IS_APPLICATION (self));
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GM_TYPE_APPLICATION, GmApplicationPrivate);

  gtk_window_present (GTK_WINDOW (self->priv->main_window));
}


void
gm_application_hide_main_window (GmApplication *self)
{
  g_return_if_fail (GM_IS_APPLICATION (self));
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GM_TYPE_APPLICATION, GmApplicationPrivate);

  gtk_widget_hide (self->priv->main_window);
}


GtkWidget *
gm_application_get_main_window (GmApplication *self)
{
  g_return_val_if_fail (GM_IS_APPLICATION (self), NULL);
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GM_TYPE_APPLICATION, GmApplicationPrivate);

  return self->priv->main_window;
}


gboolean
gm_application_show_help (GmApplication *app,
                          G_GNUC_UNUSED const gchar *link_id)
{
  g_return_val_if_fail (GM_IS_APPLICATION (app), FALSE);

  GtkWindow *parent = gtk_application_get_active_window (GTK_APPLICATION (app));

#ifdef WIN32
  gchar *locale, *loc_ , *index_path;
  int hinst = 0;

  locale = g_win32_getlocale ();
  if (strlen (locale) > 0) {

    /* try returned locale first, it may be fully qualified e.g. zh_CN */
    index_path = g_build_filename (WIN32_HELP_DIR, locale,
				   WIN32_HELP_FILE, NULL);
    hinst = (int) ShellExecute (NULL, "open", index_path, NULL,
			  	DATA_DIR, SW_SHOWNORMAL);
    g_free (index_path);
  }

  if (hinst <= 32 && (loc_ = g_strrstr (locale, "_"))) {
    /* on error, try short locale */
    *loc_ = 0;
    index_path = g_build_filename (WIN32_HELP_DIR, locale,
				   WIN32_HELP_FILE, NULL);
    hinst = (int) ShellExecute (NULL, "open", index_path, NULL,
				DATA_DIR, SW_SHOWNORMAL);
    g_free (index_path);
  }

  g_free (locale);

  if (hinst <= 32) {

    /* on error or missing locale, try default locale */
    index_path = g_build_filename (WIN32_HELP_DIR, "C", WIN32_HELP_FILE, NULL);
    (void)ShellExecute (NULL, "open", index_path, NULL,
			DATA_DIR, SW_SHOWNORMAL);
    g_free (index_path);
  }
#else /* !WIN32 */
  GError *err = NULL;
  gboolean success = FALSE;

  success = gtk_show_uri (NULL, "ghelp:" PACKAGE_NAME, GDK_CURRENT_TIME, &err);

  if (!success) {
    GtkWidget *d;
    d = gtk_message_dialog_new (NULL,
                                (GtkDialogFlags) (GTK_DIALOG_MODAL),
                                GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
                                "%s", _("Unable to open help file."));
    gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (d),
                                              "%s", err->message);
    g_signal_connect (d, "response", G_CALLBACK (gtk_widget_destroy), NULL);
    gtk_window_set_transient_for (GTK_WINDOW (d), GTK_WINDOW (parent));
    gtk_window_present (GTK_WINDOW (d));
    g_error_free (err);
    return FALSE;
  }
#endif

  return TRUE;
}


void
gm_application_show_about (GmApplication *app)
{
  g_return_if_fail (GM_IS_APPLICATION (app));

  GtkWidget *pixmap = NULL;
  gchar *filename = NULL;

  const gchar *authors [] = {
      "Damien Sandras <dsandras@seconix.com>",
      "",
      N_("Contributors:"),
      "Eugen Dedu <eugen.dedu@pu-pm.univ-fcomte.fr>",
      "Julien Puydt <julien.puydt@laposte.net>",
      "Robert Jongbloed <rjongbloed@postincrement.com>",
      "",
      N_("Artwork:"),
      "Fabian Deutsch <fabian.deutsch@gmx.de>",
      "Vinicius Depizzol <vdepizzol@gmail.com>",
      "Andreas Kwiatkowski <post@kwiat.org>",
      "Carlos Pardo <me@m4de.com>",
      "Jakub Steiner <jimmac@ximian.com>",
      "",
      N_("See AUTHORS file for full credits"),
      NULL
  };

  authors [2] = gettext (authors [2]);
  authors [7] = gettext (authors [7]);
  authors [14] = gettext (authors [14]);

  const gchar *documenters [] = {
    "Damien Sandras <dsandras@seconix.com>",
    "Christopher Warner <zanee@kernelcode.com>",
    "Matthias Redlich <m-redlich@t-online.de>",
    NULL
  };

  const gchar *license[] = {
N_("This program is free software; you can redistribute it and/or modify \
it under the terms of the GNU General Public License as published by \
the Free Software Foundation; either version 2 of the License, or \
(at your option) any later version. "),
N_("This program is distributed in the hope that it will be useful, \
but WITHOUT ANY WARRANTY; without even the implied warranty of \
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the \
GNU General Public License for more details. \
You should have received a copy of the GNU General Public License \
along with this program; if not, write to the Free Software Foundation, \
Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA."),
N_("Ekiga is licensed under the GPL license and as a special exception, \
you have permission to link or otherwise combine this program with the \
programs OPAL, OpenH323 and PWLIB, and distribute the combination, \
without applying the requirements of the GNU GPL to the OPAL, OpenH323 \
and PWLIB programs, as long as you do follow the requirements of the \
GNU GPL for all the rest of the software thus combined.")
  };

  gchar *license_trans;

  /* Translators: Please write translator credits here, and
   * separate names with \n */
  const gchar *translator_credits = _("translator-credits");
  if (g_strcmp0 (translator_credits, "translator-credits") == 0)
    translator_credits = "No translators, English by\n"
        "Damien Sandras <dsandras@seconix.com>";

  const gchar *comments =  _("Ekiga is full-featured SIP and H.323 compatible VoIP, IP-Telephony and Videoconferencing application that allows you to make audio and video calls to remote users with SIP and H.323 hardware or software.");

  license_trans = g_strconcat (_(license[0]), "\n\n", _(license[1]), "\n\n",
                               _(license[2]), "\n\n", NULL);

  filename = g_build_filename (DATA_DIR, "pixmaps", PACKAGE_NAME,
                               PACKAGE_NAME "-logo.png", NULL);
  pixmap =  gtk_image_new_from_file (filename);

  gtk_show_about_dialog (GTK_WINDOW (gtk_application_get_active_window (GTK_APPLICATION (app))),
                         "name", "Ekiga",
                         "version", VERSION,
                         "copyright", "Copyright © 2000-2014 Damien Sandras",
                         "authors", authors,
                         "documenters", documenters,
                         "translator-credits", translator_credits,
                         "comments", comments,
                         "logo", gtk_image_get_pixbuf (GTK_IMAGE (pixmap)),
                         "license", license_trans,
                         "wrap-license", TRUE,
                         "website", "http://www.ekiga.org",
                         NULL);

  g_free (license_trans);
  g_free (filename);
}


GtkWidget *
gm_application_show_call_window (GmApplication *self)
{
  GtkWidget *call_window = NULL;
  g_return_val_if_fail (GM_IS_APPLICATION (self), NULL);

  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GM_TYPE_APPLICATION, GmApplicationPrivate);

  call_window = call_window_new (self);
  gtk_window_present (GTK_WINDOW (call_window));

  return call_window;
}


void
gm_application_show_chat_window (GmApplication *self)
{
  g_return_if_fail (GM_IS_APPLICATION (self));

  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GM_TYPE_APPLICATION, GmApplicationPrivate);

  // FIXME: We should move the chat window to a build & destroy scheme
  // but unread-alert prevents this
  gtk_window_present (GTK_WINDOW (self->priv->chat_window));
}


GtkWidget *
gm_application_get_chat_window (GmApplication *self)
{
  g_return_val_if_fail (GM_IS_APPLICATION (self), NULL);

  return self->priv->chat_window;
}


void
gm_application_show_preferences_window (GmApplication *self)
{
  GtkWindow *parent = NULL;
  GtkWindow *window = NULL;

  g_return_if_fail (GM_IS_APPLICATION (self));

  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GM_TYPE_APPLICATION, GmApplicationPrivate);
  parent = gtk_application_get_active_window (GTK_APPLICATION (self));

  window = GTK_WINDOW (preferences_window_new (self));
  gtk_window_set_transient_for (GTK_WINDOW (window), GTK_WINDOW (parent));
  gtk_dialog_run (GTK_DIALOG (window));
}


void
gm_application_show_addressbook_window (GmApplication *self)
{
  g_return_if_fail (GM_IS_APPLICATION (self));

  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GM_TYPE_APPLICATION, GmApplicationPrivate);

  gtk_window_present (GTK_WINDOW (addressbook_window_new (self)));
}


void
gm_application_show_accounts_window (GmApplication *self)
{
  g_return_if_fail (GM_IS_APPLICATION (self));

  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GM_TYPE_APPLICATION, GmApplicationPrivate);

  gtk_window_present (GTK_WINDOW (accounts_window_new (self)));
}


void
gm_application_show_assistant_window (GmApplication *self)
{
  GtkWindow *parent = NULL;
  GtkWindow *window = NULL;

  g_return_if_fail (GM_IS_APPLICATION (self));

  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GM_TYPE_APPLICATION, GmApplicationPrivate);
  parent = gtk_application_get_active_window (GTK_APPLICATION (self));

  window = GTK_WINDOW (assistant_window_new (self));
  gtk_window_set_transient_for (GTK_WINDOW (window), GTK_WINDOW (parent));
  gtk_window_present (window);
}
