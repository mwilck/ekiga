
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
 *                         gmgroupseditor.c  -  description
 *                         --------------------------------
 *   begin                : Wed Sep 27 2006
 *   copyright            : (C) 2006 by Jan Schampera
 *   description          : A GTK widget that allows editing of contact group definitions
 *                          like Ekiga uses it
 *   license              : GPL
 *
 */

/*!\file gmgroupseditor.c
 * \brief The GmGroupsEditor widget
 * \author Jan Schampera <jan.schampera@unix.net>
 * \date 2006
 * \ingroup GmGroupsEditor
 */

#include "gmgroupseditor.h"
#include "gmmarshallers.h"

#include <gtk/gtktreeview.h>
#include <gtk/gtkliststore.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkhseparator.h>
#include <gtk/gtklabel.h>
#include <gtk/gtk.h>
#include <gtk/gtkmenu.h>
#include <string.h>

#ifndef DISABLE_GNOME
/* for I18N */
#include <gnome.h>
#endif

#define GM_GROUPS_EDITOR_SPACING 2
#define GM_GROUPS_EDITOR_MSG_TMOUT 2000


#ifndef _
#ifdef DISABLE_GNOME
#include <libintl.h>
#define _(x) gettext(x)
#ifdef gettext_noop
#define N_(String) gettext_noop (String)
#else
#define N_(String) (String)
#endif /* gettext_noop */
#endif /* DISABLE_GNOME */
#endif /* _ */

enum {
  SIG_GROUP_DELETE_REQUEST,
  SIG_GROUP_RENAME_REQUEST,
  SIG__LAST
};

enum {
  COL_SELECTED,
  COL_GROUPNAME,
  COL_LAST
};

/*!\struct _GmGroupsEditorPrivate
 * \brief the GmGroupsEditor private data storage, inaccessible from outside
 * \see GmGroupsEditorPrivate
 * \see GmGroupsEditor
 * \see _GmGroupsEditor
 */
struct _GmGroupsEditorPrivate {
  GSList *selected_groups;
  /*!< stringlist of the currently selected groups */

  GSList *all_groups;
  /*!< stringlist of all the groups available */

  gchar *special_group;
  /*!< name of the "special group" */

  gchar *special_label;
  /*!< label for the selection toggle of the "special group" */

  GtkWidget *message_label;
  /*!< the messaging label */

  GtkWidget *main_vbox;
  /*!< the main container */

  GtkWidget *check_button;
  /*!< the check button for the "special group" */

  GtkWidget *newg_entry;
  /*!< the entry widget to add a new group */

  GtkWidget *newg_button;
  /*!< the button to add a new group */

  GtkWidget *tree_view;
  /*!< the tree view that shows the group list */

  GtkListStore *list_store;
  /*!< the group list */

  guint last_message_id;
  /*!< the GSource ID of the last message displayed - for destruction */
};

static guint gm_groups_editor_signals [SIG__LAST] = { 0 };
/*!< the array that holds the signal numbers */

/*!\fn gm_groups_editor_class_init (GmGroupsEditorClass *)
 * \brief class init function
 *
 * \param klass the GmGroupsEditorClass to init
 */
static void
gm_groups_editor_class_init (GmGroupsEditorClass *);

/*!\fn gm_groups_editor_init (GmGroupsEditor *)
 * \brief instance init function
 *
 * \param groups_editor the GmGroupsEditor to init
 */
static void
gm_groups_editor_init (GmGroupsEditor *);

/*!\fn gm_groups_editor_fill_selectlist (GmGroupsEditor *)
 * \brief initially fill the selection list
 *
 * Initially fills the selection list and toggles groups that are also
 * found in the editor's list for selected groups.
 * \param groups_editor the GmGroupsEditor to perform that
 */
static void
gm_groups_editor_fill_selectlist (GmGroupsEditor *);

/*!\fn gm_groups_editor_flash_message (GmGroupsEditor *, gchar *, guint);
 * \brief flash a message on the editor's message label
 *
 * Flashes a message on the editor's message label. The timeout is given in
 * ms. After the timeout, the message will be cleared.
 * \param groups_editor the GmGroupsEditor to perform that
 * \param message the message to flash
 * \param timeout timeout to clear the label
 */
static void
gm_groups_editor_flash_message (GmGroupsEditor *,
				gchar *,
				guint);

/*!\fn gm_groups_editor_clear_message_cb (gpointer)
 * \brief timeout callback to clear the editor's message label
 *
 * This is called by a timeout caller from GLib. It will clear the message.
 * The call to it is installed by #gm_groups_editor_flash_message.
 * \param data a #GmGroupsEditor as gpointer
 * \return always FALSE to indicate that the call should not be scheduled again
 * \see gm_groups_editor_flash_message
 */
static gboolean
gm_groups_editor_clear_message_cb (gpointer);

/*!\fn gm_groups_editor_list_button_event_cb (GtkWidget *, GdkEventButton *, gpointer)
 * \brief button event handler for the tree view
 *
 * This button event handler gets called on a button event over the tree view
 * that shows the groups. It will show a context menu.
 * \param tree_view the tree view that generated the event
 * \param event the #GdkEventButton structure
 * \param data a #GmGroupsEditor as gpointer
 * \see gm_groups_editor_show_list_contextmenu
 * \return TRUE if the event was handled, FALSE if not.
 */
static gboolean
gm_groups_editor_list_button_event_cb (GtkWidget *,
				       GdkEventButton *,
				       gpointer);

