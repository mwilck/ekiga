
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
 *                         videograbber.cpp  -  description
 *                         --------------------------------
 *   begin                : Mon Feb 12 2001
 *   copyright            : (C) 2000-2001 by Damien Sandras
 *   description          : Video4Linux compliant functions to manipulate the 
 *                          webcam device.
 *   email                : dsandras@seconix.com
 *
 */


#include "../config.h"

#include "config.h"
#include "videograbber.h"
#include "misc.h"
#include "gdkvideoio.h"
#include "common.h"
#include "gnomemeeting.h"
#include "dialog.h"
#include "misc.h"
#include "menu.h"
#include "vfakeio.h"


#include <sys/time.h>
#include <gconf/gconf-client.h>
#ifndef DISABLE_GNOME
#include <gnome.h>
#else
#include <gtk/gtk.h>
#endif


/* Declarations */
extern GnomeMeeting *MyApp;
extern GtkWidget *gm;


/* The functions */
GMVideoGrabber::GMVideoGrabber (PIntCondMutex *m,
				BOOL start_grabbing,
				BOOL sync)
  :PThread (1000, AutoDeleteThread)
{
  height = 0, width = 0;
  whiteness = 0, brightness = 0, colour = 0, contrast = 0;

  gw = gnomemeeting_get_main_window (gm);
  pw = gnomemeeting_get_pref_window (gm);
  dw = gnomemeeting_get_druid_window (gm);
  
  /* Internal state */
  is_running = 1;
  has_to_close = 0;
  has_to_reset = 0;
  has_to_stop = 0;
  is_grabbing = start_grabbing;
  synchronous = sync;
  is_opened = 0;

  /* Initialisation */
  encoding_device = NULL;
  channel = NULL;
  grabber = NULL;
  encoding_device = NULL;
  client = gconf_client_get_default ();
  color_format = NULL;
  video_device = NULL;
  video_channel = 0;
  video_size = 0;
  video_format = PVideoDevice::Auto;
  UpdateConfig ();

  if (synchronous)
    VGOpen ();

  number_of_instances_mutex = m;

  ++(*number_of_instances_mutex);
  
  /* Start the thread */
  this->Resume ();
}


GMVideoGrabber::~GMVideoGrabber ()
{
  PWaitAndSignal m(device_mutex);

  g_free (color_format);
  g_free (video_device);

  --(*number_of_instances_mutex);
}


void GMVideoGrabber::Main ()
{
  static PMutex mutex;
  PWaitAndSignal m(mutex);

  
  /* We need those variables to facilitate the protection around mutexes */
  int to_reset = 0, to_run = 0;

  if (!synchronous)
    VGOpen ();

  var_mutex.Wait ();
  to_run = is_running;
  var_mutex.Signal ();
  
  while (to_run == 1) {

    /* We protect variables that can be modified from several threads */
    var_mutex.Wait ();
    to_reset = has_to_reset;
    to_run = is_running;
    var_mutex.Signal ();

    /* The user can ask several resets */
    if (to_reset >= 1) {

      VGClose ();
      UpdateConfig ();
      VGOpen ();
      
      var_mutex.Wait ();
      has_to_reset--;
      is_grabbing = 1;
      
      /* if we still need to reset more than one time, only reset one time */
      if (has_to_reset >= 1) 
	has_to_reset = 1;
      var_mutex.Signal ();
    }

    if (is_grabbing == 1) {

      channel->Read (video_buffer, height * width * 3);
      channel->Write (video_buffer, height * width * 3);    
    }

    Current()->Sleep (50);
  }

  VGClose ();

  gnomemeeting_threads_enter ();
  gnomemeeting_init_main_window_logo (gw->main_video_image);
  gnomemeeting_threads_leave ();
}


