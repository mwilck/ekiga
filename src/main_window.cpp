
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
#include "docklet.h"
#include "ils.h"
#include "common.h"
#include "menu.h"
#include "toolbar.h"
#include "callbacks.h"
#include "audio.h"
#include "videograbber.h"
#include "endpoint.h"
#include "pref_window.h"
#include "config.h"
#include "misc.h"
#include "e-splash.h"
#include "druid.h"
#include "chat_window.h"


#include <libgnomeui/gnome-window-icon.h>
#include <bonobo-activation/bonobo-activation-activate.h>
#include <bonobo-activation/bonobo-activation-register.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-listener.h>
#include <gdk/gdkx.h>
#include <gconf/gconf-client.h>
#include <esd.h>


#include "../pixmaps/inlines.h"
#include "../pixmaps/brightness.xpm"
#include "../pixmaps/whiteness.xpm"
#include "../pixmaps/contrast.xpm"
#include "../pixmaps/color.xpm"

#define ACT_IID "OAFIID:GNOME_gnomemeeting_Factory"


/* Declarations */

extern GtkWidget *gm;
extern GnomeMeeting *MyApp;	

static gboolean gnomemeeting_invoke_factory (int, char **);
static void gnomemeeting_new_event (BonoboListener *, const char *, 
				    const CORBA_any *, CORBA_Environment *,
				    gpointer);
static Bonobo_RegistrationResult gnomemeeting_register_as_factory (void);

static void main_notebook_page_changed (GtkNotebook *, GtkNotebookPage *,
					gint, gpointer);
static void audio_volume_changed (GtkAdjustment *, gpointer);
static void brightness_changed (GtkAdjustment *, gpointer);
static void whiteness_changed (GtkAdjustment *, gpointer);
static void colour_changed (GtkAdjustment *, gpointer);
static void contrast_changed (GtkAdjustment *, gpointer);

static void gnomemeeting_init_main_window ();
static gint gm_quit_callback (GtkWidget *, GdkEvent *, gpointer);
static void gnomemeeting_init_main_window_video_settings ();
static void gnomemeeting_init_main_window_audio_settings ();
static void gnomemeeting_init_main_window_log  ();


/* For stress testing */
/* int i = 0; */

/* GTK Callbacks */
/* gint StressTest (gpointer data)
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
*/