/*!\fn gm_groups_editor_list_delete_group (GmGroupsEditor *, const gchar *)
 * \brief delete a group from the list
 *
 * This function will delete a group from the list. It will not do something
 * related to the real existing group list in the contacts API!
 * \param groups_editor the GmGroupsEditor to perform that
 * \param groupname the name of the group to delete
 * \see gm_groups_editor_list_rename_group
 */
static void
gm_groups_editor_list_delete_group (GmGroupsEditor *,
				    const gchar *);
/*!\fn gm_groups_editor_list_rename_group (GmGroupsEditor *, const gchar *, const gchar *)
 * \brief rename a group in the list
 *
 * This function will rename a group in the list. It will not do something
 * related to the real existing group list in the contacts API!
 * \param groups_editor the GmGroupsEditor to perform that
 * \param from the name of the group to rename
 * \param to the new name of the group
 */
static void
gm_groups_editor_list_rename_group (GmGroupsEditor *,
				    const gchar *,
				    const gchar *);

/*!\fn gm_groups_editor_show_list_contextmenu (GmGroupsEditor *)
 * \brief show a list context menu
 *
 * This will show a context menu with the options to delete and rename a
 * group. The actual tree selection is used to get the group name.
 * \param groups_editor the GmGroupsEditor to perform that
 * \see gm_groups_editor_list_button_event_cb
 */
static void
gm_groups_editor_show_list_contextmenu (GmGroupsEditor *);

/*!\fn gm_groups_editor_list_menu_delete_cb (GtkMenuItem *, gpointer)
 * \brief menu callback to delete a group
 *
 * This will perform the whole delete-process: It displays a confirmation
 * dialog and sends the request signal.
 * \param menu_item the menu item the callback was connected to
 * \param data a #GmGroupsEditor as gpointer
 * \see gm_groups_editor_show_list_contextmenu
 */
static void
gm_groups_editor_list_menu_delete_cb (GtkMenuItem *,
                                      gpointer);

/*!\fn gm_groups_editor_list_menu_rename_cb (GtkMenuItem *, gpointer)
 * \brief menu callback to rename a group
 *
 * This will perform the whole rename-process: It displays a dialog to
 * rename and sends the request signal. It will do some slight sanity checks
 * on the new group name.
 * \param menu_item the menu item the callback was connected to
 * \param data a #GmGroupsEditor as gpointer
 * \see gm_groups_editor_show_list_contextmenu
 */
static void
gm_groups_editor_list_menu_rename_cb (GtkMenuItem *,
                                      gpointer);

/*!\fn gm_groups_editor_destroy (GtkObject *object)
 * \brief cleanup function for the widget's destruction
 *
 * \param object the #GmGroupsEditor as #GtkObject
 */
static void
gm_groups_editor_destroy (GtkObject *);

/*!\fn stringlist_contains (const GSList *, const gchar *)
 * \brief checks if a stringlist contains a specific string
 *
 * \param stringlist a #GSList* of #gchar*
 * \param string the string to check for
 * \return TRUE, if \p stringlist contains \p string, FALSE if not
 * \see stringlist_deep_copy
 * \see stringlist_remove
 * \see stringlist_add
 * \see stringlist_dump
 */
static gboolean
stringlist_contains (const GSList *,
		     const gchar *);

/*!\fn stringlist_deep_copy (const GSList *)
 * \brief copy a stringlist with all its data
 *
 * \param stringlist a #GSList* of #gchar*
 * \return the (new) generated stringlist
 * \see stringlist_contains
 * \see stringlist_remove
 * \see stringlist_add
 * \see stringlist_dump
 */
static GSList *
stringlist_deep_copy (const GSList *);

/*!\fn stringlist_remove (const GSList *, const gchar *)
 * \brief remove a string from a given stringlist
 *
 * \param stringlist a #GSList* of #gchar*
 * \param string the string to remove
 * \return the new list start
 * \see stringlist_contains
 * \see stringlist_deep_copy
 * \see stringlist_add
 * \see stringlist_dump
 */
static GSList *
stringlist_remove (const GSList *,
		   const gchar *);

/*!\fn stringlist_add (const GSList *, const gchar *)
 * \brief add a string to a given stringlist
 *
 * \param stringlist a #GSList* of #gchar*
 * \param string the string to add
 * \return the new list start
 * \see stringlist_contains
 * \see stringlist_deep_copy
 * \see stringlist_remove
 * \see stringlist_dump
 */
static GSList *
stringlist_add (const GSList *,
		const gchar *);

/*!\fn stringlist_dump (const GSList *, gchar)
 * \brief dump the values of a stringlist
 *
 * Dumps the several strings in \p stringlist into one string, separated
 * by \p separator. If \p stringlist is NULL, the returned string is an
 * empty string (""), not NULL.
 * \param stringlist a #GSList* of #gchar*
 * \param separator the separator character to use, must not be NUL
 * \return the generated string
 * \see stringlist_contains
 * \see stringlist_deep_copy
 * \see stringlist_remove
 * \see stringlist_add
 */
static gchar *
stringlist_dump (const GSList *,
		 gchar);

/*!\fn grouplist_toggled_cb (GtkCellRendererToggle *, gchar *, gpointer)
 * \brief called when a group the the list was toggled
 *
 * Adds and removes (depending on the toggle status) the toggled group
 * to/from the list of selected groups.
 * \param renderer the #GtkCellRendererToggle which produced that signal
 * \param path the string-path marking the position in the list
 * \param data a #GmGroupsEditor as gpointer
 */
