
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
 *                         misc.cpp  -  description
 *                         ------------------------
 *   begin                : Thu Nov 22 2001
 *   copyright            : (C) 2000-2001 by Damien Sandras
 *   description          : This file contains miscellaneous functions.
 *   email                : dsandras@seconix.com
 *
 */


#include <ptlib.h>
#include <gnome.h>

#include "misc.h"

#include "../pixmaps/text_logo.xpm"


/* Declarations */
extern GtkWidget *gm;


/* The functions */
void gnomemeeting_threads_enter () {


  if (PThread::Current ()->GetThreadName () != "gnomemeeting") {
    
    PTRACE(3, "Will Take GDK Lock");
    gdk_threads_enter ();
    PTRACE(3, "GDK Lock Taken");
  }
  else {

    PTRACE(3, "Ignore GDK Lock request : Main Thread");
  }
    
}


void gnomemeeting_threads_leave () {

  if (PThread::Current ()->GetThreadName () != "gnomemeeting") {

    PTRACE(3, "Will Release GDK Lock");
    gdk_threads_leave ();
    PTRACE(3, "GDK Lock Released");
  }
  else {

    PTRACE(3, "Ignore GDK UnLock request : Main Thread");
  }
    
}


GtkWidget *gnomemeeting_button (gchar *lbl, GtkWidget *pixmap)
{
  GtkWidget *button;
  GtkWidget *hbox2;
  GtkWidget *label;
  
  button = gtk_button_new ();
  label = gtk_label_new (N_(lbl));
  hbox2 = gtk_hbox_new (FALSE, 0);

  gtk_box_pack_start(GTK_BOX (hbox2), pixmap, TRUE, TRUE, GNOME_PAD_SMALL);  
  gtk_box_pack_start(GTK_BOX (hbox2), label, TRUE, TRUE, GNOME_PAD_SMALL);
  
  gtk_container_add (GTK_CONTAINER (button), hbox2);
  
  gtk_widget_set_usize (GTK_WIDGET (button), 40, 25);
  
  gtk_widget_show (pixmap);
  gtk_widget_show (label);
  gtk_widget_show (hbox2);
		
  return button;
}


GM_window_widgets *gnomemeeting_get_main_window (GtkWidget *gm)
{
  GM_window_widgets *gw = (GM_window_widgets *) 
    gtk_object_get_data (GTK_OBJECT (gm), "gw");

  return gw;
}


GM_pref_window_widgets *gnomemeeting_get_pref_window (GtkWidget *gm)
{
  GM_pref_window_widgets *pw = (GM_pref_window_widgets *) 
    gtk_object_get_data (GTK_OBJECT (gm), "pw");

  return pw;
}


GM_ldap_window_widgets *gnomemeeting_get_ldap_window (GtkWidget *gm)
{
  GM_ldap_window_widgets *lw = (GM_ldap_window_widgets *) 
    gtk_object_get_data (GTK_OBJECT (gm), "lw");

  return lw;
}


void gnomemeeting_log_insert (gchar *text)
{
  time_t *timeptr;
  char *time_str;

  time_str = (char *) malloc (21);
  timeptr = new (time_t);

  GM_window_widgets *gw = gnomemeeting_get_main_window (gm);

  time (timeptr);
  strftime(time_str, 20, "%H:%M:%S :  ", localtime (timeptr));

  gtk_text_insert (GTK_TEXT (gw->log_text), NULL, NULL, NULL, time_str, -1);
  gtk_text_insert (GTK_TEXT (gw->log_text), NULL, NULL, NULL, text, -1);
  gtk_text_insert (GTK_TEXT (gw->log_text), NULL, NULL, NULL, "\n", -1);

  free (time_str);
  delete (timeptr);
}


void gnomemeeting_init_main_window_logo ()
{
  GdkPixmap *text_logo;
  GdkBitmap *text_logo_mask;
  GdkRectangle update_rec;
  
  update_rec.x = 0;
  update_rec.y = 0;
  update_rec.width = GM_QCIF_WIDTH;
  update_rec.height = GM_QCIF_HEIGHT;

  GM_window_widgets *gw = gnomemeeting_get_main_window (gm);

  if (gw->drawing_area->allocation.width != GM_QCIF_WIDTH &&
      gw->drawing_area->allocation.height != GM_QCIF_HEIGHT) {
    
    gtk_drawing_area_size (GTK_DRAWING_AREA (gw->drawing_area), 
			   GM_QCIF_WIDTH, GM_QCIF_HEIGHT);
    gtk_widget_set_usize (GTK_WIDGET (gw->video_frame),
			  GM_QCIF_WIDTH + GM_FRAME_SIZE, GM_QCIF_HEIGHT);
  }

  gdk_draw_rectangle (gw->pixmap, gw->drawing_area->style->black_gc, TRUE,
		      0, 0, GM_QCIF_WIDTH, GM_QCIF_HEIGHT);

  text_logo = gdk_pixmap_create_from_xpm_d (gm->window, &text_logo_mask,
					    NULL,
					    (gchar **) text_logo_xpm);

  gdk_draw_pixmap (gw->pixmap, gw->drawing_area->style->black_gc, 
		   text_logo, 0, 0, 
		   (GM_QCIF_WIDTH - 150) / 2,
		   (GM_QCIF_HEIGHT - 62) / 2, -1, -1);

  gtk_widget_draw (gw->drawing_area, &update_rec);   
}


