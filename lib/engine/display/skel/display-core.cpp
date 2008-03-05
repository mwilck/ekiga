
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
 *                         display-core.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Matthias Schneider
 *   copyright            : (c) 2007 by Matthias Schneider
 *   description          : declaration of the interface of a display core.
 *                          A display core manages DisplayManagers.
 *
 */

#include <iostream>
#include <sstream>

#include "config.h"

#include "display-core.h"
#include "display-manager.h"

#include <math.h>

using namespace Ekiga;

DisplayCore::DisplayCore ()
{
  PWaitAndSignal m(var_mutex);

  display_stats.rx_width = display_stats.rx_height = display_stats.rx_fps = 0;
  display_stats.tx_width = display_stats.tx_height = display_stats.tx_fps = 0;
  display_stats.rx_frames = 0;
  display_stats.tx_frames = 0;
  number_times_started = 0;
  display_core_conf_bridge = NULL;
}

DisplayCore::~DisplayCore ()
{
  PWaitAndSignal m(var_mutex);

  if (display_core_conf_bridge)
    delete display_core_conf_bridge;
}

void DisplayCore::setup_conf_bridge ()
{
  PWaitAndSignal m(var_mutex);

  display_core_conf_bridge = new DisplayCoreConfBridge (*this);
}

void DisplayCore::add_manager (DisplayManager &manager)
{
  PWaitAndSignal m(var_mutex);

  managers.insert (&manager);
  manager_added.emit (manager);

  manager.display_mode_changed.connect (sigc::bind (sigc::mem_fun (this, &DisplayCore::on_display_mode_changed), &manager));
  manager.fullscreen_mode_changed.connect (sigc::bind (sigc::mem_fun (this, &DisplayCore::on_fullscreen_mode_changed), &manager));
  manager.display_size_changed.connect (sigc::bind (sigc::mem_fun (this, &DisplayCore::on_display_size_changed), &manager));
  manager.hw_accel_status_changed.connect (sigc::bind (sigc::mem_fun (this, &DisplayCore::on_hw_accel_status_changed), &manager));
  manager.logo_update_required.connect (sigc::bind (sigc::mem_fun (this, &DisplayCore::on_logo_update_required), &manager));
}


void DisplayCore::visit_managers (sigc::slot<bool, DisplayManager &> visitor)
{
  bool go_on = true;

  for (std::set<DisplayManager *>::iterator iter = managers.begin ();
       iter != managers.end () && go_on;
       iter++)
    go_on = visitor (*(*iter));
}


void DisplayCore::start ()
{
   PWaitAndSignal m(var_mutex);

   number_times_started++;
   if (number_times_started > 1)
     return;

  g_get_current_time (&last_stats);

  for (std::set<DisplayManager *>::iterator iter = managers.begin ();
       iter != managers.end ();
       iter++) {
    (*iter)->start ();
  }
}

void DisplayCore::stop ()
{
  PWaitAndSignal m(var_mutex);

  number_times_started--;

  if (number_times_started < 0) {
    number_times_started = 0;
    return;
  }

  if (number_times_started != 0)
    return;
    
  for (std::set<DisplayManager *>::iterator iter = managers.begin ();
       iter != managers.end ();
       iter++) {
    (*iter)->stop ();
  }
  display_stats.rx_width = display_stats.rx_height = display_stats.rx_fps = 0;
  display_stats.tx_width = display_stats.tx_height = display_stats.tx_fps = 0;
  display_stats.rx_frames = 0;
  display_stats.tx_frames = 0;
}

void DisplayCore::set_frame_data (unsigned width,
                                  unsigned height,
                                  const char *data,
                                  bool local,
                                  int devices_nbr)
{
  var_mutex.Wait ();

  if (local) {
    display_stats.tx_frames++;
    display_stats.tx_width = width;
    display_stats.tx_height = height;
  }
  else {
    display_stats.rx_frames++;
    display_stats.rx_width = width;
    display_stats.rx_height = height;
  }

  GTimeVal current_time;
  g_get_current_time (&current_time);

  long unsigned milliseconds = ((current_time.tv_sec - last_stats.tv_sec) * 1000) 
                             + ((current_time.tv_usec - last_stats.tv_usec) / 1000);

  if (milliseconds > 2000) {
    display_stats.tx_fps = round ((display_stats.rx_frames * 1000) / milliseconds);
    display_stats.rx_fps = round ((display_stats.tx_frames * 1000) / milliseconds);
    display_stats.rx_frames = 0;
    display_stats.tx_frames = 0;
    g_get_current_time (&last_stats);
  }

  var_mutex.Signal ();
  
  for (std::set<DisplayManager *>::iterator iter = managers.begin ();
       iter != managers.end ();
       iter++) {
    (*iter)->set_frame_data (width, height, data, local, devices_nbr);
  }
}

void DisplayCore::set_display_info (const DisplayInfo & _display_info)
{
  PWaitAndSignal m(var_mutex);

  for (std::set<DisplayManager *>::iterator iter = managers.begin ();
       iter != managers.end ();
       iter++) {
    (*iter)->set_display_info (_display_info);
  }
}


void DisplayCore::on_display_mode_changed (DisplayMode display, DisplayManager *manager)
{
  display_mode_changed.emit (*manager, display);
}

void DisplayCore::on_fullscreen_mode_changed ( FSToggle toggle, DisplayManager *manager)
{
  fullscreen_mode_changed.emit (*manager, toggle);
}

void DisplayCore::on_display_size_changed ( unsigned width, unsigned height, DisplayManager *manager)
{
  display_size_changed.emit (*manager, width, height);
}

void DisplayCore::on_hw_accel_status_changed (HwAccelStatus hw_accel_status, DisplayManager *manager)
{
  hw_accel_status_changed.emit (*manager, hw_accel_status);
}

void DisplayCore::on_logo_update_required (DisplayManager *manager)
{
  logo_update_required.emit (*manager);
}
