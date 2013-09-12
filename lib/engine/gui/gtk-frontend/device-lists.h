
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2013 Damien Sandras <dsandras@seconix.com>
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
 *                         device-lists.h  -  description
 *                         -------------------------------
 *   description          : This file declares functions to get device lists
 */

#include <glib.h>

#include "audiooutput-core.h"
#include "audioinput-core.h"
#include "videoinput-core.h"

G_BEGIN_DECLS

gchar**
vector_of_string_to_array (const std::vector<std::string>& list);

void
get_audiooutput_devices (boost::shared_ptr<Ekiga::AudioOutputCore> audiooutput_core,
			 std::vector<std::string>& device_list);

void
get_audioinput_devices (boost::shared_ptr<Ekiga::AudioInputCore> audioinput_core,
			std::vector<std::string>& device_list);

void
get_videoinput_devices (boost::shared_ptr<Ekiga::VideoInputCore> videooinput_core,
			std::vector<std::string>& device_list);

G_END_DECLS
