
/*
 * GnomeMeeting -- A Video-Conferencing application
 *
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
 *                         main_interface.cpp  -  description
 *                         ----------------------------------
 *   begin                : Mon Mar 26 2001
 *   copyright            : (C) 2000-2001 by Damien Sandras
 *   description          : This file contains all the functions needed to
 *                          build the main window.
 *   email                : dsandras@seconix.com
 */

#include "../config.h"


#include "main_window.h"
#include "gnomemeeting.h"
#include "callbacks.h"
#include "tray.h"
#include "ils.h"
#include "common.h"
#include "menu.h"
#include "toolbar.h"
#include "callbacks.h"
#include "sound_handling.h"
#include "videograbber.h"
#include "endpoint.h"
#include "pref_window.h"
#include "config.h"
#include "misc.h"
#include "e-splash.h"
#include "dialog.h"
#include "stock-icons.h"
#include "druid.h"
#include "chat_window.h"
#include "tools.h"

#include <ptclib/asner.h>
#include <libgnomeui/gnome-window-icon.h>
#include <bonobo-activation/bonobo-activation-activate.h>
#include <bonobo-activation/bonobo-activation-register.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-listener.h>
#include <gdk/gdkx.h>
#include <gconf/gconf-client.h>


#include "../pixmaps/brightness.xpm"
#include "../pixmaps/whiteness.xpm"
#include "../pixmaps/contrast.xpm"
#include "../pixmaps/color.xpm"

#ifdef __FreeBSD__
#include <libintl.h>
#endif

#define ACT_IID "OAFIID:GNOME_gnomemeeting_Factory"

#define GENERAL_KEY         "/apps/gnomemeeting/general/"
#define VIEW_KEY            "/apps/gnomemeeting/view/"
#define DEVICE_KEY         "/apps/gnomemeeting/devices/"
#define PERSONAL_KEY   "/apps/gnomemeeting/personal_data/"
#define LDAP_KEY            "/apps/gnomemeeting/ldap/"
#define GATEKEEPER_KEY      "/apps/gnomemeeting/gatekeeper/"


/* Declarations */

extern GtkWidget *gm;
extern GnomeMeeting *MyApp;	


static gboolean stats_drawing_area_exposed (GtkWidget *);

static gboolean gnomemeeting_invoke_factory (int, char **);
static void gnomemeeting_new_event (BonoboListener *, const char *, 
				    const CORBA_any *, CORBA_Environment *,
				    gpointer);
static Bonobo_RegistrationResult gnomemeeting_register_as_factory (void);

static void main_notebook_page_changed (GtkNotebook *, GtkNotebookPage *,
					gint, gpointer);
static void audio_volume_changed       (GtkAdjustment *, gpointer);
static void brightness_changed         (GtkAdjustment *, gpointer);
static void whiteness_changed          (GtkAdjustment *, gpointer);
static void colour_changed             (GtkAdjustment *, gpointer);
static void contrast_changed           (GtkAdjustment *, gpointer);

static void gnomemeeting_init_main_window ();
static gint gm_quit_callback (GtkWidget *, GdkEvent *, gpointer);
static void gnomemeeting_init_main_window_video_settings ();
static void gnomemeeting_init_main_window_audio_settings ();
static void gnomemeeting_init_main_window_stats ();

/* For stress testing */
int i = 0;

/* GTK Callbacks */
gint StressTest (gpointer data)
 {
   gdk_threads_enter ();


   GmWindow *gw = gnomemeeting_get_main_window (gm);

   if (!GTK_TOGGLE_BUTTON (gw->connect_button)->active) {

     i++;
     cout << "Call " << i << endl << flush;
   }

   gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gw->connect_button), 
 				!GTK_TOGGLE_BUTTON (gw->connect_button)->active);

   gdk_threads_leave ();
   return TRUE;
}



gint AppbarUpdate (gpointer data)
{
  long minutes, seconds;
  RTP_Session *audio_session = NULL;
  RTP_Session *video_session = NULL;
  H323Connection *connection = NULL;
  GmRtpData *rtp = (GmRtpData *) data; 
  GmWindow *gw = NULL;
  

  if (MyApp->Endpoint ()) {
    
    PString current_call_token = MyApp->Endpoint ()->GetCurrentCallToken ();

    if (current_call_token.IsEmpty ())
      return TRUE;
    
    gdk_threads_enter ();

    gw = gnomemeeting_get_main_window (gm);

    connection = MyApp->Endpoint ()->GetCurrentConnection ();

    if ((connection)&&(MyApp->Endpoint ()->GetCallingState () == 2)) {

      PTimeInterval t =
	PTime () - connection->GetConnectionStartTime();

      if (t.GetSeconds () == 4) {

	for (int cpt = 0 ; cpt < 50 ; cpt++) {

	  rtp->tr_audio_speed [cpt] = 0;
	  rtp->re_audio_speed [cpt] = 0;
	  rtp->tr_video_speed [cpt] = 0;
	  rtp->re_video_speed [cpt] = 0;
	  rtp->tr_audio_pos = 0;
	  rtp->re_audio_pos = 0;
	  rtp->tr_video_pos = 0;
	  rtp->re_video_pos = 0;
	}
      }
      
      audio_session = 
	connection->GetSession(RTP_Session::DefaultAudioSessionID);
	  
      video_session = 
	connection->GetSession(RTP_Session::DefaultVideoSessionID);
	  
      if (audio_session != NULL) {

	/* Compute the current transmitted audio speed */
	if ((rtp->tr_audio_bytes == 0) && (rtp->tr_audio_pos == 0))
	  /* Default value for the 1st element */
	  rtp->tr_audio_bytes = audio_session->GetOctetsSent();
	rtp->tr_audio_speed [rtp->tr_audio_pos] = 
	  (float) (audio_session->GetOctetsSent()
		   - rtp->tr_audio_bytes)/1024;
	rtp->tr_audio_bytes = 
	  audio_session->GetOctetsSent();	

	rtp->tr_audio_pos++;
	if (rtp->tr_audio_pos >= 50) rtp->tr_audio_pos = 0;

	/* Compute the current received audio speed */
	if ((rtp->re_audio_bytes == 0) && (rtp->re_audio_pos == 0))
	  /* Default value for the 1st element */
	  rtp->re_audio_bytes = audio_session->GetOctetsReceived();
	rtp->re_audio_speed [rtp->re_audio_pos] = 
	  (float) (audio_session->GetOctetsReceived()
		   - rtp->re_audio_bytes)/1024;
	rtp->re_audio_bytes = 
	  audio_session->GetOctetsReceived();

	rtp->re_audio_pos++;
	if (rtp->re_audio_pos >= 50) rtp->re_audio_pos = 0;
      }

      if (video_session != NULL) {

	/* Compute the current transmitted video speed */
	if ((rtp->tr_video_bytes == 0) && (rtp->tr_video_pos == 0)) 
	  /* Default value for the 1st element */
	  rtp->tr_video_bytes = video_session->GetOctetsSent();
	rtp->tr_video_speed [rtp->tr_video_pos] = 
	  (float) (video_session->GetOctetsSent()
		   - rtp->tr_video_bytes)/1024;
	rtp->tr_video_bytes = 
	  video_session->GetOctetsSent();

	rtp->tr_video_pos++;
	if (rtp->tr_video_pos >= 50) rtp->tr_video_pos = 0;

	/* Compute the current received video speed */
	if ((rtp->re_video_bytes == 0) && (rtp->re_video_pos == 0)) 
	  /* Default value for the 1st element */
	  rtp->re_video_bytes = video_session->GetOctetsReceived();
	rtp->re_video_speed [rtp->re_video_pos] = 
	  (float) (video_session->GetOctetsReceived()
		   - rtp->re_video_bytes)/1024;
	rtp->re_video_bytes = 
	  video_session->GetOctetsReceived();

	rtp->re_video_pos++;
	if (rtp->re_video_pos >= 50) rtp->re_video_pos = 0;
      }

      minutes = t.GetMinutes () % 60;
      seconds = t.GetSeconds () % 60;
	
      gchar *msg = g_strdup_printf 
	(_("%.2ld:%.2ld:%.2ld  A:%.2f/%.2f   V:%.2f/%.2f"), 
	 (long) t.GetHours (), (long) minutes, (long) seconds, 
	 rtp->tr_audio_speed [rtp->tr_audio_pos - 1], 
	 rtp->re_audio_speed [rtp->re_audio_pos - 1],
	 rtp->tr_video_speed [rtp->tr_video_pos - 1], 
	 rtp->re_video_speed [rtp->re_video_pos - 1]);
	
      float bytes_sent = 0;
      float bytes_received = 0;
      int lost_packets = 0;
      int late_packets = 0;

      if (audio_session) {
	  
	bytes_sent = (float) audio_session->GetOctetsSent ();
	bytes_received = (float) audio_session->GetOctetsReceived ();
	lost_packets = audio_session->GetPacketsLost ();
	late_packets = audio_session->GetPacketsTooLate ();
      }
	
      if (video_session) {

	bytes_sent = (float) bytes_sent+video_session->GetOctetsSent ();
	bytes_received = 
	  (float) bytes_received+video_session->GetOctetsReceived ();
	lost_packets = lost_packets+video_session->GetPacketsLost ();
	late_packets = late_packets+video_session->GetPacketsTooLate ();
      }

      bytes_sent = (float) bytes_sent / 1024 / 1024;
      bytes_received = (float) bytes_received / 1024 / 1024;

      if (t.GetSeconds () > 5) {

	gnome_appbar_clear_stack (GNOME_APPBAR (gw->statusbar));
	gnome_appbar_push (GNOME_APPBAR (gw->statusbar), msg);
	gchar *stats_msg = 
	  g_strdup_printf (_("Sent/Received: %.3f Mb/%.3f Mb\nLost/Late Packets: %d/%d\nRound trip delay: %d ms"), bytes_sent, bytes_received, lost_packets, late_packets, (int) connection->GetRoundTripDelay().GetMilliSeconds());

	gtk_label_set_text (GTK_LABEL (gw->stats_label), stats_msg);
	g_free (stats_msg);

	gtk_widget_queue_draw_area (gw->stats_drawing_area, 0, 0, GTK_WIDGET (gw->stats_drawing_area)->allocation.width, GTK_WIDGET (gw->stats_drawing_area)->allocation.height);
      }
        
      g_free (msg);
    }

    gdk_threads_leave ();
  }


  return TRUE;
}


