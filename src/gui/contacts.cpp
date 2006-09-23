
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

#include "contacts.h"
#include "../../config.h"

#include "addressbook.h"
#include "main.h"
#include "chat.h"
#include "callbacks.h"
#include "ekiga.h"
#include "urlhandler.h"
#include "misc.h"
#include "statusicon.h"

#include "gmstockicons.h"
#include "gmcontacts.h"
#include "gmdialog.h"
#include "gmconf.h"
#include "gmmenuaddon.h"

#include "toolbox/toolbox.h"


typedef struct GmContactsUIDataCarrier_ {
  GmContact *contact;
  GtkWindow *parent_window;
} GmContactsUIDataCarrier;


/* some prototyping */


static void gm_contacts_update_components (GmAddressbook *);

static void gm_contacts_editing_dialog (GmContact *, gboolean, GtkWindow *);

static gboolean gm_contacts_check_collission (GmContact *, GmContact *, GtkWindow *);


/* the callbacks for the context menu */
static void gm_contacts_cb_menu_call (GtkWidget *, gpointer);
static void gm_contacts_cb_menu_copy_clipbrd (GtkWidget *, gpointer);
static void gm_contacts_cb_menu_email (GtkWidget *, gpointer);
static void gm_contacts_cb_menu_add_to_local (GtkWidget *, gpointer);
static void gm_contacts_cb_menu_message (GtkWidget *, gpointer);
static void gm_contacts_cb_menu_properties (GtkWidget *, gpointer);
static void gm_contacts_cb_menu_delete (GtkWidget *, gpointer);
static void gm_contacts_cb_menu_add_new (GtkWidget *, gpointer);


/* implementation follows */

static void
gm_contacts_update_components (GmAddressbook *addressbook)
{
  GtkWidget *main_window = NULL;
  GtkWidget *addressbook_window = NULL;
  GtkWidget *chat_window = NULL;

  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  addressbook_window = GnomeMeeting::Process ()->GetAddressbookWindow ();
  chat_window = GnomeMeeting::Process ()->GetChatWindow ();

  g_return_if_fail (addressbook != NULL);

  /* PLAN: */
  /* Update: This addressbook in addressbook window */
  /* Update: All speeddials in main window */
  /* Update: The roster view in main window */
  /* Update: URI history */

}


