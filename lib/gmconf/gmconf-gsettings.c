
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2013 Damien Sandras <dsandras@seconix.com>
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
 *                         gmconf-gsettings.c  -  description
 *                         ------------------------------------------
 *   begin                : Mar 2013
 *   copyright            : (c) 2013 by Damien sandras,
 *   description          : gsettings implementation of Ekiga's
 *                          configuration system
 *
 */


#include "config.h"

#include <gio/gio.h>

#include <gconf/gconf-client.h>
#include <string.h>

#include <gmconf/gmconf.h>

/* Using a global variable is dirty, but the api is built like this
 */
static GConfClient *client;
static GSettings* settings;

/* this is needed in order to really hide gconf: one needs to be able to
 * call the GmConfNotifier from inside a gconf notifier, so we hide the real
 * notifier and its associated user data into the user data of a gconf
 * notifier, that will do the unwrapping, and call the real stuff */
typedef struct _GConfNotifierWrap GConfNotifierWrap;

struct _GConfNotifierWrap {
  GmConfNotifier real_notifier;
  gpointer real_user_data;
};

static GConfNotifierWrap *gconf_notifier_wrapper_new (GmConfNotifier,
                                                      gpointer);

/* gpointer, because it is a callback */
static void gconf_notifier_wrapper_destroy (gpointer);

/* this is the universal gconf notifier, that interprets its fourth
 * argument as a GConfNotifierWrap*, and fires it */
static void gconf_notifier_wrapper_trigger (GConfClient *,
                                            guint,
                                            GConfEntry *,
                                            gpointer);

/* GSettins convenience functions */
static void g_settings_get_child_and_key (const GSettings* root,
                                          const gchar* key,
                                          GSettings** nchild,
                                          gchar** nkey);

/* this functions expects a non-NULL conf notifier, and wraps it for
 * use by the universal gconf notifier */
static GConfNotifierWrap *
gconf_notifier_wrapper_new (GmConfNotifier notifier,
                            gpointer user_data)
{
  GConfNotifierWrap *result = NULL;

  g_return_val_if_fail (notifier != NULL, NULL);

  result = g_new (GConfNotifierWrap, 1);
  result->real_notifier = notifier;
  result->real_user_data = user_data;

  return result;
}

/* this function is automatically called to free the notifiers' wrappers */
static void
gconf_notifier_wrapper_destroy (gpointer wrapper)
{
  g_free ((GConfNotifierWrap *) wrapper);
}


/* this is the universal gconf notification unwrapper: it
 * expects a wrapped gm conf notifier in its user_data argument,
 * and calls it  */
static void
gconf_notifier_wrapper_trigger (G_GNUC_UNUSED GConfClient *client_,
				guint identifier,
				GConfEntry *entry,
				gpointer user_data)
{
  GConfNotifierWrap *wrapper = NULL;

  g_return_if_fail (user_data != NULL);

  wrapper = (GConfNotifierWrap *)user_data;
  wrapper->real_notifier (GUINT_TO_POINTER (identifier),
			  (GmConfEntry *)entry,
			  wrapper->real_user_data);
}


static void
g_settings_get_child_and_key (const GSettings* root,
                              const gchar* key,
                              GSettings** nchild,
                              gchar** nkey)
{
  int i = 0;
  gchar** split_key = NULL;
  gchar** children = NULL;
  gchar* new_key = NULL;
  GSettings* child = (GSettings *) root;
  GSettings* new_child = child;

  g_return_if_fail (key != NULL);

  split_key = g_strsplit (key, "_", -1);
  new_key = g_strjoinv ("-", split_key);
  g_strfreev (split_key);

  children = g_strsplit (new_key, "/", -1);
  g_free (new_key);

  while (children[i]) {

    if (!g_strcmp0 (children[i], "apps")
        || !g_strcmp0 (children[i], PACKAGE_NAME)
        || !g_strcmp0 (children[i], "")) {
      i++;
      continue;
    }

    if (children[i+1] != NULL) {
      new_child = g_settings_get_child (child, children[i]);
      if (child != root)
        g_object_unref (child);
      child = new_child;
      *nchild = child;
    }
    else {

      *nkey = g_strdup (children[i]);
      break;
    }
    i++;
  }

  g_strfreev (children);
}