static gboolean 
stats_drawing_area_exposed (GtkWidget *drawing_area)
{
  GdkColor black_clr;
  GdkColor cyan_clr;
  GdkColor green_clr;
  GdkColor red_clr;
  GdkColor blue_clr;
  GdkColor yellow_clr;
  GdkSegment s [50];
  GdkGC *gc;
  GdkColormap *colormap = NULL;
  PangoLayout *layout = NULL;
  PangoContext *pango_context = NULL;
  gchar *pango_text = NULL;
  int pango_text_size = 0;
  int size = 0;
  int x = 21, y = 21;
  int cpt = 0;
  int pos = 0;

  GmRtpData *rtp = gnomemeeting_get_rtp_data (gm); 
  GdkPoint points [50];

  int width_step = GTK_WIDGET (drawing_area)->allocation.width / 50;
  int allocation_height = GTK_WIDGET (drawing_area)->allocation.height - 15;
  float height_step = allocation_height;

  black_clr.red = 0;
  black_clr.green = 0;
  black_clr.blue = 0;

  cyan_clr.red = 169;
  cyan_clr.green = 38809;
  cyan_clr.blue = 52441;

  green_clr.red = 0;
  green_clr.green = 65535;
  green_clr.blue = 0;
  
  red_clr.red = 65535;
  red_clr.green = 0;
  red_clr.blue = 0;

  blue_clr.red = 0;
  blue_clr.green = 0;
  blue_clr.blue = 65535;

  yellow_clr.red = 65535;
  yellow_clr.green = 54756;
  yellow_clr.blue = 0;

  colormap = gdk_drawable_get_colormap (drawing_area->window);
  gc = gdk_gc_new (drawing_area->window);
  gdk_colormap_alloc_color (colormap, &black_clr, FALSE, TRUE);
  gdk_colormap_alloc_color (colormap, &cyan_clr, FALSE, TRUE);
  gdk_colormap_alloc_color (colormap, &green_clr, FALSE, TRUE);
  gdk_colormap_alloc_color (colormap, &red_clr, FALSE, TRUE);
  gdk_colormap_alloc_color (colormap, &blue_clr, FALSE, TRUE);
  gdk_colormap_alloc_color (colormap, &yellow_clr, FALSE, TRUE);

  gdk_gc_set_foreground (gc, &black_clr);
  gdk_draw_rectangle (drawing_area->window,
		      gc,
		      TRUE, 0, 0, 
		      GTK_WIDGET (drawing_area)->allocation.width,
		      GTK_WIDGET (drawing_area)->allocation.height);

  gdk_gc_set_foreground (gc, &cyan_clr);

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
  gdk_window_set_background (drawing_area->window, &black_clr);

  gdk_gc_set_line_attributes (gc, 2, GDK_LINE_SOLID, 
			      GDK_CAP_ROUND, GDK_JOIN_BEVEL);
  
  float max_tr_video = 1;
  float max_tr_audio = 1;
  float max_re_video = 1;
  float max_re_audio = 1;

  /* Compute the height_step */
  if (MyApp->Endpoint ()->GetCallingState () == 2) {

    for (int cpt = 0 ; cpt < 50 ; cpt++) {
    
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


    pango_context = gtk_widget_get_pango_context (GTK_WIDGET (drawing_area));

    
    /* Transmitted audio */
    layout = pango_layout_new (pango_context);
    pango_text = g_strdup (_("Tr. Aud."));
    pango_layout_set_text (layout, pango_text, strlen (pango_text));
    gdk_draw_layout_with_colors (GDK_DRAWABLE (drawing_area->window), 
				 gc, pango_text_size, 0,
				 layout, &red_clr, &black_clr);
    pango_layout_get_pixel_size (layout, &size, NULL);
    pango_text_size += (size + 10);
    g_object_unref (layout);
    g_free (pango_text);

    gdk_gc_set_foreground (gc, &red_clr);
    pos = rtp->tr_audio_pos;
    for (int cpt = 0 ; cpt < 50 ; cpt++) {

      points [cpt].x = cpt * width_step;

      points [cpt].y = allocation_height + 15 -
	(gint) (rtp->tr_audio_speed [pos] * height_step);
      pos++;

      if (pos >= 50) pos = 0;
    }
    gdk_draw_lines (GDK_DRAWABLE (drawing_area->window), gc, points, 50);


    /* Received audio */
    layout = pango_layout_new (pango_context);
    pango_text = g_strdup (_("Re. Aud."));
    pango_layout_set_text (layout, pango_text, strlen (pango_text));
    gdk_draw_layout_with_colors (GDK_DRAWABLE (drawing_area->window), 
				 gc, pango_text_size, 0,
				 layout, &yellow_clr, &black_clr);
    pango_layout_get_pixel_size (layout, &size, NULL);
    pango_text_size += (size + 10);
    g_object_unref (layout);
    g_free (pango_text);

    gdk_gc_set_foreground (gc, &yellow_clr);
    pos = rtp->re_audio_pos;
    for (int cpt = 0 ; cpt < 50 ; cpt++) {

      points [cpt].x = cpt * width_step;

      points [cpt].y = allocation_height + 15 -
	(gint) (rtp->re_audio_speed [pos] * height_step);
      pos++;

      if (pos >= 50) pos = 0;
    }
    gdk_draw_lines (GDK_DRAWABLE (drawing_area->window), gc, points, 50);


    /* Transmitted video */
    layout = pango_layout_new (pango_context);
    pango_text = g_strdup (_("Tr. Vid."));
    pango_layout_set_text (layout, pango_text, strlen (pango_text));
    gdk_draw_layout_with_colors (GDK_DRAWABLE (drawing_area->window), 
				 gc, pango_text_size, 0,
				 layout, &blue_clr, &black_clr);
    pango_layout_get_pixel_size (layout, &size, NULL);
    pango_text_size += (size + 10);
    g_object_unref (layout);
    g_free (pango_text);

    gdk_gc_set_foreground (gc, &blue_clr);
    pos = rtp->tr_video_pos;
    for (int cpt = 0 ; cpt < 50 ; cpt++) {

      points [cpt].x = cpt * width_step;

      points [cpt].y = allocation_height + 15 -
	(gint) (rtp->tr_video_speed [pos] * height_step);
      pos++;

      if (pos >= 50) pos = 0;
    }
    gdk_draw_lines (GDK_DRAWABLE (drawing_area->window), gc, points, 50);


    /* Received video */
    layout = pango_layout_new (pango_context);
    pango_text = g_strdup (_("Re. Vid."));
    pango_layout_set_text (layout, pango_text, strlen (pango_text));
    gdk_draw_layout_with_colors (GDK_DRAWABLE (drawing_area->window), 
				 gc, pango_text_size, 0,
				 layout, &green_clr, &black_clr);
    pango_layout_get_pixel_size (layout, &size, NULL);
    pango_text_size += (size + 10);
    g_object_unref (layout);
    g_free (pango_text);

    gdk_gc_set_foreground (gc, &green_clr);
    pos = rtp->re_video_pos;
    for (int cpt = 0 ; cpt < 50 ; cpt++) {

      points [cpt].x = cpt * width_step;

      points [cpt].y = allocation_height + 15 -
	(gint) (rtp->re_video_speed [pos] * height_step);
      pos++;

      if (pos >= 50) pos = 0;
    }
    gdk_draw_lines (GDK_DRAWABLE (drawing_area->window), gc, points, 50);
  }
  else {

    for (int cpt = 0 ; cpt < 50 ; cpt++) {

      rtp->re_audio_speed [pos] = 0;
      rtp->re_video_speed [pos] = 0;
      rtp->tr_audio_speed [pos] = 0;
      rtp->tr_video_speed [pos] = 0;
    }    
  }


  g_object_unref (gc);
  g_object_unref (colormap);

  return TRUE;
}


/* Factory stuff */


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
  
  GmWindow *gw = gnomemeeting_get_main_window (gm);

  args = (CORBA_sequence_CORBA_string *) any->_value;
  argc = args->_length;
  argv = args->_buffer;

  if (strcmp (event_name, "new_gnomemeeting")) {

      g_warning ("Unknown event '%s' on GnomeMeeting", event_name);
      return;
  }

  for (i = 0; i < argc; i++) {
    if (!strcmp (argv [i], "-c") || !strcmp (argv [i], "--callto"))
      break;
  } 


  if ((i < argc) && (i + 1 < argc) && (argv [i+1])) {
    
     /* this function will store a copy of text */
    if (MyApp->Endpoint ()->GetCallingState () == 0) {
      
      gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (gw->combo)->entry), 
			  argv [i + 1]);  
    }
    
    connect_cb (NULL, NULL);
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
      g_printerr (_("Error registering gnomemeeting with the activation service; factory mode disabled.\n"));
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


