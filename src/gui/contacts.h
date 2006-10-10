
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

#define GM_CONTACTS_ROSTER_GROUP _("Roster")
#define GM_CONTACTS_UNKNOWN_GROUP _("Unknown")

#include "common.h"
#include "gmcontacts.h"


/*!\file contacts.h
 * \brief common generic UI to manipulate contacts
 */

typedef enum {
  GM_CONTACT_MENU_FLAGS
} GmContactContextMenuFlags;


typedef struct GmContactsUIDataCarrier_ {
  GmContact *contact;
  GmAddressbook *abook;
  GtkWindow *parent_window;
} GmContactsUIDataCarrier;


/*!\fn gm_contacts_contextmenu_new (GmContact *contact, GtkWindow *parent_window)
 * \brief Returns a GtkMenu widget for the given contact
 *
 * \param contact the contact for which this menu is created for, can be #NULL
 * \param parent_window the parent window for all subsequent dialogs
 */
GtkWidget *gm_contacts_contextmenu_new (GmContact *, 
                                        GmContactContextMenuFlags, 
                                        GtkWindow *);


/*!\fn gm_contacts_dialog_new_contact (GmContact *given_contact, GmAddressbook *given_abook, GtkWindow *parent_window)
 * \brief Runs a dialog to edit a given contact
 *
 * The dialog displays the same mask as if one wants to edit an existing
 * contact. The parameter given_contact is used to fill in "template"
 * values, e.g. for a drag/drop operation from a non-local addressbook,
 * into the dialog fields. If given_abook is provided, the dialog pre-selects
 * this addressbook in the addressbook-list, if it's a valid local addressbook.
 * If the parameter category is given, then the contact will automatically be
 * added to that category as well as to the chosen category.
 * If the parameter parent_window is given, this window is used as parent for
 * the dialog, if it's NULL, the Ekiga main window is determinated and used.
 *
 * \param given_contact a "template" contact, to fill in the fields, or NULL
 * \param given_abook the addressbook that initially occours in the dialog, or NULL
 * \param parent_window the parent window for the dialog, can be NULL
 */
void gm_contacts_dialog_new_contact (GmContact *given_contact,
				     GmAddressbook *given_abook,
				     GtkWindow *parent_window);


/*!\fn gm_contacts_dialog_edit_contact (GmContact *contact, GtkWindow *parent_window)
 * \brief Runs a dialog to edit a given contact
 *
 * This dialog allows to edit a contact's properties. The addressbook the
 * contact belongs to is searched by the contact's UID from all local
 * addressbooks. If the parameter parent_window is given, this window is
 * used as parent for the dialog, if it's NULL, the Ekiga main window is
 * determinated and used.
 *
 * \param contact the contact to edit, can't be #NULL
 * \param parent_window the parent window for the dialog, can be NULL
 */
void gm_contacts_dialog_edit_contact (GmContact *contact,
				      GtkWindow *parent_window);


/*!\fn gm_contacts_dialog_delete_contact (GmContact *contact, GtkWindow *parent_window)
 * \brief deletes the given contact
 *
 * This dialog asks the user to confirm deletion of the contact. The
 * addressbook the contact belongs to is searched by the contact's UID from
 * all local addressbooks. If the parameter parent_window is given, this
 * window is used as parent for the dialog, if it's NULL, the Ekiga main
 * window is determinated and used.
 *
 * \param contact the contact to delete, can't be #NULL
 * \param parent_window the parent window for the dialog, can be NULL
 */
void gm_contacts_dialog_delete_contact (GmContact *contact,
					GtkWindow *parent_window);

/* Reusable callbacks */
/* Those callbacks can be shared by several parts of the user interface.
 * Each callback constitutes of several actions (if applicable) :
 *     - present a front-end dialog to the user
 *     - update the contacts back-end with the information entered by the user
 *     - execute the appropriate action on the SIP/H.323/... endpoint or other
 */

