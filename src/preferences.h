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

/*                         preferences.h
 *                       -------------------
 *   begin                : Tue Dec 26 2000
 *   copyright            : (C) 2000-2001 by Damien Sandras
 *   description          : This file contains all functions needed to create
 *                          the preferences window and all its callbacks
 *   email                : dsandras@seconix.com
 */

#ifndef _PREFERENCES_H_
#define _PREFERENCES_H_

#include <gnome.h>

#include "common.h"


/* DESCRIPTION  :  /
 * BEHAVIOR     :  It builds the preferences window
 *                 (sections' ctree / Notebook pages) and connect GTK signals
 *                 to appropriate callbacks
 * PRE          :  The first parameter is the calling_state when the window
 *                 is created.  The second one a pointer
 *                 to a valid GM_window_widgets
 */
void gnomemeeting_preferences_init (int, GM_window_widgets *, 
				    GM_pref_window_widgets *,
				    options *);

#endif