/**
 * DESCRIPTION  :  This callback is called when the user changes the
 *                 page in the main notebook.
 * BEHAVIOR     :  Update the gconf key accordingly.
 * PRE          :  /
 **/
static void 
main_notebook_page_changed (GtkNotebook *notebook, GtkNotebookPage *page,
			    gint page_num, gpointer user_data) 
{
  GConfClient *client = gconf_client_get_default ();

  GmWindow *gw = gnomemeeting_get_main_window (gm);

  gconf_client_set_int (client, VIEW_KEY "control_panel_section",
			gtk_notebook_get_current_page (GTK_NOTEBOOK (gw->main_notebook)), 0);
}


/**
 * DESCRIPTION  :  This callback is called when the user changes the
 *                 audio settings sliders in the main notebook.
 * BEHAVIOR     :  Update the volume.
 * PRE          :  gpointer is a valid pointer to GmPrefWindow
 **/
void 
audio_volume_changed (GtkAdjustment *adjustment, gpointer data)
{
  GConfClient *client = gconf_client_get_default ();
  int vol_play, vol_rec;
  char *audio_recorder_mixer;
  char *audio_player_mixer;

  GmWindow *gw = GM_WINDOW (data);
  
  vol_play =  (int) (GTK_ADJUSTMENT (gw->adj_play)->value) * 257;
  vol_rec =  (int) (GTK_ADJUSTMENT (gw->adj_rec)->value) * 257;

  /* return a pointer to the data, not a copy => no need to free */
  audio_player_mixer = gconf_client_get_string (client, DEVICE_KEY "audio_player_mixer", NULL);
  audio_recorder_mixer = gconf_client_get_string (client, DEVICE_KEY "audio_recorder_mixer", NULL);
  
  gnomemeeting_volume_set (audio_player_mixer, 0, &vol_play);
  gnomemeeting_volume_set (audio_recorder_mixer, 1, &vol_rec);

  g_free (audio_player_mixer);
  g_free (audio_recorder_mixer);
}


/**
 * DESCRIPTION  :  This callback is called when the user changes the 
 *                 video brightness slider in the main notebook.
 * BEHAVIOR     :  Update the value in real time.
 * PRE          :  gpointer is a valid pointer to GmWindow
 */
void 
brightness_changed (GtkAdjustment *adjustment, gpointer data)
{ 
  GmWindow *gw = GM_WINDOW (data);
  GMVideoGrabber *video_grabber = MyApp->Endpoint ()->GetVideoGrabber ();

  int brightness;

  brightness =  (int) (GTK_ADJUSTMENT (gw->adj_brightness)->value);

  video_grabber->SetBrightness (brightness * 256);
}


/**
 * DESCRIPTION  :  This callback is called when the user changes the 
 *                 video whiteness slider in the main notebook.
 * BEHAVIOR     :  Update the value in real time.
 * PRE          :  gpointer is a valid pointer to GmWindow
 */
