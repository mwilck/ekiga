
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2004 Damien Sandras
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
 * GnomeMeeting is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OpenH323 and Pwlib, and distribute the combination, without
 * applying the requirements of the GNU GPL to the OpenH323 program, as long
 * as you do follow the requirements of the GNU GPL for all the rest of the
 * software thus combined.
 */


/*
 *                         main_window.cpp  -  description
 *                         -------------------------------
 *   begin                : Mon Mar 26 2001
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : This file contains all the functions needed to
 *                          build the main window.
 */


#include "../config.h"

#include "main_window.h"
#include "gnomemeeting.h"
#include "chat_window.h"
#include "config.h"
#include "menu.h"
#include "misc.h"
#include "toolbar.h"
#include "callbacks.h"
#include "tray.h"
#include "lid.h"
#include "sound_handling.h"

#include "dialog.h"
#include "gmentrydialog.h"
#include "stock-icons.h"
#include "gm_conf.h"
#include "contacts/gm_contacts.h"


#include "../pixmaps/text_logo.xpm"


#ifndef DISABLE_GNOME
#include <libgnomeui/gnome-window-icon.h>
#include <bonobo-activation/bonobo-activation-activate.h>
#include <bonobo-activation/bonobo-activation-register.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-listener.h>
#endif

#ifndef WIN32
#include <gdk/gdkx.h>
#endif

#if defined(P_FREEBSD) || defined (P_MACOSX)
#include <libintl.h>
#endif

#include <libxml/parser.h>


#define ACT_IID "OAFIID:GNOME_gnomemeeting_Factory"

#define GM_MAIN_WINDOW(x) (GmWindow *) (x)


/* Declarations */
extern GtkWidget *gm;


/* GUI Functions */


/* DESCRIPTION  : / 
 * BEHAVIOR     : Frees a GmMainWindow and its content.
 * PRE          : A non-NULL pointer to a GmMainWindow.
 */
static void gm_mw_destroy (gpointer);


/* DESCRIPTION  : / 
 * BEHAVIOR     : Returns a pointer to the private GmMainWindow
 * 		  used by the main book GMObject.
 * PRE          : The given GtkWidget pointer must be the main window GMObject. 
 */
static GmWindow *gm_mw_get_mw (GtkWidget *);


/* Callbacks */

/* DESCRIPTION  :  This callback is called when a video window is shown.
 * BEHAVIOR     :  Set the WM HINTS to stay-on-top if the config key is set
 *                 to true.
 * PRE          :  /
 */
static void video_window_shown_cb (GtkWidget *,
				   gpointer);


/* DESCRIPTION  :  This callback is called when the user changes the
 *                 audio settings sliders in the main notebook.
 * BEHAVIOR     :  Update the volume of the choosen mixers or of the lid.
 * PRE          :  The main window GMObject.
 */
static void audio_volume_changed_cb (GtkAdjustment *, 
				     gpointer);


/* DESCRIPTION  :  This callback is called when the user changes one of the 
 *                 video settings sliders in the main notebook.
 * BEHAVIOR     :  Updates the value in real time.
 * PRE          :  gpointer is a valid pointer to the main window GmObject.
 */
static void video_settings_changed_cb (GtkAdjustment *, 
				       gpointer);


/* DESCRIPTION  :  This callback is called when the user drops a contact.
 * BEHAVIOR     :  Calls the user corresponding to the contact.
 * PRE          :  Assumes data hides a GmWindow*
 */
static void dnd_call_contact_cb (GtkWidget *widget, 
				 GmContact *contact,
				 gint x, 
				 gint y, 
				 gpointer data);


/* FIXME: move to a GObject */
static gboolean stats_drawing_area_exposed_cb (GtkWidget *,
					       GdkEventExpose *,
					       gpointer);


/* FIXME: DBUS? */
#ifndef DISABLE_GNOME
static gboolean gnomemeeting_invoke_factory (int, char **);
static void gnomemeeting_new_event (BonoboListener *, const char *, 
				    const CORBA_any *, CORBA_Environment *,
				    gpointer);
static Bonobo_RegistrationResult gnomemeeting_register_as_factory (void);
#endif


/* DESCRIPTION  :  This callback is called when the user changes the
 *                 page in the main notebook.
 * BEHAVIOR     :  Update the config key accordingly.
 * PRE          :  A valid pointer to the main window GmObject.
 */
static void 
control_panel_section_changed_cb (GtkNotebook *, 
				  GtkNotebookPage *,
				  gint, 
				  gpointer);


/* DESCRIPTION  :  This callback is called when the user 
 *                 clicks on the dialpad button.
 * BEHAVIOR     :  Generates a dialpad event.
 * PRE          :  A valid pointer to the main window GMObject.
 */
static void dialpad_button_clicked_cb (GtkButton *, 
				       gpointer);


/* DESCRIPTION  :  This callback is called when the user tries to close
 *                 the application using the window manager.
 * BEHAVIOR     :  Calls the real callback if the notification icon is 
 *                 not shown else hide GM.
 * PRE          :  A valid pointer to the main window GMObject.
 */
static gint window_closed_cb (GtkWidget *, 
			      GdkEvent *, 
			      gpointer);


/* Misc Functions */

/* DESCRIPTION  : /
 * BEHAVIOR     : Builds the stats part of the main window.
 * PRE          : The given GtkWidget pointer must be the main window GMObject. 
 */
static void gm_mw_init_stats (GtkWidget *);


/* DESCRIPTION  : /
 * BEHAVIOR     : Builds the dialpad part of the main window.
 * PRE          : The given GtkWidget pointer must be the main window GMObject. 
 */
static void gm_mw_init_dialpad (GtkWidget *);


/* DESCRIPTION  : /
 * BEHAVIOR     : Builds the video settings part of the main window.
 * PRE          : The given GtkWidget pointer must be the main window GMObject. 
 */
static void gm_mw_init_video_settings (GtkWidget *);


/* DESCRIPTION  : /
 * BEHAVIOR     : Builds the audio settings part of the main window.
 * PRE          : The given GtkWidget pointer must be the main window GMObject. 
 */
static void gm_mw_init_audio_settings (GtkWidget *);





/* For stress testing */
//int i = 0;

/* GTK Callbacks */
/*
gint StressTest (gpointer data)
 {
   gdk_threads_enter ();


   GmWindow *mw = GnomeMeeting::Process ()->GetMainWindow ();

   if (!GTK_TOGGLE_BUTTON (mw->connect_button)->active) {

     i++;
     cout << "Call " << i << endl << flush;
   }

   gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (mw->connect_button), 
 				!GTK_TOGGLE_BUTTON (mw->connect_button)->active);

   gdk_threads_leave ();
   return TRUE;
}
*/


static void
gm_mw_destroy (gpointer mw)
{
  g_return_if_fail (mw != NULL);

  delete ((GmWindow *) mw);
}


static GmWindow *
gm_mw_get_mw (GtkWidget *main_window)
{
  g_return_val_if_fail (main_window != NULL, NULL);

  return GM_MAIN_WINDOW (g_object_get_data (G_OBJECT (main_window), 
					    "GMObject"));
}


static void
video_window_shown_cb (GtkWidget *w, 
		       gpointer data)
{
  GMH323EndPoint *endpoint = NULL;

  endpoint = GnomeMeeting::Process ()->Endpoint ();

  if (endpoint 
      && gm_conf_get_bool (VIDEO_DISPLAY_KEY "stay_on_top")
      && endpoint->GetCallingState () == GMH323EndPoint::Connected)
    gdk_window_set_always_on_top (GDK_WINDOW (w->window), TRUE);
}


