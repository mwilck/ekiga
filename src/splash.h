/***************************************************************************
                          splash.h  -  description
                             -------------------
    begin                : Mon Mar 19 2001
    copyright            : (C) 2000-2001 by Damien Sandras
    description          : to display a splash screen with the logo at startup
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

#ifndef _SPLASH_H_
#define _SPLASH_H_


#include <glib.h>
#include <gnome.h>
#include <sys/time.h>


/******************************************************************************/
/* The functions                                                              */
/******************************************************************************/

// DESCRIPTION  :  /
// BEHAVIOR     :  Initialize the splash screen with the logo
// PRE          :  /
GtkWidget *GM_splash_init (void);


// DESCRIPTION  :  /
// BEHAVIOR     :  Sets the splash window to gfloat % completed with the char * text
// PRE          :  1 <= gfloat <= 0, char * not empty, GtkWidget * not Null
void GM_splash_advance_progress(GtkWidget *, char *, gfloat);

/******************************************************************************/

#endif
