
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2003 Damien Sandras
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
 *                         config.h  -  description
 *                         ------------------------
 *   begin                : Wed Feb 14 2001
 *   copyright            : (C) 2000-2003 by Damien Sandras 
 *   description          : This file contains most of gconf stuff.
 *                          All notifiers are here.
 *                          Callbacks that updates the gconf cache 
 *                          are in their file, except some generic one that
 *                          are in this file.
 *   Additional code      : Miguel Rodríguez Pérez  <migrax@terra.es>
 *
 */


#ifndef _CONFIG_H
#define _CONFIG_H

#include "common.h"


/* The functions */

/* DESCRIPTION  :  /
 * BEHAVIOR     :  This function inits all the notifiers
 *                 that GnomeMeeting uses.
 * PRE          :  /
 */
void gnomemeeting_init_gconf (GConfClient *);


/* DESCRIPTION  :  This function is called when an entry changes.
 * BEHAVIOR     :  Updates the key given as parameter to the new value of the
 *                 entry.  
 * PRE          :  Non-Null data corresponding to the gconf key to modify.
 */
void entry_changed (GtkEditable  *, gpointer);


/* DESCRIPTION  :  This function is called when an adjustment changes.
 * BEHAVIOR     :  Updates the key given as parameter to the new value of the
 *                 adjustment.  
 * PRE          :  Non-Null data corresponding to the gconf key to modify.
 */
void adjustment_changed (GtkAdjustment *, gpointer);


/* DESCRIPTION  :  This function is called when a toggle changes.
 * BEHAVIOR     :  Updates the key given as parameter to the new value of the
 *                 toggle.  
 * PRE          :  Non-Null data corresponding to the gconf key to modify.
 */
void toggle_changed (GtkCheckButton *, gpointer);


/* DESCRIPTION  :  This function is called when an int option menu changes.
 * BEHAVIOR     :  Updates the key given as parameter to the new value of the
 *                 int option menu.  
 * PRE          :  Non-Null data corresponding to the gconf key to modify.
 */
void int_option_menu_changed (GtkWidget *, gpointer);


/* DESCRIPTION  :  This function is called when a string option menu changes.
 * BEHAVIOR     :  Updates the key given as parameter to the new value of the
 *                 string option menu.  
 * PRE          :  Non-Null data corresponding to the gconf key to modify.
 */
void string_option_menu_changed (GtkWidget *, gpointer);

#endif
