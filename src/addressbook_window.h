
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2004 Damien Sandras
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
 *                         addressbook_window.h  -  description
 *                         ------------------------------------
 *   begin                : Wed Feb 28 2001
 *   copyright            : (C) 2000-2004 by Damien Sandras 
 *   description          : This file contains functions to build the 
 *                          addressbook window.
 *
 */


#ifndef _ADDRESSBOOK_WINDOW_H_
#define _ADDRESSBOOK_WINDOW_H_

#include "common.h"
#include "contacts/gm_contacts.h"


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
 *                addressbook The last argument is the parent window, if any.
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
 * PRE          : The given GtkWidget pointer must point to the address book
 * 		  GMObject. The GmAddressbook pointer must be non-NULL. The
 * 		  last parameter is the parent window.
 */
void gm_addressbook_window_delete_addressbook_dialog_run (GtkWidget *,
							  GmAddressbook *,
							  GtkWidget *);
#endif
