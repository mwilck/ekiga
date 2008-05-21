
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
 *                         chatwindowpage.cpp  -  description
 *                         ----------------------------------
 *   begin                : Sun Sep 16 2007
 *   copyright            : (C) 2000-2007 by Damien Sandras
 *   description          : A page in the ChatWindow
 */

#include "config.h"

#include "chat-window-page.h"

#include "gmtexttagaddon.h"
#include "gmtextbufferaddon.h"
#include "gmtextviewaddon.h"
#include "gmstockicons.h"

#include "call-core.h"
#include "presence-core.h"

#include <gdk/gdkkeysyms.h>
#include <vector>
#include <iostream>


struct _ChatWindowPagePrivate
{
  _ChatWindowPagePrivate (Ekiga::ServiceCore & _core) : core (_core) { }
  GtkWidget *send_button;
  GtkWidget *smiley_button;
  GtkWidget *conversation;
  GtkWidget *message;
  GtkWidget *tab_label_status;
  GtkWidget *tab_label_name;
  GtkWidget *tab_image;
  GtkWidget *tab;

  std::string uri;
  std::string display_name;

  int last_user;

  std::vector<sigc::connection> connections;
  Ekiga::ServiceCore & core;
  int unread_messages;
};

static GObjectClass *parent_class = NULL;


/*
 * GTK+ Callbacks
 */

/* DESCRIPTION  : Called when the user clicks on the cross to close a 
 *                ChatWindowPage.
 * BEHAVIOR     : Emit the close-event signal. In general, the ChatWindow 
 *                will listen to that signal in order to know when a 
 *                ChatWindowPage needs to be removed from the ChatWindow.
 * PRE          : The ChatWindowPage passed as second parameter.
 */
static void close_button_clicked_cb (GtkWidget *w,
                                     gpointer data);


/* DESCRIPTION  : Called when the user hits the RETURN key in the
 *                ChatWindowPage.
 * BEHAVIOR     : Emulate a click on the "send" button.
 * PRE          : The ChatWindowPage passed as second parameter.
 */
static gboolean chat_entry_activated_cb (GtkWidget *w,
                                         GdkEventKey *key,
                                         gpointer data);


/* DESCRIPTION  : Called when the user hits the BACKSPACE key in the
 *                ChatWindowPage.
 * BEHAVIOR     : Delete the char on the left of the cursor. If it is a smiley,
 *                then delete the smiley and the associated invisible
 *                text. (Each smiley is preceeded by its textual representation
 *                under an invisible form).
 * PRE          : /
 */
static void chat_entry_backspace_cb (GtkTextView *text_view,
                                     gpointer data);


/* DESCRIPTION  : Called when the user hits the send button.
 * BEHAVIOR     : Send the message (if not empty) to the remote peer using
 *                the engine manager. Delete the sent message from the
 *                ChatWindowPage.
 * PRE          : The ChatWindowPage passed as second parameter.
 */
static void send_button_clicked_cb (GtkWidget *w,
                                    gpointer data);


/* DESCRIPTION  : Called when the user hits the smiley button.
 * BEHAVIOR     : Popup the menu allowing to insert a smiley in the
 *                conversation.
 * PRE          : The GtkMenu to popup passed as second parameter.
 */
static void smiley_button_clicked_cb (GtkButton *w,
                                      gpointer data);


/* DESCRIPTION  : Called when the user selects a smiley in the GtkMenu.
 * BEHAVIOR     : Insert the smiley in the conversation, preceeded by
 *                its textual representation, displayed as invisible text.
 * PRE          : The ChatWindowPage passed as second parameter.
 */
static void smiley_activated_cb (GtkMenuItem *w,
                                 gpointer data);

/*
 * Engine Callbacks
 */

/* DESCRIPTION  : Called when a Ekiga::Cluster is to the added to the
 *                Ekiga::PresenceCore.
 * BEHAVIOR     : Call the visit_heaps method on the Ekiga::Cluster
 *                in order to trigger on_heap_visited for each visited
 *                Ekiga::Heap.
 * PRE          : The ChatWindowPage as second parameter.
 */
static void on_cluster_added (Ekiga::Cluster &cluster,
			      gpointer data);


/* DESCRIPTION  : Called when a Ekiga::Heap is visited.
 * BEHAVIOR     : Call the on_heap_added callback in order to add
 *                the monitored Ekiga::Heap to the list of active Heaps.
 * PRE          : The ChatWindowPage as last parameter.
 */
static bool on_heap_visited (Ekiga::Heap &heap,
                             Ekiga::Cluster *cluster,
                             gpointer data);


