
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

#include <gtk/gtk.h>

#include "pixmaps/inlines.h"
#include "gmstockicons.h"

/**
 * gnomemeeting_stock_icons_init:
 *
 * Initializes the GnomeMeeting stock icons
 *
 **/
void
gnomemeeting_stock_icons_init (void)
{
	int i;

        typedef struct
        {
                char *id;
                gint size;
                const guint8 *data;
        } GmThemeIcon;

	static const GmThemeIcon theme_builtins[] =
	{
	        { "audio-volume", 16, gm_audio_volume_16 },
		{ "brightness", 16, gm_brightness_16},
		{ "color", 16, gm_color_16},
		{ "contrast", 16, gm_contrast_16},
		{ "im-message", 16, gm_im_message_16},
		{ "im-message-new", 16, gm_im_message_new_16},
		{ "whiteness", 16, gm_whiteness_16},
	        { "video-settings", 16, gm_video_settings_16 }

	};

	/* Now install theme builtins */
	for (i = 0; i < (int) G_N_ELEMENTS (theme_builtins); i++)
	{
		GdkPixbuf *pixbuf;

		pixbuf = gdk_pixbuf_new_from_inline (-1, theme_builtins[i].data,
						     FALSE, NULL);

		gtk_icon_theme_add_builtin_icon (theme_builtins[i].id,
						 theme_builtins[i].size, pixbuf);

		g_object_unref (G_OBJECT (pixbuf));
	}
}
