
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

#ifdef HAVE_ARTS
#include <artsc.h>
#endif

#include "config.h"
#include "main_window.h"
#include "common.h"
#include "pref_window.h"
#include "callbacks.h"
#include "misc.h"

#include "../pixmaps/text_logo.xpm"


/* Declarations */
extern GtkWidget *gm;

/* The functions */


void gnomemeeting_threads_enter () {

  if (PThread::Current ()->GetThreadName () != "gnomemeeting") {
    
    //    cout << "Will take GDK Lock" << endl << flush;
    PTRACE(1, "Will Take GDK Lock");
    gdk_threads_enter ();
    PTRACE(1, "GDK Lock Taken");
    //    cout << "GDK Lock Taken" << endl << flush;
  }
  else {

    PTRACE(1, "Ignore GDK Lock Request : Main Thread");
  }
    
}


void gnomemeeting_threads_leave () {

  if (PThread::Current ()->GetThreadName () != "gnomemeeting") {

    //    cout << "Will Release GDK Lock" << endl << flush;
    //    cout << PThread::Current ()->GetThreadName () << endl << flush;
    PTRACE(1, "Will Release GDK Lock");
    gdk_threads_leave ();
    PTRACE(1, "GDK Lock Released");
    //    cout << "GDK Lock Released" << endl << flush;
  }
  else {

    PTRACE(1, "Ignore GDK UnLock request : Main Thread");
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
    
  gtk_widget_show (pixmap);
  gtk_widget_show (label);
  gtk_widget_show (hbox2);
		
  return button;
}


GmWindow *gnomemeeting_get_main_window (GtkWidget *gm)
{
  GmWindow *gw = (GmWindow *) 
    g_object_get_data (G_OBJECT (gm), "gw");

  return gw;
}


GmPrefWindow *gnomemeeting_get_pref_window (GtkWidget *gm)
{
  GmPrefWindow *pw = (GmPrefWindow *) 
    g_object_get_data (G_OBJECT (gm), "pw");

  return pw;
}


GmLdapWindow *gnomemeeting_get_ldap_window (GtkWidget *gm)
{
  GmLdapWindow *lw = (GmLdapWindow *) 
    g_object_get_data (G_OBJECT (gm), "lw");

  return lw;
}


GmTextChat *gnomemeeting_get_chat_window (GtkWidget *gm)
{
  GmTextChat *chat = (GmTextChat *) 
    g_object_get_data (G_OBJECT (gm), "chat");

  return chat;
}


void gnomemeeting_log_insert (gchar *text)
{
  GtkTextIter start, end;
  GtkTextMark *mark;

  time_t *timeptr;
  char *time_str;

  time_str = (char *) malloc (21);
  timeptr = new (time_t);

  GmWindow *gw = gnomemeeting_get_main_window (gm);

  time (timeptr);
  strftime(time_str, 20, "%H:%M:%S ", localtime (timeptr));

  gtk_text_buffer_get_bounds (GTK_TEXT_BUFFER (gw->history), 
			      &start, &end);
  gtk_text_buffer_insert (GTK_TEXT_BUFFER (gw->history), 
			  &end, time_str, -1);
  gtk_text_buffer_get_bounds (GTK_TEXT_BUFFER (gw->history), 
			      &start, &end);
  gtk_text_buffer_insert (GTK_TEXT_BUFFER (gw->history), &end, text, -1);
  gtk_text_buffer_get_bounds (GTK_TEXT_BUFFER (gw->history), 
			      &start, &end);
  gtk_text_buffer_insert (GTK_TEXT_BUFFER (gw->history), &end, "\n", -1);

  mark = gtk_text_buffer_create_mark (GTK_TEXT_BUFFER (gw->history), 
				      "current-position", &end, FALSE);

  gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (gw->history_view), mark, 
				0.0, FALSE, 0,0);
  
  free (time_str);
  delete (timeptr);
}


