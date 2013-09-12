
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2009 Damien Sandras <dsandras@seconix.com>
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
 *                         accounts.cpp  -  description
 *                         ----------------------------
 *   begin                : Sun Feb 13 2005
 *   copyright            : (C) 2000-2008 by Damien Sandras
 *   description          : This file contains all the functions needed to
 *   			    manipulate accounts.
 */

#include "accounts-window.h"

#include <glib/gi18n.h>

#include "account.h"
#include "bank.h"
#include "opal-bank.h"

#include "gmcallbacks.h"

#include "services.h"
#include "menu-builder-tools.h"
#include "menu-builder-gtk.h"
#include "form-dialog-gtk.h"
#include "optional-buttons-gtk.h"
#include "scoped-connections.h"


struct _AccountsWindowPrivate
{
  GtkWidget *accounts_list;
  GtkWidget *menu_item_core;
  GtkAccelGroup *accel;

  boost::shared_ptr<Ekiga::AccountCore> account_core;
  boost::shared_ptr<Ekiga::PersonalDetails> details;
  Ekiga::scoped_connections connections;

  std::string presence;

  OptionalButtonsGtk toolbar;
};

G_DEFINE_TYPE (AccountsWindow, accounts_window, GM_TYPE_WINDOW);


/* GTK+ Callbacks */

/* DESCRIPTION  :  This callback is called when the user clicks
 *                 on an account in the accounts window or when
 *                 there is an event_after.
 * BEHAVIOR     :  It updates the accounts window buttons sensitivity.
 * PRE          :  data is a valid pointer to the GmAccountsWindow.
 */
static gint account_clicked_cb (GtkWidget *w,
				GdkEventButton *e,
				gpointer data);

/* DESCRIPTION  :  This callback is called when the user clicks
 *                 on an account in the accounts window.
 * BEHAVIOR     :  It updates the toolbar actions to point to the right account,
 *                 and to be active/inactive depending if the action is
 *                 really available or not.
 * PRE          :  the 'data' is a pointer to the account window.
 */
static void on_selection_changed (GtkTreeSelection* /*selection*/,
				  gpointer data);

/* Columns for the VoIP accounts */
enum {

  COLUMN_ACCOUNT,
  COLUMN_ACCOUNT_ICON,
  COLUMN_ACCOUNT_IS_ENABLED,
  COLUMN_ACCOUNT_WEIGHT,
  COLUMN_ACCOUNT_ACCOUNT_NAME,
  COLUMN_ACCOUNT_STATUS,
  COLUMN_ACCOUNT_NUMBER
};


/* Engine callbacks */
static void
populate_menu (GtkWidget *window)
{
  MenuBuilderGtk builder;

  GtkWidget *item = NULL;
  GtkTreeSelection *selection = NULL;
  GtkTreeModel *model = NULL;
  GtkTreeIter iter;

  Ekiga::Account *account = NULL;
  AccountsWindow *self = ACCOUNTS_WINDOW (window);

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (self->priv->accounts_list));
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (self->priv->accounts_list));

  if (self->priv->account_core->populate_menu (builder)) {
    item = gtk_separator_menu_item_new ();
    gtk_menu_shell_append (GTK_MENU_SHELL (builder.menu), item);
  }

  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {

    gtk_tree_model_get (model, &iter,
                        COLUMN_ACCOUNT, &account,
                        -1);

    if (account->populate_menu (builder)) {
      item = gtk_separator_menu_item_new ();
      gtk_menu_shell_append (GTK_MENU_SHELL (builder.menu), item);
    }
  }
  item = gtk_image_menu_item_new_from_stock (GTK_STOCK_CLOSE, self->priv->accel);
  gtk_menu_shell_append (GTK_MENU_SHELL (builder.menu), item);
  g_signal_connect_swapped (item, "activate",
                            G_CALLBACK (gtk_widget_hide),
                            (gpointer) window);

  gtk_menu_item_set_submenu (GTK_MENU_ITEM (self->priv->menu_item_core),
                             builder.menu);

  gtk_widget_show_all (builder.menu);
}