static void
gm_contacts_editing_dialog (GmContact *contact,
			    gboolean edit_existing_contact,
			    GtkWindow *parent_window)
{
  GtkWidget *dialog = NULL;

  GtkWidget *fullname_entry = NULL;
  GtkWidget *url_entry = NULL;
  GtkWidget *email_entry = NULL;
  GtkWidget *categories_entry = NULL;
  GtkWidget *speeddial_entry = NULL;
  GtkWidget *table = NULL;
  GtkWidget *label = NULL;

  GtkWidget *option_menu = NULL;

  GmAddressbook *addb = NULL;
  GmAddressbook *addc = NULL;
  GmAddressbook *new_addressbook = NULL;
  GmAddressbook *addressbook = NULL;

  GmContact *new_contact = NULL;

  GSList *list = NULL;
  GSList *l = NULL;

  gchar *label_text = NULL;
  gint result = 0;
  gint current_menu_index = 0;
  gint pos = 0;

  gboolean collision = TRUE;
  gboolean valid = FALSE;


  /* if we're editing an existing contact, get the proper addressbook by
   * its UID */
  if (edit_existing_contact)
    addressbook = gnomemeeting_local_addressbook_get_by_contact (contact);

  /* Create the dialog to easily modify the info
   * of a specific contact */
  dialog =
    gtk_dialog_new_with_buttons (edit_existing_contact
				 ?_("Edit the Contact Information")
				 :_("New contact"),
                                 parent_window,
                                 GTK_DIALOG_MODAL,
                                 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                 GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
                                 NULL);
  gtk_dialog_set_default_response (GTK_DIALOG (dialog),
                                   GTK_RESPONSE_ACCEPT);

  table = gtk_table_new (6, 2, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 3 * GNOMEMEETING_PAD_SMALL);
  gtk_table_set_col_spacings (GTK_TABLE (table), 3 * GNOMEMEETING_PAD_SMALL);

  /* Get the list of addressbooks */
  list = gnomemeeting_get_local_addressbooks ();


  /* The Full Name entry */
  label = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  label_text = g_strdup_printf ("<b>%s</b>", _("Name:"));
  gtk_label_set_markup (GTK_LABEL (label), label_text);
  g_free (label_text);

  fullname_entry = gtk_entry_new ();
  if (contact && contact->fullname)
    gtk_entry_set_text (GTK_ENTRY (fullname_entry), contact->fullname);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL),
                    3 * GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_table_attach (GTK_TABLE (table), fullname_entry, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL),
                    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_entry_set_activates_default (GTK_ENTRY (fullname_entry), TRUE);


  /* The URL entry */
  label = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  label_text = g_strdup_printf ("<b>%s</b>", _("VoIP URL:"));
  gtk_label_set_markup (GTK_LABEL (label), label_text);
  g_free (label_text);

  url_entry = gtk_entry_new ();
  if (contact && contact->url)
    gtk_entry_set_text (GTK_ENTRY (url_entry), contact->url);
  else
    gtk_entry_set_text (GTK_ENTRY (url_entry), GMURL ().GetDefaultURL ());
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL),
                    3 * GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_table_attach (GTK_TABLE (table), url_entry, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL),
                    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_entry_set_activates_default (GTK_ENTRY (url_entry), TRUE);

  /* The email entry */
  label = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  label_text = g_strdup_printf ("<b>%s</b>", _("Email:"));
  gtk_label_set_markup (GTK_LABEL (label), label_text);
  g_free (label_text);

  email_entry = gtk_entry_new ();
  if (contact && contact->email)
    gtk_entry_set_text (GTK_ENTRY (email_entry), contact->email);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL),
                    3 * GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_table_attach (GTK_TABLE (table), email_entry, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL),
                    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_entry_set_activates_default (GTK_ENTRY (email_entry), TRUE);


  /* The Speed Dial */
  label = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  label_text = g_strdup_printf ("<b>%s</b>", _("Speed Dial:"));
  gtk_label_set_markup (GTK_LABEL (label), label_text);
  g_free (label_text);

  speeddial_entry = gtk_entry_new ();
  if (contact && contact->speeddial)
    gtk_entry_set_text (GTK_ENTRY (speeddial_entry),
                        contact->speeddial);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL),
                    3 * GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_table_attach (GTK_TABLE (table), speeddial_entry,
                    1, 2, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL),
                    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_entry_set_activates_default (GTK_ENTRY (speeddial_entry), TRUE);


  /* The Categories */
  label = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  label_text = g_strdup_printf ("<b>%s</b>", _("Categories:"));
  gtk_label_set_markup (GTK_LABEL (label), label_text);
  g_free (label_text);

  categories_entry = gtk_entry_new ();
  if (contact && contact->categories)
    gtk_entry_set_text (GTK_ENTRY (categories_entry),
                        contact->categories);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 4, 5,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL),
                    3 * GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_table_attach (GTK_TABLE (table), categories_entry,
                    1, 2, 4, 5,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL),
                    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_entry_set_activates_default (GTK_ENTRY (categories_entry), TRUE);


  /* The different local addressbooks are not displayed when
   * we are editing a contact from a local addressbook */
  if (!edit_existing_contact) {

    label = gtk_label_new (NULL);
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
    label_text = g_strdup_printf ("<b>%s</b>", _("Local Addressbook:"));
    gtk_label_set_markup (GTK_LABEL (label), label_text);
    g_free (label_text);

    option_menu = gtk_combo_box_new_text ();

    l = list;
    pos = 0;
    while (l) {

      addb = GM_ADDRESSBOOK (l->data);
      if (addressbook && addb
          && addb->name && addressbook->name
          && !strcmp (addb->name, addressbook->name))
        current_menu_index = pos;

      gtk_combo_box_append_text (GTK_COMBO_BOX (option_menu), addb->name);

      l = g_slist_next (l);
      pos++;
    }

    gtk_combo_box_set_active (GTK_COMBO_BOX (option_menu), current_menu_index);

    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 5, 6,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (GTK_FILL),
                      3 * GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
    gtk_table_attach (GTK_TABLE (table), option_menu,
                      1, 2, 5, 6,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (GTK_FILL),
                      GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  }


  /* Pack the gtk entries and the list store in the window */
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), table,
                      FALSE, FALSE, 3 * GNOMEMEETING_PAD_SMALL);
  gtk_widget_show_all (dialog);


  /* Now run the dialog */

  /* The result can be invalid if both a full name and url are missing,
   * or later if a collision is detected. The dialog will be run during
   * that time.
   */
  while (!valid)  {

    result = gtk_dialog_run (GTK_DIALOG (dialog));

    valid = strcmp (gtk_entry_get_text (GTK_ENTRY (fullname_entry)), "")
      || strcmp (gtk_entry_get_text (GTK_ENTRY (url_entry)), "");


    switch (result) {

      case GTK_RESPONSE_ACCEPT:

      if (valid) {

        new_contact = gmcontact_new ();
        new_contact->fullname =
          g_strdup (gtk_entry_get_text (GTK_ENTRY (fullname_entry)));
        new_contact->speeddial =
          g_strdup (gtk_entry_get_text (GTK_ENTRY (speeddial_entry)));
        new_contact->categories =
          g_strdup (gtk_entry_get_text (GTK_ENTRY (categories_entry)));
        new_contact->url =
          g_strdup (gtk_entry_get_text (GTK_ENTRY (url_entry)));
        new_contact->email =
          g_strdup (gtk_entry_get_text (GTK_ENTRY (email_entry)));

        /* We were editing an existing contact */
        if (edit_existing_contact) {

          /* We keep the old UID */
          new_contact->uid = g_strdup (contact->uid);
          new_addressbook = addressbook;
        }
        else {

          /* Forget the selected addressbook and use the dialog one instead
           * if the user could choose it in the dialog */
          current_menu_index = gtk_combo_box_get_active (GTK_COMBO_BOX (option_menu));

          addc = GM_ADDRESSBOOK (g_slist_nth_data (list, current_menu_index));

          if (addc)
            new_addressbook = addc;
          else {

            new_addressbook = gm_addressbook_new ();
            new_addressbook->name = _("Personal");
            (void)gnomemeeting_addressbook_add (new_addressbook);
	    /* FIXME gm_addressbook_window_update() */
          }
        }

        /* We are editing an existing contact, compare with the old values */
        if (edit_existing_contact)
	  collision = gm_contacts_check_collission (new_contact,
						    contact,
						    parent_window);

        else /* We are adding a new contact */
	  collision = gm_contacts_check_collission (new_contact,
						    NULL,
						    parent_window);

        if (!collision) {

          if (edit_existing_contact)
            gnomemeeting_addressbook_modify_contact (new_addressbook,
                                                     new_contact);
          else
            gnomemeeting_addressbook_add_contact (new_addressbook, new_contact);

          gm_contacts_update_components (new_addressbook);
        }

        gmcontact_delete (new_contact);
      }
      else {
        gnomemeeting_error_dialog (parent_window,
				   _("Missing information"),
				   _("Please make sure to provide at least a full name or an URL for the contact."));
      }

      break;

    case GTK_RESPONSE_DELETE_EVENT:
    case GTK_RESPONSE_CANCEL:

      collision = FALSE;
      valid = TRUE;
      break;
    }

    if (collision)
      valid = FALSE;
  }

  gtk_widget_destroy (dialog);

  g_slist_foreach (list, (GFunc) gm_addressbook_delete, NULL);
  g_slist_free (list);
}


