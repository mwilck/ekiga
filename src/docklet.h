/***************************************************************************
                          docklet.h  -  description
                             -------------------
    begin                : Wed Oct 3 2001
    copyright            : (C) 2000-2001 by Damien Sandras & Miguel Rodríguez
    description          : This file contains all functions needed for
                           Gnome Panel docklet
    email                : migrax@terra.es, dsandras@seconix.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef _DOCKLET_H_
#define _DOCKLET_H_

#include <glib.h>
#include <gnome.h>

#include "config.h"


/******************************************************************************/
/* The GTK callbacks                                                          */
/******************************************************************************/

// DESCRIPTION  :  This callback is called when the user clicks
//                 on the docklet
// BEHAVIOR     :  If double clic : hide or show main window
// PRE          :  /
void docklet_clicked (GtkWidget *, GdkEventButton *, gpointer);


// DESCRIPTION  :  This callback is called when the user chooses
//                 to connect in the docklet menu
// BEHAVIOR     :  Answer incoming call or call somebody
// PRE          :  /
void docklet_connect (GtkWidget *, gpointer);


// DESCRIPTION  :  This callback is called when the user chooses
//                 to disconnect in the docklet menu
// BEHAVIOR     :  Refuse incoming call or stops current call
// PRE          :  /
void docklet_disconnect (GtkWidget *, gpointer);


// DESCRIPTION  :  This callback is called when the user chooses
//                 toggle in the docklet menu
// BEHAVIOR     :  Hide or show main window
// PRE          :  /
void docklet_toggle_callback (GtkWidget *, gpointer);
     

// DESCRIPTION  :  This callback is called by a timeout function
// BEHAVIOR     :  If current picture in the docklet is globe,
//                 then displays globe2, else displays globe
// PRE          :  /
gint docklet_flash (GtkWidget *);

/******************************************************************************/


/******************************************************************************/
/* Miscellaneous functions                                                    */
/******************************************************************************/

// DESCRIPTION  :  /
// BEHAVIOR     :  Init the docklet and menus and callbacks (for this docklet)
// PRE          :  /
GtkWidget *GM_docklet_init ();


// DESCRIPTION  :  /
// BEHAVIOR     :  If int = 1, displays globe2 and plays a sound, else displays
//                 globe
// PRE          :  /
void GM_docklet_set_content (GtkWidget *, int);

// DESCRIPTION  :  /
// BEHAVIOR     :  Show the docklet
// PRE          :  /
void GM_docklet_show (GtkWidget *);

// DESCRIPTION  :
// BEHAVIOR     : Hides the docklet window
// PRE          : /
void GM_docklet_hide (GtkWidget *);

/******************************************************************************/

#endif
