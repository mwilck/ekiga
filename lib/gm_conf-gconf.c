
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2004 Damien Sandras
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *
 * GnomeMeting is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OpenH323 and Pwlib, and distribute the combination, without
 * applying the requirements of the GNU GPL to the OpenH323 program, as long
 * as you do follow the requirements of the GNU GPL for all the rest of the
 * software thus combined.
 */


/*
 *                         gm_conf-gconf.c  -  description 
 *                         ------------------------------------------
 *   begin                : Mar 2004, derived from gconf_widgets_extensions.c
 *                          started on Fri Oct 17 2003.
 *   copyright            : (c) 2000-2004 by Damien sandras,
 *                          (c) 2004 by Julien Puydt
 *   description          : gconf implementation of gnomemeeting's
 *                          configuration system
 *
 */


#include "../config.h"

#include <gtk/gtk.h>
#include <gconf/gconf-client.h>

#include <string.h>

#ifndef DISABLE_GNOME
#include <gnome.h>
#endif

#include "gm_conf.h"


#ifndef _
#ifdef DISABLE_GNOME
#include <libintl.h>
#define _(x) gettext(x)
#ifdef gettext_noop
#define N_(String) gettext_noop (String)
#else
#define N_(String) (String)
#endif
#endif
#endif


/* this is needed in order to really hide gconf: one needs to be able to
 * call the GmConfNotifier from inside a gconf notifier, so we hide the real notifier
 * and its associated user data into the user data of a gconf notifier, that will do
 * the unwrapping, and call the real stuff */

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
 * argument as a GConfNotifierWrap*, and uses it */
static void gconf_notifier_wrapper_trigger (GConfClient *,
                                            guint,
                                            GConfEntry *,
                                            gpointer);


/* this function is called whenever an error occurs in gconf: it allows
 * to use NULL as error callback in every other call */
static void gconf_error_callback (GConfClient *,
                                  GError *);


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
gconf_notifier_wrapper_trigger (GConfClient *client,
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


/* this is where we take care of error reporting from gconf */
static void
gconf_error_callback (GConfClient *client,
		      GError *err)
{
  GtkWidget *dialog = NULL;
  
  dialog =
    gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL,
                            GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
                            _("An error has happened in the" 
                              " configuration backend.\n"
                              "Maybe some of your settings won't"
                              " be saved."));

  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);
}


/* From now on, the rest is just calling the gconf functions, just checking for the key's validity */
void
gm_conf_set_bool (const gchar *key,
		  const gboolean b)
{
  GConfClient *client = NULL;

  g_return_if_fail (key != NULL);

  client = gconf_client_get_default ();
  gconf_client_set_bool (client, key, b, NULL);
}


gboolean
gm_conf_get_bool (const gchar *key)
{
  GConfClient *client = NULL;

  g_return_val_if_fail (key != NULL, FALSE);

  client = gconf_client_get_default ();
  return gconf_client_get_bool (client, key, NULL);
}


void
gm_conf_set_string (const gchar *key,
		    const gchar *v)
{
  GConfClient *client = NULL;

  g_return_if_fail (key != NULL);

  client = gconf_client_get_default ();
  gconf_client_set_string (client, key, v, NULL);
}


gchar *
gm_conf_get_string (const gchar *key)
{
  GConfClient *client = NULL;
 
  g_return_val_if_fail (key != NULL, NULL);

  client = gconf_client_get_default ();
  return gconf_client_get_string (client, key, NULL);
}


void
gm_conf_set_int (const gchar *key,
		 const int v)
{
  GConfClient *client = NULL;
 
  g_return_if_fail (key != NULL);

  client = gconf_client_get_default ();
  gconf_client_set_int (client, key, v, NULL);
}


int
gm_conf_get_int (const gchar *key)
{
  GConfClient *client = NULL;
 
  g_return_val_if_fail (key != NULL, 0);

  client = gconf_client_get_default ();
  return gconf_client_get_int (client, key, NULL);
}


void
gm_conf_set_float (const gchar *key,
		   const float v)
{
  GConfClient *client = NULL;

  g_return_if_fail (key != NULL);

  client = gconf_client_get_default ();
  gconf_client_set_float (client, key, v, NULL);
}


