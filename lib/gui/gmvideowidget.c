
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
 *                         gmvideowidget.c  -  description
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


#include "gmvideowidget.h"

#define DEFAULT_WIDTH 640
#define DEFAULT_HEIGHT 480
#define DEFAULT_SECONDARY_STREAM_SCALE 0.25
#define DEFAULT_SECONDARY_STREAM_MARGIN 12
#define DEFAULT_LOGO_SCALE 0.15
#define DEFAULT_LOGO_MARGIN 12
#define DEFAULT_ANIMATION_DURATION 2000
#define MAX_ANIMATION_DURATION 10000

/*
 * The GmVideoWidget
 */
struct _GmVideoWidgetPrivate
{
  /* Properties */
  int animation_duration;
  gchar *logo;
  gfloat secondary_stream_scale;
  unsigned secondary_stream_margin;
  gboolean secondary_stream_display;
  gfloat logo_scale;
  unsigned logo_margin;

  /* Misc */
  ClutterActor *stream[2];
  unsigned natural_width[2];
  unsigned natural_height[2];
  unsigned available_height;
  unsigned available_width;
  GM_STREAM_STATE state[2];

  ClutterActor *emblem;
  unsigned logo_width;
  unsigned logo_height;
};

G_DEFINE_TYPE (GmVideoWidget, gm_video_widget, GTK_CLUTTER_TYPE_EMBED);


enum {
  GM_VIDEO_WIDGET_ANIMATION_DURATION = 1,
  GM_VIDEO_WIDGET_LOGO = 2,
  GM_VIDEO_WIDGET_SECONDARY_STREAM_MARGIN = 3,
  GM_VIDEO_WIDGET_SECONDARY_STREAM_SCALE = 4,
  GM_VIDEO_WIDGET_LOGO_MARGIN = 5,
  GM_VIDEO_WIDGET_LOGO_SCALE = 6,
  GM_VIDEO_WIDGET_SECONDARY_STREAM_DISPLAY = 7
};

static void gm_video_widget_actor_scale (ClutterActor *actor,
                                         const unsigned available_height,
                                         const unsigned width,
                                         const unsigned height);

static void gm_video_widget_stream_resize (GmVideoWidget *self,
                                           const GM_STREAM_TYPE type);

static void gm_video_widget_stream_align (GmVideoWidget *self,
                                          const GM_STREAM_TYPE type,
                                          const gboolean pip);

static void gm_video_widget_update_emblem (GmVideoWidget *self);

static void gm_video_widget_resized_cb (ClutterActor *stage,
                                        G_GNUC_UNUSED ClutterActorBox *box,
                                        G_GNUC_UNUSED ClutterAllocationFlags flags,
                                        gpointer self);

static void gm_video_widget_shown_cb (GtkWidget *self,
                                      gpointer data);


/*
 * GObject stuff
 */
static void
gm_video_widget_finalize (GObject *obj)
{
  GmVideoWidget *self = NULL;

  self = GM_VIDEO_WIDGET (obj);

  if (self->priv->logo)
    g_free (self->priv->logo);
  self->priv->logo = NULL;

  G_OBJECT_CLASS (gm_video_widget_parent_class)->finalize (obj);
}


static void
gm_video_widget_get_property (GObject *obj,
                              guint prop_id,
                              GValue *value,
                              GParamSpec *spec)
{
  GmVideoWidget *self = NULL;

  self = GM_VIDEO_WIDGET (obj);
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GM_TYPE_VIDEO_WIDGET, GmVideoWidgetPrivate);

  switch (prop_id) {

  case GM_VIDEO_WIDGET_ANIMATION_DURATION:
    g_value_set_int (value, self->priv->animation_duration);
    break;

  case GM_VIDEO_WIDGET_LOGO:
    g_value_set_string (value, self->priv->logo);
    break;

  case GM_VIDEO_WIDGET_SECONDARY_STREAM_MARGIN:
    g_value_set_int (value, self->priv->secondary_stream_margin);
    break;

  case GM_VIDEO_WIDGET_SECONDARY_STREAM_SCALE:
    g_value_set_float (value, self->priv->secondary_stream_scale);
    break;

  case GM_VIDEO_WIDGET_LOGO_MARGIN:
    g_value_set_int (value, self->priv->logo_margin);
    break;

  case GM_VIDEO_WIDGET_LOGO_SCALE:
    g_value_set_float (value, self->priv->logo_scale);
    break;

  case GM_VIDEO_WIDGET_SECONDARY_STREAM_DISPLAY:
    g_value_set_boolean (value, self->priv->secondary_stream_display);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, spec);
    break;
  }
}


