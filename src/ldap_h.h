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
//                 closes the window (destroy or delete_event signals)
// BEHAVIOR     :  hide the window
// PRE          :  gpointer is a valid pointer to a GM_ldap_window_widgets
void ldap_window_clicked (GnomeDialog *, int, gpointer);


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
void GM_ldap_init (GM_window_widgets *, GM_ldap_window_widgets *);

void GM_ldap_init_notebook (GM_ldap_window_widgets *, int, gchar *);
/******************************************************************************/

#endif
