
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2005 Damien Sandras
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
 * programs Opal and Pwlib, and distribute the combination, without
 * applying the requirements of the GNU GPL to the OpenH323 program, as long
 * as you do follow the requirements of the GNU GPL for all the rest of the
 * software thus combined.
 */


/*
 *                         winpaths.h  -  description 
 *                         ------------------------------------------
 *   begin                : Dec 2005
 *   copyright            : (C) 2005 by Julien Puydt
 *   description          : Helper defines for paths on win32
 */

#include <glib.h>

G_BEGIN_DECLS

const gchar *win32_sysconfdir (void);
const gchar *win32_datadir (void);

#undef SYSCONFDIR
#define SYSCONFDIR win32_sysconfdir()

#undef DATA_DIR
#define DATA_DIR win32_datadir()

G_END_DECLS
