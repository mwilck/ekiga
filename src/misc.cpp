
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2002 Damien Sandras
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
 *   copyright            : (C) 2000-2002 by Damien Sandras
 *   description          : This file contains miscellaneous functions.
 *   Additional Code      : De Michele Cristiano, Miguel Rodríguez 
 *
 */


#include "../config.h"

#include <ptlib.h>
#include <gnome.h>
#include <esd.h>


#include "config.h"
#include "main_window.h"
#include "common.h"
#include "pref_window.h"
#include "callbacks.h"
#include "misc.h"
#include "dialog.h"

#include "../pixmaps/text_logo.xpm"


/* Declarations */
extern GtkWidget *gm;

/* The functions */


void 
gnomemeeting_threads_enter () 
{
  if (PThread::Current ()->GetThreadName () != "gnomemeeting") 
  {    
    //    cout << "Will take GDK Lock" << endl << flush;
    PTRACE(1, "Will Take GDK Lock " << PThread::Current ()->GetThreadName ());
    gdk_threads_enter ();
    PTRACE(1, "GDK Lock Taken " << PThread::Current ()->GetThreadName ());
    //    cout << "GDK Lock Taken" << endl << flush;
  }
  else {

    PTRACE(1, "Ignore GDK Lock Request : Main Thread");
  }
    
}


void 
gnomemeeting_threads_leave () 
{
  if (PThread::Current ()->GetThreadName () != "gnomemeeting") {

    //    cout << "Will Release GDK Lock" << endl << flush;
    //    cout << PThread::Current ()->GetThreadName () << endl << flush;
    PTRACE(1, "Will Release GDK Lock " << PThread::Current ()->GetThreadName ());
    gdk_threads_leave ();
    PTRACE(1, "GDK Lock Released " << PThread::Current ()->GetThreadName ());
    //    cout << "GDK Lock Released" << endl << flush;
  }
  else {

    PTRACE(1, "Ignore GDK UnLock request : Main Thread");
  }
    
}


GtkWidget *
gnomemeeting_button (gchar *lbl, GtkWidget *pixmap)
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
    
  gtk_widget_show (pixmap);
  gtk_widget_show (label);
  gtk_widget_show (hbox2);
		
  return button;
}

void 
gnomemeeting_log_insert (GtkWidget *text_view, gchar *text)
{
  GtkTextIter start, end;
  GtkTextMark *mark;
  GtkTextBuffer *buffer;

  time_t *timeptr;
  char *time_str;

  time_str = (char *) malloc (21);
  timeptr = new (time_t);

  time (timeptr);
  strftime(time_str, 20, "%H:%M:%S ", localtime (timeptr));

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));

  gtk_text_buffer_get_bounds (GTK_TEXT_BUFFER (buffer), 
			      &start, &end);
  gtk_text_buffer_insert (GTK_TEXT_BUFFER (buffer), 
			  &end, time_str, -1);
  gtk_text_buffer_get_bounds (GTK_TEXT_BUFFER (buffer), 
			      &start, &end);
  gtk_text_buffer_insert (GTK_TEXT_BUFFER (buffer), &end, text, -1);
  gtk_text_buffer_get_bounds (GTK_TEXT_BUFFER (buffer), 
			      &start, &end);
  gtk_text_buffer_insert (GTK_TEXT_BUFFER (buffer), &end, "\n", -1);

  mark = gtk_text_buffer_create_mark (GTK_TEXT_BUFFER (buffer), 
				      "current-position", &end, FALSE);

  gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (text_view), mark, 
				0.0, FALSE, 0,0);
  
  free (time_str);
  delete (timeptr);
}


void gnomemeeting_init_main_window_logo (GtkWidget *image)
{
  GdkPixbuf *tmp = NULL;
  GdkPixbuf *text_logo_pix = NULL;
  GtkRequisition size_request;

  GmWindow *gw = gnomemeeting_get_main_window (gm);

  gtk_widget_size_request (GTK_WIDGET (gw->video_frame), &size_request);

  if ((size_request.width != GM_QCIF_WIDTH) || 
      (size_request.height != GM_QCIF_HEIGHT)) {

     gtk_widget_set_size_request (GTK_WIDGET (gw->video_frame),
				  176, 144);
  }

  text_logo_pix = gdk_pixbuf_new_from_xpm_data ((const char **) text_logo_xpm);
  tmp = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8, 176, 144);
  gdk_pixbuf_fill (tmp, 0x000000FF);  /* Opaque black */

  gdk_pixbuf_copy_area (text_logo_pix, 0, 0, 176, 60, 
			tmp, 0, 42);
  gtk_image_set_from_pixbuf (GTK_IMAGE (image),
			     GDK_PIXBUF (tmp));

  g_object_unref (text_logo_pix);
  g_object_unref (tmp);
}


