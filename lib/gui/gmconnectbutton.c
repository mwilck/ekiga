
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
 *                         connectbutton.c  -  description
 *                         -------------------------------
 *   begin                : Tue Nov 01 2005
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : Contains a connectbutton widget
 *
 */


#include "gmconnectbutton.h"

struct _GmConnectButtonPrivate
{
  gchar* pickup;
  gchar* hangup;
  GtkIconSize stock_size;
  gboolean connected;
};

enum
{
  PROP_0,
  PROP_HANGUP,
  PROP_PICKUP,
  PROP_STOCKSIZE
};

G_DEFINE_TYPE (GmConnectButton, gm_connect_button, GTK_TYPE_BUTTON);

static void
gm_connect_button_get_property (GObject* object,
				guint prop_id,
				GValue* value,
				GParamSpec* pspec)
{
  GmConnectButton* self = GM_CONNECT_BUTTON (object);

  switch (prop_id) {

  case PROP_PICKUP:
    g_value_set_string (value, self->priv->pickup);
    break;

  case PROP_HANGUP:
    g_value_set_string (value, self->priv->hangup);
    break;

  case PROP_STOCKSIZE:
    g_value_set_enum (value, self->priv->stock_size);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

static void
gm_connect_button_set_property (GObject* object,
				guint prop_id,
				const GValue* value,
				GParamSpec* pspec)
{
  GmConnectButton* self = GM_CONNECT_BUTTON (object);

  switch (prop_id) {

  case PROP_PICKUP:
    g_free (self->priv->pickup);
    self->priv->pickup = g_value_dup_string (value);
    if ( !self->priv->connected) {

      GtkImage* image = GTK_IMAGE (gtk_button_get_image (GTK_BUTTON (self)));
      gtk_image_set_from_stock (image, self->priv->pickup, self->priv->stock_size);
    }
    break;

  case PROP_HANGUP:
    g_free (self->priv->hangup);
    self->priv->hangup = g_value_dup_string (value);
    if (self->priv->connected) {

      GtkImage* image = GTK_IMAGE (gtk_button_get_image (GTK_BUTTON (self)));
      gtk_image_set_from_stock (image, self->priv->hangup, self->priv->stock_size);
  }
    break;

  case PROP_STOCKSIZE:
    self->priv->stock_size = g_value_get_enum (value);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

static void
gm_connect_button_init (GmConnectButton *cb)
{
  cb->priv = G_TYPE_INSTANCE_GET_PRIVATE (cb, GM_TYPE_CONNECT_BUTTON, GmConnectButtonPrivate);

  cb->priv->pickup = NULL;
  cb->priv->hangup = NULL;
  cb->priv->connected = FALSE;

  gtk_button_set_relief (GTK_BUTTON (cb), GTK_RELIEF_NONE);
}


static void
gm_connect_button_finalize (GObject* object)
{
  GmConnectButton *cb = GM_CONNECT_BUTTON (object);

  g_free (cb->priv->pickup);
  g_free (cb->priv->hangup);

  G_OBJECT_CLASS (gm_connect_button_parent_class)->finalize (object);
}

static void
gm_connect_button_class_init (GmConnectButtonClass *klass)
{
  GObjectClass* gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = gm_connect_button_finalize;
  gobject_class->get_property = gm_connect_button_get_property;
  gobject_class->set_property = gm_connect_button_set_property;

  g_object_class_install_property (gobject_class, PROP_HANGUP,
				   g_param_spec_string ("hangup", "Hangup", "Hangup",
							NULL,
							G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, PROP_PICKUP,
				   g_param_spec_string ("pickup", "Pickup", "Pickup",
							NULL,
							G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, PROP_STOCKSIZE,
				   g_param_spec_enum ("stock-size", "Stock size", "Stock size",
						      GTK_TYPE_ICON_SIZE, GTK_ICON_SIZE_BUTTON,
						      G_PARAM_READWRITE));

  g_type_class_add_private (klass, sizeof (GmConnectButtonPrivate));
}


/* public api */

GtkWidget*
gm_connect_button_new (const char *pickup,
		       const char *hangup,
		       GtkIconSize size)
{
  gpointer result;
  GtkWidget* image = NULL;

  g_return_val_if_fail (pickup != NULL && hangup != NULL, NULL);

  image = gtk_image_new_from_stock (GTK_STOCK_OK, GTK_ICON_SIZE_BUTTON);

  result = g_object_new (GM_TYPE_CONNECT_BUTTON,
			 "image", image,
			 "stock-size", size,
			 "hangup", hangup,
			 "pickup", pickup,
			 NULL);

  return GTK_WIDGET (result);
}

void
gm_connect_button_set_connected (GmConnectButton *cb,
				 gboolean state)
{
  GtkWidget* image = NULL;

  g_return_if_fail (GM_IS_CONNECT_BUTTON (cb));

  cb->priv->connected = state;
  image = gtk_button_get_image (GTK_BUTTON (cb));
  gtk_image_set_from_stock (GTK_IMAGE (image), state ? cb->priv->hangup : cb->priv->pickup, cb->priv->stock_size);
}

gboolean
gm_connect_button_get_connected (GmConnectButton *cb)
{
  g_return_val_if_fail (GM_IS_CONNECT_BUTTON (cb), FALSE);

  return cb->priv->connected;
}
