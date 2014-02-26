
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2009 Damien Sandras <dsandras@seconix.com>
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
 *                         chat-window.h  -  description
 *                         -----------------------------
 *   begin                : written in july 2008 by Julien Puydt
 *   copyright            : (C) 2008 by Julien Puydt
 *   description          : Implementation of a window to display chats
 *
 */

#include <gdk/gdkkeysyms.h>
#include <glib/gi18n.h>

#include "chat-core.h"
#include "notification-core.h"

#include "form-dialog-gtk.h"
#include "scoped-connections.h"

#include "chat-window.h"

#include "conversation-page.h"

struct _ChatWindowPrivate
{
  boost::shared_ptr<Ekiga::NotificationCore> notification_core;
  Ekiga::scoped_connections connections;

  GtkWidget* notebook;
};

enum {
  UNREAD_COUNT,
  UNREAD_ALERT,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (ChatWindow, chat_window, GM_TYPE_WINDOW);

/* signal callbacks (declarations) */

static bool on_handle_questions (ChatWindow* self,
				 Ekiga::FormRequestPtr request);

static void on_close_button_clicked (GtkButton* button,
				     gpointer data);

static void on_escaped (GtkWidget *widget,
                        gpointer data);

static void on_switch_page (GtkNotebook* notebook,
			    gpointer page_,
			    guint num,
			    gpointer data);

static bool on_dialect_added (ChatWindow* self,
			      Ekiga::DialectPtr dialect);
static bool on_conversation_added (ChatWindow* self,
				   Ekiga::ConversationPtr conversation);
static void on_some_conversation_user_requested (ChatWindow* self,
						 GtkWidget* page);

static void show_chat_window_cb (ChatWindow *self);

/* helper (implementation) */

static void
on_updated (G_GNUC_UNUSED ConversationPage* page_,
	    gpointer data)
{
  ChatWindow* self = (ChatWindow*)data;
  guint unread_count = 0;

  for (gint ii = 0;
       ii < gtk_notebook_get_n_pages (GTK_NOTEBOOK (self->priv->notebook)) ;
       ii++) {

    GtkWidget* page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (self->priv->notebook), ii);
    guint page_unread_count = conversation_page_get_unread_count (page);
    GtkWidget* hbox = gtk_notebook_get_tab_label (GTK_NOTEBOOK (self->priv->notebook), page);
    GtkWidget* label = GTK_WIDGET (g_object_get_data (G_OBJECT (hbox), "label-widget"));
    const gchar* base_title = conversation_page_get_title (page);

    unread_count = unread_count + page_unread_count;

    if (page_unread_count > 0) {

      gchar* title = g_strdup_printf ("[%d] %s", page_unread_count, base_title);
      gtk_label_set_text (GTK_LABEL (label), title);
      g_free (title);
    } else {

      gtk_label_set_text (GTK_LABEL (label), base_title);
    }
  }

  g_signal_emit (self, signals[UNREAD_COUNT], 0, unread_count);
  g_signal_emit (self, signals[UNREAD_ALERT], 0, NULL);

  if (unread_count > 0) {
    gchar* info = g_strdup_printf (ngettext ("You have %d unread text message",
					     "You have %d unread text messages",
					     unread_count), unread_count);
    boost::shared_ptr<Ekiga::Notification> notif (new Ekiga::Notification (Ekiga::Notification::Warning, info, "", _("Read"), boost::bind (show_chat_window_cb, self)));
    self->priv->notification_core->push_notification (notif);
    g_free (info);
  }
}

/* signal callbacks (implementations) */

static bool on_handle_questions (ChatWindow* self,
				 Ekiga::FormRequestPtr request)
{
  GtkWidget *parent = gtk_widget_get_toplevel (GTK_WIDGET (self));
  FormDialog dialog (request, parent);

  dialog.run ();

  return true;
}

