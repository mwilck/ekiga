
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
 *                         gmconf-personal-details.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Damien Sandras
 *   copyright            : (c) 2008 by Damien Sandras
 *   description          : declaration of the representation of personal details
 *                          using gmconf
 *
 */

#include <glib.h>

#include "gmconf.h"
#include "gmconf-personal-details.h"

static void
display_name_changed_nt (G_GNUC_UNUSED gpointer id,
			 GmConfEntry* entry,
			 gpointer data)
{
  Gmconf::PersonalDetails *details = (Gmconf::PersonalDetails *) data;
  const gchar* val = gm_conf_entry_get_string (entry);

  if (val != NULL)
    details->display_name_changed (val);
  else
    details->display_name_changed ("");
}

static void
short_status_changed_nt (G_GNUC_UNUSED gpointer id,
			 GmConfEntry* entry,
			 gpointer data)
{
  Gmconf::PersonalDetails *details = (Gmconf::PersonalDetails *) data;
  const gchar* val = gm_conf_entry_get_string (entry);

  if (val != NULL)
    details->short_status_changed (val);
  else
    details->short_status_changed ("");
}

static void
long_status_changed_nt (G_GNUC_UNUSED gpointer id,
			GmConfEntry* entry,
			gpointer data)
{
  Gmconf::PersonalDetails *details = (Gmconf::PersonalDetails *) data;
  const gchar* val = gm_conf_entry_get_string (entry);

  if (val != NULL)
    details->long_status_changed (val);
  else
    details->long_status_changed ("");
}

Gmconf::PersonalDetails::PersonalDetails ()
{
  gchar* str = NULL;

  display_name_notifier
    = gm_conf_notifier_add ("/apps/ekiga/general/personal_data/full_name",
			    display_name_changed_nt, this);
  short_status_notifier
    = gm_conf_notifier_add ("/apps/ekiga/general/personal_data/short_status",
			    short_status_changed_nt, this);
  long_status_notifier
  = gm_conf_notifier_add ("/apps/ekiga/general/personal_data/long_status",
			  long_status_changed_nt, this);

  str = gm_conf_get_string ("/apps/ekiga/general/personal_data/full_name");
  if (str != NULL)
    display_name = str;
  else
    display_name = "";

  str = gm_conf_get_string ("/apps/ekiga/general/personal_data/short_status");
  if (str != NULL)
    short_status = str;
  else
    short_status = "";

  str = gm_conf_get_string ("/apps/ekiga/general/personal_data/long_status");
  if (str != NULL)
    long_status = str;
  else
    long_status = "";
}

Gmconf::PersonalDetails::~PersonalDetails ()
{
  gm_conf_notifier_remove (display_name_notifier);
  gm_conf_notifier_remove (short_status_notifier);
  gm_conf_notifier_remove (long_status_notifier);
}

const std::string
Gmconf::PersonalDetails::get_display_name () const
{
  return display_name;
}

const std::string
Gmconf::PersonalDetails::get_short_status () const
{
  return short_status;
}

const std::string
Gmconf::PersonalDetails::get_long_status () const
{
  return long_status;
}

void
Gmconf::PersonalDetails::set_display_name (const std::string display_name_)
{
  gm_conf_set_string ("/apps/ekiga/general/personal_data/full_name",
		      display_name_.c_str ());
}

void
Gmconf::PersonalDetails::set_short_status (const std::string short_status_)
{
  gm_conf_set_string ("/apps/ekiga/general/personal_data/short_status",
		      short_status_.c_str ());
}

void
Gmconf::PersonalDetails::set_long_status (const std::string long_status_)
{
  gm_conf_set_string ("/apps/ekiga/general/personal_data/long_status",
		      long_status_.c_str ());
}

void
Gmconf::PersonalDetails::display_name_changed (std::string val)
{
  display_name = val;
  updated.emit ();
}

void
Gmconf::PersonalDetails::short_status_changed (std::string val)
{
  short_status = val;
  updated.emit ();
}

void
Gmconf::PersonalDetails::long_status_changed (std::string val)
{
  long_status = val;
  updated.emit ();
}
