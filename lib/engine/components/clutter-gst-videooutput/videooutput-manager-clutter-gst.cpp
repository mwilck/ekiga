
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
 *                         videooutput-manager-clutter-gst.cpp  -  description
 *                         ---------------------------------------------------
 *   begin                : Sun 15 December 2013
 *   copyright            : (c) 2013 by Damien Sandras
 *   description          : Clutter VideoOutput Manager code.
 *
 */


#include <ptlib.h>

#include <gtk/gtk.h>
#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <clutter-gtk/clutter-gtk.h>

#include "videooutput-manager-clutter-gst.h"
#include "videoinput-info.h"

#include "runtime.h"


GMVideoOutputManager_clutter_gst::GMVideoOutputManager_clutter_gst (G_GNUC_UNUSED Ekiga::ServiceCore & _core)
{
  devices_nbr = 0;

  for (int i = 0 ; i < 3 ; i++) {
    texture[i] = NULL;
    pipeline[i] = NULL;
  }
}


GMVideoOutputManager_clutter_gst::~GMVideoOutputManager_clutter_gst ()
{
}


void
GMVideoOutputManager_clutter_gst::quit ()
{
}


void
GMVideoOutputManager_clutter_gst::open ()
{
  GstElement *appsrc = NULL;
  GstElement *videosink = NULL;
  GstElement *conv = NULL;
  GstCaps *caps = NULL;
  PWaitAndSignal m(device_mutex);

  for (int i = 0 ; i < 3 ; ++i) {

    std::ostringstream name;
    name << std::string ("appsrc") << i;
    pipeline[i] = gst_pipeline_new (NULL);
    videosink = gst_element_factory_make ("autocluttersink", "videosink");
    if (videosink == NULL)
      videosink = gst_element_factory_make ("cluttersink", "videosink");
    g_object_set (videosink, "texture", texture[i], NULL);

    appsrc = gst_element_factory_make ("appsrc", name.str ().c_str ());
    conv = gst_element_factory_make ("videoconvert", NULL);

    /* set the caps on the source */
    current_width[i] = 0;
    current_height[i]= 0;
    caps = gst_caps_new_simple ("video/x-raw",
                                "format", G_TYPE_STRING, "I420",
                                "framerate", GST_TYPE_FRACTION, 0, 1,
                                "pixel-aspect-ratio" ,GST_TYPE_FRACTION, 1, 1,
                                "width", G_TYPE_INT, 704,
                                "height", G_TYPE_INT, 576,
                                "red_mask",   G_TYPE_INT, 0x00ff0000,
                                "green_mask", G_TYPE_INT, 0x0000ff00,
                                "blue_mask",  G_TYPE_INT, 0x000000ff,
                                "endianness", G_TYPE_INT, G_LITTLE_ENDIAN,
                                NULL);

    if (!videosink || !appsrc || !conv || !pipeline[i]) {

      Ekiga::Runtime::run_in_main (boost::bind (&GMVideoOutputManager_clutter_gst::device_error_in_main,
                                                this));
      break;
    }

    gst_app_src_set_caps (GST_APP_SRC (appsrc), caps);
    g_object_set (G_OBJECT (appsrc),
                  "block", TRUE,
                  "max-bytes", MAX_VIDEO_SIZE*3/2,
                  "stream-type", GST_APP_STREAM_TYPE_STREAM,
                  NULL);
    gst_bin_add_many (GST_BIN (pipeline[i]), appsrc, conv, videosink, NULL);
    gst_element_link_many (appsrc, conv, videosink, NULL);
    gst_caps_unref (caps);
  }
}


void
GMVideoOutputManager_clutter_gst::close ()
{
  PWaitAndSignal m(device_mutex);
  for (int i = 0 ; i < 2 ; i++) {
    if (!pipeline[i])
      continue;

    std::ostringstream name;
    name << std::string ("appsrc") << i;
    GstElement *appsrc = gst_bin_get_by_name (GST_BIN (pipeline[i]), name.str ().c_str ());
    gst_app_src_end_of_stream (GST_APP_SRC (appsrc));
    gst_element_set_state (pipeline[i], GST_STATE_NULL);
    gst_object_unref (pipeline[i]);
    pipeline[i] = NULL;
  }
  devices_nbr = 0;

  Ekiga::Runtime::run_in_main (boost::bind (&GMVideoOutputManager_clutter_gst::device_closed_in_main,
                                            this));
}


