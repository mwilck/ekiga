/*  dialog.h
 *
 *  GnomeMeeting -- A Video-Conferencing application
 *  Copyright (C) 2000-2002 Damien Sandras
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  Started: Mon 17 June 2002, includes code from ?
 *
 *  Authors: Damien Sandras <dsandras@seconix.com>
 *           Jorn Baayen <jorn@nl.linux.com>
 *           Kenneth Christiansen <kenneth@gnu.org>
 */

#ifndef __GM_DIALOG_H
#define __GM_DIALOG_H

#include <stdarg.h>

G_BEGIN_DECLS

void gnomemeeting_warning_dialog_on_widget (GtkWindow *parent, 
                                            GtkWidget *widget, 
                                            const char *format, ...);
void gnomemeeting_error_dialog   (GtkWindow *parent, const char *format, ...);
void gnomemeeting_warning_dialog (GtkWindow *parent, const char *format, ...);
void gnomemeeting_message_dialog (GtkWindow *parent, const char *format, ...);

G_END_DECLS

#endif /* __GM_DIALOG_H */
