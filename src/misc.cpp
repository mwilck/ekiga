
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2003 Damien Sandras
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
 * programs OpenH323 and Pwlib, and distribute the combination, without
 * applying the requirements of the GNU GPL to the OpenH323 program, as long
 * as you do follow the requirements of the GNU GPL for all the rest of the
 * software thus combined.
 */


/*
 *                         misc.cpp  -  description
 *                         ------------------------
 *   begin                : Thu Nov 22 2001
 *   copyright            : (C) 2000-2003 by Damien Sandras
 *   description          : This file contains miscellaneous functions.
 *   Additional Code      : De Michele Cristiano, Miguel Rodríguez 
 *
 */


#include "../config.h"

#include "misc.h"
#include "gnomemeeting.h"
#include "callbacks.h"
#include "config.h"
#include "dialog.h"

#include "../pixmaps/text_logo.xpm"


/* Declarations */
extern GtkWidget *gm;
extern GnomeMeeting *MyApp;


/* The functions */


void 
gnomemeeting_threads_enter () 
{
  if ((PThread::Current () != NULL) && (PThread::Current ()->GetThreadName () != "gnomemeeting")) {    
    gdk_threads_enter ();
  }
}


void 
gnomemeeting_threads_leave () 
{
  if ((PThread::Current () != NULL) && (PThread::Current ()->GetThreadName () != "gnomemeeting")) {

    gdk_threads_leave ();
  }
}


GtkWidget *
gnomemeeting_button_new (const char *lbl, 
			 GtkWidget *pixmap)
{
  GtkWidget *button = NULL;
  GtkWidget *hbox2 = NULL;
  GtkWidget *label = NULL;
  
  button = gtk_button_new ();
  label = gtk_label_new_with_mnemonic (lbl);
  hbox2 = gtk_hbox_new (FALSE, 0);

  gtk_box_pack_start(GTK_BOX (hbox2), pixmap, TRUE, TRUE, 0);  
  gtk_box_pack_start(GTK_BOX (hbox2), label, TRUE, TRUE, 0);
  
  gtk_container_add (GTK_CONTAINER (button), hbox2);

  return button;
}


void 
gnomemeeting_log_insert (GtkWidget *text_view, gchar *text)
{
  GtkTextIter end;
  GtkTextMark *mark;
  GtkTextBuffer *buffer;
  
  time_t *timeptr;
  char *time_str;
  gchar *text_buffer = NULL;

  time_str = (char *) malloc (21);
  timeptr = new (time_t);

  time (timeptr);
  strftime(time_str, 20, "%H:%M:%S", localtime (timeptr));

  text_buffer = g_strdup_printf ("%s %s\n", time_str, text);

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));

  gtk_text_buffer_get_iter_at_offset (GTK_TEXT_BUFFER (buffer), &end, -1);
  gtk_text_buffer_insert (GTK_TEXT_BUFFER (buffer), &end, text_buffer, -1);

  mark = gtk_text_buffer_get_mark (GTK_TEXT_BUFFER (buffer), 
				   "current-position");

  if (mark)
    gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (text_view), mark, 
 				  0.0, FALSE, 0,0);
  
  g_free (text_buffer);
  free (time_str);
  delete (timeptr);
}


