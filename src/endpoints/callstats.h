
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2007 Damien Sandras
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
 *                         callstats.h  -  description
 *                         ---------------------------
 *   begin                : Tue Oct 30 2007
 *   copyright            : (C) 2000-2007 by Damien Sandras
 *   description          : This file contains a class containing information
 *                          about a call.
 *
 */

#ifndef __CALLSTATS_H_
#define __CALLSTATS_H_

#include <string>

#include "common.h"

namespace Ekiga {

  class CallStatistics
    {

  public:

      CallStatistics ();
      void Reset (); 
      
      int re_a_bytes; 
      int re_v_bytes;
      int tr_a_bytes;
      int tr_v_bytes;

      int v_re_frames;
      int v_tr_frames;

      float a_re_bandwidth;
      float v_re_bandwidth;
      float a_tr_bandwidth;
      float v_tr_bandwidth;

      int v_tr_fps;
      int v_re_fps;
      unsigned int lost_packets_per;
      unsigned int late_packets_per;
      unsigned int out_of_order_packets_per;
      unsigned int tr_width;
      unsigned int tr_height;
      unsigned int re_width;
      unsigned int re_height;

      float lost_packets;
      float out_of_order_packets;
      float late_packets;
      float total_packets;

      int jitter_buffer_size;

      PTime last_tick;
      PTime start_time;
    };
};

#endif

