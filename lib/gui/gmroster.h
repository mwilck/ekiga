
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
 *                         gmroster.h  -  description 
 *                         ------------------------------------------
 *   begin                : Sun Mar 26 2006
 *   copyright            : (C) 2000-2006 by Jan Schampera, Damien Sandras
 *   description          : Implementation of a roster. 
 *
 */


/*!\file gmroster.h
 * \brief Header file for the GMRoster Ekiga GTK widget implementation
 * \author Jan Schampera <jan.schampera@unix.net>
 * \date 2006
 */

#ifndef __GM_ROSTER_H__
#define __GM_ROSTER_H__

#include <gmcontacts.h>

G_BEGIN_DECLS

#define GMROSTER_TYPE            (gmroster_get_type ())
#define GMROSTER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GMROSTER_TYPE, GMRoster))
#define GMROSTER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GMROSTER_TYPE, GMRosterClass))
#define IS_GMROSTER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GMROSTER_TYPE))
#define IS_GMROSTER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GMROSTER_TYPE))


/*!\typedef GMRoster
 * \brief type for the _GMRoster instance structure
 * \see _GMRoster
 */
typedef struct _GMRoster GMRoster;

/*!\typedef GMRosterClass
 * \brief type for the _GMRosterClass class structure
 * \see _GMRosterClass
 */
typedef struct _GMRosterClass GMRosterClass;

/*!\typedef _GMRosterPrivate
 * \brief type for the _GMRosterPrivate structure
 */
typedef struct _GMRosterPrivate GMRosterPrivate;

/*!\struct _GMRoster
 * \brief GMRoster instance structure
 * \see GMRoster
 */
struct _GMRoster
{
  GtkTreeView treeview;
  /*!< GtkTreeView of the GMRoster */

  GdkPixbuf * icons [CONTACT_LAST_STATE];
  /*!< icons corresponding to the contact status, default: all NULL */
  
  gchar* statustexts [CONTACT_LAST_STATE];
  /*!< status texts corresponding to the contact status, default: all NULL */

  gchar* unknown_group_name;
  /*!< the string to be used when a contact has no category set, default: "unknown" */

  gchar* roster_group;
  /*!< the string to be used to identify if a contact has to be displayed
   * at all, default "Roster" */

  gboolean show_offlines;
  /*!< indicator weather to show "offline" contacts or not, default: TRUE */
  
  gboolean show_groupless_contacts;
  /*!< indicator weather to show contacts without any category or not, default: TRUE */

  gboolean show_in_multiple_groups;
  /*!< indicator wheather to show a contact in all its groups or only the first, default: FALSE */

  GMRosterPrivate *privdata;
  /*!< GMRoster private data */
};


/*!\struct _GMRosterClass
 * \brief GMRoster class structure
 * \see GMRosterClass
 */
struct _GMRosterClass
{
  GtkTreeViewClass parent_class;
  /*!< the parent class of the GMRoster widget */
};


/*!\fn gmroster_get_type (void)
 * \brief retrieve the GType of the GMRoster
 * 
 * Usually used by the macros or by GLib/GTK itself
 * \see GMROSTER_TYPE
 * \see GMROSTER()
 * \see GMROSTER_CLASS()
 * \see IS_GMROSTER()
 * \see IS_GMROSTER_CLASS()
 */
GType gmroster_get_type (void);


/*!\fn gmroster_new (void)
 * \brief return a new instance of a GMRoster
 * \see GMRoster
 * \see _GMRoster
 */
GtkWidget *gmroster_new (void);


/*!\fn gmroster_set_gmconf_key (GMRoster *, const gchar *);
 * \brief sets the GmConf key, the roster saves its status to
 *
 * The key to use is a GmConf directory, the roster decides what to save in
 * which subkeys. Currently used:
 * \p roster_expanded_groups - a stringlist of expanded groups
 * \param roster pointer to a GMRoster, must not be NULL
 * \param new_gmkonf_key the GmConf key, or NULL to disable GmConf actions
 * \see gmroster_get_gmconf_key
 */
void gmroster_set_gmconf_key (GMRoster *,
			      const gchar *);