/* GTK+ Callbacks */
static gint
account_clicked_cb (G_GNUC_UNUSED GtkWidget *w,
		    GdkEventButton *event,
		    gpointer data)
{
  AccountsWindow *self = ACCOUNTS_WINDOW (data);

  GtkTreePath *path = NULL;
  GtkTreeView *tree_view = NULL;
  GtkTreeModel *model = NULL;
  GtkTreeIter iter;

  Ekiga::Account *account = NULL;

  tree_view = GTK_TREE_VIEW (self->priv->accounts_list);
  model = gtk_tree_view_get_model (tree_view);

  if (event->type == GDK_BUTTON_PRESS || event->type == GDK_KEY_PRESS || event->type == GDK_2BUTTON_PRESS) {

    if (gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (tree_view),
                                       (gint) event->x, (gint) event->y,
                                       &path, NULL, NULL, NULL)) {

      if (gtk_tree_model_get_iter (model, &iter, path)) {

        gtk_tree_model_get (model, &iter,
                            COLUMN_ACCOUNT, &account,
                            -1);

        gtk_tree_path_free (path);
      }
    }
  }

  if (account == NULL)
    return FALSE;

  if (event->type == GDK_BUTTON_PRESS || event->type == GDK_KEY_PRESS) {

    populate_menu (GTK_WIDGET (data));

    if (event->button == 3) {

      MenuBuilderGtk builder;
      account->populate_menu (builder);
      if (!builder.empty ()) {

        gtk_widget_show_all (builder.menu);
        gtk_menu_popup (GTK_MENU (builder.menu), NULL, NULL,
                        NULL, NULL, event->button, event->time);
        g_signal_connect (builder.menu, "hide",
                          G_CALLBACK (g_object_unref),
                          (gpointer) builder.menu);
      }
      g_object_ref_sink (G_OBJECT (builder.menu));
    }
  }
  else if (event->type == GDK_2BUTTON_PRESS) {

    Ekiga::TriggerMenuBuilder builder;
    account->populate_menu (builder);
  }

  return TRUE;
}


static void
on_selection_changed (GtkTreeSelection* /*selection*/,
		      gpointer data)
{
  GtkTreeSelection* selection = NULL;
  GtkTreeModel *model = NULL;
  GtkTreeIter iter;
  Ekiga::Account* account = NULL;

  g_return_if_fail (data != NULL);
  AccountsWindow *self = ACCOUNTS_WINDOW (data);

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (self->priv->accounts_list));

  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {

    gtk_tree_model_get (model, &iter,
			COLUMN_ACCOUNT, &account,
			-1);
    if (account) {

      self->priv->toolbar.reset ();
      account->populate_menu (self->priv->toolbar);
    } else {

      self->priv->toolbar.reset ();
    }

  } else {

    self->priv->toolbar.reset ();
  }
}

static void
gm_accounts_window_add_account (GtkWidget *window,
                                Ekiga::AccountPtr account)
{
  std::string presence_icon;
  GtkTreeModel *model = NULL;

  GtkTreeIter iter;

  g_return_if_fail (window != NULL);
  AccountsWindow *self = ACCOUNTS_WINDOW (window);

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (self->priv->accounts_list));

  presence_icon =  account->is_enabled () ? ("user-" + self->priv->presence) : "user-offline";
  gtk_list_store_append (GTK_LIST_STORE (model), &iter);
  gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                      COLUMN_ACCOUNT, account.get (),
		      COLUMN_ACCOUNT_ICON, presence_icon.c_str (),
		      COLUMN_ACCOUNT_IS_ENABLED, account->is_enabled (),
                      COLUMN_ACCOUNT_WEIGHT, account->is_enabled () ? PANGO_WEIGHT_BOLD : PANGO_WEIGHT_NORMAL,
                      COLUMN_ACCOUNT_ACCOUNT_NAME, account->get_name ().c_str (),
                      -1);
}