/* DESCRIPTION  : Called when a Ekiga::Heap is added.
 * BEHAVIOR     : Call the visit_presentities method on the Ekiga::Heap
 *                in order to trigger on_presentity_visited for each visited
 *                Ekiga::Presentity.
 * PRE          : The ChatWindowPage as last parameter.
 */
static void on_heap_added (Ekiga::Cluster &cluster,
                           Ekiga::Heap &heap,
                           gpointer data);


/* DESCRIPTION  : Called when a Ekiga::Presentity is visited.
 * BEHAVIOR     : Call the on_presentity_added callback in order to add
 *                the monitored Ekiga::Presentity to the list of active 
 *                Presentities.
 * PRE          : The ChatWindowPage as last parameter.
 */
static bool on_presentity_visited (Ekiga::Presentity &presentity,
                                   Ekiga::Cluster *cluster,
                                   Ekiga::Heap *heap,
                                   gpointer data);


/* DESCRIPTION  : Called when a Ekiga::Presentity is added in a Heap.
 * BEHAVIOR     : Call the on_presentity_updated callback.
 * PRE          : The ChatWindowPage as last parameter.
 */
static void on_presentity_added (Ekiga::Cluster &cluster,
				 Ekiga::Heap &heap,
				 Ekiga::Presentity &presentity,
				 gpointer data);


/* DESCRIPTION  : Called when a Ekiga::Presentity is added in a Heap.
 * BEHAVIOR     : If the Ekiga::Presentity is the one with who we are doing 
 *                a conversation, then update its state representation in
 *                the ChatWindowPage.
 * PRE          : The ChatWindowPage as last parameter.
 */
static void on_presentity_updated (Ekiga::Cluster &cluster,
				   Ekiga::Heap &heap,
				   Ekiga::Presentity &presentity,
				   gpointer data);


/* DESCRIPTION  : Called when a Ekiga::Presentity is removed from a Heap.
 * BEHAVIOR     : If the Ekiga::Presentity is the one with who we are doing 
 *                a conversation, then update its state representation in
 *                the ChatWindowPage to unknown.
 * PRE          : The ChatWindowPage as last parameter.
 */
static void on_presentity_removed (Ekiga::Cluster &cluster,
				   Ekiga::Heap &heap,
				   Ekiga::Presentity &presentity,
				   gpointer data);


/* 
 * GObject stuff
 */
static void
chat_window_page_dispose (GObject *obj)
{
  ChatWindowPage *self = NULL;

  self = CHAT_WINDOW_PAGE (obj);

  self->priv->send_button = NULL;
  self->priv->smiley_button = NULL;
  self->priv->conversation = NULL;
  self->priv->message = NULL;
  self->priv->tab_label_name = NULL;
  self->priv->tab_label_status = NULL;
  self->priv->tab_image = NULL;
  self->priv->tab = NULL;
  self->priv->last_user = -1;
  self->priv->unread_messages = 0;

  parent_class->dispose (obj);
}


static void
chat_window_page_finalize (GObject *obj)
{
  ChatWindowPage *self = NULL;

  self = CHAT_WINDOW_PAGE (obj);

  for (std::vector<sigc::connection>::iterator iter
	 = self->priv->connections.begin ();
       iter != self->priv->connections.end ();
       iter++)
    iter->disconnect ();

  delete self->priv;
  parent_class->finalize (obj);
}


static void
chat_window_page_class_init (gpointer g_class,
                             G_GNUC_UNUSED gpointer class_data)
{
  GObjectClass *gobject_class = NULL;

  parent_class = (GObjectClass *) g_type_class_peek_parent (g_class);

  gobject_class = (GObjectClass *) g_class;
  gobject_class->dispose = chat_window_page_dispose;
  gobject_class->finalize = chat_window_page_finalize;

  g_signal_new ("close-event",
                G_OBJECT_CLASS_TYPE (g_class),
                G_SIGNAL_RUN_FIRST,
                0, NULL, NULL,
                g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0, NULL);
}


GType
chat_window_page_get_type ()
{
  static GType result = 0;

  if (result == 0) {

    static const GTypeInfo info = {
      sizeof (ChatWindowPageClass),
      NULL,
      NULL,
      chat_window_page_class_init,
      NULL,
      NULL,
      sizeof (ChatWindowPage),
      0,
      NULL,
      NULL
    };

    result = g_type_register_static (GTK_TYPE_VBOX,
                                     "ChatWindowPageType",
                                     &info, (GTypeFlags) 0);
  }

  return result;
}


/*
 * Our own stuff
 */