void
GMVideoOutputManager_clutter_gst::set_frame_data (const char *data,
                                                  unsigned width,
                                                  unsigned height,
                                                  unsigned i,
                                                  int _devices_nbr)
{
  GstBuffer *buffer = NULL;
  GstMapInfo info;
  int buffer_size = width*height*3/2;
  std::ostringstream name;

  info.memory = NULL;
  info.flags = GST_MAP_WRITE;
  info.data = NULL;
  info.size = 0;
  info.maxsize = buffer_size;

  PWaitAndSignal m(device_mutex);

  if (!pipeline[i]) {
    PTRACE (1, "GMVideoOutputManager_clutter_gst\tTrying to upload frame to closed pipeline " << i);
    return;
  }

  if (_devices_nbr != devices_nbr) {
    Ekiga::Runtime::run_in_main
      (boost::bind (&GMVideoOutputManager_clutter_gst::device_opened_in_main,
                    this,
                    (_devices_nbr > 1),
                    (_devices_nbr > 2)));
    devices_nbr = (unsigned) _devices_nbr;
  }

  if (devices_nbr == 1) { // Only one video stream (either local or remote)
    // Force whatever stream is received to be displayed as the remote
    // video (the big one).
    i = 1;
  }
  name << std::string ("appsrc") << i;
  GstElement *appsrc = gst_bin_get_by_name (GST_BIN (pipeline[i]), name.str ().c_str ());

  if (current_width[i] != width || current_height[i] != height) {

    GstCaps *caps = gst_app_src_get_caps (GST_APP_SRC (appsrc));
    GstCaps *new_caps = gst_caps_copy (caps);
    gst_caps_set_simple (new_caps,
                         "width", G_TYPE_INT, width,
                         "height", G_TYPE_INT, height, NULL);
    gst_app_src_set_caps (GST_APP_SRC (appsrc), new_caps);
    gst_caps_unref (caps);
    gst_caps_unref (new_caps);

    current_height[i] = height;
    current_width[i] = width;

    Ekiga::Runtime::run_in_main (boost::bind (&GMVideoOutputManager_clutter_gst::size_changed_in_main,
                                              this,
                                              width,
                                              height,
                                              i));
  }

  buffer = gst_buffer_new_and_alloc (buffer_size);
  gst_buffer_map (buffer, &info, GST_MAP_WRITE);
  memcpy ((void *) info.data, (const void *) data, buffer_size);
  info.size = buffer_size;
  gst_buffer_unmap (buffer, &info);
  gst_app_src_push_buffer (GST_APP_SRC (appsrc), buffer);

  gst_element_set_state (pipeline[i], GST_STATE_PLAYING);
}


void
GMVideoOutputManager_clutter_gst::set_display_info (const gpointer _local_video,
                                                    const gpointer _remote_video)
{
  texture[0] = CLUTTER_ACTOR (_local_video);
  texture[1] = CLUTTER_ACTOR (_remote_video);
}


void
GMVideoOutputManager_clutter_gst::set_ext_display_info (const gpointer _ext_video)
{
  texture[2] = CLUTTER_ACTOR (_ext_video);
}


void
GMVideoOutputManager_clutter_gst::size_changed_in_main (unsigned width,
                                                        unsigned height,
                                                        unsigned type)
{
  size_changed (width, height, type);
}

void
GMVideoOutputManager_clutter_gst::device_opened_in_main (bool both,
                                                         bool ext)
{
  device_opened (both, ext);
}

void
GMVideoOutputManager_clutter_gst::device_closed_in_main ()
{
  device_closed ();
}

void
GMVideoOutputManager_clutter_gst::device_error_in_main ()
{
  device_error ();
}