void GMVideoGrabber::UpdateConfig ()
{
  g_free (color_format);
  g_free (video_device);
 
  gnomemeeting_threads_enter ();
  video_device =  gconf_client_get_string (GCONF_CLIENT (client), "/apps/gnomemeeting/devices/video_recorder", NULL);

  color_format = gconf_client_get_string (GCONF_CLIENT (client), "/apps/gnomemeeting/devices/color_format", NULL);

  video_channel =  gconf_client_get_int (GCONF_CLIENT (client), "/apps/gnomemeeting/devices/video_channel", NULL);

  video_size =  gconf_client_get_int (GCONF_CLIENT (client), "/apps/gnomemeeting/devices/video_size", NULL);


  switch (gconf_client_get_int (GCONF_CLIENT (client), "/apps/gnomemeeting/devices/video_format", NULL)) {
    
  case 0:
    video_format = PVideoDevice::PAL;
    break;

  case 1:
    video_format = PVideoDevice::NTSC;
    break;

  case 2:
    video_format = PVideoDevice::SECAM;
    break;

  case 3:
    video_format = PVideoDevice::Auto;
    break;

  default:
    video_format = PVideoDevice::Auto;
    break;
  }
  gnomemeeting_threads_leave ();

}


void GMVideoGrabber::Close (void)
{
  var_mutex.Wait ();
  is_running = 0;
  var_mutex.Signal ();
}


void GMVideoGrabber::Reset (void)
{
  var_mutex.Wait ();
  has_to_reset++;
  var_mutex.Signal ();
}


void GMVideoGrabber::StartGrabbing (void)
{
  if (is_opened) {

    var_mutex.Wait ();
    is_grabbing = 1;
    var_mutex.Signal ();
  }
}


void GMVideoGrabber::StopGrabbing (void)
{
  var_mutex.Wait ();
  is_grabbing = 0;
  var_mutex.Signal ();
}


GDKVideoOutputDevice *GMVideoGrabber::GetEncodingDevice (void)
{
  PWaitAndSignal m(device_mutex);
  
  return encoding_device;
}


PVideoChannel *GMVideoGrabber::GetVideoChannel (void)
{
  PWaitAndSignal m(device_mutex);
  
  return channel;
}


void GMVideoGrabber::SetColour (int colour)
{
  grabber->SetColour (colour);
}


void GMVideoGrabber::SetBrightness (int brightness)
{
  grabber->SetBrightness (brightness);
}


void GMVideoGrabber::SetWhiteness (int whiteness)
{
  grabber->SetWhiteness (whiteness);
}


void GMVideoGrabber::SetContrast (int constrast)
{
  grabber->SetContrast (constrast);
}


void GMVideoGrabber::GetParameters (int *whiteness, int *brightness, 
				    int *colour, int *contrast)
{
  int hue;
  PWaitAndSignal m(device_mutex);
  
  grabber->GetParameters (whiteness, brightness, colour, contrast, &hue);

  *whiteness = (int) *whiteness / 256;
  *brightness = (int) *brightness / 256;
  *colour = (int) *colour / 256;
  *contrast = (int) *contrast / 256;
}


