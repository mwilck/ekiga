
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

#include "sip-endpoint.h"

#include "ekiga.h"

#include <gdk/gdkkeysyms.h>


struct _ChatWindowPagePrivate
{
  _ChatWindowPagePrivate (Ekiga::ServiceCore & _core) : core (_core) { }
  GtkWidget *send_button;
  GtkWidget *conversation;
  GtkWidget *message;
  GtkWidget *tab_label;
  GtkWidget *tab;

  std::string uri;

  int last_user;

  std::vector<sigc::connection> connections;
  Ekiga::ServiceCore & core;
};

static GObjectClass *parent_class = NULL;


static void close_button_clicked_cb (GtkWidget *w,
                                     gpointer data);

static gboolean chat_entry_activated_cb (GtkWidget *w,
                                         GdkEventKey *key,
                                         gpointer data);

static void send_button_clicked_cb (GtkWidget *w,
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
  self->priv->conversation = NULL;
  self->priv->message = NULL;
  self->priv->tab_label = NULL;
  self->priv->tab = NULL;
  self->priv->last_user = 0;

  parent_class->dispose (obj);
}


static void
chat_window_page_finalize (GObject *obj)
{
  ChatWindowPage *self = NULL;

  self = CHAT_WINDOW_PAGE (obj);

  delete self->priv;
  parent_class->finalize (obj);
}


static void
chat_window_page_class_init (gpointer g_class,
                             gpointer class_data)
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
static void 
close_button_clicked_cb (GtkWidget *w,
                         gpointer data)
{
  g_signal_emit_by_name (data, "close-event", NULL);
}


static gboolean 
chat_entry_activated_cb (GtkWidget *w,
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
send_button_clicked_cb (GtkWidget *w,
                        gpointer data)
{
  ChatWindowPage *self = CHAT_WINDOW_PAGE (data);

  GtkTextIter start_iter, end_iter;
  GtkTextBuffer *buffer = NULL;

  gchar *body = NULL;
  std::string message;

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (self->priv->message));
  gtk_text_buffer_get_start_iter (GTK_TEXT_BUFFER (buffer), &start_iter);
  gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (buffer), &end_iter);
  body = gtk_text_buffer_get_text (GTK_TEXT_BUFFER (buffer), &start_iter, &end_iter, FALSE);
  gtk_text_buffer_delete (GTK_TEXT_BUFFER (buffer), &start_iter, &end_iter);

  message = body;
  g_free (body);

  /* */ //FIXME protocl
  if (!message.empty ()) {
    SIP::EndPoint *endpoint =  dynamic_cast<SIP::EndPoint*>(self->priv->core.get ("sip-endpoint"));
    if (endpoint) {
      endpoint->message (self->priv->uri, message);
    }
  }
}


