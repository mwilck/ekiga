/***************************************************************************
                          menu.cpp  -  description
                             -------------------
    begin                : Sat Dec 23 2000
    copyright            : (C) 2000-2001 by Damien Sandras
    description          : Functions to create the menus
    email                : dsandras@acm.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _MENU_H_
#define _MENU_H_

#include <gnome.h>
#include "common.h"


/******************************************************************************/
/* The functions                                                              */
/******************************************************************************/

// DESCRIPTION  :  /
// BEHAVIOR     :  Creates the menus and add them to the main window
// PRE          :  gpointer is a valid pointer to the GM_window_widgets
void GM_menu_init (GtkWidget *, GM_window_widgets *, GM_pref_window_widgets *);


// DESCRIPTION  :  /
// BEHAVIOR     :  Creates the popup menu and attach it to the GtkWidget
//                 given as parameter (for the gtk_drawing_area)
// PRE          :  
void GM_popup_menu_init (GtkWidget *);


// DESCRIPTION  :  /
// BEHAVIOR     :  Creates the popup menu and attach it to the GtkWidget
//                 given as parameter (for the ldap window)
// PRE          :  
void GM_ldap_popup_menu_init (GtkWidget *, GM_ldap_window_widgets *);
/******************************************************************************/

#endif