static void
grouplist_toggled_cb (GtkCellRendererToggle *,
		      gchar *,
		      gpointer);

/*!\fn special_group_toggled_cb (GtkToggleButton *, gpointer)
 * \brief called, when the check button for the "special group" was toggled
 *
 * Adds and removes (depending on the toggle status) the "special group"
 * to/from the list of selected groups.
 * \param toggle_button the check-button (as toggle-button) that sent the signal
 * \param data a #GmGroupsEditor as gpointer
 */
static void
special_group_toggled_cb (GtkToggleButton *,
                          gpointer);

/*!\fn add_group_clicked_cb (GtkButton *, gpointer)
 * \brief called when the button to add a new group was clicked
 *
 * Will add - after some sanity checks - a new group to the list.
 * \param button the button that sent the signal
 * \param data a #GmGroupsEditor as gpointer
 * \see add_group_entry_activated_cb
 */
static void
add_group_clicked_cb (GtkButton *,
		      gpointer);

/*!\fn add_group_entry_activated_cb (GtkEntry *, gpointer)
 * \brief called when the entry for new groups was activated (RETURN pressed)
 *
 * Makes the button to add new groups send a "clicked" signal (will add a new group)
 * \param entry entry that sent the signal
 * \param data a #GmGroupsEditor as gpointer
 * \see add_group_clicked_cb
 */
static void
add_group_entry_activated_cb (GtkEntry *,
			      gpointer);

/* Implementation */

static void
grouplist_toggled_cb (GtkCellRendererToggle *renderer,
                      gchar *path,
                      gpointer data)
{
  GmGroupsEditor *groups_editor = NULL;
  GmGroupsEditorPrivate *priv = NULL;
  GtkListStore *store = NULL;
  GtkTreeIter iter;
  gboolean toggled = FALSE;
  gchar *group = NULL;

  g_return_if_fail (data != NULL);
  groups_editor = GM_GROUPS_EDITOR (data);
  g_return_if_fail (GM_IS_GROUPS_EDITOR (groups_editor));

  priv = groups_editor->priv;
  store = priv->list_store;

  if (!gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (store),
					    &iter,
					    path))
    return;

  gtk_tree_model_get (GTK_TREE_MODEL (store), &iter,
		      COL_SELECTED, &toggled,
		      COL_GROUPNAME, &group,
		      -1);

  toggled = !toggled;

  gtk_list_store_set (store, &iter,
		      COL_SELECTED, toggled,
		      -1);

  if (toggled)
    priv->selected_groups =
      stringlist_add (priv->selected_groups, group);
  else
    priv->selected_groups =
      stringlist_remove (priv->selected_groups, group);

  g_free (group);

}


static void
special_group_toggled_cb (GtkToggleButton *toggle_button,
			  gpointer data)
{
  GmGroupsEditor *groups_editor = NULL;
  GmGroupsEditorPrivate *priv = NULL;
  gchar *special_group = NULL;

  g_return_if_fail (data != NULL);
  groups_editor = GM_GROUPS_EDITOR (data);
  g_return_if_fail (GM_IS_GROUPS_EDITOR (groups_editor));

  priv = groups_editor->priv;

  special_group = g_strdup (priv->special_group);

  if (gtk_toggle_button_get_active (toggle_button))
    priv->selected_groups =
      stringlist_add (priv->selected_groups, special_group);
  else
    priv->selected_groups =
      stringlist_remove (priv->selected_groups, special_group);

  g_free (special_group);

}


static void
add_group_clicked_cb (GtkButton *button,
		      gpointer data)
{
  GmGroupsEditor *groups_editor = NULL;
  GmGroupsEditorPrivate *priv = NULL;
  GtkTreeIter iter;
  gchar *new_group = NULL;

  g_return_if_fail (data != NULL);
  groups_editor = GM_GROUPS_EDITOR (data);
  g_return_if_fail (GM_IS_GROUPS_EDITOR (groups_editor));

  priv = groups_editor->priv;

  if (stringlist_contains
      (priv->all_groups,
       gtk_entry_get_text (GTK_ENTRY (priv->newg_entry)))) {
    gtk_entry_set_text (GTK_ENTRY (priv->newg_entry), "");
    return;
  }

  new_group = g_strdup (gtk_entry_get_text (GTK_ENTRY (priv->newg_entry)));

  if (new_group &&
      !strcmp ((const char *) new_group,
	       "")) {
    g_free (new_group);
    return;
  }

  priv->all_groups =
    stringlist_add (priv->all_groups, new_group);

  priv->selected_groups =
    stringlist_add (priv->selected_groups, new_group);

  gtk_list_store_append (priv->list_store, &iter);
  gtk_list_store_set (priv->list_store, &iter,
		      COL_SELECTED, TRUE,
		      COL_GROUPNAME, (const gchar *) new_group,
		      -1);
  gtk_entry_set_text (GTK_ENTRY (priv->newg_entry), "");

  g_free (new_group);

}


static void
add_group_entry_activated_cb (GtkEntry *entry,
                              gpointer data)
{
  GmGroupsEditor *groups_editor = NULL;
  GmGroupsEditorPrivate *priv = NULL;

  g_return_if_fail (data != NULL);
  groups_editor = GM_GROUPS_EDITOR (data);
  g_return_if_fail (GM_IS_GROUPS_EDITOR (groups_editor));

  priv = groups_editor->priv;

  gtk_button_clicked (GTK_BUTTON (priv->newg_button));
}


/* STRINGLIST utility functions, quick hack (TM) by Jan
 * FIXME improvements?! */

