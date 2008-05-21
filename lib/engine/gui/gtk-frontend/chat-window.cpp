
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2007 Damien Sandras
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
 *                         chat_window.cpp  -  description
 *                         -------------------------------
 *   begin                : Wed Jan 23 2002
 *   copyright            : (C) 2000-2007 by Damien Sandras
 *   description          : This file contains functions to build the chat
 *                          window. 
 *
 */


#include "config.h"

#include "chat-window.h"
#include "chat-window-page.h"

#include "chat-core.h"

#include "gmconf.h"
#include "gmtexttagaddon.h"
#include "gmtextbufferaddon.h"
#include "gmtextviewaddon.h"
#include "gmstockicons.h"

#include <gdk/gdkkeysyms.h>
#include <sigc++/sigc++.h>
#include <vector>
#include <iostream>

/*
 * The ChatWindow
 */
struct _ChatWindowPrivate 
{
  _ChatWindowPrivate (Ekiga::ServiceCore & _core) : core (_core) { }

  GtkWidget *notebook;

  std::vector<sigc::connection> connections;
  Ekiga::ServiceCore & core;
};

enum { CHAT_WINDOW_KEY = 1 };

static GObjectClass *parent_class = NULL;

/*
 * NOTICE:
 *
 * Added support for new signal "message-event". Objects like the
 * StatusIcon can listen to that signal in order to know if there
 * are unread messages. The signal is emitted when there is a change
 * in the number of unread messages, like when a message is received
 * or the focus changes.
 */

/*
 * GTK+ Callbacks
 */

/* DESCRIPTION  : Called when the close-event signal is emitted. It happens
 *                when the user clicks on the cross in a ChatWindowPage.
 * BEHAVIOR     : Remove the corresponding ChatWindowPage from the ChatWindow.
 * PRE          : The ChatWindowPage passed as first parameter,
 *                The ChatWindow as second argument.
 */
static void close_event_cb (GtkWidget *widget,
                            gpointer data);


/* DESCRIPTION  : Called when the ChatWindow is hidden.
 * BEHAVIOR     : Remove all ChatWindowPages.
 * PRE          : The ChatWindow as second argument.
 */
static void hide_event_cb (GtkWidget *widget,
                           gpointer data);


/* DESCRIPTION  : Called when the ChatWindow is being focused.
 * BEHAVIOR     : Emits the "message-event" signal with the new number
 *                of unseen messages as the current ChatWindowPage does
 *                not have unseen messages anymore. 
 * PRE          : The ChatWindow as last argument.
 */
static void focus_in_changed_cb (GtkWidget *window,
                                 GdkEventFocus *focus,
                                 gpointer data);


/* DESCRIPTION  : Called when a new ChatWindowPage is being focused.
 * BEHAVIOR     : Emits the "message-event" signal with the new number
 *                of unseen messages as the current ChatWindowPage does
 *                not have unseen messages anymore. 
 * PRE          : The ChatWindow as last argument.
 */
static void conversation_changed_cb (GtkNotebook *notebook,
                                     GtkNotebookPage *_page,
                                     gint n,
                                     gpointer data);


/*
 * Engine Callbacks
 */

/* DESCRIPTION  : Called when an IM has been received.
 * BEHAVIOR     : Display the received IM in a newly created or existing
 *                ChatWindowPage.
 *                Emits the "message-event" signal with the new number
 *                of unseen messages if the ChatWindowPage is not currently
 *                displayed.
 * PRE          : The ChatWindow as last argument.
 */
static void on_im_received_cb (const Ekiga::ChatManager & manager,
                               const std::string & display_name,
                               const std::string & from, 
                               const std::string & message,
                               gpointer data);


/* DESCRIPTION  : Called when an IM has been sent.
 * BEHAVIOR     : Display the sent IM in a newly created or existing
 *                ChatWindowPage.
 * PRE          : The ChatWindow as last argument.
 */
static void on_im_sent_cb (const Ekiga::ChatManager & manager,
                           const std::string & to,
                           const std::string & message,
                           gpointer data);


/* DESCRIPTION  : Called when an IM could not be sent.
 * BEHAVIOR     : Display the error message in a newly created or existing
 *                ChatWindowPage.
 * PRE          : The ChatWindow as last argument.
 */
static void on_im_failed_cb (const Ekiga::ChatManager & manager,
                             const std::string & to,
                             const std::string & reason,
                             gpointer data);


/*
 * Local GUI functions
 */

/* DESCRIPTION  : /
 * BEHAVIOR     : Return the ChatWindowPage position in the ChatWindowPage
 *                corresponding to the given uri.
 * PRE          : The ChatWindow as first argument, non-empty URI string.
 */
static gint chat_window_get_page_index (ChatWindow *chat_window,
                                        const std::string uri);


