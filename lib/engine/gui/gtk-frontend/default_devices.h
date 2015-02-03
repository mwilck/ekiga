/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2010 Damien Sandras <dsandras@seconix.com>
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
 *                         default_devices.h  -  description
 *                         ----------------------
 *   begin                : Thu Nov 22 2001
 *   copyright            : (C) 2000-2010 by Damien Sandras
 *   description          : This file contains miscellaneous functions.
 *   Additional Code      : Eugen Dedu, Julien Puydt(Snark)
 *
 */


#ifndef __DEFAULT_DEVICES_H__
#define __DEFAULT_DEVICES_H__

#include <glib.h>

/* the default audio device name */

#ifdef WIN32
#define DEFAULT_AUDIO_DEVICE_NAME "Default (PTLIB/WindowsMultimedia)"
#else
#define DEFAULT_AUDIO_DEVICE_NAME "Default (PTLIB/ALSA)"
#endif

/* returns the default video name from the list of existing devices */
const gchar *get_default_video_device_name (const gchar * const *options);

#endif