void
gm_accounts_window_update_account (GtkWidget *accounts_window,
                                   Ekiga::AccountPtr account)
{
  std::string presence_icon;
  GtkTreeModel *model = NULL;
  GtkTreeSelection *selection = NULL;

  GtkTreeIter iter;
  Ekiga::Account *caccount = NULL;

  g_return_if_fail (accounts_window != NULL);
  AccountsWindow *self = ACCOUNTS_WINDOW (accounts_window);

  /* on the one end we update the view */
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (self->priv->accounts_list));

  if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter)){

    do {

      gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
			  COLUMN_ACCOUNT, &caccount, -1);

      if (caccount == account.get ()) {

        presence_icon =  account->is_enabled () ? ("user-" + self->priv->presence) : "user-offline";
        gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                            COLUMN_ACCOUNT, account.get (),
                            COLUMN_ACCOUNT_ICON, presence_icon.c_str (),
			    COLUMN_ACCOUNT_IS_ENABLED, account->is_enabled (),
                            COLUMN_ACCOUNT_WEIGHT, account->is_enabled () ? PANGO_WEIGHT_BOLD : PANGO_WEIGHT_NORMAL,
                            COLUMN_ACCOUNT_ACCOUNT_NAME, account->get_name ().c_str (),
			    COLUMN_ACCOUNT_STATUS, account->get_status ().c_str (),
                            -1);
        break;
      }
    } while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));
  }

  /* on the other end, if the updated account is the one which is selected,
   * we update the actions on it
   */
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (self->priv->accounts_list));
  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {

    gtk_tree_model_get (model, &iter,
                        COLUMN_ACCOUNT, &caccount,
                        -1);

    if (caccount == account.get ()) {

      self->priv->toolbar.reset ();
      account->populate_menu (self->priv->toolbar);
      populate_menu (accounts_window);
    }
  }
}


void
gm_accounts_window_remove_account (GtkWidget *accounts_window,
                                   Ekiga::AccountPtr account)
{
  Ekiga::Account *caccount = NULL;

  GtkTreeModel *model = NULL;

  GtkTreeIter iter;

  g_return_if_fail (accounts_window != NULL);
  AccountsWindow *self = ACCOUNTS_WINDOW (accounts_window);

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (self->priv->accounts_list));

  if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter)){

    do {

      gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
			  COLUMN_ACCOUNT, &caccount, -1);

      if (caccount == account.get ()) {

        gtk_list_store_remove (GTK_LIST_STORE (model), &iter);
        break;
      }
    } while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));
  }
}


void
gm_accounts_window_set_presence (GtkWidget *accounts_window,
                                 const std::string & presence)
{
  std::string presence_icon;

  Ekiga::Account *account = NULL;
  GtkTreeModel *model = NULL;

  GtkTreeIter iter;

  g_return_if_fail (accounts_window != NULL);
  AccountsWindow *self = ACCOUNTS_WINDOW (accounts_window);

  /* on the one end we update the view */
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (self->priv->accounts_list));

  if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter)){

    do {
      gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
			  COLUMN_ACCOUNT, &account, -1);

      presence_icon =  account->is_enabled () ? ("user-" + presence) : "user-offline";
      gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                          COLUMN_ACCOUNT_ICON, presence_icon.c_str (),
                          -1);
    } while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));
  }
}


static void
on_personal_details_updated (AccountsWindow* self,
                             boost::shared_ptr<Ekiga::PersonalDetails> details)
{
  self->priv->presence = details->get_presence ();
  gm_accounts_window_set_presence (GTK_WIDGET (self), details->get_presence ());
}


static bool
visit_accounts (Ekiga::AccountPtr account,
                gpointer data)
{
  gm_accounts_window_add_account (GTK_WIDGET (data), account);

  return true;
}


static void
on_account_added (Ekiga::BankPtr /*bank*/,
		  Ekiga::AccountPtr account,
                  gpointer data)
{
  gm_accounts_window_add_account (GTK_WIDGET (data), account);
}


static void
on_account_updated (Ekiga::BankPtr /*bank*/,
		    Ekiga::AccountPtr account,
                    gpointer data)
{
  gm_accounts_window_update_account (GTK_WIDGET (data), account);
}


static void
on_account_removed (Ekiga::BankPtr /*bank*/,
		    Ekiga::AccountPtr account,
                    gpointer data)
{
  gm_accounts_window_remove_account (GTK_WIDGET (data), account);
}


static void
on_bank_added (Ekiga::BankPtr bank,
               gpointer data)
{
  bank->visit_accounts (boost::bind (&visit_accounts, _1, data));
  populate_menu (GTK_WIDGET (data));
}

static bool
on_visit_banks (Ekiga::BankPtr bank,
		gpointer data)
{
  on_bank_added (bank, data);
  return true;
}

static bool
on_handle_questions (Ekiga::FormRequestPtr request,
                     gpointer data)
{
  FormDialog dialog (request, GTK_WIDGET (data));

  dialog.run ();

  return true;
}


/* Implementation of the GObject stuff */
static void
accounts_window_dispose (GObject *obj)
{
  AccountsWindow *self = ACCOUNTS_WINDOW (obj);

  if (self->priv->menu_item_core) {

    g_object_unref (self->priv->menu_item_core);
    self->priv->menu_item_core = NULL;
  }

  G_OBJECT_CLASS (accounts_window_parent_class)->dispose (obj);
}