/*!\fn gmroster_get_gmconf_key (GMRoster *);
 * \brief gets a copy of the roster's GmConf key
 *
 * \param roster pointer to a GMRoster, must not be NULL
 * \returns the current used GmConf key, may be NULL
 * \see gmroster_set_gmconf_key
 */
gchar *gmroster_get_gmconf_key (GMRoster *);


/*!\fn gmroster_add_entry (GMRoster * roster, GmContact * contact)
 * \brief adds an entry 
 *
 * This function takes a contact and adds it to the roster's management system.
 * The roster holds its own copy of the contact, so it can be free'd afterwards.
 * \param roster pointer to a GMRoster, must not be NULL
 * \param contact a pointer to GmContact, must not be NULL
 * \see GMRoster
 */
void gmroster_add_entry (GMRoster *,
                         GmContact *);


/*!\fn gmroster_set_status_icon (GMRoster * roster, ContactState status, GdkPixbuf * pixbuf)
 * \brief sets the icon to be displayed for the a given status
 * 
 * Sets the picture displayed left of an entry corresponding to the entry status
 * (away, offline, ...). If the given pixbuf is NULL, then no image is 
 * displayed for that status.
 * \param roster a pointer to a GMRoster, must not be NULL
 * \param status a ContactState
 * \param stock a stock icon name
 * \see GMRoster
 * \see _GmContact::state
 */
void gmroster_set_status_icon (GMRoster *,
                               ContactState,
                               const gchar *);


/*!\fn gmroster_set_status_text (GMRoster * roster, ContactState status, gchar * statustext)
 * \brief sets the status text to be displayed for a given status
 *
 * This interface sets the text displayed for a status of an entry (away, offline, ...).
 * If the given string is NULL, then no informational text is displayed at all for that status.
 * \param roster a pointer to a GMRoster, must not be NULL
 * \param status a #ContactState
 * \param statustext a string or NULL
 * \see _GmContact::state
 */
void gmroster_set_status_text (GMRoster *,
                               ContactState,
                               gchar *);

/*!\fn gmroster_presence_set_status (GMRoster * roster, gchar * uri, ContactState status)
 * \brief re-sets the status for a given URI (and displays it if the URI is served, etc..)
 *
 * \param roster a pointer to a GMRoster, must not be NULL
 * \param uri the uri the status refers to, must not be NULL
 * \param status a #ContactState
 */
void gmroster_presence_set_status (GMRoster *,
				   gchar *,
				   ContactState);


/*!\fn gmroster_set_show_offlines (GMRoster * roster, gboolean show_offlines)
 * \brief sets if the roster should show "offline" contacts
 * 
 * By default, the roster displays offline contacts as "offline" (with all the corresponding icon
 * and status text).
 * This behaviour can be changed to not display offline contacts at all.
 * \param roster a pointer to a GMRoster, must not be NULL
 * \param show_offlines wheather to show or not show offline entries
 * \see gmroster_set_status_icon
 * \see gmroster_set_status_text
 * \see _GmContact::state
 */
void gmroster_set_show_offlines (GMRoster *,
                                 gboolean);


/*!\fn gmroster_get_show_offlines (GMRoster * roster)
 * \brief returns if the roster shows "offline" contacts
 * \param roster a pointer to a GMRoster, must not be NULL
 * \see gmroster_set_show_offlines
 */
gboolean gmroster_get_show_offlines (GMRoster *);


/*!\fn gmroster_set_show_in_multiple_groups (GMRoster * roster, gboolean show_in_multiple_groups)
 * \brief sets if contacts belonging to multiple groups should be shown 
 * like that
 *
 * A GmContact can belong to more than one group (categories). 
 * By default, the roster only displays the contact in the first group 
 * it finds, in that case. 
 * \param roster a pointer to a GMRoster, must not be NULL
 * \param show_in_multiple_groups whether to show or not in multiple groups
 * \see gmroster_get_show_in_multiple_groups
 * \see gmroster_set_roster_group
 * \see _GmContact::categories
 */
void gmroster_set_show_in_multiple_groups (GMRoster *,
                                           gboolean);


/*!\fn gmroster_get_show_in_multiple_groups (GMRoster * roster)
 * \brief returns if the roster shows contacts in multiple groups
 * \param roster a pointer to a GMRoster, must not be NULL
 * \see gmroster_set_show_in_multiple_groups
 * \see _GmContact::categories
 */