static void
dnd_call_contact_cb (GtkWidget *widget, 
		     GmContact *contact,
		     gint x, 
		     gint y, 
		     gpointer data)
{
  GmWindow *mw = NULL;
  
  g_return_if_fail (data != NULL);
  
  if (contact && contact->url) {
    mw = (GmWindow *)data;
     if (GnomeMeeting::Process ()->Endpoint ()->GetCallingState () == GMH323EndPoint::Standby) {
       
       /* this function will store a copy of text */
       gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (mw->combo)->entry),
			   PString (contact->url));
       
       gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (mw->connect_button),
				     true);
     }
     gm_contact_delete (contact);
  }
}


static void 
audio_volume_changed_cb (GtkAdjustment *adjustment, 
			 gpointer data)
{
  GMH323EndPoint *ep = NULL;
  
  H323Connection *con = NULL;
  H323Codec *raw_codec = NULL;
  H323Channel *channel = NULL;

  PSoundChannel *sound_channel = NULL;

  int play_vol =  0, rec_vol = 0;


  g_return_if_fail (data != NULL);
  ep = GnomeMeeting::Process ()->Endpoint ();

  gm_main_window_get_volume_sliders_values (GTK_WIDGET (data), 
					    play_vol, rec_vol);

  gdk_threads_leave ();
 
  con = ep->FindConnectionWithLock (ep->GetCurrentCallToken ());

  if (con) {

    for (int cpt = 0 ; cpt < 2 ; cpt++) {

      channel = 
        con->FindChannel (RTP_Session::DefaultAudioSessionID, (cpt == 0));         
      if (channel) {

        raw_codec = channel->GetCodec();

        if (raw_codec) {

          sound_channel = (PSoundChannel *) raw_codec->GetRawDataChannel ();

          if (sound_channel)
            ep->SetDeviceVolume (sound_channel, 
                                 (cpt == 1), 
                                 (cpt == 1) ? rec_vol : play_vol);
        }
      }
    }
    con->Unlock ();
  }

  gdk_threads_enter ();
}


static void 
video_settings_changed_cb (GtkAdjustment *adjustment, 
			   gpointer data)
{ 
  GMH323EndPoint *ep = NULL;
  GMVideoGrabber *video_grabber = NULL;

  int brightness = -1;
  int whiteness = -1;
  int colour = -1;
  int contrast = -1;

  g_return_if_fail (data != NULL);
  
  ep = GnomeMeeting::Process ()->Endpoint ();
  
  gm_main_window_get_video_sliders_values (GTK_WIDGET (data),
					   brightness,
					   whiteness,
					   colour,
					   contrast);

  /* Notice about mutexes:
     The GDK lock is taken in the callback. We need to release it, because
     if CreateVideoGrabber is called in another thread, it will only
     release its internal mutex (also used by GetVideoGrabber) after it 
     returns, but it will return only if it is opened, and it can't open 
     if the GDK lock is held as it will wait on the GDK lock before 
     updating the GUI */
  gdk_threads_leave ();
  if (ep && (video_grabber = ep->GetVideoGrabber ())) {
    
    video_grabber->SetWhiteness (whiteness << 8);
    video_grabber->SetBrightness (brightness << 8);
    video_grabber->SetColour (colour << 8);
    video_grabber->SetContrast (contrast << 8);
    video_grabber->Unlock ();
  }
  gdk_threads_enter ();
}


static gboolean 
stats_drawing_area_exposed_cb (GtkWidget *drawing_area, 
			       GdkEventExpose *event,
			       gpointer data)
{
  GmWindow *mw = NULL;
  
  GdkSegment s [50];
  gboolean success [256];

  static PangoLayout *pango_layout = NULL;
  static PangoContext *pango_context = NULL;
  static GdkGC *gc = NULL;
  static GdkColormap *colormap = NULL;

  gchar *pango_text = NULL;
  
  int x = 0, y = 0;
  int cpt = 0;
  int pos = 0;
  
  GmRtpData *rtp = GnomeMeeting::Process ()->GetRtpData (); 
  GdkPoint points [50];

  int width_step = (int) GTK_WIDGET (drawing_area)->allocation.width / 40;
  x = width_step;
  int allocation_height = GTK_WIDGET (drawing_area)->allocation.height;
  float height_step = allocation_height;

  float max_tr_video = 1;
  float max_tr_audio = 1;
  float max_re_video = 1;
  float max_re_audio = 1;


  g_return_val_if_fail (data != NULL, FALSE);
  mw = gm_mw_get_mw (GTK_WIDGET (data));
  
  if (!gc)
    gc = gdk_gc_new (mw->stats_drawing_area->window);

  if (!colormap) {

    colormap = gdk_drawable_get_colormap (mw->stats_drawing_area->window);
    gdk_colormap_alloc_colors (colormap, mw->colors, 6, FALSE, TRUE, success);
  }

  gdk_gc_set_foreground (gc, &mw->colors [0]);
  gdk_draw_rectangle (drawing_area->window,
		      gc,
		      TRUE, 0, 0, 
		      GTK_WIDGET (drawing_area)->allocation.width,
		      GTK_WIDGET (drawing_area)->allocation.height);

  gdk_gc_set_foreground (gc, &mw->colors [1]);
  gdk_gc_set_line_attributes (gc, 1, GDK_LINE_SOLID, 
			      GDK_CAP_ROUND, GDK_JOIN_BEVEL);
  
  while ((y < GTK_WIDGET (drawing_area)->allocation.height)&&(cpt < 50)) {

    s [cpt].x1 = 0;
    s [cpt].x2 = GTK_WIDGET (drawing_area)->allocation.width;
    s [cpt].y1 = y;
    s [cpt].y2 = y;
      
    y = y + 21;
    cpt++;
  }
 
  gdk_draw_segments (GDK_DRAWABLE (drawing_area->window), gc, s, cpt);

  cpt = 0;
  while ((x < GTK_WIDGET (drawing_area)->allocation.width)&&(cpt < 50)) {

    s [cpt].x1 = x;
    s [cpt].x2 = x;
    s [cpt].y1 = 0;
    s [cpt].y2 = GTK_WIDGET (drawing_area)->allocation.height;
      
    x = x + 21;
    cpt++;
  }
 
  gdk_draw_segments (GDK_DRAWABLE (drawing_area->window), gc, s, cpt);
  gdk_window_set_background (drawing_area->window, &mw->colors [0]);


  /* Compute the height_step */
  if (GnomeMeeting::Process ()->Endpoint ()->GetCallingState () == GMH323EndPoint::Connected) {

    for (cpt = 0 ; cpt < 50 ; cpt++) {
    
      if (rtp->tr_audio_speed [cpt] > max_tr_audio)
	max_tr_audio = rtp->tr_audio_speed [cpt];
      if (rtp->re_audio_speed [cpt] > max_re_audio)
	max_re_audio = rtp->re_audio_speed [cpt];
      if (rtp->tr_video_speed [cpt] > max_tr_video)
	max_tr_video = rtp->tr_video_speed [cpt];
      if (rtp->re_video_speed [cpt] > max_re_video)
	max_re_video = rtp->re_video_speed [cpt];    
    }
    if (max_re_video > allocation_height / height_step)
      height_step = allocation_height / max_re_video;
    if (max_re_audio > allocation_height / height_step)
      height_step = allocation_height / max_re_audio;
    if (max_tr_video > allocation_height / height_step)
      height_step = allocation_height /  max_tr_video;
    if (max_tr_audio > allocation_height / height_step)
      height_step = allocation_height / max_tr_audio;

    gdk_gc_set_line_attributes (gc, 2, GDK_LINE_SOLID, 
				GDK_CAP_ROUND, GDK_JOIN_BEVEL);

    /* Transmitted audio */
    gdk_gc_set_foreground (gc, &mw->colors [3]);
    pos = rtp->tr_audio_pos;
    for (cpt = 0 ; cpt < 50 ; cpt++) {

      points [cpt].x = cpt * width_step;

      points [cpt].y = allocation_height -
	(gint) (rtp->tr_audio_speed [pos] * height_step);
      pos++;

      if (pos >= 50) pos = 0;
    }
    gdk_draw_lines (GDK_DRAWABLE (drawing_area->window), gc, points, 50);


    /* Received audio */
    gdk_gc_set_foreground (gc, &mw->colors [5]);
    pos = rtp->re_audio_pos;
    for (cpt = 0 ; cpt < 50 ; cpt++) {

      points [cpt].x = cpt * width_step;

      points [cpt].y = allocation_height -
	(gint) (rtp->re_audio_speed [pos] * height_step);
      pos++;

      if (pos >= 50) pos = 0;
    }
    gdk_draw_lines (GDK_DRAWABLE (drawing_area->window), gc, points, 50);


    /* Transmitted video */
    gdk_gc_set_foreground (gc, &mw->colors [4]);
    pos = rtp->tr_video_pos;
    for (cpt = 0 ; cpt < 50 ; cpt++) {

      points [cpt].x = cpt * width_step;

      points [cpt].y = allocation_height -
	(gint) (rtp->tr_video_speed [pos] * height_step);
      pos++;

      if (pos >= 50) pos = 0;
    }
    gdk_draw_lines (GDK_DRAWABLE (drawing_area->window), gc, points, 50);


    /* Received video */
    gdk_gc_set_foreground (gc, &mw->colors [2]);
    pos = rtp->re_video_pos;
    for (cpt = 0 ; cpt < 50 ; cpt++) {

      points [cpt].x = cpt * width_step;

      points [cpt].y = allocation_height -
	(gint) (rtp->re_video_speed [pos] * height_step);
      pos++;

      if (pos >= 50) pos = 0;
    }
    gdk_draw_lines (GDK_DRAWABLE (drawing_area->window), gc, points, 50);


    /* Text */
    if (!pango_context)
      pango_context = gtk_widget_get_pango_context (GTK_WIDGET (drawing_area));
    if (!pango_layout)
      pango_layout = pango_layout_new (pango_context);

    pango_text =
      g_strdup_printf (_("Total: %.2f MB"),
		       (float) (rtp->tr_video_bytes+rtp->tr_audio_bytes
		       +rtp->re_video_bytes+rtp->re_audio_bytes)
		       / (1024*1024));
    pango_layout_set_text (pango_layout, pango_text, strlen (pango_text));
    gdk_draw_layout_with_colors (GDK_DRAWABLE (drawing_area->window),
				 gc, 5, 2,
				 pango_layout,
				 &mw->colors [5], &mw->colors [0]);
    g_free (pango_text);
  }
  else {

    for (cpt = 0 ; cpt < 50 ; cpt++) {

      rtp->re_audio_speed [pos] = 0;
      rtp->re_video_speed [pos] = 0;
      rtp->tr_audio_speed [pos] = 0;
      rtp->tr_video_speed [pos] = 0;
    }    
  }


  return TRUE;
}


