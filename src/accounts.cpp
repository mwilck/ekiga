
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
 * GnomeMeeting is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OpenH323 and Pwlib, and distribute the combination, without
 * applying the requirements of the GNU GPL to the OpenH323 program, as long
 * as you do follow the requirements of the GNU GPL for all the rest of the
 * software thus combined.
 */


/*
 *                         accounts.cpp  -  description
 *                         ----------------------------
 *   begin                : Sun Feb 13 2005
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : This file contains all the functions needed to
 *   			    manipulate H.323/SIP/... provider accounts.
 */


#include "../config.h"

#include "accounts.h"
#include "callbacks.h"
#include "misc.h"
#include "main_window.h"
#include "pref_window.h"
#include "log_window.h"

#include "endpoint.h"
#include "sipendpoint.h"
#include "gnomemeeting.h"

#include "gm_conf.h"
#include "dialog.h"

typedef struct GmAccountsEditWindow_ {

  GtkWidget *account_entry;
  GtkWidget *protocol_option_menu;
  GtkWidget *host_label;
  GtkWidget *host_entry;
  GtkWidget *username_entry;
  GtkWidget *password_entry;
  GtkWidget *domain_label;
  GtkWidget *domain_entry;
} GmAccountsEditWindow;

#define GM_ACCOUNTS_EDIT_WINDOW(x) (GmAccountsEditWindow *) (x)

typedef struct GmAccountsWindow_ {

  GtkWidget *accounts_list;

} GmAccountsWindow;

#define GM_ACCOUNTS_WINDOW(x) (GmAccountsWindow *) (x)


/* GUI Functions */

/* DESCRIPTION  : /
 * BEHAVIOR     : Frees a GmAccountsWindow and its content.
 * PRE          : A non-NULL pointer to a GmAccountsWindow structure.
 */
static void gm_aw_destroy (gpointer);


/* DESCRIPTION  : /
 * BEHAVIOR     : Returns a pointer to the private GmAccountsWindow structure
 *                used by the preferences window GMObject.
 * PRE          : The given GtkWidget pointer must be a preferences window 
 * 		  GMObject.
 */
static GmAccountsWindow *gm_aw_get_aw (GtkWidget *);


/* DESCRIPTION  : /
 * BEHAVIOR     : Creates and run a dialog where the user can edit a new
 * 		  or an already selected account. It also checks if all
 * 		  required parameters are present, and once a valid choice
 * 		  has been entered and validated, it is added/modified in
 * 		  the accounts database.
 * PRE          : The GmAccount is a valid pointer to a valid GmAccount
 * 		  that we are editing. If it is NULL, it means we
 * 		  are creating a new account.
 */
static void gm_aw_edit_account_dialog_run (GtkWidget *accounts_window,
					   GmAccount *account,
					   GtkWidget *parent_window);


/* DESCRIPTION  : /
 * BEHAVIOR     : Creates and run a dialog asking to the user if he wants
 * 		  to delete the given account.
 * PRE          : The GmAccount is a valid pointer to a valid GmAccount
 * 		  that we are editing. If can not be NULL.
 */
static void gm_aw_delete_account_dialog_run (GtkWidget *accounts_window,
					     GmAccount *account,
					     GtkWidget *parent_window);


/* DESCRIPTION  : /
 * BEHAVIOR     : Returns the currently selected account in the main window.
 * 		  (if any).
 * PRE          : /
 */
static GmAccount *gm_aw_get_selected_account (GtkWidget *accounts_window);


/* GTK+ Callbacks */

/* DESCRIPTION  :  This callback is called when the user clicks
 *                 on an account to enable it in the accounts window.
 * BEHAVIOR     :  It updates the accounts list configuration to enable/disable
 *                 the account. It also calls the Register operation from
 *                 the GMEndpoint to refresh the status of that account.
 * PRE          :  /
 */
static void account_toggled_cb (GtkCellRendererToggle *cell,
				gchar *path_str,
				gpointer data);


/* DESCRIPTION  :  This callback is called when the user chooses to add
 * 		   an account.
 * BEHAVIOR     :  It runs the edit account dialog until the user validates
 * 		   it with correct data.
 * PRE          :  /
 */
static void add_account_cb (GtkWidget *button, 
			    gpointer data);


/* DESCRIPTION  :  This callback is called when the user chooses to edit
 * 		   an account.
 * BEHAVIOR     :  It runs the edit account dialog until the user validates
 * 		   it with correct data.
 * PRE          :  /
 */
static void edit_account_cb (GtkWidget *button, 
			     gpointer data);


/* DESCRIPTION  :  This callback is called when the user chooses to delete
 * 		   an account.
 * BEHAVIOR     :  It runs the delete account dialog until the user validates
 * 		   it.
 * PRE          :  /
 */
static void delete_account_cb (GtkWidget *button, 
			       gpointer data);


/* DESCRIPTION  :  This callback is called when the user changes the protocol
 * 		   in the account dialog.
 * BEHAVIOR     :  Updates the content and labels.
 * PRE          :  data is a valid pointer to a valid GmAccountWindow.
 */
static void account_dialog_protocol_changed_cb (GtkWidget *menu,
						gpointer data);


/* Columns for the VoIP accounts */
enum {

  COLUMN_ACCOUNT_ENABLED,
  COLUMN_ACCOUNT_AID,
  COLUMN_ACCOUNT_ACCOUNT_NAME,
  COLUMN_ACCOUNT_PROTOCOL_NAME,
  COLUMN_ACCOUNT_HOST,
  COLUMN_ACCOUNT_DOMAIN,
  COLUMN_ACCOUNT_LOGIN,
  COLUMN_ACCOUNT_PASSWORD,
  COLUMN_ACCOUNT_STATE,
  COLUMN_ACCOUNT_TIMEOUT,
  COLUMN_ACCOUNT_METHOD,
  COLUMN_ACCOUNT_ERROR_MESSAGE,
  COLUMN_ACCOUNT_ACTIVATABLE,
  COLUMN_ACCOUNT_NUMBER
};