/*
 * GTK+ Callbacks
 */
static void 
close_button_clicked_cb (G_GNUC_UNUSED GtkWidget *w,
                         gpointer data)
{
  g_return_if_fail (data != NULL);

  g_signal_emit_by_name (data, "close-event", NULL);
}


static gboolean 
chat_entry_activated_cb (G_GNUC_UNUSED GtkWidget *w,
                         GdkEventKey *key,
                         gpointer data)
{
  ChatWindowPage *self = CHAT_WINDOW_PAGE (data);

  if (key->keyval == GDK_Return) {
    gtk_button_clicked (GTK_BUTTON (self->priv->send_button));
    return true;
  }

  return false;
}


static void 
chat_entry_backspace_cb (GtkTextView *text_view,
                         G_GNUC_UNUSED gpointer data)
{
  GtkTextBuffer *buffer = gtk_text_view_get_buffer (text_view);
  GtkTextIter *start_iter = NULL;
  GtkTextIter end_iter;

  gtk_text_buffer_get_iter_at_mark (buffer, &end_iter, 
                                    gtk_text_buffer_get_insert (buffer));
  start_iter = gtk_text_iter_copy (&end_iter);
  gtk_text_iter_backward_visible_cursor_position (&end_iter);
  if (gtk_text_iter_get_pixbuf (&end_iter)) {

    gtk_text_iter_backward_search (&end_iter, " ", 
                                   GTK_TEXT_SEARCH_TEXT_ONLY, 
                                   start_iter, NULL, NULL); 
    gtk_text_buffer_delete (buffer, start_iter, &end_iter);
  }
  gtk_text_iter_free (start_iter);
}


static void
send_button_clicked_cb (G_GNUC_UNUSED GtkWidget *w,
                        gpointer data)
{
  ChatWindowPage *self = CHAT_WINDOW_PAGE (data);

  GtkTextIter start_iter, end_iter;
  GtkTextBuffer *buffer = NULL;

  gchar *body = NULL;
  std::string message;

  g_return_if_fail (data != NULL);

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (self->priv->message));
  gtk_text_buffer_get_start_iter (GTK_TEXT_BUFFER (buffer), &start_iter);
  gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (buffer), &end_iter);
  body = gtk_text_buffer_get_text (GTK_TEXT_BUFFER (buffer), &start_iter, &end_iter, TRUE);
  gtk_text_buffer_delete (GTK_TEXT_BUFFER (buffer), &start_iter, &end_iter);

  message = body;
  g_free (body);

  if (!message.empty ()) {
    
    Ekiga::CallCore *call_core = dynamic_cast<Ekiga::CallCore *>(self->priv->core.get ("call-core"));

    if (call_core) 
      call_core->send_message (self->priv->uri, message);
  }
}


static void 
smiley_button_clicked_cb (G_GNUC_UNUSED GtkButton *w,
                          gpointer data)
{
  g_return_if_fail (data != NULL);
  gtk_menu_popup (GTK_MENU (data), NULL, NULL, NULL, NULL, 0, 0);
}


static void
smiley_activated_cb (GtkMenuItem *menu,
                     gpointer data)
{
  GtkTextBuffer *buffer = NULL;

  GtkTextIter iter;

  const char *text = NULL;

  ChatWindowPage *self = CHAT_WINDOW_PAGE (data);

  g_return_if_fail (data != NULL);

  text = gtk_label_get_text (GTK_LABEL (GTK_BIN (GTK_MENU_ITEM (menu))->child));

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (self->priv->message));
  gtk_text_buffer_get_iter_at_mark (buffer, &iter, 
                                    gtk_text_buffer_get_insert (buffer));
  gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, " ", -1, "invisible", NULL);
  gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, text, -1, "invisible", NULL);
  gtk_text_buffer_insert_with_regex (buffer, &iter, text);

  gtk_widget_grab_focus (self->priv->message);
}


/*
 * Engine Callbacks
 */

static void
on_cluster_added (Ekiga::Cluster &cluster,
		  gpointer data)
{
  cluster.visit_heaps (sigc::bind (sigc::ptr_fun (on_heap_visited), &cluster, data));
}


static bool
on_heap_visited (Ekiga::Heap &heap,
                 Ekiga::Cluster *cluster,
                 gpointer data)
{
  on_heap_added (*cluster, heap, data);

  return true;
}


static void
on_heap_added (Ekiga::Cluster &cluster,
               Ekiga::Heap &heap,
               gpointer data)
{
  heap.visit_presentities (sigc::bind (sigc::ptr_fun (on_presentity_visited), &cluster, &heap, data));
}