static void
accounts_window_finalize (GObject *obj)
{
  AccountsWindow *self = ACCOUNTS_WINDOW (obj);

  delete self->priv;

  G_OBJECT_CLASS (accounts_window_parent_class)->finalize (obj);
}

static void
accounts_window_init (G_GNUC_UNUSED AccountsWindow* self)
{
  /* can't do anything here... we're waiting for a core :-/ */
}

static void
accounts_window_class_init (AccountsWindowClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->dispose = accounts_window_dispose;
  gobject_class->finalize = accounts_window_finalize;
}


/* Public API */
GtkWidget *
accounts_window_new (boost::shared_ptr<Ekiga::AccountCore> account_core,
		     boost::shared_ptr<Ekiga::PersonalDetails> details)
{
  AccountsWindow *self = NULL;

  boost::signals::connection conn;

  GtkWidget *vbox = NULL;
  GtkWidget *menu_bar = NULL;
  GtkWidget *menu_item = NULL;
  GtkWidget *menu = NULL;
  GtkWidget *item = NULL;
  GtkWidget *event_box = NULL;
  GtkWidget *scroll_window = NULL;
  GtkWidget* button_box = NULL;
  GtkWidget* button = NULL;

  GtkWidget *frame = NULL;
  GtkWidget *hbox = NULL;

  GtkCellRenderer *renderer = NULL;
  GtkListStore *list_store = NULL;
  GtkTreeViewColumn *column = NULL;

  GtkTreeSelection* selection = NULL;

  AtkObject *aobj;

  const gchar *column_names [] = {

    "",
    "",
    "",
    "",
    _("Account Name"),
    _("Status")
  };

  /* The window */
  self = (AccountsWindow *) g_object_new (ACCOUNTS_WINDOW_TYPE, NULL);

  self->priv = new AccountsWindowPrivate;
  self->priv->details = details;
  self->priv->account_core = account_core;

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
  gtk_window_set_title (GTK_WINDOW (self), _("Accounts"));

  /* The menu */
  menu_bar = gtk_menu_bar_new ();

  self->priv->accel = gtk_accel_group_new ();
  gtk_window_add_accel_group (GTK_WINDOW (self), self->priv->accel);
  g_object_unref (self->priv->accel);

  self->priv->menu_item_core = gtk_menu_item_new_with_mnemonic (_("_Accounts"));
  gtk_menu_shell_append (GTK_MENU_SHELL (menu_bar), self->priv->menu_item_core);
  g_object_ref (self->priv->menu_item_core);

  menu_item = gtk_menu_item_new_with_mnemonic (_("_Help"));
  gtk_menu_shell_append (GTK_MENU_SHELL (menu_bar), menu_item);

  menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), menu);
  item = gtk_image_menu_item_new_from_stock (GTK_STOCK_HELP, NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
  g_signal_connect (item, "activate", G_CALLBACK (help_callback), NULL);

  /* The accounts list store */
  list_store = gtk_list_store_new (COLUMN_ACCOUNT_NUMBER,
                                   G_TYPE_POINTER,
                                   G_TYPE_STRING,  /* Icon */
				   G_TYPE_BOOLEAN, /* Is account active? */
				   G_TYPE_INT,
				   G_TYPE_STRING,  /* Account Name */
				   G_TYPE_STRING,  /* Error Message */
				   G_TYPE_INT);    /* State */

  self->priv->accounts_list = gtk_tree_view_new_with_model (GTK_TREE_MODEL (list_store));
  g_object_unref (list_store);
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (self->priv->accounts_list), TRUE);
  gtk_tree_view_set_reorderable (GTK_TREE_VIEW (self->priv->accounts_list), TRUE);

  aobj = gtk_widget_get_accessible (GTK_WIDGET (self->priv->accounts_list));
  atk_object_set_name (aobj, _("Accounts"));

  renderer = gtk_cell_renderer_pixbuf_new ();
  column = gtk_tree_view_column_new ();
  g_object_set (renderer, "yalign", 0.5, "xpad", 5, NULL);
  gtk_tree_view_column_pack_start (column, renderer, FALSE);
  gtk_tree_view_column_add_attribute (column, renderer,
				      "icon-name", COLUMN_ACCOUNT_ICON);
  gtk_tree_view_append_column (GTK_TREE_VIEW (self->priv->accounts_list), column);

  /* Add all text renderers */
  for (int i = COLUMN_ACCOUNT_ACCOUNT_NAME ; i < COLUMN_ACCOUNT_NUMBER ; i++) {

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (column_names [i],
						       renderer,
						       "text",
						       i,
						       "weight",
						       COLUMN_ACCOUNT_WEIGHT,
						       NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (self->priv->accounts_list), column);
    gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN (column), TRUE);
    gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column),
				     GTK_TREE_VIEW_COLUMN_AUTOSIZE);
    gtk_tree_view_column_set_sort_column_id (column, i);
  }

  g_signal_connect (self->priv->accounts_list, "event_after",
		    G_CALLBACK (account_clicked_cb), self);

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (self->priv->accounts_list));
  gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);
  g_signal_connect (selection, "changed",
		    G_CALLBACK (on_selection_changed), self);

  /* The scrolled window with the accounts list store */
  scroll_window = gtk_scrolled_window_new (FALSE, FALSE);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_window),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_AUTOMATIC);

  event_box = gtk_event_box_new ();
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 12);
  gtk_container_add (GTK_CONTAINER (event_box), hbox);

  frame = gtk_frame_new (NULL);
  gtk_widget_set_size_request (GTK_WIDGET (frame), 250, 150);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 4);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_container_add (GTK_CONTAINER (frame), scroll_window);
  gtk_container_add (GTK_CONTAINER (scroll_window), self->priv->accounts_list);
  gtk_container_set_border_width (GTK_CONTAINER (self->priv->accounts_list), 0);
  gtk_box_pack_start (GTK_BOX (hbox), frame, TRUE, TRUE, 0);

  /* setting up a horizontal button box
   * (each button with be dynamically disabled/enabled as needed)
   */
  button_box = gtk_button_box_new (GTK_ORIENTATION_VERTICAL);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (button_box), GTK_BUTTONBOX_CENTER);
  button = gtk_button_new_with_mnemonic (_("_Enable"));
  gtk_box_pack_start (GTK_BOX (button_box), button, FALSE, FALSE, 3);
  self->priv->toolbar.add_button ("user-available", GTK_BUTTON (button));
  button = gtk_button_new_with_mnemonic (_("_Disable"));
  gtk_box_pack_start (GTK_BOX (button_box), button, FALSE, FALSE, 3);
  self->priv->toolbar.add_button ("user-offline", GTK_BUTTON (button));
  button = gtk_button_new_with_mnemonic (_("Edi_t"));
  gtk_box_pack_start (GTK_BOX (button_box), button, FALSE, FALSE, 3);
  self->priv->toolbar.add_button ("edit", GTK_BUTTON (button));
  button = gtk_button_new_with_mnemonic (_("_Remove"));
  gtk_box_pack_start (GTK_BOX (button_box), button, FALSE, FALSE, 3);
  self->priv->toolbar.add_button ("remove", GTK_BUTTON (button));
  gtk_box_pack_start (GTK_BOX (hbox), button_box, FALSE, FALSE, 10);

  populate_menu (GTK_WIDGET (self)); // This will add static and dynamic actions
  gtk_box_pack_start (GTK_BOX (vbox), menu_bar, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), event_box, TRUE, TRUE, 0);
  gtk_container_add (GTK_CONTAINER (self), vbox);
  gtk_widget_show_all (GTK_WIDGET (vbox));

  /* Engine Signals callbacks */
  conn = self->priv->account_core->bank_added.connect (boost::bind (&on_bank_added, _1, self));
  self->priv->connections.add (conn);
  conn = self->priv->account_core->account_added.connect (boost::bind (&on_account_added, _1, _2, self));
  self->priv->connections.add (conn);
  conn = self->priv->account_core->account_updated.connect (boost::bind (&on_account_updated, _1, _2, self));
  self->priv->connections.add (conn);
  conn = self->priv->account_core->account_removed.connect (boost::bind (&on_account_removed, _1, _2, self));
  self->priv->connections.add (conn);
  conn = self->priv->account_core->questions.connect (boost::bind (&on_handle_questions, _1, (gpointer) self));
  self->priv->connections.add (conn);

  self->priv->presence = self->priv->details->get_presence ();
  conn = self->priv->details->updated.connect (boost::bind (&on_personal_details_updated, self, self->priv->details));
  self->priv->connections.add (conn);

  self->priv->account_core->visit_banks (boost::bind (&on_visit_banks, _1, self));

  return GTK_WIDGET (self);
}
