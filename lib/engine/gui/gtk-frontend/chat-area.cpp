
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2008 Damien Sandras
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
 *                        chat-area.cpp  -  description
 *                         --------------------------------
 *   begin                : written in july 2008 by Julien Puydt
 *   copyright            : (C) 2008 by Julien Puydt
 *                          (C) 2008 by Jan Schampera
 *   description          : Implementation of a Chat area (view and control)
 *
 */

#include "config.h"

#include "chat-area.h"
#include "gm-text-buffer-enhancer.h"
#include "gm-text-anchored-tag.h"
#include "gm-text-smiley.h"
#include "gm-smileys.h"

#include <string.h>
#include <stdarg.h>

class ChatAreaHelper;

struct _ChatAreaPrivate
{
  Ekiga::Chat* chat;
  sigc::connection connection;
  ChatAreaHelper* helper;
  GmTextBufferEnhancer* enhancer;
  GtkWidget* smiley_menu;

  /* we contain those, so no need to unref them */
  GtkWidget* scrolled_text_window;
  GtkWidget* text_view;
  GtkWidget* entry;
};

enum {
  CHAT_AREA_PROP_CHAT = 1
};

enum {
  MESSAGE_NOTICE_EVENT,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static GObjectClass* parent_class = NULL;

/* declaration of internal api */

static void chat_area_add_notice (ChatArea* self,
				  const gchar* txt);

static void chat_area_add_message (ChatArea* self,
				   const gchar* from,
				   const gchar* txt);

/* declaration of the helping observer */
class ChatAreaHelper: public Ekiga::ChatObserver
{
public:
  ChatAreaHelper (ChatArea* area_): area(area_)
  {}

  ~ChatAreaHelper ()
  {}

  void message (const std::string from,
		const std::string msg)
  { chat_area_add_message (area, from.c_str (), msg.c_str ()); }

  void notice (const std::string msg)
  { chat_area_add_notice (area, msg.c_str ()); }

private:
  ChatArea* area;
};

/* a helper to shorten tag definitions
 * FIXME when C99 finally is supported everywhere, this
 * can be a variadic macro
 */

static void gm_chat_area_define_simple_text_tag (GtkTextBuffer*,
						 GmTextBufferEnhancer*,
						 const gchar*,
						 const gchar*,
						 const gchar*,
						 const gchar*,
						 ...);

/* declaration of callbacks */

static void on_smiley_activated (GtkMenuItem *item,
				 gpointer data);

static void on_smiley_clicked (GtkButton* button,
			       gpointer data);

static void on_bold_clicked (GtkButton* button,
			     gpointer data);

static void on_italic_clicked (GtkButton* button,
			       gpointer data);

static void on_underline_clicked (GtkButton* button,
				  gpointer data);

static void on_entry_activated (GtkWidget* entry,
				gpointer data);

static void on_chat_removed (ChatArea* self);

/* implementation of internal api */

static void
gm_chat_area_define_simple_text_tag (GtkTextBuffer* buffer,
				     GmTextBufferEnhancer* enhancer,
				     const gchar* tag_name,
				     const gchar* opening_tag,
				     const gchar* closing_tag,
				     const gchar* first_property_name,
				     ...)
{
  va_list args;
  GtkTextTag* tag = NULL;
  GmTextBufferEnhancerHelperIFace* helper = NULL;
  gchar* tmp_tagstring = NULL;

  g_return_if_fail (buffer != NULL);
  g_return_if_fail (enhancer != NULL);
  g_return_if_fail (opening_tag != NULL);
  g_return_if_fail (closing_tag != NULL);

  va_start (args, first_property_name);
  tag = gtk_text_buffer_create_tag (buffer, tag_name,
				    NULL);

  if (first_property_name)
    g_object_set_valist (G_OBJECT (tag), first_property_name,
			 args);
  va_end (args);

  /* the OPENING tag */
  tmp_tagstring = g_strdup (opening_tag);
  helper = gm_text_anchored_tag_new (tmp_tagstring, tag, TRUE);
  gm_text_buffer_enhancer_add_helper (enhancer, helper);
  g_object_unref (helper);
  g_free (tmp_tagstring);
  /* the CLOSING tag */
  tmp_tagstring = g_strdup (closing_tag);
  helper = gm_text_anchored_tag_new (tmp_tagstring, tag, FALSE);
  gm_text_buffer_enhancer_add_helper (enhancer, helper);
  g_object_unref (helper);
  g_free (tmp_tagstring);

  /* FIXME shouldn't 'tag' be unref'd? it wasn't in the original code
   * so i didn't do it here, too - TheBonsai */
}


static void
chat_area_add_notice (ChatArea* self,
		      const gchar* txt)
{
  gchar* str = NULL;
  GtkTextBuffer* buffer = NULL;
  GtkTextIter iter;

  str = g_strdup_printf ("NOTICE: %s\n", txt);
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (self->priv->text_view));
  gtk_text_buffer_get_end_iter (buffer, &iter);
  gm_text_buffer_enhancer_insert_text (self->priv->enhancer, &iter,
				       str, -1);
  g_free (str);