static gboolean
gm_contacts_check_collission (GmContact *new_contact,
			      GmContact *old_contact,
			      GtkWindow *parent_window)
{
  GSList *contacts = NULL;

  GmContact *ctct = NULL;

  GtkWidget *dialog = NULL;

  gchar *dialog_text = NULL;
  gchar *primary_text = NULL;
  gchar *secondary_text = NULL;

  int cpt = 0;
  int nbr = 0;

  gboolean to_return = FALSE;
  gboolean check_fullname = FALSE;
  gboolean check_url = FALSE;
  gboolean check_speeddial = FALSE;

  g_return_val_if_fail (new_contact != NULL, TRUE);


  /* Check the full name if we are adding a contact or if we are editing
   * a contact and added a full name, or changed the full name
   */
  if (new_contact->fullname && strcmp (new_contact->fullname, ""))
    check_fullname = (!old_contact
                      || (new_contact->fullname && !old_contact->fullname)
                      || (old_contact->fullname && new_contact->fullname
                          && strcmp (old_contact->fullname,
                                     new_contact->fullname)));

  /* Check the full url if we are adding a contact or if we are editing
   * a contact and added an url, or changed the url
   */
  if (new_contact->url && strcmp (new_contact->url, ""))
    check_url = (!old_contact
                 || (new_contact->url && !old_contact->url)
                 || (old_contact->url && new_contact->url
                     && strcmp (old_contact->url,
                                new_contact->url)));

  /* Check the full url if we are adding a contact or if we are editing
   * a contact and added an url, or changed the url
   */
  if (new_contact->url && strcmp (new_contact->url, ""))
    check_url = (!old_contact
                 || (new_contact->url && !old_contact->url)
                 || (old_contact->url && new_contact->url
                     && strcmp (old_contact->url,
                                new_contact->url)));

  /* Check the speed dial if we are adding a contact or if we are editing
   * a contact and added a speed dial, or changed the speed dial
   */
  if (new_contact->speeddial && strcmp (new_contact->speeddial, ""))
    check_speeddial = (!old_contact
                       || (new_contact->speeddial && !old_contact->speeddial)
                       || (old_contact->speeddial && new_contact->speeddial
                           && strcmp (old_contact->speeddial,
                                      new_contact->speeddial)));

  /* First do a search on the fields, then on the speed dials. Not clean,
   * but E-D-S doesn't permit to do better for now...
   */
  while (cpt < 2) {

    if (cpt == 0) {

      /* Is there any user with the same speed dial ? */
      if (check_speeddial)
        contacts =
          gnomemeeting_addressbook_get_contacts (NULL,
                                                 nbr,
                                                 FALSE,
                                                 NULL,
                                                 NULL,
                                                 NULL,
                                                 NULL,
                                                 check_speeddial ?
                                                 new_contact->speeddial:
                                                 NULL);
    }
    else if (check_fullname || check_url)
      contacts =
        gnomemeeting_addressbook_get_contacts (NULL,
                                               nbr,
                                               FALSE,
                                               check_fullname ?
                                               new_contact->fullname:
                                               NULL,
                                               check_url?
                                               new_contact->url:
                                               NULL,
                                               NULL,
                                               NULL,
                                               NULL);

    if (contacts && contacts->data) {

      ctct = GM_CONTACT (contacts->data);

      primary_text = g_strdup_printf ("<span weight=\"bold\" size=\"larger\">%s</span>", _("Contact collision"));
      if (cpt == 0)
        secondary_text = g_strdup_printf (_("Another contact with the same speed dial already exists in your address book:\n\n<b>Name</b>: %s\n<b>URL</b>: %s\n<b>Speed Dial</b>: %s\n"), ctct->fullname?ctct->fullname:_("None"), ctct->url?ctct->url:_("None"), ctct->speeddial?ctct->speeddial:_("None"));
      else
        secondary_text = g_strdup_printf (_("Another contact with similar information already exists in your address book:\n\n<b>Name</b>: %s\n<b>URL</b>: %s\n<b>Speed Dial</b>: %s\n\nDo you still want to add the contact?"), ctct->fullname?ctct->fullname:_("None"), ctct->url?ctct->url:_("None"), ctct->speeddial?ctct->speeddial:_("None"));


      dialog_text =
        g_strdup_printf ("%s\n\n%s", primary_text, secondary_text);

      dialog =
        gtk_message_dialog_new (parent_window,
                                GTK_DIALOG_MODAL,
                                (cpt == 0) ?
                                GTK_MESSAGE_ERROR
                                :
                                GTK_MESSAGE_WARNING,
                                (cpt == 0) ?
                                GTK_BUTTONS_OK
                                :
                                GTK_BUTTONS_YES_NO,
                                NULL);

      gtk_window_set_title (GTK_WINDOW (dialog), "");
      gtk_label_set_markup (GTK_LABEL (GTK_MESSAGE_DIALOG (dialog)->label),
                            dialog_text);

      switch (gtk_dialog_run (GTK_DIALOG (dialog)))
        {
        case GTK_RESPONSE_YES:
          to_return = FALSE;
          break;

        default:
          to_return = TRUE;

        }

      gtk_widget_destroy (dialog);

      g_slist_foreach (contacts, (GFunc) gmcontact_delete, NULL);
      g_slist_free (contacts);

      g_free (primary_text);
      g_free (secondary_text);
      g_free (dialog_text);

      break;
    }

    cpt++;
  }

  return to_return;
}