GtkWidget *
chat_window_page_new (Ekiga::ServiceCore & core,
                      const std::string display_name,
                      const std::string uri)
{
  ChatWindowPage *self = NULL;
  
  GtkWidget *close_button = NULL;
  GtkWidget *close_image = NULL;

  GtkWidget *scr = NULL;
  GtkWidget *hbox = NULL;
  GtkWidget *hbox2 = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *label = NULL;
  GtkWidget *image = NULL;
  GtkWidget *frame = NULL;
  GtkWidget *vpane = NULL;
  GtkWidget *align = NULL;

  GtkTextIter iter;
  GtkTextBuffer *buffer = NULL;
  GtkTextMark *mark = NULL;
  GtkTextTag *regex_tag = NULL;

  self = CHAT_WINDOW_PAGE (g_object_new (CHAT_WINDOW_PAGE_TYPE, NULL));
  self->priv = new ChatWindowPagePrivate (core);

  self->priv->send_button = NULL;
  self->priv->conversation = NULL;
  self->priv->message = NULL;
  self->priv->tab_label = NULL;
  self->priv->tab = NULL;
  self->priv->last_user = 0;

  /* Vertical pane to contain the chat and the message to send */
  vpane = gtk_vpaned_new ();
  gtk_container_set_border_width (GTK_CONTAINER (vpane), 4);

  // Above part
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

  // Bottom part
  vbox = gtk_vbox_new (FALSE, 4);

  self->priv->message = gtk_text_view_new ();
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (self->priv->message), 
                               GTK_WRAP_WORD_CHAR);
  self->priv->last_user = -1;
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  scr = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (scr), self->priv->message);
  gtk_container_add (GTK_CONTAINER (frame), scr);
  gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);

  hbox = gtk_hbox_new (FALSE, 4);

  align = gtk_alignment_new (1.0, 0.5, 0.0, 0.0);
  self->priv->send_button = gtk_button_new ();
  image = gtk_image_new_from_stock (GTK_STOCK_JUMP_TO, 
                                    GTK_ICON_SIZE_MENU);
  hbox2 = gtk_hbox_new (FALSE, 0);
  label = gtk_label_new (NULL);
  gtk_label_set_markup_with_mnemonic (GTK_LABEL (label), _("_Send"));
  gtk_box_pack_start (GTK_BOX (hbox2), image, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox2), label, FALSE, FALSE, 6);
  gtk_container_add (GTK_CONTAINER (self->priv->send_button), hbox2);
  gtk_container_add (GTK_CONTAINER (align), self->priv->send_button);
  gtk_box_pack_start (GTK_BOX (hbox), align, FALSE, FALSE, 0);

  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

  // Decent size for the pane
  gtk_widget_set_size_request (GTK_WIDGET (vbox), 150, -1);
  gtk_paned_pack2 (GTK_PANED (vpane), vbox, FALSE, FALSE);

  gtk_container_add (GTK_CONTAINER (self), vpane);
  gtk_widget_show_all (GTK_WIDGET (self));
  /* FIXME
  regex_tag = gtk_text_buffer_create_tag (buffer, "uri-http", "foreground", "blue", "underline", PANGO_UNDERLINE_SINGLE,  NULL);
  if (gtk_text_tag_set_regex (regex_tag, "\\<(http[s]?|[s]?ftp)://[^[:blank:]]+\\>")) 
    gtk_text_tag_add_actions_to_regex (regex_tag, _("_Open URL"), gm_open_uri, _("_Copy URL to Clipboard"), copy_uri_cb, NULL);

  regex_tag = gtk_text_buffer_create_tag (buffer, "uri-gm", "foreground", "blue", "underline", PANGO_UNDERLINE_SINGLE, NULL);
  if (gtk_text_tag_set_regex (regex_tag, "\\<((h323|sip|callto):[^[:blank:]]+)\\>"))
                                                                    gtk_text_tag_add_actions_to_regex (regex_tag, _("C_all Contact"), connect_uri_cb, _("_Copy URI to Clipboard"), copy_uri_cb, NULL);
*/
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

  /* FIXME
  regex_tag = gtk_text_buffer_create_tag (buffer, "latex", "foreground", "grey",NULL);
  if (gtk_text_tag_set_regex (regex_tag, "(\\$[^$]*\\$|\\$\\$[^$]*\\$\\$)"))
    gtk_text_tag_add_actions_to_regex (regex_tag, _("_Copy Equation"), copy_uri_cb, NULL);
*/

  g_signal_connect (GTK_OBJECT (self->priv->message), "key-press-event",
                    G_CALLBACK (chat_entry_activated_cb), self);

  g_signal_connect (GTK_OBJECT (self->priv->send_button), "clicked",
                    G_CALLBACK (send_button_clicked_cb), self);


  /* The GTK Notebook page label */
  self->priv->uri = uri;
  self->priv->tab = gtk_hbox_new (FALSE, 0);	   
  align = gtk_alignment_new (0.0, 0.5, 0.0, 0.0);
  self->priv->tab_label = gtk_label_new (display_name.c_str ());
  gtk_container_add (GTK_CONTAINER (align), self->priv->tab_label);
  gtk_box_pack_start (GTK_BOX (self->priv->tab), align, TRUE, TRUE, 0);

  close_button = gtk_button_new ();
  gtk_button_set_relief (GTK_BUTTON (close_button), GTK_RELIEF_NONE);
  close_image = gtk_image_new_from_stock (GTK_STOCK_CLOSE,
                                          GTK_ICON_SIZE_MENU); 
  gtk_container_add (GTK_CONTAINER (close_button), close_image);
  gtk_widget_set_size_request (close_button, 17, 17);
  gtk_box_pack_start (GTK_BOX (self->priv->tab), close_button, FALSE, FALSE, 0);
  gtk_widget_show_all (self->priv->tab);

  g_signal_connect (GTK_OBJECT (close_button), "clicked",
                    G_CALLBACK (close_button_clicked_cb), 
                    self);

  return GTK_WIDGET (self);
}


GtkWidget *
chat_window_page_get_label (ChatWindowPage *page)
{
  ChatWindowPage *self = NULL;

  self = CHAT_WINDOW_PAGE (page);

  return self->priv->tab;
}


const std::string
chat_window_page_get_uri (ChatWindowPage *page)
{
  ChatWindowPage *self = NULL;

  self = CHAT_WINDOW_PAGE (page);

  return self->priv->uri;
}


void 
chat_window_page_add_message (ChatWindowPage *page,
                              const std::string display_name,
                              const std::string uri,
                              const std::string message,
                              gboolean is_sent)
{
  ChatWindowPage *self = CHAT_WINDOW_PAGE (page);

  GdkColor color;

  GtkTextIter iter;
  GtkTextBuffer *buffer = NULL;
  GtkTextMark *mark = NULL;
  
  gboolean has_focus = false;
  gchar *msg = NULL;

  GTimeVal timeval;
  GDate date;
  gchar time_buffer [20];

  gdk_color_parse ("blue", &color);

  /* Get iter */
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (self->priv->conversation));
  gtk_text_buffer_get_end_iter (buffer, &iter);

  /* Insert user name */
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

  /* check if this page/tab is focused, 
   * if not change the colour of tab name to red */
  g_object_get (G_OBJECT (self->priv->message), "has-focus", &has_focus, NULL);
  if (!has_focus)
    gtk_widget_modify_fg (GTK_WIDGET(self->priv->tab_label), GTK_STATE_ACTIVE, &color);
}


void 
chat_window_page_add_error (ChatWindowPage *page,
                            const std::string uri,
                            const std::string message)
{
  ChatWindowPage *self = CHAT_WINDOW_PAGE (page);

  GdkColor color;

  GtkTextIter iter;
  GtkTextBuffer *buffer = NULL;
  GtkTextMark *mark = NULL;
  
  gboolean has_focus = false;

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

  /* check if this page/tab is focused, 
   * if not change the colour of tab name to red */
  g_object_get (G_OBJECT (self->priv->message), "has-focus", &has_focus, NULL);
  if (!has_focus)
    gtk_widget_modify_fg (GTK_WIDGET(self->priv->tab_label), GTK_STATE_ACTIVE, &color);
}