static gboolean
stringlist_contains (const GSList *stringlist,
		     const gchar *string)
{
  GSList *stringlist_iter = NULL;

  g_return_val_if_fail (string != NULL, FALSE);

  if (!stringlist)
    return FALSE;

  stringlist_iter = (GSList *) stringlist;
  while (stringlist_iter)
    {
      if (stringlist_iter->data &&
	  !strcmp ((const char *) stringlist_iter->data,
		   (const char *) string))
	return TRUE;
      stringlist_iter = g_slist_next (stringlist_iter);
    }
  return FALSE;
}


static GSList *
stringlist_deep_copy (const GSList *stringlist)
{
  GSList *stringlist_iter = NULL;
  GSList *new_list = NULL;

  if (!stringlist)
    return NULL;

  stringlist_iter = (GSList *) stringlist;
  while (stringlist_iter)
    {
      new_list =
	g_slist_append (new_list,
			(gpointer) g_strdup
			((const gchar *) stringlist_iter->data));
      stringlist_iter = g_slist_next (stringlist_iter);
    }
  return new_list;
}


static GSList *
stringlist_remove (const GSList *list,
                   const gchar *string)
{
  GSList *new_list = NULL;
  GSList *list_iter = NULL;
  gboolean leave = FALSE;
  gboolean last_element = FALSE;

  if (!list)
    return NULL;
  if (!string)
    return (GSList *) list;

  if (!stringlist_contains (list, string))
    return (GSList *) list;

  new_list = (GSList *) list;

  while (!leave) {
    list_iter = new_list;
    last_element = (g_slist_length (list_iter) == 1);
    while (list_iter) {
      if (list_iter->data &&
	  !strcmp ((const char *) list_iter->data,
		   string)) {
	g_free (list_iter->data);
	new_list = g_slist_delete_link (new_list,
					list_iter);
	if (last_element) new_list = NULL;
	break;
      }
      list_iter = g_slist_next (list_iter);
    }
    if (!stringlist_contains (list, string))
      leave = TRUE;
  }

  return new_list;
}


static GSList *
stringlist_add (const GSList *list,
                const gchar *string)
{
  if (!string)
    return (GSList *) list;

  return
    g_slist_append ((GSList *) list,
		    (gpointer) g_strdup (string));
}


static gchar *
stringlist_dump (const GSList *list,
                 gchar separator)
{
  GSList *list_iter = NULL;
  gchar *tmpstr[2] = { NULL, NULL };

  g_return_val_if_fail (separator != '\0', NULL);

  if (!list)
    return g_strdup ("");

  list_iter = (GSList *) list;
  while (list_iter) {
    if (list_iter->data) {
      if (tmpstr[0]) {
	tmpstr[1] = tmpstr[0];
	tmpstr[0] = g_strdup_printf ("%s%c%s",
				     tmpstr[1],
				     separator,
				     (const gchar *) list_iter->data);
	g_free (tmpstr[1]);
      }
      else
	tmpstr[0] = g_strdup (list_iter->data);
    }
    list_iter = g_slist_next (list_iter);
  }
  return tmpstr[0];
}


GType
gm_groups_editor_get_type (void)
{
  static GType gm_groups_editor_type = 0;

  if (!gm_groups_editor_type)
    {
      static const GTypeInfo gm_groups_editor_info =
	{
	  sizeof (GmGroupsEditorClass),
	  NULL,
	  NULL,
	  (GClassInitFunc) gm_groups_editor_class_init,
	  NULL,
	  NULL,
	  sizeof (GmGroupsEditor),
	  0,
	  (GInstanceInitFunc) gm_groups_editor_init,
	};
      gm_groups_editor_type =
	g_type_register_static (GTK_TYPE_EXPANDER,
				"GmGroupsEditor",
				&gm_groups_editor_info, (GTypeFlags) 0);
    }
  return gm_groups_editor_type;
}


static void
gm_groups_editor_class_init (GmGroupsEditorClass *klass)
{
  GtkObjectClass *object_class = NULL;

  object_class = GTK_OBJECT_CLASS (klass);

  object_class->destroy = gm_groups_editor_destroy;

  gm_groups_editor_signals [SIG_GROUP_DELETE_REQUEST] =
    g_signal_new ("group-delete-request",
		  G_OBJECT_CLASS_TYPE (object_class),
		  G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
		  G_STRUCT_OFFSET (GmGroupsEditorClass, group_delete_request),
		  NULL, NULL,
		  gm_marshal_BOOLEAN__STRING,
		  G_TYPE_BOOLEAN, 1,
		  G_TYPE_STRING);

  gm_groups_editor_signals [SIG_GROUP_RENAME_REQUEST] =
    g_signal_new ("group-rename-request",
		  G_OBJECT_CLASS_TYPE (object_class),
		  G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
		  G_STRUCT_OFFSET (GmGroupsEditorClass, group_rename_request),
		  NULL, NULL,
		  gm_marshal_BOOLEAN__STRING_STRING,
		  G_TYPE_BOOLEAN, 2,
		  G_TYPE_STRING,
		  G_TYPE_STRING);
}


