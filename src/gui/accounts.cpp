
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

#include "accounts.h"

#include "account.h"
#include "bank.h"
#include "opal-bank.h"
#include "account-core.h"

#include "callbacks.h"
#include "misc.h"
#include "main.h"
#include "preferences.h"

#include "gmconf.h"
#include "gmdialog.h"

#include "services.h"
#include "menu-builder-tools.h"
#include "menu-builder-gtk.h"
#include "form-dialog-gtk.h"
#include "optional-buttons-gtk.h"

typedef struct GmAccountsWindow_ {

  GmAccountsWindow_ (Ekiga::ServiceCore & _core) : core (_core)
  {}

  GtkWidget *accounts_list;
  GtkWidget *menu_item_core;
  GtkAccelGroup *accel;

  Ekiga::ServiceCore &core;
  OptionalButtonsGtk toolbar;

} GmAccountsWindow;

#define GM_ACCOUNTS_WINDOW(x) (GmAccountsWindow *) (x)


/* GUI Functions */

/* DESCRIPTION  : /
 * BEHAVIOR     : Frees a GmAccountsWindow and its content.
 * PRE          : A non-NULL pointer to a GmAccountsWindow structure.
 */
static void gm_aw_destroy (gpointer aw);


/* DESCRIPTION  : /
 * BEHAVIOR     : Returns a pointer to the private GmAccountsWindow structure
 *                used by the preferences window GMObject.
 * PRE          : The given GtkWidget pointer must be a preferences window
 * 		  GMObject.
 */
static GmAccountsWindow *gm_aw_get_aw (GtkWidget *account_window);


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
  COLUMN_ACCOUNT_WEIGHT,
  COLUMN_ACCOUNT_ACCOUNT_NAME,
  COLUMN_ACCOUNT_STATUS,
  COLUMN_ACCOUNT_STATE,
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


/* Engine callbacks */

static void
populate_menu (GtkWidget *window)
{
  GmAccountsWindow *aw = NULL;

  MenuBuilderGtk builder;

  GtkWidget *item = NULL;
  GtkTreeSelection *selection = NULL;
  GtkTreeModel *model = NULL;
  GtkTreeIter iter;

  Ekiga::Account *account = NULL;

  aw = gm_aw_get_aw (GTK_WIDGET (window));

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (aw->accounts_list));
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (aw->accounts_list));

  gmref_ptr<Ekiga::AccountCore> account_core = aw->core.get ("account-core");

  if (account_core->populate_menu (builder)) {
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
  item = gtk_image_menu_item_new_from_stock (GTK_STOCK_CLOSE, aw->accel);
  gtk_menu_shell_append (GTK_MENU_SHELL (builder.menu), item);
  g_signal_connect_swapped (G_OBJECT (item), "activate",
                            G_CALLBACK (gtk_widget_hide),
                            (gpointer) window);

  gtk_menu_item_set_submenu (GTK_MENU_ITEM (aw->menu_item_core),
                             builder.menu);

  gtk_widget_show_all (builder.menu);
}


/* GTK+ Callbacks */
static gint
account_clicked_cb (G_GNUC_UNUSED GtkWidget *w,
		    GdkEventButton *event,
		    gpointer data)
{
  GmAccountsWindow *aw = NULL;

  GtkTreePath *path = NULL;
  GtkTreeView *tree_view = NULL;
  GtkTreeModel *model = NULL;
  GtkTreeIter iter;

  Ekiga::Account *account = NULL;

  aw = gm_aw_get_aw (GTK_WIDGET (data));

  tree_view = GTK_TREE_VIEW (aw->accounts_list);
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
        g_signal_connect (G_OBJECT (builder.menu), "hide",
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
  GmAccountsWindow* aw = (GmAccountsWindow*) data;
  GtkTreeSelection* selection = NULL;
  GtkTreeModel *model = NULL;
  GtkTreeIter iter;
  Ekiga::Account* account = NULL;

  g_return_if_fail (aw != NULL);

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (aw->accounts_list));

  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {

    gtk_tree_model_get (model, &iter,
			COLUMN_ACCOUNT, &account,
			-1);
    if (account) {

      aw->toolbar.reset ();
      account->populate_menu (aw->toolbar);
    } else {

      aw->toolbar.reset ();
    }

  } else {

    aw->toolbar.reset ();
  }
}

