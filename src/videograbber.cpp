
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
#include "misc.h"

#include <sys/time.h>
#include <gconf/gconf-client.h>

/* Declarations */
extern GnomeMeeting *MyApp;
extern GtkWidget *gm;


/* The functions */
GMVideoGrabber::GMVideoGrabber ()
  :PThread (1000, NoAutoDeleteThread)
{
  gw = gnomemeeting_get_main_window (gm);
  pw = gnomemeeting_get_pref_window (gm);

  /* Internal state */
  is_running = 1;
  has_to_open = 0;
  has_to_close = 0;
  has_to_reset = 0;
  is_grabbing = 0;
  is_opened = 0;

  /* Initialisation */
  grabber = NULL;
  client = gconf_client_get_default ();
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

  gdk_threads_enter ();

  /* Disable the video settings frame */
  gtk_widget_set_sensitive (GTK_WIDGET (gw->video_settings_frame),
			    FALSE);

  /* Enable the video preview buttons */
  gtk_widget_set_sensitive (GTK_WIDGET (gw->preview_button),
			    TRUE);
  gtk_widget_set_sensitive (GTK_WIDGET (pw->video_preview),
			    TRUE);

  /* Display the logo */
  gnomemeeting_init_main_window_logo ();

  gdk_threads_leave ();
}


void GMVideoGrabber::Main ()
{
  /* Take the mutex, it will be released at the end of the method
     because at the end of the method, the class can be deleted. */
  quit_mutex.Wait ();

  while (is_running == 1) {

    if (has_to_open == 1) {
     
      UpdateConfig ();
      VGOpen ();
    }

    if (has_to_close == 1) {

      VGClose ();
      UpdateConfig ();
    }
      
    /* The user can ask several resets */
    if (has_to_reset >= 1) {

      if (is_opened) {

 	VGClose (0);
 	UpdateConfig ();
 	VGOpen ();
 	is_grabbing = 1;
      }

      has_to_reset--;
      
      /* if we still need to reset more than one time, only reset one time */
      if (has_to_reset >= 1)
	has_to_reset = 1;
    }

    if (is_grabbing == 1) {

      grabbing_mutex.Wait ();
      
      channel->Read (video_buffer, height * width * 3);
      channel->Write (video_buffer, height * width * 3);
      
      grabbing_mutex.Signal ();
    }

    Current()->Sleep (100);
  }

  /* Disable the video preview button
     It will be reenabled in the constructeur
     that must be called just after the Main method exits. */
  gdk_threads_enter ();
  gtk_widget_set_sensitive (GTK_WIDGET (gw->preview_button), FALSE);
  gtk_widget_set_sensitive (GTK_WIDGET (pw->video_preview), FALSE);
  gdk_threads_leave ();
  
  /* if opened, we close */
  if (is_opened == 1) {

    grabber->Close ();
    channel->Close ();
    Current ()->Sleep (500);

    delete (channel);
  }

  quit_mutex.Signal ();
}


void GMVideoGrabber::UpdateConfig ()
{
  g_free (video_device);
 
  video_device =  gconf_client_get_string (GCONF_CLIENT (client), "/apps/gnomemeeting/devices/video_recorder", NULL);
  
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
  /* If the user asks for a synchronous call (typically, from
     OpenVideoChannel), Open synchronously if an async opening
     was not on the road. If an async opening was on the road,
     then we wait till it is opened so that the user can be sure
     that the device is opened when this function returns. */
  if (synchronous == 1) {
    if (has_to_open != 1)
      VGOpen ();
    else
      while (!IsOpened ()) 
	{PThread::Current ()->Sleep (100);}
  }
  else 
    has_to_open = 1;
  
  is_grabbing = has_to_grab;
}


void GMVideoGrabber::Close (int synchronous)
{
  if (synchronous == 1) {
    
    if (has_to_close != 1)
      VGClose ();
  }
  else
    has_to_close = 1;
}


void GMVideoGrabber::Reset (void)
{
  has_to_reset++;
}


void GMVideoGrabber::Start (void)
{
  MyApp->Endpoint ()->SetCurrentDisplay (0);
  is_grabbing = 1;
}


void GMVideoGrabber::Stop (void)
{
  is_grabbing = 0;
}