/* DESCRIPTION  : /
 * BEHAVIOR     : Return the ChatWindowPage GtkWidget in the ChatWindow
 *                corresponding to the given uri.
 * PRE          : The ChatWindow as first argument, non-empty URI string.
 */
static GtkWidget *chat_window_get_page (ChatWindow *chat_window,
                                        const std::string uri,
                                        const std::string display_name = "");


/* DESCRIPTION  : /
 * BEHAVIOR     : Return the number of unread messages.
 * PRE          : The ChatWindow as argument.
 */
static int chat_window_get_unread_messages (ChatWindow *chat_window);


/* 
 * GObject stuff
 */
static void
chat_window_dispose (GObject *obj)
{
  ChatWindow *self = NULL;

  self = CHAT_WINDOW (obj);

  self->priv->notebook = NULL;

  parent_class->dispose (obj);
}


static void
chat_window_finalize (GObject *obj)
{
  ChatWindow *self = NULL;

  self = CHAT_WINDOW (obj);

  for (std::vector<sigc::connection>::iterator iter = self->priv->connections.begin () ;
       iter != self->priv->connections.end ();
       iter++)
    iter->disconnect ();
  
  parent_class->finalize (obj);
}


static void
chat_window_class_init (gpointer g_class,
			G_GNUC_UNUSED gpointer class_data)
{
  GObjectClass *gobject_class = NULL;

  static gboolean initialised = false;

  parent_class = (GObjectClass *) g_type_class_peek_parent (g_class);
  g_type_class_add_private (g_class, sizeof (ChatWindowPrivate));

  gobject_class = (GObjectClass *) g_class;
  gobject_class->dispose = chat_window_dispose;
  gobject_class->finalize = chat_window_finalize;

  if (!initialised) {

    g_signal_new ("message-event",
                  G_OBJECT_CLASS_TYPE (g_class),
                  G_SIGNAL_RUN_FIRST,
                  0, NULL, NULL,
                  g_cclosure_marshal_VOID__UINT,
                  G_TYPE_NONE,
                  1, G_TYPE_UINT, NULL);
    initialised = true;
  }
}


GType
chat_window_get_type ()
{
  static GType result = 0;

  if (result == 0) {

    static const GTypeInfo info = {
      sizeof (ChatWindowClass),
      NULL,
      NULL,
      chat_window_class_init,
      NULL,
      NULL,
      sizeof (ChatWindow),
      0,
      NULL,
      NULL
    };

    result = g_type_register_static (GM_WINDOW_TYPE,
				     "ChatWindowType",
				     &info, (GTypeFlags) 0);
  }

  return result;
}


/* 
 * GTK+ Callbacks
 */

static void
close_event_cb (G_GNUC_UNUSED GtkWidget *widget,
                gpointer data)
{
  ChatWindow *self = CHAT_WINDOW (data);
  std::string uri;

  int i = 0;
  int n = 0;

  uri = chat_window_page_get_uri (CHAT_WINDOW_PAGE (widget));
  n = gtk_notebook_get_n_pages (GTK_NOTEBOOK (self->priv->notebook));
  
  if (i != -1 && n > 1)
    chat_window_remove_page (CHAT_WINDOW (data), uri);
}


static void
hide_event_cb (G_GNUC_UNUSED GtkWidget *widget,
               gpointer data)
{
  ChatWindow *self = CHAT_WINDOW (data);

  int n = 0;

  n = gtk_notebook_get_n_pages (GTK_NOTEBOOK (self->priv->notebook));

  for (int i = 0 ; i < n ; i++) {
    gtk_notebook_remove_page (GTK_NOTEBOOK (self->priv->notebook), 0);
  }
}


static void
focus_in_changed_cb (G_GNUC_UNUSED GtkWidget *window,
                     G_GNUC_UNUSED GdkEventFocus *focus,
                     gpointer data)
{
  ChatWindow *self = CHAT_WINDOW (data);
  ChatWindowPage *page = NULL;

  int n = 0;

  n = gtk_notebook_get_current_page (GTK_NOTEBOOK (self->priv->notebook));
  page = CHAT_WINDOW_PAGE (gtk_notebook_get_nth_page (GTK_NOTEBOOK (self->priv->notebook), n));

  chat_window_page_set_unread_messages (page, 0);
  g_signal_emit_by_name (CHAT_WINDOW (data), "message-event", 
                         chat_window_get_unread_messages (CHAT_WINDOW (data)));
}


