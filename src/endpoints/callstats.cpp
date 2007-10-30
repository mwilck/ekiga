
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
 *                         callstats.cpp  -  description
 *                         ----------------------------
 *   begin                : Tue Oct 30 2007
 *   copyright            : (C) 2000-2007 by Damien Sandras
 *   description          : This file contains a class containing information
 *                          about a call.
 *
 */

#include "config.h"

#include "callstats.h"


Ekiga::CallStatistics::CallStatistics ()
{
  Reset ();
}


void Ekiga::CallStatistics::Reset ()
{ 
  re_a_bytes = 0; 
  re_v_bytes = 0;
  tr_a_bytes = 0;
  tr_v_bytes = 0;

  v_re_frames = 0;
  v_tr_frames = 0;
  v_tr_fps = 0;
  v_re_fps = 0;

  jitter_buffer_size = 0;

  a_re_bandwidth = 0;
  v_re_bandwidth = 0;
  a_tr_bandwidth = 0;
  v_tr_bandwidth = 0;
  lost_packets = 0;
  out_of_order_packets = 0;
  late_packets = 0;
  total_packets = 0; 
  lost_packets_per = 0;
  late_packets_per = 0;
  out_of_order_packets_per = 0;
}

