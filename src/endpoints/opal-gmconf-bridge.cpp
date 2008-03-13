
/*
 * Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2007 Damien Sandras

 * This program is free software; you can  redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version. This program is distributed in the hope
 * that it will be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Ekiga is licensed under the GPL license and as a special exception, you
 * have permission to link or otherwise combine this program with the
 * programs OPAL, OpenH323 and PWLIB, and distribute the combination, without
 * applying the requirements of the GNU GPL to the OPAL, OpenH323 and PWLIB
 * programs, as long as you do follow the requirements of the GNU GPL for all
 * the rest of the software thus combined.
 */


/*
 *                         opal-gmconf-bridge.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Damien Sandras
 *   copyright            : (c) 2008 by Damien Sandras
 *   description          : declaration of an object able to do the bridging
 *                          between gmconf and opal
 *
 */

#include <iostream>
#include <sigc++/sigc++.h>

#include "gmconf.h"
#include "opal-gmconf-bridge.h"

#include "manager.h"
#include "sip.h"

using namespace Opal;


ConfBridge::ConfBridge (Ekiga::Service & _service)
 : Ekiga::ConfBridge (_service)
{
  Ekiga::ConfKeys keys;
  property_changed.connect (sigc::mem_fun (this, &ConfBridge::on_property_changed));

  keys.push_back (AUDIO_CODECS_KEY "enable_silence_detection");
  keys.push_back (AUDIO_CODECS_KEY "enable_echo_cancelation");

  keys.push_back (AUDIO_CODECS_KEY "list");
  keys.push_back (VIDEO_CODECS_KEY "list");

  keys.push_back (AUDIO_CODECS_KEY "minimum_jitter_buffer");
  keys.push_back (AUDIO_CODECS_KEY "maximum_jitter_buffer");

  keys.push_back (VIDEO_CODECS_KEY "maximum_video_tx_bitrate");
  keys.push_back (VIDEO_CODECS_KEY "temporal_spatial_tradeoff");
  keys.push_back (VIDEO_DEVICES_KEY "size"); 
  keys.push_back (VIDEO_CODECS_KEY "max_frame_rate");
  keys.push_back (VIDEO_CODECS_KEY "maximum_video_rx_bitrate");

  keys.push_back (SIP_KEY "outbound_proxy_host");
  keys.push_back (SIP_KEY "dtmf_mode");
  keys.push_back (NAT_KEY "binding_timeout");

  keys.push_back (PERSONAL_DATA_KEY "full_name");

  load (keys);
}