void 
whiteness_changed (GtkAdjustment *adjustment, gpointer data)
{ 
  GmWindow *gw = GM_WINDOW (data);
  GMVideoGrabber *video_grabber = MyApp->Endpoint ()->GetVideoGrabber ();

  int whiteness;

  whiteness =  (int) (GTK_ADJUSTMENT (gw->adj_whiteness)->value);

  video_grabber->SetWhiteness (whiteness * 256);
}


/**
 * DESCRIPTION  :  This callback is called when the user changes the 
 *                 video colour slider in the main notebook.
 * BEHAVIOR     :  Update the value in real time.
 * PRE          :  gpointer is a valid pointer to GmWindow
 */
void 
colour_changed (GtkAdjustment *adjustment, gpointer data)
{ 
  GmWindow *gw = GM_WINDOW (data);
  GMVideoGrabber *video_grabber = MyApp->Endpoint ()->GetVideoGrabber ();

  int colour;

  colour =  (int) (GTK_ADJUSTMENT (gw->adj_colour)->value);

  video_grabber->SetColour (colour * 256);
}


/**
 * DESCRIPTION  :  This callback is called when the user changes the 
 *                 video contrast slider in the main notebook.
 * BEHAVIOR     :  Update the value in real time.
 * PRE          :  gpointer is a valid pointer to GmWindow
 **/
void 
contrast_changed (GtkAdjustment *adjustment, gpointer data)
{ 
  GmWindow *gw = GM_WINDOW (data);
  GMVideoGrabber *video_grabber = MyApp->Endpoint ()->GetVideoGrabber ();

  int contrast;

  contrast =  (int) (GTK_ADJUSTMENT (gw->adj_contrast)->value);

  video_grabber->SetContrast (contrast * 256);
}


/**
 * DESCRIPTION  :  This callback is called when the user tries to close
 *                 the application using the window manager.
 * BEHAVIOR     :  Calls the real callback.
 * PRE          :  /
 **/
static gint 
gm_quit_callback (GtkWidget *widget, GdkEvent *event, 
			      gpointer data)
{
  quit_callback (NULL, data);

  return (TRUE);
}  


/* The functions */
int
gnomemeeting_window_appbar_update (gpointer data) 
{
  GtkWidget *statusbar = (GtkWidget *) data;
  
  gdk_threads_enter ();

  GtkProgressBar *progress = 
    gnome_appbar_get_progress (GNOME_APPBAR (statusbar));

  if (GTK_WIDGET_VISIBLE (GTK_WIDGET (progress)))
    gtk_progress_bar_pulse (GTK_PROGRESS_BAR (progress));

  gdk_threads_leave ();

  return 1;
}


void 
gnomemeeting_init (GmWindow *gw, 
                   GmPrefWindow *pw,
                   GmLdapWindow *lw, 
                   GmRtpData *rtp,
		   GmTextChat *chat,
		   GmCommandLineOptions *clo,
                   int argc, char ** argv, char ** envp)
{
  GMH323EndPoint *endpoint = NULL;
  gchar *firstname = NULL;
  gchar *lastname = NULL;
  gchar *alias = NULL;
  bool show_splash;
  GConfClient *client;


  /* Prevents ESD to spawn */
  setenv ("ESD_NO_SPAWN", "1", 1);


  /* Cope with command line options */
  static struct poptOption arguments[] =
    {
      {"debug", 'd', POPT_ARG_INT, &clo->debug_level, 
       1, N_("Prints debug messages in the console (level between 1 and 6)"), 
       NULL},
      {"callto", 'c', POPT_ARG_STRING, &clo->url,
       1, N_("Makes GnomeMeeting call the given callto:// URL"), NULL},
      {NULL, '\0', 0, NULL, 0, NULL, NULL}
    };


  /* Gnome Initialisation */
  gnome_program_init ("GnomeMeeting", VERSION,
		      LIBGNOMEUI_MODULE, argc, argv,
		      GNOME_PARAM_POPT_TABLE, arguments,
		      GNOME_PARAM_HUMAN_READABLE_NAME,
		      _("GnomeMeeting"),
		      GNOME_PARAM_APP_DATADIR, DATADIR,
		      (void *)NULL);

  gm = gnome_app_new ("gnomemeeting", _("GnomeMeeting"));


  /* The factory */
  if (gnomemeeting_invoke_factory (argc, argv)) {

    delete (gw);
    delete (lw);
    delete (pw);
    delete (rtp);
    delete (chat);
    exit (1);
  }


  /* Some little gconf stuff */  
  client = gconf_client_get_default ();
  gconf_client_add_dir (client, "/apps/gnomemeeting",
			GCONF_CLIENT_PRELOAD_RECURSIVE, 0);
  int gconf_test = -1;
  
  gconf_test = gconf_client_get_int (client, GENERAL_KEY "gconf_test_age", 
				     NULL);
  
  if (gconf_test != SCHEMA_AGE)  {

    /* We can't use gnomemeeting_error_dialog here, cause we need the
       dialog_run and dialog_run can't be used in gnomemeeting_error_dialog
       because it doesn't work in threads */
    gchar *buffer = g_strdup_printf (_("GnomeMeeting got %d for the GConf key \"/apps/gnomemeeting/gconf_test_age\", but %d was expected.\n\nThat key represents the revision of GnomeMeeting setting. If it doesn't correspond to the expected value, it means that your GConf schemas have not been correctly installed or the that permissions are not correct.\n\nPlease check the FAQ (http://www.gnomemeeting.org/faq.php), the throubleshoot section of the GConf site (http://www.gnome.org/projects/gconf/) or the mailing list archives for more information (http://mail.gnome.org).\n\nUsing 'gnomemeeting-config-tool' could help you to fix this problem."), gconf_test, SCHEMA_AGE);
    GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (gm),  
						GTK_DIALOG_DESTROY_WITH_PARENT,
						GTK_MESSAGE_ERROR,
						GTK_BUTTONS_OK,
						buffer);

    gtk_dialog_run (GTK_DIALOG (dialog));

    g_free (buffer);

    delete (gw);
    delete (lw);
    delete (pw);
    delete (rtp);
    delete (chat);
    exit (-1);
  }


  /* Create the global tooltips */
  gw->tips = gtk_tooltips_new ();


#ifdef SPEEX_CODEC
  /* New Speex Audio codec in 0.94 */
  if (gconf_client_get_int (client, GENERAL_KEY "version", NULL) < 94) {

    GSList *list = NULL;

    list = g_slist_append (list, (void *) "Speex-8k=1");
    list = g_slist_append (list, (void *) "MS-GSM=1");
    list = g_slist_append (list, (void *) "Speex-15k=1");
    list = g_slist_append (list, (void *) "GSM-06.10=1");
    list = g_slist_append (list, (void *) "G.726-32k=1");
    list = g_slist_append (list, (void *) "G.711-uLaw-64k=1");
    list = g_slist_append (list, (void *) "G.711-ALaw-64k=1");
    list = g_slist_append (list, (void *) "LPC10=1");
    gconf_client_set_list (GCONF_CLIENT (client),
			   "/apps/gnomemeeting/audio_codecs/codecs_list", 
			   GCONF_VALUE_STRING, list, NULL);

    g_slist_free (list);

    gnomemeeting_message_dialog (GTK_WINDOW (gm), _("GnomeMeeting just set the new Speex audio codec as default. Speex is a high quality, GPL audio codec introduced in GnomeMeeting 0.94."));
  }