/* GUI Functions */
static void
gm_aw_destroy (gpointer aw)
{
  g_return_if_fail (aw != NULL);

  delete ((GmAccountsWindow *) aw);
}


static GmAccountsWindow *
gm_aw_get_aw (GtkWidget *accounts_window)
{
  g_return_val_if_fail (accounts_window != NULL, NULL);

  return GM_ACCOUNTS_WINDOW (g_object_get_data (G_OBJECT (accounts_window), "GMObject"));
}


static void
gm_aw_edit_account_dialog_run (GtkWidget *accounts_window,
			       GmAccount *account,
			       GtkWidget *parent_window)
{
  GtkWidget *dialog = NULL;

  GtkWidget *table = NULL;
  GtkWidget *label = NULL;

  GtkWidget *menu = NULL;
  GtkWidget *item = NULL;

  PRegularExpression regex ("^[a-z1-9][a-z0-9-. ]*$", 
			    PRegularExpression::IgnoreCase);

  PString username;
  PString host;
  PString domain;
  PString password;
  PString account_name;

  gint protocol = 0;

  gboolean valid = FALSE;
  gboolean is_editing = FALSE;

  GmAccountsEditWindow *aew = NULL;

  is_editing = (account != NULL);

  /**/
  aew = new GmAccountsEditWindow ();
  dialog =
    gtk_dialog_new_with_buttons (_("Edit the Account Information"), 
				 GTK_WINDOW (NULL),
				 GTK_DIALOG_MODAL,
				 GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
				 GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
				 NULL);
  gtk_dialog_set_default_response (GTK_DIALOG (dialog),
				   GTK_RESPONSE_ACCEPT);
  gtk_window_set_transient_for (GTK_WINDOW (dialog), 
				GTK_WINDOW (parent_window));

  table = gtk_table_new (6, 2, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 3);
  gtk_table_set_col_spacings (GTK_TABLE (table), 6);
  gtk_container_set_border_width (GTK_CONTAINER (table), 12);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), table);


  /* Account Name */
  label = gtk_label_new (_("Account Name:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  aew->account_entry = gtk_entry_new ();
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 0, 1); 
  gtk_table_attach_defaults (GTK_TABLE (table), aew->account_entry, 1, 2, 0, 1); 
  gtk_entry_set_activates_default (GTK_ENTRY (aew->account_entry), TRUE);
  if (account && account->account_name)
    gtk_entry_set_text (GTK_ENTRY (aew->account_entry), account->account_name);

  /* Protocol */
  if (!is_editing) {

    label = gtk_label_new (_("Protocol:"));
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
    menu = gtk_menu_new ();
    aew->protocol_option_menu = gtk_option_menu_new ();
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), aew->protocol_option_menu);
    item = gtk_menu_item_new_with_label ("SIP");
    gtk_widget_show (item);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
    item = gtk_menu_item_new_with_label ("H.323");
    gtk_widget_show (item);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
    gtk_option_menu_set_menu (GTK_OPTION_MENU (aew->protocol_option_menu), menu);
    gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 1, 2); 
    gtk_table_attach_defaults (GTK_TABLE (table), aew->protocol_option_menu, 
			       1, 2, 1, 2); 

    g_signal_connect (GTK_OPTION_MENU (aew->protocol_option_menu)->menu,
		      "deactivate", 
		      G_CALLBACK (account_dialog_protocol_changed_cb),
		      (gpointer) aew);
  }

  /* Host */
  if (!account || !strcmp (account->protocol_name, "SIP"))
    aew->host_label = gtk_label_new (_("Registrar:"));
  else
    aew->host_label = gtk_label_new (_("Gatekeeper:"));
  gtk_misc_set_alignment (GTK_MISC (aew->host_label), 0.0, 0.5);
  aew->host_entry = gtk_entry_new ();
  gtk_table_attach_defaults (GTK_TABLE (table), aew->host_label, 0, 1, 2, 3); 
  gtk_table_attach_defaults (GTK_TABLE (table), aew->host_entry, 1, 2, 2, 3); 
  gtk_entry_set_activates_default (GTK_ENTRY (aew->host_entry), TRUE);
  if (account && account->host)
    gtk_entry_set_text (GTK_ENTRY (aew->host_entry), account->host);

  /* Realm/Domain */
  if (!account || !strcmp (account->protocol_name, "SIP"))
    aew->domain_label = gtk_label_new (_("Realm/Domain:"));
  else
    aew->domain_label = gtk_label_new (_("Gatekeeper ID:"));
  gtk_misc_set_alignment (GTK_MISC (aew->domain_label), 0.0, 0.5);
  aew->domain_entry = gtk_entry_new ();
  gtk_table_attach_defaults (GTK_TABLE (table), aew->domain_label, 0, 1, 3, 4); 
  gtk_table_attach_defaults (GTK_TABLE (table), aew->domain_entry, 1, 2, 3, 4); 
  gtk_entry_set_activates_default (GTK_ENTRY (aew->domain_entry), TRUE);
  if (account && account->domain)
    gtk_entry_set_text (GTK_ENTRY (aew->domain_entry), account->domain);

  /* User Name */
  label = gtk_label_new (_("User Name:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  aew->username_entry = gtk_entry_new ();
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 4, 5); 
  gtk_table_attach_defaults (GTK_TABLE (table), aew->username_entry, 
			     1, 2, 4, 5); 
  gtk_entry_set_activates_default (GTK_ENTRY (aew->username_entry), TRUE);
  if (account && account->login)
    gtk_entry_set_text (GTK_ENTRY (aew->username_entry), account->login);

  /* Password */
  label = gtk_label_new (_("Password:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  aew->password_entry = gtk_entry_new ();
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 5, 6); 
  gtk_table_attach_defaults (GTK_TABLE (table), aew->password_entry, 
			     1, 2, 5, 6); 
  gtk_entry_set_activates_default (GTK_ENTRY (aew->password_entry), TRUE);
  gtk_entry_set_visibility (GTK_ENTRY (aew->password_entry), FALSE);
  if (account && account->password)
    gtk_entry_set_text (GTK_ENTRY (aew->password_entry), account->password);


  gtk_widget_show_all (dialog);
  while (!valid) {

    switch (gtk_dialog_run (GTK_DIALOG (dialog))) {

    case GTK_RESPONSE_ACCEPT:

      username = gtk_entry_get_text (GTK_ENTRY (aew->username_entry));
      account_name = gtk_entry_get_text (GTK_ENTRY (aew->account_entry));
      host = gtk_entry_get_text (GTK_ENTRY (aew->host_entry));
      password = gtk_entry_get_text (GTK_ENTRY (aew->password_entry));
      domain = gtk_entry_get_text (GTK_ENTRY (aew->domain_entry));
      if (!is_editing)
	protocol = // take it from the menu
	  gtk_option_menu_get_history (GTK_OPTION_MENU (aew->protocol_option_menu));
      else // take it from the existing account field
	protocol = (account->protocol_name 
		    && !strcmp (account->protocol_name, "SIP") ? 0 : 1);

      /* Check at least an account name, registrar, 
       * and username are provided */
      if (protocol == 0) // SIP
	valid = (username.FindRegEx (regex) != P_MAX_INDEX
		 && domain.FindRegEx (regex) != P_MAX_INDEX
		 && account_name.FindRegEx (regex) != P_MAX_INDEX
		 && host.FindRegEx (regex) != P_MAX_INDEX);
      else // H.323
	valid = (account_name.FindRegEx (regex) != P_MAX_INDEX);

      if (valid) {

	if (!is_editing)
	  account = gm_account_new ();

	g_free (account->login);
	g_free (account->account_name);
	g_free (account->host);
	g_free (account->password);
	g_free (account->domain);
	if (!is_editing)
	  g_free (account->protocol_name);

	account->account_name = g_strdup (account_name);
	account->host = g_strdup (host);
	account->domain = g_strdup (domain);
	account->login = g_strdup (username);
	account->password = g_strdup (password);
	if (!is_editing)
	  account->protocol_name = g_strdup ((protocol == 0) ? "SIP" : "H.323");

	/* The GUI will be updated through the GmConf notifiers */
	if (is_editing) 
	  gnomemeeting_account_modify (account);
	else 
	  gnomemeeting_account_add (account);
      }
      else {
	if (protocol == 0) // SIP
	  gnomemeeting_error_dialog (GTK_WINDOW (dialog), _("Missing information"), _("Please make sure to provide a valid account name, host name, realm and user name."));
	else // H.323
	  gnomemeeting_error_dialog (GTK_WINDOW (dialog), _("Missing information"), _("Please make sure to provide a valid account name, and host name."));
      }
      break;

    case GTK_RESPONSE_REJECT:
      valid = TRUE;
      break;
    }
  }

  delete ((GmAccountsEditWindow *) aew);
  gtk_widget_destroy (dialog);
}


static void 
gm_aw_delete_account_dialog_run (GtkWidget *accounts_window,
				 GmAccount *account,
				 GtkWidget *parent_window)
{
  GtkWidget *dialog = NULL;

  gchar *confirm_msg = NULL;

  g_return_if_fail (accounts_window != NULL);
  g_return_if_fail (account != NULL);

  /* Create the dialog to delete the account */
  confirm_msg = 
    g_strdup_printf (_("Are you sure you want to delete account %s?"), 
		     account->account_name);
  dialog =
    gtk_message_dialog_new (GTK_WINDOW (accounts_window),
			    GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION,
			    GTK_BUTTONS_YES_NO, confirm_msg);
  g_free (confirm_msg);

  gtk_dialog_set_default_response (GTK_DIALOG (dialog),
				   GTK_RESPONSE_YES);

  gtk_widget_show_all (dialog);


  /* Now run the dialg */
  switch (gtk_dialog_run (GTK_DIALOG (dialog))) {

  case GTK_RESPONSE_YES:

    /* The GUI will be updated throught the GmConf notifiers */
    gnomemeeting_account_delete (account);
    break;
  }

  gtk_widget_destroy (dialog);
}


GmAccount *
gm_aw_get_selected_account (GtkWidget *accounts_window)
{
  GmAccountsWindow *aw = NULL;

  GtkTreeModel *model = NULL;
  GtkTreeSelection *selection = NULL;
  GtkTreeIter iter;

  GmAccount *account = NULL;

  g_return_val_if_fail (accounts_window != NULL, NULL);

  /* Get the required data */
  aw = gm_aw_get_aw (accounts_window);

  g_return_val_if_fail (aw != NULL, NULL);

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (aw->accounts_list));
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (aw->accounts_list));

  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {

    account = gm_account_new ();

    gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 
			COLUMN_ACCOUNT_ENABLED, &account->enabled,
			COLUMN_ACCOUNT_AID, &account->aid,
			COLUMN_ACCOUNT_ACCOUNT_NAME, &account->account_name,
			COLUMN_ACCOUNT_PROTOCOL_NAME, &account->protocol_name,
			COLUMN_ACCOUNT_HOST, &account->host,
			COLUMN_ACCOUNT_DOMAIN, &account->domain,
			COLUMN_ACCOUNT_LOGIN, &account->login,
			COLUMN_ACCOUNT_PASSWORD, &account->password,
			COLUMN_ACCOUNT_TIMEOUT, &account->timeout,
			COLUMN_ACCOUNT_METHOD, &account->method,
			-1); 
  }

  return account;
}


