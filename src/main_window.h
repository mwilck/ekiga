
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2001 Damien Sandras
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/*
 *                         main_interface.cpp  -  description
 *                         ----------------------------------
 *   begin                : Mon Mar 26 2001
 *   copyright            : (C) 2000-2001 by Damien Sandras
 *   description          : This file contains all the functions needed to
 *                          build the main window.
 *   email                : dsandras@seconix.com
 */


#ifndef _MAIN_INTERFACE_H
#define _MAIN_INTERFACE_H

#include <gnome.h>

#include "common.h"


/* COMMON NOTICE : GM_window_widgets, GM_ldap_window_widgets and options 
                   pointers must be valid,
                   options must have been read before calling these 
		   functions */


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Initialise gnomemeeting and builds the windows.
 * PRE          :  /
 */
void gnomemeeting_init (GM_window_widgets *, GM_pref_window_widgets *, 
			GM_ldap_window_widgets *, GM_rtp_data *, 
			GmTextChat *, int, char **, char **);


#endif
