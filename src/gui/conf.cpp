
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2009 Damien Sandras <dsandras@seconix.com>
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
 *                         config.cpp  -  description
 *                         --------------------------
 *   begin                : Wed Feb 14 2001
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : This file contains most of config stuff.
 *                          All notifiers are here.
 *                          Callbacks that updates the config cache
 *                          are in their file, except some generic one that
 *                          are in this file.
 *   Additional code      : Miguel Rodríguez Pérez  <miguelrp@gmail.com>
 *
 */


#include "config.h"

#include "conf.h"

#include "gmconf.h"

/* The functions */
void
gnomemeeting_conf_upgrade ()
{
  int version = gm_conf_get_int (GENERAL_KEY "version");

  /* Install the sip:, h323: and callto: GNOME URL Handlers */
  gchar *conf_url = gm_conf_get_string ("/desktop/gnome/url-handlers/callto/command");

  if (!conf_url
      || !g_strcmp0 (conf_url, "gnomemeeting -c \"%s\"")) {

    gm_conf_set_string ("/desktop/gnome/url-handlers/callto/command",
			"ekiga -c \"%s\"");

    gm_conf_set_bool ("/desktop/gnome/url-handlers/callto/needs_terminal",
		      false);

    gm_conf_set_bool ("/desktop/gnome/url-handlers/callto/enabled", true);
  }
  g_free (conf_url);

  conf_url = gm_conf_get_string ("/desktop/gnome/url-handlers/h323/command");
  if (!conf_url
      || !g_strcmp0 (conf_url, "gnomemeeting -c \"%s\"")) {

    gm_conf_set_string ("/desktop/gnome/url-handlers/h323/command",
                        "ekiga -c \"%s\"");

    gm_conf_set_bool ("/desktop/gnome/url-handlers/h323/needs_terminal", false);

    gm_conf_set_bool ("/desktop/gnome/url-handlers/h323/enabled", true);
  }
  g_free (conf_url);

  conf_url = gm_conf_get_string ("/desktop/gnome/url-handlers/sip/command");
  if (!conf_url
      || !g_strcmp0 (conf_url, "gnomemeeting -c \"%s\"")) {

    gm_conf_set_string ("/desktop/gnome/url-handlers/sip/command",
                        "ekiga -c \"%s\"");

    gm_conf_set_bool ("/desktop/gnome/url-handlers/sip/needs_terminal", false);

    gm_conf_set_bool ("/desktop/gnome/url-handlers/sip/enabled", true);
  }
  g_free (conf_url);

  /* New full name key */
  conf_url = gm_conf_get_string (PERSONAL_DATA_KEY "full_name");
  if (!conf_url || (conf_url && !g_strcmp0 (conf_url, ""))) {

    gchar *fullname = NULL;
    gchar *firstname = gm_conf_get_string (PERSONAL_DATA_KEY "firstname");
    gchar *lastname = gm_conf_get_string (PERSONAL_DATA_KEY "lastname");

    if (firstname && lastname && g_strcmp0 (firstname, "") && g_strcmp0 (lastname, "")) {
      fullname = g_strdup_printf ("%s %s", firstname, lastname);
      gm_conf_set_string (PERSONAL_DATA_KEY "firstname", "");
      gm_conf_set_string (PERSONAL_DATA_KEY "lastname", "");
      gm_conf_set_string (PERSONAL_DATA_KEY "full_name", fullname);
      g_free (fullname);
    }
    g_free (firstname);
    g_free (lastname);
  }
  g_free (conf_url);

  /* diamondcard is now set at sip.diamondcard.us */
  GSList *accounts = gm_conf_get_string_list ("/apps/" PACKAGE_NAME "/protocols/accounts_list");
  GSList *accounts_iter = accounts;
  GRegex* regex = g_regex_new ("eugw\\.ast\\.diamondcard\\.us",
			       (GRegexCompileFlags)0,
			       (GRegexMatchFlags)0,
			       NULL);
  gchar* replaced_acct = NULL;
  while (accounts_iter) {

    replaced_acct = g_regex_replace (regex, (gchar *) accounts_iter->data,
				     -1, 0, "sip.diamondcard.us",
				     (GRegexMatchFlags)0, NULL);
    g_free (accounts_iter->data);
    accounts_iter->data = replaced_acct;
    accounts_iter = g_slist_next (accounts_iter);
  }
  g_regex_unref (regex);

  gm_conf_set_string_list ("/apps/" PACKAGE_NAME "/protocols/accounts_list", accounts);
  g_slist_foreach (accounts, (GFunc) g_free, NULL);
  g_slist_free (accounts);

  /* Audio devices */
  gchar *plugin = NULL;
  gchar *device = NULL;
  gchar *new_device = NULL;
  plugin = gm_conf_get_string (AUDIO_DEVICES_KEY "plugin");
  if (plugin && g_strcmp0 (plugin, "")) {
    device = gm_conf_get_string (AUDIO_DEVICES_KEY "input_device");
    new_device = g_strdup_printf ("%s (PTLIB/%s)", device, plugin);
    gm_conf_set_string (AUDIO_DEVICES_KEY "plugin", "");
    gm_conf_set_string (AUDIO_DEVICES_KEY "input_device", new_device);
    g_free (device);
    g_free (new_device);

    device = gm_conf_get_string (AUDIO_DEVICES_KEY "output_device");
    new_device = g_strdup_printf ("%s (PTLIB/%s)", device, plugin);
    gm_conf_set_string (AUDIO_DEVICES_KEY "plugin", "");
    gm_conf_set_string (AUDIO_DEVICES_KEY "output_device", new_device);
    g_free (device);
    g_free (new_device);

    device = gm_conf_get_string (SOUND_EVENTS_KEY "output_device");
    new_device = g_strdup_printf ("%s (PTLIB/%s)", device, plugin);
    gm_conf_set_string (SOUND_EVENTS_KEY "plugin", "");
    gm_conf_set_string (SOUND_EVENTS_KEY "output_device", new_device);
    g_free (device);
    g_free (new_device);
  }
  g_free (plugin);

  /* Video devices */
  plugin = gm_conf_get_string (VIDEO_DEVICES_KEY "plugin");
  if (plugin && g_strcmp0 (plugin, "")) {
    device = gm_conf_get_string (VIDEO_DEVICES_KEY "input_device");
    new_device = g_strdup_printf ("%s (PTLIB/%s)", device, plugin);
    gm_conf_set_string (VIDEO_DEVICES_KEY "plugin", "");
    gm_conf_set_string (VIDEO_DEVICES_KEY "input_device", new_device);
    g_free (device);
    g_free (new_device);
  }
  g_free (plugin);

  // a migration could be checked like this:
  // version >= first version where the old option appeared
  // && version <= last version where the new option still does not exist
  // this allows to read the old option only when it exists
  //   and also not to be used during the very first execution of ekiga

  // migrate from Disable to Enable network detection
  if (version >= 3020 && version < 3030)
    gm_conf_set_bool (NAT_KEY "enable_stun",
                      ! gm_conf_get_bool (NAT_KEY "disable_stun"));

  // migrate from cancelation to cancellation
  if (version > 0 && version <= 3031)
    gm_conf_set_bool (AUDIO_CODECS_KEY "enable_echo_cancellation",
                      gm_conf_get_bool (AUDIO_CODECS_KEY "enable_echo_cancelation"));

  // migrate short_status
  gchar *ss = gm_conf_get_string (PERSONAL_DATA_KEY "short_status");
  if (ss && !g_strcmp0 (ss, "online"))
    gm_conf_set_string (PERSONAL_DATA_KEY "short_status", "available");
  else if (ss && !g_strcmp0 (ss, "dnd"))
    gm_conf_set_string (PERSONAL_DATA_KEY "short_status", "busy");
  g_free (ss);

  // migrate custom statuses from online to available, and from dnd to busy
  GSList *old_values = NULL;
  old_values = gm_conf_get_string_list (PERSONAL_DATA_KEY "online_custom_status");
  if (old_values) {
    gm_conf_set_string_list (PERSONAL_DATA_KEY "available_custom_status", old_values);
    g_slist_foreach (old_values, (GFunc) g_free, NULL);
    g_slist_free (old_values);
    gm_conf_set_string_list (PERSONAL_DATA_KEY "online_custom_status", NULL);
  }

  old_values = gm_conf_get_string_list (PERSONAL_DATA_KEY "dnd_custom_status");
  if (old_values) {
    gm_conf_set_string_list (PERSONAL_DATA_KEY "busy_custom_status", old_values);
    g_slist_foreach (old_values, (GFunc) g_free, NULL);
    g_slist_free (old_values);
    gm_conf_set_string_list (PERSONAL_DATA_KEY "dnd_custom_status", NULL);
  }

  // migrate first port in UDP port ranges from 5060 to 5061, see bug #690621
  if (version >= 0 && version <= 4000) {
    gchar *ports = gm_conf_get_string (PORTS_KEY "udp_port_range");
    if (ports && !g_ascii_strncasecmp (ports, "5060", 4)) {
      ports[3] = '1';
      gm_conf_set_string (PORTS_KEY "udp_port_range", ports);
    }
    g_free (ports);
  }
}