static bool
on_presentity_visited (Ekiga::Presentity &presentity,
                       Ekiga::Cluster *cluster,
                       Ekiga::Heap *heap,
                       gpointer data)
{
  on_presentity_added (*cluster, *heap, presentity, data);

  return true;
}


static void
on_presentity_added (Ekiga::Cluster &cluster,
		     Ekiga::Heap &heap,
		     Ekiga::Presentity &presentity,
		     gpointer data)
{
  on_presentity_updated (cluster, heap, presentity, data);
}


static void
on_presentity_updated (G_GNUC_UNUSED Ekiga::Cluster &cluster,
		       G_GNUC_UNUSED Ekiga::Heap &heap,
		       Ekiga::Presentity &presentity,
		       gpointer data)
{
  ChatWindowPage *self = CHAT_WINDOW_PAGE (data);

  if (presentity.get_uri () == self->priv->uri) {

    gtk_image_set_from_stock (GTK_IMAGE (self->priv->tab_image), 
                              presentity.get_presence ().c_str (), 
                              GTK_ICON_SIZE_MENU);
    gtk_label_set_text (GTK_LABEL (self->priv->tab_label_name),
                        presentity.get_name ().c_str ());
    gtk_label_set_text (GTK_LABEL (self->priv->tab_label_status),
                        presentity.get_status ().c_str ());
    self->priv->display_name = presentity.get_name (); 
  }
}


static void
on_presentity_removed (G_GNUC_UNUSED Ekiga::Cluster &cluster,
		       G_GNUC_UNUSED Ekiga::Heap &heap,
		       Ekiga::Presentity &presentity,
		       gpointer data)
{
  ChatWindowPage *self = CHAT_WINDOW_PAGE (data);

  if (presentity.get_uri () == self->priv->uri) {

    gtk_image_set_from_stock (GTK_IMAGE (self->priv->tab_image), 
                              "presence-unknown",
                              GTK_ICON_SIZE_MENU);
    gtk_label_set_text (GTK_LABEL (self->priv->tab_label_name),
                        presentity.get_name ().c_str ());
    self->priv->display_name = presentity.get_name (); 
  }
}


/*
 * Public API
 */