void GMVideoGrabber::VGOpen (void)
{
  gchar *msg = NULL;
  gchar *video_image = NULL;
  int error_code = -1;  // No error
  int opened;

  PWaitAndSignal m(device_mutex);
  
  var_mutex.Wait ();
  opened = is_opened;
  var_mutex.Signal ();

  if (!opened) {

    /* Disable the video preview button while opening */
    gnomemeeting_threads_enter ();
    gtk_widget_set_sensitive (GTK_WIDGET (gw->preview_button), FALSE);
#ifndef DISABLE_GNOME
    gtk_widget_set_sensitive (GTK_WIDGET (dw->video_test_button), FALSE);
#endif
    gnomemeeting_statusbar_flash (gw->statusbar, _("Opening Video device"));
    gnomemeeting_log_insert (gw->history_text_view, _("Opening Video device"));
    gnomemeeting_threads_leave ();
    
#ifdef TRY_1394AVC
    if (video_device == "/dev/raw1394" ||
       strncmp (video_device, "/dev/video1394", 14) == 0) {
            grabber = new PVideoInput1394AvcDevice();
       }
    else
#endif
#ifdef TRY_1394DC
    if (video_device == "/dev/raw1394" ||
        strncmp (video_device, "/dev/video1394", 14) == 0)
           grabber = new PVideoInput1394DcDevice();
    else
#endif
      {
	 grabber = new PVideoInputDevice();
      }

    if (video_size == 0) { 
      
      height = GM_QCIF_HEIGHT; 
      width = GM_QCIF_WIDTH; 
    }
    else { 
      
      height = GM_CIF_HEIGHT; 
      width = GM_CIF_WIDTH; 
    }
    
    grabber->SetPreferredColourFormat (color_format);

    /* no error if Picture is choosen as video device */
    if (!strcmp (video_device, _("Picture")))
      error_code = -2;

    if (error_code != -2) {

      if (!grabber->Open (video_device, FALSE))
	error_code = 0;
      else
	if (!grabber->SetVideoFormat(video_format))
	  error_code = 2;
      else
        if (!grabber->SetChannel(video_channel))
          error_code = 2;
      else
	if (!grabber->SetFrameSizeConverter (width, height, FALSE))
	   error_code = 5;
      else
	if (!grabber->SetColourFormatConverter ("YUV420P"))
	  error_code = 3;
      else
	if (!grabber->SetFrameRate (30))
	  error_code = 4;
    }


    /* If no error */
    if (error_code == -1) {

      gnomemeeting_threads_enter ();
      msg = g_strdup_printf 
	(_("Successfully opened video device %s, channel %d"), 
	 video_device, video_channel);
      gnomemeeting_log_insert (gw->history_text_view, msg);
      gnomemeeting_statusbar_flash (gw->statusbar, _("Video Device Opened"));
      g_free (msg);
      gnomemeeting_threads_leave ();
    }
    else {
    
      /* If we want to open the fake device for a real error, and not because
	 the user chose the Picture device */

      if (error_code != -2) {

	gnomemeeting_threads_enter ();
	msg = g_strdup_printf 
	  (_("Error while opening video device %s, channel %d.\nThe chosen Video Image will be transmitted during calls. If you didn't choose any image, then the default GnomeMeeting logo will be transmitted. Notice that you can always transmit a given image or the GnomeMeeting logo by choosing \"Picture\" as video device."), video_device, video_channel);
	gnomemeeting_statusbar_flash (gw->statusbar, _("Can't open the Video Device"));
	
	switch (error_code)	{
	  
	case 0:
	  msg = g_strconcat (msg, "\n", _("Error while opening the device."), 
			     NULL);
	  break;
	  
	case 1:
	  msg = g_strconcat (msg, "\n", _("Your video driver doesn't support the requested video format."), NULL);
	  break;

	case 2:
	  msg = g_strconcat (msg, "\n", _("Could not open the chosen channel with the chosen video format."), NULL);
	  break;
      
	case 3:
	  msg = g_strconcat (msg, "\n", g_strdup_printf(_("Your driver doesn't support the %s format.\n Please check your kernel driver documentation in order to determine which Palette is supported. Set it as GnomeMeeting default with:\n gconftool --set \"/apps/gnomemeeting/devices/color_format\" YOURPALETTE --type string"), color_format), NULL);
	  break;
      
	case 4:
	  msg = g_strconcat (msg, "\n", _("Error with the frame rate."), NULL);
	  break;

	case 5:
	  msg = g_strconcat (msg, "\n", _("Error with the frame size."), NULL);
	  break;
	}

	gnomemeeting_warning_dialog_on_widget (GTK_WINDOW (gm), gw->preview_button, msg);
	g_free (msg);

	gnomemeeting_threads_leave ();
      }

      /* delete the failed grabber and open the fake grabber, either
       because there was an error, either because the user chose to do so */
      delete grabber;
      
      gnomemeeting_threads_enter ();
      video_image = gconf_client_get_string (GCONF_CLIENT (client), "/apps/gnomemeeting/devices/video_image", NULL);
      gnomemeeting_threads_leave ();

      grabber = new GMH323FakeVideoInputDevice (video_image);
      grabber->SetColourFormatConverter ("YUV420P");
      grabber->SetVideoFormat (PVideoDevice::PAL);
      grabber->SetChannel (1);    
      grabber->SetFrameRate (6);
      grabber->SetFrameSizeConverter (width, height, FALSE);

      gnomemeeting_threads_enter ();
#ifndef DISABLE_GNOME
      gtk_widget_set_sensitive (GTK_WIDGET (dw->video_test_button), true);
#endif
      gnomemeeting_threads_leave ();
      
      g_free (video_image);
    }

    
    grabber->Start ();

    var_mutex.Wait ();
    channel = new PVideoChannel ();
    encoding_device = new GDKVideoOutputDevice (1, gw);
    var_mutex.Signal ();
    
    channel->AttachVideoReader (grabber);
    channel->AttachVideoPlayer (encoding_device);

    var_mutex.Wait ();
    is_opened = 1;
    var_mutex.Signal ();
  
    encoding_device->SetFrameSize (width, height);  

    /* Setup the video settings */
    GetParameters (&whiteness, &brightness, &colour, &contrast);
    gnomemeeting_threads_enter ();
    gtk_adjustment_set_value (GTK_ADJUSTMENT (gw->adj_brightness),
			      brightness);
    gtk_adjustment_set_value (GTK_ADJUSTMENT (gw->adj_whiteness),
			      whiteness);
    gtk_adjustment_set_value (GTK_ADJUSTMENT (gw->adj_colour),
			      colour);
    gtk_adjustment_set_value (GTK_ADJUSTMENT (gw->adj_contrast),
			      contrast);
    
    /* Enable the video settings frame */
    gtk_widget_set_sensitive (GTK_WIDGET (gw->video_settings_frame),
			      TRUE);
    gnomemeeting_threads_leave ();

    /* Enable the video preview button if not in a call */
    if (MyApp->Endpoint ()->GetCallingState () == 0) {

      gnomemeeting_threads_enter ();      
      gtk_widget_set_sensitive (GTK_WIDGET (gw->preview_button), true);
      gnomemeeting_threads_leave ();
    }


    /* Enable zoom and fullscreen settings in the view menu */
    gnomemeeting_threads_enter ();
    gnomemeeting_zoom_submenu_set_sensitive (TRUE);

#ifdef HAS_SDL
    gnomemeeting_fullscreen_option_set_sensitive (TRUE);
#endif

    gnomemeeting_threads_leave ();
  }  
}
  