static void
conversation_changed_cb (GtkNotebook *notebook,
                         G_GNUC_UNUSED GtkNotebookPage *_page,
                         gint n,
                         gpointer data)
{
  ChatWindowPage *page = NULL;

  page = CHAT_WINDOW_PAGE (gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), n));

  chat_window_page_set_unread_messages (page, 0);
  g_signal_emit_by_name (CHAT_WINDOW (data), "message-event", 
                         chat_window_get_unread_messages (CHAT_WINDOW (data)));
}


/*
 * Engine Callbacks
 */

static void
on_im_received_cb (const Ekiga::ChatManager & /*manager*/,
                   const std::string & display_name,
                   const std::string & from, 
                   const std::string & message,
                   gpointer data)
{
  GtkWidget *page = chat_window_get_page (CHAT_WINDOW (data), from, display_name);
  GtkWidget *notebook_page = NULL;

  int n = 0;
  int current_unread_messages = 0;
  gboolean visible = false;

  if (page == NULL)
    page = chat_window_add_page (CHAT_WINDOW (data), display_name, from);

  chat_window_page_add_message (CHAT_WINDOW_PAGE (page), display_name, from, message, false);

  g_object_get (CHAT_WINDOW (data), "visible", &visible, NULL);
  n = gtk_notebook_get_current_page (GTK_NOTEBOOK (CHAT_WINDOW (data)->priv->notebook));
  notebook_page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (CHAT_WINDOW (data)->priv->notebook), n);

  if (!visible || page != notebook_page) {

    current_unread_messages = chat_window_page_get_unread_messages (CHAT_WINDOW_PAGE (page));
    current_unread_messages++;
  }
  else {
   
    current_unread_messages = 0;
  }

  chat_window_page_set_unread_messages (CHAT_WINDOW_PAGE (page), current_unread_messages);
  g_signal_emit_by_name (CHAT_WINDOW (data), "message-event", current_unread_messages);
}


static void
on_im_sent_cb (const Ekiga::ChatManager & /*manager*/,
               const std::string & to,
               const std::string & message,
               gpointer data)
{
  GtkWidget *page = chat_window_get_page (CHAT_WINDOW (data), to);

  if (page == NULL)
    page = chat_window_add_page (CHAT_WINDOW (data), "", to);

  chat_window_page_add_message (CHAT_WINDOW_PAGE (page), "", to, message, true);
}


static void
on_im_failed_cb (const Ekiga::ChatManager & /*manager*/,
                 const std::string & to,
                 const std::string & reason,
                 gpointer data)
{
  GtkWidget *page = chat_window_get_page (CHAT_WINDOW (data), to);

  if (page == NULL)
    page = chat_window_add_page (CHAT_WINDOW (data), "", to);

  chat_window_page_add_error (CHAT_WINDOW_PAGE (page), to, reason);
}


/* 
 * Local GUI functions
 */

static gint
chat_window_get_page_index (ChatWindow *chat_window,
                            const std::string uri)
{
  ChatWindow *self = CHAT_WINDOW (chat_window);
  GtkWidget *page = NULL;

  bool found = false;
  int i = 0;
  int n = 0;
  
  n = chat_window_get_n_pages (self);

  while (i < n && !found) {

    page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (self->priv->notebook), i);

    if (page != NULL && chat_window_page_get_uri (CHAT_WINDOW_PAGE (page)) == uri) 
      return i;

    i++;
  }

  return -1;
}


static GtkWidget *
chat_window_get_page (ChatWindow *chat_window,
                      const std::string uri,
                      const std::string display_name)
{
  ChatWindow *self = CHAT_WINDOW (chat_window);
  GtkWidget *page = NULL;

  std::string host1, host2;
  bool found = false;
  int i = 0;
  int n = 0;
  
  n = chat_window_get_n_pages (self);

  while (i < n && !found) {

    std::size_t pos;

    page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (self->priv->notebook), i);
    host1 = chat_window_page_get_uri (CHAT_WINDOW_PAGE (page));
    host2 = uri;

    pos = host1.find_first_of (':');
    if (pos == 4)
      host1 = host1.substr (pos+1);
    pos = host2.find_first_of (':');
    if (pos == 4)
      host1 = host1.substr (pos+1);
    pos = host1.find_last_of ('@');
    if (pos != std::string::npos)
      host1 = host1.substr (pos+1);
    pos = host2.find_last_of ('@');
    if (pos != std::string::npos)
      host2 = host2.substr (pos+1);

    if (page != NULL && chat_window_page_get_uri (CHAT_WINDOW_PAGE (page)) == uri) 
      return page;

    if (page != NULL && host1 != host2 && chat_window_page_get_display_name (CHAT_WINDOW_PAGE (page)) == display_name)
      return page;

    i++;
  }

  return NULL;
}