gint AppbarUpdate (gpointer data)
{
  long minutes, seconds;
  float tr_audio_speed = 0, tr_video_speed = 0;
  float re_audio_speed = 0, re_video_speed = 0;
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

      if (t.GetSeconds () > 1) {

	audio_session = 
	  connection->GetSession(RTP_Session::DefaultAudioSessionID);
	  
	video_session = 
	  connection->GetSession(RTP_Session::DefaultVideoSessionID);
	  
	if (audio_session != NULL) {

	  tr_audio_speed = 
	    (float) (audio_session->GetOctetsSent()-rtp->tr_audio_bytes)/1024.00;
	  rtp->tr_audio_bytes = audio_session->GetOctetsSent();

	  re_audio_speed = 
	    (float) (audio_session->GetOctetsReceived()-rtp->re_audio_bytes)/1024.00;
	  rtp->re_audio_bytes = audio_session->GetOctetsReceived();
	}

	if (video_session != NULL) {

	  tr_video_speed = 
	    (float) (video_session->GetOctetsSent()-rtp->tr_video_bytes)/1024.00;
	  rtp->tr_video_bytes = video_session->GetOctetsSent();

	  re_video_speed = 
	    (float) (video_session->GetOctetsReceived()-rtp->re_video_bytes)/1024.00;
	  rtp->re_video_bytes = video_session->GetOctetsReceived();
	}

	minutes = t.GetMinutes () % 60;
	seconds = t.GetSeconds () % 60;
	
	gchar *msg = g_strdup_printf 
	  (_("%.2ld:%.2ld:%.2ld  A:%.2f/%.2f   V:%.2f/%.2f"), 
	   (long) t.GetHours (), (long) minutes, (long) seconds, 
	   tr_audio_speed, re_audio_speed,
	   tr_video_speed, re_video_speed);
	

	if (t.GetSeconds () > 3) {

	  gnome_appbar_clear_stack (GNOME_APPBAR (gw->statusbar));
	  gnome_appbar_push (GNOME_APPBAR (gw->statusbar), msg);
	}

	g_free (msg);
      }
    }

    gdk_threads_leave ();
  }

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

  args = ( CORBA_sequence_CORBA_string *) any->_value;
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
    if (MyApp->Endpoint ()->GetCallingState () == 0)
      gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (gw->combo)->entry), 
			  argv [i + 1]);
      
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
    for (i = 0; i < args._length; i++)
      args._buffer [i] = argv [i];
      
    Bonobo_Listener_event (listener, "new_gnomemeeting", &any, &ev);
    CORBA_Object_release (listener, &ev);

    if (!BONOBO_EX (&ev))
      return TRUE;

    CORBA_exception_free (&ev);
  }
  else
    g_printerr (_("Failed to retrieve gnomemeeting server from activation server\n"));
  
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

  GnomeUIInfo *notebook_view_uiinfo =
    (GnomeUIInfo *) g_object_get_data (G_OBJECT (gm),
				       "notebook_view_uiinfo");

  GmWindow *gw = gnomemeeting_get_main_window (gm);

  gconf_client_set_int (client, "/apps/gnomemeeting/view/control_panel_section",
			gtk_notebook_get_current_page (GTK_NOTEBOOK (gw->main_notebook)), 0);
}


