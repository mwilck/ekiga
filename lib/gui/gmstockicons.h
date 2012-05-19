
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2009 Damien Sandras <dsandras@seconix.com>
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
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Ekiga is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OPAL, OpenH323 and PWLIB, and distribute the combination,
 * without applying the requirements of the GNU GPL to the OPAL, OpenH323
 * and PWLIB programs, as long as you do follow the requirements of the
 * GNU GPL for all the rest of the software thus combined.
 */


/*
 *  Authors: Jorn Baayen <jorn@nl.linux.com>
 *           Kenneth Christiansen <kenneth@gnu.org>
 */

#ifndef __GM_STOCK_ICONS_H
#define __GM_STOCK_ICONS_H

#include "config.h"
#include <gtk/gtk.h>

G_BEGIN_DECLS

/* Icon theme builtins */
#define GM_ICON_AUDIO_VOLUME_HIGH          "audio-volume-high"
#define GM_ICON_LOGO                       PACKAGE_NAME
#define GM_ICON_ADDRESSBOOK                "stock_addressbook"

/* True stock icons */
#define GM_STOCK_MESSAGE          "gm_message_stock"

#define GM_STOCK_PHONE_HANG_UP_16    "gm_phone_hang_up_stock_16"
#define GM_STOCK_PHONE_PICK_UP_16    "gm_phone_pick_up_stock_16"
#define GM_STOCK_PHONE_HANG_UP_24    "gm_phone_up_stock_24"
#define GM_STOCK_PHONE_PICK_UP_24    "gm_phone_pick_up_stock_24"

void gnomemeeting_stock_icons_init (void);

G_END_DECLS

#endif /* __GM_STOCK_ICONS_H */
