
/* Ekiga -- A Video-Conferencing application
 * Copyright (C) 2000-2006 Damien Sandras
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
 * Ekiga is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs Opal and Pwlib, and distribute the combination, without
 * applying the requirements of the GNU GPL to the Opal program, as long
 * as you do follow the requirements of the GNU GPL for all the rest of the
 * software thus combined.
 */


/*
 *                         statusicon.h  -  description
 *                         --------------------------
 *   begin                : Thu Jan 12 2006
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *                          (C) 2002 by Miguel Rodriguez
 *                          (C) 2006 by Julien Puydt
 *   description          : High level tray api interface
 */


#ifndef _STATUSICON_H_
#define _STATUSICON_H_

#include "common.h"
#include "manager.h"

G_BEGIN_DECLS

GtkWidget *gm_statusicon_new (void);

void gm_statusicon_update_full (GtkWidget *widget,
				GMManager::CallingState state,
				IncomingCallMode mode,
				gboolean forward_on_busy);


void gm_statusicon_update_menu (GtkWidget *widget,
				GMManager::CallingState state);

void gm_statusicon_signal_message (GtkWidget *widget,
				   gboolean has_message);

void gm_statusicon_ring (GtkWidget *widget,
			 guint interval);

void gm_statusicon_stop_ringing (GtkWidget *widget);

gboolean gm_statusicon_is_embedded (GtkWidget *widget);

G_END_DECLS

#endif