/**
 * DESCRIPTION  :  This callback is called when the user changes the
 *                 audio settings sliders in the main notebook.
 * BEHAVIOR     :  Update the volume.
² * PRE          :  gpointer is a valid pointer to GmPrefWindow
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
  audio_player_mixer = gconf_client_get_string (client, "/apps/gnomemeeting/devices/audio_player_mixer", NULL);
  audio_recorder_mixer = gconf_client_get_string (client, "/apps/gnomemeeting/devices/audio_recorder_mixer", NULL);
  
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
                   int argc, char ** argv, char ** envp)
{
  GMH323EndPoint *endpoint = NULL;
  gchar *firstname = NULL;
  gchar *lastname = NULL;
  gchar *alias = NULL;
  bool show_splash;
  GConfClient *client;
  int debug = 0;
  int esd_client = 0;

  /* Cope with command line options */
  static struct poptOption arguments[] =
    {
      {"debug", 'd', POPT_ARG_NONE, &debug, 
       0, N_("Prints debug messages in the console"), NULL},
      {"callto", 'c', POPT_ARG_STRING, NULL,
       1, N_("Makes GnomeMeeting call the given callto:// URL"), NULL},
      {NULL, '\0', 0, NULL, 0, NULL, NULL}
    };

  for (int i = 0; i < argc; i++) {
    if (!strcmp (argv [i], "-d") || !strcmp (argv [i], "--debug"))
      debug = 1;
  } 


  /* Gnome Initialisation */
  GnomeProgram *p = gnome_program_init ("GnomeMeeting", VERSION,
					LIBGNOMEUI_MODULE, argc, argv,
					GNOME_PARAM_POPT_TABLE, arguments,
					GNOME_PARAM_HUMAN_READABLE_NAME,
					_("GnomeMeeting"),
					GNOME_PARAM_APP_DATADIR, DATADIR,
					NULL);

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
			GCONF_CLIENT_PRELOAD_RECURSIVE,
			0);
  int gconf_test = -1;
  gconf_test = gconf_client_get_int (client, "/apps/gnomemeeting/general/gconf_test_age", NULL);

   if (gconf_test != SCHEMA_AGE) {

     int reply = 0;
     gchar *msg = g_strdup_printf (_("GnomeMeeting got %d for the GConf key \"/apps/gnomemeeting/gconf_test\", but %d was expected.\n\nThat key represents the revision number of GnomeMeeting default settings. If it is not correct, it means that your GConf schemas have not been correctly installed or the that permissions are not correct.\n\nPlease check the FAQ (http://www.gnomemeeting.org/faq.php), the throubleshoot section of the GConf site (http://www.gnome.org/projects/gconf/) or the mailing list archives for more information (http://mail.gnome.org).\n\nUsing 'gnomemeeting-config-tool' could help you fix these problem."), gconf_test, SCHEMA_AGE);

     GtkWidget *dialog = 
       gtk_message_dialog_new (GTK_WINDOW (gm),
			       GTK_DIALOG_DESTROY_WITH_PARENT,
			       GTK_MESSAGE_ERROR,
			       GTK_BUTTONS_CLOSE,
			       msg);
     g_free (msg);
     gtk_dialog_run (GTK_DIALOG (dialog));
     gtk_widget_destroy (dialog);

     delete (gw);
     delete (lw);
     delete (pw);
     delete (rtp);
     delete (chat);
     exit (-1);
   }

  /* We store all the pointers to the structure as data of gm */
  g_object_set_data (G_OBJECT (gm), "gw", gw);
  g_object_set_data (G_OBJECT (gm), "lw", lw);
  g_object_set_data (G_OBJECT (gm), "pw", pw);
  g_object_set_data (G_OBJECT (gm), "chat", chat);

  /* Startup Process */
  gw->docklet = gnomemeeting_init_docklet ();

  /* Init the splash screen */
  gw->splash_win = e_splash_new ();
  g_signal_connect (G_OBJECT (gw->splash_win), "delete_event",
		    G_CALLBACK (gtk_widget_hide_on_delete), 0);

  show_splash = gconf_client_get_bool (client, "/apps/gnomemeeting/"
				      "view/show_splash", 0);  

  if (show_splash) {

    /* We show the splash screen */
    gtk_widget_show (gw->splash_win);

    while (gtk_events_pending ())
      gtk_main_iteration ();
  }
  

  /* Search for devices */
  esd_client = esd_open_sound (NULL);
  esd_standby (esd_client);
  gw->audio_player_devices = 
    PSoundChannel::GetDeviceNames (PSoundChannel::Player);

  gw->audio_recorder_devices = 
    PSoundChannel::GetDeviceNames (PSoundChannel::Recorder);

  gw->video_devices = PVideoInputDevice::GetInputDeviceNames ();
  esd_resume (esd_client);
  esd_close (esd_client);


  /* Build the interface */
  gnomemeeting_init_main_window ();
  gnomemeeting_init_ldap_window ();
  gnomemeeting_init_pref_window ();  
  gnomemeeting_init_menu ();
  gnomemeeting_init_toolbar ();	

 
  /* Launch the GnomeMeeting H.323 part */
  static GnomeMeeting instance;
  endpoint = MyApp->Endpoint ();

  if (debug)
    PTrace::Initialise (3);

 
  /* Start the video preview */
  if (gconf_client_get_bool (client, "/apps/gnomemeeting/devices/video_preview", NULL)) {
    GMVideoGrabber *vg = NULL;
    vg = MyApp->Endpoint ()->GetVideoGrabber ();
    
    if (vg)
      vg->Open (TRUE);
  }


  /* Set the local User name */
  firstname =
    gconf_client_get_string (client, 
			     "/apps/gnomemeeting/personal_data/firstname", 
			     0);
  lastname =
    gconf_client_get_string (client, 
			     "/apps/gnomemeeting/personal_data/lastname", 0);
  
  if ((firstname) && (lastname)) {
    
    gchar *local_name = g_strdup ("");
    local_name = g_strconcat (local_name, firstname, " ", lastname, NULL);
    
    /* It is the first alias for the gatekeeper */
    if (local_name != NULL) {

      gchar *iso_8859_1_local_name = NULL;
      iso_8859_1_local_name = g_convert (local_name, strlen (local_name),
					 "ISO-8859-1", "UTF8", 0, 0, 0);
      endpoint->SetLocalUserName (iso_8859_1_local_name);
      g_free (iso_8859_1_local_name);
    }
  }

  alias =
    gconf_client_get_string (client, 
			     "/apps/gnomemeeting/gatekeeper/gk_alias", 
			     0);
  
  if (alias != NULL) {
    
    PString Alias = PString (alias);
    
    if (!Alias.IsEmpty ())
      endpoint->AddAliasName (Alias);
  }

  
  /* The LDAP part, if needed */
  if (gconf_client_get_bool (GCONF_CLIENT (client), "/apps/gnomemeeting/ldap/register", NULL)) {

    GMILSClient *gm_ils_client = (GMILSClient *) endpoint->GetILSClient ();
    gm_ils_client->Register ();
  }
  
  
  if (!endpoint->StartListener ()) {

    GtkWidget *dialog = 
       gtk_message_dialog_new (GTK_WINDOW (gm),
			       GTK_DIALOG_MODAL,
			       GTK_MESSAGE_ERROR,
			       GTK_BUTTONS_OK,
			       _("Could not start the listener thread. You will not be able to receive incoming calls."));

     gtk_dialog_run (GTK_DIALOG (dialog));
     gtk_widget_destroy (dialog);
  }

  
  /* Start the Gconf notifiers */
  gnomemeeting_init_gconf (client);


  /* Register to the Gatekeeper */
  int method = gconf_client_get_int (GCONF_CLIENT (client), "/apps/gnomemeeting/gatekeeper/registering_method", 0);

  /* We do that through the notifier */
  if (method)
    gconf_client_set_int (GCONF_CLIENT (client),
			  "/apps/gnomemeeting/gatekeeper/registering_method",
			  method, 0);


  /* Init the druid */
  if (gconf_client_get_int (client, 
			    "/apps/gnomemeeting/general/version", NULL) 
      < 100 * MAJOR_VERSION + MINOR_VERSION)

    gnomemeeting_init_druid ((gpointer) "first");

  else {

  /* Show the main window */
  if (!gconf_client_get_bool (GCONF_CLIENT (client), 
			     "/apps/gnomemeeting/view/show_docklet", 0) ||
      !gconf_client_get_bool (GCONF_CLIENT (client),
			     "/apps/gnomemeeting/view/start_docked", 0))

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

  gnomemeeting_init_main_window_logo ();


  /* The gtk_widget_show (gm) will show the toolbar, hide it if needed */
  if (!gconf_client_get_bool (client, "/apps/gnomemeeting/view/left_toolbar", 0)) 
    gtk_widget_hide (GTK_WIDGET (gnome_app_get_dock_item_by_name(GNOME_APP (gm), "left_toolbar")));
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
  int main_notebook_section = 0;
  
  GmWindow *gw = gnomemeeting_get_main_window (gm);

  /* Create a table in the main window to attach things like buttons */
  table = gtk_table_new (3, 4, FALSE);
  
  gnome_app_set_contents (GNOME_APP (gm), GTK_WIDGET (table));


  /* The Notebook */
  gw->main_notebook = gtk_notebook_new ();
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (gw->main_notebook), GTK_POS_BOTTOM);
  gtk_notebook_popup_enable (GTK_NOTEBOOK (gw->main_notebook));
  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (gw->main_notebook), TRUE);

  gnomemeeting_init_main_window_log ();
  gnomemeeting_init_main_window_audio_settings ();
  gnomemeeting_init_main_window_video_settings ();

  gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (gw->main_notebook),
		    0, 2, 2, 3,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    0, 0); 

  main_notebook_section = 
    gconf_client_get_int (client, "/apps/gnomemeeting/view/control_panel_section", 0);
  if (main_notebook_section != 3) {

    gtk_widget_show_all (GTK_WIDGET (gw->main_notebook));
    gtk_notebook_set_current_page (GTK_NOTEBOOK ((gw->main_notebook)), 
				   main_notebook_section);
  }


  /* The drawing area that will display the webcam images */
  frame = gtk_frame_new (NULL);
  gw->video_frame = gtk_handle_box_new();

  gtk_container_add (GTK_CONTAINER (frame), gw->video_frame);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);

  gtk_widget_set_size_request (GTK_WIDGET (gw->video_frame), 
			       GM_QCIF_WIDTH + GM_FRAME_SIZE, GM_QCIF_HEIGHT);

  gw->video_image = gtk_image_new ();
  gtk_container_add (GTK_CONTAINER (gw->video_frame), gw->video_image);

  gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (frame), 
		    0, 2, 0, 1,
		    GTK_EXPAND,
		    (GtkAttachOptions) NULL,
		    10, 10);

  gtk_widget_show_all (GTK_WIDGET (frame));


  /* The Chat Window */
  gnomemeeting_init_text_chat_window ();
  gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (gw->chat_window), 
 		    2, 4, 0, 3,
 		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
 		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
 		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);
  if (gconf_client_get_bool 
      (client, "/apps/gnomemeeting/view/show_chat_window", 0))
          gtk_widget_show_all (GTK_WIDGET (gw->chat_window));


  /* The remote name */
  gw->remote_name = gtk_entry_new ();
  gtk_editable_set_editable (GTK_EDITABLE (gw->remote_name), FALSE);

  gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (gw->remote_name), 
		    0, 2, 1, 2,
		    (GtkAttachOptions) NULL,
		    (GtkAttachOptions) NULL,
		    10, 10);

  gtk_widget_show (GTK_WIDGET (gw->remote_name));


  /* The statusbar */
  gw->statusbar = gnome_appbar_new (TRUE, TRUE, 
				    GNOME_PREFERENCES_NEVER);	
  gtk_widget_hide (GTK_WIDGET (gnome_appbar_get_progress (GNOME_APPBAR (gw->statusbar))));
  gtk_widget_set_size_request (GTK_WIDGET (gnome_appbar_get_progress (GNOME_APPBAR (gw->statusbar))), 35, -1);
  gnome_app_set_statusbar (GNOME_APP (gm), gw->statusbar);

  if (gconf_client_get_bool (client, 
			     "/apps/gnomemeeting/view/show_status_bar", 0))
    gtk_widget_show (GTK_WIDGET (gw->statusbar));
  else
    gtk_widget_hide (GTK_WIDGET (gw->statusbar));

  
  gtk_widget_set_size_request (GTK_WIDGET (gw->main_notebook), 230, 170);
  gtk_widget_set_size_request (GTK_WIDGET (gm), -1, -1);

  g_signal_connect_after (G_OBJECT (gw->main_notebook), "switch-page",
			  G_CALLBACK (main_notebook_page_changed), NULL);
}


