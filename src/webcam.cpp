/***************************************************************************
                          webcam.cxx  -  description
                             -------------------
    begin                : Mon Feb 12 2001
    copyright            : (C) 2001 by Damien Sandras
    description          : Video4Linux compliant functions to manipulate the 
                           webcam device
    email                : dsandras@acm.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "webcam.h"
#include "main_interface.h"
#include "gdkvideoio.h"
#include "config.h"
#include "common.h"
#include "main.h"

#include <sys/time.h>

extern GnomeMeeting *MyApp;


GMH323Webcam::GMH323Webcam (GM_window_widgets *g, options *o)
  :PThread (1000, AutoDeleteThread)
{
  gw = g;
  opts = o;

  channel = new PVideoChannel ();
  encoding_device = new GDKVideoOutputDevice (1, gw);
  grabber = new PVideoInputDevice();

  running = 1;
  grabbing = 1;
  initialised = 0;
  sensitivity_change = 0;

  gw->video_grabber_thread_count++;

  this->Resume ();
}


GMH323Webcam::~GMH323Webcam ()
{ 
  usleep (800);

  while (!this->IsTerminated ())
    usleep (100);

  gdk_threads_enter ();

  if (sensitivity_change)
    {
      // Enable the video preview button
      gtk_widget_set_sensitive (GTK_WIDGET (gw->preview_button), TRUE);
    }

  // Disable the video settings frame
  gtk_widget_set_sensitive (GTK_WIDGET (gw->video_settings_frame),
			    FALSE);

  // Display the logo
  GM_init_main_interface_logo (gw);
  gdk_threads_leave ();

  gw->video_grabber_thread_count--;
}


void GMH323Webcam::Main ()
{
  int height = 0, width = 0;
  int whiteness = 0, brightness = 0, colour = 0, contrast = 0;

  if (opts->video_size == 0)
    { height = GM_QCIF_WIDTH; width = GM_QCIF_HEIGHT; }
  else
    { height = GM_CIF_WIDTH; width = GM_CIF_HEIGHT; }

  Initialise ();

  // Enable the video preview button
  gdk_threads_enter ();
  gtk_widget_set_sensitive (GTK_WIDGET (gw->preview_button), TRUE);

  // Setup the video settings

  GetParameters (&whiteness, &brightness, &colour, &contrast);

  gtk_adjustment_set_value (GTK_ADJUSTMENT (gw->adj_brightness),
			    brightness);
  gtk_adjustment_set_value (GTK_ADJUSTMENT (gw->adj_whiteness),
			    whiteness);
  gtk_adjustment_set_value (GTK_ADJUSTMENT (gw->adj_colour),
			    colour);
  gtk_adjustment_set_value (GTK_ADJUSTMENT (gw->adj_contrast),
			    contrast);

  // Enable the video settings frame
  gtk_widget_set_sensitive (GTK_WIDGET (gw->video_settings_frame),
			    TRUE);

  gdk_threads_leave ();

  initialised = 1;

  while (running == 1)
    {
      if (grabbing == 1)
	{
	  channel->Read (video_buffer, height * width * 3);
	  channel->Write (video_buffer, height * width * 3);
	}

      Current()->Sleep (10);
    }

  if (sensitivity_change)
    {
      // Disable the video preview button
      gdk_threads_enter ();
      gtk_widget_set_sensitive (GTK_WIDGET (gw->preview_button), FALSE);
      gdk_threads_leave ();
    }

  grabber->Close ();
  channel->Close ();
  Current ()->Sleep (500);

  delete (channel);
}


void GMH323Webcam::Initialise (void)
{
  int height, width;
  gchar *msg = NULL;

  if (opts->video_size == 0)
    { height = GM_QCIF_WIDTH; width = GM_QCIF_HEIGHT; }
  else
    { height = GM_CIF_WIDTH; width = GM_CIF_HEIGHT; }
    
  encoding_device->SetFrameSize (height, width);

  if (grabber->Open (opts->video_device, FALSE) &&
      grabber->SetVideoFormat 
      (opts->video_format ? PVideoDevice::NTSC : PVideoDevice::PAL) &&
      grabber->SetChannel (opts->video_channel) &&
      grabber->SetColourFormatConverter ("YUV420P") &&
      grabber->SetFrameRate (opts->tr_fps)  &&
      grabber->SetFrameSizeConverter (height, width, FALSE))

    {
      gdk_threads_enter ();

      msg = g_strdup_printf 
	(_("Successfully opened video device %s, channel %d"), 
	 opts->video_device, opts->video_channel);
      GM_log_insert (gw->log_text, msg);
      g_free (msg);

      gdk_threads_leave ();			     
    }
  else
    {
      gdk_threads_enter ();

      msg = g_strdup_printf 
	(_("Error while opening video device %s, channel %d. A test image will be transmitted."), 
	 opts->video_device, opts->video_channel);
      GM_log_insert (gw->log_text, msg);
      g_free (msg);

      gdk_threads_leave ();			     


      // delete the failed grabber and open the fake grabber
      delete grabber;
      
      grabber = new PFakeVideoInputDevice();
      grabber->SetColourFormatConverter ("YUV420P");
      grabber->SetVideoFormat (PVideoDevice::PAL);
      grabber->SetChannel (100);     //NTSC static image.
      grabber->SetFrameRate (10);
      grabber->SetFrameSizeConverter (height, width, FALSE);
    }
    
  grabber->Start ();
     
  channel->AttachVideoReader (grabber);
  channel->AttachVideoPlayer (encoding_device);
}


int GMH323Webcam::Initialised (void)
{
  return initialised;
}


void GMH323Webcam::Start (void)
{
  MyApp->Endpoint ()->DisplayConfig (0);
}


void GMH323Webcam::Stop (int s = 1)
{
  running = 0;
  sensitivity_change = s;
}


void GMH323Webcam::StopGrabbing (void)
{
  grabbing = 0;
}


GDKVideoOutputDevice *GMH323Webcam::Device (void)
{
  return encoding_device;
}


PVideoChannel *GMH323Webcam::Channel (void)
{
  return channel;
}


void GMH323Webcam::SetColour (int colour)
{
  grabber->SetColour (colour);
}


void GMH323Webcam::SetBrightness (int brightness)
{
  grabber->SetBrightness (brightness);
}


void GMH323Webcam::SetWhiteness (int whiteness)
{
  grabber->SetWhiteness (whiteness);
}


void GMH323Webcam::SetContrast (int constrast)
{
  grabber->SetContrast (constrast);
}



void GMH323Webcam::GetParameters (int *whiteness, int *brightness, 
				  int *colour, int *contrast)
{
  *whiteness = (int) grabber->GetWhiteness () / 256;
  *brightness = (int) grabber->GetBrightness () / 256;
  *colour = (int) grabber->GetColour () / 256;
  *contrast = (int) grabber->GetContrast () / 256;
}

/******************************************************************************/
