/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2012, Xunta de Galicia <ocfloss@xunta.es>
 *
 * Author: Victor Jaquez, Igalia S.L., AGASOL. <vjaquez@igalia.com>
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

#include "ext-window.h"
#include "gmconf.h"

#ifndef WIN32
#include <gdk/gdkx.h>
#else
#include "platform/winpaths.h"
#include <gdk/gdkwin32.h>
#endif

G_DEFINE_TYPE (EkigaExtWindow, ekiga_ext_window, GTK_TYPE_WINDOW);

struct _EkigaExtWindowPrivate {
#ifndef WIN32
  GC gc;
#endif
  GtkWidget *video, *zin, *zout;
  boost::shared_ptr<Ekiga::VideoOutputCore> vocore;
};

static void
set_zoom_buttons_sensitive (EkigaExtWindow *ew, guint zoom)
{
  gtk_widget_set_sensitive (ew->priv->zin, zoom != 200);
  gtk_widget_set_sensitive (ew->priv->zout, zoom != 50);
}

static void
zoom_in (G_GNUC_UNUSED GtkWidget *widget, gpointer user_data)
{
  guint zoom;

  zoom = gm_conf_get_int (VIDEO_DISPLAY_KEY "ext_zoom");
  if (zoom < 200)
    zoom = zoom * 2;
  gm_conf_set_int (VIDEO_DISPLAY_KEY "ext_zoom", zoom);

  set_zoom_buttons_sensitive (EKIGA_EXT_WINDOW (user_data), zoom);
}

static void
zoom_out (G_GNUC_UNUSED GtkWidget *widget, gpointer user_data)
{
  guint zoom;

  zoom = gm_conf_get_int (VIDEO_DISPLAY_KEY "ext_zoom");
  if (zoom > 50)
    zoom = (guint) zoom / 2;
  gm_conf_set_int (VIDEO_DISPLAY_KEY "ext_zoom", zoom);

  set_zoom_buttons_sensitive (EKIGA_EXT_WINDOW (user_data), zoom);
}

static void
gui_layout (EkigaExtWindow *ew)
{
  GtkWidget *zin, *zout, *vbox, *hbox;

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add (GTK_CONTAINER (ew), vbox);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

  zin = gtk_button_new_from_stock (GTK_STOCK_ZOOM_IN);
  gtk_box_pack_start (GTK_BOX (hbox), zin, FALSE, FALSE, 0);

  zout = gtk_button_new_from_stock (GTK_STOCK_ZOOM_OUT);
  gtk_box_pack_start (GTK_BOX (hbox), zout, FALSE, FALSE, 0);

  ew->priv->video = gtk_image_new ();
  gtk_box_pack_start (GTK_BOX (vbox), ew->priv->video, FALSE, FALSE, 0);

  ew->priv->zin = zin;
  ew->priv->zout = zout;

  g_signal_connect (zin, "clicked", G_CALLBACK (zoom_in), ew);
  g_signal_connect (zout, "clicked", G_CALLBACK (zoom_out), ew);

  gtk_widget_show_all (vbox);

  gtk_window_set_resizable (GTK_WINDOW (ew), FALSE);
}

static inline void
clear_display_info (EkigaExtWindow *ew)
{
  Ekiga::DisplayInfo info;

  info.x = 0;
  info.y = 0;
  info.widget_info_set = false;
  info.zoom = 0;
  info.mode = Ekiga::VO_MODE_UNSET;
  info.config_info_set = false;
#ifdef WIN32
  info.hwnd = 0;
#else
  info.gc = 0;
  info.window = 0;
#endif

  ew->priv->vocore->set_ext_display_info (info);
}

static GObject *
constructor (GType type, guint n_properties, GObjectConstructParam *params)
{
  GObject *object;

  object = G_OBJECT_CLASS (ekiga_ext_window_parent_class)->constructor (type,
                                                                        n_properties,
                                                                        params);

  gui_layout (EKIGA_EXT_WINDOW (object));

  return object;
}

static void
finalize (GObject* gobject)
{
  EkigaExtWindow *ew = EKIGA_EXT_WINDOW (gobject);

  delete ew->priv;
  ew->priv = NULL;

  G_OBJECT_CLASS (ekiga_ext_window_parent_class)->finalize (gobject);
}