void gnomemeeting_init_main_window_logo ()
{
  GdkPixbuf *text_logo_pix = NULL;
  GtkRequisition size_request;

  GmWindow *gw = gnomemeeting_get_main_window (gm);

  gtk_widget_size_request (GTK_WIDGET (gw->video_frame), &size_request);

  if ((size_request.width - GM_FRAME_SIZE != 176) || 
      (size_request.height != 144)) {

     gtk_widget_set_size_request (GTK_WIDGET (gw->video_frame),
				  176 + GM_FRAME_SIZE, 144);
  }

  text_logo_pix = gdk_pixbuf_new_from_xpm_data ((const char **) text_logo_xpm);
  gtk_image_set_from_pixbuf (GTK_IMAGE (gw->video_image),
			     GDK_PIXBUF (text_logo_pix));

  g_object_unref (text_logo_pix);
}


gint PlaySound (GtkWidget *widget)
{
  void *object = NULL;

  if (widget != NULL) {

    /* First we check if it is the phone or the globe that is displayed.
       We can't call gnomemeeting_threads_enter as idles and timers
       are executed in the main thread */
    gdk_threads_enter ();
    object = g_object_get_data (G_OBJECT (widget), "pixmapg");
    gdk_threads_leave ();
  }

  /* If the applet contents the phone pixmap */
  if (object == NULL) {

    gnome_triggers_do ("", "program", "gnomemeeting", 
		       "incoming_call", NULL);
  }

  return TRUE;
}


/* Helper functions por the PAssert dialog */
static void passert_close_cb (GtkDialog *dialog, gpointer data)
{
  _exit (1);
}


/* This function overrides from a pwlib function */
void PAssertFunc (const char * file, int line, const char * msg)

{
  static bool inAssert;
  gchar *mesg = NULL;

  if (inAssert)
    return;

  inAssert = true;

  gnomemeeting_threads_enter ();
  mesg = g_strdup_printf (_("Error: %s\n"), msg);

  GtkWidget *dialog = 
    gtk_dialog_new_with_buttons (_("GnomeMeeting Error"),
				 GTK_WINDOW (gm),
				 GTK_DIALOG_MODAL,
				 _("Ignore"), 0,
				 NULL);

  GtkWidget *label = gtk_label_new (mesg);
  
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), label, TRUE, TRUE, 0);
  
  gtk_widget_show_all (dialog);
  gnomemeeting_threads_leave ();

  gnomemeeting_threads_enter ();

  g_signal_connect_swapped (GTK_OBJECT (dialog),
			    "response",
			    G_CALLBACK (gtk_widget_destroy),
			    GTK_OBJECT (dialog));  

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
  gchar *file = NULL;
  gchar *msg = NULL;
  GmWindow *gw = NULL;


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


void gnomemeeting_statusbar_flash (GtkWidget *widget, const char *msg, ...)
{
  va_list args;
  char buffer [1025];

  va_start (args, msg);
  vsnprintf (buffer, 1024, msg, args);

  gnome_appbar_clear_stack (GNOME_APPBAR (GNOME_APP (widget)->statusbar));
  gnome_app_flash (GNOME_APP (widget), buffer);

  va_end (args);
}


void gnomemeeting_sound_daemons_suspend (void)
{
  int esd_client = 0;

  /* Put esd into standby mode */
  esd_client = esd_open_sound (NULL);
  if (esd_standby (esd_client) != 1) {
    
    gnomemeeting_log_insert (_("Could not suspend ESD"));
  }
      
  esd_close (esd_client);


  /* Put artsd into standby mode */
#ifdef HAVE_ARTS
  int artserror = arts_init();
  if (artserror) {
    
    gchar* artsmsg = g_strdup(arts_error_text(artserror));
    gnomemeeting_log_insert(artsmsg);
  } 
  else {
    
    if (0 == arts_suspend()) {
      
      gnomemeeting_log_insert (_("Could not suspend artsd"));
    } 
          
    arts_free();
  }
#endif
}


void gnomemeeting_sound_daemons_resume (void)
{
  int esd_client = 0;

  /* Put esd into normal mode */
  esd_client = esd_open_sound (NULL);

  if (esd_resume (esd_client) != 1) {

    gnomemeeting_log_insert (_("Could not resume ESD"));
  }

  esd_close (esd_client);
}
