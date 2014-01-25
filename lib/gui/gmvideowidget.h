
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
 *                         gmvideowidget.h  -  description
 *                         -------------------------------
 *   begin                : Wed Jan 1 2014
 *   copyright            : (C) 2000-2014 by Damien Sandras
 *   description          : This file contains all the functions needed to
 *                          build the a Gtk Clutter object supporting the
 *                          display of one main video stream and one
 *                          secondary video stream. The secondary stream
 *                          is displayed in a PIP video_widget when both streams
 *                          are active.
 */


#ifndef __GM_VIDEO_WIDGET_H__
#define __GM_VIDEO_WIDGET_H__

#include <clutter-gtk/clutter-gtk.h>

G_BEGIN_DECLS

typedef struct _GmVideoWidget GmVideoWidget;
typedef struct _GmVideoWidgetPrivate GmVideoWidgetPrivate;
typedef struct _GmVideoWidgetClass GmVideoWidgetClass;


/* GObject thingies */
struct _GmVideoWidget
{
  GtkClutterEmbed parent;
  GmVideoWidgetPrivate *priv;
};

struct _GmVideoWidgetClass
{
  GtkClutterEmbedClass parent;
};

typedef enum {
  PRIMARY_STREAM,
  SECONDARY_STREAM,
  MAX_STREAM
} GM_STREAM_TYPE;

typedef enum {
  STREAM_STATE_PLAYING,
  STREAM_STATE_STOPPED
} GM_STREAM_STATE;


/* Public API */

/** Create a new GmVideoWidget.
 * @return A GmVideoWidget
 */
GtkWidget *gm_video_widget_new ();


/** Update the GmVideoWidget stream state.
 * @param self is the GmVideoWidget
 * @param type is the requested GmVideoWidget type
 * @param state is the video stream state
 */
void gm_video_widget_set_stream_state (GmVideoWidget *self,
                                       const GM_STREAM_TYPE type,
                                       const GM_STREAM_STATE state);


/** Return the video stream.
 * @param self is the GmVideoWidget
 * @param type is the requested GmVideoWidget type
 * @return The video stream
 */
ClutterActor *gm_video_widget_get_stream (GmVideoWidget *self,
                                          const GM_STREAM_TYPE type);


/** Set the video stream natural size.
 * @param self is the GmVideoWidget
 * @param type is the GmVideoWidget type
 * @param width is the natural video stream width
 * @param height is the natural video stream height
 */
void gm_video_widget_set_stream_natural_size (GmVideoWidget *self,
                                              const GM_STREAM_TYPE type,
                                              const unsigned width,
                                              const unsigned height);


/** Get the video stream natural size.
 * @param self is the GmVideoWidget
 * @param type is the GmVideoWidget type
 * @param width is the natural video stream width
 * @param height is the natural video stream height
 */
void gm_video_widget_get_stream_natural_size (GmVideoWidget *self,
                                              const GM_STREAM_TYPE type,
                                              unsigned *width,
                                              unsigned *height);


/** Set if the GmVideoWidget object should display animations or not.
 * @param video_widget is the GmVideoWidget
 * @param animation_duration is the animation duration
 */
void gm_video_widget_set_animation_duration (GmVideoWidget *video_widget,
                                             const int animation_duration);


/** Return the GmVideoWidget object animation duration.m
 * @param video_widget is the GmVideoWidget
 */
int gm_video_widget_get_animation_duration (GmVideoWidget *video_widget);


/** Set if the GmVideoWidget object should display a logo.
 *  The logo is displayed on the top right when streams are active,
 *  and as background when the stream is inactive.
 * @param video_widget is the GmVideoWidget
 * @param logo is the logo filename with path
 */
void gm_video_widget_set_logo (GmVideoWidget *video_widget,
                               const gchar *logo);


/** Set the GmVideoWidget secondary stream margin when both streams are active.
 * @param video_widget is the GmVideoWidget
 * @param margin is the margin
 */
void gm_video_widget_set_secondary_stream_margin (GmVideoWidget *video_widget,
                                                  const unsigned margin);


/** Get the GmVideoWidget secondary stream margin when both streams are active.
 * @param video_widget is the GmVideoWidget
 * @return the margin
 */
unsigned gm_video_widget_get_secondary_stream_margin (GmVideoWidget *video_widget);


/** Set the GmVideoWidget secondary stream scale when both streams are active.
 * @param video_widget is the GmVideoWidget
 * @param scale is the scale
 */
void gm_video_widget_set_secondary_stream_scale (GmVideoWidget *video_widget,
                                                 const float scale);


/** Get the GmVideoWidget secondary stream scale when both streams are active.
 * @param video_widget is the GmVideoWidget
 * @return the scale
 */
gfloat gm_video_widget_get_secondary_stream_scale (GmVideoWidget *video_widget);


/** Set if the GmVideoWidget secondary stream should be displayed
 *  when both streams are active.
 * @param video_widget is the GmVideoWidget
 * @param display is true if it should be displayed
 */
void gm_video_widget_set_secondary_stream_display (GmVideoWidget *video_widget,
                                                   const gboolean display);


/** Get if the GmVideoWidget secondary stream should be displayed
 *  when both streams are active.
 * @param video_widget is the GmVideoWidget
 * @return true if the stream should be displayed, false otherwise
 */
gboolean gm_video_widget_get_secondary_stream_display (GmVideoWidget *video_widget);


/** Set the GmVideoWidget logo when both streams are active.
 * @param video_widget is the GmVideoWidget
 * @param margin is the margin
 */
void gm_video_widget_set_logo_margin (GmVideoWidget *video_widget,
                                      const unsigned margin);


/** Get the GmVideoWidget logo margin when both streams are active.
 * @param video_widget is the GmVideoWidget
 * @return the margin
 */
unsigned gm_video_widget_get_logo_margin (GmVideoWidget *video_widget);


/** Set the GmVideoWidget logo scale when both streams are active.
 * @param video_widget is the GmVideoWidget
 * @param scale is the scale
 */
void gm_video_widget_set_logo_scale (GmVideoWidget *video_widget,
                                     const float scale);


/** Get the GmVideoWidget logo scale when both streams are active.
 * @param video_widget is the GmVideoWidget
 * @return the scale
 */
gfloat gm_video_widget_get_logo_scale (GmVideoWidget *video_widget);

/** Set the GmVideoWidget fullscreen or not.
 * @param video_widget is the GmVideoWidget
 */
void gm_video_widget_set_fullscreen (GmVideoWidget *self,
                                     const gboolean fs);

/* GObject boilerplate */

#define GM_TYPE_VIDEO_WIDGET (gm_video_widget_get_type ())

#define GM_VIDEO_WIDGET(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GM_TYPE_VIDEO_WIDGET, GmVideoWidget))

#define GM_IS_VIDEO_WIDGET(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GM_TYPE_VIDEO_WIDGET))

#define GM_VIDEO_WIDGET_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GM_TYPE_VIDEO_WIDGET, GmVideoWidgetClass))

#define GM_IS_VIDEO_WIDGET_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GM_TYPE_VIDEO_WIDGET))

#define GM_VIDEO_WIDGET_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GM_TYPE_VIDEO_WIDGET, GmVideoWidgetClass))

GType gm_video_widget_get_type ();

G_END_DECLS

#endif
