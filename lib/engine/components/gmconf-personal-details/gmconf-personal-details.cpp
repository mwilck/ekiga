
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

#include <iostream>

using namespace Gmconf;

static void
display_name_changed_nt (gpointer /*id*/,
                         GmConfEntry *entry,
                         gpointer data)
{
  PersonalDetails *details = (PersonalDetails *) data;
  const gchar *name = NULL;

  if (gm_conf_entry_get_type (entry) == GM_CONF_STRING) {

    name = gm_conf_entry_get_string (entry);
    if (name != NULL)
      details->set_display_name (name);
  }
}

static void
short_status_changed_nt (gpointer /*id*/,
                         GmConfEntry *entry,
                         gpointer data)
{
  PersonalDetails *details = (PersonalDetails *) data;
  const gchar *status = NULL;

  if (gm_conf_entry_get_type (entry) == GM_CONF_STRING) {

    status = gm_conf_entry_get_string (entry);
    if (status != NULL)
      details->set_short_status (status);
  }
}

static void
long_status_changed_nt (gpointer /*id*/,
                        GmConfEntry *entry,
                        gpointer data)
{
  PersonalDetails *details = (PersonalDetails *) data;
  const gchar *status = NULL;

  if (gm_conf_entry_get_type (entry) == GM_CONF_STRING) {

    status = gm_conf_entry_get_string (entry);
    if (status != NULL)
      details->set_long_status (status);
  }
}

PersonalDetails::PersonalDetails ()
{
  gm_conf_notifier_add ("/apps/ekiga/general/personal_data/full_name",
                        display_name_changed_nt, this);
  gm_conf_notifier_add ("/apps/ekiga/general/personal_data/short_status",
                        short_status_changed_nt, this);
  gm_conf_notifier_add ("/apps/ekiga/general/personal_data/long_status",
                        long_status_changed_nt, this);
  gm_conf_notifier_trigger ("/apps/ekiga/general/personal_data/full_name");
  gm_conf_notifier_trigger ("/apps/ekiga/general/personal_data/short_status");
  gm_conf_notifier_trigger ("/apps/ekiga/general/personal_data/long_status");
}