GtkWidget *
chat_window_page_new (Ekiga::ServiceCore & core,
                      const std::string display_name,
                      const std::string uri)
{
  ChatWindowPage *self = NULL;

  sigc::connection conn;
  
  GtkWidget *close_button = NULL;
  GtkWidget *close_image = NULL;
  GtkRcStyle *rc_style;

  GtkWidget *scr = NULL;
  GtkWidget *hbox = NULL;
  GtkWidget *hbox2 = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *label = NULL;
  GtkWidget *image = NULL;
  GtkWidget *frame = NULL;
  GtkWidget *vpane = NULL;
  GtkWidget *align = NULL;
  GtkWidget *arrow = NULL;
  GtkWidget *sep = NULL;

  PangoAttrList *attr_lst = NULL;
  PangoAttribute *attr = NULL;

  GtkTextIter iter;
  GtkTextBuffer *buffer = NULL;
  GtkTextMark *mark = NULL;
  GtkTextTag *regex_tag = NULL;

  GdkColor color_fg;

  Ekiga::PresenceCore *presence_core = NULL; 

  const char *smileys [] = 
    {
      "face-angel",
      "face-cool",
      "face-crying",
      "face-embarrassed",
      "face-devilish",
      "face-kiss",
      "face-monkey",
      "face-plain",
      "face-raspberry",
      "face-sad",
      "face-smile",
      "face-smile-big",
      "face-smirk",
      "face-surprise",
      "face-wink"
    };

  const char *smiley_texts [] = 
    {
      "0:-)",
      "B-)",
      ":'-(",
      ":-[",
      ">:-)",
      ":-*",
      ":-(|)",
      ":-[",
      ":-P",
      ":-(",
      ":-)",
      ":-D",
      ":-!",
      ":-O",
      ";-)"
    };

  GtkIconTheme *theme = NULL;
  GdkPixbuf *pixbuf = NULL;

  GtkWidget *menu = NULL;
  GtkWidget *menu_item = NULL;

  /* Start building the page */
  self = CHAT_WINDOW_PAGE (g_object_new (CHAT_WINDOW_PAGE_TYPE, NULL));
  self->priv = new ChatWindowPagePrivate (core);

  self->priv->send_button = NULL;
  self->priv->smiley_button = NULL;
  self->priv->conversation = NULL;
  self->priv->message = NULL;
  self->priv->tab_label_name = NULL;
  self->priv->tab_label_status = NULL;
  self->priv->tab_image = NULL;
  self->priv->tab = NULL;
  self->priv->last_user = -1;
  self->priv->unread_messages = 0;

  /* Vertical pane to contain the chat and the message to send */
  vpane = gtk_vpaned_new ();
  gtk_container_set_border_width (GTK_CONTAINER (vpane), 12);

  /* Above part */
  vbox = gtk_vbox_new (FALSE, 4);

  self->priv->conversation = gtk_text_view_new_with_regex ();
  gtk_text_view_set_editable (GTK_TEXT_VIEW (self->priv->conversation), FALSE);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (self->priv->conversation),
                               GTK_WRAP_WORD);

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (self->priv->conversation));
  gtk_text_buffer_get_end_iter (buffer, &iter);
  gtk_text_view_set_cursor_visible  (GTK_TEXT_VIEW (self->priv->conversation),
                                     FALSE);
  mark = gtk_text_buffer_create_mark (buffer, "current-position", &iter, FALSE);
  gtk_text_buffer_create_tag (buffer, "remote-user",
                              "foreground", "red", 
                              "weight", 900, NULL);
  gtk_text_buffer_create_tag (buffer, "local-user",
                              "foreground", "darkblue", 
                              "weight", 900, NULL);
  gtk_text_buffer_create_tag (buffer, "timestamp",
                              "foreground", "darkgray", 
                              "left-margin", 15,
                              "stretch", PANGO_STRETCH_CONDENSED, NULL);
  gtk_text_buffer_create_tag (buffer, "error",
                              "foreground", "red", NULL); 

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  scr = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (scr), self->priv->conversation);
  gtk_container_add (GTK_CONTAINER (frame), scr);
  gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);

  gtk_paned_pack1 (GTK_PANED (vpane), vbox, TRUE, FALSE);

  /* Bottom part */
  // The message part
  vbox = gtk_vbox_new (FALSE, 4);
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_container_add (GTK_CONTAINER (frame), vbox);

  self->priv->message = gtk_text_view_new_with_regex ();
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (self->priv->message), 
                               GTK_WRAP_WORD_CHAR);
  gtk_text_view_set_cursor_visible  (GTK_TEXT_VIEW (self->priv->message), true);
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (self->priv->message));
  gtk_text_buffer_create_tag (buffer, "invisible",
                              "invisible", true, NULL);

  scr = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (scr), self->priv->message);
  gtk_box_pack_start (GTK_BOX (vbox), scr, TRUE, TRUE, 0);

  // An horizontal separator
  sep = gtk_hseparator_new ();
  gtk_box_pack_start (GTK_BOX (vbox), sep, FALSE, FALSE, 0);

  // The actions bar
  hbox = gtk_hbox_new (FALSE, 0);

  // The smiley button
  theme = gtk_icon_theme_get_default();
  menu = gtk_menu_new ();
  for (int i = 0 ; i < 15 ; i++) {

    menu_item = gtk_image_menu_item_new_with_label (smiley_texts [i]);
    pixbuf = gtk_icon_theme_load_icon (theme, smileys [i], 
                                       16, (GtkIconLookupFlags) 0, 
                                       NULL);
    image = gtk_image_new_from_pixbuf (pixbuf);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);
    gtk_widget_show_all (menu_item);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

    g_signal_connect (GTK_OBJECT (menu_item), "activate",
                      G_CALLBACK (smiley_activated_cb), self);

  }
  pixbuf = gtk_icon_theme_load_icon (theme, "face-smile",
                                     16, (GtkIconLookupFlags) 0, 
                                     NULL);
  image = gtk_image_new_from_pixbuf (pixbuf);
  align = gtk_alignment_new (0.0, 0.5, 0.0, 0.0);
  self->priv->smiley_button = gtk_button_new ();
  gtk_container_add (GTK_CONTAINER (align), self->priv->smiley_button);
  gtk_button_set_relief (GTK_BUTTON (self->priv->smiley_button), GTK_RELIEF_NONE);
  hbox2 = gtk_hbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (self->priv->smiley_button), hbox2);
  gtk_box_pack_start (GTK_BOX (hbox2), image, FALSE, FALSE, 0);
  arrow = gtk_arrow_new (GTK_ARROW_DOWN, GTK_SHADOW_NONE);
  gtk_menu_attach_to_widget (GTK_MENU (menu), arrow, NULL);
  gtk_box_pack_start (GTK_BOX (hbox2), GTK_WIDGET (arrow), FALSE, FALSE, 0);

  gtk_box_pack_start (GTK_BOX (hbox), align, FALSE, FALSE, 0);

  // The send message button
  align = gtk_alignment_new (1.0, 0.5, 0.0, 0.0);
  self->priv->send_button = gtk_button_new ();
  gtk_button_set_relief (GTK_BUTTON (self->priv->send_button), GTK_RELIEF_NONE);
  image = gtk_image_new_from_stock (GTK_STOCK_JUMP_TO, 
                                    GTK_ICON_SIZE_MENU);
  hbox2 = gtk_hbox_new (FALSE, 0);
  label = gtk_label_new (NULL);
  gtk_label_set_markup_with_mnemonic (GTK_LABEL (label), _("_Send"));
  gtk_box_pack_start (GTK_BOX (hbox2), image, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox2), label, FALSE, FALSE, 6);
  gtk_container_add (GTK_CONTAINER (self->priv->send_button), hbox2);
  gtk_container_add (GTK_CONTAINER (align), self->priv->send_button);
  gtk_box_pack_start (GTK_BOX (hbox), align, TRUE, TRUE, 0);

  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

  /* Decent size for the pane */
  gtk_widget_set_size_request (GTK_WIDGET (vbox), 150, -1);
  gtk_paned_pack2 (GTK_PANED (vpane), frame, FALSE, FALSE);

  gtk_container_add (GTK_CONTAINER (self), vpane);
  gtk_widget_show_all (GTK_WIDGET (self));

  /* Regex init */
  for (int i = 0 ; i < 2 ; i++) {

    if (i == 0)
      buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (self->priv->conversation));
    else
      buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (self->priv->message));

    // FIXME Add more regexes (call, http, ftp, ...)
    regex_tag = gtk_text_buffer_create_tag (buffer, "smileys", "foreground", "grey", NULL);
    if (gtk_text_tag_set_regex (regex_tag, gtk_text_buffer_get_smiley_regex ()))
      gtk_text_tag_set_regex_display (regex_tag, gtk_text_buffer_insert_smiley);

    regex_tag = gtk_text_buffer_create_tag (buffer, "bold", "weight", PANGO_WEIGHT_BOLD, NULL);
    if (gtk_text_tag_set_regex (regex_tag, "(<b>.*</b>|<B>.*</B>)"))
      gtk_text_tag_set_regex_display (regex_tag, gtk_text_buffer_insert_markup);

    regex_tag = gtk_text_buffer_create_tag (buffer, "italic", "style", PANGO_STYLE_ITALIC, NULL);
    if (gtk_text_tag_set_regex (regex_tag, "(<i>.*</i>|<I>.*</I>)"))
      gtk_text_tag_set_regex_display (regex_tag, gtk_text_buffer_insert_markup);

    regex_tag = gtk_text_buffer_create_tag (buffer, "underline", "underline", PANGO_UNDERLINE_SINGLE, NULL);
    if (gtk_text_tag_set_regex (regex_tag, "(<u>.*</u>|<U>.*</U>)"))
      gtk_text_tag_set_regex_display (regex_tag, gtk_text_buffer_insert_markup);
  }

  /* The GTK Notebook page label */
  self->priv->uri = uri;
  self->priv->tab = gtk_hbox_new (FALSE, 6);
  gtk_container_set_border_width (GTK_CONTAINER (self->priv->tab), 3);
  vbox = gtk_vbox_new (FALSE, 0);

  align = gtk_alignment_new (0.0, 0.5, 0.0, 0.0);
  self->priv->tab_image = gtk_image_new ();
  gtk_image_set_from_stock (GTK_IMAGE (self->priv->tab_image), "presence-unknown", GTK_ICON_SIZE_MENU);
  gtk_container_add (GTK_CONTAINER (align), self->priv->tab_image);
  gtk_box_pack_start (GTK_BOX (self->priv->tab), align, FALSE, FALSE, 0);

  self->priv->tab_label_name = gtk_label_new (display_name.c_str ());
  self->priv->display_name = display_name;
  align = gtk_alignment_new (0.0, 0.0, 0.0, 0.0);
  gtk_container_add (GTK_CONTAINER (align), self->priv->tab_label_name);
  gtk_box_pack_start (GTK_BOX (vbox), align, FALSE, FALSE, 0);

  self->priv->tab_label_status = gtk_label_new (NULL);
  attr_lst = pango_attr_list_new ();
  attr = pango_attr_scale_new (PANGO_SCALE_SMALL);   
  pango_attr_list_insert (attr_lst, attr);
  gdk_color_parse ("darkgray", &color_fg);
  attr = pango_attr_foreground_new (color_fg.red, color_fg.green, color_fg.blue);   
  pango_attr_list_insert (attr_lst, attr);
  gtk_label_set_attributes (GTK_LABEL(self->priv->tab_label_status), attr_lst);
  pango_attr_list_unref (attr_lst);
  align = gtk_alignment_new (0.0, 0.0, 0.0, 0.0);
  gtk_container_add (GTK_CONTAINER (align), self->priv->tab_label_status);
  gtk_box_pack_start (GTK_BOX (vbox), align, FALSE, FALSE, 0);

  align = gtk_alignment_new (0.0, 0.0, 0.0, 0.0);
  gtk_container_add (GTK_CONTAINER (align), vbox);
  gtk_box_pack_start (GTK_BOX (self->priv->tab), align, FALSE, FALSE, 0);

  align = gtk_alignment_new (1.0, 0.5, 0.0, 0.0);
  close_button = gtk_button_new ();
  gtk_button_set_relief (GTK_BUTTON (close_button), GTK_RELIEF_NONE);
  gtk_button_set_focus_on_click (GTK_BUTTON (close_button), FALSE);
  close_image = gtk_image_new_from_stock (GTK_STOCK_CLOSE,
                                          GTK_ICON_SIZE_MENU); 
  gtk_container_add (GTK_CONTAINER (close_button), close_image);

  rc_style = gtk_widget_get_modifier_style (close_button);
  rc_style->xthickness = 0;
  rc_style->ythickness = 0;
  gtk_widget_modify_style (close_button, rc_style);

  gtk_container_add (GTK_CONTAINER (align), close_button);
  gtk_box_pack_start (GTK_BOX (self->priv->tab), align, TRUE, TRUE, 0);
  gtk_widget_show_all (self->priv->tab);

  /* GTK+ signals callbacks */
  g_signal_connect (G_OBJECT (self->priv->smiley_button), "clicked",
                    G_CALLBACK (smiley_button_clicked_cb), menu);

  g_signal_connect (GTK_OBJECT (self->priv->message), "key-press-event",
                    G_CALLBACK (chat_entry_activated_cb), self);

  g_signal_connect (GTK_OBJECT (self->priv->message), "backspace",
                    G_CALLBACK (chat_entry_backspace_cb), NULL);

  g_signal_connect (GTK_OBJECT (self->priv->send_button), "clicked",
                    G_CALLBACK (send_button_clicked_cb), self);

  g_signal_connect (GTK_OBJECT (close_button), "clicked",
                    G_CALLBACK (close_button_clicked_cb), 
                    self);

  /* Engine Signals callbacks */
  presence_core = dynamic_cast<Ekiga::PresenceCore *>(core.get ("presence-core"));
  conn = presence_core->cluster_added.connect (sigc::bind (sigc::ptr_fun (on_cluster_added), (gpointer) self));
  self->priv->connections.push_back (conn);

  conn = presence_core->heap_added.connect (sigc::bind (sigc::ptr_fun (on_heap_added), (gpointer) self));
  self->priv->connections.push_back (conn);

  conn = presence_core->presentity_added.connect (sigc::bind (sigc::ptr_fun (on_presentity_added), (gpointer) self));
  self->priv->connections.push_back (conn);

  conn = presence_core->presentity_updated.connect (sigc::bind (sigc::ptr_fun (on_presentity_updated), self));
  self->priv->connections.push_back (conn);

  conn = presence_core->presentity_removed.connect (sigc::bind (sigc::ptr_fun (on_presentity_removed), (gpointer) self));
  self->priv->connections.push_back (conn);

  presence_core->visit_clusters (sigc::bind_return (sigc::bind (sigc::ptr_fun (on_cluster_added), (gpointer) self), true));

  return GTK_WIDGET (self);
}


