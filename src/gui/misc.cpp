
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2009 Damien Sandras <dsandras@seconix.com>
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
 *                         misc.cpp  -  description
 *                         ------------------------
 *   begin                : Thu Nov 22 2001
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : This file contains miscellaneous functions.
 *   Additional Code      : De Michele Cristiano, Miguel Rodríguez
 *
 */


#include "config.h"

#include "misc.h"
#include "ekiga.h"
#include "callbacks.h"

#include "gmdialog.h"
#include "gmconf.h"

#include <glib/gi18n.h>


/* return the default audio device name */
const gchar *get_default_audio_device_name (void)
{
#ifdef WIN32
  return "Default (PTLIB/WindowsMultimedia)";
#else
  return "Default (PTLIB/ALSA)";
#endif
}

/* return the default video name from the list of existing devices */
const gchar *get_default_video_device_name (const gchar * const *options)
{
#ifdef WIN32
  /* look for the entry containing "PTLIB/DirectShow" or "PTLIB/VideoForWindows" */
  for (int i = 0; options[i]; i++)
    if (g_strrstr (options[i], "PTLIB/DirectShow")
        || g_strrstr (options[i], "PTLIB/VideoForWindows"))
      return options[i];
#else
  /* look for the entry containing "PTLIB/V4L2", otherwise "PTLIB/V4L" */
  for (int i = 0; options[i]; i++)
    if (g_strrstr (options[i], "PTLIB/V4L2"))
      return options[i];
  for (int i = 0; options[i]; i++)
    if (g_strrstr (options[i], "PTLIB/V4L"))
      return options[i];
#endif
  return NULL;  // not found
}