/* GTK+ Callbacks */
static void
account_toggled_cb (GtkCellRendererToggle *cell,
		    gchar *path_str,
		    gpointer data)
{
  GMEndPoint *ep = NULL;

  GmAccountsWindow *aw = NULL;
  GmAccount *account = NULL;

  GtkTreePath *path = NULL;
  GtkTreeSelection *selection = NULL;

  GtkWidget *accounts_window = NULL;

  accounts_window = GnomeMeeting::Process ()->GetAccountsWindow ();
  ep = GnomeMeeting::Process ()->Endpoint ();

  aw = gm_aw_get_aw (accounts_window);


  /* Make sure the toggled row is selected */
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (aw->accounts_list));
  path = gtk_tree_path_new_from_string (path_str);
  gtk_tree_selection_select_path (selection, path);
  gtk_tree_path_free (path);

  /* Update the config */
  account = gm_aw_get_selected_account (accounts_window);
  gnomemeeting_account_toggle_active (account);

  gdk_threads_leave ();
  ep->Register (account);
  gdk_threads_enter ();

  gm_account_delete (account);
}


static void
add_account_cb (GtkWidget *button, 
		gpointer data)
{
  GtkWidget *accounts_window = NULL;

  accounts_window = GnomeMeeting::Process ()->GetAccountsWindow (); 

  gm_aw_edit_account_dialog_run (GTK_WIDGET (accounts_window), 
				 NULL, 
				 GTK_WIDGET (accounts_window));
}


