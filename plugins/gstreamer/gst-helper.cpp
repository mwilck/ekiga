/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2012 Damien Sandras <dsandras@seconix.com>
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
 *                         gst-helper.cpp  -  description
 *                         ------------------------------------
 *   begin                : Wed 1 February 2012
 *   copyright            : (C) 2012 by Julien Puydt
 *   description          : Gstreamer helper code implementation
 *
 */

#include "gst-helper.h"

#include <gst/base/gstadapter.h>
#include <gst/app/gstappsrc.h>
#include <gst/app/gstappsink.h>
#include <gst/app/gstappbuffer.h>

struct gst_helper
{
  GstElement* pipeline;
  GstElement* active;
  GstElement* volume;
  GstAdapter* adapter;
};

static void
gst_helper_destroy (gst_helper* self)
{
  gst_element_set_state (self->pipeline, GST_STATE_NULL);
  gst_object_unref (self->adapter);
  self->adapter = NULL;
  g_object_unref (self->active);
  self->active = NULL;
  if (self->volume)
    g_object_unref (self->volume);
  self->volume = NULL;
  g_object_unref (self->pipeline);
  self->pipeline = NULL;
  g_free (self);
}

gst_helper*
gst_helper_new (const gchar* command)
{
  gst_helper* self = g_new0 (gst_helper, 1);
  self->adapter = gst_adapter_new ();
  self->pipeline = gst_parse_launch (command, NULL);
  self->volume = gst_bin_get_by_name (GST_BIN (self->pipeline), "ekiga_volume");
  self->active = gst_bin_get_by_name (GST_BIN (self->pipeline), "ekiga_sink");
  if (self->active == NULL) {

    self->active = gst_bin_get_by_name (GST_BIN (self->pipeline), "ekiga_src");
  }
  (void)gst_element_set_state (self->pipeline, GST_STATE_PLAYING);

  return self;
}

void
gst_helper_close (gst_helper* self)
{
  gst_helper_destroy (self);
}

bool
gst_helper_get_frame_data (gst_helper* self,
			   char* data,
			   unsigned size,
			   unsigned& read)
{
  GstBuffer* buffer = NULL;

  buffer = gst_app_sink_pull_buffer (GST_APP_SINK (self->active));
  if (buffer != NULL)
    gst_adapter_push (self->adapter, buffer);

  read = MIN(size, gst_adapter_available (self->adapter));
  gst_adapter_copy (self->adapter, (guint8*)data, 0, read);
  gst_adapter_flush (self->adapter, read);
  g_usleep (20 * G_TIME_SPAN_MILLISECOND);

  return true;
}

void
gst_helper_set_frame_data (gst_helper* self,
			   const char* data,
			   unsigned size)
{
  gchar* tmp = NULL;
  GstBuffer* buffer = NULL;

  if (self->active) {

    tmp = (gchar*)g_malloc0 (size);
    memcpy (tmp, data, size);
    buffer = gst_app_buffer_new (tmp, size,
				 (GstAppBufferFinalizeFunc)g_free, tmp);
    gst_app_src_push_buffer (GST_APP_SRC (self->active), buffer);
    g_usleep (20 * G_TIME_SPAN_MILLISECOND);
  }
}

void
gst_helper_set_volume (gst_helper* self,
		       gfloat valf)
{
  if (self->volume)
    g_object_set (G_OBJECT (self->volume),
		  "volume", valf,
		  NULL);
}

gfloat
gst_helper_get_volume (gst_helper* self)
{
  gfloat result = -1;
  if (self->volume)
    g_object_get (G_OBJECT (self->volume),
		  "volume", &result,
		  NULL);

  return result;
}

void
gst_helper_set_buffer_size (gst_helper* self,
			    unsigned size)
{
  if (self->active)
    g_object_set (G_OBJECT (self->active),
		  "blocksize", size,
		  NULL);
}
