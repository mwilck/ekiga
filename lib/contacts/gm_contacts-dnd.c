
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
 *                         gm_contacts-dnd.c  -  description
 *                         ------------------------------------------
 *   begin                : July 2004
 *   copyright            : (C) 2004 by Julien Puydt
 *   description          : Drag'n drop of contacts made easy (implementation)
 *
 */

#include <string.h>
#include "gm_contacts.h"
#ifndef _GM_CONTACTS_H_INSIDE__
#define _GM_CONTACTS_H_INSIDE__
#include "gm_contacts-dnd.h"
#include "gm_contacts-convert.h"
#undef _GM_CONTACTS_H_INSIDE__
#endif
/* 
 * Implementation of the gtk callbacks that will do the real work
 */

void
drag_data_get_cb (GtkWidget *widget, 
		  GdkDragContext *context,
                  GtkSelectionData *data, 
		  guint info,
		  guint time,
                  gpointer user_data)
{
  GmDndGetContact helper = NULL;
  GmContact *contact = NULL;
  gpointer additional_data = NULL;
  gchar *str = NULL;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (user_data != NULL);
  g_return_if_fail (info >= DND_GMCONTACT && info < DND_NUMBER_OF_TARGETS);

  helper = (GmDndGetContact)user_data;
  additional_data = g_object_get_data (G_OBJECT (widget), "GmDnD-Source");
  contact = helper (widget, additional_data);

  switch (info) {
  case DND_GMCONTACT:
    gtk_selection_data_set (data, GDK_TARGET_STRING, 8,
			    (const guchar *)&contact, sizeof (contact));
    break;
  case DND_VCARD:
    str = gmcontact_to_vcard (contact);
    if (str != NULL) {
      gtk_selection_data_set (data, GDK_TARGET_STRING, 8,
			      str, strlen (str));
      g_free (str);
    }
    gm_contact_delete (contact);
    break;
  default:
    g_warning ("Oups! Dragging a %d-type contact not yet implemented!\n",
	       info);
  }
}


void
drag_data_received_cb (GtkWidget *widget,
		       GdkDragContext *context,
                       gint x,
		       gint y,
                       GtkSelectionData *data,
		       guint info, 
		       guint time,
                       gpointer user_data)
{
  GmDndPutContact helper = NULL;
  GmContact *contact = NULL;
  gpointer additional_data = NULL;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (user_data != NULL);
  g_return_if_fail (info >= DND_GMCONTACT && info < DND_NUMBER_OF_TARGETS);

  helper = (GmDndPutContact)user_data;
  additional_data = g_object_get_data (G_OBJECT (widget), "GmDnD-Target");

  switch (info) {
  case DND_GMCONTACT:
    contact = *(GmContact **)data->data;
    break;
  case DND_VCARD:
    contact = vcard_to_gmcontact (data->data);
    break;
  default:
    g_warning ("Oups! Dropping a %d-type contact not yet implemented!\n",
	       info);
    contact = NULL;
  }

  helper (widget, contact, x, y, additional_data);
}


gboolean 
drag_motion_cb (GtkWidget *widget,
		GdkDragContext *context,
		int x,
		int y,
		guint time,
		gpointer user_data)
{
  GmDndAllowDrop checker = NULL;
  gpointer additional_data = NULL;

  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (user_data != NULL, FALSE);
  
  checker = (GmDndAllowDrop)user_data;
  additional_data = g_object_get_data (G_OBJECT (widget), "GmDnD-Target");
  
  return checker (widget, x, y, additional_data);
}
