
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2003 Damien Sandras
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
 * GnomeMeting is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OpenH323 and Pwlib, and distribute the combination, without
 * applying the requirements of the GNU GPL to the OpenH323 program, as long
 * as you do follow the requirements of the GNU GPL for all the rest of the
 * software thus combined.
 */


/*
 *                         videograbber.cpp  -  description
 *                         --------------------------------
 *   begin                : Mon Feb 12 2001
 *   copyright            : (C) 2000-2003 by Damien Sandras
 *   description          : Video4Linux compliant functions to manipulate the 
 *                          webcam device.
 *
 */


#include "../config.h"

#include "videograbber.h"
#include "gnomemeeting.h"
#include "misc.h"

#include "dialog.h"
#include "gconf_widgets_extensions.h"


/* Declarations */
extern GtkWidget *gm;


/* The functions */
GMVideoGrabber::GMVideoGrabber (BOOL start_grabbing,
				BOOL sync)
  :PThread (1000, NoAutoDeleteThread)
{
  height = 0, width = 0;
  whiteness = 0, brightness = 0, colour = 0, contrast = 0;

  gw = GnomeMeeting::Process ()->GetMainWindow ();
  pw = GnomeMeeting::Process ()->GetPrefWindow ();
  dw = GnomeMeeting::Process ()->GetDruidWindow ();

  /* Internal state */
  stop = FALSE;
  is_grabbing = start_grabbing;
  synchronous = sync;
  is_opened = FALSE;

  /* Initialisation */
  encoding_device = NULL;
  channel = NULL;
  grabber = NULL;
  encoding_device = NULL;
  video_channel = 0;
  video_size = 0;
  video_format = PVideoDevice::Auto;
  client = gconf_client_get_default ();

  UpdateConfig ();

  if (synchronous)
    VGOpen ();

  /* Start the thread */
  this->Resume ();
  thread_sync_point.Wait ();
}


GMVideoGrabber::~GMVideoGrabber ()
{
  stop = TRUE;
  is_grabbing = 0;
  device_mutex.Wait ();

  PWaitAndSignal m(quit_mutex);
}


void GMVideoGrabber::Main ()
{
  PWaitAndSignal m(quit_mutex);
  
  thread_sync_point.Signal ();
 
  if (!synchronous)
    VGOpen ();

  while (!stop) {

    if (is_grabbing == 1) {

      channel->Read (video_buffer, height * width * 3);
      channel->Write (video_buffer, height * width * 3);    
    }

    Current()->Sleep (50);
  }

  VGClose ();
}