static void
edit_account_cb (GtkWidget *button, 
		 gpointer data)
{
  GmAccount *account = NULL;
  GtkWidget *accounts_window = NULL;

  accounts_window = GnomeMeeting::Process ()->GetAccountsWindow (); 

  account = gm_aw_get_selected_account (accounts_window);
  gm_aw_edit_account_dialog_run (GTK_WIDGET (accounts_window), 
				 account, 
				 GTK_WIDGET (accounts_window));
  gm_account_delete (account);
}


static void
delete_account_cb (GtkWidget *button, 
		   gpointer data)
{
  GmAccount *account = NULL;
  GtkWidget *accounts_window = NULL;

  accounts_window = GnomeMeeting::Process ()->GetAccountsWindow (); 

  account = gm_aw_get_selected_account (accounts_window);
  if (account)
    gm_aw_delete_account_dialog_run (GTK_WIDGET (accounts_window), 
				     account, 
				     GTK_WIDGET (accounts_window));
  gm_account_delete (account);
}


static void
account_dialog_protocol_changed_cb (GtkWidget *menu,
				    gpointer data)
{
  GmAccountsEditWindow *aew = NULL;

  g_return_if_fail (data != NULL);

  aew = GM_ACCOUNTS_EDIT_WINDOW (data);

  switch (gtk_option_menu_get_history (GTK_OPTION_MENU (aew->protocol_option_menu)))
    {
    case 0:
      gtk_label_set_text (GTK_LABEL (aew->host_label), _("Registrar:"));
      gtk_label_set_text (GTK_LABEL (aew->domain_label), _("Realm/Domain:"));
      break;

    case 1:
      gtk_label_set_text (GTK_LABEL (aew->host_label), _("Gatekeeper:"));
      gtk_label_set_text (GTK_LABEL (aew->domain_label), _("Gatekeeper ID:"));
      break;
    };
}


/* Implementation of public functions */
GmAccount *
gm_account_new ()
{
  GmAccount *account = NULL;

  account = g_new (GmAccount, 1);

  account->aid = g_strdup (OpalGloballyUniqueID ().AsString ());
  account->account_name = NULL;
  account->protocol_name = NULL;
  account->host = NULL;
  account->domain = NULL;
  account->login = NULL;
  account->password = NULL;
  account->enabled = FALSE;
  account->timeout = 0;
  account->method = 0;

  return account;
}


void
gm_account_delete (GmAccount *account)
{
  if (!account)
    return;

  g_free (account->aid);
  g_free (account->account_name);
  g_free (account->protocol_name);
  g_free (account->domain);
  g_free (account->login);
  g_free (account->password);
  g_free (account->host);

  g_free (account);
}


GmAccount *
gm_account_copy (GmAccount *a)
{
  GmAccount *account = NULL;

  if (!a)
    return account;

  account = g_new (GmAccount, 1);

  account->aid = g_strdup (a->aid);
  account->account_name = g_strdup (a->account_name);
  account->protocol_name = g_strdup (a->protocol_name);
  account->host = g_strdup (a->host);
  account->domain = g_strdup (a->domain);
  account->login = g_strdup (a->login);
  account->password = g_strdup (a->password);
  account->enabled = a->enabled;
  account->timeout = a->timeout;
  account->method = a->method;

  return account;
}


gboolean 
gnomemeeting_account_add (GmAccount *account)
{
  GSList *list = NULL;
  gchar *entry = NULL;

  list = 
    gm_conf_get_string_list (PROTOCOLS_KEY "accounts_list");

  entry = g_strdup_printf ("%d|%s|%s|%s|%s|%s|%s|%s|%d|%d", 
			   account->enabled,
			   account->aid, 
			   account->account_name, 
			   account->protocol_name,
			   account->host,
			   account->domain,
			   account->login,
			   account->password,
			   account->timeout,
			   account->method);

  list = g_slist_append (list, (gpointer) entry);
  gm_conf_set_string_list (PROTOCOLS_KEY "accounts_list", 
			   list);

  g_slist_foreach (list, (GFunc) g_free, NULL);
  g_slist_free (list);

  return TRUE;
}


gboolean 
gnomemeeting_account_delete (GmAccount *account)
{
  GSList *list = NULL;
  GSList *l = NULL;

  gchar *entry = NULL;

  gboolean found = FALSE;

  list = 
    gm_conf_get_string_list (PROTOCOLS_KEY "accounts_list");

  entry = g_strdup_printf ("%d|%s|%s|%s|%s|%s|%s|%s|%d|%d", 
			   account->enabled,
			   account->aid, 
			   account->account_name, 
			   account->protocol_name,
			   account->host,
			   account->domain,
			   account->login,
			   account->password,
			   account->timeout,
			   account->method);

  l = list;
  while (l && !found) {

    if (l->data && !strcmp ((const char *) l->data, entry)) {

      found = TRUE;
      break;
    }

    l = g_slist_next (l);
  }

  if (found) {

    list = g_slist_remove_link (list, l);

    g_free (l->data);
    g_slist_free_1 (l);

    gm_conf_set_string_list (PROTOCOLS_KEY "accounts_list", 
			     list);
  }

  g_slist_foreach (list, (GFunc) g_free, NULL);
  g_slist_free (list);

  g_free (entry);

  return found;
}


