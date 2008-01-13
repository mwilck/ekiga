
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


using namespace Ekiga;


void DisplayCore::add_manager (DisplayManager &manager)
{
  managers.insert (&manager);
  manager_added.emit (manager);

  manager.display_type_changed.connect (sigc::bind (sigc::mem_fun (this, &DisplayCore::on_display_type_changed), &manager));
  manager.fullscreen_mode_changed.connect (sigc::bind (sigc::mem_fun (this, &DisplayCore::on_fullscreen_mode_changed), &manager));
  manager.size_changed.connect (sigc::bind (sigc::mem_fun (this, &DisplayCore::on_size_changed), &manager));
  manager.logo_update_required.connect (sigc::bind (sigc::mem_fun (this, &DisplayCore::on_logo_update_required), &manager));
  manager.display_info_update_required.connect (sigc::bind (sigc::mem_fun (this, &DisplayCore::on_display_info_update_required), &manager));
}


void DisplayCore::visit_managers (sigc::slot<void, DisplayManager &> visitor)
{
  for (std::set<DisplayManager *>::iterator iter = managers.begin ();
       iter != managers.end ();
       iter++)
    visitor (*(*iter));
}


void DisplayCore::start ()
{
  for (std::set<DisplayManager *>::iterator iter = managers.begin ();
       iter != managers.end ();
       iter++) {
    (*iter)->start ();
  }
}

void DisplayCore::stop ()
{
  for (std::set<DisplayManager *>::iterator iter = managers.begin ();
       iter != managers.end ();
       iter++) {
    (*iter)->stop ();
  }
}

void DisplayCore::set_frame_data (unsigned width,
                                  unsigned height,
                                  const char *data,
                                  bool local,
                                  int devices_nbr)
{
  for (std::set<DisplayManager *>::iterator iter = managers.begin ();
       iter != managers.end ();
       iter++) {
    (*iter)->set_frame_data (width, height, data, local, devices_nbr);
  }
}

void DisplayCore::set_video_info (const DisplayInfo & newVideoInfo)
{
  for (std::set<DisplayManager *>::iterator iter = managers.begin ();
       iter != managers.end ();
       iter++) {
    (*iter)->set_display_info (newVideoInfo);
  }
}


void DisplayCore::on_display_type_changed (DisplayMode display, DisplayManager *manager)
{
  display_type_changed.emit (*manager, display);
}

void DisplayCore::on_fullscreen_mode_changed ( FSToggle_new toggle, DisplayManager *manager)
{
  fullscreen_mode_changed.emit (*manager, toggle);
}

void DisplayCore::on_size_changed ( unsigned width, unsigned height, DisplayManager *manager)
{
  size_changed.emit (*manager, width, height);
}

void DisplayCore::on_logo_update_required (DisplayManager *manager)
{
  logo_update_required.emit (*manager);
}

void DisplayCore::on_display_info_update_required (DisplayManager *manager)
{
  display_info_update_required.emit (*manager);
}
