
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2001 Damien Sandras
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
 */

/*
 *                         config.h  -  description
 *                         ------------------------
 *   begin                : Wed Feb 14 2001
 *   copyright            : (C) 2000-2001 by Damien Sandras
 *   description          : Functions to store the config options.
 *   email                : dsandras@seconix.com
 *
 */


#ifndef _CONFIG_H
#define _CONFIG_H

#include <gnome.h>
#include <ptlib.h>

#include <gconf/gconf-client.h>

#include "common.h"


/* The functions */

/* DESCRIPTION  :  /
 * BEHAVIOR     :  This function inits all the notifiers
 *                 that GnomeMeeting uses.
 * PRE          :  /
 */
void gnomemeeting_init_gconf (GConfClient *);


/* DESCRIPTION  :  This callback is called when the user changes
 *                 an entry
 * BEHAVIOR     :  Updates the gconf cache
 * PRE          :  data is the gconf key
 */
void entry_changed (GtkEditable  *, gpointer);


/* DESCRIPTION  :  This callback is called when the user changes               
 *                 the adjustment value                                        
 * BEHAVIOR     :  It updates the gconf cache                                  
 * PRE          :  /                                                           
 */
void adjustment_changed (GtkAdjustment  *, gpointer);


/* DESCRIPTION  :  This callback is called when the user changes               
 *                 the toggle value                                            
 * BEHAVIOR     :  It updates the gconf cache                                  
 * PRE          :  /                                                           
 */
void toggle_changed (GtkCheckButton  *, gpointer);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Store the configuration parameters from the 
 *                 options structure in the gnomemeeting config file.
 * PRE          :  /
 */
void gnomemeeting_store_config (options *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Read the configuration from the GnomeMeeting config file
 *                 and store it the options structure.
 * PRE          :  /
 */
void gnomemeeting_read_config (options *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Free the fields of the options struct
 * PRE          :  /
 */
void g_options_free (options *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Read the configuration from the preferences widgets 
 *                 and returns the corresponding options struct
 *                 !!!!!!! config in this structure should no be freed,
 *                 it contains pointers to the text fields of the widgets,
 *                 which will be destroyed with the pref window
 * PRE          :  /
 */
options *gnomemeeting_read_config_from_struct ();


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Read the configuration from the main GUI and from 
 *                 the ILS window.
 * PRE          :  /
 */
void gnomemeeting_read_config_from_gui (options *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Check if all required fields are correctly set in
 *                 the struct, returns FALSE if error.
 * PRE          :  /
 */
gboolean gnomemeeting_check_config_from_struct ();


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Return TRUE if GnomeMeeting is run for the first time and
 *                 needs a default config file.
 * PRE          :  /
 */
int gnomemeeting_config_first_time (void);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Create a default config file for first time execution.
 * PRE          :  /
 */
void gnomemeeting_init_config (void);

#endif
