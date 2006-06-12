
/* Ekiga -- A VoIP and Video-Conferencing application
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
#include <glib.h>
#include <stdio.h>
#include <string.h>

#include "../../pixmaps/inlines.h"
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
	GtkIconFactory *factory;
	int i;

        typedef struct 
        {
                char *id;
                const guint8 *data;
        } GmStockIcon;                

	static GmStockIcon items[] =
	{
	        { GM_STOCK_16,            gm_16_stock_data },
	        { GM_STOCK_ADDRESSBOOK_24, gm_addressbook_24_stock_data },
	        { GM_STOCK_ADDRESSBOOK_16, gm_addressbook_16_stock_data },
	        { GM_STOCK_AUDIO_VOLUME_MEDIUM, gm_audio_volume_medium_stock_data },
	        { GM_STOCK_COLOR_BRIGHTNESS_CONTRAST, gm_color_brightness_contrast_stock_data },
		{ GM_STOCK_TEXT_CHAT,     gm_text_chat_stock_data },
		{ GM_STOCK_CONTROL_PANEL, gm_control_panel_stock_data },
		{ GM_STOCK_CONNECT,       gm_connect_stock_data },
		{ GM_STOCK_DISCONNECT,    gm_disconnect_stock_data },
		{ GM_STOCK_EDIT,          gm_edit_stock_data },
		{ GM_STOCK_CAMERA_VIDEO,  gm_camera_video_stock_data },
		{ GM_STOCK_VOLUME,        gm_volume_stock_data },
		{ GM_STOCK_MICROPHONE,    gm_microphone_stock_data },
                { GM_STOCK_SPEAKER_PHONE, gm_speaker_phone_stock_data },
		{ GM_STOCK_MEDIA_PLAYBACK_PAUSE, gm_media_playback_pause_stock_data },
		{ GM_STOCK_STATUS_AVAILABLE, gm_status_available_stock_data },
		{ GM_STOCK_STATUS_RINGING,   gm_status_ringing_stock_data},
		{ GM_STOCK_STATUS_DO_NOT_DISTURB, gm_status_do_not_disturb_stock_data},
		{ GM_STOCK_STATUS_FORWARD, gm_status_forward_stock_data },
		{ GM_STOCK_STATUS_OFFLINE, gm_status_offline_stock_data },
		{ GM_STOCK_STATUS_AUTO_ANSWER, gm_status_auto_answer_stock_data },
		{ GM_STOCK_STATUS_IN_A_CALL, gm_status_in_a_call_stock_data },
	
		{ GM_STOCK_REMOTE_CONTACT, gm_remote_contact_stock_data},
		{ GM_STOCK_LOCAL_CONTACT, gm_local_contact_stock_data},
		{ GM_STOCK_WHITENESS, gm_whiteness_stock_data},
		{ GM_STOCK_BRIGHTNESS, gm_brightness_stock_data},
		{ GM_STOCK_COLOURNESS, gm_colourness_stock_data},
		{ GM_STOCK_CONTRAST, gm_contrast_stock_data},
		{ GM_STOCK_CONNECT_16, gm_connect_16_stock_data},
		{ GM_STOCK_DISCONNECT_16, gm_disconnect_16_stock_data},
		{ GM_STOCK_CALLS_HISTORY, gm_calls_history_stock_data},
		{ GM_STOCK_MESSAGE, gm_message_stock_data},
		{ GM_STOCK_FIND_CONTACT, gm_find_contact_stock_data},
		{ GM_STOCK_ADD_CONTACT, gm_add_contact_stock_data},
		{ GM_STOCK_CALL_PLACED, gm_call_placed_stock_data},
		{ GM_STOCK_CALL_MISSED, gm_call_missed_stock_data},
		{ GM_STOCK_CALL_RECEIVED, gm_call_received_stock_data},
	};

	factory = gtk_icon_factory_new ();
	gtk_icon_factory_add_default (factory);

	for (i = 0; i < (int) G_N_ELEMENTS (items); i++)
	{
		GtkIconSet *icon_set;
		GdkPixbuf *pixbuf;

                pixbuf = gdk_pixbuf_new_from_inline (-1, items[i].data, FALSE, NULL);

		icon_set = gtk_icon_set_new_from_pixbuf (pixbuf);

		gtk_icon_factory_add (factory, items[i].id, icon_set);
		gtk_icon_set_unref (icon_set);
		
		g_object_unref (G_OBJECT (pixbuf));
	}

	g_object_unref (G_OBJECT (factory));
}