void gnomemeeting_init_main_window_logo (GtkWidget *image)
{
  GdkPixbuf *tmp = NULL;
  GdkPixbuf *text_logo_pix = NULL;
  GtkRequisition size_request;

  GmWindow *gw = MyApp->GetMainWindow ();

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
#ifndef STATIC_LIBS_USED
static gboolean
assert_error_msg (gpointer data)
{
  /* FIX ME: message */
  gdk_threads_enter ();
  gnomemeeting_error_dialog (GTK_WINDOW (gm), 
			     _("Error: %s\n"), (gchar *) data);
  gdk_threads_leave ();

  return FALSE;
}


void 
PAssertFunc (const char *file, int line, 
	     const char *className, const char *msg)
{
  static bool inAssert;

  if (inAssert)
    return;

  inAssert = true;

  cout << msg << endl << flush;
  
  g_idle_add (assert_error_msg, (gpointer) msg);

  inAssert = FALSE;

  return;
}
#endif


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
  gchar     *msg = NULL;
  GmWindow  *gw = NULL;



  msg = g_strdup_printf (_("Call from %s\nusing %s"),
			 (const char*) utf8_name,
			 (const char *) utf8_app);
    
  gw = MyApp->GetMainWindow ();

  widget = gtk_dialog_new ();
  b2 = gtk_dialog_add_button (GTK_DIALOG (widget),
			      _("Reject"), 0);
  b1 = gtk_dialog_add_button (GTK_DIALOG (widget),
			      _("Accept"), 1);

  gtk_dialog_set_default_response (GTK_DIALOG (widget), 1);

  label = gtk_label_new (msg);
  hbox = gtk_hbox_new (0, 0);
  
  gtk_box_pack_start (GTK_BOX 
		      (GTK_DIALOG (widget)->vbox), 
		      hbox, TRUE, TRUE, 0);
  
  pixbuf = 
    gdk_pixbuf_new_from_file (GNOMEMEETING_IMAGES 
			      "/gnomemeeting-logo-icon.png", NULL);
  image = gtk_image_new_from_pixbuf (pixbuf);
  gtk_box_pack_start (GTK_BOX (hbox), 
		      image, TRUE, TRUE, 10);
  gtk_box_pack_start (GTK_BOX (hbox), 
		      label, TRUE, TRUE, 10);
  g_object_unref (pixbuf);
    
  g_signal_connect (G_OBJECT (b1), "clicked",
		    G_CALLBACK (connect_cb), gw);

  g_signal_connect (G_OBJECT (b2), "clicked",
		    G_CALLBACK (disconnect_cb), gw);

  g_signal_connect (G_OBJECT (widget), "delete-event",
		    G_CALLBACK (disconnect_cb), gw);
  
  gtk_window_set_transient_for (GTK_WINDOW (widget),
				GTK_WINDOW (gm));

  gtk_widget_show_all (widget);

  g_free (msg);  

  return widget;
}


static int statusbar_clear_msg (gpointer data)
{
  GmWindow *gw = NULL;
  int id = 0;

  gdk_threads_enter ();

  gw = MyApp->GetMainWindow ();
  id = gtk_statusbar_get_context_id (GTK_STATUSBAR (gw->statusbar),
				     "statusbar");

  gtk_statusbar_remove (GTK_STATUSBAR (gw->statusbar), id, 
			GPOINTER_TO_INT (data));

  gdk_threads_leave ();

  return FALSE;
}


void 
gnomemeeting_statusbar_flash (GtkWidget *widget, const char *msg, ...)
{
  va_list args;
  char    buffer [1025];
  int timeout_id = 0;
  int msg_id = 0;

  gint id = 0;
  int len = g_slist_length ((GSList *) (GTK_STATUSBAR (widget)->messages));
  id = gtk_statusbar_get_context_id (GTK_STATUSBAR (widget), "statusbar");
  
  for (int i = 0 ; i < len ; i++)
    gtk_statusbar_pop (GTK_STATUSBAR (widget), id);


  va_start (args, msg);
  vsnprintf (buffer, 1024, msg, args);

  msg_id = gtk_statusbar_push (GTK_STATUSBAR (widget), id, buffer);

  timeout_id = gtk_timeout_add (4000, statusbar_clear_msg, 
				GINT_TO_POINTER (msg_id));

  va_end (args);
}


