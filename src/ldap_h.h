/***************************************************************************
                          ldap_h.h  -  description
                             -------------------
    begin                : Wed Feb 28 2001
    copyright            : (C) 2000-2001 by Damien Sandras
    description          : This file contains all the functions needed 
                           for ILS support
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


#ifndef _LDAP_H_H_
#define _LDAP_H_H_

#include <lber.h>
#include <ldap.h>
#include <iostream.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <gnome.h>
#include <glib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <ptlib.h>

#include "common.h"


/******************************************************************************/
/* The GTK callbacks                                                          */
/******************************************************************************/

// DESCRIPTION  :  This callback is called when the user clicks
//                 on a button : OK, CANCEL, APPLY, or closed by the WM
// BEHAVIOR     :  free memory and cancel fetch results thread
// PRE          :  gpointer is a valid pointer to a GM_ldap_window_widgets
void ldap_window_clicked (GnomeDialog *, int, gpointer);


// DESCRIPTION  :  This callback is called when the user destroys
//                 the window thanks to the WM
// BEHAVIOR     :  prevents the window to be closed
// PRE          :  gpointer is a valid pointer to a GM_ldap_window_widgets
gint ldap_window_destroyed (GtkWidget *, gpointer);


// DESCRIPTION  :  This callback is called when the user has modified
//                 the search entry field
// BEHAVIOR     :  Reset the search counter
// PRE          :  gpointer is a valid pointer to a GM_ldap_window_widgets
void search_entry_modified (GtkWidget *, gpointer);


// DESCRIPTION  :  This callback is called when the user clicks
//                 on the refresh button
// BEHAVIOR     :  If there is no fetch results thread, calls GM_ldap_populate
//                 in a new thread to browse the ILS directory
// PRE          :  gpointer is a valid pointer to a GM_ldap_window_widgets
void refresh_button_clicked (GtkButton *, gpointer);


// DESCRIPTION  :  This callback is called when the user clicks on the add
//                 button
// BEHAVIOR     :  if a row is selected in the users list, had his IP to the
//                 IP entry field of the main window
// PRE          :  gpointer is a valid pointer to a GM_ldap_window_widgets
void user_add_button_clicked (GtkButton *, gpointer);


// DESCRIPTION  :  This callback is called when the user clicks on the apply
//                 filter button
// BEHAVIOR     :  make an incremental search on the entry content
// PRE          :  gpointer is a valid pointer to a GM_ldap_window_widgets
void apply_filter_button_clicked (GtkButton *, gpointer);


// DESCRIPTION  :  This callback is called when the user selects
//                 a row in the users list
// BEHAVIOR     :  Simply selects the row and put the selected row number in a
//                 variable
// PRE          :  gpointer is a valid pointer to a GM_ldap_window_widgets
void ldap_clist_row_selected (GtkWidget *, gint, gint,
				     GdkEventButton *, gpointer);


// DESCRIPTION  :  This callback is called when the user clicks
//                 on a column of the users list
// BEHAVIOR     :  Reorder the column using an ascending or descending sort
// PRE          :  gpointer is a valid pointer to a GM_ldap_window_widgets
void ldap_clist_column_clicked (GtkCList *, gint, gpointer);

/******************************************************************************/


/******************************************************************************/
/* The different functions to build the ldap window                           */
/******************************************************************************/

// DESCRIPTION  :  /
// BEHAVIOR     :  builds the ILS window
// PRE          :  GM_window_widgets is a valid pointer to a
//                 GM_window_widgets
void GM_ldap_init (GM_window_widgets *);


// DESCRIPTION  :  /
// BEHAVIOR     :  populate the users list IF ldap support is enabled
// PRE          :  /
void *GM_ldap_populate_ldap_users_clist (void *);


// DESCRIPTION  :  /
// BEHAVIOR     :  register to a ILS directory
// PRE          :  user options must be ok for ILS support, GM_window_widgets ok
void *GM_ldap_register (char *, GM_window_widgets *);


// DESCRIPTION  :  /
// BEHAVIOR     :  refresh the ttl entry of a ILS directory
// PRE          :  user options must be ok for ILS support
void *GM_ldap_refresh (void);

/******************************************************************************/

#endif