void GMVideoGrabber::UpdateConfig ()
{
  gnomemeeting_threads_enter ();
  video_recorder = gconf_get_string (VIDEO_DEVICES_KEY "input_device");
  video_driver = gconf_get_string (VIDEO_DEVICES_KEY "plugin");
  video_channel = gconf_get_int (VIDEO_DEVICES_KEY "channel");
  video_size = gconf_get_int (VIDEO_DEVICES_KEY "size");

  switch (gconf_get_int (VIDEO_DEVICES_KEY "format")) {
    
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


BOOL GMVideoGrabber::IsGrabbing (void)
{
  BOOL isg = FALSE;

  var_mutex.Wait ();
  isg = is_grabbing;
  var_mutex.Signal ();

  return isg;
}


GDKVideoOutputDevice *GMVideoGrabber::GetEncodingDevice (void)
{
  return encoding_device;
}


PVideoChannel *GMVideoGrabber::GetVideoChannel (void)
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
  int hue;
  grabber->GetParameters (whiteness, brightness, colour, contrast, &hue);

  *whiteness = (int) *whiteness / 256;
  *brightness = (int) *brightness / 256;
  *colour = (int) *colour / 256;
  *contrast = (int) *contrast / 256;
}


void GMVideoGrabber::Lock ()
{
  device_mutex.Wait ();
}


void GMVideoGrabber::Unlock ()
{
  device_mutex.Signal ();
}


void GMVideoGrabber::VGOpen (void)
{
  gchar *title_msg = NULL;
  gchar *msg = NULL;
  gchar *video_image = NULL;
  int error_code = -1;  // No error

  if (!is_opened) {

    /* Disable the video preview button while opening */
    gnomemeeting_threads_enter ();
    gnomemeeting_log_insert (gw->history_text_view, _("Opening Video device"));
    gnomemeeting_threads_leave ();

    if (video_size == 0) { 
      
      height = GM_QCIF_HEIGHT; 
      width = GM_QCIF_WIDTH; 
    }
    else { 
      
      height = GM_CIF_HEIGHT; 
      width = GM_CIF_WIDTH; 
    }
    

    grabber = 
      PVideoInputDevice::CreateOpenedDevice (video_driver,
					     video_recorder,
					     FALSE);

    if (grabber == NULL)
      error_code = 0;
    else 
      if (!grabber->SetVideoFormat(video_format))
	error_code = 2;
      else
        if (!grabber->SetChannel(video_channel))
          error_code = 2;
	else
	  if (!grabber->SetColourFormatConverter ("YUV420P"))
	    error_code = 3;
	  else
	    if (!grabber->SetFrameRate (30))
	      error_code = 4;
	    else
	      if (!grabber->SetFrameSizeConverter (width, height, FALSE))
		error_code = 5;


    /* If no error */
    if (error_code == -1) {

      gnomemeeting_threads_enter ();
      msg = g_strdup_printf 
	(_("Successfully opened video device %s, channel %d"), 
	 (const char *) video_recorder, video_channel);
      gnomemeeting_log_insert (gw->history_text_view, msg);
      g_free (msg);
      gnomemeeting_threads_leave ();
    }
    else {

      
      /* If we want to open the fake device for a real error, and not because
	 the user chose the Picture device */
      gnomemeeting_threads_enter ();
      title_msg =
	g_strdup_printf (_("Error while opening video device %s"), (const char *) video_recorder);

      /* Translators: Do not translate MovingLogo and StaticPicture */
      msg = g_strdup (_("A moving GnomeMeeting logo will be transmitted during calls. Notice that you can always transmit a given image or the moving GnomeMeeting logo by choosing \"Picture\" as video plugin and \"MovingLogo\" or \"StaticPicture\" as device."));
      gnomemeeting_statusbar_flash (gw->statusbar, _("Can't open the Video Device"));
	
      switch (error_code) {
	  
      case 0:
	msg = g_strconcat (msg, "\n\n", _("There was an error while opening the device. Please check your permissions and make sure that the appropriate driver is loaded."), 
			   NULL);
	break;
	  
      case 1:
	msg = g_strconcat (msg, "\n\n", _("Your video driver doesn't support the requested video format."), NULL);
	break;

      case 2:
	msg = g_strconcat (msg, "\n\n", _("Could not open the chosen channel with the chosen video format."), NULL);
	break;
      
      case 3:
	msg = g_strconcat (msg, "\n\n", g_strdup_printf(_("Your driver doesn't seem to support any of the colour formats supported by GnomeMeeting.\n Please check your kernel driver documentation in order to determine which Palette is supported.")), NULL);
	break;
      
      case 4:
	msg = g_strconcat (msg, "\n\n", _("Error while setting the frame rate."), NULL);
	break;

      case 5:
	msg = g_strconcat (msg, "\n\n", _("Error while setting the frame size."), NULL);
	break;
      }

      gnomemeeting_warning_dialog_on_widget (GTK_WINDOW (gm), gw->preview_button, title_msg, msg);
      g_free (msg);
      g_free (title_msg);

      gnomemeeting_threads_leave ();
   
         
      /* delete the failed grabber and open the fake grabber, either
	 because there was an error, either because the user chose to do so */
      if (grabber)
	delete grabber;


      gnomemeeting_threads_enter ();
      video_image = gconf_get_string (VIDEO_DEVICES_KEY "image");
      gnomemeeting_threads_leave ();
    
      grabber =
	PVideoInputDevice::CreateOpenedDevice ("Picture",
					       "MovingLogo",
					       FALSE);
      if (grabber) {
	
	grabber->SetColourFormatConverter ("YUV420P");
	grabber->SetVideoFormat (PVideoDevice::PAL);
	grabber->SetChannel (1);    
	grabber->SetFrameRate (6);
	grabber->SetFrameSizeConverter (width, height, FALSE);
      }
      
      g_free (video_image);
    }
      
    grabber->Start ();

    var_mutex.Wait ();
    channel = new PVideoChannel ();
    encoding_device = new GDKVideoOutputDevice (1, gw);
    encoding_device->SetColourFormatConverter ("YUV420P");
    var_mutex.Signal ();
    
    channel->AttachVideoReader (grabber);
    channel->AttachVideoPlayer (encoding_device);

    var_mutex.Wait ();
    is_opened = TRUE;
    var_mutex.Signal ();
  
    encoding_device->SetFrameSize (width, height);  

    /* Setup the video settings */
    GetParameters (&whiteness, &brightness, &colour, &contrast);
    gnomemeeting_threads_enter ();
    GTK_ADJUSTMENT (gw->adj_brightness)->value = brightness;
    GTK_ADJUSTMENT (gw->adj_whiteness)->value = whiteness;
    GTK_ADJUSTMENT (gw->adj_colour)->value = colour;
    GTK_ADJUSTMENT (gw->adj_contrast)->value = contrast;
    gtk_widget_queue_draw (GTK_WIDGET (gw->video_settings_frame));

    /* Enable the video settings frame */
    gtk_widget_set_sensitive (GTK_WIDGET (gw->video_settings_frame),
			      TRUE);
    gnomemeeting_threads_leave ();

    /* Enable the video preview button if not in a call */
    if (GnomeMeeting::Process ()->Endpoint ()->GetCallingState () == GMH323EndPoint::Standby) {

      gnomemeeting_threads_enter ();      
      gnomemeeting_menu_update_sensitivity (TRUE, FALSE, TRUE);
      gnomemeeting_threads_leave ();
    }
  }
}
  

void GMVideoGrabber::VGClose ()
{
  if (is_opened) {

    var_mutex.Wait ();
    is_grabbing = 0;
    var_mutex.Signal ();


    if (channel)
      delete (channel);

    
    /* Enable video preview button */
    gnomemeeting_threads_enter ();
  

    if (GnomeMeeting::Process ()->Endpoint ()->GetCallingState () == GMH323EndPoint::Standby && !gconf_get_bool (VIDEO_DEVICES_KEY "enable_preview")) {
      
      gnomemeeting_menu_update_sensitivity (TRUE, FALSE, FALSE);
      gnomemeeting_init_main_window_logo (gw->main_video_image);
    }
    
    /* Disable the video settings frame */
    gtk_widget_set_sensitive (GTK_WIDGET (gw->video_settings_frame),
			      FALSE);
  
    gnomemeeting_threads_leave ();
    
    
    /* Initialisation */
    grabber = NULL;
    var_mutex.Wait ();
    channel = NULL;
    is_opened = FALSE;
    encoding_device = NULL;
    var_mutex.Signal ();
  }

  /* Quick Hack for buggy drivers that return from the ioctl before the device
     is really closed */
  PThread::Current ()->Sleep (1000);
}


/* The video tester class */
GMVideoTester::GMVideoTester (gchar *m,
			      gchar *r)
  :PThread (1000, AutoDeleteThread)
{
#ifndef DISABLE_GNOME
  if (m)
    video_manager = PString (m);
  if (r)
    video_recorder = PString (r);

  test_dialog = NULL;
  test_label = NULL;
  
  this->Resume ();
  thread_sync_point.Wait ();
#endif
}


GMVideoTester::~GMVideoTester ()
{
#ifndef DISABLE_GNOME
  PWaitAndSignal m(quit_mutex);
#endif
}


void GMVideoTester::Main ()
{
#ifndef DISABLE_GNOME
  GmWindow *gw = NULL;
  GmDruidWindow *dw = NULL;

  PVideoInputDevice *grabber = NULL;
  
  int height = GM_QCIF_HEIGHT; 
  int width = GM_QCIF_WIDTH; 
  int error_code = -1;
  int cpt = 0;

  gchar *msg = NULL;
  gchar *tmp = NULL;

  gw = GnomeMeeting::Process ()->GetMainWindow ();
  dw = GnomeMeeting::Process ()->GetDruidWindow ();

  PWaitAndSignal m(quit_mutex);
  thread_sync_point.Signal ();

  if (video_recorder.IsEmpty ()
      || video_manager.IsEmpty ()
      || video_recorder == PString (_("No device found"))
      || video_recorder == PString (_("Picture")))
    return;
  
  gdk_threads_enter ();
  test_dialog =
    gtk_dialog_new_with_buttons ("Video test running",
				 GTK_WINDOW (gw->druid_window),
				 (enum GtkDialogFlags) (GTK_DIALOG_MODAL),
				 GTK_STOCK_OK,
				 GTK_RESPONSE_ACCEPT,
				 NULL);
  msg = 
    g_strdup_printf (_("GnomeMeeting is now testing the %s video device. If you experience machine crashes, then report a bug to the video driver author."), (const char *) video_recorder);
  test_label = gtk_label_new (msg);
  gtk_label_set_line_wrap (GTK_LABEL (test_label), true);
  g_free (msg);

  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (test_dialog)->vbox), test_label,
		      FALSE, FALSE, 2);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (test_dialog)->vbox), 
		      gtk_hseparator_new (), FALSE, FALSE, 2);

  test_label = gtk_label_new (NULL);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (test_dialog)->vbox), 
		      test_label, FALSE, FALSE, 2);

  g_signal_connect (G_OBJECT (test_dialog), "delete-event",
		    G_CALLBACK (gtk_widget_hide_on_delete), NULL);

  gtk_window_set_transient_for (GTK_WINDOW (test_dialog),
				GTK_WINDOW (gw->druid_window));
  gtk_widget_show_all (test_dialog);
  gdk_threads_leave ();

  
  while (cpt < 6 && error_code == -1) {

    if (!video_recorder.IsEmpty ()
	&& !video_manager.IsEmpty ()) {

      error_code = -1;
      
      grabber = 
	PVideoInputDevice::CreateOpenedDevice (video_manager,
					       video_recorder,
					       FALSE);

      if (!grabber)
	error_code = 0;
      else
	if (!grabber->SetVideoFormat (PVideoDevice::Auto))
	  error_code = 2;
      else
        if (!grabber->SetChannel (0))
          error_code = 2;
      else
	if (!grabber->SetColourFormatConverter ("YUV420P"))
	  error_code = 3;
      else
	if (!grabber->SetFrameRate (30))
	  error_code = 4;
      else
	if (!grabber->SetFrameSizeConverter (width, height, FALSE))
	  error_code = 5;
      else
	grabber->Close ();

      if (grabber)
	delete (grabber);
    }


    if (error_code == -1) 
      msg = g_strdup_printf (_("Test %d done"), cpt);
    else
      msg = g_strdup_printf (_("Test %d failed"), cpt);

    tmp = g_strdup_printf ("<b>%s</b>", msg);
    gdk_threads_enter ();
    gtk_label_set_markup (GTK_LABEL (test_label), tmp);
    gdk_threads_leave ();
    g_free (msg);
    g_free (tmp);

    cpt++;
    PThread::Current () ->Sleep (100);
  }

  
  if (error_code != - 1) {
    
    switch (error_code)	{
	  
    case 0:
      msg = g_strdup_printf (_("Error while opening %s."),
			     (const char *) video_recorder);
      break;
      
    case 1:
      msg = g_strdup_printf (_("Your video driver doesn't support the requested video format."));
      break;
      
    case 2:
      msg = g_strdup_printf (_("Could not open the chosen channel with the chosen video format."));
      break;
      
    case 3:
      msg = g_strdup_printf (_("Your driver doesn't support any of the color formats tried by GnomeMeeting"));
      break;
      
    case 4:
      msg = g_strdup_printf ( _("Error with the frame rate."));
      break;
      
    case 5:
      msg = g_strdup_printf (_("Error with the frame size."));
      break;
    }

    gdk_threads_enter ();
    gnomemeeting_error_dialog (GTK_WINDOW (gw->druid_window),
			       _("Failed to open the device"),
			       msg);
    gdk_threads_leave ();
    
    g_free (msg);
  }

  gdk_threads_enter ();
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dw->video_test_button),
				FALSE);
  if (test_dialog)
    gtk_widget_destroy (test_dialog);
  gdk_threads_leave ();
#endif
}