#endif

  
  /* Install the URL Handler */
  gchar *gconf_url = 
    gconf_client_get_string (client, 
			     "/desktop/gnome/url-handlers/callto/command", 0);
					       
  if (gconf_url == NULL) {
    
    gconf_client_set_string (client,
			     "/desktop/gnome/url-handlers/callto/command", 
			     "gnomemeeting -c \"%s\"", NULL);
    gconf_client_set_bool (client,
			   "/desktop/gnome/url-handlers/callto/need-terminal", 
			   false, NULL);
    gconf_client_set_bool (client,
			   "/desktop/gnome/url-handlers/callto/enabled", 
			   true, NULL);

    gnomemeeting_message_dialog (GTK_WINDOW (gm), _("GnomeMeeting just installed an URL handler for callto:// URLs. callto URLs are an easy way to call people on the internet using GnomeMeeting. They are now available to all Gnome programs able to cope with URLs. You can for example create an URL launcher on the Gnome panel of the form \"callto://ils.seconix.com/me@foo.com\" to call the person registered on the ILS server ils.seconix.com with the me@foo.com e-mail address."));
  }
  g_free (gconf_url);


  /* We store all the pointers to the structure as data of gm */
  g_object_set_data (G_OBJECT (gm), "gw", gw);
  g_object_set_data (G_OBJECT (gm), "lw", lw);
  g_object_set_data (G_OBJECT (gm), "pw", pw);
  g_object_set_data (G_OBJECT (gm), "chat", chat);
  g_object_set_data (G_OBJECT (gm), "rtp", rtp);


  /* Startup Process */
  gnomemeeting_stock_icons_init ();


  /* Init the tray icon */
  gw->docklet = GTK_WIDGET (gnomemeeting_init_tray ());
  if (gconf_client_get_bool 
      (client, "/apps/gnomemeeting/general/do_not_disturb", 0)) 
    gnomemeeting_tray_set_content (G_OBJECT (gw->docklet), 2);


  /* Init the splash screen */
  gw->splash_win = e_splash_new ();
  g_signal_connect (G_OBJECT (gw->splash_win), "delete_event",
		    G_CALLBACK (gtk_widget_hide_on_delete), 0);

  show_splash = gconf_client_get_bool (client, VIEW_KEY "show_splash", 0);  

  if (show_splash) 
  {
    /* We show the splash screen */
    gtk_widget_show (gw->splash_win);

    while (gtk_events_pending ())
      gtk_main_iteration ();
  }
  

  /* Search for devices */
  gnomemeeting_sound_daemons_suspend ();
  gw->audio_player_devices = 
    PSoundChannel::GetDeviceNames (PSoundChannel::Player);

  gw->audio_recorder_devices = 
    PSoundChannel::GetDeviceNames (PSoundChannel::Recorder);

  gw->video_devices = PVideoInputDevice::GetInputDeviceNames ();


  /* Build the interface */
  gnomemeeting_init_history_window ();
  gnomemeeting_init_calls_history_window ();
  gnomemeeting_init_main_window ();
  gnomemeeting_init_ldap_window ();
  gnomemeeting_init_pref_window ();  
  gnomemeeting_init_menu ();
  gnomemeeting_init_toolbar ();


  /* Launch the GnomeMeeting H.323 part */
  static GnomeMeeting instance;
  endpoint = MyApp->Endpoint ();
  gnomemeeting_sound_daemons_resume ();
 
  /* Launch the GnomeMeeting H.323 part */
  if (clo->debug_level != 0)
    PTrace::Initialise (clo->debug_level);

  /* Start the video preview */
  if (gconf_client_get_bool (client, DEVICE_KEY "video_preview", NULL)) {
    GMVideoGrabber *vg = NULL;
    vg = MyApp->Endpoint ()->GetVideoGrabber ();
    
    if (vg)
      vg->Open (TRUE);
  }


  /* Set the local User name */
  firstname = gconf_client_get_string (client, PERSONAL_KEY "firstname", 0);
  lastname  = gconf_client_get_string (client, PERSONAL_KEY "lastname", 0);
  
  if ((firstname) && (lastname) && (strcmp (firstname, ""))) 
  { 
    gchar *local_name = NULL;
    local_name = g_strconcat (firstname, " ", lastname, NULL);

    /* It is the first alias for the gatekeeper */
    if (local_name != NULL) {

      endpoint->SetLocalUserName (gnomemeeting_from_utf8_to_ucs2 (local_name));
    }

    g_free (firstname);
    g_free (lastname);
    g_free (local_name);
  }

  alias = gconf_client_get_string (client, GATEKEEPER_KEY "gk_alias", 0);
  
  if (alias != NULL) {
    
    PString Alias = PString (alias);
    
    if (!Alias.IsEmpty ())
      endpoint->AddAliasName (Alias);
  }
  g_free (alias);


  /* Register to gatekeeper */
  if (gconf_client_get_int (client, GATEKEEPER_KEY "registering_method", 0))
    endpoint->GatekeeperRegister ();


  /* The LDAP part, if needed */
  if (gconf_client_get_bool (GCONF_CLIENT (client), LDAP_KEY "register", NULL)) 
  {
      GMILSClient *gm_ils_client = (GMILSClient *) endpoint->GetILSClient ();
      gm_ils_client->Register ();
  }
  
  
  if (!endpoint->StartListener ()) 
  {
      gnomemeeting_error_dialog (GTK_WINDOW (gm), _("Could not start the listener thread. You will not be able to receive incoming calls."));
  }

  
  /* Start the Gconf notifiers */
  gnomemeeting_init_gconf (client);


  /* Register to the Gatekeeper */
  int method = gconf_client_get_int (GCONF_CLIENT (client), 
                                     GATEKEEPER_KEY "registering_method", 0);

  /* We do that through the notifier */
  if (method)
    gconf_client_set_int (GCONF_CLIENT (client),
			  GATEKEEPER_KEY "registering_method",
			  method, 0);


  /* Init the druid */
  if (gconf_client_get_int (client, GENERAL_KEY "version", NULL) 
      < 100 * MAJOR_VERSION + MINOR_VERSION)

    gnomemeeting_init_druid ((gpointer) "first");

  else {

  /* Show the main window */
    if (!gconf_client_get_bool (GCONF_CLIENT (client), 
				VIEW_KEY "show_docklet", 0) ||
	!gconf_client_get_bool (GCONF_CLIENT (client),
				VIEW_KEY "start_docked", 0))

    gtk_widget_show (GTK_WIDGET (gm));
  }


  /* Set icon */
  GdkPixbuf *pixbuf_icon = 
    gdk_pixbuf_new_from_file (GNOMEMEETING_IMAGES "/gnomemeeting-logo-icon.png", NULL); 

  gtk_window_set_icon (GTK_WINDOW (gm), pixbuf_icon);
  g_object_unref (G_OBJECT (pixbuf_icon));
  gtk_window_set_resizable (GTK_WINDOW (gm), false);

  /* Hide the splash */
  if (gw->splash_win)
    gtk_widget_hide (gw->splash_win);


  /* if the user tries to close the window : delete_event */
  g_signal_connect (G_OBJECT (gm), "delete_event",
		    G_CALLBACK (gm_quit_callback), (gpointer) gw);


  /* Add the popup menu and change all menus sensitivity */
  gnomemeeting_popup_menu_init (gw->main_video_image);
  gnomemeeting_video_submenu_set_sensitive (FALSE, LOCAL_VIDEO);
  gnomemeeting_video_submenu_set_sensitive (FALSE, REMOTE_VIDEO);
  gnomemeeting_zoom_submenu_set_sensitive (FALSE);
