
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
 *                         gmconf-ekiga-keys.h  -  description
 *                         ------------------------
 *   begin                : Sat Dec 23 2000
 *   copyright            : (C) 2000-2013 by Damien Sandras
 *   description          : This file defines gmconf keys for all
 *                          of ekiga to use
 *
 */


#ifndef GMCONF_EKIGA_KEYS_H_
#define GMCONF_EKIGA_KEYS_H_

#define GENERAL_KEY         "/apps/" PACKAGE_NAME "/general/"
#define USER_INTERFACE_KEY "/apps/" PACKAGE_NAME "/general/user_interface/"
#define CONTACTS_KEY "/apps/" PACKAGE_NAME "/contacts/"
#define VIDEO_DISPLAY_KEY USER_INTERFACE_KEY "video_display/"
#define SOUND_EVENTS_KEY  "/apps/" PACKAGE_NAME "/general/sound_events/"
#define AUDIO_DEVICES_KEY "/apps/" PACKAGE_NAME "/devices/audio/"
#define VIDEO_DEVICES_KEY "/apps/" PACKAGE_NAME "/devices/video/"
#define PERSONAL_DATA_KEY "/apps/" PACKAGE_NAME "/general/personal_data/"
#define CALL_OPTIONS_KEY "/apps/" PACKAGE_NAME "/general/call_options/"
#define NAT_KEY "/apps/" PACKAGE_NAME "/general/nat/"
#define PROTOCOLS_KEY "/apps/" PACKAGE_NAME "/protocols/"
#define H323_KEY "/apps/" PACKAGE_NAME "/protocols/h323/"
#define SIP_KEY "/apps/" PACKAGE_NAME "/protocols/sip/"
#define PORTS_KEY "/apps/" PACKAGE_NAME "/protocols/ports/"
#define CALL_FORWARDING_KEY "/apps/" PACKAGE_NAME "/protocols/call_forwarding/"
#define LDAP_KEY "/apps/" PACKAGE_NAME "/protocols/ldap/"
#define CODECS_KEY "/apps/" PACKAGE_NAME "/codecs/"
#define AUDIO_CODECS_KEY "/apps/" PACKAGE_NAME "/codecs/audio/"
#define VIDEO_CODECS_KEY  "/apps/" PACKAGE_NAME "/codecs/video/"

#endif /* GMCONF_EKIGA_KEYS_H */
