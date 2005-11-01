
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
 *                         connectbutton.c  -  description
 *                         -------------------------------
 *   begin                : Tue Nov 01 2005
 *   copyright            : (C) 2000-2004 by Damien Sandras 
 *   description          : Contains a connectbutton widget 
 *
 */



#ifndef __GM_connect_button_H
#define __GM_connect_button_H

#include <glib-object.h>
#include <gtk/gtk.h>


G_BEGIN_DECLS

#define GM_CONNECT_BUTTON_TYPE (gm_connect_button_get_type ())
#define GM_CONNECT_BUTTON(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), GM_CONNECT_BUTTON_TYPE, GmConnectButton))
#define GM_CONNECT_BUTTON_CLASS(k) (G_TYPE_CHECK_CLASS_CAST((k), GM_CONNECT_BUTTON_TYPE, GmConnectButtonClass))
#define GM_IS_CONNECT_BUTTON(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), GM_CONNECT_BUTTON_TYPE))
#define GM_IS_CONNECT_BUTTON_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), GM_CONNECT_BUTTON_TYPE))
#define GM_CONNECT_BUTTON_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), GM_CONNECT_BUTTON_TYPE, GmConnectButtonClass))


typedef struct GmConnectButtonPrivate GmConnectButtonPrivate;


typedef struct
{
  GtkToggleButton parent;
  GtkWidget *image;
  gchar *connected_stock_id;
  gchar *disconnected_stock_id;
  
} GmConnectButton;


typedef struct
{
  GtkToggleButtonClass parent_class;
  
} GmConnectButtonClass;


/* The functions */

/* DESCRIPTION  :  /
 * BEHAVIOR     :  Returns the GType for the GmConnectButton.
 * PRE          :  /
 */
GType gm_connect_button_get_type (void);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Creates a new GmConnectButton.
 * PRE          :  The connect and disconnect stock icons.
 */
GtkWidget *gm_connect_button_new (const char *,
				  const char *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Update the connected state of the button.
 * PRE          :  First parameter must be != NULL.
 */
void gm_connect_button_set_connected (GmConnectButton *,
				      gboolean);


G_END_DECLS

#endif /* __GM_connect_button_H */