void 
gnomemeeting_statusbar_push (GtkWidget *widget, const char *msg, ...)
{
  gint id = 0;
  int len = g_slist_length ((GSList *) (GTK_STATUSBAR (widget)->messages));
  id = gtk_statusbar_get_context_id (GTK_STATUSBAR (widget), "statusbar");
  
  for (int i = 0 ; i < len ; i++)
    gtk_statusbar_pop (GTK_STATUSBAR (widget), id);

  if (msg) {

    va_list args;
    char buffer [1025];

    va_start (args, msg);
    vsnprintf (buffer, 1024, msg, args);

    gtk_statusbar_push (GTK_STATUSBAR (widget), id, buffer);

    va_end (args);
  }
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


gchar *gnomemeeting_get_utf8 (PString str)
{
  gchar *utf8_str = NULL;

  if (g_utf8_validate ((gchar *) (const unsigned char*) str, -1, NULL))
    utf8_str = g_strdup ((char *) (const char *) (str));
  else
    utf8_str = gnomemeeting_from_iso88591_to_utf8 (str);

  return utf8_str;
}


GtkWidget *
gnomemeeting_table_add_entry (GtkWidget *table,
			      gchar *label_txt,
			      gchar *gconf_key,
			      gchar *tooltip,
			      int row,
			      gboolean box)
{
  GValue value = { 0 };
  int cols = 0;
  GtkWidget *entry = NULL;
  GtkWidget *label = NULL;
  GtkWidget *hbox = NULL;
  
  gchar *gconf_string = NULL;

  GConfClient *client = NULL;
  GmPrefWindow *pw = NULL;

  pw = MyApp->GetPrefWindow ();
  client = gconf_client_get_default ();

  if (box) {
    
    hbox = gtk_hbox_new (FALSE, 0);
    g_value_init (&value, G_TYPE_INT);
    g_object_get_property (G_OBJECT (table), "n-columns", &value);
    cols = g_value_get_int (&value);
    g_value_unset (&value);
  }
  
  label = gtk_label_new_with_mnemonic (label_txt);

  if (box)
    gtk_box_pack_start (GTK_BOX (hbox), label,
			FALSE, FALSE, GNOMEMEETING_PAD_SMALL * 2);
  else
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (GTK_FILL),
		      0, 0);

  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);

  entry = gtk_entry_new ();
  gtk_label_set_mnemonic_widget ( GTK_LABEL(label), entry);

  if (box)
    gtk_box_pack_start (GTK_BOX (hbox), entry,
			FALSE, FALSE, GNOMEMEETING_PAD_SMALL * 2);
  else
    gtk_table_attach (GTK_TABLE (table), entry, 1, 2, row, row+1,
		      (GtkAttachOptions) (NULL),
		      (GtkAttachOptions) (NULL),
		      0, 0);
  
  gconf_string =
    gconf_client_get_string (GCONF_CLIENT (client), gconf_key, NULL);

  if (gconf_string != NULL)
    gtk_entry_set_text (GTK_ENTRY (entry), gconf_string);

  g_free (gconf_string);

  /* We set the key as data to be able to get the data in order to block       
     the signal in the gconf notifier */
  g_object_set_data (G_OBJECT (entry), "gconf_key", (void *) gconf_key);

  g_signal_connect (G_OBJECT (entry), "changed", G_CALLBACK (entry_changed),
		    (gpointer) g_object_get_data (G_OBJECT (entry),
						  "gconf_key"));

  if (box)
    gtk_table_attach (GTK_TABLE (table), hbox, 0, cols, row, row+1,
		      (GtkAttachOptions) (NULL),
		      (GtkAttachOptions) (NULL),
		      0, 0);
        
  if (tooltip)
    gtk_tooltips_set_tip (pw->tips, entry, tooltip, NULL);

  return entry;
}                                                                              
                                                                               
                                                                               
GtkWidget *
gnomemeeting_table_add_toggle (GtkWidget *table,
			       gchar *label_txt,
			       gchar *gconf_key,
			       gchar *tooltip,
			       int row)
{
  GValue value = { 0 };
  GtkWidget *toggle = NULL;
  int cols = 0;
  
  GmPrefWindow *pw = NULL;
  GConfClient *client = NULL;
  
  client = gconf_client_get_default ();
  pw = MyApp->GetPrefWindow ();

  g_value_init (&value, G_TYPE_INT);
  g_object_get_property (G_OBJECT (table), "n-columns", &value);
  cols = g_value_get_int (&value);
  g_value_unset (&value);
  
  toggle = gtk_check_button_new_with_mnemonic (label_txt);
  gtk_table_attach (GTK_TABLE (table), toggle, 0, cols, row, row+1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL),
                    0, 0);
                                                                               
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle),
				gconf_client_get_bool (client,
						       gconf_key, NULL));
  gtk_tooltips_set_tip (pw->tips, toggle, tooltip, NULL);

  /* We set the key as data to be able to get the data in order to block       
     the signal in the gconf notifier */
  g_object_set_data (G_OBJECT (toggle), "gconf_key", (void *) gconf_key);

  g_signal_connect (G_OBJECT (toggle), "toggled", G_CALLBACK (toggle_changed),
		    (gpointer) gconf_key);

  return toggle;
}                                                                              


