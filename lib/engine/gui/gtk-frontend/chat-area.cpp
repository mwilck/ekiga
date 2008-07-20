
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
 *   description          : Implementation of a Chat area (view and control)
 *
 */

#include "chat-area.h"
#include "gm-text-buffer-enhancer.h"
#include "gm-text-anchored-tag.h"
#include "gm-text-smiley.h"
#include "gm-smileys.h"

#include <string.h>

class ChatAreaHelper;

struct _ChatAreaPrivate
{
  Ekiga::Chat* chat;
  sigc::connection connection;
  ChatAreaHelper* helper;
  GmTextBufferEnhancer* enhancer;
  GtkWidget* smiley_menu;

  /* we contain those, so no need to unref them */
  GtkWidget* text_view;
  GtkWidget* entry;
};

enum {
  CHAT_AREA_PROP_CHAT = 1
};

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
}

static void
chat_area_add_message (ChatArea* self,
		       const gchar* from,
		       const gchar* txt)
{
  gchar* str = NULL;
  GtkTextBuffer* buffer = NULL;
  GtkTextIter iter;

  str = g_strdup_printf ("%s says: %s\n", from, txt);
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (self->priv->text_view));
  gtk_text_buffer_get_end_iter (buffer, &iter);
  gm_text_buffer_enhancer_insert_text (self->priv->enhancer, &iter,
				       str, -1);
  g_free (str);
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

  /* first the area has a text view to display */

  self->priv->text_view = gtk_text_view_new ();
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (self->priv->text_view));

  gtk_text_view_set_editable (GTK_TEXT_VIEW (self->priv->text_view), FALSE);
  gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (self->priv->text_view),
				    FALSE);
  gtk_text_view_set_justification (GTK_TEXT_VIEW (self->priv->text_view),
				   GTK_JUSTIFY_LEFT);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (self->priv->text_view),
			       GTK_WRAP_WORD);

  gtk_widget_show (self->priv->text_view);
  gtk_box_pack_start (GTK_BOX (self), self->priv->text_view,
		      TRUE, TRUE, 2);

  /* then we want to enhance this display */

  self->priv->enhancer = gm_text_buffer_enhancer_new (buffer);

  helper = gm_text_smiley_new ();
  gm_text_buffer_enhancer_add_helper (self->priv->enhancer, helper);
  g_object_unref (helper);

  tag = gtk_text_buffer_create_tag (buffer, "bold",
				    "weight", PANGO_WEIGHT_BOLD,
				    NULL);
  helper = gm_text_anchored_tag_new ("<b>", tag, TRUE);
  gm_text_buffer_enhancer_add_helper (self->priv->enhancer, helper);
  g_object_unref (helper);
  helper = gm_text_anchored_tag_new ("</b>", tag, FALSE);
  gm_text_buffer_enhancer_add_helper (self->priv->enhancer, helper);
  g_object_unref (helper);

  tag = gtk_text_buffer_create_tag (buffer, "italic",
				    "style", PANGO_STYLE_ITALIC,
				    NULL);
  helper = gm_text_anchored_tag_new ("<i>", tag, TRUE);
  gm_text_buffer_enhancer_add_helper (self->priv->enhancer, helper);
  g_object_unref (helper);
  helper = gm_text_anchored_tag_new ("</i>", tag, FALSE);
  gm_text_buffer_enhancer_add_helper (self->priv->enhancer, helper);
  g_object_unref (helper);

  tag = gtk_text_buffer_create_tag (buffer, "underline",
				    "underline", PANGO_UNDERLINE_SINGLE,
				    NULL);
  helper = gm_text_anchored_tag_new ("<u>", tag, TRUE);
  gm_text_buffer_enhancer_add_helper (self->priv->enhancer, helper);
  g_object_unref (helper);
  helper = gm_text_anchored_tag_new ("</u>", tag, FALSE);
  gm_text_buffer_enhancer_add_helper (self->priv->enhancer, helper);
  g_object_unref (helper);

  /* and finally the chat area has a nice entry system */
  GtkWidget* vbox = NULL;
  GtkWidget* bbox = NULL;
  GtkWidget* button = NULL;
  const gchar** smileys = gm_get_smileys ();
  gint smiley;
  GdkPixbuf* pixbuf = NULL;
  GtkWidget* image = NULL;
  GtkWidget* smiley_item = NULL;

  /* we need to build a nice */
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
  gtk_box_pack_start (GTK_BOX (vbox), bbox,
		      FALSE, TRUE, 2);
  gtk_widget_show (bbox);

  button = gtk_button_new_from_stock (GTK_STOCK_INFO); // FIXME
  g_signal_connect (G_OBJECT (button), "clicked",
		    G_CALLBACK (on_smiley_clicked), self);
  gtk_box_pack_start (GTK_BOX (bbox), button,
		      FALSE, TRUE, 2);
  gtk_widget_show (button);

  button = gtk_button_new_from_stock (GTK_STOCK_BOLD);
  g_signal_connect (G_OBJECT (button), "clicked",
		    G_CALLBACK (on_bold_clicked), self);
  gtk_box_pack_start (GTK_BOX (bbox), button,
		      FALSE, TRUE, 2);
  gtk_widget_show (button);

  button = gtk_button_new_from_stock (GTK_STOCK_ITALIC);
  g_signal_connect (G_OBJECT (button), "clicked",
		    G_CALLBACK (on_italic_clicked), self);
  gtk_box_pack_start (GTK_BOX (bbox), button,
		      FALSE, TRUE, 2);
  gtk_widget_show (button);

  button = gtk_button_new_from_stock (GTK_STOCK_UNDERLINE);
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
chat_area_get_title (ChatArea* chat)
{
  return chat->priv->chat->get_title ();
}