GtkWidget *
chat_window_page_get_label (ChatWindowPage *page)
{
  ChatWindowPage *self = NULL;

  g_return_val_if_fail (page != NULL, NULL);

  self = CHAT_WINDOW_PAGE (page);

  return self->priv->tab;
}


const std::string
chat_window_page_get_uri (ChatWindowPage *page)
{
  ChatWindowPage *self = NULL;

  g_return_val_if_fail (page != NULL, "");

  self = CHAT_WINDOW_PAGE (page);

  return self->priv->uri;
}


const std::string
chat_window_page_get_display_name (ChatWindowPage *page)
{
  ChatWindowPage *self = NULL;

  g_return_val_if_fail (page != NULL, "");

  self = CHAT_WINDOW_PAGE (page);

  return self->priv->display_name;
}


void
chat_window_page_add_message (ChatWindowPage *page,
                              const std::string display_name,
                              G_GNUC_UNUSED const std::string uri,
                              const std::string message,
                              gboolean is_sent)
{
  ChatWindowPage *self = CHAT_WINDOW_PAGE (page);

  GtkTextIter iter;
  GtkTextBuffer *buffer = NULL;
  GtkTextMark *mark = NULL;
  
  gchar *msg = NULL;
  gboolean same_user = false;

  GTimeVal timeval;
  GDate date;
  gchar time_buffer [20];

  g_return_if_fail (page != NULL);

  /* Get iter */
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (self->priv->conversation));
  gtk_text_buffer_get_end_iter (buffer, &iter);

  if (is_sent) {
    same_user = (self->priv->last_user == 0);
    self->priv->last_user = 0;
  }
  else {
    same_user = (self->priv->last_user == 1);
    self->priv->last_user = 1;
  }

  /* Insert user name */
  if (!same_user) {
    if (is_sent) {
      msg = g_strdup_printf (_("You say:\n"));
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, msg, 
                                                -1, "local-user", NULL);
    }
    else {
      msg = g_strdup_printf ("%s %s\n", display_name.c_str (), _("says:"));
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, msg, 
                                                -1, "remote-user", NULL);
    }
    g_free (msg);
  }

  /* Insert body */
  g_get_current_time (&timeval);
  g_date_set_time_val (&date, &timeval);
  strftime (time_buffer, sizeof (time_buffer), "%H:%M", localtime (&timeval.tv_sec));
  msg = g_strdup_printf ("[%s]: ", time_buffer);
  gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, msg, 
                                            -1, "timestamp", NULL);
  gtk_text_buffer_insert_with_regex (buffer, &iter, message.c_str ());
  g_free (msg);
  
  gtk_text_buffer_insert (buffer, &iter, "\n", -1);

  /* Auto-scroll */
  mark = gtk_text_buffer_get_mark (buffer, "current-position");
  gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (self->priv->conversation), mark, 
                                0.0, FALSE, 0,0);
}


