
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2003 Damien Sandras
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
 *
 *
 * GnomeMeting is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OpenH323 and Pwlib, and distribute the combination, without
 * applying the requirements of the GNU GPL to the OpenH323 program, as long
 * as you do follow the requirements of the GNU GPL for all the rest of the
 * software thus combined.
 */


/*
 *                         ldap_window.h  -  description
 *                         -----------------------------
 *   begin                : Wed Feb 28 2001
 *   copyright            : (C) 2000-2003 by Damien Sandras 
 *   description          : This file contains functions to build the 
 *                          addressbook window.
 *
 */


#ifndef _LDAP_WINDOW_H_
#define _LDAP_WINDOW_H_

#include "common.h"
#include "urlhandler.h"


/* The different cell renderers for the ILS browser */
enum {

  COLUMN_ILS_STATUS,
  COLUMN_ILS_AUDIO,
  COLUMN_ILS_VIDEO,
  COLUMN_ILS_NAME,
  COLUMN_ILS_COMMENT,
  COLUMN_ILS_LOCATION,
  COLUMN_ILS_URL,
  COLUMN_ILS_VERSION,
  COLUMN_ILS_IP,
  COLUMN_ILS_COLOR,
  NUM_COLUMNS_SERVERS
};

 
/* The different cell renderers for the local contacts */
enum {

  COLUMN_NAME,
  COLUMN_URL,
  COLUMN_SPEED_DIAL,
  NUM_COLUMNS_GROUPS
};


/* The different cell renderers for the different contacts sections (servers
   or groups */
enum {

  COLUMN_PIXBUF,
  COLUMN_CONTACT_SECTION_NAME,
  COLUMN_NOTEBOOK_PAGE,
  COLUMN_PIXBUF_VISIBLE,
  NUM_COLUMNS_CONTACTS
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


/* DESCRIPTION  :  / 
 * BEHAVIOR     :  Fills in the GtkListStore with the members of the group
 *                 given as second parameter.
 * PRE          :  /
 */
void gnomemeeting_addressbook_group_populate (GtkListStore *, char *);


/* DESCRIPTION  :  / 
 * BEHAVIOR     :  Fills in the arborescence of servers and groups.
 * PRE          :  /
 */
void gnomemeeting_addressbook_sections_populate ();


/* DESCRIPTION  :  / 
 * BEHAVIOR     :  Returns the GMURL for the given speed dial or NULL if none.
 * PRE          :  /
 */
GMURL gnomemeeting_addressbook_get_url_from_speed_dial (const char *);
#endif