static void
on_close_button_clicked (GtkButton* button,
			 gpointer data)
{
  ChatWindow* self = (ChatWindow*)data;
  GtkWidget* page = NULL;
  gint num = 0;

  page = (GtkWidget*)g_object_get_data (G_OBJECT (button), "page-widget");
  num = gtk_notebook_page_num (GTK_NOTEBOOK (self->priv->notebook), page);

  /* FIXME: we add a page when the conversation is added ; ok. But if
   * we get rid of the page here, then if the same conversation is
   * still alive and kicking, we have no way to add a corresponding
   * page again. So there's something lacking in our API: how does one
   * close an Ekiga::Conversation?
   *
   */
  g_print ("FIXME %s\n", __PRETTY_FUNCTION__);
  num = num + 1; // FIXME: just to stop a warning
  //  gtk_notebook_remove_page (GTK_NOTEBOOK (self->priv->notebook), num);

  if (gtk_notebook_get_n_pages (GTK_NOTEBOOK (self->priv->notebook)) == 0)
    gtk_widget_hide (GTK_WIDGET (self));
}

static void
on_escaped (GtkWidget */*widget*/,
            gpointer data)
{
  ChatWindow* self = (ChatWindow*)data;
  gint num = 0;

  num = gtk_notebook_get_current_page (GTK_NOTEBOOK (self->priv->notebook));
  gtk_notebook_remove_page (GTK_NOTEBOOK (self->priv->notebook), num);

  if (gtk_notebook_get_n_pages (GTK_NOTEBOOK (self->priv->notebook)) == 0)
    gtk_widget_hide (GTK_WIDGET (self));
}

static void
on_switch_page (G_GNUC_UNUSED GtkNotebook* notebook,
		G_GNUC_UNUSED gpointer page_,
		guint num,
		gpointer data)
{
  ChatWindow* self = (ChatWindow*)data;
  GtkWidget* page = NULL;

  page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (self->priv->notebook), num);

  gtk_widget_grab_focus (page);
}

static bool
on_dialect_added (ChatWindow* self,
		  Ekiga::DialectPtr dialect)
{
  self->priv->connections.add (dialect->conversation_added.connect (boost::bind(&on_conversation_added, self, _1)));

  dialect->visit_conversations (boost::bind (&on_conversation_added, self, _1));

  return true;
}

static bool
on_conversation_added (ChatWindow* self,
		       Ekiga::ConversationPtr conversation)
{
  GtkWidget* page = NULL;
  GtkWidget* hbox = NULL;
  GtkWidget* label = NULL;
  GtkWidget* close_button = NULL;
  GtkWidget* close_image = NULL;
  const std::string title = conversation->get_title ();

  page = conversation_page_new (conversation);
  g_signal_connect (page, "updated",
		    G_CALLBACK (on_updated), self);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);

  label = gtk_label_new (title.c_str ());
  g_object_set_data (G_OBJECT (hbox), "label-widget", label);

  close_button = gtk_button_new ();
  gtk_widget_set_size_request (close_button, 16, 16); // FIXME: hardcoded!?
  gtk_button_set_relief (GTK_BUTTON (close_button), GTK_RELIEF_NONE);
  gtk_button_set_focus_on_click (GTK_BUTTON (close_button), FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (close_button), 0);
  g_object_set_data (G_OBJECT (close_button), "page-widget", page);
  g_signal_connect (close_button, "clicked",
		    G_CALLBACK (on_close_button_clicked), self);

  close_image = gtk_image_new_from_icon_name ("gtk-close",
					      GTK_ICON_SIZE_MENU);
  gtk_widget_set_size_request (close_image, 12, 12); // FIXME hardcoded!?
  gtk_container_add (GTK_CONTAINER (close_button), close_image);

  gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 2);
  gtk_box_pack_start (GTK_BOX (hbox), close_button, FALSE, FALSE, 2);
  gtk_widget_show_all (hbox);

  gtk_notebook_append_page (GTK_NOTEBOOK (self->priv->notebook),
			    page, hbox);
  gtk_widget_show_all (page);

  self->priv->connections.add (conversation->user_requested.connect (boost::bind (&on_some_conversation_user_requested, self, page)));

  return true;
}