/**
 * DESCRIPTION  :  /
 * BEHAVIOR     :  Builds the history log part of the main window.
 * PRE          :  /
 **/
void gnomemeeting_init_main_window_log ()
{
  GtkWidget *frame;
  GtkWidget *label;
  GtkWidget *scr;

  GmWindow *gw = gnomemeeting_get_main_window (gm);

  frame = gtk_frame_new (_("History Log"));
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);

  gw->history_view = gtk_text_view_new ();
  gtk_text_view_set_editable (GTK_TEXT_VIEW (gw->history_view), FALSE);

  gw->history = gtk_text_view_get_buffer (GTK_TEXT_VIEW (gw->history_view));
  gtk_text_view_set_editable (GTK_TEXT_VIEW (gw->history_view), FALSE);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (gw->history_view),
			       GTK_WRAP_WORD);

  scr = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (frame), scr);
  gtk_container_set_border_width (GTK_CONTAINER (frame), GNOME_PAD_SMALL);
  gtk_container_set_border_width (GTK_CONTAINER (scr), GNOME_PAD_SMALL);
  
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_ALWAYS);

  gtk_container_add (GTK_CONTAINER (scr), gw->history_view);    

  label = gtk_label_new (_("History"));

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
  
  GtkTooltips *tip;

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

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, hscale_brightness,
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

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, hscale_whiteness,
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

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, hscale_colour,
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

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, hscale_contrast,
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
  GdkPixbuf *mic, *volume;
  GtkWidget *pixmap;

  int vol = 0;

  GtkWidget *frame;
  GConfClient *client = gconf_client_get_default ();

  GmWindow *gw = gnomemeeting_get_main_window (gm);

  volume = gdk_pixbuf_new_from_inline (-1, inline_volume, FALSE, NULL);
  mic = gdk_pixbuf_new_from_inline (-1, inline_mic, FALSE, NULL);

  frame = gtk_frame_new (_("Audio Settings"));
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);

  audio_table = gtk_table_new (4, 4, TRUE);
  gtk_container_add (GTK_CONTAINER (frame), audio_table);
  gtk_container_set_border_width (GTK_CONTAINER (frame), GNOME_PAD_SMALL);

  pixmap = gtk_image_new_from_pixbuf (volume);

  gtk_table_attach (GTK_TABLE (audio_table), pixmap, 0, 1, 0, 1,
		    (GtkAttachOptions) NULL,
		    (GtkAttachOptions) NULL,
		    GNOME_PAD_SMALL, 0);

  gchar *player_mixer = gconf_client_get_string (client, "/apps/gnomemeeting/devices/audio_player_mixer", NULL);
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


  pixmap = gtk_image_new_from_pixbuf (mic);

  gtk_table_attach (GTK_TABLE (audio_table), pixmap, 0, 1, 1, 2,
		    (GtkAttachOptions) NULL,
		    (GtkAttachOptions) NULL,
		    GNOME_PAD_SMALL, 0);

  gchar *recorder_mixer = gconf_client_get_string (client, "/apps/gnomemeeting/devices/audio_recorder_mixer", NULL);
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
  PProcess::PreInitialise (argc, argv, envp);

  /* The different structures needed by most of the classes and functions */
  GmWindow *gw = NULL;
  GmLdapWindow *lw = NULL;
  GmPrefWindow *pw = NULL;
  GmTextChat *chat = NULL;
  GmRtpData *rtp = NULL;


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
  lw->ldap_servers_list = NULL;


  /* Init the RTP stats structure */
  rtp = new (GmRtpData);
  rtp->re_audio_bytes = 0;
  rtp->re_video_bytes = 0;
  rtp->tr_video_bytes = 0;
  rtp->tr_audio_bytes = 0;


  /* Init the TextChat structure */
  chat = new (GmTextChat);


  /* Threads + Locale Init + Gconf */
  g_thread_init (NULL);
  gdk_threads_init ();

  gdk_threads_enter ();
  gconf_init (argc, argv, 0);

  textdomain (GETTEXT_PACKAGE);
  bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");

  /* GnomeMeeting main initialisation */
  gnomemeeting_init (gw, pw, lw, rtp, chat, argc, argv, envp);

  /* Set a default gconf error handler */
  gconf_client_set_error_handling (gconf_client_get_default (),
				   GCONF_CLIENT_HANDLE_UNRETURNED);
  gconf_client_set_global_default_error_handler (gconf_error_callback);


  /* Quick hack to make the GUI refresh even on high load from the other
     threads */
  gtk_timeout_add (500, (GtkFunction) AppbarUpdate, 
  		   rtp);
  /* gtk_timeout_add (10000, (GtkFunction) StressTest, 
     NULL);
  */
  /* The GTK loop */
  gtk_main ();
  gdk_threads_leave ();

  delete (gw);
  delete (lw);
  delete (pw);
  delete (rtp);
  delete (chat); 

  return 0;
}