static int 
chat_window_get_unread_messages (ChatWindow *chat_window)
{
  ChatWindow *self = CHAT_WINDOW (chat_window);

  GtkWidget *page = NULL;

  int unread = 0;
  int i = 0;
  int n = 0;
  
  n = chat_window_get_n_pages (self);

  while (i < n) {

    page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (self->priv->notebook), i);

    if (page != NULL)
      unread += chat_window_page_get_unread_messages (CHAT_WINDOW_PAGE (page));

    i++;
  }

  return unread;
}


/* Public API */
GtkWidget *
chat_window_new (Ekiga::ServiceCore & core)
{
  ChatWindow *self = NULL;

  Ekiga::ChatCore *chat_core = NULL;

  sigc::connection conn;

  self = CHAT_WINDOW (g_object_new (CHAT_WINDOW_TYPE, NULL));
  self->priv = new ChatWindowPrivate (core);

  self->priv->notebook = NULL;

  /* The window */
  gtk_window_set_title (GTK_WINDOW (self), _("Chat Window"));
  gtk_window_set_position (GTK_WINDOW (self), GTK_WIN_POS_CENTER);
  g_object_set (G_OBJECT (self), "focus-on-map", true, NULL);

  /* Build the window */
  self->priv->notebook = gtk_notebook_new ();
  gtk_notebook_set_scrollable (GTK_NOTEBOOK (self->priv->notebook), TRUE);
  gtk_container_add (GTK_CONTAINER (self), self->priv->notebook);

  /* Engine signals */
  chat_core = dynamic_cast<Ekiga::ChatCore *> (core.get ("chat-core"));
  conn = chat_core->im_received.connect (sigc::bind (sigc::ptr_fun (on_im_received_cb), self));
  self->priv->connections.push_back (conn);
  conn = chat_core->im_sent.connect (sigc::bind (sigc::ptr_fun (on_im_sent_cb), self));
  self->priv->connections.push_back (conn);
  conn = chat_core->im_failed.connect (sigc::bind (sigc::ptr_fun (on_im_failed_cb), self));
  self->priv->connections.push_back (conn);

  g_signal_connect (G_OBJECT (self), "hide",
		    G_CALLBACK (hide_event_cb), self);

  g_signal_connect (G_OBJECT (self), "focus_in_event",
                    G_CALLBACK (focus_in_changed_cb), self);

  g_signal_connect (G_OBJECT (self->priv->notebook), "switch_page",
                    G_CALLBACK (conversation_changed_cb), self);

  gtk_widget_show_all (GTK_WIDGET (self->priv->notebook));

  return GTK_WIDGET (self);
}


GtkWidget *
chat_window_new_with_key (Ekiga::ServiceCore & _core,
                          const std::string _key)
{
  ChatWindow *self = CHAT_WINDOW (chat_window_new (_core));

  g_object_set (self, "key", _key.c_str (), NULL); 

  return GTK_WIDGET (self);
}


GtkWidget *
chat_window_add_page (ChatWindow *chat_window,
                      const std::string display_name,
                      const std::string uri)
{
  ChatWindow *self = CHAT_WINDOW (chat_window);
  GtkWidget *page = NULL;
  int i = 0;
  
  i = chat_window_get_page_index (self, uri);

  if (i == -1) {

    page = chat_window_page_new (self->priv->core, display_name, uri);
    gtk_notebook_append_page (GTK_NOTEBOOK (self->priv->notebook), 
                              page, 
                              chat_window_page_get_label (CHAT_WINDOW_PAGE (page)));
    gtk_notebook_set_tab_label_packing (GTK_NOTEBOOK (self->priv->notebook), 
                                        page, TRUE, TRUE, GTK_PACK_START);

    i = chat_window_get_n_pages (self);
    i--;

    g_signal_connect (page, "close-event",
                      G_CALLBACK (close_event_cb), chat_window);
  }
  else
    page = chat_window_get_page (chat_window, uri, display_name);

  gtk_notebook_set_current_page (GTK_NOTEBOOK (self->priv->notebook), i);

  chat_window_page_grab_focus (CHAT_WINDOW_PAGE (page));

  gtk_widget_show_all (GTK_WIDGET (page));

  return page;
}


void
chat_window_remove_page (ChatWindow *chat_window,
                         const std::string uri)
{
  ChatWindow *self = CHAT_WINDOW (chat_window);

  int i = 0;
  int n = 0;

  i = chat_window_get_page_index (self, uri);
  n = chat_window_get_n_pages (self);

  if (i > -1 && n > 1) 
    gtk_notebook_remove_page (GTK_NOTEBOOK (self->priv->notebook), i);
}


int 
chat_window_get_n_pages (ChatWindow *chat_window)
{
  ChatWindow *self = CHAT_WINDOW (chat_window);

  return gtk_notebook_get_n_pages (GTK_NOTEBOOK (self->priv->notebook));
}
