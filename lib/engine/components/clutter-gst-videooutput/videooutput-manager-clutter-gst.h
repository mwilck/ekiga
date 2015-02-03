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
 *                         videooutput-manager-clutter-gst.h  -  description
 *                         -------------------------------------------------
 *   begin                : Sun 15 December 2013
 *   copyright            : (c) 2013 by Damien Sandras
 *   description          : Clutter VideoOutput Manager code.
 *
 */

#ifndef _VIDEOOUTPUT_MANAGER_CLUTTER_GST_H_
#define _VIDEOOUTPUT_MANAGER_CLUTTER_GST_H_

#include "services.h"
#include "videooutput-manager.h"

#include <glib.h>

/**
 * @addtogroup videooutput
 * @{
 */

class GMVideoOutputManager_clutter_gst : public Ekiga::VideoOutputManager
{
public:
  GMVideoOutputManager_clutter_gst (Ekiga::ServiceCore & _core);

  ~GMVideoOutputManager_clutter_gst ();

  void quit ();

  void open ();

  void close ();

  void set_frame_data (const char *data,
                       unsigned width,
                       unsigned height,
                       Ekiga::VideoOutputManager::VideoView type,
                       int devices_nbr);

  void set_display_info (const gpointer local_video,
                         const gpointer remote_video);

  void set_ext_display_info (const gpointer ext_video);

private:
  void size_changed_in_main (Ekiga::VideoOutputManager::VideoView type,
                             unsigned width,
			     unsigned height);

  void device_opened_in_main (Ekiga::VideoOutputManager::VideoView type,
                              unsigned width,
                              unsigned height,
                              bool both,
			      bool ext);

  void device_closed_in_main ();

  void device_error_in_main ();

  // Variables
  PMutex device_mutex;

  // 0 = local, 1 = remote, 2 = extended
  unsigned current_width[3];
  unsigned current_height[3];
  GstElement *pipeline[3];
  ClutterActor *texture[3];

  int devices_nbr;
};

/**
 * @}
 */

#endif