gboolean 
gnomemeeting_account_modify (GmAccount *account)
{
  GSList *list = NULL;
  GSList *l = NULL;

  gchar *entry = NULL;
  gchar **couple = NULL;

  gboolean found = FALSE;

  list = 
    gm_conf_get_string_list (PROTOCOLS_KEY "accounts_list");

  entry = g_strdup_printf ("%d|%s|%s|%s|%s|%s|%s|%s|%d|%d", 
			   account->enabled,
			   account->aid, 
			   account->account_name, 
			   account->protocol_name,
			   account->host,
			   account->domain,
			   account->login,
			   account->password,
			   account->timeout,
			   account->method);


  l = list;
  while (l && !found) {

    if (l->data) {

      couple = g_strsplit ((const char *) l->data, "|", 0);
      if (couple && couple [1] && !strcmp (couple [1], account->aid)) {

	found = TRUE;
	break;
      }
    }

    l = g_slist_next (l);
  }

  if (found) {

    list = g_slist_insert_before (list, l, (gpointer) entry);
    list = g_slist_remove_link (list, l);

    g_free (l->data);
    g_slist_free_1 (l);

    gm_conf_set_string_list (PROTOCOLS_KEY "accounts_list", 
			     list);
  }

  g_slist_foreach (list, (GFunc) g_free, NULL);
  g_slist_free (list);

  return found;
}


GSList *
gnomemeeting_get_accounts_list ()
{
  GSList *result = NULL;

  GSList *accounts_data_iter = NULL;
  GSList *accounts_data = NULL;

  GmAccount *account = NULL;

  gint size = 0;
  gchar **couple = NULL;

  accounts_data = 
    gm_conf_get_string_list (PROTOCOLS_KEY "accounts_list");

  accounts_data_iter = accounts_data;
  while (accounts_data_iter) {

    couple = g_strsplit ((gchar *) accounts_data_iter->data, "|", 0);

    if (couple) {

      while (couple [size])
	size++;
      size = size + 1;

      account = gm_account_new ();

      if (size >= 1 && couple [0])
	account->enabled = atoi (couple [0]);
      if (size >= 2 && couple [1])
	account->aid = g_strdup (couple [1]);
      if (size >= 3 && couple [2])
	account->account_name = g_strdup (couple [2]);
      if (size >= 4 && couple [3])
	account->protocol_name = g_strdup (couple [3]);
      if (size >= 5 && couple [4])
	account->host = g_strdup (couple [4]);
      if (size >= 6 && couple [5])
	account->domain = g_strdup (couple [5]);
      if (size >= 7 && couple [6])
	account->login = g_strdup (couple [6]);
      if (size >= 8 && couple [7])
	account->password = g_strdup (couple [7]);
      if (size >= 9 && couple [8])
	account->timeout = atoi (couple [8]);
      if (size >= 10 && couple [9])
	account->method = atoi (couple [9]);

      result = g_slist_append (result, (void *) account);

      g_strfreev (couple);
    }

    accounts_data_iter = g_slist_next (accounts_data_iter);
  }

  g_slist_foreach (accounts_data, (GFunc) g_free, NULL);
  g_slist_free (accounts_data);

  return result;
}


gboolean 
gnomemeeting_account_toggle_active (GmAccount *account)
{
  if (!account)
    return FALSE;

  account->enabled = !account->enabled;

  return gnomemeeting_account_modify (account);
}