static void
gm_video_widget_set_property (GObject *obj,
                              guint prop_id,
                              const GValue *value,
                              GParamSpec *spec)
{
  GmVideoWidget *self = NULL;
  GdkPixbuf *pixbuf = NULL;
  ClutterContent *image = NULL;
  const gchar *str = NULL;

  self = GM_VIDEO_WIDGET (obj);
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GM_TYPE_VIDEO_WIDGET, GmVideoWidgetPrivate);

  switch (prop_id) {

  case GM_VIDEO_WIDGET_ANIMATION_DURATION:
    self->priv->animation_duration = g_value_get_int (value);
    break;

  case GM_VIDEO_WIDGET_LOGO:
    if (self->priv->logo)
      g_free (self->priv->logo);
    str = g_value_get_string (value);
    self->priv->logo = g_strdup (str ? str : "");

    pixbuf = gdk_pixbuf_new_from_file (self->priv->logo, NULL);
    self->priv->logo_width = 0;
    self->priv->logo_height = 0;
    if (pixbuf) {
      image = clutter_image_new ();
      self->priv->logo_width = gdk_pixbuf_get_width (pixbuf);
      self->priv->logo_height = gdk_pixbuf_get_height (pixbuf);
      clutter_image_set_data (CLUTTER_IMAGE (image),
                              gdk_pixbuf_get_pixels (pixbuf),
                              gdk_pixbuf_get_has_alpha (pixbuf)?
                              COGL_PIXEL_FORMAT_RGBA_8888:COGL_PIXEL_FORMAT_RGB_888,
                              gdk_pixbuf_get_width (pixbuf),
                              gdk_pixbuf_get_height (pixbuf),
                              gdk_pixbuf_get_rowstride (pixbuf),
                              NULL);
      clutter_actor_set_content (self->priv->emblem, image);
      g_object_unref (image);
      clutter_actor_set_opacity (self->priv->emblem, 0);
      clutter_actor_set_size (self->priv->emblem,
                              gdk_pixbuf_get_width (pixbuf),
                              gdk_pixbuf_get_height (pixbuf));
      g_object_unref (pixbuf);
    }
    break;

  case GM_VIDEO_WIDGET_SECONDARY_STREAM_MARGIN:
    self->priv->secondary_stream_margin = g_value_get_int (value);
    break;

  case GM_VIDEO_WIDGET_SECONDARY_STREAM_SCALE:
    self->priv->secondary_stream_scale = g_value_get_float (value);
    break;

  case GM_VIDEO_WIDGET_LOGO_MARGIN:
    self->priv->logo_margin = g_value_get_int (value);
    break;

  case GM_VIDEO_WIDGET_LOGO_SCALE:
    self->priv->logo_scale = g_value_get_float (value);
    break;

  case GM_VIDEO_WIDGET_SECONDARY_STREAM_DISPLAY:
    self->priv->secondary_stream_display = g_value_get_boolean (value);
    if (self->priv->state[PRIMARY_STREAM] == STREAM_STATE_PLAYING
        && self->priv->state[SECONDARY_STREAM] == STREAM_STATE_PLAYING) {
      clutter_actor_set_opacity (self->priv->stream[SECONDARY_STREAM],
                                 self->priv->secondary_stream_display ? 255:0);
    }
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, spec);
    break;
  }
}


