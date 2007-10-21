
/* Ekiga -- A VoIP and Video-Conferencing application
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
 *                         statusicon.cpp  -  description
 *                         --------------------------
 *   begin                : Thu Jan 12 2006
 *   copyright            : (C) 2000-2007 by Damien Sandras
 *   description          : High level tray api implementation
 */


#include <gdk/gdkkeysyms.h>

#include "statusicon.h"

#include "gmstockicons.h"
#include "gmmenuaddon.h"

#include "callbacks.h" // FIXME SHOULD GET RID OF THIS
#include "misc.h" // FIXME same here
#include "ekiga.h"

#include "gtk-frontend.h"

#include <sigc++/sigc++.h>
#include <vector>

#ifdef HAVE_GNOME
#undef _
#undef N_
#include <gnome.h>
#endif

#include "callinfo.h"
#include "services.h"
#include "gtk-frontend.h"


/*
 * The StatusIcon
 */
struct _StatusIconPrivate 
{
  _StatusIconPrivate (Ekiga::ServiceCore & _core) : core (_core) { }

  GtkWidget *popup_menu;
  gboolean has_message;

  std::vector<sigc::connection> connections;

  gchar *key;
  int blink_id;
  int status;
  bool unread_messages;
  bool blinking;

  gchar *blink_image;

  Ekiga::ServiceCore & core;
};

enum { STATUSICON_KEY = 1 };

static GObjectClass *parent_class = NULL;


/* 
 * Declaration of Callbacks 
 */
static void
show_popup_menu_cb (GtkStatusIcon *icon,
                    guint button,
                    guint activate_time,
                    gpointer data);

static void
statusicon_activated_cb (GtkStatusIcon *icon,
                         gpointer data);

static void 
message_event_cb (GtkWidget *widget,
                  guint messages,
                  gpointer data);

static gboolean
statusicon_blink_cb (gpointer data);

static void
statusicon_key_updated_cb (gpointer id, 
                           GmConfEntry *entry,
                           gpointer data);

static void
on_call_event_cb (GMManager::CallingState i,
                  Ekiga::CallInfo & info,
                  gpointer data);


/*
 * Declaration of local functions
 */
static GtkWidget *
statusicon_build_menu ();

static void
statusicon_start_blinking (StatusIcon *icon,
                           const char *stock_id);

static void
statusicon_stop_blinking (StatusIcon *icon);

static void
statusicon_set_status (StatusIcon *widget,
                       guint status);


/* 
 * GObject stuff
 */
static void
statusicon_dispose (GObject *obj)
{
  StatusIcon *icon = NULL;

  icon = STATUSICON (obj);

  icon->priv->blink_image = NULL;
  icon->priv->key = NULL;

  parent_class->dispose (obj);
}


static void
statusicon_finalize (GObject *obj)
{
  StatusIcon *self = NULL;

  self = STATUSICON (obj);

  if (self->priv->blink_image) 
    g_free (self->priv->blink_image);

  if (self->priv->key)
    g_free (self->priv->key);

  for (std::vector<sigc::connection>::iterator iter = self->priv->connections.begin () ;
       iter != self->priv->connections.end ();
       iter++)
    iter->disconnect ();
  
  parent_class->finalize (obj);
}


static void
statusicon_get_property (GObject *obj,
                         guint prop_id,
                         GValue *value,
                         GParamSpec *spec)
{
  StatusIcon *self = NULL;

  self = STATUSICON (self);

  switch (prop_id) {

  case STATUSICON_KEY:
    g_value_set_string (value, self->priv->key);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, spec);
    break;
  }
}


static void
statusicon_set_property (GObject *obj,
                         guint prop_id,
                         const GValue *value,
                         GParamSpec *spec)
{
  StatusIcon *self = NULL;
  const gchar *str = NULL;

  self = STATUSICON (obj);

  switch (prop_id) {

  case STATUSICON_KEY:
    g_free ((gchar *) self->priv->key);
    str = g_value_get_string (value);
    self->priv->key = g_strdup (str ? str : "");
    if (str) {
      gm_conf_notifier_add (str, statusicon_key_updated_cb, self);
      statusicon_set_status (self, gm_conf_get_int (str));
    }
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, spec);
    break;
  }
}


