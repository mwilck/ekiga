
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
 *                         ldap_h.h  -  description
 *                         ------------------------
 *   begin                : Wed Feb 28 2001
 *   copyright            : (C) 2000-2001 by Damien Sandras
 *   description          : This file contains functions to build the ldap
 *                          window.
 *   email                : dsandras@seconix.com
 *
 */


#ifndef _LDAP_H_H_
#define _LDAP_H_H_

#include <lber.h>
#include <ldap.h>
#include <iostream.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <gnome.h>
#include <glib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <ptlib.h>

#include "common.h"


/* The functions  */

/* DESCRIPTION  :  /
 * BEHAVIOR     :  Build the LDAP window.
 * PRE          :  options* is valid
 */
void gnomemeeting_init_ldap_window (options *);

#endif