gint PlaySound (GtkWidget *widget)
{
  GtkWidget *object = NULL;

  if (widget != NULL) {

    /* First we check if it is the phone or the globe that is displayed.
       We can't call gnomemeeting_threads_enter as idles and timers
       are executed in the main thread */
    gdk_threads_enter ();
    object = (GtkWidget *) gtk_object_get_data (GTK_OBJECT (widget),
						"pixmapg");
    gdk_threads_leave ();
  }

  /* If the applet contents the phone pixmap */
  if (object == NULL) {

    gnome_triggers_do ("", "program", "GnomeMeeting", 
		       "incoming_call", NULL);
  }
  
  return TRUE;
}


void gnomemeeting_disable_connect ()
{ 
  GtkWidget *object;
  object = (GtkWidget *) gtk_object_get_data (GTK_OBJECT (gm),
					      "toolbar");

  GnomeUIInfo *main_toolbar = (GnomeUIInfo *) object;

  gtk_widget_set_sensitive (main_toolbar [0].widget, FALSE);
  gtk_widget_set_sensitive (main_toolbar [3].widget, FALSE);
  gtk_widget_set_sensitive (main_toolbar [5].widget, FALSE);

  object = (GtkWidget *) gtk_object_get_data (GTK_OBJECT (gm),
					      "file_menu_uiinfo");

  GnomeUIInfo *file_menu_uiinfo = (GnomeUIInfo *) object;

  gtk_widget_set_sensitive (file_menu_uiinfo [0].widget, FALSE);


  object = (GtkWidget *) gtk_object_get_data (GTK_OBJECT (gm),
					      "settings_menu_uiinfo");

  GnomeUIInfo *settings_menu_uiinfo = (GnomeUIInfo *) object;

  gtk_widget_set_sensitive (settings_menu_uiinfo [0].widget, FALSE);
}


void gnomemeeting_enable_connect ()
{
  GtkWidget *object;
  object = (GtkWidget *) gtk_object_get_data (GTK_OBJECT (gm),
					      "toolbar");

  GnomeUIInfo *main_toolbar = (GnomeUIInfo *) object;

  gtk_widget_set_sensitive (main_toolbar [0].widget, TRUE);
  gtk_widget_set_sensitive (main_toolbar [3].widget, TRUE);
  gtk_widget_set_sensitive (main_toolbar [5].widget, TRUE);

  object = (GtkWidget *) gtk_object_get_data (GTK_OBJECT (gm),
					      "file_menu_uiinfo");

  GnomeUIInfo *file_menu_uiinfo = (GnomeUIInfo *) object;

  gtk_widget_set_sensitive (file_menu_uiinfo [0].widget, TRUE);


  object = (GtkWidget *) gtk_object_get_data (GTK_OBJECT (gm),
					      "settings_menu_uiinfo");

  GnomeUIInfo *settings_menu_uiinfo = (GnomeUIInfo *) object;

  gtk_widget_set_sensitive (settings_menu_uiinfo [0].widget, TRUE);
}


void gnomemeeting_enable_disconnect ()
{
  GtkWidget *object;

  object = (GtkWidget *) gtk_object_get_data (GTK_OBJECT (gm),
					      "file_menu_uiinfo");

  GnomeUIInfo *file_menu_uiinfo = (GnomeUIInfo *) object;

  gtk_widget_set_sensitive (file_menu_uiinfo [1].widget, TRUE);


  object = (GtkWidget *) gtk_object_get_data (GTK_OBJECT (gm),
					      "toolbar");

  GnomeUIInfo *main_toolbar = (GnomeUIInfo *) object;

  gtk_widget_set_sensitive (main_toolbar [1].widget, TRUE);
  gtk_widget_set_sensitive (main_toolbar [3].widget, TRUE);
  gtk_widget_set_sensitive (main_toolbar [5].widget, TRUE);
}


void gnomemeeting_disable_disconnect ()
{
  GtkWidget *object;

  object = (GtkWidget *) gtk_object_get_data (GTK_OBJECT (gm),
					      "file_menu_uiinfo");

  GnomeUIInfo *file_menu_uiinfo = (GnomeUIInfo *) object;

  gtk_widget_set_sensitive (file_menu_uiinfo [1].widget, FALSE);


  object = (GtkWidget *) gtk_object_get_data (GTK_OBJECT (gm),
					      "toolbar");

  GnomeUIInfo *main_toolbar = (GnomeUIInfo *) object;

  gtk_widget_set_sensitive (main_toolbar [1].widget, FALSE);
  gtk_widget_set_sensitive (main_toolbar [3].widget, FALSE);
  gtk_widget_set_sensitive (main_toolbar [5].widget, FALSE);
}
