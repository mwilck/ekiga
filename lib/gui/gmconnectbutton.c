
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
 *                         connectbutton.c  -  description
 *                         -------------------------------
 *   begin                : Tue Nov 01 2005
 *   copyright            : (C) 2000-2006 by Damien Sandras 
 *   description          : Contains a connectbutton widget 
 *
 */


#include "gmconnectbutton.h"


/* Static functions and declarations */
static void gm_connect_button_class_init (GmConnectButtonClass *);

static void gm_connect_button_init (GmConnectButton *);

static void gm_connect_button_destroy (GtkObject *);

static GtkToggleButtonClass *parent_class = NULL;


static void
gm_connect_button_class_init (GmConnectButtonClass *klass)
{
  GObjectClass *object_class = NULL;
  GtkObjectClass *gtkobject_class = NULL;
  GmConnectButtonClass *connect_button_class = NULL;

  gtkobject_class = GTK_OBJECT_CLASS (klass);
  object_class = G_OBJECT_CLASS (klass);
  parent_class = g_type_class_peek_parent (klass);
  connect_button_class = GM_CONNECT_BUTTON_CLASS (klass);

  gtkobject_class->destroy = gm_connect_button_destroy;
}


static void
gm_connect_button_init (GmConnectButton *cb)
{
  g_return_if_fail (cb != NULL);
  g_return_if_fail (GM_IS_CONNECT_BUTTON (cb));

  cb->image = NULL;
  cb->connected_stock_id = NULL;
  cb->disconnected_stock_id = NULL;
  cb->connected_label = NULL;
  cb->disconnected_label = NULL;
}


static void
gm_connect_button_destroy (GtkObject *object)
{
  GmConnectButton *cb = NULL;

  g_return_if_fail (object != NULL);
  g_return_if_fail (GM_IS_CONNECT_BUTTON (object));

  cb = GM_CONNECT_BUTTON (object);
  
  if (cb->connected_stock_id) {
    g_free (cb->connected_stock_id);
    cb->connected_stock_id = NULL;
  }

  if (cb->disconnected_stock_id) {
    g_free (cb->disconnected_stock_id);
    cb->disconnected_stock_id = NULL;
  }
  
  if (cb->connected_label) {
    g_free (cb->connected_label);
    cb->connected_label = NULL;
  }
  
  if (cb->disconnected_label) {
    g_free (cb->disconnected_label);
    cb->disconnected_label = NULL;
  }

  if (GTK_OBJECT_CLASS (parent_class)->destroy)
    (*GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}


/* Global functions */
GType
gm_connect_button_get_type (void)
{
  static GType gm_connect_button_type = 0;
  
  if (gm_connect_button_type == 0)
  {
    static const GTypeInfo connect_button_info =
    {
      sizeof (GmConnectButtonClass),
      NULL,
      NULL,
      (GClassInitFunc) gm_connect_button_class_init,
      NULL,
      NULL,
      sizeof (GmConnectButton),
      0,
      (GInstanceInitFunc) gm_connect_button_init
    };
    
    gm_connect_button_type =
      g_type_register_static (GTK_TYPE_TOGGLE_BUTTON,
			      "GmConnectButton",
			      &connect_button_info,
			      (GTypeFlags) 0);
  }
  
  return gm_connect_button_type;
}


GtkWidget *
gm_connect_button_new (const char *connected,
		       const char *disconnected,
		       GtkIconSize size,
		       const char *con_label,
		       const char *dis_label)
{
  GmConnectButton *cb = NULL;
  
  GtkWidget *hbox = NULL;
  
  g_return_val_if_fail (connected != NULL, NULL);
  g_return_val_if_fail (disconnected != NULL, NULL);
  
  cb = GM_CONNECT_BUTTON (g_object_new (GM_CONNECT_BUTTON_TYPE, NULL));

  
  cb->image = gtk_image_new ();
  cb->label = gtk_label_new (NULL);
  cb->stock_size = size;
  cb->connected_stock_id = g_strdup (connected);
  cb->disconnected_stock_id = g_strdup (disconnected);
  cb->connected_label = g_strdup (con_label);
  cb->disconnected_label = g_strdup (dis_label);

  if (con_label && dis_label) {

    hbox = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), cb->image, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), cb->label, FALSE, FALSE, 6);
    gtk_container_add (GTK_CONTAINER (cb), hbox);
  }
  else {
    
    gtk_widget_set_size_request (GTK_WIDGET (cb), 35, 35);
    gtk_container_add (GTK_CONTAINER (cb), cb->image);
  }

  gm_connect_button_set_connected (cb, FALSE);

  return GTK_WIDGET (cb);
}


void 
gm_connect_button_set_connected (GmConnectButton *cb,
				 gboolean state)
{
  g_return_if_fail (cb != NULL);
  g_return_if_fail (GM_IS_CONNECT_BUTTON (cb));

  gtk_image_set_from_stock (GTK_IMAGE (cb->image), 
			    state?
			    cb->connected_stock_id:cb->disconnected_stock_id,
			    cb->stock_size);
    
  if (state && cb->connected_label)
    gtk_label_set_markup_with_mnemonic (GTK_LABEL (cb->label), 
					cb->connected_label);
  else if (!state && cb->disconnected_label)
    gtk_label_set_markup_with_mnemonic (GTK_LABEL (cb->label), 
					cb->disconnected_label);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (cb), state);
}


gboolean 
gm_connect_button_get_connected (GmConnectButton *cb)
{
  gboolean connected = FALSE;
  gchar *stock_id = NULL;
  GtkIconSize size;
  
  g_return_val_if_fail (cb != NULL, FALSE);
  g_return_val_if_fail (GM_IS_CONNECT_BUTTON (cb), FALSE);

  gtk_image_get_stock (GTK_IMAGE (cb->image), &stock_id, &size);

  connected = !strcmp (stock_id, cb->connected_stock_id);

  return connected;
}


