
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
 *                         misc.h  -  description
 *                         ----------------------
 *   begin                : Thu Nov 22 2001
 *   copyright            : (C) 2000-2001 by Damien Sandras
 *   description          : This file contains miscellaneous functions.
 *   email                : dsandras@seconix.com
 *
 */


#ifndef _MISC_H_
#define _MISC_H_


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Takes the GDK lock if we are not in the main thread.
 * PRE          :  Must not be called instead of gdk_threads_enter in timers
 *                 or idle functions, because they are executed in the main
 *                 thread.
 */
void gnomemeeting_threads_enter ();


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Releases the GDK lock if we are not in the main thread.
 * PRE          :  Must not be called instead of gdk_threads_leave in timers
 *                 or idle functions, because they are executed in the main
 *                 thread.
 */
void gnomemeeting_threads_leave ();

#endif
