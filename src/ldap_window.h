
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
 *                         ldap_h.h  -  description
 *                         ------------------------
 *   begin                : Wed Feb 28 2001
 *   copyright            : (C) 2000-2001 by Damien Sandras
 *   description          : This file contains functions to build the ldap
 *                          window.
 *   email                : dsandras@seconix.com
 *
 */


#ifndef _LDAP_H_H_
#define _LDAP_H_H_

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <pthread.h>
#include <sys/socket.h>
#include <ptlib.h>


enum {

  COLUMN_ILS_STATUS,
  COLUMN_ILS_AUDIO,
  COLUMN_ILS_VIDEO,
  COLUMN_ILS_NAME,
  COLUMN_ILS_CALLTO,
  COLUMN_ILS_LOCATION,
  COLUMN_ILS_COMMENT,
  COLUMN_ILS_VERSION,
  COLUMN_ILS_IP,
  COLUMN_ILS_COLOR,
  NUM_COLUMNS_SERVERS
};

 
enum {

  COLUMN_NAME,
  COLUMN_CALLTO,
  COLUMN_NETMASK,
  NUM_COLUMNS_GROUPS
};


/* The functions  */

/* DESCRIPTION  :  /
 * BEHAVIOR     :  Build the LDAP window.
 * PRE          :  /
 */
void gnomemeeting_init_ldap_window ();

/* DESCRIPTION  :  / 
 * BEHAVIOR     :  Build the notebook inside the LDAP window if the server
 *                 name was not already present. Returns its page number
 *                 if it was already present.
 * PRE          :  The server name, the type of page to create 
 *                 (CONTACTS_SERVERS / CONTACTS_GROUPS)
 */
int gnomemeeting_init_ldap_window_notebook (gchar *, int);

void
gnomemeeting_addressbook_group_populate (GtkListStore *list_store,
					 char *group_name);

#endif