static void
on_some_conversation_user_requested (ChatWindow* self,
				     GtkWidget* page)
{
  gint num;

  num = gtk_notebook_page_num (GTK_NOTEBOOK (self->priv->notebook), page);
  gtk_notebook_set_current_page (GTK_NOTEBOOK (self->priv->notebook), num);
  gtk_widget_show (GTK_WIDGET (self));
  gtk_window_present (GTK_WINDOW (self));
}

static void
show_chat_window_cb (ChatWindow *self)
{
  gtk_widget_show (GTK_WIDGET (self));
  gtk_window_present (GTK_WINDOW (self));
}


/* GObject code */

static void
chat_window_finalize (GObject* obj)
{
  ChatWindow* self = NULL;

  self = CHAT_WINDOW (obj);

  delete self->priv;
  self->priv = NULL;

  G_OBJECT_CLASS (chat_window_parent_class)->finalize (obj);
}

static void
chat_window_class_init (ChatWindowClass* klass)
{
  GObjectClass* gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = chat_window_finalize;

  signals[UNREAD_COUNT] =
    g_signal_new ("unread-count",
		  G_OBJECT_CLASS_TYPE (gobject_class),
		  G_SIGNAL_RUN_LAST,
		  G_STRUCT_OFFSET (ChatWindowClass, unread_count),
		  NULL, NULL,
		  g_cclosure_marshal_VOID__UINT,
		  G_TYPE_NONE, 1,
		  G_TYPE_UINT);

  signals[UNREAD_ALERT] =
    g_signal_new ("unread-alert",
		  G_OBJECT_CLASS_TYPE (gobject_class),
		  G_SIGNAL_RUN_LAST,
		  G_STRUCT_OFFSET (ChatWindowClass, unread_alert),
		  NULL, NULL,
		  g_cclosure_marshal_VOID__VOID,
		  G_TYPE_NONE, 0);
}

static void
chat_window_init (ChatWindow* self)
{
  /* we can't do much here since we get the Chat as reference... */
  gtk_window_set_title (GTK_WINDOW (self), _("Chat Window"));
}

/* public api */

GtkWidget*
chat_window_new (Ekiga::ServiceCore& core,
		 const char* key)
{
  ChatWindow* self = NULL;
  GtkAccelGroup *accel = NULL;

  self = (ChatWindow*)g_object_new (CHAT_WINDOW_TYPE,
				    "key", key,
                                    "hide_on_esc", FALSE,
				    NULL);

  self->priv = new ChatWindowPrivate;

  self->priv->notification_core =
    core.get<Ekiga::NotificationCore>("notification-core");

  self->priv->notebook = gtk_notebook_new ();
  gtk_container_add (GTK_CONTAINER (self), self->priv->notebook);
  gtk_widget_show (self->priv->notebook);

  accel = gtk_accel_group_new ();
  gtk_window_add_accel_group (GTK_WINDOW (self), accel);
  gtk_accel_group_connect (accel, GDK_KEY_Escape, (GdkModifierType) 0, GTK_ACCEL_LOCKED,
                           g_cclosure_new_swap (G_CALLBACK (on_escaped), (gpointer) self, NULL));
  g_object_unref (accel);

  g_signal_connect (self->priv->notebook, "switch-page",
		    G_CALLBACK (on_switch_page), self);

  boost::shared_ptr<Ekiga::ChatCore> chat_core =
    core.get<Ekiga::ChatCore> ("chat-core");
  self->priv->connections.add (chat_core->dialect_added.connect (boost::bind (&on_dialect_added, self, _1)));
  self->priv->connections.add (chat_core->questions.connect (boost::bind (&on_handle_questions, self, _1)));
  chat_core->visit_dialects (boost::bind (&on_dialect_added, self, _1));

  return (GtkWidget*)self;
}