  g_signal_emit (self, signals[MESSAGE_NOTICE_EVENT], 0);
}

static void
chat_area_add_message (ChatArea* self,
		       const gchar* from,
		       const gchar* txt)
{
  gchar* str = NULL;
  GtkTextBuffer* buffer = NULL;
  GtkTextIter iter;

  str = g_strdup_printf ("<i>%s %s</i> %s\n", from, _("says:"), txt);
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (self->priv->text_view));
  gtk_text_buffer_get_end_iter (buffer, &iter);
  gm_text_buffer_enhancer_insert_text (self->priv->enhancer, &iter,
				       str, -1);
  g_free (str);

  g_signal_emit (self, signals[MESSAGE_NOTICE_EVENT], 0);
}

/* implementation of callbacks */

static void
on_smiley_activated (GtkMenuItem *item,
		     gpointer data)
{
  const gchar* text = NULL;
  ChatArea* self = NULL;
  gint position;

  self = (ChatArea*)data;

  /* FIXME: that will break when gtk+ will change... */
  text = gtk_label_get_text (GTK_LABEL(GTK_BIN(GTK_MENU_ITEM (item))->child));

  position = gtk_editable_get_position (GTK_EDITABLE (self->priv->entry));

  gtk_editable_insert_text (GTK_EDITABLE (self->priv->entry),
			    text, strlen (text),
			    &position);
}

static void
on_smiley_clicked (G_GNUC_UNUSED GtkButton* button,
		   gpointer data)
{
  ChatArea* self = NULL;

  self = (ChatArea*)data;

  gtk_menu_popup (GTK_MENU (self->priv->smiley_menu),
		  NULL, NULL, NULL, NULL, 0, 0);
}

static void
on_bold_clicked (G_GNUC_UNUSED GtkButton* button,
		 gpointer data)
{
  ChatArea* self = NULL;
  gint start;
  gint end;
  gint position;

  self = (ChatArea*)data;

  if (gtk_editable_get_selection_bounds (GTK_EDITABLE (self->priv->entry),
					 &start, & end)) {
    gtk_editable_insert_text (GTK_EDITABLE (self->priv->entry),
			      "</b>", 4, &end);
    gtk_editable_insert_text (GTK_EDITABLE (self->priv->entry),
			      "<b>", 3, &start);
    gtk_editable_select_region (GTK_EDITABLE (self->priv->entry),
    				start, end - 1);
  } else {

    position = gtk_editable_get_position (GTK_EDITABLE (self->priv->entry));
    gtk_editable_insert_text (GTK_EDITABLE (self->priv->entry),
			      "<b></b>", 7, &position);
    gtk_editable_set_position (GTK_EDITABLE (self->priv->entry),
			       position - 4);
  }
}

static void
on_italic_clicked (G_GNUC_UNUSED GtkButton* button,
		   gpointer data)
{
  ChatArea* self = NULL;
  gint start;
  gint end;
  gint position;

  self = (ChatArea*)data;

  if (gtk_editable_get_selection_bounds (GTK_EDITABLE (self->priv->entry),
					 &start, & end)) {
    gtk_editable_insert_text (GTK_EDITABLE (self->priv->entry),
			      "</i>", 4, &end);
    gtk_editable_insert_text (GTK_EDITABLE (self->priv->entry),
			      "<i>", 3, &start);
    gtk_editable_select_region (GTK_EDITABLE (self->priv->entry),
    				start, end - 1);
  } else {

    position = gtk_editable_get_position (GTK_EDITABLE (self->priv->entry));
    gtk_editable_insert_text (GTK_EDITABLE (self->priv->entry),
			      "<i></i>", 7, &position);
    gtk_editable_set_position (GTK_EDITABLE (self->priv->entry),
			       position - 4);
  }
}

