
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2014 Damien Sandras <dsandras@seconix.com>
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
 *                         gm-info-bar.h  -  description
 *                         -----------------------------
 *   begin                : Sat Nov 1 2014
 *   copyright            : (C) 2014 by Damien Sandras
 *   description          : Contains a GmInfoBary implementation handling
 *                          GtkInfoBar info and error messages.
 *
 */


#include "gm-info-bar.h"


struct _GmInfoBarPrivate {

  GtkWidget *label;
  guint timeout;
};

G_DEFINE_TYPE (GmInfoBar, gm_info_bar, GTK_TYPE_INFO_BAR);


/* Callbacks */
static gboolean on_info_bar_delayed_hide (gpointer self);

static void on_info_bar_response (GmInfoBar *self,
                                  gint response_id,
                                  gpointer data);


/* Static GObject functions and declarations */
static void gm_info_bar_dispose (GObject* obj);

static void gm_info_bar_class_init (GmInfoBarClass *);

static void gm_info_bar_init (GmInfoBar *);


/* Callbacks */
static gboolean
on_info_bar_delayed_hide (gpointer self)
{
  gtk_widget_hide (GTK_WIDGET (self));

  return FALSE;
}

static void
on_info_bar_response (GmInfoBar *self,
                      gint response_id,
                      G_GNUC_UNUSED gpointer data)
{
  g_return_if_fail (GM_IS_INFO_BAR (self));

  if (response_id == GTK_RESPONSE_CLOSE)
    gtk_widget_hide (GTK_WIDGET (self));
}


/* Implementation of GObject stuff */
static void
gm_info_bar_dispose (GObject* obj)
{
  GmInfoBarPrivate *priv = GM_INFO_BAR (obj)->priv;

  if (priv->timeout > 0) {
    g_source_remove (priv->timeout);
    priv->timeout = 0;
  }

  G_OBJECT_CLASS (gm_info_bar_parent_class)->dispose (obj);
}


static void
gm_info_bar_class_init (GmInfoBarClass *klass)
{
  g_type_class_add_private (klass, sizeof (GmInfoBarPrivate));

  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->dispose = gm_info_bar_dispose;
}


static void
gm_info_bar_init (GmInfoBar* self)
{
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                            GM_TYPE_INFO_BAR,
					    GmInfoBarPrivate);

  self->priv->timeout = 0;
  self->priv->label = gtk_label_new (NULL);
  gtk_label_set_line_wrap (GTK_LABEL (self->priv->label), TRUE);

  gtk_box_pack_start (GTK_BOX (gtk_info_bar_get_content_area (GTK_INFO_BAR (self))),
                      self->priv->label, FALSE, FALSE, 0);

  g_signal_connect (G_OBJECT (self), "response",
                    G_CALLBACK (on_info_bar_response), NULL);
}


/* public api */
GtkWidget *
gm_info_bar_new ()
{
  return GTK_WIDGET (g_object_new (GM_TYPE_INFO_BAR, NULL));
}


void
gm_info_bar_set_message (GmInfoBar *self,
                         GtkMessageType type,
                         const char *message)
{
  g_return_if_fail (GM_IS_INFO_BAR (self) || !message || !g_strcmp0 (message, ""));

  gtk_info_bar_set_message_type (GTK_INFO_BAR (self), type);
  gtk_label_set_text (GTK_LABEL (self->priv->label), message);
  gtk_info_bar_set_show_close_button (GTK_INFO_BAR (self), (type != GTK_MESSAGE_INFO));
  gtk_widget_show_all (GTK_WIDGET (self));

  /* Flash the information message. Please don't abuse this.
   */
  if (type == GTK_MESSAGE_INFO) {
    self->priv->timeout = g_timeout_add_seconds (4, on_info_bar_delayed_hide, self);
  }
}
