
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
 *                         callinfo.cpp  -  description
 *                         ----------------------------
 *   begin                : Sun Sep 09 2007
 *   copyright            : (C) 2000-2007 by Damien Sandras
 *   description          : This file contains a class containing information
 *                          about a call.
 *
 */


#include "callinfo.h"

Ekiga::CallInfo::CallInfo ()
{
}


Ekiga::CallInfo::CallInfo (OpalConnection &connection)
{
  char special_chars [] = "([@";
  int i = 0;
  std::string::size_type idx;
  remote_party_name = (const char *) connection.GetRemotePartyName ();
  remote_application = (const char *) connection.GetRemoteApplication (); 

  while (i < 3) {

    idx = remote_party_name.find_first_of (special_chars [i]);
    if (idx != std::string::npos)
      remote_party_name = remote_party_name.substr (0, idx);

    idx = remote_application.find_first_of (special_chars [i]);
    if (idx != std::string::npos)
      remote_application = remote_application.substr (0, idx);
    
    i++;
  }

  remote_uri = (const char *) connection.GetRemotePartyCallbackURL ();
}


std::string
Ekiga::CallInfo::get_remote_party_name ()
{
  return remote_party_name;
}


std::string
Ekiga::CallInfo::get_remote_application ()
{
  return remote_application;
}


std::string
Ekiga::CallInfo::get_remote_uri ()
{
  return remote_uri;
}


int
Ekiga::CallInfo::get_call_duration ()
{
  return 3600;
}
