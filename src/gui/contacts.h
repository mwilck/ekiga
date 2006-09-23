
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
 *                         contacts.h  -  description
 *                         ------------------------------------
 *   begin                : Fri Sep 22 2006
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *                          (C) 2006      by Jan Schampera
 *   description          : GUI (dialogs, ...) for contacts
 *
 */

#ifndef __CONTACTS_H__
#define __CONTACTS_H__

#include "common.h"
#include "gmcontacts.h"


/* common menu widgets */
/*!fn gm_contacts_contextmenu_new
 * \brief Returns a GtkMenu widget proper for the given contact (SIP, ...)
 *
 * \param contact
 */
GtkWidget *gm_contacts_contextmenu_new (GmContact *, GtkWindow *);


/*!\fn gm_contacts_dialog_edit_contact
 * \brief Runs a dialog to edit a given contact
 *
 * This dialog allows to edit a contact's properties. The addressbook the
 * contact belongs to is searched by the contact's UID from all local
 * addressbooks.
 *
 * \param parent_window
 */
void gm_contacts_dialog_new_contact (GtkWindow *);


/*!\fn gm_contacts_dialog_edit_contact
 * \brief Runs a dialog to edit a given contact
 *
 * This dialog allows to edit a contact's properties. The addressbook the
 * contact belongs to is searched by the contact's UID from all local
 * addressbooks.
 *
 * \param contact
 * \param parent_window
 */
void gm_contacts_dialog_edit_contact (GmContact *,
				      GtkWindow *);


/*!\fn gm_contacts_dialog_delete_contact
 * \brief deletes the given contact
 *
 * This dialog asks the user to confirm deletion of the contact. The
 * addressbook the contact belongs to is searched by the contact's UID from
 * all local addressbooks.
 *
 * \param contact
 * \param parent_window
 */
void gm_contacts_dialog_delete_contact (GmContact *,
					GtkWindow *);

#endif /* __CONTACTS_H__ */