gfloat
gm_conf_get_float (const gchar *key)
{
  GConfClient *client = NULL;

  g_return_val_if_fail (key != NULL, (float)0);

  client = gconf_client_get_default ();
  return gconf_client_get_float (client, key, NULL);
}


void
gm_conf_set_string_list (const gchar *key,
			 GSList *l)
{
  GConfClient *client = NULL;
 
  g_return_if_fail (key != NULL);

  client = gconf_client_get_default ();
  gconf_client_set_list (client, key, GCONF_VALUE_STRING, l, NULL);
}


GSList *
gm_conf_get_string_list (const gchar *key)
{
  GConfClient *client = NULL;
 
  g_return_val_if_fail (key != NULL, NULL);

  client = gconf_client_get_default ();
  return gconf_client_get_list (client, key, GCONF_VALUE_STRING, NULL);
}


gchar *
gm_conf_escape_key (gchar *key, 
                    gint len)
{
  return gconf_escape_key (key, len);
}


gchar *
gm_conf_unescape_key (gchar *key, 
                      gint len)
{
  return gconf_unescape_key (key, len);
}


gboolean
gm_conf_is_key_writable (gchar *key)
{
  GConfClient *client = NULL;

  g_return_val_if_fail (key != NULL, FALSE);

  client = gconf_client_get_default ();
  return gconf_client_key_is_writable (client, key, NULL);
}


void
gm_conf_init (int argc, 
              char **argv)
{
  gconf_init (argc, argv, 0);
  gconf_client_set_error_handling (gconf_client_get_default (),
				   GCONF_CLIENT_HANDLE_UNRETURNED);
  gconf_client_set_global_default_error_handler (gconf_error_callback);
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
  return gconf_value_get_bool (gconf_entry->value);
}


gint
gm_conf_entry_get_int (GmConfEntry *entry)
{
  GConfEntry *gconf_entry = NULL;

  g_return_val_if_fail (entry != NULL, 0);

  gconf_entry = (GConfEntry *)entry;
  return gconf_value_get_int (gconf_entry->value);
}


const gchar *
gm_conf_entry_get_string (GmConfEntry *entry)
{
  GConfEntry *gconf_entry = NULL;

  g_return_val_if_fail (entry != NULL, NULL);

  gconf_entry = (GConfEntry *)entry;
  return gconf_value_get_string (gconf_entry->value);
}


const GSList *
gm_conf_entry_get_list (GmConfEntry *entry)
{
  GConfEntry *gconf_entry = NULL;

  g_return_val_if_fail (entry != NULL, NULL);

  gconf_entry = (GConfEntry *)entry;
  return gconf_value_get_list (gconf_entry->value);
}


gpointer
gm_conf_notifier_add (const gchar *namespac, 
		      GmConfNotifier func,
		      gpointer user_data)
{
  GConfClient *client = NULL;
  GConfNotifierWrap *wrapper = NULL;

  g_return_val_if_fail (namespac != NULL, NULL);
  g_return_val_if_fail (func != NULL, NULL);
  
  client = gconf_client_get_default ();
  wrapper = gconf_notifier_wrapper_new (func, user_data);
  return GUINT_TO_POINTER(gconf_client_notify_add (client, namespac,
						   gconf_notifier_wrapper_trigger,
						   wrapper,
						   gconf_notifier_wrapper_destroy, NULL));
}


void 
gm_conf_notifier_remove (gpointer identifier)
{
  GConfClient *client = NULL;

  g_return_if_fail (identifier != NULL);

  client = gconf_client_get_default ();  
  gconf_client_notify_remove (client, GPOINTER_TO_UINT (identifier));
}


void
gm_conf_watch ()
{
  GConfClient *client = NULL;

  client = gconf_client_get_default ();  
  gconf_client_add_dir (client, "/apps/gnomemeeting",
			GCONF_CLIENT_PRELOAD_NONE, NULL);
}


void 
gm_conf_unwatch ()
{
  GConfClient *client = NULL;

  client = gconf_client_get_default ();  
  gconf_client_remove_dir (client, "/apps/gnomemeeting", NULL);
}


void 
gm_conf_destroy (const gchar *namespac)
{
  GConfClient *client = NULL;

  g_return_if_fail (namespac != NULL);

  client = gconf_client_get_default ();  
  gconf_client_unset (client, namespac, NULL);
}