gboolean gmroster_get_show_in_multiple_groups (GMRoster *);


/*!\fn gmroster_set_unknown_group_name (GMRoster * roster, gchar * groupname)
 * \brief sets the "unknown" group name
 *
 * A GmContact doesn't necessarily have any groups (categories) configured. 
 * This function controls under which group name such a contact should be shown.
 * \param roster a pointer to a GMRoster, must not be NULL
 * \param groupname a string holding the name displayed for the "unknown" group, can be NULL
 * \see gmroster_get_unknown_group_name
 * \see gmroster_set_show_groupless_contacts
 * \see gmroster_get_show_groupless_contacts
 * \see _GmContact::categories
 */
void gmroster_set_unknown_group_name (GMRoster *,
                                      const gchar *);


/*!\fn gmroster_get_unknown_group_name (GMRoster * roster)
 * \brief returns the unknown group name (const!)
 * \param roster a pointer to a GMRoster, must not be NULL
 * \see gmroster_set_unknown_group_name
 * \see gmroster_set_show_groupless_contacts
 * \see gmroster_get_show_groupless_contacts
 */
const gchar *gmroster_get_unknown_group_name (GMRoster *);


/*!\fn gmroster_set_show_groupless_contacts (GMRoster * roster, gboolean show_groupless_contacts)
 * \brief sets, if the roster should show groupless contacts
 * \param roster a pointer to a GMRoster, must not be NULL
 * \param show_groupless_contacts  wheather to show or not show groupless contacts
 * \see gmroster_set_show_groupless_contacts
 * \see gmroster_set_unknown_group_name
 */
void gmroster_set_show_groupless_contacts (GMRoster *,
                                           gboolean);


/*!\fn gmroster_get_show_groupless_contacts (GMRoster * roster)
 * \brief returns, if the roster shows groupless contacts
 * \param roster a pointer to a GMRoster, must not be NULL
 * \see gmroster_set_show_groupless_contacts
 * \see gmroster_set_unknown_group_name
 */
gboolean gmroster_get_show_groupless_contacts (GMRoster *);


/*!\fn gmroster_sync_with_contacts (GMRoster *, GSList *)
 * \brief syncs the GMRoster with the given GSList* of GmContact*
 *
 * All entries are scanned for contacts to display (roster group).
 * \param roster a pointer to a GMRoster, must not be NULL
 * \param contacts a GSList* of GmContact*, must not be NULL
 * \see gmroster_set_roster_group
 * \see gmroster_get_roster_group
 */
void gmroster_sync_with_contacts (GMRoster *,
				  GSList *);


/*!\fn gmroster_set_roster_group (GMRoster * roster, gchar * rostergroup)
 * \brief sets the group name of the roster group
 *
 * The roster group is used by GMRoster to detect if a contact has to
 * be displayed or not. It's matched against the groups in
 * gmcontact->categories to detect that. GMRoster makes a private copy of
 * that string.
 * \param roster a pointer to a GMRoster, must not be NULL
 * \param rostergroup a gchar* to the roster group name
 * \see gmroster_get_roster_group
 */
void gmroster_set_roster_group (GMRoster *,
				const gchar *);


/*!\fn gmroster_get_roster_group (GMRoster * roster)
 * \brief returns a gchar* to the roster group name, or NULL if unset. Don't free!
 * \param roster a pointer to a GMRoster, must not be NULL
 */
const gchar *gmroster_get_roster_group (GMRoster *);


/*!\fn gmroster_get_selected_uri (GMRoster * roster)
 * \brief returns a copy(!) of the currently selectwed contact's UID or NULL if
 *  nothing is selected or a group is selected
 * \param roster a pointer to a GMRoster, must not be NULL
 */
gchar *gmroster_get_selected_uid (GMRoster *);

/*!\fn gmroster_get_selected_uri (GMRoster * roster)
 * \brief returns a copy(!) of the currently selectwed contact's URI or NULL if
 *  nothing is selected or a group is selected
 * \param roster a pointer to a GMRoster, must not be NULL
 */
gchar *gmroster_get_selected_uri (GMRoster *);

#endif


G_END_DECLS

