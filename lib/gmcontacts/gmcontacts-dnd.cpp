
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
 *                         gmcontacts-dnd.c  -  description
 *                         ------------------------------------------
 *   begin                : July 2004
 *   copyright            : (C) 2004 by Julien Puydt
 *   description          : Drag'n drop of contacts made easy (implementation)
 *
 */

#include <string.h>
#include "gmcontacts.h"
#ifndef _GM_CONTACTS_H_INSIDE__
#define _GM_CONTACTS_H_INSIDE__
#include "gmcontacts-convert.h"
#undef _GM_CONTACTS_H_INSIDE__
#endif


/* 
 * the various contacts' formats supported by this code
 */


typedef enum {
  DND_GMCONTACT, /* the internal contact format */
  DND_VCARD, /* the vcard format, based on evolution-data-server's code */
  DND_NUMBER_OF_TARGETS /* keep it last */
} dnd_type;


static GtkTargetEntry dnd_targets [] =
{
    {"GmContact", GTK_TARGET_SAME_APP, DND_GMCONTACT},
    {"text/x-vcard", 0, DND_VCARD}
};


/*
 * Declaration of the gtk callbacks
 */


/* All of them expect a widget with either a "GmDnd-Source" or "GmDnd-Target"
 * data attached, containing the user-provided data when the widget was set up
 * as source/target, and a user_data pointer that is the user-defined callback
 * that manages only GmContact* contacts.
 *
 * That means they all check they get non-NULL widget & callbacks.
 * Additionally, those that get the info argument check that it is a
 * valid dnd_type.
 *
 * They are all gtk callbacks that translate the raw contact information to
 * a GmContact* contact information, then call the user-defined callbacks.
 */


static void drag_data_get_cb (GtkWidget *widget, 
			      GdkDragContext *context,
			      GtkSelectionData *data, 
			      guint info,
			      guint time,
			      gpointer user_data);


static void drag_data_received_cb (GtkWidget *widget,
				   GdkDragContext *context,
				   gint x,
				   gint y, 
				   GtkSelectionData *data,
				   guint info,
				   guint time,
				   gpointer user_data);


static gboolean drag_motion_cb (GtkWidget *widget,
				GdkDragContext *context,
				int x, 
				int y, 
				guint time, 
				gpointer user_data);


/* 
 * Implementation of the gtk callbacks
 */

static void
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
			    (const guchar *) &contact, sizeof (contact));
    break;
  case DND_VCARD:
    str = gmcontact_to_vcard (contact);
    if (str != NULL) {
      gtk_selection_data_set (data, GDK_TARGET_STRING, 8,
			      (const guchar *) str, strlen (str));
      g_free (str);
    }
    gmcontact_delete (contact);
    break;
  default:
    g_warning ("Oups! Dragging a %d-type contact not yet implemented!\n",
	       info);
  }
}


static void
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
    contact = vcard_to_gmcontact ((const gchar *) data->data);
    break;
  default:
    g_warning ("Oups! Dropping a %d-type contact not yet implemented!\n",
	       info);
    contact = NULL;
  }

  helper (widget, contact, x, y, additional_data);
}


static gboolean 
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


/* Implementation of the external api */


void
gmcontacts_dnd_set_source (GtkWidget *widget,
			    GmDndGetContact helper, 
			    gpointer data)
{
  g_return_if_fail (widget != NULL);
  g_return_if_fail (helper != NULL);

  gtk_drag_source_set (widget, GDK_BUTTON1_MASK,
                       dnd_targets, DND_NUMBER_OF_TARGETS,
                       GDK_ACTION_COPY);

  g_signal_connect (G_OBJECT (widget), "drag_data_get",
                    G_CALLBACK (drag_data_get_cb), (gpointer)helper);

  g_object_set_data (G_OBJECT (widget), "GmDnD-Source", data);
}


void
gmcontacts_dnd_set_dest (GtkWidget *widget, 
			  GmDndPutContact helper,
			  gpointer data)
{
  g_return_if_fail (widget != NULL);
  g_return_if_fail (helper != NULL);

  gtk_drag_dest_set (widget,
                     GTK_DEST_DEFAULT_ALL,
                     dnd_targets, DND_NUMBER_OF_TARGETS,
                     GDK_ACTION_COPY);
  
  g_signal_connect (G_OBJECT (widget), "drag_data_received",
                    G_CALLBACK (drag_data_received_cb), (gpointer)helper);

  g_object_set_data (G_OBJECT (widget), "GmDnD-Target", data);
}


void
gmcontacts_dnd_set_dest_conditional (GtkWidget *widget, 
				      GmDndPutContact helper,
				      GmDndAllowDrop checker,
				      gpointer data)
{
  g_return_if_fail (widget != NULL);
  g_return_if_fail (helper != NULL);
  g_return_if_fail (checker != NULL);

  gmcontacts_dnd_set_dest (widget, helper, data);

  g_signal_connect (G_OBJECT (widget), "drag_motion",
		    G_CALLBACK (drag_motion_cb), (gpointer)checker);
}
