/***************************************************************************
                          config.h  -  description
                             -------------------
    begin                : Wed Feb 14 2001
    copyright            : (C) 2001 by Damien Sandras
    description          : Functions to store the config options
    email                : dsandras@acm.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _CONFIG_H
#define _CONFIG_H

#include <gnome.h>
#include <ptlib.h>

#include "common.h"


/******************************************************************************/
/* The functions                                                              */
/******************************************************************************/

// DESCRIPTION  :  /
// BEHAVIOR     :  Store the configuration parameters from the options structure
//                 in the gnomemeeting config file 
// PRE          :  valid options *
void store_config (options *);


// DESCRIPTION  :  /
// BEHAVIOR     :  Read the configuration from the GnomeMeeting config file
//                 and store it the options structure
// PRE          :  valid options * (must be allocated)
void read_config (options *);


// DESCRIPTION  :  /
// BEHAVIOR     :  Free the fields of the options struct
// PRE          :  valid options * 
void g_options_free (options *);


// DESCRIPTION  :  /
// BEHAVIOR     :  Read the configuration from the preferences widgets and returns
//                 the corresponding options struct
//                 !!!!!!! config in this structure should no be freed,
//                 it contains pointers to the text fields of the widgets,
//                 which will be destroyed with the pref window
// PRE          :  valid GM_pref_window_widgets
options * read_config_from_struct (GM_pref_window_widgets *);


// DESCRIPTION  :  /
// BEHAVIOR     :  Read the configuration from the main GUI and from the ILS window
//                 !!!!!!! config in this structure should no be freed,
//                 it contains pointers to the text fields of the widgets,
//                 which will be destroyed with the pref window
// PRE          :  valid GM_window_widgets, and GM_ldap_window_widgets, and options
void read_config_from_gui (GM_window_widgets *, GM_ldap_window_widgets *, options *);


// DESCRIPTION  :  /
// BEHAVIOR     :  Checks if all required fields are correctly set in
//                 the struct, returns FALSE if error 
// PRE          :  valid GM_pref_window_widgets
gboolean check_config_from_struct (GM_pref_window_widgets *);


// DESCRIPTION  :  /
// BEHAVIOR     :  Returns TRUE if GnomeMeeting is run for the first time and
//                 needs a default config file
// PRE          :  /
int config_first_time (void);


// DESCRIPTION  :  /
// BEHAVIOR     :  Creates a default config file for first time execution
// PRE          :  /
void init_config (void);

/******************************************************************************/    
#endif
