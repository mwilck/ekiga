
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
 *                         chat_window.cpp  -  description
 *                         -------------------------------
 *   begin                : Wed Jan 23 2002
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : This file contains functions to build the chat
 *                          window. It uses DTMF tones.
 *   Additional code      : Kenneth Christiansen  <kenneth@gnu.org>
 *
 */


#include "config.h"

#include "chat-window.h"
#include "chat-window-page.h"
#include "ekiga.h"

#include "gmconf.h"
#include "gmtexttagaddon.h"
#include "gmtextbufferaddon.h"
#include "gmtextviewaddon.h"
#include "gmstockicons.h"

#include <gdk/gdkkeysyms.h>

/*
 * The ChatWindow
 */
struct _ChatWindowPrivate 
{
  _ChatWindowPrivate (Ekiga::ServiceCore & _core) : core (_core) { }

  GtkWidget *notebook;
  gchar *key;

  std::vector<sigc::connection> connections;
  Ekiga::ServiceCore & core;
};

enum { CHAT_WINDOW_KEY = 1 };

static GObjectClass *parent_class = NULL;


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
                       gpointer class_data)
{
  GObjectClass *gobject_class = NULL;

  parent_class = (GObjectClass *) g_type_class_peek_parent (g_class);
  g_type_class_add_private (g_class, sizeof (ChatWindowPrivate));

  gobject_class = (GObjectClass *) g_class;
  gobject_class->dispose = chat_window_dispose;
  gobject_class->finalize = chat_window_finalize;
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

typedef struct GmTextChatWindowPage_ GmTextChatWindowPage;

#define GM_TEXT_CHAT_WINDOW_PAGE(x) (GmTextChatWindowPage *) (x)


/* Declarations */

/* GUI functions */
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
      return page;

    i++;
  }

  return NULL;
}


/* Callbacks */
static void
on_im_received_cb (std::string display_name,
                   std::string from, 
                   std::string message,
                   gpointer data)
{
  GtkWidget *page = chat_window_get_page (CHAT_WINDOW (data), from);

  if (page)
    chat_window_page_add_message (CHAT_WINDOW_PAGE (page), display_name, from, message, false);
}


static void
on_im_sent_cb (std::string to,
               std::string message,
               gpointer data)
{
  GtkWidget *page = chat_window_get_page (CHAT_WINDOW (data), to);

  if (page)
    chat_window_page_add_message (CHAT_WINDOW_PAGE (page), "", to, message, true);
}


static void
on_im_failed_cb (std::string to,
                 std::string reason,
                 gpointer data)
{
  GtkWidget *page = chat_window_get_page (CHAT_WINDOW (data), to);

  if (page)
    chat_window_page_add_error (CHAT_WINDOW_PAGE (page), to, reason);
}


static void
close_event_cb (GtkWidget *widget,
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
hide_event_cb (GtkWidget *widget,
               gpointer data)
{
  ChatWindow *self = CHAT_WINDOW (data);

  int n = 0;

  n = gtk_notebook_get_n_pages (GTK_NOTEBOOK (self->priv->notebook));

  for (int i = 0 ; i < n ; i++) {
    gtk_notebook_remove_page (GTK_NOTEBOOK (self->priv->notebook), 0);
  }
}


/* Public API */
GtkWidget *
chat_window_new (Ekiga::ServiceCore & core)
{
  ChatWindow *self = NULL;

  GtkWidget *vbox = NULL;

  sigc::connection conn;

  self = CHAT_WINDOW (g_object_new (CHAT_WINDOW_TYPE, NULL));
  self->priv = new ChatWindowPrivate (core);

  self->priv->notebook = NULL;
  self->priv->key = g_strdup ("");

  /* The window */
  gtk_window_set_title (GTK_WINDOW (self), _("Chat Window"));
  gtk_window_set_position (GTK_WINDOW (self), GTK_WIN_POS_CENTER);

  /* Build the window */
  vbox = gtk_vbox_new (FALSE, 0);
  self->priv->notebook = gtk_notebook_new ();
  gtk_notebook_set_scrollable (GTK_NOTEBOOK (self->priv->notebook), TRUE);
  gtk_container_add (GTK_CONTAINER (self), self->priv->notebook);

  // FIXME GnomeMeeting::Process should disappear
  conn = GnomeMeeting::Process ()->GetManager ()->im_received.connect (sigc::bind (sigc::ptr_fun (on_im_received_cb), self));
  self->priv->connections.push_back (conn);
  conn = GnomeMeeting::Process ()->GetManager ()->im_sent.connect (sigc::bind (sigc::ptr_fun (on_im_sent_cb), self));
  self->priv->connections.push_back (conn);
  conn = GnomeMeeting::Process ()->GetManager ()->im_failed.connect (sigc::bind (sigc::ptr_fun (on_im_failed_cb), self));
  self->priv->connections.push_back (conn);

  g_signal_connect (G_OBJECT (self), "hide",
		    G_CALLBACK (hide_event_cb), self);

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


void
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

  gtk_notebook_set_current_page (GTK_NOTEBOOK (self->priv->notebook), i);
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

