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
 *                         gst-helper.h  -  description
 *                         ------------------------------------
 *   begin                : Wed 1 February 2012
 *   copyright            : (C) 2012 by Julien Puydt
 *   description          : Gstreamer helper code declaration
 *
 */


/* This code is a factorization of a pattern common to the three gstreamer
 * classes in the ekiga plugin code :
 * - there must be a way to create a new helper, which can have an optional
 * ekiga_volume element, and a mandatory active element, which is either an
 * ekiga_sink or an ekiga_src ;
 * - it should be possible to ask this helper to just kill itself ;
 * - it should be possible to either put data into it, or get data from it ;
 * - the optional volume should be modifyable (-1 means the option is disabled) ;
 * - it should be possible to set the buffer size.
 */


#include <glib/gi18n.h>

struct gst_helper;

gst_helper* gst_helper_new (const gchar* command);

void gst_helper_close (gst_helper* self);

bool gst_helper_get_frame_data (gst_helper* self,
				char* data,
				unsigned size,
				unsigned& read);

void gst_helper_set_frame_data (gst_helper* self,
				const char* data,
				unsigned size);

void gst_helper_set_volume (gst_helper* self,
			    gfloat valf);

gfloat gst_helper_get_volume (gst_helper* self);

void gst_helper_set_buffer_size (gst_helper* self,
				 unsigned size);
