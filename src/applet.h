/***************************************************************************
                          applet.h  -  description
                             -------------------
    begin                : Mon Mar 19 2000
    copyright            : (C) 2000-2001 by Damien Sandras
    description          : This file contains all functions needed for
                           Gnome Panel applet
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


#ifndef _APPLET_H_
#define _APPLET_H_

#include <glib.h>
#include <gnome.h>
#include <applet-widget.h>

#include "config.h"


/******************************************************************************/
/* The GTK callbacks                                                          */
/******************************************************************************/

// DESCRIPTION  :  This callback is called when the user clicks
//                 on the applet
// BEHAVIOR     :  If double clic : hide or show main window
// PRE          :  /
void applet_clicked (GtkWidget *, GdkEventButton *, gpointer);


// DESCRIPTION  :  This callback is called when the user chooses
//                 to connect in the applet menu
// BEHAVIOR     :  Answer incoming call or call somebody
// PRE          :  /
void applet_connect (AppletWidget *, gpointer);


// DESCRIPTION  :  This callback is called when the user chooses
//                 to disconnect in the applet menu
// BEHAVIOR     :  Refuse incoming call or stops current call
// PRE          :  /
void applet_disconnect (AppletWidget *, gpointer);


// DESCRIPTION  :  This callback is called when the user chooses
//                 toggle in the applet menu
// BEHAVIOR     :  Hide or show main window
// PRE          :  /
void applet_toggle_callback (AppletWidget *, gpointer);
     

// DESCRIPTION  :  This callback is called by a timeout function
// BEHAVIOR     :  If current picture in applet the applet is globe,
//                 then displays globe2, else displays globe
// PRE          :  /
gint AppletFlash (GtkWidget *);

/******************************************************************************/


/******************************************************************************/
/* Miscellaneous functions                                                    */
/******************************************************************************/

// DESCRIPTION  :  /
// BEHAVIOR     :  Init the applet and menus and callbacks (for this applet)
// PRE          :  /
GtkWidget *GM_applet_init (int, char **);


// DESCRIPTION  :  /
// BEHAVIOR     :  If int = 1, displays globe2 and plays a sound, else displays
//                 globe
// PRE          :  /
void GM_applet_set_content (GtkWidget *, int);

/******************************************************************************/

#endif