/* Factory stuff */

#ifndef DISABLE_GNOME
/* DESCRIPTION  :  /
 * BEHAVIOR     :  Invoked remotely to instantiate GnomeMeeting 
 *                 with the given URL.
 * PRE          :  /
 */
static void
gnomemeeting_new_event (BonoboListener    *listener,
			const char        *event_name, 
			const CORBA_any   *any,
			CORBA_Environment *ev,
			gpointer           user_data)
{
  int i;
  int argc;
  char **argv;
  CORBA_sequence_CORBA_string *args;
  
  GmWindow *mw = GnomeMeeting::Process ()->GetMainWindow ();

  args = (CORBA_sequence_CORBA_string *) any->_value;
  argc = args->_length;
  argv = args->_buffer;

  if (strcmp (event_name, "new_gnomemeeting")) {

      g_warning ("Unknown event '%s' on GnomeMeeting", event_name);
      return;
  }

  
  for (i = 1; i < argc; i++) {
    if (!strcmp (argv [i], "-c") || !strcmp (argv [i], "--callto")) 
      break;
  } 


  if ((i < argc) && (i + 1 < argc) && (argv [i+1])) {
    
     /* this function will store a copy of text */
    if (GnomeMeeting::Process ()->Endpoint ()->GetCallingState () == GMH323EndPoint::Standby) {

      gdk_threads_enter ();
      gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (mw->combo)->entry), 
			  argv [i + 1]);
      gdk_threads_leave ();
    }

    gdk_threads_enter ();
    connect_cb (NULL, NULL);
    gdk_threads_leave ();
  }
  else {

    gdk_threads_enter ();
    gnomemeeting_warning_dialog (GTK_WINDOW (gm), _("Cannot run GnomeMeeting"), _("GnomeMeeting is already running, if you want it to call a given callto or h323 URL, please use \"gnomemeeting -c URL\"."));
    gdk_threads_leave ();
  }
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Registers GnomeMeeting as a factory.
 * PRE          :  Returns the registration result.
 */
