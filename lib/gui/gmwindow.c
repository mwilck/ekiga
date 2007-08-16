
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
 *                         gmwindow.c -  description
 *                         -------------------------
 *   begin                : 16 August 2007 
 *   copyright            : (c) 2007 by Damien Sandras 
 *   description          : Implementation of a GtkWindow able to restore
 *                          its position and size in a GmConf key.
 *
 */

#include "gmwindow.h"

#include <gdk/gdkkeysyms.h>



/*
 * The GmWindow
 */
struct _GmWindowPrivate
{
  const gchar *key;
  int x;
  int y;
  int width;
  int height;
};


static GObjectClass *parent_class = NULL;


/* 
 * GObject stuff
 */
static void
gm_window_dispose (GObject *obj)
{
  GmWindow *window = NULL;

  window = GM_WINDOW (obj);

  window->priv->key = NULL;

  parent_class->dispose (obj);
}


static void
gm_window_finalize (GObject *obj)
{
  GmWindow *window = NULL;

  window = GM_WINDOW (obj);

  g_free ((gchar *) window->priv->key);

  parent_class->finalize (obj);
}


static void
gm_window_class_init (gpointer g_class,
                      gpointer class_data)
{
  GObjectClass *gobject_class = NULL;

  parent_class = (GObjectClass *) g_type_class_peek_parent (g_class);

  gobject_class = (GObjectClass *) g_class;
  gobject_class->dispose = gm_window_dispose;
  gobject_class->finalize = gm_window_finalize;
}


GType
gm_window_get_type ()
{
  static GType result = 0;

  if (result == 0) {

    static const GTypeInfo info = {
      sizeof (GmWindowClass),
      NULL,
      NULL,
      gm_window_class_init,
      NULL,
      NULL,
      sizeof (GmWindow),
      0,
      NULL,
      NULL
    };

    result = g_type_register_static (GTK_TYPE_WINDOW,
				     "GmWindowType",
				     &info, (GTypeFlags) 0);
  }

  return result;
}


static gboolean 
gm_window_is_visible (GtkWidget *w)
{
  return (GTK_WIDGET_VISIBLE (w) && !(gdk_window_get_state (GDK_WINDOW (w->window)) & GDK_WINDOW_STATE_ICONIFIED));
}


static void
gm_window_show (GtkWidget *w,
                gpointer data)
{
  int x = 0;
  int y = 0;

  GmWindow *self = NULL;

  gchar *window_name = NULL;
  gchar *conf_key_size = NULL;
  gchar *conf_key_position = NULL;
  gchar *size = NULL;
  gchar *position = NULL;
  gchar **couple = NULL;

  g_return_if_fail (w != NULL);
  
  self = GM_WINDOW (w);

  conf_key_position =
    g_strdup_printf ("%s/position", self->priv->key);
  conf_key_size =
    g_strdup_printf ("%s/size", self->priv->key);

  if (gtk_window_get_resizable (GTK_WINDOW (w))) {

    size = gm_conf_get_string (conf_key_size);
    if (size)
      couple = g_strsplit (size, ",", 0);

    if (couple && couple [0])
      x = atoi (couple [0]);
    if (couple && couple [1])
      y = atoi (couple [1]);

    if (x > 0 && y > 0) {
      gtk_window_resize (GTK_WINDOW (w), x, y);
    }

    g_strfreev (couple);
    g_free (size);
  }

  position = gm_conf_get_string (conf_key_position);
  if (position)
    couple = g_strsplit (position, ",", 0);

  if (couple && couple [0])
    x = atoi (couple [0]);
  if (couple && couple [1])
    y = atoi (couple [1]);

  if (x != 0 && y != 0)
    gtk_window_move (GTK_WINDOW (w), x, y);

  g_strfreev (couple);
  couple = NULL;
  g_free (position);

  gtk_widget_realize (GTK_WIDGET (w));

  g_free (conf_key_position);
  g_free (conf_key_size);
}


static void
gm_window_hide (GtkWidget *w,
                gpointer data)
{
  GmWindow *self = NULL;

  int x = 0;
  int y = 0;

  gchar *window_name = NULL;
  gchar *conf_key_size = NULL;
  gchar *conf_key_position = NULL;
  gchar *size = NULL;
  gchar *position = NULL;
  
  g_return_if_fail (w != NULL);
  
  self = GM_WINDOW (w);

  conf_key_position =
    g_strdup_printf ("%s/position", self->priv->key);
  conf_key_size =
    g_strdup_printf ("%s/size", self->priv->key);

  position = g_strdup_printf ("%d,%d", self->priv->x, self->priv->y);
  gm_conf_set_string (conf_key_position, position);
  g_free (position);

  if (gtk_window_get_resizable (GTK_WINDOW (w))) {

    size = g_strdup_printf ("%d,%d", self->priv->width, self->priv->height);
    gm_conf_set_string (conf_key_size, size);
    g_free (size);
  }
  
  g_free (conf_key_position);
  g_free (conf_key_size);
}


static gboolean 
gm_window_configure_event (GtkWidget *widget,
                           GdkEventConfigure *event)
{
  GM_WINDOW (widget)->priv->x = event->x;
  GM_WINDOW (widget)->priv->y = event->y;

  GM_WINDOW (widget)->priv->width = event->width;
  GM_WINDOW (widget)->priv->height = event->height;

  return FALSE;
}


/* 
 * Public API
 */
GtkWidget *
gm_window_new (const char *key)
{
  GmWindow *self = NULL;

  GtkAccelGroup *accel = NULL;

  self = GM_WINDOW (g_object_new (GM_WINDOW_TYPE, NULL));

  self->priv = (GmWindowPrivate *) g_malloc (sizeof (GmWindowPrivate));
  self->priv->key = g_strdup (key);

  accel = gtk_accel_group_new ();
  gtk_window_add_accel_group (GTK_WINDOW (self), accel);
  gtk_accel_group_connect (accel, GDK_Escape, (GdkModifierType) 0, GTK_ACCEL_LOCKED,
                           g_cclosure_new_swap (G_CALLBACK (gtk_widget_hide), (gpointer) self, NULL));

  g_signal_connect (G_OBJECT (self), "delete_event",
		    G_CALLBACK (gtk_widget_hide_on_delete), NULL);

  g_signal_connect (G_OBJECT (self), "show",
                    G_CALLBACK (gm_window_show), self);

  g_signal_connect (G_OBJECT (self), "hide",
                    G_CALLBACK (gm_window_hide), self);

  g_signal_connect (G_OBJECT (self), "configure-event",
                    G_CALLBACK (gm_window_configure_event), self);

  return GTK_WIDGET (self);
}