GtkWidget *
gnomemeeting_table_add_spin (GtkWidget *table,       
			     gchar *label_txt,
			     gchar *gconf_key,       
			     gchar *tooltip,
			     double min,
			     double max,
			     double step,
			     int row,
			     gchar *label_txt2,
			     gboolean box)
{
  GtkWidget *hbox = NULL;
  GtkAdjustment *adj = NULL;
  GtkWidget *label = NULL;
  GtkWidget *spin_button = NULL;
  GmPrefWindow *pw = NULL;
  
  GConfClient *client = NULL;

  client = gconf_client_get_default ();
  pw = MyApp->GetPrefWindow ();

  if (box)
    hbox = gtk_hbox_new (FALSE, 0);
  
  label = gtk_label_new_with_mnemonic (label_txt);

  if (box)
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE,
			GNOMEMEETING_PAD_SMALL * 2);
  else
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (GTK_FILL),
		      0, 0);

  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
  
  adj = (GtkAdjustment *) 
    gtk_adjustment_new (gconf_client_get_int (client, gconf_key, 0),
			min, max, step,
			2.0, 1.0);
  spin_button = gtk_spin_button_new (adj, 1.0, 0);

  if (box)
    gtk_box_pack_start (GTK_BOX (hbox), spin_button, FALSE, FALSE,
			GNOMEMEETING_PAD_SMALL * 2);
  else
    gtk_table_attach (GTK_TABLE (table), spin_button, 1, 2, row, row+1,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (GTK_FILL),
		      0, 0);

  if (box && label_txt2) {
    
    label = gtk_label_new_with_mnemonic (label_txt2);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE,
			GNOMEMEETING_PAD_SMALL * 2);
  }

  if (box)
    gtk_table_attach (GTK_TABLE (table), hbox, 0, 1, row, row+1,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (GTK_FILL),
		      0, 0);

  gtk_tooltips_set_tip (pw->tips, spin_button, tooltip, NULL);     

  /* We set the key as data to be able to get the data in order to block       
     the signal in the gconf notifier */
  g_object_set_data (G_OBJECT (adj), "gconf_key", (void *) gconf_key);
                                                                               
  g_signal_connect (G_OBJECT (adj), "value-changed",
		    G_CALLBACK (adjustment_changed),
		    (gpointer) gconf_key);

  return spin_button;
}