static Bonobo_RegistrationResult
gnomemeeting_register_as_factory (void)
{
  char *per_display_iid;
  BonoboListener *listener;
  Bonobo_RegistrationResult result;

  listener = bonobo_listener_new (gnomemeeting_new_event, NULL);

  per_display_iid = 
    bonobo_activation_make_registration_id (ACT_IID, 
					    DisplayString (gdk_display));

  result = 
    bonobo_activation_active_server_register (per_display_iid, 
					      BONOBO_OBJREF (listener));

  if (result != Bonobo_ACTIVATION_REG_SUCCESS)
    bonobo_object_unref (BONOBO_OBJECT (listener));

  g_free (per_display_iid);

  return result;
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Invoke the factory.
 * PRE          :  Registers the new factory, or use the already registered 
 *                 factory, or displays an error in the terminal.
 */
static gboolean
gnomemeeting_invoke_factory (int argc, char **argv)
{
  Bonobo_Listener listener;

  switch (gnomemeeting_register_as_factory ())
    {
    case Bonobo_ACTIVATION_REG_SUCCESS:
      /* we were the first GnomeMeeting to register */
      return FALSE;

    case Bonobo_ACTIVATION_REG_NOT_LISTED:
      g_printerr (_("It appears that you do not have gnomemeeting.server installed in a valid location. Factory mode disabled.\n"));
      return FALSE;
      
    case Bonobo_ACTIVATION_REG_ERROR:
      g_printerr (_("Error registering GnomeMeeting with the activation service; factory mode disabled.\n"));
      return FALSE;

    case Bonobo_ACTIVATION_REG_ALREADY_ACTIVE:
      /* lets use it then */
      break;
    }


  listener = 
    bonobo_activation_activate_from_id (ACT_IID, 
					Bonobo_ACTIVATION_FLAG_EXISTING_ONLY, 
					NULL, NULL);
  
  if (listener != CORBA_OBJECT_NIL) {

    int i;
    CORBA_any any;
    CORBA_sequence_CORBA_string args;
    CORBA_Environment ev;
    
    CORBA_exception_init (&ev);

    any._type = TC_CORBA_sequence_CORBA_string;
    any._value = &args;

    args._length = argc;
    args._buffer = g_newa (CORBA_char *, args._length);
    for (i = 0; i < (signed) (args._length); i++)
      args._buffer [i] = argv [i];
      
    Bonobo_Listener_event (listener, "new_gnomemeeting", &any, &ev);
    CORBA_Object_release (listener, &ev);

    if (!BONOBO_EX (&ev))
      return TRUE;

    CORBA_exception_free (&ev);
  } 
  else {    
    g_printerr (_("Failed to retrieve gnomemeeting server from activation server\n"));
  }
  
  return FALSE;
}
#endif


static void 
control_panel_section_changed_cb (GtkNotebook *notebook, 
				  GtkNotebookPage *page,
				  gint page_num, 
				  gpointer data) 
{
  GmWindow *mw = NULL;
  
  gint current_page = 0;

  
  g_return_if_fail (data != NULL);
  mw = gm_mw_get_mw (GTK_WIDGET (data));

  
  current_page = 
    gtk_notebook_get_current_page (GTK_NOTEBOOK (mw->main_notebook));
  gm_conf_set_int (USER_INTERFACE_KEY "main_window/control_panel_section",
		   current_page);
}


static void 
dialpad_button_clicked_cb (GtkButton *button, 
			   gpointer data)
{
  GtkWidget *label = NULL;
  const char *button_text = NULL;

  g_return_if_fail (data != NULL);

  
  /* FIXME: separation dans dialpad event du code du endpoint */
  label = gtk_bin_get_child (GTK_BIN (button));
  button_text = gtk_label_get_text (GTK_LABEL (label));

  if (button_text
      && strcmp (button_text, "")
      && strlen (button_text) > 1
      && button_text [0])
    gm_main_window_dialpad_event (GTK_WIDGET (data),
				  button_text [0]);
}


static gint 
window_closed_cb (GtkWidget *widget, 
		  GdkEvent *event,
		  gpointer data)
{
  GmWindow *mw = NULL;
  gboolean b = FALSE;

  g_return_val_if_fail (data != NULL, FALSE);
  mw = gm_mw_get_mw (GTK_WIDGET (data));

  b = gnomemeeting_tray_is_embedded (mw->docklet);

  if (!b)
    quit_callback (NULL, data);
  else 
    gnomemeeting_window_hide (GTK_WIDGET (gm));

  return (TRUE);
}  


/* Misc functions */
static void 
gm_mw_init_stats (GtkWidget *main_window)
{
  GmWindow *mw = NULL;
  
  GtkWidget *frame2 = NULL;
  GtkWidget *label = NULL;
  GtkWidget *vbox = NULL;

  
  g_return_if_fail (main_window != NULL);
  mw = gm_mw_get_mw (main_window);

  
  /* The first frame with statistics display */
  frame2 = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame2), GTK_SHADOW_IN);

  vbox = gtk_vbox_new (FALSE, 6);
  mw->stats_drawing_area = gtk_drawing_area_new ();

  gtk_box_pack_start (GTK_BOX (vbox), frame2, FALSE, TRUE, 0);
  gtk_container_add (GTK_CONTAINER (frame2), mw->stats_drawing_area);

  gtk_widget_set_size_request (GTK_WIDGET (frame2), GM_QCIF_WIDTH, 47);
  mw->colors [0].red = 0;
  mw->colors [0].green = 0;
  mw->colors [0].blue = 0;

  mw->colors [1].red = 169;
  mw->colors [1].green = 38809;
  mw->colors [1].blue = 52441;

  mw->colors [2].red = 0;
  mw->colors [2].green = 65535;
  mw->colors [2].blue = 0;
  
  mw->colors [3].red = 65535;
  mw->colors [3].green = 0;
  mw->colors [3].blue = 0;

  mw->colors [4].red = 0;
  mw->colors [4].green = 0;
  mw->colors [4].blue = 65535;

  mw->colors [5].red = 65535;
  mw->colors [5].green = 54756;
  mw->colors [5].blue = 0;

  g_signal_connect (G_OBJECT (mw->stats_drawing_area), "expose_event",
		    G_CALLBACK (stats_drawing_area_exposed_cb), 
		    main_window);

  gtk_widget_queue_draw_area (mw->stats_drawing_area, 0, 0, 
			      GTK_WIDGET (mw->stats_drawing_area)->allocation.width, 
			      GTK_WIDGET (mw->stats_drawing_area)->allocation.height);


  /* The second one with some labels */
  mw->stats_label =
    gtk_label_new (_("Lost packets:\nLate packets:\nRound-trip delay:\nJitter buffer:"));
  gtk_misc_set_alignment (GTK_MISC (mw->stats_label), 0, 0);
  gtk_box_pack_start (GTK_BOX (vbox), mw->stats_label, FALSE, TRUE,
		      GNOMEMEETING_PAD_SMALL);
  
  label = gtk_label_new (_("Statistics"));

  gtk_notebook_append_page (GTK_NOTEBOOK (mw->main_notebook), vbox, label);
}


static void 
gm_mw_init_dialpad (GtkWidget *main_window)
{
  GmWindow *mw = NULL;
  
  GtkWidget *label = NULL;
  GtkWidget *table = NULL;
  GtkWidget *button = NULL;

  int i = 0;

  char *key_n [] = { "1", "2", "3", "4", "5", "6", "7", "8", "9",
		     "*", "0", "#"};
  char *key_a []= { "  ", "abc", "def", "ghi", "jkl", "mno", "pqrs", "tuv",
		   "wxyz", "  ", "  ", "  "};

  gchar *text_label = NULL;
  

  g_return_if_fail (main_window != NULL);
  mw = gm_mw_get_mw (main_window);

  table = gtk_table_new (4, 3, TRUE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 6);
  
  for (i = 0 ; i < 12 ; i++) {

    label = gtk_label_new (NULL);
    text_label =
      g_strdup_printf ("%s<sub><span size=\"small\">%s</span></sub>",
		       key_n [i], key_a [i]);
    gtk_label_set_markup (GTK_LABEL (label), text_label); 
    button = gtk_button_new ();
    gtk_container_set_border_width (GTK_CONTAINER (button), 0);
    gtk_container_add (GTK_CONTAINER (button), label);
    
    gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (button), 
		      i%3, i%3+1, i/3, i/3+1,
		      (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		      (GtkAttachOptions) (GTK_FILL),
		      1, 1);
    
    g_signal_connect (G_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (dialpad_button_clicked_cb), 
		      main_window);

    g_free (text_label);
  }
  
  label = gtk_label_new (_("Dialpad"));

  gtk_notebook_append_page (GTK_NOTEBOOK (mw->main_notebook),
			    table, label);
}