static void
statusicon_class_init (gpointer g_class,
                       gpointer class_data)
{
  GObjectClass *gobject_class = NULL;
  GParamSpec *spec = NULL;

  parent_class = (GObjectClass *) g_type_class_peek_parent (g_class);

  gobject_class = (GObjectClass *) g_class;
  gobject_class->dispose = statusicon_dispose;
  gobject_class->finalize = statusicon_finalize;
  gobject_class->get_property = statusicon_get_property;
  gobject_class->set_property = statusicon_set_property;

  spec = g_param_spec_string ("key", "Key", "Key", 
                              NULL, (GParamFlags) G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, STATUSICON_KEY, spec); 
}


GType
statusicon_get_type ()
{
  static GType result = 0;

  if (result == 0) {

    static const GTypeInfo info = {
      sizeof (StatusIconClass),
      NULL,
      NULL,
      statusicon_class_init,
      NULL,
      NULL,
      sizeof (StatusIcon),
      0,
      NULL,
      NULL
    };

    result = g_type_register_static (GTK_TYPE_STATUS_ICON,
				     "StatusIconType",
				     &info, (GTypeFlags) 0);
  }

  return result;
}


/*
 * Our own stuff
 */

/*
 * Callbacks
 */
static void
show_popup_menu_cb (GtkStatusIcon *icon,
                    guint button,
                    guint activate_time,
                    gpointer data)
{
  GtkWidget *popup = NULL;

  popup = GTK_WIDGET (data);

  gtk_menu_popup (GTK_MENU (popup),
                  NULL, NULL, NULL, NULL,
                  button, activate_time);
}


static void
statusicon_activated_cb (GtkStatusIcon *icon,
                         gpointer data)
{
  StatusIcon *self = STATUSICON (data);

  GtkWidget *window = NULL;
  
  if (!self->priv->unread_messages) {
  
    window = GnomeMeeting::Process ()->GetMainWindow (); //FIXME

    // FIXME when the main window becomes a gobject
    if (!gnomemeeting_window_is_visible (window))
      gnomemeeting_window_show (window);
    else
      gnomemeeting_window_hide (window);
  }
  else {

    GtkFrontend *frontend = dynamic_cast<GtkFrontend*>(self->priv->core.get ("gtk-frontend"));
    GtkWidget *w = GTK_WIDGET (frontend->get_chat_window ());

    gtk_widget_show (w);
    gtk_window_present (GTK_WINDOW (w));
  }
}


static void 
message_event_cb (GtkWidget *widget,
                  guint messages,
                  gpointer data)
{
  StatusIcon *self = STATUSICON (data);

  gchar *msg1 = NULL;
  gchar *msg2 = NULL;
  char *message = NULL;

  if (messages > 0) 
    statusicon_start_blinking (self, GM_STOCK_MESSAGE);
  else 
    statusicon_stop_blinking (self);

  if (messages > 0) {

    msg1 = g_strdup_printf (_("You have %d messages"), messages);
    msg2 = g_strdup_printf (_("You have %d message"), messages);
    message = ngettext (msg2, msg1, messages);
    
    gtk_status_icon_set_tooltip (GTK_STATUS_ICON (self), message);

    g_free (msg1);
    g_free (msg2);
  }
  else {

    gtk_status_icon_set_tooltip (GTK_STATUS_ICON (self), NULL);
  }

  self->priv->unread_messages = (messages > 0);
}


static gboolean
statusicon_blink_cb (gpointer data)
{
  StatusIcon *statusicon = STATUSICON (data);

  gdk_threads_enter ();
  if (statusicon->priv->blinking) 
    gtk_status_icon_set_from_stock (GTK_STATUS_ICON (statusicon), statusicon->priv->blink_image);
  else  
    statusicon_set_status (statusicon, statusicon->priv->status);
  gdk_threads_leave ();

  statusicon->priv->blinking = !statusicon->priv->blinking;

  return true;
}