static void
gm_video_widget_class_init (GmVideoWidgetClass* klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GParamSpec *spec = NULL;

  g_type_class_add_private (klass, sizeof (GmVideoWidgetPrivate));

  gobject_class->finalize = gm_video_widget_finalize;
  gobject_class->get_property = gm_video_widget_get_property;
  gobject_class->set_property = gm_video_widget_set_property;

  spec = g_param_spec_int ("animation_duration", "Animation Duration", "The duration of animations",
                           0, MAX_ANIMATION_DURATION, DEFAULT_ANIMATION_DURATION,
                           (GParamFlags) G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, GM_VIDEO_WIDGET_ANIMATION_DURATION, spec);

  spec = g_param_spec_string ("logo", "Application Logo", "The application logo",
                              NULL, (GParamFlags) G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, GM_VIDEO_WIDGET_LOGO, spec);

  spec = g_param_spec_int ("secondary_stream_margin", "Secondary Stream Margin",
                           "The secondary stream margin when both streams are active",
                           0, 120, DEFAULT_SECONDARY_STREAM_MARGIN,
                           (GParamFlags) G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, GM_VIDEO_WIDGET_SECONDARY_STREAM_MARGIN, spec);

  spec = g_param_spec_float ("secondary_stream_scale", "Secondary Stream Scale",
                             "The secondary stream scale when both streams are active",
                             0.0, 1.0, DEFAULT_SECONDARY_STREAM_SCALE,
                             (GParamFlags) G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, GM_VIDEO_WIDGET_SECONDARY_STREAM_SCALE, spec);

  spec = g_param_spec_int ("logo_margin", "Logo Margin",
                           "The logo margin when both streams are active",
                           0, 120, DEFAULT_LOGO_MARGIN,
                           (GParamFlags) G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, GM_VIDEO_WIDGET_LOGO_MARGIN, spec);

  spec = g_param_spec_float ("logo_scale", "Logo Scale",
                             "The logo scale when both streams are active",
                             0.0, 1.0, DEFAULT_LOGO_SCALE,
                             (GParamFlags) G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, GM_VIDEO_WIDGET_LOGO_SCALE, spec);

  spec = g_param_spec_boolean ("secondary_stream_display", "Display secondary stream",
                               "Display secondary stream when both streams are active",
                               TRUE, (GParamFlags) G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, GM_VIDEO_WIDGET_SECONDARY_STREAM_DISPLAY, spec);
}


static void
gm_video_widget_init (GmVideoWidget* self)
{
  int i = 0;

  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GM_TYPE_VIDEO_WIDGET, GmVideoWidgetPrivate);
  self->priv->animation_duration = DEFAULT_ANIMATION_DURATION;
  self->priv->logo = NULL;
  self->priv->secondary_stream_scale = DEFAULT_SECONDARY_STREAM_SCALE;
  self->priv->secondary_stream_margin = DEFAULT_SECONDARY_STREAM_MARGIN;
  self->priv->secondary_stream_display = TRUE;
  self->priv->logo_scale = DEFAULT_LOGO_SCALE;
  self->priv->logo_margin = DEFAULT_LOGO_MARGIN;

  ClutterActor *stage = NULL;

  gtk_widget_set_size_request (GTK_WIDGET (self), DEFAULT_WIDTH, DEFAULT_HEIGHT);

  stage = gtk_clutter_embed_get_stage (GTK_CLUTTER_EMBED (self));
  clutter_actor_set_background_color (CLUTTER_ACTOR (stage), CLUTTER_COLOR_Black);
  clutter_stage_set_user_resizable (CLUTTER_STAGE (stage), TRUE);
  self->priv->available_height = clutter_actor_get_height (stage);
  self->priv->available_width = clutter_actor_get_width (stage);

  for (i = 0 ; i < MAX_STREAM ; ++i) {
    self->priv->state[i] = STREAM_STATE_STOPPED;
    self->priv->natural_width[i] = 0;
    self->priv->natural_height[i] = 0;
    self->priv->stream[i] =
      CLUTTER_ACTOR (g_object_new (CLUTTER_TYPE_TEXTURE, "disable-slicing", TRUE, NULL));
    clutter_actor_add_child (stage, self->priv->stream[i]);
    clutter_actor_set_opacity (self->priv->stream[i], 0);
    clutter_actor_add_constraint (self->priv->stream[i],
                                  clutter_align_constraint_new (stage, CLUTTER_ALIGN_BOTH, 0.5));
  }

  self->priv->emblem = clutter_actor_new ();
  self->priv->logo_width = 0;
  self->priv->logo_height = 0;
  clutter_actor_set_opacity (self->priv->emblem, 0);
  clutter_actor_add_constraint (self->priv->emblem,
                                clutter_align_constraint_new (stage, CLUTTER_ALIGN_BOTH, 0.5));
  clutter_actor_add_child (stage, self->priv->emblem);

  g_signal_connect (stage, "allocation-changed",
                    G_CALLBACK (gm_video_widget_resized_cb), self);
  g_signal_connect (gtk_widget_get_toplevel (GTK_WIDGET (self)), "show",
                    G_CALLBACK (gm_video_widget_shown_cb), self);
}


/*
 * Our own stuff
 */
