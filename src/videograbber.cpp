
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
GMVideoGrabber::GMVideoGrabber ()
  :PThread (1000, NoAutoDeleteThread)
{
  height = 0, width = 0;
  whiteness = 0, brightness = 0, colour = 0, contrast = 0;

  gw = gnomemeeting_get_main_window (gm);
  pw = gnomemeeting_get_pref_window (gm);

  /* Internal state */
  is_running = 1;
  has_to_open = 0;
  has_to_close = 0;
  has_to_reset = 0;
  has_to_stop = 0;
  is_grabbing = 0;
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
  tr_fps = 0;
  UpdateConfig ();

  /* Start the thread */
  this->Resume ();
}


GMVideoGrabber::~GMVideoGrabber ()
{ 
  is_running = 0;
  is_grabbing = 0;

  /* So that we can wait, in the Thread (Main ())
     till the channel and grabber are closed, after the Main method
     has exited */
  quit_mutex.Wait ();


  gnomemeeting_threads_enter ();

  /* Disable the video settings frame */
  gtk_widget_set_sensitive (GTK_WIDGET (gw->video_settings_frame),
			    FALSE);

  /* Enable the video preview buttons */
  gtk_widget_set_sensitive (GTK_WIDGET (gw->preview_button),
			    TRUE);
  gtk_widget_set_sensitive (GTK_WIDGET (gw->video_test_button),
			    TRUE);

  /* Display the logo */
  gnomemeeting_init_main_window_logo (gw->main_video_image);

  gnomemeeting_threads_leave ();

  g_free (video_device);
  g_free (color_format);
}


void GMVideoGrabber::Main ()
{
  /* We need those variables to facilitate the protection around mutexes */
  int to_open, to_close, to_reset;

  /* Take the mutex, it will be released at the end of the method
     because at the end of the method, the class can be deleted. */
  quit_mutex.Wait ();

  while (is_running == 1) {

    /* We protect variables that can be modified from several threads */
    var_mutex.Wait ();
    to_open = has_to_open;
    to_close = has_to_close;
    to_reset = has_to_reset;
    var_mutex.Signal ();

    if (to_open == 1) {
     
      UpdateConfig ();
      VGOpen ();
    }

    if (to_close == 1) {

      VGClose ();
      UpdateConfig ();
    }
      
    /* The user can ask several resets */
    if (to_reset >= 1) {

      if (IsOpened ()) {

 	VGClose (0);
	UpdateConfig ();
 	VGOpen ();

	var_mutex.Wait ();
 	is_grabbing = 1;
	var_mutex.Signal ();
      }

      var_mutex.Wait ();
      has_to_reset--;
      
      /* if we still need to reset more than one time, only reset one time */
      if (has_to_reset >= 1) 
	has_to_reset = 1;
      var_mutex.Signal ();
    }

    if (is_grabbing == 1) {

      device_mutex.Wait ();
      grabbing_mutex.Wait ();
      
      if (IsOpened ()) {

	channel->Read (video_buffer, height * width * 3);
	channel->Write (video_buffer, height * width * 3);
      }
    
      grabbing_mutex.Signal ();
      device_mutex.Signal ();
    }

    Current()->Sleep (50);
  }

  /* Disable the video preview button
     It will be reenabled in the constructeur
     that must be called just after the Main method exits. */
  gnomemeeting_threads_enter ();
  gtk_widget_set_sensitive (GTK_WIDGET (gw->preview_button), FALSE);
  gtk_widget_set_sensitive (GTK_WIDGET (gw->video_test_button), FALSE);
  gnomemeeting_threads_leave ();
  
  /* if opened, we close */
  if (IsOpened ()) {

    grabber->Close ();
    channel->Close ();
    Current ()->Sleep (500);

    delete (channel);
  }

  quit_mutex.Signal ();
}


void GMVideoGrabber::UpdateConfig ()
{
  g_free (color_format);
  g_free (video_device);
 
  video_device =  gconf_client_get_string (GCONF_CLIENT (client), "/apps/gnomemeeting/devices/video_recorder", NULL);

  color_format = gconf_client_get_string (GCONF_CLIENT (client), "/apps/gnomemeeting/devices/color_format", NULL);

  video_channel =  gconf_client_get_int (GCONF_CLIENT (client), "/apps/gnomemeeting/devices/video_channel", NULL);

  video_size =  gconf_client_get_int (GCONF_CLIENT (client), "/apps/gnomemeeting/devices/video_size", NULL);


  /* The number of Transmitted FPS must be equal to 0 to disable */
  if (!gconf_client_get_bool (client, "/apps/gnomemeeting/video_settings/enable_fps", NULL))
    tr_fps = 0;
  else
    tr_fps = gconf_client_get_int (client, "/apps/gnomemeeting/video_settings/tr_fps", NULL);

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
}