static gboolean
focus_in_event (GtkWidget *widget, GdkEventFocus *event)
{
  if (gtk_window_get_urgency_hint (GTK_WINDOW (widget)))
    gtk_window_set_urgency_hint (GTK_WINDOW (widget), false);

  return GTK_WIDGET_CLASS (ekiga_ext_window_parent_class)->focus_in_event (widget,
                                                                           event);
}

static void
show (GtkWidget *widget)
{
  GdkWindow *w = gtk_widget_get_window (widget);

  if (w && gm_conf_get_bool (VIDEO_DISPLAY_KEY "stay_on_top"))
    gdk_window_set_keep_above (w, true);

  GTK_WIDGET_CLASS (ekiga_ext_window_parent_class)->show (widget);

  gtk_widget_queue_draw (widget);
}

static gboolean
draw_event (GtkWidget *widget,
            cairo_t *context)
{
  EkigaExtWindow *ew = EKIGA_EXT_WINDOW (widget);
  Ekiga::DisplayInfo info;
  gboolean handled;
  GtkAllocation alloc;

  handled = (*GTK_WIDGET_CLASS (ekiga_ext_window_parent_class)->draw) (widget, context);

  gtk_widget_get_allocation (ew->priv->video, &alloc);
  info.x = alloc.x;
  info.y = alloc.y;

#ifdef WIN32
  info.hwnd = (HWND) GDK_WINDOW_HWND (gtk_widget_get_window (ew->priv->video));
#else
  info.window = gdk_x11_window_get_xid (gtk_widget_get_window (ew->priv->video));
  g_return_val_if_fail (info.window != 0, handled);

  if (!ew->priv->gc) {
    Display *display;
    display = GDK_WINDOW_XDISPLAY (gtk_widget_get_window (ew->priv->video));
    ew->priv->gc = XCreateGC(display, info.window, 0, 0);
    g_return_val_if_fail (ew->priv->gc != NULL, handled);
  }

  info.gc = ew->priv->gc;

  gdk_flush ();
#endif

  info.widget_info_set = TRUE;
  info.mode = Ekiga::VO_MODE_REMOTE_EXT;
  info.config_info_set = TRUE;

  ew->priv->vocore->set_ext_display_info (info);

  return handled;
}

static void
ekiga_ext_window_class_init (EkigaExtWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructor = constructor;
  object_class->finalize = finalize;

  widget_class->show = show;
  widget_class->draw = draw_event;
  widget_class->focus_in_event = focus_in_event;
}

static void
ekiga_ext_window_init (EkigaExtWindow *ew)
{
  ew->priv = new EkigaExtWindowPrivate;
#ifndef WIN32
  ew->priv->gc = NULL;
#endif
}

GtkWidget *
ext_window_new (boost::shared_ptr<Ekiga::VideoOutputCore> &vocore)
{
  EkigaExtWindow *ew =
    EKIGA_EXT_WINDOW (g_object_new (EKIGA_TYPE_EXT_WINDOW, NULL));

  g_signal_connect (ew, "delete-event", G_CALLBACK (gtk_true), NULL);

  ew->priv->vocore = vocore;

  return GTK_WIDGET (ew);
}

void
ekiga_ext_window_set_size (EkigaExtWindow *ew, int width, int height)
{
  int pw, ph;

  g_return_if_fail (width > 0 && height > 0);

  gtk_widget_get_size_request (ew->priv->video, &pw, &ph);

  /* No size requisition yet
   * It's our first call so we silently set the new requisition and exit...
   */
  if (pw == -1) {
    gtk_widget_set_size_request (ew->priv->video, width, height);
    return;
  }

  /* Do some kind of filtering here. We often get duplicate "size-changed" events...
   * Note that we currently only bother about the width of the video.
   */
  if (pw == width)
    return;

  gtk_widget_set_size_request (ew->priv->video, width, height);
  gdk_window_invalidate_rect (gtk_widget_get_window (GTK_WIDGET (ew)), NULL, TRUE);
}

void
ekiga_ext_window_destroy (EkigaExtWindow *ew)
{
  clear_display_info (ew);

  /* dirty cheats done dirt cheap: if gtk_widget_destroy it crashes */
  gtk_widget_hide (GTK_WIDGET (ew));
}
