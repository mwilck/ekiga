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
  :PThread (1000, NoAutoDeleteThread)
{
  gw = g;
  opts = o;

  channel = new PVideoChannel ();
  encoding_device = new GDKVideoOutputDevice (1, gw);
  grabber = new PVideoInputDevice();

  running = 0;
  grabbing = 0;

  this->Resume ();
}


GMH323Webcam::~GMH323Webcam ()
{
  Stop ();
  running = 0;

  delete (channel);
}


void GMH323Webcam::Main ()
{
  GtkWidget *msg_box = NULL;
  int height, width;
  int error = 0;

  if (opts->video_size == 0)
    { height = 176; width = 144; }
  else
    { height = 352; width = 288; }
    
  encoding_device->SetFrameSize (height, width);

  if (grabber->Open (opts->video_device, FALSE) &&
      grabber->SetVideoFormat 
      (opts->video_format ? PVideoDevice::NTSC : PVideoDevice::PAL) &&
      grabber->SetChannel (opts->video_channel) &&
      grabber->SetFrameRate (opts->tr_fps)  &&
      grabber->SetFrameSize (height, width)) 
    {
      // Quick hack for Philips Webcams (which should not be needed)
      if (!grabber->SetColourFormatConverter ("YUV411P"))
	error = !grabber->SetColourFormatConverter ("YUV420P");
    }
  else
    error = 1;
  
  if (error == 1)
    {
      gdk_threads_enter ();
      msg_box = 
	gnome_message_box_new (_("Impossible to open the video device."), 
			       GNOME_MESSAGE_BOX_ERROR, "OK", NULL);
      
      gtk_widget_show (msg_box);
      gdk_threads_leave ();

      error = 1;

      // delete the failed grabber and open the fake grabber
      delete grabber;
      
      grabber = new PFakeVideoInputDevice();
      grabber->SetColourFormatConverter ("YUV411P");
      grabber->SetVideoFormat (PVideoDevice::PAL);
      grabber->SetChannel (100);     //NTSC static image.
      grabber->SetFrameRate (10);
      grabber->SetFrameSize (height, width);
    }
    
  grabber->Start ();
     
  channel->AttachVideoReader (grabber);
  channel->AttachVideoPlayer (encoding_device);
    
  // Ready to run
  running = 1;

  while (running == 1)
    {
      if (grabbing == 1)
	{
	  if (channel->Read (video_buffer, height * width * 3))
	    channel->Write (video_buffer, height * width * 3);
	  Current()->Sleep (50);
	}
      else
	  Current()->Sleep (10);
    }
}


void GMH323Webcam::Start (void)
{
  MyApp->Endpoint ()->DisplayConfig (0);
  grabbing = 1;
}


void GMH323Webcam::Stop (void)
{
  grabbing = 0;
}


int GMH323Webcam::Running (void)
{
  return running;
}

void GMH323Webcam::Terminate (void)
{
  PThread::Current()->Terminate ();
}


PVideoChannel *GMH323Webcam::Channel (void)
{
  return channel;
}


GDKVideoOutputDevice *GMH323Webcam::Device (void)
{
  return encoding_device;
}


void GMH323Webcam::Restart (void)
{
  grabbing = 0;
  grabber->Close ();
  Current ()->Restart ();
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
		