static void
statusicon_key_updated_cb (gpointer id, 
                           GmConfEntry *entry,
                           gpointer data)
{
  guint status = CONTACT_ONLINE;

  g_return_if_fail (data != NULL);

  if (gm_conf_entry_get_type (entry) == GM_CONF_INT) {

    gdk_threads_enter ();
    status = gm_conf_entry_get_int (entry);
    statusicon_set_status (STATUSICON (data), status);
    gdk_threads_leave ();
  }
}


static void
on_call_event_cb (GMManager::CallingState i,
                  Ekiga::CallInfo & info,
                  gpointer data)
{
  StatusIcon *statusicon = STATUSICON (data);

  if (i == GMManager::Called) 
    statusicon_start_blinking (statusicon, GM_STOCK_STATUS_RINGING);
  else 
    statusicon_stop_blinking (statusicon);
}


/* 
 * Local functions
 */
static GtkWidget *
statusicon_build_menu ()
{
  GtkWidget *main_window = NULL;

  Ekiga::ServiceCore *services = NULL;
  GtkFrontend *gtk_frontend = NULL;

  guint status = 0;

  services = GnomeMeeting::Process ()->GetServiceCore ();
  gtk_frontend = dynamic_cast<GtkFrontend *>(services->get ("gtk-frontend"));
  main_window = GnomeMeeting::Process ()->GetMainWindow ();

  status = gm_conf_get_int (PERSONAL_DATA_KEY "status");

  static MenuEntry menu [] =
    {
      GTK_MENU_RADIO_ENTRY("online", _("_Online"), NULL,
			   NULL, 0,
			   GTK_SIGNAL_FUNC (radio_menu_changed_cb),
			   (gpointer) PERSONAL_DATA_KEY "status",
			   (status == CONTACT_ONLINE), TRUE),

      GTK_MENU_RADIO_ENTRY("away", _("_Away"), NULL,
			   NULL, 0,
			   GTK_SIGNAL_FUNC (radio_menu_changed_cb),
			   (gpointer) PERSONAL_DATA_KEY "status",
			   (status == CONTACT_AWAY), TRUE),

      GTK_MENU_RADIO_ENTRY("dnd", _("Do Not _Disturb"), NULL,
			   NULL, 0,
			   GTK_SIGNAL_FUNC (radio_menu_changed_cb),
			   (gpointer) PERSONAL_DATA_KEY "status",
			   (status == CONTACT_DND), TRUE),

      GTK_MENU_RADIO_ENTRY("free_for_chat", _("_Free For Chat"), NULL,
			   NULL, 0,
			   GTK_SIGNAL_FUNC (radio_menu_changed_cb),
			   (gpointer) PERSONAL_DATA_KEY "status",
			   (status == CONTACT_FREEFORCHAT), TRUE),
      
      GTK_MENU_RADIO_ENTRY("invisible", _("_Invisible"), NULL,
			   NULL, 0,
			   GTK_SIGNAL_FUNC (radio_menu_changed_cb),
			   (gpointer) PERSONAL_DATA_KEY "status",
			   (status == CONTACT_INVISIBLE), TRUE),

      GTK_MENU_SEPARATOR,

#ifdef HAVE_GNOME
      GTK_MENU_ENTRY("help", NULL,
                     _("Get help by reading the Ekiga manual"),
                     GTK_STOCK_HELP, GDK_F1,
                     GTK_SIGNAL_FUNC (help_cb), NULL, TRUE),

      GTK_MENU_ENTRY("about", NULL,
		     _("View information about Ekiga"),
		     GNOME_STOCK_ABOUT, 'a',
		     GTK_SIGNAL_FUNC (about_callback), (gpointer) main_window,
		     TRUE),
#else
      GTK_MENU_ENTRY("help", _("_Contents"),
                     _("Get help by reading the Ekiga manual"),
                     GTK_STOCK_HELP, GDK_F1,
                     NULL, NULL, FALSE),

      GTK_MENU_ENTRY("about", _("_About"),
		     _("View information about Ekiga"),
		     NULL, 'a',
		     GTK_SIGNAL_FUNC (about_callback), (gpointer) main_window,
		     TRUE),
#endif
      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("quit", NULL, _("Quit"),
		     GTK_STOCK_QUIT, 'Q',
		     GTK_SIGNAL_FUNC (quit_callback),
		     main_window, TRUE),

      GTK_MENU_END
    };

  return GTK_WIDGET (gtk_build_popup_menu (NULL, menu, NULL));
}