GtkWidget *
gm_accounts_window_new ()
{
  GmAccountsWindow *aw = NULL;

  GtkWidget *window = NULL;

  GtkWidget *event_box = NULL;
  GtkWidget *scroll_window = NULL;

  GtkWidget *frame = NULL;
  GtkWidget *hbox = NULL;

  GtkWidget *button = NULL;
  GtkWidget *buttons_vbox = NULL;
  GtkWidget *alignment = NULL;

  GtkCellRenderer *renderer = NULL;
  GtkListStore *list_store = NULL;
  GtkTreeViewColumn *column = NULL;

  gchar *column_names [] = {

    "",
    "",
    _("Account Name"),
    _("Protocol"),
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    _("Status"),
    ""
  };

  /* The window */
  window = gtk_dialog_new ();
  gtk_dialog_add_button (GTK_DIALOG (window), GTK_STOCK_CLOSE, 0);

  g_object_set_data_full (G_OBJECT (window), "window_name",
			  g_strdup ("accounts_window"), g_free);

  gtk_window_set_title (GTK_WINDOW (window), _("Accounts"));
  gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);

  aw = new GmAccountsWindow ();
  g_object_set_data_full (G_OBJECT (window), "GMObject", 
			  aw, gm_aw_destroy);


  /* The accounts list store */
  list_store = gtk_list_store_new (COLUMN_ACCOUNT_NUMBER,
				   G_TYPE_BOOLEAN, /* Enabled? */
				   G_TYPE_STRING,  /* AID */
				   G_TYPE_STRING,  /* Account Name */
				   G_TYPE_STRING,  /* Protocol Name */
				   G_TYPE_STRING,  /* Host */
				   G_TYPE_STRING,  /* Domain */
				   G_TYPE_STRING,  /* Login */
				   G_TYPE_STRING,  /* Password */
				   G_TYPE_INT,     /* State */
				   G_TYPE_INT,     /* Timeout */
				   G_TYPE_INT,     /* Method */
				   G_TYPE_STRING,  /* Error Message */  
				   G_TYPE_INT);    /* Activatable */

  aw->accounts_list = 
    gtk_tree_view_new_with_model (GTK_TREE_MODEL (list_store));
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (aw->accounts_list), TRUE);
  gtk_tree_view_set_reorderable (GTK_TREE_VIEW (aw->accounts_list), TRUE);
  gtk_tree_view_set_search_column (GTK_TREE_VIEW (aw->accounts_list),0);

  renderer = gtk_cell_renderer_toggle_new ();
  column = gtk_tree_view_column_new_with_attributes (_("A"),
						     renderer,
						     "active", 
						     COLUMN_ACCOUNT_ENABLED,
						     NULL);
  gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 25);
  gtk_tree_view_append_column (GTK_TREE_VIEW (aw->accounts_list), column);
  gtk_tree_view_column_add_attribute (column, renderer, 
				      "activatable", 
				      COLUMN_ACCOUNT_ACTIVATABLE);
  g_signal_connect (G_OBJECT (renderer), "toggled",
  		    G_CALLBACK (account_toggled_cb),
  		    (gpointer) aw->accounts_list);


  /* Add all text renderers, ie all except the "ACCOUNT_ENABLED" column */
  for (int i = COLUMN_ACCOUNT_AID ; i < COLUMN_ACCOUNT_NUMBER - 1 ; i++) {

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (column_names [i],
						       renderer,
						       "text", 
						       i,
						       NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (aw->accounts_list), column);

    if (i == COLUMN_ACCOUNT_AID 
	|| i == COLUMN_ACCOUNT_HOST
	|| i == COLUMN_ACCOUNT_TIMEOUT
	|| i == COLUMN_ACCOUNT_METHOD 
	|| i == COLUMN_ACCOUNT_DOMAIN
	|| i == COLUMN_ACCOUNT_LOGIN
	|| i == COLUMN_ACCOUNT_PASSWORD
	|| i == COLUMN_ACCOUNT_STATE)
      g_object_set (G_OBJECT (column), "visible", false, NULL);
  }


  /* The scrolled window with the accounts list store */
  scroll_window = gtk_scrolled_window_new (FALSE, FALSE);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_window), 
				  GTK_POLICY_NEVER, 
				  GTK_POLICY_AUTOMATIC);

  event_box = gtk_event_box_new ();
  hbox = gtk_hbox_new (FALSE, 4);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 4);
  gtk_container_add (GTK_CONTAINER (event_box), hbox);

  frame = gtk_frame_new (NULL);
  gtk_widget_set_size_request (GTK_WIDGET (frame), 250, 150);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 
				  2 * GNOMEMEETING_PAD_SMALL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_container_add (GTK_CONTAINER (frame), scroll_window);
  gtk_container_add (GTK_CONTAINER (scroll_window), aw->accounts_list);
  gtk_container_set_border_width (GTK_CONTAINER (aw->accounts_list), 0);
  gtk_box_pack_start (GTK_BOX (hbox), frame, TRUE, TRUE, 0);


  /* The buttons */
  alignment = gtk_alignment_new (1, 0.5, 0, 0);
  buttons_vbox = gtk_vbutton_box_new ();

  gtk_box_set_spacing (GTK_BOX (buttons_vbox), 2 * GNOMEMEETING_PAD_SMALL);

  gtk_container_add (GTK_CONTAINER (alignment), buttons_vbox);
  gtk_box_pack_start (GTK_BOX (hbox), alignment, 
		      FALSE, FALSE, 2 * GNOMEMEETING_PAD_SMALL);

  button = gtk_button_new_from_stock (GTK_STOCK_ADD);
  gtk_box_pack_start (GTK_BOX (buttons_vbox), button, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button), "clicked", 
  		    GTK_SIGNAL_FUNC (add_account_cb), NULL); 

  button = gtk_button_new_from_stock (GTK_STOCK_REMOVE);
  gtk_box_pack_start (GTK_BOX (buttons_vbox), button, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button), "clicked", 
  		    GTK_SIGNAL_FUNC (delete_account_cb), NULL); 

  button = gtk_button_new_from_stock (GTK_STOCK_PROPERTIES);
  gtk_box_pack_start (GTK_BOX (buttons_vbox), button, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button), "clicked", 
  		    GTK_SIGNAL_FUNC (edit_account_cb), NULL); 

  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->vbox), 
		      event_box, TRUE, TRUE, 0);


  /* Generic signals */
  g_signal_connect_swapped (GTK_OBJECT (window), 
			    "response", 
			    G_CALLBACK (gnomemeeting_window_hide),
			    (gpointer) window);

  g_signal_connect (GTK_OBJECT (window), "delete-event", 
		    G_CALLBACK (delete_window_cb), NULL);

  gtk_widget_show_all (GTK_WIDGET (GTK_DIALOG (window)->vbox));

  
  /* We update it the accounts list to make sure the cursor is updated */
  gm_accounts_window_update_accounts_list (window);

  
  return window;
}


void
gm_accounts_window_update_account_state (GtkWidget *accounts_window,
					 gboolean refreshing,
					 const char *domain,
					 const char *login,
					 const char *status)
{
  GtkTreeModel *model = NULL;

  GtkTreeIter iter;

  gchar *host = NULL;
  gchar *realm = NULL;
  gchar *username = NULL;

  GmAccountsWindow *aw = NULL;

  g_return_if_fail (accounts_window != NULL);
  g_return_if_fail (login != NULL);
  g_return_if_fail (status != NULL);

  aw = gm_aw_get_aw (accounts_window);

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (aw->accounts_list));

  if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter)){

    do {

      gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
			  COLUMN_ACCOUNT_HOST, &host,
			  COLUMN_ACCOUNT_DOMAIN, &realm,
			  COLUMN_ACCOUNT_LOGIN, &username,
			  -1);

      if (((host && domain && !strcmp (host, domain))
	   || (realm && domain && !strcmp (realm, domain)))
	  && (login && username && !strcmp (login, username))) 
	gtk_list_store_set (GTK_LIST_STORE (model), &iter,
			    COLUMN_ACCOUNT_STATE, refreshing,
			    COLUMN_ACCOUNT_ERROR_MESSAGE, status, -1);

      g_free (host);
      g_free (realm);
      g_free (username);

    } while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));
  }


  /* We update it the accounts list to make sure the cursor is updated */
  gm_accounts_window_update_accounts_list (accounts_window);
}


