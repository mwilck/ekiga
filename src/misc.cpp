
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
#include <esd.h>

#ifndef DISABLE_GNOME
#include <gnome.h>
#else
#include <gtk/gtk.h>
#endif

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

  gtk_box_pack_start(GTK_BOX (hbox2), pixmap, TRUE, TRUE, 0);  
  gtk_box_pack_start(GTK_BOX (hbox2), label, TRUE, TRUE, 0);
  
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
#ifndef STATIC_LIBS_USED
void 
PAssertFunc (const char *file, int line, 
	     const char *className, const char *msg)
{
  static bool inAssert;

  if (inAssert)
    return;

  inAssert = true;

  cout << msg << endl << flush;
  
  gnomemeeting_threads_enter ();
  gnomemeeting_error_dialog (GTK_WINDOW (gm), 
			     _("Error: %s\n"), msg);
  gnomemeeting_threads_leave ();

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
  gtk_window_set_modal (GTK_WINDOW (widget), TRUE);

  gtk_widget_show_all (widget);

  g_free (msg);  

  return widget;
}


static int statusbar_clear_msg (gpointer data)
{
  GmWindow *gw = NULL;
  int id = 0;

  gdk_threads_enter ();

  gw = gnomemeeting_get_main_window (gm);
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
			      int row)                 
{                                                                              
  GtkWidget *entry = NULL;                                                     
  GtkWidget *label = NULL;                                                     
  gchar *gconf_string = NULL;                                                  
  GConfClient *client = NULL;                                                  
  GmPrefWindow *pw = gnomemeeting_get_pref_window (gm);
                                                                          
  client = gconf_client_get_default ();
                                                                               

  label = gtk_label_new (label_txt);                                           

  gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1,                
                    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),                
                    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),                
                    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);           
                                                                               
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);                         
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_RIGHT);                

                                                                               
  entry = gtk_entry_new ();                                                    
  gtk_table_attach (GTK_TABLE (table), entry, 1, 2, row, row+1,                
                    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),                
                    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),                
                    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);           
                                                                               
  gconf_string =  gconf_client_get_string (GCONF_CLIENT (client),              
                                           gconf_key, NULL);                   
                                                                               
  if (gconf_string != NULL)                                                    
    gtk_entry_set_text (GTK_ENTRY (entry), gconf_string);                      
                                                                               
  g_free (gconf_string);                                                       
                                                                               
                                                                               
  /* We set the key as data to be able to get the data in order to block       
     the signal in the gconf notifier */                             
  g_object_set_data (G_OBJECT (entry), "gconf_key", (void *) gconf_key);

  g_signal_connect (G_OBJECT (entry), "changed",                           
		    G_CALLBACK (entry_changed),                         
		    (gpointer) g_object_get_data (G_OBJECT (entry),
						  "gconf_key"));

  if (tooltip)
    gtk_tooltips_set_tip (pw->tips, entry, tooltip, NULL);

  return entry;                                                                
}                                                                              
                                                                               
                                                                               
GtkWidget *
gnomemeeting_table_add_toggle (GtkWidget *table,       
			       gchar *label_txt,       
			       gchar *gconf_key,       
			       gchar *tooltip,         
			       int row, int col)       
{
  GtkWidget *toggle = NULL;  
  GmPrefWindow *pw = gnomemeeting_get_pref_window (gm);
  GConfClient *client = NULL;                                                  
                                                                               
  client = gconf_client_get_default ();                                        

                                                                               
  toggle = gtk_check_button_new_with_label (label_txt);                        
  gtk_table_attach (GTK_TABLE (table), toggle, col, col+2, row, row+1,         
                    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),                
                    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),                
                    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);           
                                                                               
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
			     double min, double max, double step, int row)
{
  GtkAdjustment *adj = NULL;
  GtkWidget *label = NULL;
  GtkWidget *spin_button = NULL;
  GmPrefWindow *pw = gnomemeeting_get_pref_window (gm);

  GConfClient *client = NULL;                                                  
                                                                               
  client = gconf_client_get_default ();                                        


  label = gtk_label_new (label_txt);                                           

  gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1,                
                    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),                
                    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),                
                    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);           
                                                                               
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);                         
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_RIGHT);                


  adj = (GtkAdjustment *) 
    gtk_adjustment_new (gconf_client_get_int (client, gconf_key, 0), min, max, step, 
			2.0, 1.0);

  spin_button =
    gtk_spin_button_new (adj, 1.0, 0);
                                                                               
  gtk_table_attach (GTK_TABLE (table), spin_button, 1, 2, row, row+1,         
                    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),                
                    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),                
                    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);           
                                                                               
  gtk_adjustment_set_value (GTK_ADJUSTMENT (adj), 
			    gconf_client_get_int (client, gconf_key, NULL));

                                                                               
  gtk_tooltips_set_tip (pw->tips, spin_button, tooltip, NULL);     

  /* We set the key as data to be able to get the data in order to block       
     the signal in the gconf notifier */                             
  g_object_set_data (G_OBJECT (adj), "gconf_key", (void *) gconf_key);
                                                                               
  g_signal_connect (G_OBJECT (adj), "value-changed", G_CALLBACK (adjustment_changed),
		    (gpointer) gconf_key);                                   
                                                                               
  return spin_button;
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
  GmPrefWindow *pw = gnomemeeting_get_pref_window (gm);

  int cpt = 0;                                                   

  GConfClient *client = NULL;                                                  
                                                                               
  client = gconf_client_get_default ();                                        


  label = gtk_label_new (label_txt);                                           

  gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1,                
                    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),                
                    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),                
                    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);           
                                                                               
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);                         
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_RIGHT);                


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
                    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),                
                    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),                
                    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);           
                                                                               
                                                                               
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
  GmPrefWindow *pw = gnomemeeting_get_pref_window (gm);  
  gchar *gconf_string = NULL;
  int history = -1;

  int cpt = 0;                                                   

  GConfClient *client = NULL;                                                  
                                                                               
  client = gconf_client_get_default ();                                        


  label = gtk_label_new (label_txt);                                           

  gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1,                
                    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),                
                    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),                
                    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);           
                                                                               
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);                         
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_RIGHT);                


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
                    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),                
                    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),                
                    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);           
                                                                               
                           
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
gnomemeeting_vbox_add_table (GtkWidget *vbox,         
			     gchar *frame_name,       
			     int rows, int cols)      
{                                                                              
  GtkWidget *frame = NULL;
  GtkWidget *table = NULL;
  GtkWidget *label = NULL;
  
  PangoAttrList *attrs = NULL;
  PangoAttribute *attr = NULL;
   
  
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
                                                                              
  gtk_container_add (GTK_CONTAINER (frame), table); 
  gtk_container_set_border_width (GTK_CONTAINER (frame), 4);
  gtk_container_set_border_width (GTK_CONTAINER (table), 4); 
                                                                               
  gtk_table_set_row_spacings (GTK_TABLE (table), GNOMEMEETING_PAD_SMALL);     
  gtk_table_set_col_spacings (GTK_TABLE (table), GNOMEMEETING_PAD_SMALL);     
  
  return table;
}                                                                              
        
