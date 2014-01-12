
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2009 Damien Sandras <dsandras@seconix.com>
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
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Ekiga is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OPAL, OpenH323 and PWLIB, and distribute the combination,
 * without applying the requirements of the GNU GPL to the OPAL, OpenH323
 * and PWLIB programs, as long as you do follow the requirements of the
 * GNU GPL for all the rest of the software thus combined.
 */


/*
 *                         gst-audiooutput.cpp  -  description
 *                         ------------------------------------
 *   begin                : Sat 27 September 2008
 *   copyright            : (C) 2008 by Julien Puydt
 *   description          : Gstreamer audio output code
 *
 */

#include <string.h>

#include <glib/gi18n.h>

#include <gst/interfaces/propertyprobe.h>

#include "runtime.h"

#include "gst-audiooutput.h"

GST::AudioOutputManager::AudioOutputManager ():
  already_detected_devices(false)
{
  worker[0]=NULL;
  worker[1]=NULL;
}

GST::AudioOutputManager::~AudioOutputManager ()
{
}

void
GST::AudioOutputManager::get_devices (std::vector<Ekiga::AudioOutputDevice>& devices)
{
  detect_devices ();

  for (std::map<std::pair<std::string, std::string>, std::string>::const_iterator iter
	 = devices_by_name.begin ();
       iter != devices_by_name.end ();
       ++iter) {

    Ekiga::AudioOutputDevice device;
    device.type = "GStreamer";
    device.source = iter->first.first;
    device.name = iter->first.second;
    devices.push_back (device);
  }
}

bool
GST::AudioOutputManager::set_device (Ekiga::AudioOutputPS ps,
				     const Ekiga::AudioOutputDevice& device)
{
  bool result = false;

  if ( !already_detected_devices)
    detect_devices ();

  if (device.type == "GStreamer"
      && devices_by_name.find (std::pair<std::string,std::string>(device.source, device.name)) != devices_by_name.end ()) {

    unsigned ii = (ps == Ekiga::primary)?0:1;
    current_state[ii].opened = false;
    current_state[ii].device = device;
    result = true;
  }
  return result;
}

bool
GST::AudioOutputManager::open (Ekiga::AudioOutputPS ps,
			       unsigned channels,
			       unsigned samplerate,
			       unsigned bits_per_sample)
{
  unsigned ii = (ps == Ekiga::primary)?0:1;
  gchar* command = NULL;

  if ( !already_detected_devices)
    detect_devices ();

  command = g_strdup_printf ("appsrc"
			     " is-live=true format=time do-timestamp=true"
			     " min-latency=1 max-latency=5000000"
			     " name=ekiga_src"
			     " caps=audio/x-raw-int"
			     ",rate=%d"
			     ",channels=%d"
			     ",width=%d"
			     ",depth=%d"
			     ",signed=true,endianness=1234"
			     " ! %s",
			     samplerate, channels, bits_per_sample, bits_per_sample,
			     devices_by_name[std::pair<std::string,std::string>(current_state[ii].device.source, current_state[ii].device.name)].c_str ());
  worker[ii] = gst_helper_new (command);
  g_free (command);

  Ekiga::AudioOutputSettings settings;
  gfloat vol = gst_helper_get_volume (worker[ii]);
  if (vol >= 0) {

    settings.volume = (unsigned)(255*vol);
    settings.modifyable = true;
  } else {

    settings.modifyable = false;
  }
  current_state[ii].channels = channels;
  current_state[ii].samplerate = samplerate;
  current_state[ii].bits_per_sample = bits_per_sample;
  current_state[ii].opened = true;
  Ekiga::Runtime::run_in_main (boost::bind (boost::ref(device_opened), ps, current_state[ii].device, settings));

  return true;
}

void
GST::AudioOutputManager::close (Ekiga::AudioOutputPS ps)
{
  unsigned ii = (ps == Ekiga::primary)?0:1;

  if (worker[ii])
    gst_helper_close (worker[ii]);
  Ekiga::Runtime::run_in_main (boost::bind (boost::ref(device_closed), ps, current_state[ii].device));
  current_state[ii].opened = false;
  worker[ii] = NULL;
}

void
GST::AudioOutputManager::set_buffer_size (Ekiga::AudioOutputPS ps,
					  unsigned buffer_size,
					  unsigned /*num_buffers*/)
{
  unsigned ii = (ps == Ekiga::primary)?0:1;
  gst_helper_set_buffer_size (worker[ii], buffer_size);
}

