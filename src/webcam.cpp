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


GMVideoGrabber::GMVideoGrabber (GM_window_widgets *g, options *o)
  :PThread (1000, NoAutoDeleteThread)
{
  gw = g;
  opts = o;

  running = 1;
  has_to_open = 0;
  has_to_close = 0;
  grabbing = 0;
  opened = 0;

  this->Resume ();
}


GMVideoGrabber::~GMVideoGrabber ()
{ 
  running = 0;
  grabbing = 0;

  quit_mutex.Wait ();

  gdk_threads_enter ();

  // Disable the video settings frame
  gtk_widget_set_sensitive (GTK_WIDGET (gw->video_settings_frame),
			    FALSE);

  // Display the logo
  GM_init_main_interface_logo (gw);

  gdk_threads_leave ();
}


void GMVideoGrabber::Main ()
{
  quit_mutex.Wait ();

  while (running == 1)
    {

      if (has_to_open == 1)
	VGOpen ();

      if (grabbing == 1)
	{
	  grabbing_mutex.Wait ();

	  channel->Read (video_buffer, height * width * 3);
	  channel->Write (video_buffer, height * width * 3);

	  grabbing_mutex.Signal ();
	}

      if (has_to_close == 1)
	VGClose ();

      Current()->Sleep (10);
    }

  if (sensitivity_change)
    {
      // Disable the video preview button
      gdk_threads_enter ();
      gtk_widget_set_sensitive (GTK_WIDGET (gw->preview_button), FALSE);
      gdk_threads_leave ();
    }

  if (opened == 1)
    {
      grabber->Close ();
      channel->Close ();
      Current ()->Sleep (500);

      delete (channel);
    }

  quit_mutex.Signal ();
}


void GMVideoGrabber::VGOpen (void)
{
  gchar *msg = NULL;

  // Disable the video preview button while opening
  gdk_threads_enter ();
  gtk_widget_set_sensitive (GTK_WIDGET (gw->preview_button), FALSE);
  gdk_threads_leave ();

  channel = new PVideoChannel ();
  encoding_device = new GDKVideoOutputDevice (1, gw);
  grabber = new PVideoInputDevice();

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

  has_to_open = 0;
  opened = 1;

  gdk_threads_enter ();

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

  if (MyApp->Endpoint ()->CallingState () == 0)
    gtk_widget_set_sensitive (GTK_WIDGET (gw->preview_button), TRUE);

  gdk_threads_leave ();
}


void GMVideoGrabber::Close (void)
{
  has_to_close = 1;
}


void GMVideoGrabber::VGClose (void)
{
  grabbing = 0;

  // Disable the video preview button while closing
  gdk_threads_enter ();
  gtk_widget_set_sensitive (GTK_WIDGET (gw->preview_button), FALSE);
  gdk_threads_leave ();

  grabbing_mutex.Wait ();

  grabber->Close ();
  channel->Close ();

  grabbing_mutex.Signal ();

  delete (channel);

  has_to_close = 0;
  opened = 0;

  gdk_threads_enter ();
  gtk_widget_set_sensitive (GTK_WIDGET (gw->preview_button), TRUE);

  // Display the logo again
  GM_init_main_interface_logo (gw);

  // Disable the video settings frame
  gtk_widget_set_sensitive (GTK_WIDGET (gw->video_settings_frame),
			    FALSE);
  gdk_threads_leave ();
}


void GMVideoGrabber::Open (int has_to_grab)
{
  has_to_open = 1;
  grabbing = has_to_grab;
}


int GMVideoGrabber::IsOpened (void)
{
  return opened;
}


void GMVideoGrabber::Start (void)
{
  MyApp->Endpoint ()->DisplayConfig (0);
  grabbing = 1;
}


void GMVideoGrabber::Stop (int s = 1)
{
  running = 0;
  sensitivity_change = s;
}


void GMVideoGrabber::StopGrabbing (void)
{
  grabbing = 0;

  grabbing_mutex.Wait ();
  grabbing_mutex.Signal ();
}


GDKVideoOutputDevice *GMVideoGrabber::Device (void)
{
  return encoding_device;
}


PVideoChannel *GMVideoGrabber::Channel (void)
{
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
  *whiteness = (int) grabber->GetWhiteness () / 256;
  *brightness = (int) grabber->GetBrightness () / 256;
  *colour = (int) grabber->GetColour () / 256;
  *contrast = (int) grabber->GetContrast () / 256;
}


int GM_cam (gchar *video_device, int video_channel)
{

  PVideoInputDevice *grabber;

  grabber = new PVideoInputDevice ();

  if (!grabber->Open (video_device, FALSE) || !grabber->SetChannel(video_channel))
    {
      delete (grabber);
      return 0;
    }
  else
    {
      delete (grabber);
      return 1;
    }
}

/******************************************************************************/