int GMVideoGrabber::IsOpened (void)
{
  return is_opened;
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
  int error_code = -1;  // No error
  
  device_mutex.Wait ();

  if (!is_opened) {

    /* Disable the video preview button while opening */
    gdk_threads_enter ();
    gtk_widget_set_sensitive (GTK_WIDGET (gw->preview_button), FALSE);
    gtk_widget_set_sensitive (GTK_WIDGET (pw->video_preview), FALSE);
    gnome_appbar_push (GNOME_APPBAR (gw->statusbar), 
		       _("Opening Video device"));
    gnomemeeting_log_insert (_("Opening Video device"));
    
    channel = new PVideoChannel ();
    encoding_device = new GDKVideoOutputDevice (1, gw);
    grabber = new PVideoInputDevice();

    if (video_size == 0) { 
      
      height = GM_QCIF_WIDTH; 
      width = GM_QCIF_HEIGHT; 
    }
    else { 
      
      height = GM_CIF_WIDTH; 
      width = GM_CIF_HEIGHT; 
    }
    

    if (!grabber->Open (video_device, FALSE))
      error_code = 0;
    else
      if (!grabber->SetColourFormatConverter ("YUV420P"))
	error_code = 3;
    else 
      if (!grabber->SetVideoFormat (video_format))
	error_code = 1;
    else
      if (!grabber->SetChannel (video_channel))
	error_code = 2;
    else
      if (!grabber->SetFrameRate (tr_fps))
	error_code = 4;
    else
      if (!grabber->SetFrameSizeConverter (height, width, FALSE))
	error_code = 5;
    
    /* If no error */
    if (error_code == -1) {

      msg = g_strdup_printf 
	(_("Successfully opened video device %s, channel %d"), 
	 video_device, video_channel);
      gnomemeeting_log_insert (msg);
      gnome_appbar_push (GNOME_APPBAR (gw->statusbar), _("Done"));
      g_free (msg);
    }
    else {
    
      msg = g_strdup_printf 
	(_("Error while opening video device %s, channel %d.\nA test image will be transmitted."), video_device, video_channel);
    
      switch (error_code)	{
	
      case 0:
	msg = g_strconcat (msg, "\n", _("Error while opening the device."), 
			   NULL);
	break;
      
      case 1:
	msg = g_strconcat (msg, "\n", _("Your video driver doesn't support the requested video format."), NULL);
	break;

      case 2:
	msg = g_strconcat (msg, "\n", _("Could not open the chosen channel."),
			   NULL);
	break;
      
      case 3:
	msg = g_strconcat (msg, "\n", _("Your driver doesn't support the YUV420P format."), NULL);
	break;
      
      case 4:
	msg = g_strconcat (msg, "\n", _("Error with the frame rate."), NULL);
	break;

      case 5:
	msg = g_strconcat (msg, "\n", _("Error with the frame size."), NULL);
	break;
      }

      gnomemeeting_log_insert (msg);
      g_free (msg);
      

      /* delete the failed grabber and open the fake grabber */
      delete grabber;
      
      grabber = new PFakeVideoInputDevice();
      grabber->SetColourFormatConverter ("YUV420P");
      grabber->SetVideoFormat (PVideoDevice::PAL);
      grabber->SetChannel (150);    
      grabber->SetFrameRate (10);
      grabber->SetFrameSize (height, width);
    }

    grabber->Start ();
    
    channel->AttachVideoReader (grabber);
    channel->AttachVideoPlayer (encoding_device);

    has_to_open = 0;
    is_opened = 1;
  
    encoding_device->SetFrameSize (height, width);  

    /* Setup the video settings */
    GetParameters (&whiteness, &brightness, &colour, &contrast);
 
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
  
    /* Enable the video preview button if not in a call */
    if (MyApp->Endpoint ()->GetCallingState () == 0) {
      
      gtk_widget_set_sensitive (GTK_WIDGET (gw->preview_button),
				TRUE);
      gtk_widget_set_sensitive (GTK_WIDGET (pw->video_preview),
				TRUE);
    }
  }  

  gdk_threads_leave ();

  device_mutex.Signal ();
}
  

void GMVideoGrabber::VGClose (int display_logo)
{
  device_mutex.Wait ();

  if (is_opened) {

    is_grabbing = 0;

    /* Disable the video preview button while closing */
    gdk_threads_enter ();
    gtk_widget_set_sensitive (GTK_WIDGET (gw->preview_button), FALSE);
    gtk_widget_set_sensitive (GTK_WIDGET (pw->video_preview), FALSE);
    gdk_threads_leave ();
    
    grabbing_mutex.Wait ();
    
    grabber->Close ();
    channel->Close ();
    
    grabbing_mutex.Signal ();
    
    delete (channel);
  
    has_to_close = 0;
    is_opened = 0;
  
    /* Enable video preview button */
    gdk_threads_enter ();
  
    /* Enable the video preview button  */
    gtk_widget_set_sensitive (GTK_WIDGET (gw->preview_button), TRUE);
    gtk_widget_set_sensitive (GTK_WIDGET (pw->video_preview), TRUE);
    
    /* Display the logo again */
    if (display_logo)
      gnomemeeting_init_main_window_logo ();

    /* Disable the video settings frame */
    gtk_widget_set_sensitive (GTK_WIDGET (gw->video_settings_frame),
			      FALSE);
  
    gdk_threads_leave ();
    
    /* Initialisation */
    grabber = NULL;
    channel = NULL;
  }

  device_mutex.Signal ();
}