void
gnomemeeting_table_add_spin_range (GtkWidget *table,
				   gchar *label1_txt,
				   GtkWidget **spin1,
				   gchar *label2_txt,
				   GtkWidget **spin2,
				   gchar *label3_txt,
				   gchar *spin1_gconf_key,
				   gchar *spin2_gconf_key,
				   gchar *spin1_tooltip,
				   gchar *spin2_tooltip,
				   double spin1_min,
				   double spin2_min,
				   double spin1_max,
				   double spin2_max,
				   double spins_step,
				   int row)
{
  int val1 = 0, val2 = 0;
  GtkWidget *hbox = NULL;
  GtkAdjustment *adj1 = NULL;
  GtkAdjustment *adj2 = NULL;
  GtkWidget *label = NULL;

  GmPrefWindow *pw = NULL;
  GConfClient *client = NULL;

  pw = MyApp->GetPrefWindow ();
  client = gconf_client_get_default ();

  hbox = gtk_hbox_new (FALSE, 0);
  label = gtk_label_new_with_mnemonic (label1_txt);
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE,
		      GNOMEMEETING_PAD_SMALL * 2);
  
  val1 = gconf_client_get_int (client, spin1_gconf_key, 0);
  adj1 = (GtkAdjustment *) 
    gtk_adjustment_new (val1, spin1_min, spin1_max, spins_step, 2.0, 1.0);
  *spin1 = gtk_spin_button_new (adj1, 1.0, 0);
  gtk_box_pack_start (GTK_BOX (hbox), *spin1, FALSE, FALSE,
		      GNOMEMEETING_PAD_SMALL * 2);

  label = gtk_label_new_with_mnemonic (label2_txt);
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE,
		      GNOMEMEETING_PAD_SMALL * 2);

  val2 = gconf_client_get_int (client, spin2_gconf_key, 0);
  adj2 = (GtkAdjustment *) 
    gtk_adjustment_new (val2, spin2_min, spin2_max, spins_step, 2.0, 1.0);
  *spin2 = gtk_spin_button_new (adj2, 1.0, 0);
  gtk_box_pack_start (GTK_BOX (hbox), *spin2, FALSE, FALSE,
		      GNOMEMEETING_PAD_SMALL * 2);
  
  label = gtk_label_new_with_mnemonic (label3_txt);
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE,
		      GNOMEMEETING_PAD_SMALL * 2);

  gtk_table_attach (GTK_TABLE (table), hbox, 0, 1, row, row+1,
                    (GtkAttachOptions) (NULL),
		    (GtkAttachOptions) (NULL),
		    0, 0);
  
  gtk_tooltips_set_tip (pw->tips, *spin1, spin2_tooltip, NULL);
  gtk_tooltips_set_tip (pw->tips, *spin2, spin2_tooltip, NULL);
  
  
  /* We set the key as data to be able to get the data in order to block       
     the signal in the gconf notifier */
  g_object_set_data (G_OBJECT (adj1), "gconf_key", (void *) spin1_gconf_key);
  g_object_set_data (G_OBJECT (adj2), "gconf_key", (void *) spin2_gconf_key);

  g_signal_connect (G_OBJECT (adj1), "value-changed",
		    G_CALLBACK (adjustment_changed),
		    (gpointer) spin1_gconf_key);
  g_signal_connect (G_OBJECT (adj2), "value-changed",
		    G_CALLBACK (adjustment_changed),
		    (gpointer) spin2_gconf_key);
}                                                                              


GtkWidget *
gnomemeeting_table_add_int_option_menu (GtkWidget *table,
					gchar *label_txt,
					gchar **options,
					gchar *gconf_key,
					gchar *tooltip,
					int row)
{
  GtkWidget *item = NULL;
  GtkWidget *label = NULL;
  GtkWidget *option_menu = NULL;
  GtkWidget *menu = NULL;
  GmPrefWindow *pw = NULL;

  int cpt = 0;

  GConfClient *client = NULL;                                                  
  
  client = gconf_client_get_default ();                                        
  pw = MyApp->GetPrefWindow ();

  label = gtk_label_new_with_mnemonic (label_txt);

  gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL),
                    0, 0);

  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);

  menu = gtk_menu_new ();
  option_menu = gtk_option_menu_new ();

  while (options [cpt]) {

    item = gtk_menu_item_new_with_label (options [cpt]);
    gtk_widget_show (item);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
    cpt++;
  }

  gtk_option_menu_set_menu (GTK_OPTION_MENU (option_menu), menu);
  gtk_option_menu_set_history (GTK_OPTION_MENU (option_menu),
 			       gconf_client_get_int (client, gconf_key, NULL));

  gtk_table_attach (GTK_TABLE (table), option_menu, 1, 2, row, row+1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL),
                    0, 0);

  gtk_tooltips_set_tip (pw->tips, option_menu, tooltip, NULL);

  /* We set the key as data to be able to get the data in order to block
     the signal in the gconf notifier */
  g_object_set_data (G_OBJECT (option_menu), "gconf_key", (void *) gconf_key);

  g_signal_connect (G_OBJECT (GTK_OPTION_MENU (option_menu)->menu),
		    "deactivate", G_CALLBACK (int_option_menu_changed),
  		    (gpointer) gconf_key);

  return option_menu;
}                                                                              


