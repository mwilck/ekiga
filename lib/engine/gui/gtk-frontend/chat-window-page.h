
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
 *                         chatwindowpage.h  -  description
 *                         --------------------------------
 *   begin                : Sun Sep 16 2007
 *   copyright            : (C) 2000-2007 by Damien Sandras
 *   description          : A page in the ChatWindow
 */


#ifndef _CHAT_WINDOW_PAGE_H_
#define _CHAT_WINDOW_PAGE_H_

#include <gtk/gtk.h>
#include "services.h"


G_BEGIN_DECLS

typedef struct _ChatWindowPage ChatWindowPage;
typedef struct _ChatWindowPagePrivate ChatWindowPagePrivate;
typedef struct _ChatWindowPageClass ChatWindowPageClass;


/* GObject thingies */
struct _ChatWindowPage
{
  GtkVBox parent;
  ChatWindowPagePrivate *priv;
};

struct _ChatWindowPageClass
{
  GtkVBoxClass parent;
};

#define CHAT_WINDOW_PAGE_TYPE (chat_window_page_get_type ())

#define CHAT_WINDOW_PAGE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), CHAT_WINDOW_PAGE_TYPE, ChatWindowPage))

#define IS_CHAT_WINDOW_PAGE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CHAT_WINDOW_PAGE_TYPE))

#define CHAT_WINDOW_PAGE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), CHAT_WINDOW_PAGE_TYPE, ChatWindowPageClass))

#define IS_CHAT_WINDOW_PAGE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CHAT_WINDOW_PAGE_TYPE))

#define CHAT_WINDOW_PAGE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), CHAT_WINDOW_PAGE_TYPE, ChatWindowPageClass))

GType chat_window_page_get_type ();


/* Public API */

/* DESCRIPTION  : /
 * BEHAVIOR     : Crate a new ChatWindowPage and listen for signals emitted
 *                by the Ekiga::ServiceCore.
 * PRE          : The Ekiga::ServiceCore as argument. The uri should not be
 *                empty.
 */
GtkWidget *chat_window_page_new (Ekiga::ServiceCore & core,
                                 const std::string display_name,
                                 const std::string uri);


/* DESCRIPTION  : /
 * BEHAVIOR     : Return the GtkWidget containing the label to add in 
 *                GtkNotebook in which the ChatWindowPage will be added.
 * PRE          : The ChatWindowPage as argument. 
 */
GtkWidget *chat_window_page_get_label (ChatWindowPage *page);


/* DESCRIPTION  : /
 * BEHAVIOR     : Return the std::string uri associated with the
 *                ChatWindowPage.
 * PRE          : The ChatWindowPage as argument. 
 */
const std::string chat_window_page_get_uri (ChatWindowPage *page);


/* DESCRIPTION  : /
 * BEHAVIOR     : Return the std::string display name associated with the
 *                ChatWindowPage.
 * PRE          : The ChatWindowPage as argument. 
 */
const std::string chat_window_page_get_display_name (ChatWindowPage *page);


/* DESCRIPTION  : /
 * BEHAVIOR     : Add the given message from the given uri into the
 *                ChatWindowPage.
 * PRE          : The ChatWindowPage as argument. 
 */
void chat_window_page_add_message (ChatWindowPage *page,
                                   const std::string display_name,
                                   const std::string uri,
                                   const std::string message,
                                   gboolean is_sent);


/* DESCRIPTION  : /
 * BEHAVIOR     : Add the given error message from the given uri into the
 *                ChatWindowPage.
 * PRE          : The ChatWindowPage as argument. 
 */
void chat_window_page_add_error (ChatWindowPage *page,
                                 const std::string uri,
                                 const std::string message);


/* DESCRIPTION  : /
 * BEHAVIOR     : Update the number of current unread messages for the
 *                given ChatWindowPage.
 * PRE          : The ChatWindowPage as argument and the unread messages. 
 */
void chat_window_page_set_unread_messages (ChatWindowPage *page,
                                           int messages);


/* DESCRIPTION  : /
 * BEHAVIOR     : Return the number of current unread messages for the
 *                given ChatWindowPage.
 * PRE          : The ChatWindowPage as argument.
 */
int chat_window_page_get_unread_messages (ChatWindowPage *page);


/* DESCRIPTION  : /
 * BEHAVIOR     : Grab the focus in the messages part of the 
 *                given ChatWindowPage.
 * PRE          : The ChatWindowPage as argument.
 */
void chat_window_page_grab_focus (ChatWindowPage *page);

G_END_DECLS

#endif