void
gm_contacts_dialog_new_contact (GtkWindow * parent_window)
{
  if (parent_window)
    g_return_if_fail (GTK_IS_WINDOW (parent_window));

  gm_contacts_editing_dialog (NULL, FALSE, parent_window);
}

void
gm_contacts_dialog_edit_contact (GmContact * contact,
				 GtkWindow * parent_window)
{
  if (parent_window)
    g_return_if_fail (GTK_IS_WINDOW (parent_window));

  gm_contacts_editing_dialog (contact, TRUE, parent_window);
}


void
gm_contacts_dialog_delete_contact (GmContact * contact,
				   GtkWindow * parent_window)
{
  if (parent_window)
    g_return_if_fail (GTK_IS_WINDOW (parent_window));
}


GtkWidget *
gm_contacts_contextmenu_new (GmContact *contact, GtkWindow *parent_window)
{
  GtkWidget *menu = NULL;

  GmAddressbook *addressbook = NULL;

  GmContactsUIDataCarrier *data_carrier = NULL;

  gboolean local = TRUE;
  gboolean is_sip = FALSE;
  gboolean has_email = TRUE;

  data_carrier = g_new (GmContactsUIDataCarrier, 1);
  data_carrier->contact = contact;
  data_carrier->parent_window = parent_window;

  addressbook = gnomemeeting_local_addressbook_get_by_contact (contact);

  if (!addressbook || !gnomemeeting_addressbook_is_local (addressbook))
    local = FALSE;

  if (contact)
    is_sip = (GMURL (contact->url).GetType () == "sip");

  /* mi_ variables: (m)enu(i)tem, mi_sm_ indicates a (s)ub(m)enu,
   * implemented as array */
  static MenuEntry mi_call_contact =
    /* call a contact, usage: general */
    GTK_MENU_ENTRY("call", _("C_all Contact"), NULL,
                   GM_STOCK_CONNECT_16, 0,
                   GTK_SIGNAL_FUNC ( gm_contacts_cb_menu_call),
                   data_carrier, TRUE);

  static MenuEntry mi_copy_url =
    /* copy a contact's URL to clipboard, usage: general */
    GTK_MENU_ENTRY("copy", _("_Copy URL to Clipboard"), NULL,
                   GTK_STOCK_COPY, 0,
                   GTK_SIGNAL_FUNC ( gm_contacts_cb_menu_copy_clipbrd),
                   data_carrier, TRUE);

  static MenuEntry mi_email =
    GTK_MENU_ENTRY("emailwrite", _("_Write e-Mail"), NULL,
                   GM_STOCK_EDIT, 0,
                   GTK_SIGNAL_FUNC ( gm_contacts_cb_menu_email),
                   data_carrier, has_email);

  static MenuEntry mi_add_to_local =
    /* add a contact to the local addressbook, usage: remote contacts only */
    GTK_MENU_ENTRY("add", _("Add Contact to _Address Book"), NULL,
                   GTK_STOCK_ADD, 0,
                   GTK_SIGNAL_FUNC (gm_contacts_cb_menu_add_to_local),
                   data_carrier, TRUE);

  static MenuEntry mi_send_message =
    /* send a contact a (SIP!) message, usage: SIP contacts only */
    GTK_MENU_ENTRY("message", _("_Send Message"), NULL,
                   GM_STOCK_MESSAGE, 0,
                   GTK_SIGNAL_FUNC ( gm_contacts_cb_menu_message),
                   data_carrier, TRUE);


  static MenuEntry mi_edit_properties =
    /* edit a local contact's addressbook entry, usage: local contacts */
    GTK_MENU_ENTRY("properties", _("_Properties"), NULL,
                   GTK_STOCK_PROPERTIES, 0,
                   GTK_SIGNAL_FUNC ( gm_contacts_cb_menu_properties),
                   data_carrier, TRUE);

  static MenuEntry mi_delete_local =
    /* delete a local contact entry, usage: local contacts */
    GTK_MENU_ENTRY("delete", _("_Delete"), NULL,
                   GTK_STOCK_DELETE, 'd',
                   GTK_SIGNAL_FUNC ( gm_contacts_cb_menu_delete),
                   data_carrier, TRUE);

  static MenuEntry mi_new_contact =
    /* "new contact" dialog, usage: local context */
    GTK_MENU_ENTRY("add", _("New _Contact"), NULL,
                   GTK_STOCK_NEW, 0,
                   GTK_SIGNAL_FUNC ( gm_contacts_cb_menu_add_new),
                   data_carrier, TRUE);

  static MenuEntry add_contact_menu_local [] =
    {
      mi_new_contact,

      GTK_MENU_END
    };

  static MenuEntry contact_menu_local [] =
    {
      mi_call_contact,

      mi_copy_url,

      mi_email,

      GTK_MENU_SEPARATOR,

      mi_edit_properties,

      GTK_MENU_SEPARATOR,

      mi_delete_local,

      GTK_MENU_SEPARATOR,

      mi_new_contact,

      GTK_MENU_END
    };


  static MenuEntry contact_menu_sip_local [] =
    {
      mi_call_contact,

      mi_send_message,

      mi_copy_url,

      mi_email,

      GTK_MENU_SEPARATOR,

      mi_edit_properties,

      GTK_MENU_SEPARATOR,

      mi_delete_local,

      GTK_MENU_SEPARATOR,

      mi_new_contact,

      GTK_MENU_END
    };


  static MenuEntry contact_menu_not_local [] =
    {
      mi_call_contact,

      mi_copy_url,

      mi_email,

      GTK_MENU_SEPARATOR,

      mi_add_to_local,

      GTK_MENU_END
    };


  static MenuEntry contact_menu_sip_not_local [] =
    {
      mi_call_contact,

      mi_send_message,

      mi_copy_url,

      mi_email,

      GTK_MENU_SEPARATOR,

      mi_add_to_local,

      GTK_MENU_END
    };

  if (contact && addressbook) {

    menu = gtk_menu_new ();
    if (local)
      gtk_build_menu (menu, is_sip?contact_menu_sip_local:contact_menu_local, NULL, NULL);
    else
      gtk_build_menu (menu, is_sip?contact_menu_sip_not_local:contact_menu_not_local, NULL, NULL);
  }
  else if (local) {

    menu = gtk_menu_new ();
    gtk_build_menu (menu, add_contact_menu_local, NULL, NULL);
  }

  return menu;
}