static void
statusicon_start_blinking (StatusIcon *icon,
                           const char *stock_id)
{
  icon->priv->blink_image = g_strdup (stock_id);
  if (icon->priv->blink_id == -1)
    icon->priv->blink_id = g_timeout_add (1000, statusicon_blink_cb, icon);
}


static void
statusicon_stop_blinking (StatusIcon *icon)
{
  if (icon->priv->blink_image) {

    g_free (icon->priv->blink_image);
    icon->priv->blink_image = NULL;
  }

  if (icon->priv->blink_id != -1) {

    g_source_remove (icon->priv->blink_id);
    icon->priv->blink_id = -1;
    icon->priv->blinking = false;
  }

  statusicon_set_status (STATUSICON (icon), 
                         gm_conf_get_int (icon->priv->key));
}


void
statusicon_set_status (StatusIcon *statusicon,
                       guint status)
{
  GtkWidget *menu = NULL;

  g_return_if_fail (statusicon != NULL);

  /* Update the menu */
  menu = gtk_menu_get_widget (statusicon->priv->popup_menu, "online");
  gtk_radio_menu_select_with_widget (GTK_WIDGET (menu), status);

  /* Update the status icon */
  switch (status) {

  case CONTACT_ONLINE:
    gtk_status_icon_set_from_stock (GTK_STATUS_ICON (statusicon), GM_STOCK_STATUS_ONLINE);
    break;

  case (CONTACT_AWAY):
    gtk_status_icon_set_from_stock (GTK_STATUS_ICON (statusicon), GM_STOCK_STATUS_AWAY);
    break;

  case (CONTACT_DND):
    gtk_status_icon_set_from_stock (GTK_STATUS_ICON (statusicon), GM_STOCK_STATUS_DND);
    break;

  case (CONTACT_FREEFORCHAT):
    gtk_status_icon_set_from_stock (GTK_STATUS_ICON (statusicon), GM_STOCK_STATUS_FREEFORCHAT);
    break;

  case (CONTACT_INVISIBLE):
    gtk_status_icon_set_from_stock (GTK_STATUS_ICON (statusicon), GM_STOCK_STATUS_OFFLINE);
    break;

  default:
    break;
  }

  statusicon->priv->status = status;
}


/*
 * Public API
 */
StatusIcon *
statusicon_new (Ekiga::ServiceCore & core,
                const char *key)
{
  StatusIcon *self = NULL;
  sigc::connection conn;

  self = STATUSICON (g_object_new (STATUSICON_TYPE, NULL));
  self->priv = new StatusIconPrivate (core);

  self->priv->popup_menu = statusicon_build_menu ();
  self->priv->has_message = FALSE;
  self->priv->blink_id = -1;
  self->priv->blinking = false;
  self->priv->blink_image = NULL;
  self->priv->unread_messages = false;
  self->priv->key = g_strdup ("");

  g_object_set (self, "key", key, NULL);

  // FIXME GnomeMeeting::Process should disappear
  conn = GnomeMeeting::Process ()->GetManager ()->call_event.connect (sigc::bind (sigc::ptr_fun (on_call_event_cb), self));
  self->priv->connections.push_back (conn);

  GtkFrontend *frontend = dynamic_cast<GtkFrontend*>(core.get ("gtk-frontend"));
  GtkWidget *chat_window = GTK_WIDGET (frontend->get_chat_window ());

  g_signal_connect (self, "popup-menu",
                    G_CALLBACK (show_popup_menu_cb), self->priv->popup_menu);

  g_signal_connect (self, "activate",
                    G_CALLBACK (statusicon_activated_cb), self);

  g_signal_connect (chat_window, "message-event",
                    G_CALLBACK (message_event_cb), self);

  return self;
}
