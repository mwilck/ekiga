
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2006 Damien Sandras
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
 *                         addressbook_window.h  -  description
 *                         ------------------------------------
 *   begin                : Wed Feb 28 2001
 *   copyright            : (C) 2000-2006 by Damien Sandras 
 *   description          : This file contains functions to build the 
 *                          addressbook window.
 *
 */


#ifndef _ADDRESSBOOK_WINDOW_H_
#define _ADDRESSBOOK_WINDOW_H_

#include "common.h"
#include "gmcontacts.h"


/* DESCRIPTION  : / 
 * BEHAVIOR     : Returns a newly created gnomemeeting address book window
 * 		  GmObject.
 * PRE          : /
 */
GtkWidget *gm_addressbook_window_new ();


/* DESCRIPTION  : / 
 * BEHAVIOR     : Runs a dialog permitting to edit or add a GmContact to the
 * 		  given GmAddressbook based on what is selected in the given
 * 		  address book window GMObject. The boolean should be set to
 *                TRUE in case the given contact is known to exist in the given
 *                addressbook. The last argument is the parent window, if any.
 * 		  Notice that the main window speed dials menu is updated when
 * 		  a contact is modified or added.
 * 		  Updates the urls history of the main and chat windows.
 * PRE          : The given GtkWidget pointer must point to the address book
 * 		  GMObject.
 */
void gm_addressbook_window_edit_contact_dialog_run (GtkWidget *,
						    GmAddressbook *,
						    GmContact *,
						    gboolean,
						    GtkWidget *);


/* DESCRIPTION  : / 
 * BEHAVIOR     : Runs a dialog permitting to delete a GmContact from the
 * 		  given GmAddressbook based on what is selected in the given
 * 		  address book window GMObject. The last arguement is the parent
 * 		  window, if any.
 * 		  Notice that the main window speed dials menu is updated when
 * 		  a contact is deleted.
 * 		  Updates the urls history of the main and chat windows.
 * PRE          : The given GtkWidget pointer must point to the address book
 * 		  GMObject. The GmAddressbook pointer must be non-NULL, the
 * 		  GmContact pointer too.
 */
void gm_addressbook_window_delete_contact_dialog_run (GtkWidget *,
						      GmAddressbook *,
						      GmContact *,
						      GtkWidget *);


/* DESCRIPTION  : / 
 * BEHAVIOR     : Runs a dialog permitting to edit or add a GmAddressbook.
 * PRE          : The given GtkWidget pointer must point to the address book
 * 		  GMObject. The GmAddressbook pointer can be NULL when adding
 * 		  a new address book. The last parameter is the parent window.
 */
void gm_addressbook_window_edit_addressbook_dialog_run (GtkWidget *,
							GmAddressbook *,
							GtkWidget *);



/* DESCRIPTION  : / 
 * BEHAVIOR     : Runs a dialog permitting to delete a GmAddressbook.
 * 		  Notice that the main window speed dials menu is updated when
 * 		  an address book is deleted.
 * 		  Updates the urls history of the main window.
 * PRE          : The given GtkWidget pointer must point to the address book
 * 		  GMObject. The GmAddressbook pointer must be non-NULL. The
 * 		  last parameter is the parent window.
 */
void gm_addressbook_window_delete_addressbook_dialog_run (GtkWidget *,
							  GmAddressbook *,
							  GtkWidget *);
#endif