static void 
gm_mw_init_video_settings (GtkWidget *main_window)
{
  GmWindow *mw = NULL;
  
  GtkWidget *label = NULL;
  GtkWidget *hbox = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *image = NULL;

  GtkWidget *hscale_brightness = NULL;
  GtkWidget *hscale_colour = NULL;
  GtkWidget *hscale_contrast = NULL;
  GtkWidget *hscale_whiteness = NULL;

  int brightness = 0, colour = 0, contrast = 0, whiteness = 0;
  

  g_return_if_fail (main_window != NULL);
  mw = gm_mw_get_mw (main_window);

  
  /* Webcam Control Frame, we need it to disable controls */		
  mw->video_settings_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (mw->video_settings_frame), 
			     GTK_SHADOW_NONE);
  gtk_container_set_border_width (GTK_CONTAINER (mw->video_settings_frame), 0);
  
  /* Category */
  vbox = gtk_vbox_new (0, FALSE);
  gtk_container_add (GTK_CONTAINER (mw->video_settings_frame), vbox);

  
  /* Brightness */
  hbox = gtk_hbox_new (0, FALSE);
  image = gtk_image_new_from_stock (GM_STOCK_BRIGHTNESS, GTK_ICON_SIZE_MENU);
  gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);

  mw->adj_brightness = gtk_adjustment_new (brightness, 0.0, 
					   255.0, 1.0, 5.0, 1.0);
  hscale_brightness = gtk_hscale_new (GTK_ADJUSTMENT (mw->adj_brightness));
  gtk_range_set_update_policy (GTK_RANGE (hscale_brightness),
			       GTK_UPDATE_DELAYED);
  gtk_scale_set_draw_value (GTK_SCALE (hscale_brightness), FALSE);
  gtk_scale_set_value_pos (GTK_SCALE (hscale_brightness), GTK_POS_RIGHT);
  gtk_box_pack_start (GTK_BOX (hbox), hscale_brightness, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

  gtk_tooltips_set_tip (mw->tips, hscale_brightness,
			_("Adjust brightness"), NULL);

  g_signal_connect (G_OBJECT (mw->adj_brightness), "value-changed",
		    G_CALLBACK (video_settings_changed_cb), 
		    (gpointer) gm);


  /* Whiteness */
  hbox = gtk_hbox_new (0, FALSE);
  image = gtk_image_new_from_stock (GM_STOCK_WHITENESS, GTK_ICON_SIZE_MENU);
  gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);

  mw->adj_whiteness = gtk_adjustment_new (whiteness, 0.0, 
					  255.0, 1.0, 5.0, 1.0);
  hscale_whiteness = gtk_hscale_new (GTK_ADJUSTMENT (mw->adj_whiteness));
  gtk_range_set_update_policy (GTK_RANGE (hscale_whiteness),
			       GTK_UPDATE_DELAYED);
  gtk_scale_set_draw_value (GTK_SCALE (hscale_whiteness), FALSE);
  gtk_scale_set_value_pos (GTK_SCALE (hscale_whiteness), GTK_POS_RIGHT);
  gtk_box_pack_start (GTK_BOX (hbox), hscale_whiteness, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

  gtk_tooltips_set_tip (mw->tips, hscale_whiteness,
			_("Adjust whiteness"), NULL);

  g_signal_connect (G_OBJECT (mw->adj_whiteness), "value-changed",
		    G_CALLBACK (video_settings_changed_cb), 
		    (gpointer) gm);


  /* Colour */
  hbox = gtk_hbox_new (0, FALSE);
  image = gtk_image_new_from_stock (GM_STOCK_COLOURNESS, GTK_ICON_SIZE_MENU);
  gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);

  mw->adj_colour = gtk_adjustment_new (colour, 0.0, 
				       255.0, 1.0, 5.0, 1.0);
  hscale_colour = gtk_hscale_new (GTK_ADJUSTMENT (mw->adj_colour));
  gtk_range_set_update_policy (GTK_RANGE (hscale_colour),
			       GTK_UPDATE_DELAYED);
  gtk_scale_set_draw_value (GTK_SCALE (hscale_colour), FALSE);
  gtk_scale_set_value_pos (GTK_SCALE (hscale_colour), GTK_POS_RIGHT);
  gtk_box_pack_start (GTK_BOX (hbox), hscale_colour, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

  gtk_tooltips_set_tip (mw->tips, hscale_colour,
			_("Adjust color"), NULL);

  g_signal_connect (G_OBJECT (mw->adj_colour), "value-changed",
		    G_CALLBACK (video_settings_changed_cb), 
		    (gpointer) gm);


  /* Contrast */
  hbox = gtk_hbox_new (0, FALSE);
  image = gtk_image_new_from_stock (GM_STOCK_CONTRAST, GTK_ICON_SIZE_MENU);
  gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);
  
  mw->adj_contrast = gtk_adjustment_new (contrast, 0.0, 
					 255.0, 1.0, 5.0, 1.0);
  hscale_contrast = gtk_hscale_new (GTK_ADJUSTMENT (mw->adj_contrast));
  gtk_range_set_update_policy (GTK_RANGE (hscale_contrast),
			       GTK_UPDATE_DELAYED);
  gtk_scale_set_draw_value (GTK_SCALE (hscale_contrast), FALSE);
  gtk_scale_set_value_pos (GTK_SCALE (hscale_contrast), GTK_POS_RIGHT);
  gtk_box_pack_start (GTK_BOX (hbox), hscale_contrast, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

  gtk_tooltips_set_tip (mw->tips, hscale_contrast,
			_("Adjust contrast"), NULL);

  g_signal_connect (G_OBJECT (mw->adj_contrast), "value-changed",
		    G_CALLBACK (video_settings_changed_cb), (gpointer) gm);
  

  gtk_widget_set_sensitive (GTK_WIDGET (mw->video_settings_frame), FALSE);

  label = gtk_label_new (_("Video"));  

  gtk_notebook_append_page (GTK_NOTEBOOK(mw->main_notebook), 
			    mw->video_settings_frame, label);
}



static void 
gm_mw_init_audio_settings (GtkWidget *main_window)
{
  GmWindow *mw = NULL;
  
  GtkWidget *hscale_play = NULL; 
  GtkWidget *hscale_rec = NULL;
  GtkWidget *label = NULL;
  GtkWidget *hbox = NULL;
  GtkWidget *vbox = NULL;
  

  /* Get the data from the GMObject */
  mw = gm_mw_get_mw (main_window);
  

  /* Audio control frame, we need it to disable controls */		
  mw->audio_volume_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (mw->audio_volume_frame), 
			     GTK_SHADOW_NONE);
  gtk_container_set_border_width (GTK_CONTAINER (mw->audio_volume_frame), 0);


  /* The vbox */
  vbox = gtk_vbox_new (0, FALSE);
  gtk_container_add (GTK_CONTAINER (mw->audio_volume_frame), vbox);
  gtk_widget_set_sensitive (GTK_WIDGET (mw->audio_volume_frame), FALSE);
  

  /* Output volume */
  hbox = gtk_hbox_new (0, FALSE);
  gtk_box_pack_start (GTK_BOX (hbox),
		      gtk_image_new_from_stock (GM_STOCK_VOLUME, 
						GTK_ICON_SIZE_SMALL_TOOLBAR),
		      FALSE, FALSE, 0);
  
  mw->adj_output_volume = gtk_adjustment_new (0, 0.0, 100.0, 1.0, 5.0, 1.0);
  hscale_play = gtk_hscale_new (GTK_ADJUSTMENT (mw->adj_output_volume));
  gtk_range_set_update_policy (GTK_RANGE (hscale_play),
			       GTK_UPDATE_DELAYED);
  gtk_scale_set_value_pos (GTK_SCALE (hscale_play), GTK_POS_RIGHT); 
  gtk_scale_set_draw_value (GTK_SCALE (hscale_play), FALSE);
  gtk_box_pack_start (GTK_BOX (hbox), hscale_play, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 3);


  /* Input volume */
  hbox = gtk_hbox_new (0, FALSE);
  gtk_box_pack_start (GTK_BOX (hbox),
		      gtk_image_new_from_stock (GM_STOCK_MICROPHONE, 
						GTK_ICON_SIZE_SMALL_TOOLBAR),
		      FALSE, FALSE, 0);

  mw->adj_input_volume = gtk_adjustment_new (0, 0.0, 100.0, 1.0, 5.0, 1.0);
  hscale_rec = gtk_hscale_new (GTK_ADJUSTMENT (mw->adj_input_volume));
  gtk_range_set_update_policy (GTK_RANGE (hscale_rec),
			       GTK_UPDATE_DELAYED);
  gtk_scale_set_value_pos (GTK_SCALE (hscale_rec), GTK_POS_RIGHT); 
  gtk_scale_set_draw_value (GTK_SCALE (hscale_rec), FALSE);
  gtk_box_pack_start (GTK_BOX (hbox), hscale_rec, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 3);


  g_signal_connect (G_OBJECT (mw->adj_output_volume), "value-changed",
		    G_CALLBACK (audio_volume_changed_cb), main_window);

  g_signal_connect (G_OBJECT (mw->adj_input_volume), "value-changed",
		    G_CALLBACK (audio_volume_changed_cb), main_window);

		    
  label = gtk_label_new (_("Audio"));

  gtk_notebook_append_page (GTK_NOTEBOOK (mw->main_notebook),
			    mw->audio_volume_frame, label);
}