void
chat_window_page_add_error (ChatWindowPage *page,
                            G_GNUC_UNUSED const std::string uri,
                            const std::string message)
{
  ChatWindowPage *self = CHAT_WINDOW_PAGE (page);

  GdkColor color;

  GtkTextIter iter;
  GtkTextBuffer *buffer = NULL;
  GtkTextMark *mark = NULL;

  g_return_if_fail (page != NULL);
  
  gdk_color_parse ("red", &color);

  /* Get iter */
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (self->priv->conversation));
  gtk_text_buffer_get_end_iter (buffer, &iter);

  /* Insert body */
  gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, message.c_str (), 
                                            -1, "error", NULL);
  gtk_text_buffer_insert (buffer, &iter, "\n", -1);

  /* Auto-scroll */
  mark = gtk_text_buffer_get_mark (buffer, "current-position");
  gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (self->priv->conversation), mark, 
                                0.0, FALSE, 0,0);
}


void 
chat_window_page_set_unread_messages (ChatWindowPage *page,
                                      int messages)
{
  ChatWindowPage *self = CHAT_WINDOW_PAGE (page);

  PangoFontDescription *description = NULL;
  GdkColor color;

  g_return_if_fail (page != NULL);

  if (messages > 0) {

    description = pango_font_description_new ();
    pango_font_description_set_weight (description, PANGO_WEIGHT_BOLD);

    gdk_color_parse ("blue", &color);

    gtk_widget_modify_fg (GTK_WIDGET (self->priv->tab_label_name), GTK_STATE_ACTIVE, &color);
    gtk_widget_modify_font (GTK_WIDGET (self->priv->tab_label_name), description);

    pango_font_description_free (description);
  }
  else {

    gtk_widget_modify_fg (GTK_WIDGET (self->priv->tab_label_name), GTK_STATE_ACTIVE, NULL);
    gtk_widget_modify_font (GTK_WIDGET (self->priv->tab_label_name), NULL);
  }

  self->priv->unread_messages = messages;
}


int
chat_window_page_get_unread_messages (ChatWindowPage *page)
{
  ChatWindowPage *self = CHAT_WINDOW_PAGE (page);

  g_return_val_if_fail (page != NULL, -1);

  return self->priv->unread_messages;
}


void
chat_window_page_grab_focus (ChatWindowPage *page)
{
  ChatWindowPage *self = CHAT_WINDOW_PAGE (page);

  g_return_if_fail (page != NULL);

  gtk_widget_grab_focus (self->priv->message);
}