static void
gm_accounts_window_add_account (GtkWidget *window,
                                Ekiga::AccountPtr account)
{
  GmAccountsWindow *aw = NULL;
  GtkTreeModel *model = NULL;

  GtkTreeIter iter;

  g_return_if_fail (window != NULL);

  aw = gm_aw_get_aw (window);

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (aw->accounts_list));

  gtk_list_store_append (GTK_LIST_STORE (model), &iter);
  gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                      COLUMN_ACCOUNT, account.get (),
                      COLUMN_ACCOUNT_WEIGHT, PANGO_WEIGHT_NORMAL,
                      COLUMN_ACCOUNT_ACCOUNT_NAME, account->get_name ().c_str (),
                      -1);
}


void
gm_accounts_window_update_account (GtkWidget *accounts_window,
                                   Ekiga::AccountPtr account)
{
  GmAccountsWindow *aw = NULL;

  GtkTreeModel *model = NULL;
  GtkTreeSelection *selection = NULL;

  GtkTreeIter iter;
  Ekiga::Account *caccount = NULL;

  g_return_if_fail (accounts_window != NULL);

  aw = gm_aw_get_aw (accounts_window);

  /* on the one end we update the view */
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (aw->accounts_list));

  if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter)){

    do {

      gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
			  COLUMN_ACCOUNT, &caccount, -1);

      if (caccount == account.get ()) {

        gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                            COLUMN_ACCOUNT, account.get (),
                            COLUMN_ACCOUNT_WEIGHT, PANGO_WEIGHT_NORMAL,
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
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (aw->accounts_list));
  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {

    gtk_tree_model_get (model, &iter,
                        COLUMN_ACCOUNT, &caccount,
                        -1);

    if (caccount == account.get ()) {

      aw->toolbar.reset ();
      account->populate_menu (aw->toolbar);
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

  GmAccountsWindow *aw = NULL;

  g_return_if_fail (accounts_window != NULL);

  aw = gm_aw_get_aw (accounts_window);

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (aw->accounts_list));

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
  bank->visit_accounts (sigc::bind (sigc::ptr_fun (visit_accounts), data));
  populate_menu (GTK_WIDGET (data));
}


static bool
on_handle_questions (Ekiga::FormRequestPtr request,
                     gpointer data)
{
  FormDialog dialog (request, GTK_WIDGET (data));

  dialog.run ();

  return true;
}


GtkWidget *
gm_accounts_window_new (Ekiga::ServiceCore &core)
{
  GmAccountsWindow *aw = NULL;

  GtkWidget *window = NULL;

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
    _("Account Name"),
    _("Status"),
    ""
  };

  /* The window */
  window = gtk_dialog_new ();
  gtk_dialog_add_button (GTK_DIALOG (window), GTK_STOCK_CLOSE, GTK_RESPONSE_CANCEL);

  g_object_set_data_full (G_OBJECT (window), "window_name",
			  g_strdup ("accounts_window"), g_free);

  gtk_window_set_title (GTK_WINDOW (window), _("Accounts"));
  gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);

  aw = new GmAccountsWindow (core);
  g_object_set_data_full (G_OBJECT (window), "GMObject",
			  aw, gm_aw_destroy);

  /* The menu */
  menu_bar = gtk_menu_bar_new ();

  aw->accel = gtk_accel_group_new ();
  gtk_window_add_accel_group (GTK_WINDOW (window), aw->accel);
  g_object_unref (aw->accel);

  aw->menu_item_core = gtk_menu_item_new_with_mnemonic (_("_Accounts"));
  gtk_menu_shell_append (GTK_MENU_SHELL (menu_bar), aw->menu_item_core);
  g_object_ref (aw->menu_item_core);

  menu_item = gtk_menu_item_new_with_mnemonic (_("_Help"));
  gtk_menu_shell_append (GTK_MENU_SHELL (menu_bar), menu_item);

  menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), menu);
  item = gtk_image_menu_item_new_from_stock (GTK_STOCK_HELP, NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
  g_signal_connect (G_OBJECT (item), "activate", G_CALLBACK (help_cb), NULL);

  /* The accounts list store */
  list_store = gtk_list_store_new (COLUMN_ACCOUNT_NUMBER,
                                   G_TYPE_POINTER,
				   G_TYPE_INT,
				   G_TYPE_STRING,  /* Account Name */
				   G_TYPE_STRING,  /* Error Message */
				   G_TYPE_INT);    /* State */

  aw->accounts_list = gtk_tree_view_new_with_model (GTK_TREE_MODEL (list_store));
  g_object_unref (list_store);
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (aw->accounts_list), TRUE);
  gtk_tree_view_set_reorderable (GTK_TREE_VIEW (aw->accounts_list), TRUE);

  aobj = gtk_widget_get_accessible (GTK_WIDGET (aw->accounts_list));
  atk_object_set_name (aobj, _("Accounts"));

  /* Add all text renderers */
  for (int i = COLUMN_ACCOUNT_ACCOUNT_NAME ; i < COLUMN_ACCOUNT_NUMBER - 1 ; i++) {

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (column_names [i],
						       renderer,
						       "text",
						       i,
						       "weight",
						       COLUMN_ACCOUNT_WEIGHT,
						       NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (aw->accounts_list), column);
    gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN (column), TRUE);
    gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column),
				     GTK_TREE_VIEW_COLUMN_AUTOSIZE);
    gtk_tree_view_column_set_sort_column_id (column, i);
  }

  g_signal_connect (G_OBJECT (aw->accounts_list), "event_after",
		    G_CALLBACK (account_clicked_cb), window);

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (aw->accounts_list));
  gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);
  g_signal_connect (selection, "changed",
		    G_CALLBACK (on_selection_changed), aw);

  /* The scrolled window with the accounts list store */
  scroll_window = gtk_scrolled_window_new (FALSE, FALSE);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_window),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_AUTOMATIC);

  event_box = gtk_event_box_new ();
  hbox = gtk_hbox_new (FALSE, 4);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 12);
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

  /* setting up a horizontal button box
   * (each button with be dynamically disabled/enabled as needed)
   */
  button_box = gtk_hbutton_box_new ();
  button = gtk_button_new_with_label (_("Enable"));
  gtk_box_pack_start (GTK_BOX (button_box), button, FALSE, FALSE, 0);
  aw->toolbar.add_button ("enable", GTK_BUTTON (button));
  button = gtk_button_new_with_label (_("Disable"));
  gtk_box_pack_start (GTK_BOX (button_box), button, FALSE, FALSE, 0);
  aw->toolbar.add_button ("disable", GTK_BUTTON (button));
  button = gtk_button_new_with_label (_("Edit"));
  gtk_box_pack_start (GTK_BOX (button_box), button, FALSE, FALSE, 0);
  aw->toolbar.add_button ("edit", GTK_BUTTON (button));
  button = gtk_button_new_with_label (_("Remove"));
  gtk_box_pack_start (GTK_BOX (button_box), button, FALSE, FALSE, 0);
  aw->toolbar.add_button ("remove", GTK_BUTTON (button));

  populate_menu (window); // This will add static and dynamic actions
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->vbox), menu_bar, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->vbox), event_box, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->vbox), button_box, TRUE, TRUE, 0);

  /* Generic signals */
  g_signal_connect_swapped (GTK_OBJECT (window),
			    "response",
			    G_CALLBACK (gnomemeeting_window_hide),
			    (gpointer) window);

  g_signal_connect (GTK_OBJECT (window), "delete-event",
		    G_CALLBACK (delete_window_cb), NULL);

  gtk_widget_show_all (GTK_WIDGET (GTK_DIALOG (window)->vbox));


  /* Engine Signals callbacks */
  // FIXME sigc::connection conn;

  gmref_ptr<Ekiga::AccountCore> account_core = core.get ("account-core");
  account_core->bank_added.connect (sigc::bind (sigc::ptr_fun (on_bank_added), window));
  account_core->account_added.connect (sigc::bind (sigc::ptr_fun (on_account_added), window));
  account_core->account_updated.connect (sigc::bind (sigc::ptr_fun (on_account_updated), window));
  account_core->account_removed.connect (sigc::bind (sigc::ptr_fun (on_account_removed), window));
  account_core->questions.add_handler (sigc::bind (sigc::ptr_fun (on_handle_questions), (gpointer) window));

  account_core->visit_banks (sigc::bind_return (sigc::bind (sigc::ptr_fun (on_bank_added), window), true));

  return window;
}