/* The functions */
void 
gm_main_window_update_logo (GtkWidget *main_window)
{
  GmWindow *mw = NULL;
  
  GdkPixbuf *tmp = NULL;
  GdkPixbuf *text_logo_pix = NULL;
  GtkRequisition size_request;

  g_return_if_fail (main_window != NULL);

  mw = gm_mw_get_mw (main_window);
  

  gtk_widget_size_request (GTK_WIDGET (mw->video_frame), &size_request);

  if ((size_request.width != GM_QCIF_WIDTH) || 
      (size_request.height != GM_QCIF_HEIGHT)) {

     gtk_widget_set_size_request (GTK_WIDGET (mw->video_frame),
				  176, 144);
  }

  text_logo_pix = gdk_pixbuf_new_from_xpm_data ((const char **) text_logo_xpm);
  tmp = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8, 176, 144);
  gdk_pixbuf_fill (tmp, 0x000000FF);  /* Opaque black */

  gdk_pixbuf_copy_area (text_logo_pix, 0, 0, 176, 60, 
			tmp, 0, 42);
  gtk_image_set_from_pixbuf (GTK_IMAGE (mw->main_video_image),
			     GDK_PIXBUF (tmp));

  g_object_unref (text_logo_pix);
  g_object_unref (tmp);
}


void 
gm_main_window_dialpad_event (GtkWidget *main_window,
			      const char d)
{
  GmWindow *mw = NULL;
  
  GMH323EndPoint *endpoint = NULL;
  H323Connection *connection = NULL;

#ifdef HAS_IXJ
  GMLid *lid = NULL;
#endif
  
  PString url;
  PString new_url;

  char dtmf = d;
  gchar *msg = NULL;
  
  
  g_return_if_fail (main_window != NULL);
  mw = gm_mw_get_mw (main_window);
  
  endpoint = GnomeMeeting::Process ()->Endpoint ();

  if (mw->transfer_call_popup)
    url = gm_entry_dialog_get_text (GM_ENTRY_DIALOG (mw->transfer_call_popup));
  else
    url = gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (mw->combo)->entry)); 

  
  if (endpoint->GetCallingState () == GMH323EndPoint::Standby) {

    /* Replace the * by a . */
    if (dtmf == '*') 
      dtmf = '.';
  }
      
  new_url = PString (url) + dtmf;

  if (mw->transfer_call_popup)
    gm_entry_dialog_set_text (GM_ENTRY_DIALOG (mw->transfer_call_popup),
			      new_url);
  else if (endpoint->GetCallingState () == GMH323EndPoint::Standby)
    gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (mw->combo)->entry), new_url);

  if (dtmf == '#' && mw->transfer_call_popup)
    gtk_dialog_response (GTK_DIALOG (mw->transfer_call_popup),
			 GTK_RESPONSE_ACCEPT);
  
  if (endpoint->GetCallingState () == GMH323EndPoint::Connected
      && !mw->transfer_call_popup) {

    gdk_threads_leave ();
    connection = 
      endpoint->FindConnectionWithLock (endpoint->GetCurrentCallToken ());
            
    if (connection) {

      msg = g_strdup_printf (_("Sent dtmf %c"), dtmf);
      
      connection->SendUserInput (dtmf);
      connection->Unlock ();
    }
    gdk_threads_enter ();

    if (msg) {

      gnomemeeting_statusbar_flash (mw->statusbar, msg);
      g_free (msg);
    }
  }

#ifdef HAS_IXJ
  lid = endpoint->GetLid ();
  if (lid) {

    lid->StopTone (0);
    lid->Unlock ();
  }
#endif
}


void
gm_main_window_update_sensitivity (//GtkWidget *main_window,
				   unsigned calling_state)
{
  GmWindow *mw = NULL;

  mw = GnomeMeeting::Process ()->GetMainWindow ();
  
  switch (calling_state)
    {
    case GMH323EndPoint::Standby:

      gtk_widget_set_sensitive (GTK_WIDGET (mw->preview_button), TRUE);
      connect_button_update_pixmap (GTK_TOGGLE_BUTTON (mw->connect_button), 0);
      break;


    case GMH323EndPoint::Calling:

      gtk_widget_set_sensitive (GTK_WIDGET (mw->preview_button), FALSE);
      connect_button_update_pixmap (GTK_TOGGLE_BUTTON (mw->connect_button), 1);
      break;


    case GMH323EndPoint::Connected:

      gtk_widget_set_sensitive (GTK_WIDGET (mw->preview_button), FALSE);
      connect_button_update_pixmap (GTK_TOGGLE_BUTTON (mw->connect_button), 1);
      break;


    case GMH323EndPoint::Called:

      gtk_widget_set_sensitive (GTK_WIDGET (mw->preview_button), FALSE);
      break;
    }
}


void
gm_main_window_update_sensitivity (//GtkWidget *,
				   BOOL is_video,
				   BOOL is_receiving,
				   BOOL is_transmitting)
{
  GmWindow *mw = NULL;
  GtkWidget *button = NULL;
  GtkWidget *frame = NULL;
  
  mw = GnomeMeeting::Process ()->GetMainWindow ();
  
  if (is_video) {

    frame = mw->video_settings_frame;
    button = mw->video_chan_button;
  }
  else {

    frame = mw->audio_volume_frame;
    button = mw->audio_chan_button;
  }
  
  gtk_widget_set_sensitive (GTK_WIDGET (button), is_transmitting);
  gtk_widget_set_sensitive (GTK_WIDGET (frame), is_transmitting);
}


void
gm_main_window_set_volume_sliders_values (GtkWidget *main_window,
					  int output_volume, 
					  int input_volume)
{
  GmWindow *mw = NULL;

  g_return_if_fail (main_window != NULL);

  mw = gm_mw_get_mw (main_window);

  g_return_if_fail (mw != NULL);

  if (output_volume != -1)
    GTK_ADJUSTMENT (mw->adj_output_volume)->value = output_volume;

  if (input_volume != -1)
    GTK_ADJUSTMENT (mw->adj_input_volume)->value = input_volume;
  
  gtk_widget_queue_draw (GTK_WIDGET (mw->audio_volume_frame));
}


void
gm_main_window_get_volume_sliders_values (GtkWidget *main_window,
					  int &output_volume, 
					  int &input_volume)
{
  GmWindow *mw = NULL;

  g_return_if_fail (main_window != NULL);

  mw = gm_mw_get_mw (main_window);

  g_return_if_fail (mw != NULL);

  output_volume = (int) GTK_ADJUSTMENT (mw->adj_output_volume)->value;
  input_volume = (int) GTK_ADJUSTMENT (mw->adj_input_volume)->value;
}


void
gm_main_window_set_video_sliders_values (GtkWidget *main_window,
					 int whiteness,
					 int brightness,
					 int colour,
					 int contrast)
{
  GmWindow *mw = NULL;

  g_return_if_fail (main_window != NULL);

  mw = gm_mw_get_mw (main_window);

  g_return_if_fail (mw != NULL);

  if (whiteness != -1)
    GTK_ADJUSTMENT (mw->adj_whiteness)->value = whiteness;

  if (brightness != -1)
    GTK_ADJUSTMENT (mw->adj_brightness)->value = brightness;
  
  if (colour != -1)
    GTK_ADJUSTMENT (mw->adj_colour)->value = colour;
  
  if (contrast != -1)
    GTK_ADJUSTMENT (mw->adj_contrast)->value = contrast;
  
  gtk_widget_queue_draw (GTK_WIDGET (mw->video_settings_frame));
}


void
gm_main_window_get_video_sliders_values (GtkWidget *main_window,
					 int &whiteness, 
					 int &brightness,
					 int &colour,
					 int &contrast)
{
  GmWindow *mw = NULL;

  g_return_if_fail (main_window != NULL);

  mw = gm_mw_get_mw (main_window);

  g_return_if_fail (mw != NULL);

  whiteness = (int) GTK_ADJUSTMENT (mw->adj_whiteness)->value;
  brightness = (int) GTK_ADJUSTMENT (mw->adj_brightness)->value;
  colour = (int) GTK_ADJUSTMENT (mw->adj_colour)->value;
  contrast = (int) GTK_ADJUSTMENT (mw->adj_contrast)->value;
}