static void
on_underline_clicked (G_GNUC_UNUSED GtkButton* button,
		      gpointer data)
{
  ChatArea* self = NULL;
  gint start;
  gint end;
  gint position;

  self = (ChatArea*)data;

  if (gtk_editable_get_selection_bounds (GTK_EDITABLE (self->priv->entry),
					 &start, & end)) {
    gtk_editable_insert_text (GTK_EDITABLE (self->priv->entry),
			      "</u>", 4, &end);
    gtk_editable_insert_text (GTK_EDITABLE (self->priv->entry),
			      "<u>", 3, &start);
    gtk_editable_select_region (GTK_EDITABLE (self->priv->entry),
    				start, end - 1);
  } else {

    position = gtk_editable_get_position (GTK_EDITABLE (self->priv->entry));
    gtk_editable_insert_text (GTK_EDITABLE (self->priv->entry),
			      "<u></u>", 7, &position);
    gtk_editable_set_position (GTK_EDITABLE (self->priv->entry),
			       position - 4);
  }
}

static void
on_entry_activated (GtkWidget* entry,
		    gpointer data)
{
  ChatArea* self = NULL;
  const gchar* text = NULL;

  self = CHAT_AREA (data);

  text = gtk_entry_get_text (GTK_ENTRY (entry));

  if (text != NULL && !g_str_equal (text, "")) {

    if (self->priv->chat->send_message (text))
      gtk_entry_set_text (GTK_ENTRY (entry), "");
  }
}

static void
on_chat_removed (ChatArea* self)
{
  gtk_widget_hide (self->priv->entry);
}

/* GObject code */

static void
chat_area_dispose (GObject* obj)
{
  ChatArea* self = NULL;

  self = (ChatArea*)obj;

  if (self->priv->enhancer != NULL) {

    g_object_unref (self->priv->enhancer);
    self->priv->enhancer = NULL;
  }

  if (self->priv->smiley_menu != NULL) {

    g_object_unref (self->priv->smiley_menu);
    self->priv->smiley_menu = NULL;
  }

  parent_class->dispose (obj);
}

static void
chat_area_finalize (GObject* obj)
{
  ChatArea* self = NULL;

  self = (ChatArea*)obj;

  if (self->priv->chat) {

    self->priv->connection.disconnect ();
    if (self->priv->helper) {
      self->priv->chat->disconnect (*(self->priv->helper));
      delete self->priv->helper;
      self->priv->helper = NULL;
    }
    self->priv->chat = NULL;
  }

  parent_class->finalize (obj);
}

static void
chat_area_get_property (GObject* obj,
			guint prop_id,
			GValue* value,
			GParamSpec* spec)
{
  ChatArea* self = NULL;

  self = (ChatArea*)obj;

  switch (prop_id) {

  case CHAT_AREA_PROP_CHAT:
    g_value_set_pointer (value, self->priv->chat);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, spec);
    break;
  }
}

static void
chat_area_set_property (GObject* obj,
			guint prop_id,
			const GValue* value,
			GParamSpec* spec)
{
  ChatArea* self = NULL;
  gpointer ptr = NULL;

  self = (ChatArea* )obj;

  switch (prop_id) {

  case CHAT_AREA_PROP_CHAT:
    ptr = g_value_get_pointer (value);
    self->priv->chat = (Ekiga::Chat *)ptr;
    self->priv->connection = self->priv->chat->removed.connect (sigc::bind (sigc::ptr_fun (on_chat_removed), self));
    self->priv->helper = new ChatAreaHelper (self);
    self->priv->chat->connect (*(self->priv->helper));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, spec);
    break;
  }
}

