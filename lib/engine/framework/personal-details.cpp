
/*
 * Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2008 Damien Sandras

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
 *                         personal-details.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Damien Sandras
 *   copyright            : (c) 2008 by Damien Sandras
 *   description          : declaration of the representation of personal details
 *
 */


#include "personal-details.h"

using namespace Ekiga;


const std::string & PersonalDetails::get_display_name () 
{ 
  return display_name; 
}


const std::string & PersonalDetails::get_short_status () 
{ 
  return short_status; 
}


const std::string & PersonalDetails::get_long_status () 
{ 
  return long_status; 
}


void PersonalDetails::set_display_name (const std::string & _display_name)
{
  display_name = _display_name;
  personal_details_updated.emit (*this);
}


void PersonalDetails::set_short_status (const std::string & _short_status)
{
  short_status = _short_status;
  personal_details_updated.emit (*this);
}


void PersonalDetails::set_long_status (const std::string & _long_status)
{
  long_status = _long_status;
  personal_details_updated.emit (*this);
}


