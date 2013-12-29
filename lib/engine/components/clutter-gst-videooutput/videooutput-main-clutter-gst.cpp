
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
 *                         videooutput-main-clutter.cpp  -  description
 *                         --------------------------------------------
 *   begin                : Sun 15 December 2013
 *   copyright            : (c) 2013 by Damien Sandras
 *   description          : code to hook the Clutter display manager into the main program
 *
 */

#include <clutter/clutter.h>
#include <gst/gst.h>
#include <clutter-gtk/clutter-gtk.h>

#include "videooutput-core.h"

#include "videooutput-main-clutter-gst.h"
#include "videooutput-manager-clutter-gst.h"

bool
videooutput_clutter_gst_init (Ekiga::ServiceCore &core,
                              G_GNUC_UNUSED int *argc,
                              G_GNUC_UNUSED char **argv[])
{
  bool result = false;
  boost::shared_ptr<Ekiga::VideoOutputCore> videooutput_core =
    core.get<Ekiga::VideoOutputCore> ("videooutput-core");

  if (videooutput_core) {

    gst_init (argc, argv);
    gtk_clutter_init (argc, argv);
    GMVideoOutputManager_clutter_gst *videooutput_manager = new GMVideoOutputManager_clutter_gst (core);

    videooutput_core->add_manager (*videooutput_manager);
    result = true;
  }

  return result;
}