static void
chat_area_class_init (gpointer g_class,
		      G_GNUC_UNUSED gpointer class_data)
{
  ChatAreaClass* chat_area_class = NULL;
  GObjectClass* gobject_class = NULL;
  GParamSpec* spec = NULL;

  parent_class = (GObjectClass*)g_type_class_peek_parent (g_class);

  g_type_class_add_private (g_class, sizeof (ChatAreaPrivate));

  gobject_class = (GObjectClass*)g_class;
  gobject_class->dispose = chat_area_dispose;
  gobject_class->finalize = chat_area_finalize;
  gobject_class->get_property = chat_area_get_property;
  gobject_class->set_property = chat_area_set_property;

  spec = g_param_spec_pointer ("chat",
			       "displayed chat",
			       "Displayed chat",
			       (GParamFlags)(G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
  g_object_class_install_property (gobject_class,
				   CHAT_AREA_PROP_CHAT,
				   spec);

  signals[MESSAGE_NOTICE_EVENT] =
    g_signal_new ("message-notice-event",
		  G_OBJECT_CLASS_TYPE (gobject_class),
		  G_SIGNAL_RUN_LAST,
		  G_STRUCT_OFFSET (ChatAreaClass, message_notice_event),
		  NULL, NULL,
		  g_cclosure_marshal_VOID__VOID,
		  G_TYPE_NONE, 0);

  /* FIXME: is it useful? */
  chat_area_class = (ChatAreaClass*)g_class;
  chat_area_class->message_notice_event = NULL;
}

static void
chat_area_init (GTypeInstance* instance,
		G_GNUC_UNUSED gpointer g_class)
{
  ChatArea* self = NULL;
  GtkTextBuffer* buffer = NULL;
  GmTextBufferEnhancerHelperIFace* helper = NULL;
  GtkTextTag* tag = NULL;

  self = (ChatArea*)instance;

  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
					    TYPE_CHAT_AREA,
					    ChatAreaPrivate);
  self->priv->chat = NULL;

  /* first the area has a text view to display
     the GtkScrolledWindow is there to make
     the GtkTextView scrollable */

  self->priv->scrolled_text_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy
    (GTK_SCROLLED_WINDOW (self->priv->scrolled_text_window),
     GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

  self->priv->text_view = gtk_text_view_new ();
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (self->priv->text_view));

  gtk_text_view_set_editable (GTK_TEXT_VIEW (self->priv->text_view), FALSE);
  gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (self->priv->text_view),
				    FALSE);
  gtk_text_view_set_justification (GTK_TEXT_VIEW (self->priv->text_view),
				   GTK_JUSTIFY_LEFT);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (self->priv->text_view),
			       GTK_WRAP_WORD);
  gtk_text_view_set_left_margin (GTK_TEXT_VIEW (self->priv->text_view),
				 2);
  gtk_text_view_set_right_margin (GTK_TEXT_VIEW (self->priv->text_view),
				  2);

  gtk_container_add (GTK_CONTAINER (self->priv->scrolled_text_window),
		     self->priv->text_view);
  gtk_box_pack_start (GTK_BOX (self),
		      self->priv->scrolled_text_window, TRUE, TRUE, 2);
  gtk_widget_show_all (self->priv->scrolled_text_window);

  /* then we want to enhance this display */

  self->priv->enhancer = gm_text_buffer_enhancer_new (buffer);

  helper = gm_text_smiley_new ();
  gm_text_buffer_enhancer_add_helper (self->priv->enhancer, helper);
  g_object_unref (helper);

  gm_chat_area_define_simple_text_tag (buffer, self->priv->enhancer,
				       "bold", "<b>", "</b>",
				       "weight", PANGO_WEIGHT_BOLD,
				       NULL);

  gm_chat_area_define_simple_text_tag (buffer, self->priv->enhancer,
				       "italic", "<i>", "</i>",
				       "style", PANGO_STYLE_ITALIC,
				       NULL);

  gm_chat_area_define_simple_text_tag (buffer, self->priv->enhancer,
				       "underline", "<u>", "</u>",
				       "underline", PANGO_UNDERLINE_SINGLE,
				       NULL);

  gm_chat_area_define_simple_text_tag (buffer, self->priv->enhancer,
				       "col_black", "<color=black>", "</color>",
				       "foreground", "#000000",
				       NULL);

  gm_chat_area_define_simple_text_tag (buffer, self->priv->enhancer,
                                       "col_white", "<color=white>", "</color>",
                                       "foreground", "#FFFFFF",
                                       NULL);

  gm_chat_area_define_simple_text_tag (buffer, self->priv->enhancer,
                                       "col_red", "<color=red>", "</color>",
                                       "foreground", "#FF0000",
                                       NULL);

  gm_chat_area_define_simple_text_tag (buffer, self->priv->enhancer,
                                       "col_cyan", "<color=cyan>", "</color>",
                                       "foreground", "#00FFFF",
                                       NULL);

  gm_chat_area_define_simple_text_tag (buffer, self->priv->enhancer,
                                       "col_green", "<color=green>", "</color>",
                                       "foreground", "#00FF00",
                                       NULL);

  gm_chat_area_define_simple_text_tag (buffer, self->priv->enhancer,
                                       "col_magenta", "<color=magenta>", "</color>",
                                       "foreground", "#FF00FF",
                                       NULL);

  gm_chat_area_define_simple_text_tag (buffer, self->priv->enhancer,
                                       "col_blue", "<color=blue>", "</color>",
                                       "foreground", "#0000FF",
                                       NULL);

  gm_chat_area_define_simple_text_tag (buffer, self->priv->enhancer,
                                       "col_yellow", "<color=yellow>", "</color>",
                                       "foreground", "#FFFF00",
                                       NULL);


  /* and finally the chat area has a nice entry system */
  GtkWidget* vbox = NULL;
  GtkWidget* bbox = NULL;
  GtkWidget* button = NULL;
  const gchar** smileys = gm_get_smileys ();
  gint smiley;
  GdkPixbuf* pixbuf = NULL;
  GtkWidget* image = NULL;
  GtkWidget* smiley_item = NULL;

  /* we need to build a nice menu for smileys */
  self->priv->smiley_menu = gtk_menu_new ();
  g_object_ref (self->priv->smiley_menu);
  for (smiley = 0;
       smileys[smiley] != NULL;
       smiley = smiley + 2) {

    smiley_item = gtk_image_menu_item_new_with_label (smileys[smiley]);
    pixbuf = gtk_icon_theme_load_icon (gtk_icon_theme_get_default (),
				       smileys[smiley + 1], 16,
				       (GtkIconLookupFlags)0, NULL);
    image = gtk_image_new_from_pixbuf (pixbuf);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (smiley_item),
				   image);
    gtk_widget_show_all (smiley_item);
    gtk_menu_shell_append (GTK_MENU_SHELL (self->priv->smiley_menu),
			   smiley_item);

    g_signal_connect (G_OBJECT (smiley_item), "activate",
		      G_CALLBACK (on_smiley_activated), self);
  }

  vbox = gtk_vbox_new (FALSE, 2);
  gtk_box_pack_end (GTK_BOX (self), vbox,
		    FALSE, TRUE, 2);
  gtk_widget_show (vbox);

  bbox = gtk_hbutton_box_new ();
  /* FIXME gtk_box_set_spacing() seems to be neccesary, though we
     define a padding with the pack() methods */
  /* FIXME the box doesn't do the 2px space at the left and right edges! */
  gtk_box_set_spacing (GTK_BOX (bbox), 2);
  gtk_box_pack_start (GTK_BOX (vbox), bbox,
		      FALSE, TRUE, 2);
  gtk_widget_show (bbox);

  button = gtk_button_new_from_stock (GTK_STOCK_INFO); // FIXME
  gtk_button_set_focus_on_click (GTK_BUTTON (button), FALSE);
  g_signal_connect (G_OBJECT (button), "clicked",
		    G_CALLBACK (on_smiley_clicked), self);
  gtk_box_pack_start (GTK_BOX (bbox), button,
		      FALSE, TRUE, 2);
  gtk_widget_show (button);

  button = gtk_button_new_from_stock (GTK_STOCK_BOLD);
  gtk_button_set_focus_on_click (GTK_BUTTON (button), FALSE);
  g_signal_connect (G_OBJECT (button), "clicked",
		    G_CALLBACK (on_bold_clicked), self);
  gtk_box_pack_start (GTK_BOX (bbox), button,
		      FALSE, TRUE, 2);
  gtk_widget_show (button);

  button = gtk_button_new_from_stock (GTK_STOCK_ITALIC);
  gtk_button_set_focus_on_click (GTK_BUTTON (button), FALSE);
  g_signal_connect (G_OBJECT (button), "clicked",
		    G_CALLBACK (on_italic_clicked), self);
  gtk_box_pack_start (GTK_BOX (bbox), button,
		      FALSE, TRUE, 2);
  gtk_widget_show (button);

  button = gtk_button_new_from_stock (GTK_STOCK_UNDERLINE);
  gtk_button_set_focus_on_click (GTK_BUTTON (button), FALSE);
  g_signal_connect (G_OBJECT (button), "clicked",
		    G_CALLBACK (on_underline_clicked), self);
  gtk_box_pack_start (GTK_BOX (bbox), button,
		      FALSE, TRUE, 2);
  gtk_widget_show (button);

  self->priv->entry = gtk_entry_new ();
  gtk_box_pack_start (GTK_BOX (vbox), self->priv->entry,
		      FALSE, TRUE, 2);
  g_signal_connect (self->priv->entry, "activate",
		    G_CALLBACK (on_entry_activated), self);
  gtk_widget_show (self->priv->entry);
}


GType
chat_area_get_type ()
{
  static GType result = 0;

  if (result == 0) {

    static const GTypeInfo info = {
      sizeof (ChatAreaClass),
      NULL,
      NULL,
      chat_area_class_init,
      NULL,
      NULL,
      sizeof (ChatArea),
      0,
      chat_area_init,
      NULL
    };

    result = g_type_register_static (GTK_TYPE_VBOX,
				     "ChatArea",
				     &info, (GTypeFlags) 0);
  }

  return result;
}

/* public api */

GtkWidget*
chat_area_new (Ekiga::Chat& chat)
{
  return (GtkWidget*)g_object_new (TYPE_CHAT_AREA,
				   "chat", &chat,
				   NULL);
}

const std::string
chat_area_get_title (ChatArea* area)
{
  return area->priv->chat->get_title ();
}