static void
gm_groups_editor_init (GmGroupsEditor *groups_editor)
{
  GmGroupsEditorPrivate *priv = NULL;
  GtkCellRenderer *renderer = NULL;
  GtkWidget *hbox = NULL;
  GtkWidget *hsep = NULL;
  GtkWidget *scrollwindow = NULL;

  priv = g_new (GmGroupsEditorPrivate, 1);
  groups_editor->priv = priv;

  priv->selected_groups = NULL;
  priv->all_groups = NULL;
  priv->special_group = NULL;
  priv->special_label = NULL;
  priv->message_label = NULL;
  priv->main_vbox = NULL;
  priv->check_button = NULL;
  priv->newg_entry = NULL;
  priv->newg_button = NULL;
  priv->tree_view = NULL;
  priv->list_store = NULL;

  /* the main vbox */
  priv->main_vbox = gtk_vbox_new (FALSE,
				  GM_GROUPS_EDITOR_SPACING);

  /* the separator at the bottom */
  hsep = gtk_hseparator_new ();
  gtk_box_pack_end (GTK_BOX (priv->main_vbox), hsep,
		    FALSE, FALSE, GM_GROUPS_EDITOR_SPACING);

  /* the messaging label below the list */
  priv->message_label = gtk_label_new ("");
  gtk_misc_set_alignment (GTK_MISC (priv->message_label), 0, 0.5);

  gtk_box_pack_end (GTK_BOX (priv->main_vbox), priv->message_label,
		    FALSE, FALSE, GM_GROUPS_EDITOR_SPACING);

  /* the selection list */
  priv->tree_view = gtk_tree_view_new ();
  scrollwindow = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrollwindow),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (scrollwindow),
		     priv->tree_view);

  gtk_widget_set_size_request (scrollwindow, -1, 200);

  renderer = gtk_cell_renderer_toggle_new ();
  gtk_tree_view_insert_column_with_attributes
    (GTK_TREE_VIEW (priv->tree_view),
     -1, "select", renderer,
     "active", COL_SELECTED,
     NULL);

  g_signal_connect (G_OBJECT (renderer),
                    "toggled",
                    (GCallback) grouplist_toggled_cb,
                    (gpointer) groups_editor);

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes
    (GTK_TREE_VIEW (priv->tree_view),
     -1, _("Group"), renderer,
     "text", COL_GROUPNAME,
     NULL);

  priv->list_store = gtk_list_store_new (COL_LAST,
                                         G_TYPE_BOOLEAN,
                                         G_TYPE_STRING);

  gtk_tree_view_set_model (GTK_TREE_VIEW (priv->tree_view),
                           GTK_TREE_MODEL (priv->list_store));

  gtk_box_pack_end (GTK_BOX (priv->main_vbox),
		    scrollwindow,
		    TRUE, TRUE,
		    GM_GROUPS_EDITOR_SPACING);

  g_signal_connect(G_OBJECT (priv->tree_view),
			     "button-press-event",
			     (GCallback) gm_groups_editor_list_button_event_cb,
			     (gpointer) groups_editor);

  /* the "add group" stuff */
  hbox = gtk_hbox_new (FALSE, GM_GROUPS_EDITOR_SPACING);
  priv->newg_entry = gtk_entry_new ();
  gtk_entry_set_width_chars (GTK_ENTRY (priv->newg_entry), 5);
  priv->newg_button = gtk_button_new_from_stock (GTK_STOCK_ADD);

  gtk_box_pack_start (GTK_BOX (hbox), priv->newg_entry,
		      TRUE, TRUE, GM_GROUPS_EDITOR_SPACING);

  gtk_box_pack_start (GTK_BOX (hbox), priv->newg_button,
		      FALSE, FALSE, GM_GROUPS_EDITOR_SPACING);

  g_signal_connect (G_OBJECT (priv->newg_entry),
		    "activate",
		    (GCallback) add_group_entry_activated_cb,
		    (gpointer) groups_editor);

  g_signal_connect (G_OBJECT (priv->newg_button),
		    "clicked",
		    (GCallback) add_group_clicked_cb,
		    (gpointer) groups_editor);

  gtk_box_pack_end (GTK_BOX (priv->main_vbox),
		    hbox,
		    FALSE, FALSE,
		    GM_GROUPS_EDITOR_SPACING);

  /* put all together into expander */
  gtk_container_add (GTK_CONTAINER (groups_editor),
		     priv->main_vbox);

  gtk_widget_show_all (GTK_WIDGET (priv->main_vbox));
}


GtkWidget *
gm_groups_editor_new (const gchar *label,
		      const GSList *selected,
		      const GSList *allgroups,
		      const gchar *specialgroup,
		      const gchar *speciallabel)
{
  GmGroupsEditor *groups_editor = NULL;
  GmGroupsEditorPrivate *priv = NULL;

  groups_editor = g_object_new (gm_groups_editor_get_type (), NULL);

  gtk_expander_set_label (GTK_EXPANDER (groups_editor),
			  label);

  gtk_container_set_resize_mode (GTK_CONTAINER (groups_editor),
				 GTK_RESIZE_PARENT);

  priv = groups_editor->priv;

  priv->selected_groups = stringlist_deep_copy (selected);
  priv->all_groups = stringlist_deep_copy (allgroups);
  priv->special_group = g_strdup (specialgroup);
  priv->special_label = g_strdup (speciallabel);

  /* if the "special" group is present, filter it from the main list
   * before filling the selection list FIXME?*/
  if (priv->special_group)
    priv->all_groups =
      stringlist_remove (priv->all_groups, priv->special_group);

  gm_groups_editor_fill_selectlist (groups_editor);

  /* add it again ...*/
  if (priv->special_group)
    priv->all_groups =
      stringlist_add (priv->all_groups, priv->special_group);

  if (priv->special_group) {
    priv->check_button =
      gtk_check_button_new_with_label (priv->special_label?
				       priv->special_label:
				       priv->special_group);
    gtk_box_pack_end (GTK_BOX (priv->main_vbox),
		      priv->check_button,
		      FALSE, FALSE,
		      GM_GROUPS_EDITOR_SPACING);

    /* activate the CheckButton, if the special group already was
     * in the "selected" list */
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->check_button),
				  stringlist_contains (priv->selected_groups,
						       (priv->special_group)));

    g_signal_connect (G_OBJECT (priv->check_button),
		      "toggled",
		      (GCallback) special_group_toggled_cb,
		      (gpointer) groups_editor);

    gtk_widget_show (GTK_WIDGET (priv->check_button));
  }

  return GTK_WIDGET (groups_editor);
}


