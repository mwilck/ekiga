
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
 *                         gmgroupseditor.h  -  description
 *                         --------------------------------
 *   begin                : Wed Sep 27 2006
 *   copyright            : (C) 2006 by Jan Schampera
 *   description          : header file for a GTK widget that allows editing
 *                          of contact group definitions like Ekiga uses it
 *   license              : GPL
 *
 */

/*!\defgroup GmGroupsEditor
 */

/*!\file gmgroupseditor.h
 * \brief Header file for the GmGroupsEditor widget
 * \author Jan Schampera <jan.schampera@unix.net>
 * \date 2006
 * \ingroup GmGroupsEditor
 */

#include <glib.h>
#include <gtk/gtkexpander.h>
#include <gtk/gtkmarshal.h>

#ifndef __GM_GROUPS_EDITOR_H__
#define __GM_GROUPS_EDITOR_H__

G_BEGIN_DECLS

#define GM_GROUPS_EDITOR_TYPE             (gm_groups_editor_get_type())
#define GM_GROUPS_EDITOR(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GM_GROUPS_EDITOR_TYPE,GmGroupsEditor))
#define GM_GROUPS_EDITOR_CLASS(klass)     ((G_TYPE_CHECK_CLASS_CAST ((klass), GM_GROUPS_EDITOR_TYPE,GmGroupsEditorClass)))
#define GM_IS_GROUPS_EDITOR(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GM_GROUPS_EDITOR_TYPE))
#define GM_IS_GROUPS_EDITOR_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GM_GROUPS_EDITOR_TYPE))


/* API */

typedef struct _GmGroupsEditor GmGroupsEditor;

typedef struct _GmGroupsEditorClass GmGroupsEditorClass;

typedef struct _GmGroupsEditorPrivate GmGroupsEditorPrivate;

/*!\struct _GmGroupsEditor
 * \brief GmGroupsEditor Widget
 *
 * The GmGroupsEditor is a widget that is able, when given the right lists,
 * to let the user select and unselect groups, create new groups, delete or
 * existing ones. The renaming and deletion requests are not done by the
 * widget itself, they are passed with a signal. So this widget is more
 * independent from the rest of the API.
 * \ingroup GmGroupsEditor
 * \since 2.1.0
 * \see gm_groups_editor_get_type
 * \see gm_groups_editor_new
 * \see gm_groups_editor_get_stringlist
 * \see gm_groups_editor_get_commalist
 * \see GmGroupsEditorPrivate
 * \section gmgroupseditor_sigs Signals
 * \li \b group-delete-request
 * \see _GmGroupsEditorClass::group_delete_request
 * \li \b group-rename-request
 * \see GmGroupsEditorClass::group_rename_request
 */
struct _GmGroupsEditor
{
  GtkExpander expander;
  /*!< parent widget, a GtkExpander */

  GmGroupsEditorPrivate *priv;
  /*!< private data storage */
};

/*!\struct _GmGroupsEditorClass
 * \brief GmGroupsEditorClass class structure
 */
struct _GmGroupsEditorClass
{
  GtkExpanderClass parent_class;
  /*!< parent class, a GtkExpanderClass */

  gboolean (* group_delete_request) (GmGroupsEditor *groups_editor,
				     gchar *group);
  /*!< signal \t group-delete-request handler prototype
   *
   * \t group-delete-request is sent when the user chooses to delete a group
   * \param groups_editor the GmGroupsEditor which sent the signal
   * \param group the name of the group which was requested to be deleted
   * \return TRUE if the request could be performed, FALSE if not
   */

  gboolean (* group_rename_request) (GmGroupsEditor *groups_editor,
				     gchar *fromname,
				     gchar *toname);
  /*!< signal \t group-rename-request handler prototype
   *
   * \t group-rename-request is sent when the user chooses to rename a group
   * \param groups_editor the GmGroupsEditor which sent the signal
   * \param fromname the name of the group which was requested to be renamed
   * \param toname the new name
   * \return TRUE if the request could be performed, FALSE if not
   */
};


/*!\fn gm_groups_editor_get_type (void)
 * \brief retrieve the GType of the GmGroupsEditor
 *
 * Usually used by the macros or by GLib/GTK itself. Happiness for GTK/GLib.
 * \return the GType of GmGroupsEditor
 * \see GM_GROUPS_EDITOR_TYPE
 * \see GM_GROUPS_EDITOR
 * \see GM_GROUPS_EDITOR_CLASS
 * \see GM_IS_GROUPS_EDITOR
 * \see GM_IS_GROUPS_EDITOR_CLASS
 */
GType gm_groups_editor_get_type (void);


/*!\fn gm_groups_editor_new (const gchar *, const GSList *, const GSList *, const gchar *, const gchar *)
 * \brief returns a newly created GmGroupsEditor
 *
 * \param label the label to show on the editor's expander
 * \param selected a GSList* of gchar* of the selected groups (contact's groups)
 * \param allgroups a GSList* of gchar* of all groups available
 * \param specialgroup the name of a group that is extra selectable (roster)
 * \param speciallabel the label for the specialgroup
 * \return a GmGroupsEditor as GtkWidget
 */
GtkWidget *gm_groups_editor_new (const gchar *,
				 const GSList *,
				 const GSList *,
				 const gchar *,
				 const gchar *);


/*!\fn gm_groups_editor_get_stringlist (GmGroupsEditor *groups_editor)
 * \brief retrieve the actual list of selected groups as a GSList *
 *
 * \param groups_editor a GmGroupsEditor
 * \return the stringlist
 */
GSList *gm_groups_editor_get_stringlist (GmGroupsEditor *);


/*!\fn gm_groups_editor_get_commalist (GmGroupsEditor *groups_editor)
 * \brief retrieve the actual list of selected groups as a comma separated list
 *
 * Returns all selected groups as comma-separated GmContact like string. If no
 * groups are selected, the returned string is "" (empty string), not NULL.
 * \param groups_editor a GmGroupsEditor
 * \return the comma-separated selected groups in a string
 */
gchar *gm_groups_editor_get_commalist (GmGroupsEditor *);

G_END_DECLS

#endif /* __GM_GROUPS_EDITOR_H__ */