/* This function overrides from a pwlib function */
void 
PAssertFunc (const char *file, int line, 
	     const char *className, const char *msg)
{
  static bool inAssert;

  if (inAssert)
    return;

  inAssert = true;

  gnomemeeting_threads_enter ();
  gnomemeeting_error_dialog (GTK_WINDOW (gm), 
			     _("Error: %s\n"), msg);
  gnomemeeting_threads_leave ();

  inAssert = FALSE;

  return;
}


GtkWidget * 
gnomemeeting_incoming_call_popup_new (gchar * utf8_name, 
				      gchar * utf8_app)
{
  GtkWidget *label = NULL;
  GdkPixbuf *pixbuf = NULL;
  GtkWidget *image = NULL;
  GtkWidget *hbox = NULL;
  GtkWidget *widget = NULL;
  GtkWidget *b1 = NULL, *b2 = NULL;
  gchar     *file = NULL;
  gchar     *msg = NULL;
  GmWindow  *gw = NULL;


  msg = g_strdup_printf (_("Call from %s\nusing %s"), (const char*) utf8_name, 
			 (const char *) utf8_app);
  gw = gnomemeeting_get_main_window (gm);

  widget = gtk_dialog_new ();
  b1 = gtk_dialog_add_button (GTK_DIALOG (widget),
			      _("Connect"), 0);
  b2 = gtk_dialog_add_button (GTK_DIALOG (widget),
			      _("Disconnect"), 1);
  
  label = gtk_label_new (msg);
  hbox = gtk_hbox_new (0, 0);
  
  gtk_box_pack_start (GTK_BOX 
		      (GTK_DIALOG (widget)->vbox), 
		      hbox, TRUE, TRUE, 0);
  
  file = 
    gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_PIXMAP, 
			       "gnomemeeting-logo-icon.png", TRUE, NULL);
  pixbuf = gdk_pixbuf_new_from_file (file, NULL);
  image = gtk_image_new_from_pixbuf (pixbuf);
  gtk_box_pack_start (GTK_BOX (hbox), 
		      image, TRUE, TRUE, GNOME_PAD_BIG);
  gtk_box_pack_start (GTK_BOX (hbox), 
		      label, TRUE, TRUE, GNOME_PAD_BIG);
  g_object_unref (pixbuf);
  g_free (file);
    
  g_signal_connect (G_OBJECT (b1), "clicked",
		    G_CALLBACK (connect_cb), gw);

  g_signal_connect (G_OBJECT (b2), "clicked",
		    G_CALLBACK (disconnect_cb), gw);

  g_signal_connect (G_OBJECT (widget), "delete-event",
		    G_CALLBACK (disconnect_cb), gw);
  
  gtk_window_set_transient_for (GTK_WINDOW (widget),
				GTK_WINDOW (gm));
  gtk_window_set_modal (GTK_WINDOW (widget), TRUE);

  gtk_widget_show_all (widget);

  g_free (msg);  

  return widget;
}


void 
gnomemeeting_statusbar_flash (GtkWidget *widget, const char *msg, ...)
{
  va_list args;
  char    buffer [1025];

  va_start (args, msg);
  vsnprintf (buffer, 1024, msg, args);

  gnome_appbar_clear_stack (GNOME_APPBAR (GNOME_APP (widget)->statusbar));
  gnome_app_flash (GNOME_APP (widget), buffer);

  va_end (args);
}


GtkWidget *gnomemeeting_video_window_new (gchar *title, GtkWidget *&image,
					  int x, int y)
{
  GtkWidget *window = NULL;
  GtkWidget *frame = NULL;

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), title);

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);

  image = gtk_image_new ();
  gtk_container_add (GTK_CONTAINER (frame), image);
  gtk_container_add (GTK_CONTAINER (window), frame);

  gtk_window_set_default_size (GTK_WINDOW (window), 
			       x, y);

  g_signal_connect (G_OBJECT (window), "delete_event",
		    G_CALLBACK (gtk_widget_hide_on_delete), 0);

  return window;
}


PString gnomemeeting_pstring_cut (PString s)
{
  PString s2 = s;

  if (s.IsEmpty ())
    return s2;

  PINDEX bracket = s2.Find('[');                                          
                                                                               
  if (bracket != P_MAX_INDEX)                                                
    s2 = s2.Left (bracket);                                            

  bracket = s2.Find('(');                                                 
                                                                               
  if (bracket != P_MAX_INDEX)                                                
    s2 = s2.Left (bracket);     

  return s2;
}


gchar *gnomemeeting_from_iso88591_to_utf8 (PString iso_string)
{
  if (iso_string.IsEmpty ())
    return NULL;

  gchar *utf_8_string =
    g_convert ((const char *) iso_string.GetPointer (),
               iso_string.GetSize (), "UTF-8", "ISO-8859-1", 0, 0, 0);
  
  return utf_8_string;
}