static void
gm_groups_editor_fill_selectlist (GmGroupsEditor *groups_editor)
{
  GmGroupsEditorPrivate *priv = NULL;
  GtkTreeIter iter;
  GSList *a_list_iter = NULL;
  gchar *check_group = NULL;

  g_return_if_fail (groups_editor != NULL);
  g_return_if_fail (GM_IS_GROUPS_EDITOR (groups_editor));

  priv = groups_editor->priv;

  g_return_if_fail (priv->tree_view != NULL);
  g_return_if_fail (priv->list_store != NULL);

  if (!priv->selected_groups &&
      !priv->all_groups)
    return;

  /* fill all groups into the list, deselected */
  a_list_iter = priv->all_groups;
  while (a_list_iter) {
      if (a_list_iter->data) {
	gtk_list_store_append (priv->list_store, &iter);
	gtk_list_store_set (priv->list_store, &iter,
			    COL_SELECTED, FALSE,
			    COL_GROUPNAME,
			    g_strdup ((const gchar *) a_list_iter->data),
			    -1);
      }
      a_list_iter = g_slist_next (a_list_iter);
  }

  /* walk again through the list store and select all groups
   * that are in priv->selected_groups */
  if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (priv->list_store), &iter)) {
    do {
      gtk_tree_model_get (GTK_TREE_MODEL (priv->list_store), &iter,
			  COL_GROUPNAME, &check_group,
			  -1);

      if (stringlist_contains (priv->selected_groups,
			       check_group))
	gtk_list_store_set (priv->list_store, &iter,
			    COL_SELECTED, TRUE, -1);

      g_free (check_group);
    } while (gtk_tree_model_iter_next
	     (GTK_TREE_MODEL (priv->list_store), &iter));
  }
}


GSList *
gm_groups_editor_get_stringlist (GmGroupsEditor *groups_editor)
{
  GmGroupsEditorPrivate *priv = NULL;

  g_return_val_if_fail (groups_editor != NULL, NULL);
  g_return_val_if_fail (GM_IS_GROUPS_EDITOR (groups_editor), NULL);

  priv = groups_editor->priv;

  return stringlist_deep_copy (priv->selected_groups);
}

gchar *
gm_groups_editor_get_commalist (GmGroupsEditor *groups_editor)
{
  GmGroupsEditorPrivate *priv = NULL;

  g_return_val_if_fail (groups_editor != NULL, NULL);
  g_return_val_if_fail (GM_IS_GROUPS_EDITOR (groups_editor), NULL);

  priv = groups_editor->priv;

  return stringlist_dump (priv->selected_groups, ',');
}


static void
gm_groups_editor_flash_message (GmGroupsEditor *groups_editor,
				gchar *message,
				guint timeout)
{
  GmGroupsEditorPrivate *priv = NULL;

  g_return_if_fail (groups_editor != NULL);
  g_return_if_fail (GM_IS_GROUPS_EDITOR (groups_editor));

  priv = groups_editor->priv;

  gtk_label_set_text (GTK_LABEL (priv->message_label), message);

  priv->last_message_id =
    g_timeout_add (timeout,
		   (GSourceFunc) gm_groups_editor_clear_message_cb,
		   (gpointer) groups_editor);
}


static gboolean
gm_groups_editor_clear_message_cb (gpointer data)
{
  GmGroupsEditor *groups_editor = NULL;
  GmGroupsEditorPrivate *priv = NULL;

  g_return_val_if_fail (data != NULL, FALSE);
  groups_editor = GM_GROUPS_EDITOR (data);
  g_return_val_if_fail (GM_IS_GROUPS_EDITOR (groups_editor), FALSE);

  priv = groups_editor->priv;

  gtk_label_set_text (GTK_LABEL (priv->message_label), "");

  priv->last_message_id = 0;

  return FALSE;
}


static gboolean
gm_groups_editor_list_button_event_cb (GtkWidget *tree_view,
                                       GdkEventButton *event,
                                       gpointer data)
{
  GmGroupsEditor *groups_editor = NULL;
  GtkTreePath *path = NULL;
  GtkTreeSelection *selection = NULL;

  g_return_val_if_fail (data != NULL, FALSE);
  groups_editor = GM_GROUPS_EDITOR (data);
  g_return_val_if_fail (GM_IS_GROUPS_EDITOR (groups_editor), FALSE);

  if (event->type == GDK_BUTTON_PRESS  &&  event->button == 3) {
    /* select the row where the button was pressed */
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));
    if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(tree_view),
				      (gint) event->x,
				      (gint) event->y,
				      &path, NULL, NULL, NULL)) {
      gtk_tree_selection_unselect_all (selection);
      gtk_tree_selection_select_path (selection, path);
      gtk_tree_path_free (path);
      /* show nice (*vomit*) context menu */
      gm_groups_editor_show_list_contextmenu (groups_editor);
      return TRUE;
    }
  }

  return FALSE;
}