/* the callbacks for the context menu */
static void
gm_contacts_cb_menu_call (GtkWidget *unused,
			  gpointer data)
{
  GmContactsUIDataCarrier *data_carrier = NULL;
  GMManager *ep = NULL;
  GtkWidget *main_window = NULL;

  g_return_if_fail (data != NULL);

  ep = GnomeMeeting::Process ()->GetManager ();
  main_window = GnomeMeeting::Process ()->GetMainWindow ();

  data_carrier = (GmContactsUIDataCarrier*) data;

  if (ep->GetCallingState () != GMManager::Standby)
    return;

  if (data_carrier->contact && data_carrier->contact->url) {
    /* present the main window and call */
    gtk_window_present (GTK_WINDOW (main_window));
    GnomeMeeting::Process ()->Connect (data_carrier->contact->url);
  }
}


static void
gm_contacts_cb_menu_copy_clipbrd (GtkWidget *unused,
				  gpointer data)
{
  GmContactsUIDataCarrier *data_carrier = NULL;
  GtkClipboard *cb = NULL;

  g_return_if_fail (data != NULL);

  data_carrier = (GmContactsUIDataCarrier*) data;

  if (data_carrier->contact && data_carrier->contact->url)
    {
      cb = gtk_clipboard_get (GDK_NONE);
      gtk_clipboard_set_text (cb, data_carrier->contact->url, -1);
    }
}


