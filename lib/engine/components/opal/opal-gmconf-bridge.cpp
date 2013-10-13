
/*
 * Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2009 Damien Sandras <dsandras@seconix.com>

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
#include <boost/signals2.hpp>
#include <boost/bind.hpp>

#include "config.h"

#include "gmconf.h"

#include "opal-gmconf-bridge.h"
#include "opal-call-manager.h"

#include "sip-endpoint.h"

using namespace Opal;

ConfBridge::ConfBridge (Opal::CallManager& _core): manager(_core)
{
  Ekiga::ConfKeys keys;
  property_changed.connect (boost::bind (&ConfBridge::on_property_changed, this, _1, _2));

  keys.push_back (PROTOCOLS_KEY "rtp_tos_field");

  keys.push_back (PORTS_KEY "udp_port_range");
  keys.push_back (PORTS_KEY "tcp_port_range");

  keys.push_back (AUDIO_CODECS_KEY "enable_silence_detection");
  keys.push_back (AUDIO_CODECS_KEY "enable_echo_cancellation");

  keys.push_back (AUDIO_CODECS_KEY "media_list");
  keys.push_back (VIDEO_CODECS_KEY "media_list");

  keys.push_back (AUDIO_CODECS_KEY "maximum_jitter_buffer");

  keys.push_back (VIDEO_CODECS_KEY "maximum_video_tx_bitrate");
  keys.push_back (VIDEO_CODECS_KEY "maximum_video_rx_bitrate");
  keys.push_back (VIDEO_CODECS_KEY "temporal_spatial_tradeoff");
  keys.push_back (VIDEO_DEVICES_KEY "size"); 
  keys.push_back (VIDEO_DEVICES_KEY "max_frame_rate");

  keys.push_back (PERSONAL_DATA_KEY "full_name");

  keys.push_back (CALL_FORWARDING_KEY "forward_on_no_answer");
  keys.push_back (CALL_FORWARDING_KEY "forward_on_busy");
  keys.push_back (CALL_FORWARDING_KEY "always_forward");
  keys.push_back (CALL_OPTIONS_KEY "no_answer_timeout");
  keys.push_back (CALL_OPTIONS_KEY "auto_answer");

  keys.push_back (NAT_KEY "stun_server");
  keys.push_back (NAT_KEY "enable_stun");

  load (keys);
}


void
ConfBridge::on_property_changed (std::string key,
				 GmConfEntry *entry)
{
  //
  // Personal Data Key
  //
  if (key == PERSONAL_DATA_KEY "full_name") {

    gchar* str = gm_conf_entry_get_string (entry);
    if (str != NULL)
      manager.set_display_name (str);
    g_free (str);
  }


  //
  // Misc keys
  //
  else if (key == CALL_FORWARDING_KEY "forward_on_no_answer") {

    manager.set_forward_on_no_answer (gm_conf_entry_get_bool (entry));
  }
  else if (key == CALL_FORWARDING_KEY "forward_on_busy") {

    manager.set_forward_on_busy (gm_conf_entry_get_bool (entry));
  }
  else if (key == CALL_FORWARDING_KEY "always_forward") {

    manager.set_unconditional_forward (gm_conf_entry_get_bool (entry));
  }
  else if (key == CALL_OPTIONS_KEY "no_answer_timeout") {

    manager.set_reject_delay (gm_conf_entry_get_int (entry));
  }
  else if (key == CALL_OPTIONS_KEY "auto_answer") {

    manager.set_auto_answer (gm_conf_entry_get_bool (entry));
  }
  else if (key == PROTOCOLS_KEY "rtp_tos_field") {

    manager.set_rtp_tos (gm_conf_entry_get_int (entry));
  }


  //
  // Ports keys
  //
  else if (key == PORTS_KEY "udp_port_range"
           || key == PORTS_KEY "tcp_port_range") {

    gchar* ports = NULL;
    gchar **couple = NULL;
    unsigned min_port = 0;
    unsigned max_port = 0;

    ports = gm_conf_entry_get_string (entry);
    if (ports)
      couple = g_strsplit (ports, ":", 2);
    g_free (ports);

    if (couple && couple [0])
      min_port = atoi (couple [0]);

    if (couple && couple [1])
      max_port = atoi (couple [1]);

    if (key == PORTS_KEY "udp_port_range")
      manager.set_udp_ports (min_port, max_port);
    else
      manager.set_tcp_ports (min_port, max_port);

    g_strfreev (couple);
  }

}