static void
gm_groups_editor_list_delete_group (GmGroupsEditor *groups_editor,
                                    const gchar *groupname)
{
  GtkTreeIter iter;
  GmGroupsEditorPrivate *priv = NULL;
  gchar *check_group = NULL;

  g_return_if_fail (groups_editor != NULL);
  g_return_if_fail (GM_IS_GROUPS_EDITOR (groups_editor));
  g_return_if_fail (groupname != NULL);

  priv = groups_editor->priv;

  if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (priv->list_store), &iter)) {
    do {
      gtk_tree_model_get (GTK_TREE_MODEL (priv->list_store), &iter,
                          COL_GROUPNAME, &check_group,
                          -1);
      if (check_group &&
	  !strcmp ((const char *) check_group,
		   (const char *) groupname)) {
	g_free (check_group);
	(void) gtk_list_store_remove (priv->list_store, &iter);
	break;
      }
      g_free (check_group);
    } while (gtk_tree_model_iter_next
             (GTK_TREE_MODEL (priv->list_store), &iter));
  }

  stringlist_remove (priv->selected_groups, groupname);
  stringlist_remove (priv->all_groups, groupname);
}


static void
gm_groups_editor_list_rename_group (GmGroupsEditor *groups_editor,
                                    const gchar *from,
                                    const gchar *to)
{
  GtkTreeIter iter;
  GmGroupsEditorPrivate *priv = NULL;
  gchar *check_group = NULL;
  gboolean selected = FALSE;

  g_return_if_fail (groups_editor != NULL);
  g_return_if_fail (GM_IS_GROUPS_EDITOR (groups_editor));
  g_return_if_fail (from != NULL || to != NULL);

  priv = groups_editor->priv;

  if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (priv->list_store), &iter)) {
    do {
      gtk_tree_model_get (GTK_TREE_MODEL (priv->list_store), &iter,
			  COL_GROUPNAME, &check_group,
			  COL_SELECTED, &selected,
			  -1);
      if (check_group &&
	  !strcmp ((const char *) check_group,
		   (const char *) from)) {
	g_free (check_group);
	gtk_list_store_set (priv->list_store, &iter,
			    COL_GROUPNAME, to,
			    -1);
	break;
      }
      g_free (check_group);
    } while (gtk_tree_model_iter_next
	     (GTK_TREE_MODEL (priv->list_store), &iter));
  }

  stringlist_remove (priv->selected_groups, from);
  stringlist_remove (priv->all_groups, from);

  if (selected)
    stringlist_add (priv->selected_groups, to);
  stringlist_add (priv->all_groups, to);
}

static void
gm_groups_editor_show_list_contextmenu (GmGroupsEditor *groups_editor)
{
  GtkWidget *popup_menu = NULL;
  GtkWidget *menu_item = NULL;

  g_return_if_fail (groups_editor != NULL);
  g_return_if_fail (GM_IS_GROUPS_EDITOR (groups_editor));

  /* build the menu and connect the callbacks */
  popup_menu = gtk_menu_new ();

  menu_item = gtk_menu_item_new_with_label (_("Rename"));
  gtk_menu_shell_append (GTK_MENU_SHELL (popup_menu),
			 menu_item);

  g_signal_connect (G_OBJECT (menu_item),
		    "activate",
		    (GCallback) gm_groups_editor_list_menu_rename_cb,
		    (gpointer) groups_editor);

  menu_item = gtk_menu_item_new_with_label (_("Delete"));
  gtk_menu_shell_append (GTK_MENU_SHELL (popup_menu),
			 menu_item);

  g_signal_connect (G_OBJECT (menu_item),
		    "activate",
		    (GCallback) gm_groups_editor_list_menu_delete_cb,
		    (gpointer) groups_editor);

  gtk_widget_show_all (popup_menu);

  gtk_menu_popup (GTK_MENU (popup_menu),
		  NULL, NULL, NULL, NULL,
		  0, gtk_get_current_event_time ());

  g_signal_connect (G_OBJECT (popup_menu),
		    "hide",
                   GTK_SIGNAL_FUNC (g_object_unref),
		   (gpointer) popup_menu);

  g_object_ref_sink ((gpointer) popup_menu);
}


static void
gm_groups_editor_list_menu_delete_cb (GtkMenuItem *menu_item,
				      gpointer data)
{
  GtkTreeSelection *selection;
  GtkTreeModel     *model;
  GtkTreeIter       iter;
  GmGroupsEditor *groups_editor = NULL;
  GmGroupsEditorPrivate *priv = NULL;
  GtkWidget *toplevel = NULL;
  GtkWidget *dialog = NULL;
  gchar *groupname = NULL;
  gboolean confirmed = FALSE;
  gboolean was_deleted = FALSE;

  g_return_if_fail (data != NULL);
  groups_editor = GM_GROUPS_EDITOR (data);
  g_return_if_fail (GM_IS_GROUPS_EDITOR (groups_editor));

  priv = groups_editor->priv;

  toplevel = gtk_widget_get_toplevel (GTK_WIDGET (groups_editor));

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->tree_view));

  if (gtk_tree_selection_get_selected(selection, &model, &iter))
  {
    gtk_tree_model_get (model, &iter,
			COL_GROUPNAME, &groupname,
			-1);
  }

  if (!groupname)
    return;

  dialog =
    gtk_message_dialog_new (GTK_WINDOW (toplevel),
                            GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                            GTK_MESSAGE_QUESTION,
                            GTK_BUTTONS_YES_NO,
                            _("Delete group %s?"),
                            groupname);

  confirmed =
    (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_YES);

  gtk_widget_destroy (dialog);

  if (!confirmed) {
    g_free (groupname);
    return;
  }

  g_signal_emit ((gpointer) groups_editor,
                 gm_groups_editor_signals [SIG_GROUP_DELETE_REQUEST],
                 0,
                 groupname,
                 &was_deleted);

  if (was_deleted) {
    gm_groups_editor_list_delete_group (groups_editor,
					groupname);
    gm_groups_editor_flash_message (groups_editor,
				    _("Group deleted."),
				    GM_GROUPS_EDITOR_MSG_TMOUT);
  }
  else
    gm_groups_editor_flash_message (groups_editor,
                                    _("Could not delete group!"),
                                    GM_GROUPS_EDITOR_MSG_TMOUT);

  g_free (groupname);
}