GtkWidget *
gnomemeeting_table_add_string_option_menu (GtkWidget *table,       
					   gchar *label_txt, 
					   gchar **options,
					   gchar *gconf_key,       
					   gchar *tooltip,         
					   int row)       
{
  GtkWidget *item = NULL;
  GtkWidget *label = NULL;                                                     
  GtkWidget *option_menu = NULL;
  GtkWidget *menu = NULL;
  GmPrefWindow *pw = MyApp->GetPrefWindow ();  
  gchar *gconf_string = NULL;
  int history = -1;

  int cpt = 0;                                                   

  GConfClient *client = NULL;                                                  
                                                                               
  client = gconf_client_get_default ();                                        


  label = gtk_label_new (label_txt);                                           

  gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1,                
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL),
                    0, 0);
                                                                               
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);                         
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);


  menu = gtk_menu_new ();
  option_menu = gtk_option_menu_new ();

  gconf_string = gconf_client_get_string (client, gconf_key, NULL);

  while (options [cpt]) {

    if (gconf_string != NULL)
      if (!strcmp (gconf_string, options [cpt]))
	history = cpt;

    item = gtk_menu_item_new_with_label (options [cpt]);
    gtk_widget_show (item);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
    cpt++;
  }

  if (history == -1) {

    if (options [0])
      gconf_client_set_string (client, gconf_key, options [0], NULL);
    history = 0;
  }

  gtk_option_menu_set_menu (GTK_OPTION_MENU (option_menu), menu);
  gtk_option_menu_set_history (GTK_OPTION_MENU (option_menu), 
 			       history);


  gtk_table_attach (GTK_TABLE (table), option_menu, 1, 2, row, row+1,
                    (GtkAttachOptions) (GTK_FILL),                
                    (GtkAttachOptions) (GTK_FILL),                
                    0, 0);           
                                                                               
                           
  gtk_tooltips_set_tip (pw->tips, option_menu, tooltip, NULL);


  /* We set the key as data to be able to get the data in order to block       
     the signal in the gconf notifier */                             
  g_object_set_data (G_OBJECT (option_menu), "gconf_key", (void *) gconf_key);
                                                                               
  g_signal_connect (G_OBJECT (GTK_OPTION_MENU (option_menu)->menu), 
		    "deactivate", G_CALLBACK (string_option_menu_changed),
  		    (gpointer) gconf_key);                                   

  g_free (gconf_string); 

  return option_menu;
}


GtkWidget *
gnomemeeting_table_add_pstring_option_menu (GtkWidget *table,       
					    gchar *label_txt, 
					    PStringArray options,
					    gchar *gconf_key,       
					    gchar *tooltip,         
					    int row)       
{
  GtkWidget *item = NULL;
  GtkWidget *label = NULL;                                                     
  GtkWidget *option_menu = NULL;
  GtkWidget *menu = NULL;
  GmPrefWindow *pw = MyApp->GetPrefWindow ();  
  gchar *gconf_string = NULL;
  int history = -1;

  int cpt = 0;                                                   

  GConfClient *client = NULL;                                                  
                                                                               
  client = gconf_client_get_default ();                                        


  label = gtk_label_new_with_mnemonic (label_txt);

  gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1,                
                    (GtkAttachOptions) (GTK_FILL),                
                    (GtkAttachOptions) (GTK_FILL),                
                    0, 0);           
                                                                               
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);                         
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);                


  menu = gtk_menu_new ();
  option_menu = gtk_option_menu_new ();

  gconf_string = gconf_client_get_string (client, gconf_key, NULL);

  while (cpt < options.GetSize ()) {

    if (gconf_string != NULL)
      if (!strcmp (gconf_string, options [cpt]))
	history = cpt;

    item = gtk_menu_item_new_with_label (options [cpt]);
    gtk_widget_show (item);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
    cpt++;
  }

  if (history == -1) {

    if (!options [0].IsEmpty ())
      gconf_client_set_string (client, gconf_key, options [0], NULL);
    history = 0;
  }

  gtk_option_menu_set_menu (GTK_OPTION_MENU (option_menu), menu);
  gtk_option_menu_set_history (GTK_OPTION_MENU (option_menu), 
 			       history);


  gtk_table_attach (GTK_TABLE (table), option_menu, 1, 2, row, row+1,         
                    (GtkAttachOptions) (GTK_FILL),                
                    (GtkAttachOptions) (GTK_FILL),                
                    0, 0);           
                                                                               
                           
  gtk_tooltips_set_tip (pw->tips, option_menu, tooltip, NULL);


  /* We set the key as data to be able to get the data in order to block       
     the signal in the gconf notifier */                             
  g_object_set_data (G_OBJECT (option_menu), "gconf_key", (void *) gconf_key);
                                                                               
  g_signal_connect (G_OBJECT (GTK_OPTION_MENU (option_menu)->menu), 
		    "deactivate", G_CALLBACK (string_option_menu_changed),
  		    (gpointer) gconf_key);                                   

  g_free (gconf_string); 

  return option_menu;
}