#ifdef HAS_SDL
  gnomemeeting_fullscreen_option_set_sensitive (FALSE);
#endif


  /* The gtk_widget_show (gm) will show the toolbar, hide it if needed */
  if (!gconf_client_get_bool (client, VIEW_KEY "left_toolbar", 0)) 
    gtk_widget_hide (GTK_WIDGET (gnome_app_get_dock_item_by_name(GNOME_APP (gm), "left_toolbar")));


  /* Call the given host if needed */
  if (clo->url) {

     gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (gw->combo)->entry), 
			 clo->url);
     connect_cb (NULL, NULL);
  }
}


/**
 * DESCRIPTION  :  /
 * BEHAVIOR     :  Builds the main window.
 * PRE          :  Valid options.
 **/
void gnomemeeting_init_main_window ()
{ 
  GConfClient *client = gconf_client_get_default ();
  GtkWidget *table;	
  GtkWidget *frame;
  GtkWidget *vbox;
  int main_notebook_section = 0;
  int x = GM_QCIF_WIDTH;
  int y = GM_QCIF_HEIGHT;
  
  GmWindow *gw = gnomemeeting_get_main_window (gm);

  /* Create a table in the main window to attach things like buttons */
  table = gtk_table_new (3, 4, FALSE);
  
  gnome_app_set_contents (GNOME_APP (gm), GTK_WIDGET (table));


  /* The Notebook */
  gw->main_notebook = gtk_notebook_new ();
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (gw->main_notebook), GTK_POS_BOTTOM);
  gtk_notebook_popup_enable (GTK_NOTEBOOK (gw->main_notebook));
  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (gw->main_notebook), TRUE);
  gtk_notebook_set_scrollable (GTK_NOTEBOOK (gw->main_notebook), TRUE);


  gnomemeeting_init_main_window_stats ();
  gnomemeeting_init_main_window_audio_settings ();
  gnomemeeting_init_main_window_video_settings ();

  gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (gw->main_notebook),
		    0, 2, 2, 3,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    0, 0); 

  main_notebook_section = 
    gconf_client_get_int (client, VIEW_KEY "control_panel_section", 0);

  if (main_notebook_section != GM_MAIN_NOTEBOOK_HIDDEN) {

    gtk_widget_show_all (GTK_WIDGET (gw->main_notebook));
    gtk_notebook_set_current_page (GTK_NOTEBOOK ((gw->main_notebook)), 
				   main_notebook_section);
  }


  /* The drawing area that will display the webcam images */
  frame = gtk_frame_new (NULL);
  gw->video_frame = gtk_frame_new (NULL);

  vbox = gtk_vbox_new (FALSE, 0);

  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_box_pack_start (GTK_BOX (vbox), gw->video_frame, TRUE, TRUE, 0);
  gtk_frame_set_shadow_type (GTK_FRAME (gw->video_frame), GTK_SHADOW_IN);

  gw->main_video_image = gtk_image_new ();
  gtk_container_set_border_width (GTK_CONTAINER (gw->video_frame), 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 0);
  gtk_container_add (GTK_CONTAINER (gw->video_frame), gw->main_video_image);

  gtk_widget_set_size_request (GTK_WIDGET (gw->video_frame), 
			       GM_QCIF_WIDTH, GM_QCIF_HEIGHT); 

  gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (frame), 
		    0, 2, 0, 1,
		    GTK_EXPAND,
		    (GtkAttachOptions) NULL,
		    10, 10);

  gtk_widget_show_all (GTK_WIDGET (frame));

  
  /* The 2 video window popups */
  x = gconf_client_get_int (client, 
			    "/apps/gnomemeeting/video_display/local_video_width", 
			    NULL);
  y = gconf_client_get_int (client, 
			    "/apps/gnomemeeting/video_display/local_video_height", 
			    NULL);
  gw->local_video_window =
    gnomemeeting_video_window_new (_("Local Video"), gw->local_video_image,
				   x, y);

  x = gconf_client_get_int (client, 
			    "/apps/gnomemeeting/video_display/remote_video_width", 
			    NULL);
  y = gconf_client_get_int (client, 
			    "/apps/gnomemeeting/video_display/remote_video_height", 
			    NULL);
  gw->remote_video_window =
    gnomemeeting_video_window_new (_("Remote Video"), gw->remote_video_image,
				   x, y);
  gnomemeeting_init_main_window_logo (gw->main_video_image);


  /* The remote name */
  gw->remote_name = gtk_entry_new ();
  gtk_editable_set_editable (GTK_EDITABLE (gw->remote_name), FALSE);
  gtk_widget_set_size_request (GTK_WIDGET (gw->remote_name), 
			       GM_QCIF_WIDTH, -1);

  gtk_box_pack_start (GTK_BOX (vbox), gw->remote_name, TRUE, TRUE, 0);

  gtk_widget_show (GTK_WIDGET (gw->remote_name));


  /* The Chat Window */
  gnomemeeting_text_chat_init ();
  gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (gw->chat_window), 
 		    2, 4, 0, 3,
 		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
 		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
 		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);
  if (gconf_client_get_bool 
      (client, "/apps/gnomemeeting/view/show_chat_window", 0))
          gtk_widget_show_all (GTK_WIDGET (gw->chat_window));


  /* The statusbar */
  gw->statusbar = gnome_appbar_new (TRUE, TRUE, 
				    GNOME_PREFERENCES_NEVER);	
  gtk_widget_hide (GTK_WIDGET (gnome_appbar_get_progress (GNOME_APPBAR (gw->statusbar))));
  gtk_widget_set_size_request (GTK_WIDGET (gnome_appbar_get_progress (GNOME_APPBAR (gw->statusbar))), 35, -1);
  gnome_app_set_statusbar (GNOME_APP (gm), gw->statusbar);

  if (gconf_client_get_bool (client, 
			     VIEW_KEY "show_status_bar", 0))
    gtk_widget_show (GTK_WIDGET (gw->statusbar));
  else
    gtk_widget_hide (GTK_WIDGET (gw->statusbar));

  
  gtk_widget_set_size_request (GTK_WIDGET (gw->main_notebook), 230, -1);
  gtk_widget_set_size_request (GTK_WIDGET (gm), -1, -1);

  g_signal_connect_after (G_OBJECT (gw->main_notebook), "switch-page",
			  G_CALLBACK (main_notebook_page_changed), NULL);
}


/**
 * DESCRIPTION  :  /
 * BEHAVIOR     :  Builds the statistics part of the main window.
 * PRE          :  /
 **/