static void
gm_groups_editor_list_menu_rename_cb (GtkMenuItem *menu_item,
                                      gpointer data)
{
  GtkTreeSelection *selection;
  GtkTreeModel     *model;
  GtkTreeIter       iter;
  GmGroupsEditor *groups_editor = NULL;
  GmGroupsEditorPrivate *priv = NULL;
  GtkWidget *toplevel = NULL;
  GtkWidget *dialog = NULL;
  GtkWidget *name_entry = NULL;
  gchar *groupname = NULL;
  gchar *new_groupname = NULL;
  gboolean confirmed = FALSE;
  gboolean was_renamed = FALSE;

  g_return_if_fail (data != NULL);
  groups_editor = GM_GROUPS_EDITOR (data);
  g_return_if_fail (GM_IS_GROUPS_EDITOR (groups_editor));

  priv = groups_editor->priv;

  toplevel = gtk_widget_get_toplevel (GTK_WIDGET (groups_editor));

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->tree_view));

  if (gtk_tree_selection_get_selected(selection, &model, &iter))
  {
    gtk_tree_model_get (model, &iter,
                        COL_GROUPNAME, &groupname,
                        -1);
  }

  if (!groupname)
    return;

  dialog =
    gtk_dialog_new_with_buttons (_("Rename group"),
				 GTK_WINDOW (toplevel),
				 GTK_DIALOG_MODAL |
				 GTK_DIALOG_DESTROY_WITH_PARENT,
				 GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
				 GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
				 NULL);

  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);

  name_entry = gtk_entry_new ();
  gtk_entry_set_text (GTK_ENTRY (name_entry), groupname);
  gtk_editable_select_region (GTK_EDITABLE (name_entry), 0, -1);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), name_entry,
		      TRUE, TRUE, GM_GROUPS_EDITOR_SPACING);
  gtk_entry_set_activates_default (GTK_ENTRY (name_entry), TRUE);
  gtk_widget_show (name_entry);

  confirmed =
    (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT);

  new_groupname =
    g_strdup (gtk_entry_get_text (GTK_ENTRY (name_entry)));

  gtk_widget_destroy (dialog);

  if (!confirmed)
    {
      g_free (groupname);
      return;
    }

  /* little sanity check */
  if (groupname &&
      new_groupname &&
      (!strcmp ((const char *) groupname,
		(const char *) new_groupname) ||
       !strcmp ((const char *) new_groupname,
		""))) {
    g_free (groupname);
    g_free (new_groupname);
    return;
  }

  g_signal_emit ((gpointer) groups_editor,
                 gm_groups_editor_signals [SIG_GROUP_RENAME_REQUEST],
                 0,
                 groupname,
		 new_groupname,
                 &was_renamed);

  if (was_renamed) {
    gm_groups_editor_list_rename_group (groups_editor,
					groupname,
					new_groupname);
    gm_groups_editor_flash_message (groups_editor,
                                    _("Group renamed."),
                                    GM_GROUPS_EDITOR_MSG_TMOUT);
  }
  else
    gm_groups_editor_flash_message (groups_editor,
                                    _("Could not rename group!"),
                                    GM_GROUPS_EDITOR_MSG_TMOUT);

  g_free (groupname);
  g_free (new_groupname);
}


static void
gm_groups_editor_destroy (GtkObject *object)
{
  GmGroupsEditor *groups_editor = NULL;
  GmGroupsEditorPrivate *priv = NULL;
  GSource *msg_clearer_source = NULL;

  g_return_if_fail (object != NULL);
  groups_editor = GM_GROUPS_EDITOR (object);
  g_return_if_fail (GM_IS_GROUPS_EDITOR (groups_editor));

  priv = groups_editor->priv;

  if (priv->last_message_id)
    {
      /* a message is displayed and a message clear call is pending */
      msg_clearer_source =
	g_main_context_find_source_by_id (NULL, priv->last_message_id);
      if (msg_clearer_source)
	g_source_destroy (msg_clearer_source);
    }

  /* free all used data... */
  /* FIXME crash crash crash .... GNARG*/
//  g_free (priv->special_group);
//  g_free (priv->special_label);
/*
  if (priv->selected_groups) {
    g_slist_foreach (priv->selected_groups, (GFunc) g_free, NULL);
    g_slist_free (priv->selected_groups);
  }

  if (priv->all_groups) {
    g_slist_foreach (priv->all_groups, (GFunc) g_free, NULL);
    g_slist_free (priv->all_groups);
  }

  if (priv->list_store)
    gtk_list_store_clear (priv->list_store);
    */
}

