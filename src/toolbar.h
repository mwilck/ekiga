/***************************************************************************
                          toolbar.h  -  description
                             -------------------
    begin                : Sat Dec 23 2000
    copyright            : (C) 2000 by Damien Sandras
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

#ifndef _TOOLBAR_H_
#define _TOOLBAR_H_

#include <gnome.h>
#include "common.h"

void GM_toolbar_init (GtkWidget *, GM_window_widgets*);
void enable_connect ();
void disable_connect ();
void enable_disconnect ();
void disable_disconnect ();

#endif