void gnomemeeting_init_main_window_stats ()
{
  GtkWidget *frame = NULL;
  GtkWidget *frame2 = NULL;
  GtkWidget *label = NULL;
  GtkWidget *vbox = NULL;

  GmWindow *gw = gnomemeeting_get_main_window (gm);

  frame = gtk_frame_new (_("Statistics"));
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);

  
  /* The first frame with statistics display */
  frame2 = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);

  vbox = gtk_vbox_new (FALSE, 4);
  gw->stats_drawing_area = gtk_drawing_area_new ();
  gtk_box_pack_start (GTK_BOX (vbox), frame2, FALSE, TRUE, 0);
  gtk_container_add (GTK_CONTAINER (frame2), gw->stats_drawing_area);

  gtk_widget_set_size_request (GTK_WIDGET (frame2), 180, 67);

  g_signal_connect (G_OBJECT (gw->stats_drawing_area), "expose_event",
		    G_CALLBACK (stats_drawing_area_exposed), NULL);

  gtk_container_add (GTK_CONTAINER (frame), vbox);    
  gtk_container_set_border_width (GTK_CONTAINER (frame),
				  GNOME_PAD_SMALL);

  gtk_widget_queue_draw_area (gw->stats_drawing_area, 0, 0, GTK_WIDGET (gw->stats_drawing_area)->allocation.width, GTK_WIDGET (gw->stats_drawing_area)->allocation.height);


  /* The second one with some labels */
  gw->stats_label = gtk_label_new (_("Sent/Received:\nLost/Late Packets:\nRound trip delay:"));
  gtk_misc_set_alignment (GTK_MISC (gw->stats_label), 0, 0);
  gtk_box_pack_start (GTK_BOX (vbox), gw->stats_label, FALSE, TRUE, 0);
  
  label = gtk_label_new (_("Statistics"));

  gtk_notebook_append_page (GTK_NOTEBOOK (gw->main_notebook), frame, label);
}


/**
 * DESCRIPTION  :  /
 * BEHAVIOR     :  Builds the video settings part of the main window. This
 *                 part is made unsensitive while the grabber is not enabled.
 * PRE          :  /
 **/
void gnomemeeting_init_main_window_video_settings ()
{
  GtkWidget *label;
  
  GtkWidget *table;

  GtkWidget *pixmap;
  GdkPixbuf *pixbuf;
  GtkWidget *hscale_brightness, *hscale_colour, 
    *hscale_contrast, *hscale_whiteness;

  int brightness = 0, colour = 0, contrast = 0, whiteness = 0;
  
  GmWindow *gw = gnomemeeting_get_main_window (gm);

  /* Webcam Control Frame */		
  gw->video_settings_frame = gtk_frame_new (_("Video Settings"));
  gtk_frame_set_shadow_type (GTK_FRAME (gw->video_settings_frame), 
			     GTK_SHADOW_ETCHED_OUT);

  /* Put a table in the first frame */
  table = gtk_table_new (4, 4, FALSE);
  gtk_container_add (GTK_CONTAINER (gw->video_settings_frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (gw->video_settings_frame), 
				  GNOME_PAD_SMALL);


  /* Brightness */
  pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **) brightness_xpm);
  pixmap =  gtk_image_new_from_pixbuf (pixbuf);
  g_object_unref (G_OBJECT (pixbuf));
  gtk_table_attach (GTK_TABLE (table), pixmap, 0, 1, 0, 1,
		    (GtkAttachOptions) (NULL),
		    (GtkAttachOptions) (NULL),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);

  gw->adj_brightness = gtk_adjustment_new (brightness, 0.0, 
					   255.0, 1.0, 5.0, 1.0);
  hscale_brightness = gtk_hscale_new (GTK_ADJUSTMENT (gw->adj_brightness));
  gtk_scale_set_draw_value (GTK_SCALE (hscale_brightness), TRUE);
  gtk_scale_set_value_pos (GTK_SCALE (hscale_brightness), GTK_POS_RIGHT);
  gtk_table_attach (GTK_TABLE (table), hscale_brightness, 1, 4, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, 0);

  gtk_tooltips_set_tip (gw->tips, hscale_brightness,
			_("Adjust brightness"), NULL);

  g_signal_connect (G_OBJECT (gw->adj_brightness), "value-changed",
		    G_CALLBACK (brightness_changed), (gpointer) gw);


  /* Whiteness */
  pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **) whiteness_xpm);
  pixmap =  gtk_image_new_from_pixbuf (pixbuf);
  g_object_unref (G_OBJECT (pixbuf));
  gtk_table_attach (GTK_TABLE (table), pixmap, 0, 1, 1, 2,
 (GtkAttachOptions) (NULL),
		    (GtkAttachOptions) (NULL),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);

  gw->adj_whiteness = gtk_adjustment_new (whiteness, 0.0, 
					  255.0, 1.0, 5.0, 1.0);
  hscale_whiteness = gtk_hscale_new (GTK_ADJUSTMENT (gw->adj_whiteness));
  gtk_scale_set_draw_value (GTK_SCALE (hscale_whiteness), TRUE);
  gtk_scale_set_value_pos (GTK_SCALE (hscale_whiteness), GTK_POS_RIGHT);
  gtk_table_attach (GTK_TABLE (table), hscale_whiteness, 1, 4, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, 0);

  gtk_tooltips_set_tip (gw->tips, hscale_whiteness,
			_("Adjust whiteness"), NULL);

  g_signal_connect (G_OBJECT (gw->adj_whiteness), "value-changed",
		    G_CALLBACK (whiteness_changed), (gpointer) gw);


  /* Colour */
  pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **) color_xpm);
  pixmap =  gtk_image_new_from_pixbuf (pixbuf);
  g_object_unref (G_OBJECT (pixbuf));
  gtk_table_attach (GTK_TABLE (table), pixmap, 0, 1, 2, 3,
		    (GtkAttachOptions) (NULL),
		    (GtkAttachOptions) (NULL),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);

  gw->adj_colour = gtk_adjustment_new (colour, 0.0, 
				       255.0, 1.0, 5.0, 1.0);
  hscale_colour = gtk_hscale_new (GTK_ADJUSTMENT (gw->adj_colour));
  gtk_scale_set_draw_value (GTK_SCALE (hscale_colour), TRUE);
  gtk_scale_set_value_pos (GTK_SCALE (hscale_colour), GTK_POS_RIGHT);
  gtk_table_attach (GTK_TABLE (table), hscale_colour, 1, 4, 2, 3,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, 0);

  gtk_tooltips_set_tip (gw->tips, hscale_colour,
			_("Adjust color"), NULL);

  g_signal_connect (G_OBJECT (gw->adj_colour), "value-changed",
		    G_CALLBACK (colour_changed), (gpointer) gw);


  /* Contrast */
  pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **) contrast_xpm);
  pixmap =  gtk_image_new_from_pixbuf (pixbuf);
  g_object_unref (G_OBJECT (pixbuf));
  gtk_table_attach (GTK_TABLE (table), pixmap, 0, 1, 3, 4,
		    (GtkAttachOptions) (NULL),
		    (GtkAttachOptions) (NULL),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);

  gw->adj_contrast = gtk_adjustment_new (contrast, 0.0, 
					 255.0, 1.0, 5.0, 1.0);
  hscale_contrast = gtk_hscale_new (GTK_ADJUSTMENT (gw->adj_contrast));
  gtk_scale_set_draw_value (GTK_SCALE (hscale_contrast), TRUE);
  gtk_scale_set_value_pos (GTK_SCALE (hscale_contrast), GTK_POS_RIGHT);
  gtk_table_attach (GTK_TABLE (table), hscale_contrast, 1, 4, 3, 4,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, 0);

  gtk_tooltips_set_tip (gw->tips, hscale_contrast,
			_("Adjust contrast"), NULL);

  g_signal_connect (G_OBJECT (gw->adj_contrast), "value-changed",
		    G_CALLBACK (contrast_changed), (gpointer) gw);
  
  gtk_widget_set_sensitive (GTK_WIDGET (gw->video_settings_frame), FALSE);

  label = gtk_label_new (_("Video"));  

  gtk_notebook_append_page (GTK_NOTEBOOK(gw->main_notebook), 
			    gw->video_settings_frame, 
			    label);

}


/**
 * DESCRIPTION  :  /
 * BEHAVIOR     :  Builds the audio setting part of the main window.
 * PRE          :  /
 **/