void
gm_accounts_window_update_accounts_list (GtkWidget *accounts_window)
{
  GmAccountsWindow *aw = NULL;

  GdkCursor *cursor = NULL;

  GtkTreeSelection *selection = NULL;
  GtkTreeModel *model = NULL;
  GtkTreeIter iter;

  gchar *selected_aid = NULL;
  gchar *aid = NULL;

  GmAccount *account = NULL;

  GSList *accounts_data = NULL;
  GSList *accounts_data_iter = NULL;

  gboolean found = FALSE;
  gboolean enabled = FALSE;
  gboolean refreshing = FALSE;
  gboolean busy = FALSE;
  gboolean valid_iter = TRUE;

  g_return_if_fail (accounts_window != NULL);

  aw = gm_aw_get_aw (accounts_window);


  /* Get the data and the selected codec */
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (aw->accounts_list));
  selection = 
    gtk_tree_view_get_selection (GTK_TREE_VIEW (aw->accounts_list));

  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {

    gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
			COLUMN_ACCOUNT_AID, &selected_aid, -1);
  }


  /* Loop through all accounts in the configuration.
   * Then find that account in the GUI and updates it. 
   * If we do not find the account in the GUI, append the new account
   * at the end.
   */
  accounts_data = gnomemeeting_get_accounts_list (); 
  accounts_data_iter = accounts_data;
  while (accounts_data_iter && accounts_data_iter->data) {

    account = GM_ACCOUNT (accounts_data_iter->data);

    found = FALSE;
    if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter)) {

      do {

	gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
			    COLUMN_ACCOUNT_ENABLED, &enabled,
			    COLUMN_ACCOUNT_STATE, &refreshing,
			    COLUMN_ACCOUNT_AID, &aid, -1);
	if (aid && account->aid && !strcmp (account->aid, aid)) {

	  busy = busy || refreshing;
	  found = TRUE;
	}
	g_free (aid);

      } while (!found && 
	       gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter)); 
    }
    if (!found) /* No existing entry for that account */ 
      gtk_list_store_append (GTK_LIST_STORE (model), &iter);

    gtk_list_store_set (GTK_LIST_STORE (model), &iter, 
			COLUMN_ACCOUNT_ENABLED, account->enabled,
			COLUMN_ACCOUNT_AID, account->aid,
			COLUMN_ACCOUNT_ACCOUNT_NAME, account->account_name,
			COLUMN_ACCOUNT_PROTOCOL_NAME, account->protocol_name,
			COLUMN_ACCOUNT_HOST, account->host,
			COLUMN_ACCOUNT_DOMAIN, account->domain,
			COLUMN_ACCOUNT_LOGIN, account->login,
			COLUMN_ACCOUNT_PASSWORD, account->password,
			COLUMN_ACCOUNT_TIMEOUT, account->timeout,
			COLUMN_ACCOUNT_METHOD, account->method,
			-1); 

    if (selected_aid && account->aid 
	&& !strcmp (selected_aid, account->aid))
      gtk_tree_selection_select_iter (selection, &iter);

    accounts_data_iter = g_slist_next (accounts_data_iter);
  }


  /* Loop through the accounts in the window, and check
   * if they are in the configuration. If not, remove them from the GUI.
   */
  if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter)) {

    do {

      found = FALSE;
      valid_iter = TRUE;

      gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
			  COLUMN_ACCOUNT_AID, &aid, -1);

      accounts_data_iter = accounts_data;
      while (accounts_data_iter && accounts_data_iter->data && !found) {

	account = GM_ACCOUNT (accounts_data_iter->data);

	if (account->aid && aid && !strcmp (account->aid, aid))
	  found = TRUE;

	accounts_data_iter = g_slist_next (accounts_data_iter);
      }

      if (!found)
	valid_iter = gtk_list_store_remove (GTK_LIST_STORE (model), &iter);

      g_free (aid);

    } while (valid_iter &&
	     gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter)); 
  }


  /* Free things */
  g_slist_foreach (accounts_data, (GFunc) gm_account_delete, NULL);
  g_slist_free (accounts_data);


  /* Update the cursor and the activatable state of all accounts
   * following we are refreshing or not */
  if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter)) {

    do {

      gtk_list_store_set (GTK_LIST_STORE (model), &iter,
			  COLUMN_ACCOUNT_ACTIVATABLE, !busy, -1);
    } while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));
  }

  gtk_widget_realize (GTK_WIDGET (accounts_window));

  if (busy) {

    cursor = gdk_cursor_new (GDK_WATCH);
    gdk_window_set_cursor (GTK_WIDGET (accounts_window)->window, cursor);
    gdk_cursor_unref (cursor);
  }
  else
    gdk_window_set_cursor (GTK_WIDGET (accounts_window)->window, NULL);
}



/* Class to register accounts in a thread.
*/
GMAccountsManager::GMAccountsManager (GmAccount *a)
:PThread (1000, NoAutoDeleteThread)
{
  account = gm_account_copy (a);

  this->Resume ();
  thread_sync_point.Wait ();
}


GMAccountsManager::~GMAccountsManager ()
{
  /* Nothing to do here */
  PWaitAndSignal m(quit_mutex);

  gm_account_delete (account);
}