void
gm_conf_set_bool (const gchar *key,
		  const gboolean b)
{
  GSettings *nchild = NULL;
  gchar *nkey = NULL;
  g_return_if_fail (key != NULL);

  g_settings_get_child_and_key (settings, key, &nchild, &nkey);
  g_settings_set_boolean (nchild, nkey, b);

  g_clear_object (&nchild);
  g_free (nkey);
}


gboolean
gm_conf_get_bool (const gchar *key)
{
  GSettings *nchild = NULL;
  gchar *nkey = NULL;
  gboolean val;
  g_return_val_if_fail (key != NULL, FALSE);

  g_settings_get_child_and_key (settings, key, &nchild, &nkey);
  val = g_settings_get_boolean (nchild, nkey);

  g_clear_object (&nchild);
  g_free (nkey);

  return val;
}


void
gm_conf_set_string (const gchar *key,
		    const gchar *v)
{
  GSettings *nchild = NULL;
  gchar *nkey = NULL;
  g_return_if_fail (key != NULL);

  g_settings_get_child_and_key (settings, key, &nchild, &nkey);
  g_settings_set_string (nchild, nkey, v);

  g_clear_object (&nchild);
  g_free (nkey);
}


gchar *
gm_conf_get_string (const gchar *key)
{
  GSettings *nchild = NULL;
  gchar *nkey = NULL;
  gchar *val;
  g_return_val_if_fail (key != NULL, FALSE);

  g_settings_get_child_and_key (settings, key, &nchild, &nkey);
  val = g_settings_get_string (nchild, nkey);

  g_clear_object (&nchild);
  g_free (nkey);

  return val;
}


void
gm_conf_set_int (const gchar *key,
		 const int v)
{
  GSettings *nchild = NULL;
  gchar *nkey = NULL;
  g_return_if_fail (key != NULL);

  g_settings_get_child_and_key (settings, key, &nchild, &nkey);
  g_settings_set_int (nchild, nkey, v);

  g_clear_object (&nchild);
  g_free (nkey);
}


int
gm_conf_get_int (const gchar *key)
{
  GSettings *nchild = NULL;
  gchar *nkey = NULL;
  int val;
  g_return_val_if_fail (key != NULL, FALSE);

  g_settings_get_child_and_key (settings, key, &nchild, &nkey);
  val = g_settings_get_int (nchild, nkey);

  g_clear_object (&nchild);
  g_free (nkey);

  return val;
}


void
gm_conf_set_string_list (const gchar *key,
			 GSList *l)
{
  GSettings *nchild = NULL;
  gchar *nkey = NULL;
  GSList *iter = NULL;
  gchar **v = NULL;
  int i = 0;
  g_return_if_fail (key != NULL);

  v = (gchar**) g_malloc (sizeof(gchar*) * (g_slist_length (l) + 1));
  iter = l;
  while (iter) {
    v[i] = g_strdup ((char *) iter->data);
    iter = g_slist_next (iter);
    i++;
  }
  v[i] = NULL;
  g_settings_get_child_and_key (settings, key, &nchild, &nkey);
  g_settings_set_strv (nchild, nkey, (const gchar *const*) v);

  g_clear_object (&nchild);
  g_free (nkey);
  g_strfreev (v);
}


GSList *
gm_conf_get_string_list (const gchar *key)
{
  GSettings *nchild = NULL;
  gchar *nkey = NULL;
  GSList *list = NULL;
  gchar **v = NULL;
  int i = 0;
  g_return_val_if_fail (key != NULL, NULL);

  g_settings_get_child_and_key (settings, key, &nchild, &nkey);
  v = g_settings_get_strv (nchild, nkey);
  while (v[i]) {
    list = g_slist_append (list, (gpointer) g_strdup (v[i]));
    i++;
  }

  g_clear_object (&nchild);
  g_free (nkey);
  g_strfreev (v);

  return list;
}


gchar *
gm_conf_escape_key (const gchar *key,
                    gint len)
{
  return gconf_escape_key (key, len);
}


gchar *
gm_conf_unescape_key (const gchar *key,
                      gint len)
{
  return gconf_unescape_key (key, len);
}


gboolean
gm_conf_is_key_writable (const gchar *key)
{
  g_return_val_if_fail (key != NULL, FALSE);

  return gconf_client_key_is_writable (client, key, NULL);
}