void gnomemeeting_init_main_window_audio_settings ()
{
  GtkWidget *label;
  GtkWidget *hscale_play, *hscale_rec;
  GtkWidget *audio_table;

  int vol = 0;

  GtkWidget *frame;
  GConfClient *client = gconf_client_get_default ();

  GmWindow *gw = gnomemeeting_get_main_window (gm);

  frame = gtk_frame_new (_("Audio Settings"));
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);

  audio_table = gtk_table_new (4, 4, TRUE);
  gtk_container_add (GTK_CONTAINER (frame), audio_table);
  gtk_container_set_border_width (GTK_CONTAINER (frame), GNOME_PAD_SMALL);

  gtk_table_attach (GTK_TABLE (audio_table), 
                    gtk_image_new_from_stock (GM_STOCK_VOLUME, GTK_ICON_SIZE_SMALL_TOOLBAR), 
                    0, 1, 0, 1,
		    (GtkAttachOptions) NULL,
		    (GtkAttachOptions) NULL,
		    GNOME_PAD_SMALL, 0);

  gchar *player_mixer = gconf_client_get_string (client, DEVICE_KEY "audio_player_mixer", NULL);
  gnomemeeting_volume_get (player_mixer, 0, &vol);
  g_free (player_mixer);
  gw->adj_play = gtk_adjustment_new (vol / 257, 0.0, 100.0, 1.0, 5.0, 1.0);
  hscale_play = gtk_hscale_new (GTK_ADJUSTMENT (gw->adj_play));
  gtk_scale_set_value_pos (GTK_SCALE (hscale_play),GTK_POS_RIGHT); 
  gtk_scale_set_draw_value (GTK_SCALE (hscale_play), TRUE);

  gtk_table_attach (GTK_TABLE (audio_table), hscale_play, 1, 4, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    0, 0);


  gtk_table_attach (GTK_TABLE (audio_table), 
                    gtk_image_new_from_stock (GM_STOCK_MICROPHONE, GTK_ICON_SIZE_SMALL_TOOLBAR), 
                    0, 1, 1, 2,
		    (GtkAttachOptions) NULL,
		    (GtkAttachOptions) NULL,
		    GNOME_PAD_SMALL, 0);

  gchar *recorder_mixer = gconf_client_get_string (client, DEVICE_KEY "audio_recorder_mixer", NULL);
  gnomemeeting_volume_get (recorder_mixer, 1, &vol);
  g_free (recorder_mixer);
  gw->adj_rec = gtk_adjustment_new (vol / 257, 0.0, 100.0, 1.0, 5.0, 1.0);
  hscale_rec = gtk_hscale_new (GTK_ADJUSTMENT (gw->adj_rec));
  gtk_scale_set_value_pos (GTK_SCALE (hscale_rec),GTK_POS_RIGHT); 
  gtk_scale_set_draw_value (GTK_SCALE (hscale_rec), TRUE);

  gtk_table_attach (GTK_TABLE (audio_table), hscale_rec, 1, 4, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    0, 0);

  g_signal_connect (G_OBJECT (gw->adj_play), "value-changed",
		    G_CALLBACK (audio_volume_changed), (gpointer) gw);

  g_signal_connect (G_OBJECT (gw->adj_rec), "value-changed",
		    G_CALLBACK (audio_volume_changed), (gpointer) gw);

  
  label = gtk_label_new (_("Audio"));

  gtk_notebook_append_page (GTK_NOTEBOOK (gw->main_notebook), frame, label);
}


/* The main () */

int main (int argc, char ** argv, char ** envp)
{
  int x, y;
  PProcess::PreInitialise (argc, argv, envp);

  /* The different structures needed by most of the classes and functions */
  GmWindow *gw = NULL;
  GmLdapWindow *lw = NULL;
  GmPrefWindow *pw = NULL;
  GmTextChat *chat = NULL;
  GmRtpData *rtp = NULL;
  GmCommandLineOptions *clo = NULL;


  /* Init the GmWindow */
  gw = new (GmWindow);
  gw->pref_window = NULL;
  gw->ldap_window = NULL;
  gw->incoming_call_popup = NULL;
  gw->progress_timeout = 0;
  gw->cleaner_thread_count = 0;
  gw->zoom = 1;


  /* Init the GmPrefWindow structure */
  pw = new (GmPrefWindow); 


  /* Init the GmLdapWindow structure */
  lw = new (GmLdapWindow);


  /* Init the RTP stats structure */
  rtp = new (GmRtpData);
  rtp->tr_audio_bytes = 0;
  rtp->re_audio_bytes = 0;
  rtp->re_video_bytes = 0;
  rtp->tr_video_bytes = 0;
  rtp->tr_audio_pos = 0;
  rtp->tr_video_pos = 0;
  rtp->re_audio_pos = 0;
  rtp->re_video_pos = 0;


  /* Init the TextChat structure */
  chat = new (GmTextChat);


  /* Init the CommandLineOptions structure */
  clo = new (GmCommandLineOptions);
  clo->debug_level = 0;
  clo->url = NULL;
  clo->daemon = 0;


  /* Threads + Locale Init + Gconf */
  g_thread_init (NULL);
  gdk_threads_init ();

  gdk_threads_enter ();
  gconf_init (argc, argv, 0);

  textdomain (GETTEXT_PACKAGE);
  bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");


  /* GnomeMeeting main initialisation */
  gnomemeeting_init (gw, pw, lw, rtp, chat, clo, argc, argv, envp);

  /* Set a default gconf error handler */
  gconf_client_set_error_handling (gconf_client_get_default (),
				   GCONF_CLIENT_HANDLE_UNRETURNED);
  gconf_client_set_global_default_error_handler (gconf_error_callback);


  /* Quick hack to make the GUI refresh even on high load from the other
     threads */
  gtk_timeout_add (500, (GtkFunction) AppbarUpdate, rtp);

//   gtk_timeout_add (10000, (GtkFunction) StressTest, 
// 		   NULL);
  

  /* The GTK loop */
  gtk_main ();


  /* Save the 2 popup dimensions */
  gtk_window_get_size (GTK_WINDOW (gw->local_video_window), &x, &y);
  gconf_client_set_int (gconf_client_get_default (), 
			"/apps/gnomemeeting/video_display/local_video_width", 
			x, NULL);
  gconf_client_set_int (gconf_client_get_default (), 
			"/apps/gnomemeeting/video_display/local_video_height", 
			y, NULL);
  
  gtk_window_get_size (GTK_WINDOW (gw->remote_video_window), &x, &y);
  gconf_client_set_int (gconf_client_get_default (), 
			"/apps/gnomemeeting/video_display/remote_video_width",
			x, NULL);
  gconf_client_set_int (gconf_client_get_default (), 
			"/apps/gnomemeeting/video_display/remote_video_height",
			y, NULL);


  /* Hide the gm widget and deletes them */
  gtk_widget_hide (GTK_WIDGET (gm));
  gtk_widget_destroy (GTK_WIDGET (gw->ldap_window));
  gtk_widget_destroy (GTK_WIDGET (gw->pref_window));
  gtk_widget_destroy (GTK_WIDGET (gw->calls_history_window));
  gtk_widget_destroy (GTK_WIDGET (gm));

  gdk_threads_leave ();

  delete (gw);
  delete (lw);
  delete (pw);
  delete (rtp);
  delete (chat); 
  delete (clo);

  exit (0);
}