static void
gm_contacts_cb_menu_email (GtkWidget *unused,
			   gpointer data)
{
  GmContactsUIDataCarrier *data_carrier = NULL;
  gchar *email_uri = NULL;

  g_return_if_fail (data != NULL);

  data_carrier = (GmContactsUIDataCarrier*) data;

  if (data_carrier->contact && data_carrier->contact->email)
    {
      email_uri =
	g_strdup_printf ("mailto:%s <%s>",
			 data_carrier->contact->fullname,
			 data_carrier->contact->email);
      gm_open_uri (email_uri);
      g_free (email_uri);
    }
}


static void
gm_contacts_cb_menu_add_to_local (GtkWidget *menu,
				  gpointer data)
{
  GmContactsUIDataCarrier *data_carrier = NULL;

  g_return_if_fail (data != NULL);
  g_return_if_fail (menu != NULL);

  data_carrier = (GmContactsUIDataCarrier*) data;

  gm_contacts_dialog_edit_contact (data_carrier->contact,
				   data_carrier->parent_window);
}


static void
gm_contacts_cb_menu_message (GtkWidget *unused,
			     gpointer data)
{
  GmContactsUIDataCarrier *data_carrier = NULL;
  GtkWidget *chat_window = NULL;
  
  g_return_if_fail (data != NULL);

  chat_window = GnomeMeeting::Process ()->GetChatWindow ();

  data_carrier = (GmContactsUIDataCarrier*) data;
}


static void
gm_contacts_cb_menu_properties (GtkWidget *menu,
				gpointer data)
{
  GmContactsUIDataCarrier *data_carrier = NULL;

  g_return_if_fail (data != NULL);

  data_carrier = (GmContactsUIDataCarrier*) data;

  gm_contacts_dialog_edit_contact (data_carrier->contact,
                                   data_carrier->parent_window);
}


static void
gm_contacts_cb_menu_delete (GtkWidget *unused,
			    gpointer data)
{
  GmContactsUIDataCarrier *data_carrier = NULL;

  g_return_if_fail (data != NULL);

  data_carrier = (GmContactsUIDataCarrier*) data;
}


static void
gm_contacts_cb_menu_add_new (GtkWidget *menu,
			     gpointer data)
{
  GmContactsUIDataCarrier *data_carrier = NULL;

  /* just in case... */
  g_return_if_fail (data == NULL);

  g_return_if_fail (menu != NULL);

  data_carrier = (GmContactsUIDataCarrier*) data;
  
  gm_contacts_dialog_new_contact (data_carrier->parent_window);
}