static void
gm_video_widget_actor_scale (ClutterActor *actor,
                             const unsigned available_height,
                             const unsigned width,
                             const unsigned height)
{
  gfloat zoom = 0;

  zoom = (gfloat) available_height / height;
  clutter_actor_set_size (actor, width * zoom, height * zoom);
}


static void
gm_video_widget_stream_resize (GmVideoWidget *self,
                               const GM_STREAM_TYPE type)
{
  g_return_if_fail (GM_IS_VIDEO_WIDGET (self));

  gfloat ratio = 0;

  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GM_TYPE_VIDEO_WIDGET, GmVideoWidgetPrivate);

  if (self->priv->available_height > 0
      && self->priv->available_width > 0
      && self->priv->natural_height[type] > 0
      && self->priv->natural_width[type] > 0) {

    ratio = (type == SECONDARY_STREAM
             && self->priv->state[PRIMARY_STREAM] == STREAM_STATE_PLAYING
             && self->priv->state[SECONDARY_STREAM] == STREAM_STATE_PLAYING) ?
      self->priv->secondary_stream_scale : 1;

    clutter_actor_save_easing_state (self->priv->stream[type]);
    clutter_actor_set_easing_duration (self->priv->stream[type],
                                       self->priv->animation_duration);
    gm_video_widget_actor_scale (self->priv->stream[type],
                                 self->priv->available_height * ratio,
                                 self->priv->natural_width[type],
                                 self->priv->natural_height[type]);
    clutter_actor_restore_easing_state (CLUTTER_ACTOR (self->priv->stream[type]));
  }
}


static void
gm_video_widget_stream_align (GmVideoWidget *self,
                              const GM_STREAM_TYPE type,
                              const gboolean pip)
{
  ClutterActor *stage = NULL;
  g_return_if_fail (GM_IS_VIDEO_WIDGET (self));

  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GM_TYPE_VIDEO_WIDGET, GmVideoWidgetPrivate);

  stage = gtk_clutter_embed_get_stage (GTK_CLUTTER_EMBED (self));

  clutter_actor_clear_constraints (self->priv->stream[type]);
  if (pip) {
    clutter_actor_add_constraint (self->priv->stream[type],
                                  clutter_align_constraint_new (stage,
                                                                CLUTTER_ALIGN_X_AXIS,
                                                                0.0));
    clutter_actor_add_constraint (self->priv->stream[type],
                                  clutter_align_constraint_new (stage,
                                                                CLUTTER_ALIGN_Y_AXIS,
                                                                1.0));
  }
  else
    clutter_actor_add_constraint (self->priv->stream[type],
                                  clutter_align_constraint_new (stage, CLUTTER_ALIGN_BOTH, 0.5));

  clutter_actor_set_margin_bottom (self->priv->stream[type],
                                   pip ? self->priv->secondary_stream_margin : 0);
  clutter_actor_set_margin_left (self->priv->stream[type],
                                 pip ? self->priv->secondary_stream_margin : 0);
}


static void
gm_video_widget_update_emblem (GmVideoWidget *self)
{
  ClutterActor *stage = NULL;

  g_return_if_fail (GM_IS_VIDEO_WIDGET (self));

  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GM_TYPE_VIDEO_WIDGET, GmVideoWidgetPrivate);

  if (!self->priv->logo)
    return;

  stage = gtk_clutter_embed_get_stage (GTK_CLUTTER_EMBED (self));

  clutter_actor_clear_constraints (self->priv->emblem);
  if (self->priv->state[PRIMARY_STREAM] == STREAM_STATE_PLAYING
      || self->priv->state[SECONDARY_STREAM] == STREAM_STATE_PLAYING) {

    clutter_actor_add_constraint (self->priv->emblem,
                                  clutter_align_constraint_new (stage,
                                                                CLUTTER_ALIGN_X_AXIS,
                                                                1.0));
    clutter_actor_add_constraint (self->priv->emblem,
                                  clutter_align_constraint_new (stage,
                                                                CLUTTER_ALIGN_Y_AXIS,
                                                                0.0));
    clutter_actor_set_margin_top (self->priv->emblem, self->priv->logo_margin);
    clutter_actor_set_margin_right (self->priv->emblem, self->priv->logo_margin);
    gm_video_widget_actor_scale (self->priv->emblem,
                                 self->priv->available_height * self->priv->logo_scale,
                                 self->priv->logo_width,
                                 self->priv->logo_height);
  }
  else {
    clutter_actor_add_constraint (self->priv->emblem,
                                  clutter_align_constraint_new (stage, CLUTTER_ALIGN_BOTH, 0.5));
    clutter_actor_set_margin_top (self->priv->emblem, 0);
    clutter_actor_set_margin_right (self->priv->emblem, 0);
    clutter_actor_set_size (self->priv->emblem,
                            self->priv->logo_width,
                            self->priv->logo_height);
  }
}