void
gnomemeeting_update_pstring_option_menu (GtkWidget *option_menu,
					 PStringArray options,
					 gchar *gconf_key)
{
  GtkWidget *menu = NULL;
  GtkWidget *item = NULL;

  gchar *gconf_string = NULL;
  
  int history = -1;

  int cpt = 0;                                                   

  GConfClient *client = NULL;                                                  
                                                                               
  client = gconf_client_get_default ();
  gconf_string = gconf_client_get_string (client, gconf_key, NULL);
					  
  history = options.GetValuesIndex (PString (gconf_string));

  gtk_option_menu_remove_menu (GTK_OPTION_MENU (option_menu));
  menu = gtk_menu_new ();

  cpt = 0;
  while (cpt < options.GetSize ()) {

    item = gtk_menu_item_new_with_label (options [cpt]);
    gtk_widget_show (item);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

    cpt++;
  }

  if (history == P_MAX_INDEX) {
    
    history = 0;

    if (!options [0].IsEmpty ())
      gconf_client_set_string (client, gconf_key, options [0], NULL);
  }

  gtk_option_menu_set_menu (GTK_OPTION_MENU (option_menu), menu);
  gtk_option_menu_set_history (GTK_OPTION_MENU (option_menu), history);

  
  /* We set the key as data to be able to get the data in order to block       
     the signal in the gconf notifier */                             
  g_object_set_data (G_OBJECT (option_menu), "gconf_key", (void *) gconf_key);
                                                                               
  g_signal_connect (G_OBJECT (GTK_OPTION_MENU (option_menu)->menu), 
		    "deactivate", G_CALLBACK (string_option_menu_changed),
  		    (gpointer) gconf_key);                                   

  g_free (gconf_string); 
}


GtkWidget *
gnomemeeting_vbox_add_table (GtkWidget *vbox,         
			     gchar *frame_name,       
			     int rows, int cols)      
{                                                         
  GtkWidget *hbox = NULL;
  GtkWidget *frame = NULL;
  GtkWidget *table = NULL;
  GtkWidget *label = NULL;
  
  PangoAttrList *attrs = NULL;
  PangoAttribute *attr = NULL;
   
  hbox = gtk_hbox_new (FALSE, 6);

  frame = gtk_frame_new (frame_name);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);
  
  attrs = pango_attr_list_new ();
  attr = pango_attr_weight_new (PANGO_WEIGHT_BOLD);
  attr->start_index = 0;
  attr->end_index = strlen (frame_name);
  pango_attr_list_insert (attrs, attr);

  label = gtk_frame_get_label_widget (GTK_FRAME (frame));
  gtk_label_set_attributes (GTK_LABEL (label), attrs);
  pango_attr_list_unref (attrs);
    
  gtk_box_pack_start (GTK_BOX (vbox), frame,
                      FALSE, FALSE, 0);                                        
                                                                              
  table = gtk_table_new (rows, cols, FALSE);                                   
                                                                              
  gtk_container_add (GTK_CONTAINER (frame), hbox); 

  gtk_container_set_border_width (GTK_CONTAINER (hbox), 3);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 6);

  label = gtk_label_new ("    ");
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), table, FALSE, FALSE, 0);
                                                                               
  gtk_table_set_row_spacings (GTK_TABLE (table), 1);     
  gtk_table_set_col_spacings (GTK_TABLE (table), 6);     
  
  return table;
}                                                                              
        