void GMVideoGrabber::Open (int has_to_grab, int synchronous)
{
  int to_open;

  /* Make sure to open with the right config */
  UpdateConfig ();

  var_mutex.Wait ();
  to_open = has_to_open;
  var_mutex.Signal ();

  /* If the user asks for a synchronous call (typically, from
     OpenVideoChannel), Open synchronously if an async opening
     was not on the road. If an async opening was on the road,
     then we wait till it is opened so that the user can be sure
     that the device is opened when this function returns. */
  if (synchronous == 1) {
    if (to_open != 1)
      VGOpen ();
    else
      while (!IsOpened ()) 
	{PThread::Current ()->Sleep (100);}
  }
  else {

    var_mutex.Wait ();
    has_to_open = 1;
    var_mutex.Signal ();
  }
    
  var_mutex.Wait ();
  is_grabbing = has_to_grab;
  var_mutex.Signal ();
}


void GMVideoGrabber::Close (int synchronous)
{
  int to_close;

  var_mutex.Wait ();
  to_close = has_to_close;
  var_mutex.Signal ();

  if (synchronous == 1) {

    if (to_close != 1)
      VGClose (FALSE);
  }
  else {

    var_mutex.Wait ();
    has_to_close = 1;
    var_mutex.Signal ();
  }
}


void GMVideoGrabber::Reset (void)
{
  var_mutex.Wait ();
  has_to_reset++;
  var_mutex.Signal ();
}


void GMVideoGrabber::Start (void)
{
  var_mutex.Wait ();
  is_grabbing = 1;
  var_mutex.Signal ();
}


void GMVideoGrabber::Stop (void)
{
  var_mutex.Wait ();
  is_grabbing = 0;
  var_mutex.Signal ();
}


void GMVideoGrabber::VGStop (void)
{
  var_mutex.Wait ();
  is_grabbing = 0;
  has_to_stop = 0;
  var_mutex.Signal ();
}


int GMVideoGrabber::IsOpened (void)
{
  int is_open;

  var_mutex.Wait ();
  is_open = is_opened;
  var_mutex.Signal ();
  
  return is_open;
}


GDKVideoOutputDevice *GMVideoGrabber::GetEncodingDevice (void)
{
  return encoding_device;
}


PVideoChannel *GMVideoGrabber::GetVideoChannel (void)
{
  return channel;
}


void GMVideoGrabber::SetFrameRate (int fps)
{
  fps = PMAX (0, PMIN (fps, 30));

  if (grabber != NULL)
    grabber->SetFrameRate (fps);
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

  grabber->GetParameters (whiteness, brightness, colour, contrast, &hue);

  *whiteness = (int) *whiteness / 256;
  *brightness = (int) *brightness / 256;
  *colour = (int) *colour / 256;
  *contrast = (int) *contrast / 256;
}


void GMVideoGrabber::SetFrameSize (int height, int width)
{
  if ((encoding_device != NULL) && (grabber != NULL) && (is_opened)) {
    
    grabber->SetFrameSizeConverter (height, width, FALSE);
    encoding_device->SetFrameSize (height, width);
  }
}


void GMVideoGrabber::SetChannel (int cnum)
{
  if (grabber != NULL)
    grabber->SetChannel (cnum);
}