/*!\fn gm_contacts_datacarrier_new (GmContact *contact, GmAddressbook *abook, GtkWindow *parent_window)
 * \brief Returns the object associate a contact, his address book and the
 * parent window of the dialog editing object.
 *
 * \param contact the contact, can be #NULL
 * \param abook his address book, can be #NULL
 * \param parent_window the parent window, can be #NULL
 */
GmContactsUIDataCarrier *gm_contacts_datacarrier_new (GmContact *contact,
                                                      GmAddressbook *abook,
                                                      GtkWindow *parent_window);


/*!\fn gm_contacts_datacarrier_delete (GmContactsUIDataCarrier *carrier)
 * \brief Frees the memory associated to the data carrier.
 *
 * \param carrier, a pointer to the GmContactsUIDataCarrier, can not be #NULL
 */
void gm_contacts_datacarrier_delete (GmContactsUIDataCarrier *carrier);


/*!\fn gm_contacts_call_contact_cb (GtkWidget *widget, gpointer data)
 * \brief Call the contact given as argument in the GmContactsUIDataCarrier.
 *
 * \param data, a pointer to the GmContactsUIDataCarrier, can not be #NULL. The associated memory is freed after the callback has been executed.
 */
void gm_contacts_call_contact_cb (GtkWidget *, gpointer);


/*!\fn gm_contacts_copy_contact_to_clipboard_cb (GtkWidget *widget, gpointer data)
 * \brief Copy the address of the contact given as argument in the GmContactsUIDataCarrier to the clipboard.
 *
 * \param data, a pointer to the GmContactsUIDataCarrier, can not be #NULL. The associated memory is freed after the callback has been executed.
 */
void gm_contacts_copy_contact_to_clipboard_cb (GtkWidget *, gpointer);


/*!\fn gm_contacts_email_contact_cb (GtkWidget *widget, gpointer data)
 * \brief E-mail the contact given as argument in the GmContactsUIDataCarrier.
 *
 * \param data, a pointer to the GmContactsUIDataCarrier, can not be #NULL. The associated memory is freed after the callback has been executed.
 */
void gm_contacts_email_contact_cb (GtkWidget *, gpointer);


/*!\fn gm_contacts_add_contact_to_addressbook_cb (GtkWidget *widget, gpointer data)
 * \brief Add the contact given as argument in the GmContactsUIDataCarrier to the address book.
 *
 * \param data, a pointer to the GmContactsUIDataCarrier, can not be #NULL. The associated memory is freed after the callback has been executed.
 */
void gm_contacts_add_contact_to_addressbook_cb (GtkWidget *, gpointer);


/*!\fn gm_contacts_message_contact_cb (GtkWidget *widget, gpointer data)
 * \brief Send the message to the contact given as argument in the GmContactsUIDataCarrier.
 *
 * \param data, a pointer to the GmContactsUIDataCarrier, can not be #NULL. The associated memory is freed after the callback has been executed.
 */
void gm_contacts_message_contact_cb (GtkWidget *, gpointer);


/*!\fn gm_contacts_edit_contact_cb (GtkWidget *widget, gpointer data)
 * \brief Edit the properties of the contact given as argument in the GmContactsUIDataCarrier.
 *
 * \param data, a pointer to the GmContactsUIDataCarrier, can not be #NULL. The associated memory is freed after the callback has been executed.
 */
void gm_contacts_edit_contact_cb (GtkWidget *, gpointer);


/*!\fn gm_contacts_call_contact_cb (GtkWidget *widget, gpointer data)
 * \brief Delete the contact given as argument in the GmContactsUIDataCarrier.
 *
 * \param data, a pointer to the GmContactsUIDataCarrier, can not be #NULL. The associated memory is freed after the callback has been executed.
 */
void gm_contacts_delete_contact_cb (GtkWidget *, gpointer);


/*!\fn gm_contacts_call_contact_cb (GtkWidget *widget, gpointer data)
 * \brief Add a new contact to the system.
 *
 * \param data, a pointer to the GmContactsUIDataCarrier, can not be #NULL. The associated memory is freed after the callback has been executed.
 */
void gm_contacts_add_new_contact_cb (GtkWidget *, gpointer);

#endif /* __CONTACTS_H__ */