GtkWidget *
gm_main_window_new (GmWindow *mw)
{
  GtkWidget *window = NULL;
  GtkWidget *table = NULL;	
  GtkWidget *frame = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *hbox = NULL;
  GdkPixbuf *pixbuf = NULL;
  GtkAccelGroup *accel = NULL;
#ifdef DISABLE_GNOME
  GtkWidget *window_vbox = NULL;
  GtkWidget *window_hbox = NULL;
#endif
  GtkWidget *event_box = NULL;
  GtkWidget *main_toolbar = NULL;
  GtkWidget *left_toolbar = NULL;
  GtkWidget *chat_window = NULL;

  int main_notebook_section = 0;
  
  /* The Top-level window */
#ifndef DISABLE_GNOME
  window = gnome_app_new ("gnomemeeting", NULL);
#else
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
#endif
  //FIXME
  gm = window;
  g_object_set_data_full (G_OBJECT (window), "window_name",
			  g_strdup ("main_window"), g_free);

  gtk_window_set_title (GTK_WINDOW (window), 
			_("GnomeMeeting"));
  gtk_window_set_position (GTK_WINDOW (window), 
			   GTK_WIN_POS_CENTER);


  /* The GMObject data */
  //mw = new GmWindow ();
  g_object_set_data_full (G_OBJECT (window), "GMObject", 
			  mw, (GDestroyNotify) gm_mw_destroy);

  
  accel = gtk_accel_group_new ();
  gtk_window_add_accel_group (GTK_WINDOW (window), accel);

#ifdef DISABLE_GNOME
  window_vbox = gtk_vbox_new (0, FALSE);
  gtk_container_add (GTK_CONTAINER (window), window_vbox);
  gtk_widget_show (window_vbox);
#endif

  
  /* The main menu */
  mw->statusbar = gtk_statusbar_new ();
  mw->main_menu = gnomemeeting_init_menu (accel);
#ifndef DISABLE_GNOME
  gnome_app_add_docked (GNOME_APP (window), 
			mw->main_menu,
			"menubar",
			BONOBO_DOCK_ITEM_BEH_EXCLUSIVE,
  			BONOBO_DOCK_TOP, 0, 0, 0);
#else
  gtk_box_pack_start (GTK_BOX (window_vbox), mw->main_menu,
		      FALSE, FALSE, 0);
#endif


  /* The main and left toolbar */
  main_toolbar = gnomemeeting_init_main_toolbar ();
#ifndef DISABLE_GNOME
  gnome_app_add_docked (GNOME_APP (window), main_toolbar, "main_toolbar",
  			BONOBO_DOCK_ITEM_BEH_EXCLUSIVE,
  			BONOBO_DOCK_TOP, 1, 0, 0);
#else
  gtk_box_pack_start (GTK_BOX (window_vbox), main_toolbar, 
		      FALSE, FALSE, 0);
#endif

  left_toolbar = gnomemeeting_init_left_toolbar ();
#ifndef DISABLE_GNOME
  gnome_app_add_toolbar (GNOME_APP (window), GTK_TOOLBAR (left_toolbar),
 			 "left_toolbar", BONOBO_DOCK_ITEM_BEH_EXCLUSIVE,
 			 BONOBO_DOCK_LEFT, 2, 0, 0);
#else
  window_hbox = gtk_hbox_new (0, FALSE);
  gtk_box_pack_start (GTK_BOX (window_vbox), window_hbox, 
		      FALSE, FALSE, 0);

  gtk_box_pack_start (GTK_BOX (window_hbox), left_toolbar, 
		      FALSE, FALSE, 0);
  gtk_widget_show (window_hbox);
#endif

  gtk_widget_show (main_toolbar);
  gtk_widget_show (left_toolbar);

  
  /* Create a table in the main window to attach things like buttons */
  table = gtk_table_new (3, 4, FALSE);
#ifdef DISABLE_GNOME
  gtk_box_pack_start (GTK_BOX (window_hbox), table, FALSE, FALSE, 0);
#else
  gnome_app_set_contents (GNOME_APP (window), table);
#endif
  gtk_widget_show (table);

  /* The Notebook */
  mw->main_notebook = gtk_notebook_new ();
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (mw->main_notebook), GTK_POS_BOTTOM);
  gtk_notebook_popup_enable (GTK_NOTEBOOK (mw->main_notebook));
  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (mw->main_notebook), TRUE);
  gtk_notebook_set_scrollable (GTK_NOTEBOOK (mw->main_notebook), TRUE);

  gm_mw_init_stats (window);
  gm_mw_init_dialpad (window);
  gm_mw_init_audio_settings (window);
  gm_mw_init_video_settings (window);

  gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (mw->main_notebook),
		    0, 2, 2, 3,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    6, 6); 

  main_notebook_section = 
    gm_conf_get_int (USER_INTERFACE_KEY "main_window/control_panel_section");

  if (main_notebook_section != GM_MAIN_NOTEBOOK_HIDDEN) {

    gtk_widget_show_all (GTK_WIDGET (mw->main_notebook));
    gtk_notebook_set_current_page (GTK_NOTEBOOK ((mw->main_notebook)), 
				   main_notebook_section);
  }


  /* The frame that contains video and remote name display */
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);

  /* The frame that contains the video */
  mw->video_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (mw->video_frame), GTK_SHADOW_IN);
  
  event_box = gtk_event_box_new ();
  mw->video_popup_menu = gnomemeeting_video_popup_init_menu (event_box, accel);

  vbox = gtk_vbox_new (FALSE, 0);

  gtk_container_add (GTK_CONTAINER (frame), event_box);
  gtk_container_add (GTK_CONTAINER (event_box), vbox);
  gtk_box_pack_start (GTK_BOX (vbox), mw->video_frame, TRUE, TRUE, 0);

  mw->main_video_image = gtk_image_new ();
  gtk_container_set_border_width (GTK_CONTAINER (mw->video_frame), 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 0);
  gtk_container_add (GTK_CONTAINER (mw->video_frame), mw->main_video_image);

  gtk_widget_set_size_request (GTK_WIDGET (mw->video_frame), 
			       GM_QCIF_WIDTH + GM_FRAME_SIZE, 
			       GM_QCIF_HEIGHT + GM_FRAME_SIZE); 

  gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (frame), 
		    0, 2, 0, 1,
		    (GtkAttachOptions) GTK_EXPAND,
		    (GtkAttachOptions) GTK_EXPAND,
		    6, 6);

  gtk_widget_show_all (GTK_WIDGET (frame));

  
  /* The statusbar and the progressbar */
  hbox = gtk_hbox_new (0, FALSE);
#ifdef DISABLE_GNOME
  gtk_box_pack_start (GTK_BOX (window_vbox), hbox, 
		      FALSE, FALSE, 0);
#else
  gnome_app_add_docked (GNOME_APP (window), hbox, "statusbar",
  			BONOBO_DOCK_ITEM_BEH_EXCLUSIVE,
  			BONOBO_DOCK_BOTTOM, 3, 0, 0);