void GMAccountsManager::Main ()
{
  GSList *accounts = NULL;
  GSList *accounts_iter = NULL;

  GmAccount *list_account = NULL;

  PWaitAndSignal m(quit_mutex);
  thread_sync_point.Signal ();


  /* Let's go */
  if (account) {

    if (!strcmp (account->protocol_name, "SIP")) 
      SIPRegister (account);
    else
      H323Register (account);
  }
  else {

    accounts = gnomemeeting_get_accounts_list ();

    accounts_iter = accounts;
    while (accounts_iter) {

      if (accounts_iter->data) {

	list_account = GM_ACCOUNT (accounts_iter)->data;

	/* Register SIP account */
	if (list_account->protocol_name) {

	  if (!strcmp (list_account->protocol_name, "SIP")) 
	    SIPRegister (list_account);
	  else
	    H323Register (list_account);
	}
      }

      accounts_iter = g_slist_next (accounts_iter);
    }

    g_slist_foreach (accounts, (GFunc) gm_account_delete, NULL);
    g_slist_free (accounts);
  }
}


void GMAccountsManager::SIPRegister (GmAccount *a)
{
  GtkWidget *accounts_window = NULL;
  GtkWidget *main_window = NULL;
  GtkWidget *history_window = NULL;

  gchar *msg = NULL;

  gboolean result = FALSE;

  GMEndPoint *endpoint = NULL;
  GMSIPEndPoint *sipEP = NULL;

  endpoint = GnomeMeeting::Process ()->Endpoint ();

  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  history_window = GnomeMeeting::Process ()->GetHistoryWindow ();
  accounts_window = GnomeMeeting::Process ()->GetAccountsWindow ();

  sipEP = endpoint->GetSIPEndPoint ();

  if (!a)
    return;

  /* Account is enabled, and we are not registered */
  if (a->enabled && !sipEP->IsRegistered (a->host)) {

    gnomemeeting_threads_enter ();
    gm_accounts_window_update_account_state (accounts_window,
					     TRUE,
					     a->host,
					     a->login,
					     _("Registering"));
    gnomemeeting_threads_leave ();

    result = sipEP->Register (a->host, a->login, a->password, a->domain);

    if (!result) {

      msg = g_strdup_printf (_("Registration of %s to %s failed"), 
			     a->login?a->login:"", 
			     a->host?a->host:"");

      gnomemeeting_threads_enter ();
      gm_main_window_push_message (main_window, msg);
      gm_history_window_insert (history_window, msg);
      gm_accounts_window_update_account_state (accounts_window,
					       FALSE,
					       a->host,
					       a->login,
					       _("Registration failed"));
      gnomemeeting_threads_leave ();

      g_free (msg);
    }
  }
  else if (!a->enabled && sipEP->IsRegistered (a->host)) {

    gnomemeeting_threads_enter ();
    gm_accounts_window_update_account_state (accounts_window,
					     TRUE,
					     a->host,
					     a->login,
					     _("Unregistering"));
    gnomemeeting_threads_leave ();

    sipEP->Unregister (a->host,
		       a->login);
  }
}


void GMAccountsManager::H323Register (GmAccount *a)
{
  GtkWidget *accounts_window = NULL;
  GtkWidget *main_window = NULL;
  GtkWidget *history_window = NULL;

  gchar *msg = NULL;

  gboolean result = FALSE;

  GMEndPoint *endpoint = NULL;
  H323EndPoint *h323EP = NULL;
  H323Gatekeeper *gatekeeper = NULL;

  endpoint = GnomeMeeting::Process ()->Endpoint ();

  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  history_window = GnomeMeeting::Process ()->GetHistoryWindow ();
  accounts_window = GnomeMeeting::Process ()->GetAccountsWindow ();

  h323EP = (H323EndPoint *) endpoint->GetH323EndPoint ();

  if (!a)
    return;

  /* Account is enabled, and we are not registered, only one
   * account can be enabled at a time */
  if (a->enabled) {

    h323EP->RemoveGatekeeper (0);

    gnomemeeting_threads_enter ();
    gm_accounts_window_update_account_state (accounts_window,
					     TRUE,
					     a->host,
					     a->login,
					     _("Registering"));
    gnomemeeting_threads_leave ();

    if (a->login && strcmp (a->login, ""))
      h323EP->AddAliasName (a->login);
    h323EP->SetGatekeeperPassword (a->password);
    result = h323EP->UseGatekeeper (a->host, a->domain);

    /* There was an error (missing parameter or registration failed)
       or the user chose to not register */
    if (!result) {

      /* Registering failed */
      gatekeeper = h323EP->GetGatekeeper ();
      if (gatekeeper) {

	switch (gatekeeper->GetRegistrationFailReason()) {

	case H323Gatekeeper::DuplicateAlias :
	  msg = g_strdup (_("Gatekeeper registration failed: duplicate alias"));
	  break;
	case H323Gatekeeper::SecurityDenied :
	  msg = 
	    g_strdup (_("Gatekeeper registration failed: bad login/password"));
	  break;
	case H323Gatekeeper::TransportError :
	  msg = g_strdup (_("Gatekeeper registration failed: transport error"));
	  break;
	default :
	  msg = g_strdup (_("Gatekeeper registration failed"));
	  break;
	}
      }
      else
	msg = g_strdup (_("Gatekeeper registration failed"));
    }
    else
      msg = g_strdup_printf (_("Registered to %s"), a->host);

    gnomemeeting_threads_enter ();
    gm_main_window_push_message (main_window, msg);
    gm_history_window_insert (history_window, msg);
    gm_accounts_window_update_account_state (accounts_window,
					     FALSE,
					     a->host,
					     a->login,
					     result?
					     _("Registered")
					     :_("Registration failed"));
    gnomemeeting_threads_leave ();
    g_free (msg);
  }
  else if (!a->enabled && h323EP->IsRegisteredWithGatekeeper ()) {

    gnomemeeting_threads_enter ();
    gm_accounts_window_update_account_state (accounts_window,
					     TRUE,
					     a->host,
					     a->login,
					     _("Unregistering"));
    gnomemeeting_threads_leave ();

    h323EP->RemoveAliasName (a->login);
    h323EP->RemoveGatekeeper (0);

    gnomemeeting_threads_enter ();
    gm_accounts_window_update_account_state (accounts_window,
					     FALSE,
					     a->host,
					     a->login,
					     _("Unregistered"));
    gnomemeeting_threads_leave ();
  }
}