static void
gm_video_widget_resized_cb (ClutterActor *stage,
                            G_GNUC_UNUSED ClutterActorBox *box,
                            G_GNUC_UNUSED ClutterAllocationFlags flags,
                            gpointer data)
{
  g_return_if_fail (GM_IS_VIDEO_WIDGET (data));

  int i = 0;
  GmVideoWidget *self = GM_VIDEO_WIDGET (data);
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GM_TYPE_VIDEO_WIDGET, GmVideoWidgetPrivate);

  self->priv->available_height = clutter_actor_get_height (stage);
  self->priv->available_width = clutter_actor_get_width (stage);
  for (i = 0 ; i < MAX_STREAM ; ++i)
    gm_video_widget_stream_resize (self, i);
}


static void
gm_video_widget_shown_cb (G_GNUC_UNUSED GtkWidget *selfe,
                          gpointer data)
{
  g_return_if_fail (GM_IS_VIDEO_WIDGET (data));

  GmVideoWidget *self = GM_VIDEO_WIDGET (data);
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GM_TYPE_VIDEO_WIDGET, GmVideoWidgetPrivate);

  clutter_actor_save_easing_state (self->priv->emblem);
  clutter_actor_set_easing_duration (self->priv->emblem, 8000);
  clutter_actor_set_opacity (self->priv->emblem, 255);
  clutter_actor_restore_easing_state (self->priv->emblem);
}


/*
 * Public API
 */
GtkWidget *
gm_video_widget_new ()
{
  return GTK_WIDGET (g_object_new (GM_TYPE_VIDEO_WIDGET, NULL));
}


void
gm_video_widget_set_stream_state (GmVideoWidget *self,
                                  const GM_STREAM_TYPE type,
                                  const GM_STREAM_STATE state)
{
  g_return_val_if_fail (self != NULL, NULL);

  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GM_TYPE_VIDEO_WIDGET, GmVideoWidgetPrivate);

  if (self->priv->state[type] == state)
    return;

  self->priv->state[type] = state;

  /* Update the emblem position and size */
  gm_video_widget_update_emblem (self);

  /* If both streams are playing, move and resize the secondary stream */
  if (self->priv->state[PRIMARY_STREAM] == STREAM_STATE_PLAYING
      && self->priv->state[SECONDARY_STREAM] == STREAM_STATE_PLAYING) {

    g_return_if_fail (self->priv->natural_width[SECONDARY_STREAM] > 0
                      && self->priv->natural_height[SECONDARY_STREAM] > 0);

    gm_video_widget_stream_align (self, SECONDARY_STREAM, TRUE);
    gm_video_widget_stream_resize (self, SECONDARY_STREAM);

    clutter_actor_set_opacity (self->priv->stream[PRIMARY_STREAM], 255);
    clutter_actor_set_opacity (self->priv->stream[SECONDARY_STREAM],
                               self->priv->secondary_stream_display ? 255:0);
  }
  else
    clutter_actor_set_opacity (self->priv->stream[type],
                               (self->priv->state[type] == STREAM_STATE_PLAYING) ? 255:0);

  if (state == STREAM_STATE_PLAYING) {
    g_return_if_fail (self->priv->natural_width[type] > 0 && self->priv->natural_height[type] > 0);
    gm_video_widget_stream_resize (self, type);
  }
  else {

    gm_video_widget_stream_align (self, type, FALSE);
    self->priv->natural_width[type] = 0;
    self->priv->natural_height[type] = 0;
  }
}


ClutterActor *
gm_video_widget_get_stream (GmVideoWidget *self,
                            const GM_STREAM_TYPE type)
{
  g_return_val_if_fail (self != NULL, NULL);
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GM_TYPE_VIDEO_WIDGET, GmVideoWidgetPrivate);

  return CLUTTER_ACTOR (self->priv->stream[type]);
}