#endif
  gtk_widget_show (hbox);

  
  gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (mw->statusbar), FALSE);
  gtk_box_pack_start (GTK_BOX (hbox), mw->statusbar, 
		      TRUE, TRUE, 0);

  if (gm_conf_get_bool (USER_INTERFACE_KEY "main_window/show_status_bar"))
    gtk_widget_show (GTK_WIDGET (mw->statusbar));
  else
    gtk_widget_hide (GTK_WIDGET (mw->statusbar));
  
  
  /* The 2 video window popups */
  mw->local_video_window =
    gnomemeeting_video_window_new (_("Local Video"),
				   mw->local_video_image,
				   "local_video_window");
  mw->remote_video_window =
    gnomemeeting_video_window_new (_("Remote Video"),
				   mw->remote_video_image,
				   "remote_video_window");
  
  gm_main_window_update_logo (window);

  g_signal_connect (G_OBJECT (mw->local_video_window), "show", 
		    GTK_SIGNAL_FUNC (video_window_shown_cb), NULL);
  g_signal_connect (G_OBJECT (mw->remote_video_window), "show", 
		    GTK_SIGNAL_FUNC (video_window_shown_cb), NULL);


  /* The remote name */
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);

  mw->remote_name = gtk_label_new (NULL);
  gtk_widget_set_size_request (GTK_WIDGET (mw->remote_name), 
			       GM_QCIF_WIDTH, -1);

  gtk_container_add (GTK_CONTAINER (frame), mw->remote_name);
  gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);
  gtk_widget_show_all (GTK_WIDGET (frame));


  /* The Chat Window */
  chat_window = GnomeMeeting::Process ()->GetMainWindow ()->chat_window;
  gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (chat_window), 
 		    2, 4, 0, 3,
 		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
 		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
 		    6, 6);
  if (gm_conf_get_bool (USER_INTERFACE_KEY "main_window/show_chat_window"))
    gtk_widget_show_all (GTK_WIDGET (chat_window));
  
  gtk_widget_set_size_request (GTK_WIDGET (mw->main_notebook),
			       GM_QCIF_WIDTH + GM_FRAME_SIZE, -1);
  gtk_widget_set_size_request (GTK_WIDGET (window), -1, -1);

  
  /* Add the window icon and title */
  gtk_window_set_title (GTK_WINDOW (window), _("GnomeMeeting"));
  pixbuf = 
    gdk_pixbuf_new_from_file (GNOMEMEETING_IMAGES
			      "gnomemeeting-logo-icon.png", NULL);
  gtk_window_set_icon (GTK_WINDOW (window), pixbuf);
  gtk_widget_realize (window);
  g_object_unref (G_OBJECT (pixbuf));
  gtk_window_set_resizable (GTK_WINDOW (window), false);

  g_signal_connect_after (G_OBJECT (mw->main_notebook), "switch-page",
			  G_CALLBACK (control_panel_section_changed_cb), 
			  window);


  /* Init the Drag and drop features */
  gm_contacts_dnd_set_dest (GTK_WIDGET (window), dnd_call_contact_cb, mw);

  /* if the user tries to close the window : delete_event */
  g_signal_connect (G_OBJECT (window), "delete_event",
		    G_CALLBACK (window_closed_cb), 
		    (gpointer) window);

  g_signal_connect (G_OBJECT (window), "show", 
		    GTK_SIGNAL_FUNC (video_window_shown_cb), NULL);
  
  return window;
}


/* The main () */

int main (int argc, char ** argv, char ** envp)
{
  PProcess::PreInitialise (argc, argv, envp);

  GtkWidget *dialog = NULL;
  
  GmWindow *mw = NULL;

  gchar *url = NULL;
  gchar *key_name = NULL;
  gchar *msg = NULL;

  int debug_level = 0;

  
#ifndef WIN32
  setenv ("ESD_NO_SPAWN", "1", 1);
#endif
  

  /* Threads + Locale Init + config */
  g_thread_init (NULL);
  gdk_threads_init ();
  
#ifndef WIN32
  gtk_init (&argc, &argv);
#else
  gtk_init (NULL, NULL);
#endif

  xmlInitParser ();

  gm_conf_init (argc, argv);

  
  /* Upgrade the preferences */
  gnomemeeting_conf_upgrade ();

  /* Initialize gettext */
  textdomain (GETTEXT_PACKAGE);
  bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");

  
  /* Select the Mic as default source for OSS. Will be removed when
   * ALSA will be everywhere 
   */
  gnomemeeting_mixers_mic_select ();
  
#ifndef DISABLE_GNOME
  /* Cope with command line options */
  struct poptOption arguments [] =
    {
      {"debug", 'd', POPT_ARG_INT, &debug_level, 
       1, N_("Prints debug messages in the console (level between 1 and 6)"), 
       NULL},
      {"call", 'c', POPT_ARG_STRING, &url,
       1, N_("Makes GnomeMeeting call the given URL"), NULL},
      {NULL, '\0', 0, NULL, 0, NULL, NULL}
    };
  
  /* GnomeMeeting Initialisation */
  gnome_program_init ("gnomemeeting", VERSION,
		      LIBGNOMEUI_MODULE, argc, argv,
		      GNOME_PARAM_POPT_TABLE, arguments,
		      GNOME_PARAM_HUMAN_READABLE_NAME,
		      "gnomemeeting",
		      GNOME_PARAM_APP_DATADIR, GNOMEMEETING_DATADIR,
		      (void *) NULL);
#endif
  
  gdk_threads_enter ();
 
  /* The factory */
#ifndef DISABLE_GNOME
  if (gnomemeeting_invoke_factory (argc, argv))
    exit (1);
#endif

  /* GnomeMeeting main initialisation */
  static GnomeMeeting instance;

  /* Debug */
  if (debug_level != 0)
    PTrace::Initialise (PMAX (PMIN (4, debug_level), 0), NULL,
			PTrace::Timestamp | PTrace::Thread
			| PTrace::Blocks | PTrace::DateAndTime);

  
  /* Detect the devices, exit if it fails */
  if (!GnomeMeeting::Process ()->DetectDevices ()) {

    dialog = gnomemeeting_error_dialog (NULL, _("No usable audio manager detected"), _("GnomeMeeting didn't find any usable sound manager. Make sure that your installation is correct."));
    
    g_signal_handlers_disconnect_by_func (G_OBJECT (dialog),
					  (gpointer) gtk_widget_destroy,
					  G_OBJECT (dialog));

    gtk_dialog_run (GTK_DIALOG (dialog));
    exit (-1);
  }


  /* Init the process and build the GUI */
  GnomeMeeting::Process ()->BuildGUI ();
  GnomeMeeting::Process ()->Init ();


  /* Init the config DB, exit if it fails */
  if (!gnomemeeting_conf_init ()) {

    key_name = g_strdup ("\"/apps/gnomemeeting/general/gconf_test_age\"");
    msg = g_strdup_printf (_("GnomeMeeting got an invalid value for the GConf key %s.\n\nIt probably means that your GConf schemas have not been correctly installed or the that permissions are not correct.\n\nPlease check the FAQ (http://www.gnomemeeting.org/faq.php), the throubleshoot section of the GConf site (http://www.gnome.org/projects/gconf/) or the mailing list archives for more information (http://mail.gnome.org) about this problem."), key_name);
    
    dialog = gnomemeeting_error_dialog (GTK_WINDOW (gm),
					_("Gconf key error"), msg);

    g_signal_handlers_disconnect_by_func (G_OBJECT (dialog),
					  (gpointer) gtk_widget_destroy,
					  G_OBJECT (dialog));


    g_free (msg);
    g_free (key_name);
    
    gtk_dialog_run (GTK_DIALOG (dialog));
    exit (-1);
  }

  
  /* Call the given host if needed */
  if (url) {

    mw = GnomeMeeting::Process ()->GetMainWindow ();
    gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (mw->combo)->entry), url);
    connect_cb (NULL, NULL);
  }

  //  gtk_timeout_add (15000, (GtkFunction) StressTest, 
  //		   NULL);
  
  /* The GTK loop */
  gtk_main ();
  gdk_threads_leave ();

  //  delete (GnomeMeeting::Process ());
  
  gm_conf_save ();

  return 0;
}


#ifdef WIN32
int APIENTRY WinMain(HINSTANCE hInstance,
		     HINSTANCE hPrevInstance,
		     LPSTR     lpCmdLine,
		     int       nCmdShow)
{
  return main (0, NULL, NULL);
}
#endif
