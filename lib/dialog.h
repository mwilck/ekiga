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

/**
 * gnomemeeting_warning_dialog_on_widget:
 *
 * @parent: The parent of the dialog
 * @widget: The widget to associate the warning with.
 * @format: String containing printf like syntax.
 * @ ...  : Variables of different kinds called from the 
 *          format line.
 *
 * Only shows a dialog if the users has not clicked on
 * 'Do not show this dialog again' when associated with 
 * the same widget. 
 *
 * This can be useful in certain situations. For instance
 * you might have a toggle button for a setting that is not
 * allowed to change while the app is in a certain state.
 * When you change the toggle it checks if the change is 
 * allowed or else it calls this function associating it  
 * with the toggle button. If the user chooses to ignore 
 * the dialog in the rest of the session, then this dialog 
 * will not popup with new calls to this function when 
 * associating with the same toggle button.
 *
 * This function only works in the current session.
 **/
void gnomemeeting_warning_dialog_on_widget (GtkWindow *parent, 
                                            GtkWidget *widget, 
                                            const char *format, ...);

/**
 * gnomemeeting_error_dialog:
 *
 * @parent is parent window.
 *
 * Constructs and shows an error dialog.
 **/
void gnomemeeting_error_dialog   (GtkWindow *parent, const char *format, ...);

/**
 * gnomemeeting_warning_dialog:
 *
 * @parent is parent window.
 *
 * Constructs and shows a warning dialog.
 **/
void gnomemeeting_warning_dialog (GtkWindow *parent, const char *format, ...);

/**
 * gnomemeeting_message_dialog:
 *
 * @parent is parent window.
 *
 * Constructs and shows a message dialog.
 **/
void gnomemeeting_message_dialog (GtkWindow *parent, const char *format, ...);

G_END_DECLS

#endif /* __GM_DIALOG_H */