bool
GST::AudioOutputManager::set_frame_data (Ekiga::AudioOutputPS ps,
					 const char* data,
					 unsigned size,
					 unsigned& written)
{
  bool result;
  unsigned ii = (ps == Ekiga::primary)?0:1;

  if (worker[ii]) {

    gst_helper_set_frame_data (worker[ii], data, size);
    written = size;
    result = true;
  } else {

    // we're closed already!
    written = 0;
    result = false;
  }
  return result;
}

void
GST::AudioOutputManager::set_volume (Ekiga::AudioOutputPS ps,
				     unsigned valu)
{
  unsigned ii = (ps == Ekiga::primary)?0:1;

  gst_helper_set_volume (worker[ii], valu / 255.0);
}

bool
GST::AudioOutputManager::has_device (const std::string& source,
				     const std::string& device_name,
				     Ekiga::AudioOutputDevice& /*device*/)
{
  return (devices_by_name.find (std::pair<std::string,std::string>(source, device_name)) != devices_by_name.end ());
}

void
GST::AudioOutputManager::detect_devices ()
{
  devices_by_name.clear ();
  detect_fakesink_devices ();
  detect_alsasink_devices ();
  detect_pulsesink_devices ();
  detect_sdlsink_devices ();
  devices_by_name[std::pair<std::string,std::string>("FILE","event")] = "volume name=ekiga_volume ! filesink location=/tmp/event";
  devices_by_name[std::pair<std::string,std::string>("FILE","in_a_call")] = "volume name=ekiga_volume ! filesink location=/tmp/in_a_call";
}

void
GST::AudioOutputManager::detect_fakesink_devices ()
{
  GstElement* elt = NULL;

  elt = gst_element_factory_make ("fakesink", "fakesinkpresencetest");

  if (elt != NULL) {

    devices_by_name[std::pair<std::string,std::string>(_("Silent"), _("Silent"))] = "fakesink";
    gst_object_unref (GST_OBJECT (elt));
  }
}

void
GST::AudioOutputManager::detect_alsasink_devices ()
{
  GstElement* elt = NULL;

  elt = gst_element_factory_make ("alsasink", "alsasinkpresencetest");

  if (elt != NULL) {

    GstPropertyProbe* probe = NULL;
    const GParamSpec* pspec = NULL;
    GValueArray* array = NULL;

    gst_element_set_state (elt, GST_STATE_PAUSED);
    probe = GST_PROPERTY_PROBE (elt);
    pspec = gst_property_probe_get_property (probe, "device");

    array = gst_property_probe_probe_and_get_values (probe, pspec);
    if (array != NULL) {

      for (guint index = 0; index < array->n_values; index++) {

	GValue* device = NULL;
	gchar* name = NULL;
	gchar* descr = NULL;

	device = g_value_array_get_nth (array, index);
	g_object_set_property (G_OBJECT (elt), "device", device);
	g_object_get (G_OBJECT (elt), "device-name", &name, NULL);
	descr = g_strdup_printf ("volume name=ekiga_volume ! alsasink device=%s",
				 g_value_get_string (device));

	if (name != 0) {

	  devices_by_name[std::pair<std::string,std::string>("ALSA", name)] = descr;
	  g_free (name);
	}
	g_free (descr);
      }
      g_value_array_free (array);
    }

    devices_by_name[std::pair<std::string,std::string>("ALSA","---")] = "volume name=ekiga_volume ! alsasink";

    gst_element_set_state (elt, GST_STATE_NULL);
    gst_object_unref (GST_OBJECT (elt));
  }
}

void
GST::AudioOutputManager::detect_pulsesink_devices ()
{
  GstElement* elt = NULL;

  elt = gst_element_factory_make ("pulsesink", "pulsesinkpresencetest");

  if (elt != NULL) {

    devices_by_name[std::pair<std::string,std::string>("PULSEAUDIO", "Default")] = "pulsesink name=ekiga_volume";

    gst_object_unref (GST_OBJECT (elt));
  }
}

void
GST::AudioOutputManager::detect_sdlsink_devices ()
{
  gchar* descr = NULL;
  descr = g_strdup_printf ("volume name=ekiga_volume ! sdlaudiosink");
  devices_by_name[std::pair<std::string,std::string>("SDL", "Default")] = descr;
  g_free (descr);
}