void
gm_video_widget_set_stream_natural_size (GmVideoWidget *self,
                                         const GM_STREAM_TYPE type,
                                         const unsigned width,
                                         const unsigned height)
{
  g_return_if_fail (self != NULL);
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GM_TYPE_VIDEO_WIDGET, GmVideoWidgetPrivate);

  self->priv->natural_width[type] = width;
  self->priv->natural_height[type] = height;

  gm_video_widget_stream_resize (self, type);
}


void
gm_video_widget_get_stream_natural_size (GmVideoWidget *self,
                                         const GM_STREAM_TYPE type,
                                         unsigned *width,
                                         unsigned *height)
{
  g_return_if_fail (self && width && height);
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GM_TYPE_VIDEO_WIDGET, GmVideoWidgetPrivate);

  *width = self->priv->natural_width[type];
  *height = self->priv->natural_height[type];
}


void
gm_video_widget_set_animation_duration (GmVideoWidget *self,
                                        const int animation_duration)
{
  g_return_if_fail (GM_IS_VIDEO_WIDGET (self));

  g_object_set (self, "animation_duration", animation_duration, NULL);
}


int
gm_video_widget_get_animation_duration (GmVideoWidget *self)
{
  g_return_val_if_fail (GM_IS_VIDEO_WIDGET (self), -1);
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GM_TYPE_VIDEO_WIDGET, GmVideoWidgetPrivate);

  return self->priv->animation_duration;
}


void
gm_video_widget_set_logo (GmVideoWidget *self,
                          const gchar *logo)
{
  g_return_if_fail (GM_IS_VIDEO_WIDGET (self));

  g_object_set (self, "logo", logo, NULL);
}


void
gm_video_widget_set_secondary_stream_margin (GmVideoWidget *self,
                                             const unsigned margin)
{
  g_return_if_fail (GM_IS_VIDEO_WIDGET (self));

  g_object_set (self, "secondary_stream_margin", margin, NULL);
}


unsigned
gm_video_widget_get_secondary_stream_margin (GmVideoWidget *self)
{
  g_return_val_if_fail (GM_IS_VIDEO_WIDGET (self), -1);
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GM_TYPE_VIDEO_WIDGET, GmVideoWidgetPrivate);

  return self->priv->secondary_stream_margin;
}


void
gm_video_widget_set_secondary_stream_scale (GmVideoWidget *self,
                                            const float scale)
{
  g_return_if_fail (GM_IS_VIDEO_WIDGET (self));

  g_object_set (self, "secondary_stream_scale", scale, NULL);
}


gfloat
gm_video_widget_get_secondary_stream_scale (GmVideoWidget *self)
{
  g_return_val_if_fail (GM_IS_VIDEO_WIDGET (self), -1);
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GM_TYPE_VIDEO_WIDGET, GmVideoWidgetPrivate);

  return self->priv->secondary_stream_scale;
}


void
gm_video_widget_set_secondary_stream_display (GmVideoWidget *self,
                                              const gboolean display)
{
  g_return_if_fail (GM_IS_VIDEO_WIDGET (self));

  g_object_set (self, "secondary_stream_display", display, NULL);
}


gboolean
gm_video_widget_get_secondary_stream_display (GmVideoWidget *self)
{
  g_return_val_if_fail (GM_IS_VIDEO_WIDGET (self), -1);
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GM_TYPE_VIDEO_WIDGET, GmVideoWidgetPrivate);

  return self->priv->secondary_stream_display;
}


void
gm_video_widget_set_logo_margin (GmVideoWidget *self,
                                 const unsigned margin)
{
  g_return_if_fail (GM_IS_VIDEO_WIDGET (self));

  g_object_set (self, "logo_margin", margin, NULL);
}


unsigned
gm_video_widget_get_logo_margin (GmVideoWidget *self)
{
  g_return_val_if_fail (GM_IS_VIDEO_WIDGET (self), -1);
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GM_TYPE_VIDEO_WIDGET, GmVideoWidgetPrivate);

  return self->priv->logo_margin;
}


void
gm_video_widget_set_logo_scale (GmVideoWidget *self,
                                const float scale)
{
  g_return_if_fail (GM_IS_VIDEO_WIDGET (self));

  g_object_set (self, "logo_scale", scale, NULL);
}


gfloat
gm_video_widget_get_logo_scale (GmVideoWidget *self)
{
  g_return_val_if_fail (GM_IS_VIDEO_WIDGET (self), -1);
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GM_TYPE_VIDEO_WIDGET, GmVideoWidgetPrivate);

  return self->priv->logo_scale;
}