void ConfBridge::on_property_changed (std::string key, GmConfEntry *entry)
{
  GMManager & manager = (GMManager &) service;

  //
  // Video options
  //
  if (key == VIDEO_CODECS_KEY "maximum_video_tx_bitrate") {

    GMManager::VideoOptions options;
    manager.get_video_options (options);
    options.maximum_transmitted_bitrate = gm_conf_entry_get_int (entry);
    manager.set_video_options (options);
  }
  else if (key == VIDEO_CODECS_KEY "temporal_spatial_tradeoff") {

    GMManager::VideoOptions options;
    manager.get_video_options (options);
    options.temporal_spatial_tradeoff = gm_conf_entry_get_int (entry);
    manager.set_video_options (options);
  }
  else if (key == VIDEO_DEVICES_KEY "size") {

    GMManager::VideoOptions options;
    manager.get_video_options (options);
    options.size = gm_conf_entry_get_int (entry);
    manager.set_video_options (options);
  }
  else if (key == VIDEO_CODECS_KEY "max_frame_rate") {

    GMManager::VideoOptions options;
    manager.get_video_options (options);
    options.maximum_frame_rate = gm_conf_entry_get_int (entry);
    manager.set_video_options (options);
  }
  else if (key == VIDEO_CODECS_KEY "maximum_video_rx_bitrate") {

    GMManager::VideoOptions options;
    manager.get_video_options (options);
    options.maximum_received_bitrate = gm_conf_entry_get_int (entry);
    manager.set_video_options (options);
  }


  // 
  // Jitter buffer configuration
  //
  if (key == AUDIO_CODECS_KEY "minimum_jitter_buffer") {

    unsigned max = (unsigned) gm_conf_get_int (AUDIO_CODECS_KEY "maximum_jitter_buffer");
    manager.set_jitter_buffer_size (gm_conf_entry_get_int (entry), max);
  }
  else if (key == AUDIO_CODECS_KEY "maximum_jitter_buffer") {

    unsigned min = (unsigned) gm_conf_get_int (AUDIO_CODECS_KEY "minimum_jitter_buffer");
    manager.set_jitter_buffer_size (min, gm_conf_entry_get_int (entry));
  }


  // 
  // Silence detection
  //
  else if (key == AUDIO_CODECS_KEY "enable_silence_detection") {

    manager.set_silence_detection (gm_conf_entry_get_bool (entry));
  }


  //
  // Echo cancelation
  //
  else if (key == AUDIO_CODECS_KEY "enable_echo_cancelation") {

    manager.set_echo_cancelation (gm_conf_entry_get_bool (entry));
  }
  
  
  // 
  // Audio & video codecs
  //
  else if (key == AUDIO_CODECS_KEY "list"
           || key == VIDEO_CODECS_KEY "list") {

    // This is a bit longer, we are not sure the list stored in the 
    // configuration is complete, and it could also contain unsupported codecs
    GSList *audio_codecs = NULL;
    GSList *video_codecs = NULL;

    if (key == AUDIO_CODECS_KEY "list") {

      audio_codecs = gm_conf_entry_get_list (entry);
      video_codecs = gm_conf_get_string_list (VIDEO_CODECS_KEY "list");
    }
    else {

      video_codecs = gm_conf_entry_get_list (entry);
      audio_codecs = gm_conf_get_string_list (AUDIO_CODECS_KEY "list");
    }

    Ekiga::CodecList codecs;
    Ekiga::CodecList a_codecs (audio_codecs);
    Ekiga::CodecList v_codecs (video_codecs);

    // Update the manager codecs
    codecs = a_codecs;
    codecs.append (v_codecs);
    manager.set_codecs (codecs);

    g_slist_foreach (audio_codecs, (GFunc) g_free, NULL);
    g_slist_free (audio_codecs);
    g_slist_foreach (video_codecs, (GFunc) g_free, NULL);
    g_slist_free (video_codecs);

    // Update the GmConf keys, in case we would have missed some codecs or
    // used codecs we do not really support
    if (a_codecs != codecs.get_audio_list ()) {

      audio_codecs = codecs.get_audio_list ().gslist ();
      gm_conf_set_string_list (AUDIO_CODECS_KEY "list", audio_codecs);
      g_slist_foreach (audio_codecs, (GFunc) g_free, NULL);
      g_slist_free (audio_codecs);
    }

    if (v_codecs != codecs.get_video_list ()) {

      video_codecs = codecs.get_video_list ().gslist ();
      gm_conf_set_string_list (VIDEO_CODECS_KEY "list", video_codecs);
      g_slist_foreach (video_codecs, (GFunc) g_free, NULL);
      g_slist_free (video_codecs);
    }
  }

  //
  // SIP related keys
  // 
  else if (key == SIP_KEY "outbound_proxy_host") {

    manager.GetSIPEndpoint ()->set_outbound_proxy (gm_conf_entry_get_string (entry));
  }
  else if (key == SIP_KEY "dtmf_mode") {

    manager.GetSIPEndpoint ()->set_dtmf_mode (gm_conf_entry_get_int (entry));
  }

  //
  // NAT related keys
  //
  else if (key == NAT_KEY "binding_timeout") {

    manager.GetSIPEndpoint ()->set_nat_binding_delay (gm_conf_entry_get_int (entry));
  }


  //
  // Personal Data Key
  //
  else if (key == PERSONAL_DATA_KEY "full_name") {

    manager.set_fullname (gm_conf_entry_get_string (entry));
  }
}