void
gm_conf_init ()
{
  client = gconf_client_get_default ();
  gconf_client_set_error_handling (client, GCONF_CLIENT_HANDLE_UNRETURNED);
  gconf_client_add_dir (client, "/apps/" PACKAGE_NAME,
			GCONF_CLIENT_PRELOAD_NONE, NULL);

  settings = g_settings_new ("org.gnome." PACKAGE_NAME);
}


void
gm_conf_shutdown ()
{
  gconf_client_remove_dir (client, "/apps/" PACKAGE_NAME, NULL);
  g_object_unref (client);

  g_clear_object (&settings);
}


void
gm_conf_save ()
{
  /* nothing needed */
}


GmConfEntryType
gm_conf_entry_get_type (GmConfEntry *entry)
{
  GConfEntry *gconf_entry = NULL;

  g_return_val_if_fail (entry != NULL, GM_CONF_OTHER);

  gconf_entry = (GConfEntry *)entry;
  if (gconf_entry->value == NULL)
    return GM_CONF_OTHER;

  switch (gconf_entry->value->type)
    {
    case GCONF_VALUE_BOOL:
      return GM_CONF_BOOL;
    case GCONF_VALUE_INT:
      return GM_CONF_INT;
    case GCONF_VALUE_STRING:
      return GM_CONF_STRING;
    case GCONF_VALUE_LIST:
      return GM_CONF_LIST;
    case GCONF_VALUE_INVALID:
    case GCONF_VALUE_FLOAT:
    case GCONF_VALUE_SCHEMA:
    case GCONF_VALUE_PAIR:
    default:
      return GM_CONF_OTHER;
    }
}


const gchar *
gm_conf_entry_get_key (GmConfEntry *entry)
{
  GConfEntry *gconf_entry = NULL;

  g_return_val_if_fail (entry != NULL, NULL);

  gconf_entry = (GConfEntry *)entry;
  return gconf_entry_get_key (gconf_entry);
}


gboolean
gm_conf_entry_get_bool (GmConfEntry *entry)
{
  GConfEntry *gconf_entry = NULL;

  g_return_val_if_fail (entry != NULL, FALSE);

  gconf_entry = (GConfEntry *)entry;
  if (gconf_entry->value)
    return gconf_value_get_bool (gconf_entry->value);

  return FALSE;
}


gint
gm_conf_entry_get_int (GmConfEntry *entry)
{
  GConfEntry *gconf_entry = NULL;

  g_return_val_if_fail (entry != NULL, 0);

  gconf_entry = (GConfEntry *)entry;
  if (gconf_entry->value)
    return gconf_value_get_int (gconf_entry->value);

  return 0;
}


gchar *
gm_conf_entry_get_string (GmConfEntry *entry)
{
  GConfEntry *gconf_entry = NULL;

  g_return_val_if_fail (entry != NULL, NULL);

  gconf_entry = (GConfEntry *)entry;
  if (gconf_entry->value)
    return g_strdup (gconf_value_get_string (gconf_entry->value));

  return NULL;
}


GSList *
gm_conf_entry_get_list (GmConfEntry *entry)
{
  GConfEntry *gconf_entry = NULL;
  GSList *list = NULL;
  GSList *it = NULL;

  g_return_val_if_fail (entry != NULL, NULL);

  gconf_entry = (GConfEntry *)entry;

  if (gconf_entry->value)
    it = gconf_value_get_list (gconf_entry->value);
  while (it) {

    list = g_slist_append (list, g_strdup ((char *) gconf_value_get_string (it->data)));
    it = g_slist_next (it);
  }

  return list;
}


gpointer
gm_conf_notifier_add (const gchar *namespac,
		      GmConfNotifier func,
		      gpointer user_data)
{
  gpointer result;
  GConfNotifierWrap *wrapper = NULL;

  g_return_val_if_fail (namespac != NULL, NULL);
  g_return_val_if_fail (func != NULL, NULL);

  wrapper = gconf_notifier_wrapper_new (func, user_data);
  return GUINT_TO_POINTER(gconf_client_notify_add (client, namespac,
						     gconf_notifier_wrapper_trigger,
						     wrapper,
						     gconf_notifier_wrapper_destroy, NULL));

  return result;
}


void
gm_conf_notifier_remove (gpointer identifier)
{
  g_return_if_fail (identifier != NULL);

  gconf_client_notify_remove (client, GPOINTER_TO_UINT (identifier));
}

void
gm_conf_notifier_trigger (const gchar *namespac)
{
  g_return_if_fail (namespac != NULL);

  gconf_client_notify (client, namespac);
}