void GMVideoGrabber::VGClose ()
{
  int opened;

  PWaitAndSignal m(device_mutex);  

  var_mutex.Wait ();
  opened = is_opened;
  var_mutex.Signal ();

  if (opened) {

    var_mutex.Wait ();
    is_grabbing = 0;
    var_mutex.Signal ();

    /* Disable the video preview button while closing */
    gnomemeeting_threads_enter ();
    gtk_widget_set_sensitive (GTK_WIDGET (gw->preview_button), FALSE);
#ifndef DISABLE_GNOME
    gtk_widget_set_sensitive (GTK_WIDGET (dw->video_test_button), FALSE);
#endif
    gnomemeeting_threads_leave ();
    
    if (channel) 
      delete (channel);

    var_mutex.Wait ();
    has_to_close = 0;
    is_opened = 0;
    var_mutex.Signal ();
  
    /* Enable video preview button */
    gnomemeeting_threads_enter ();
  
    /* Enable the video preview button  */
    gtk_widget_set_sensitive (GTK_WIDGET (gw->preview_button), TRUE);
#ifndef DISABLE_GNOME
    gtk_widget_set_sensitive (GTK_WIDGET (dw->video_test_button), TRUE);
#endif
    
    gnomemeeting_statusbar_flash (gw->statusbar, _("Video Device Closed"));
    
    /* Disable the video settings frame */
    gtk_widget_set_sensitive (GTK_WIDGET (gw->video_settings_frame),
			      FALSE);
  
    gnomemeeting_threads_leave ();
    
    
    /* Disable the zoom and fullscreen options */
    gnomemeeting_threads_enter ();
    gnomemeeting_zoom_submenu_set_sensitive (FALSE);

#ifdef HAS_SDL
    gnomemeeting_fullscreen_option_set_sensitive (FALSE);
#endif

    gnomemeeting_threads_leave ();


    /* Initialisation */
    grabber = NULL;
    var_mutex.Wait ();
    channel = NULL;
    encoding_device = NULL;
    var_mutex.Signal ();
  }

  /* Quick Hack for buggy drivers that return from the ioctl before the device
     is really closed */
  PThread::Current ()->Sleep (1000);
}


