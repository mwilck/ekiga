

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
 *                         menu.h  -  description
 *                            -------------------
 *   begin                : Tue Dec 23 2000
 *   copyright            : (C) 2000-2001 by Damien Sandras
 *   description          :  Functions to create the menus.
 *   email                : dsandras@seconix.com
 *
 */

#ifndef _MENU_H_
#define _MENU_H_

#include <gnome.h>
#include "common.h"


/* The functions */


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Creates the menus and add them to the main window.
 * PRE          :  /
 */
void gnomemeeting_init_menu ();


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Creates the popup menu and attach it to the GtkWidget
 *                 given as parameter (for the gtk_drawing_area).
 * PRE          :  /
 */
void gnomemeeting_popup_menu_init (GtkWidget *, GM_window_widgets *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Creates the popup menu and attach it to the GtkWidget
 *                 given as parameter (for the ldap window).
 * PRE          :  /
 */
void gnomemeeting_init_ldap_window_popup_menu (GtkWidget *);

#endif