void GMVideoGrabber::VGOpen (void)
{
  gchar *msg = NULL;
  gchar *video_image = NULL;
  int error_code = -1;  // No error
  int opened;

  device_mutex.Wait ();

  var_mutex.Wait ();
  opened = is_opened;
  var_mutex.Signal ();

  if (!opened) {

    /* Disable the video preview button while opening */
    gnomemeeting_threads_enter ();
    gtk_widget_set_sensitive (GTK_WIDGET (gw->preview_button), FALSE);
    gtk_widget_set_sensitive (GTK_WIDGET (gw->video_test_button), FALSE);
    gnomemeeting_statusbar_flash (gw->statusbar, _("Opening Video device"));
    gnomemeeting_log_insert (gw->history_text_view, _("Opening Video device"));
    gnomemeeting_threads_leave ();
    
    channel = new PVideoChannel ();
    encoding_device = new GDKVideoOutputDevice (1, gw);
    grabber = new PVideoInputDevice();

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
	if (!grabber->SetVideoChannelFormat (video_channel, video_format))
          error_code = 2;
      else
	if (!grabber->SetColourFormatConverter ("YUV420P"))
	  error_code = 3;
      else
	if (!grabber->SetFrameRate (tr_fps))
	  error_code = 4;
      else
	if (!grabber->SetFrameSizeConverter (width, height, FALSE))
	   error_code = 5;
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
      
      video_image = gconf_client_get_string (GCONF_CLIENT (client), "/apps/gnomemeeting/devices/video_image", NULL);

      grabber = new GMH323FakeVideoInputDevice (video_image);
      grabber->SetColourFormatConverter ("YUV420P");
      grabber->SetVideoFormat (PVideoDevice::PAL);
      grabber->SetChannel (1);    
      grabber->SetFrameRate (6);
      grabber->SetFrameSizeConverter (width, height, FALSE);

      g_free (video_image);
    }

    grabber->Start ();
    
    channel->AttachVideoReader (grabber);
    channel->AttachVideoPlayer (encoding_device);


    var_mutex.Wait ();
    has_to_open = 0;
    is_opened = 1;
    var_mutex.Signal ();
  
    /* If error */
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
      gtk_widget_set_sensitive (GTK_WIDGET (gw->preview_button),
				TRUE);
      gtk_widget_set_sensitive (GTK_WIDGET (gw->video_test_button),
				TRUE);
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

  device_mutex.Signal ();
}
  

void GMVideoGrabber::VGClose (int display_logo)
{
  int opened;

  grabbing_mutex.Wait ();
  device_mutex.Wait ();

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
    gtk_widget_set_sensitive (GTK_WIDGET (gw->video_test_button), FALSE);
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
    gtk_widget_set_sensitive (GTK_WIDGET (gw->video_test_button), TRUE);
    
    /* Display the logo again */
    if (display_logo)
      gnomemeeting_init_main_window_logo (gw->main_video_image);


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
    channel = NULL;
    encoding_device = NULL;
  }

  /* Quick Hack for buggy drivers that return from the ioctl before the device
     is really closed */
  PThread::Current ()->Sleep (1000);

  grabbing_mutex.Signal ();
  device_mutex.Signal ();
}


/* The video tester class */
GMVideoTester::GMVideoTester (GtkWidget *p, GtkWindow *w)
  :PThread (1000, AutoDeleteThread)
{
  progress = p;
  window = w;

  this->Resume ();
}


GMVideoTester::~GMVideoTester ()
{
  quit_mutex.Wait ();

  quit_mutex.Signal ();
}


void GMVideoTester::Main ()
{
  quit_mutex.Wait ();

  PVideoInputDevice *grabber = new PVideoInputDevice();
  int height = GM_QCIF_HEIGHT; 
  int width = GM_QCIF_WIDTH; 
  int error_code = -1;
  int cpt = 0;
  gdouble per = 0.0;

  gchar *msg = NULL;
  gchar *video_device = NULL;

  GConfClient *client = gconf_client_get_default ();

  video_device =  gconf_client_get_string (GCONF_CLIENT (client), "/apps/gnomemeeting/devices/video_recorder", NULL);

  while (cpt <= 5) {

    gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (progress), per);
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

    per = cpt * 0.25;
    cpt++;
  }


  if (error_code != - 1) {
    
    msg = g_strdup_printf (_("Error while opening the video device %s."),
			   video_device);

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
      msg = g_strconcat (msg, "\n", g_strdup_printf(_("Your driver doesn't support the %s format.\n Please check your kernel driver documentation in order to determine which Palette is supported. Set it as GnomeMeeting default with:\n gconftool --set \"/apps/gnomemeeting/devices/color_format\" YOURPALETTE --type string"), "YUV420P"), NULL);
      break;
      
    case 4:
      msg = g_strconcat (msg, "\n", _("Error with the frame rate."), NULL);
      break;
      
    case 5:
      msg = g_strconcat (msg, "\n", _("Error with the frame size."), NULL);
      break;
    }
  }  
  else
    msg = g_strdup (_("GnomeMeeting successfully tested your video device. The driver seems to be usable for videoconferencing with GnomeMeeting."));

  gnomemeeting_threads_enter ();
  if (error_code == - 1)
    gnomemeeting_message_dialog (GTK_WINDOW (window), msg);
  else
    gnomemeeting_error_dialog (GTK_WINDOW (window), msg);
  gnomemeeting_threads_leave ();
  
  g_free (msg);

  delete (grabber);
  quit_mutex.Signal ();
}