/* The video tester class */
GMVideoTester::GMVideoTester ()
  :PThread (1000, AutoDeleteThread)
{
#ifndef DISABLE_GNOME

  this->Resume ();
#endif
}


GMVideoTester::~GMVideoTester ()
{
#ifndef DISABLE_GNOME
  quit_mutex.Wait ();

  quit_mutex.Signal ();
 #endif
}


void GMVideoTester::Main ()
{
#ifndef DISABLE_GNOME
  quit_mutex.Wait ();

  GmWindow *gw = NULL;
  GmDruidWindow *dw = NULL;
  
  PVideoInputDevice *grabber = new PVideoInputDevice();
  
  int height = GM_QCIF_HEIGHT; 
  int width = GM_QCIF_WIDTH; 
  int error_code = -1;
  int cpt = 0;
  gdouble per = 0.0;

  gchar *msg = NULL;
  gchar *video_device = NULL;

  gnomemeeting_threads_enter ();
  GConfClient *client = gconf_client_get_default ();

  video_device =  gconf_client_get_string (GCONF_CLIENT (client), "/apps/gnomemeeting/devices/video_recorder", NULL);

  gw = gnomemeeting_get_main_window (gm);
  dw = gnomemeeting_get_druid_window (gm);
  gtk_widget_set_sensitive (GTK_WIDGET (dw->video_test_button), FALSE);
  gtk_progress_bar_set_text (GTK_PROGRESS_BAR (dw->progress), "");
  gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (dw->progress), 0.0);
  gnomemeeting_threads_leave ();

  
  while (cpt <= 3) {

    gdk_threads_enter ();
    gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (dw->progress), per);
    gtk_widget_queue_draw (GTK_WIDGET (dw->progress));
    gdk_threads_leave ();
    
    if (strcmp (video_device, _("Picture"))) {
      
      if (!grabber->Open (video_device, FALSE))
	error_code = 0;
      else
	if (!grabber->SetVideoChannelFormat (0,  PVideoDevice::Auto))
	  error_code = 2;
	else
	  if (!grabber->SetColourFormatConverter ("YUV420P"))
	    error_code = 3;
	  else
	    if (!grabber->SetFrameRate (10))
	      error_code = 4;
	    else
	      if (!grabber->SetFrameSizeConverter (width, height, FALSE))
		error_code = 5;
	      else
		grabber->Close ();

      if (error_code != -1)
	break;
    }
    
    msg = g_strdup_printf (_("Test %d done"), cpt);
    gdk_threads_enter ();
    gtk_progress_bar_set_text (GTK_PROGRESS_BAR (dw->progress), msg);
    gdk_threads_leave ();
    g_free (msg);

    per = cpt * 0.33;
    cpt++;
    PThread::Current () ->Sleep (100);
  }


  if (error_code != - 1) {
    
    switch (error_code)	{
	  
    case 0:
      msg = g_strdup_printf (_("Error while opening %s."), video_device);
      break;
      
    case 1:
      msg = g_strdup_printf (_("Your video driver doesn't support the requested video format."));
      break;
      
    case 2:
      msg = g_strdup_printf (_("Could not open the chosen channel with the chosen video format."));
      break;
      
    case 3:
      msg = g_strdup_printf (_("Your driver doesn't support the YUV420P color format"));
      break;
      
    case 4:
      msg = g_strdup_printf ( _("Error with the frame rate."));
      break;
      
    case 5:
      msg = g_strdup_printf (_("Error with the frame size."));
      break;
    }
  }  
  else
    msg = g_strdup (_("Tests OK!"));

  gdk_threads_enter ();
  gtk_progress_bar_set_text (GTK_PROGRESS_BAR (dw->progress), msg);
  gdk_threads_leave ();
  g_free (msg);

  delete (grabber);

  gdk_threads_enter ();
  gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (dw->progress), 1.0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dw->video_test_button),
				FALSE);
  gtk_widget_set_sensitive (GTK_WIDGET (dw->video_test_button), TRUE);
  gdk_threads_leave ();

  quit_mutex.Signal ();
#endif
}


