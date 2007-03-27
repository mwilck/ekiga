
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2006 Damien Sandras
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
 *                         gm-plugin-manager.c  -  description
 *                         ------------------------------------------
 *   begin                : written in 2006 by Julien Puydt
 *   copyright            : (c) 2006 by Julien Puydt
 *   description          : implementation of the plugin managing code
 *
 */

/*!\file gm-plugin-manager.c
* \brief implementation of the plugin managing code
* \author Julien Puydt
* \date 2006-2007
* \ingroup Plugins
*/

#include <gmodule.h>

#include "gm-plugin-manager.h"

struct _GmPluginManagerPrivate
{

  GSList *pending_plugins;
  GSList *plugins;
};

static GObjectClass *parent_class = NULL;

/* private api */

static gboolean
already_loaded (GmPluginManager *self,
		const gchar *name)
{
  GSList *ptr = NULL;
  gboolean result = FALSE;

  for (ptr = self->priv->plugins;
       (ptr != NULL) && (result == FALSE);
       ptr = g_slist_next (ptr))
    if (g_ascii_strcasecmp (name, g_module_name ((GModule *)(ptr->data))) == 0)
      result = TRUE;

  for (ptr = self->priv->pending_plugins;
       (ptr != NULL) && (result == FALSE);
       ptr = g_slist_next (ptr))
    if (g_ascii_strcasecmp (name, g_module_name ((GModule *)(ptr->data))) == 0)
      result = TRUE;

  return result;
}

static void
clean_pending (GmPluginManager *self)
{
  GSList *ptr = NULL;

  for (ptr = self->priv->pending_plugins;
       ptr != NULL;
       ptr = g_slist_next (ptr))
    g_module_close ((GModule *)ptr->data);

  g_slist_free (self->priv->pending_plugins);
  self->priv->pending_plugins = NULL;
}

static void
bootstrap_plugins (GmPluginManager *self)
{
  GSList *failed_plugins = NULL;
  GModule *module = NULL;
  GmPluginInfo *(*gm_get_plugin_info) () = NULL;
  GmPluginInfo *plugin_info = NULL;
  gboolean success = TRUE;

  while (success) {

    success = FALSE;

    /* first push all pending plugins :
     * - either to good plugins
     * - or as failed plugins
     */
    while (self->priv->pending_plugins) {

      module = (GModule *)self->priv->pending_plugins->data;
      self->priv->pending_plugins =
	g_slist_delete_link (self->priv->pending_plugins,
			     self->priv->pending_plugins);
      if (g_module_symbol (module, "gm_get_plugin_info",
			   (gpointer)&gm_get_plugin_info)) {

	plugin_info = gm_get_plugin_info ();
	if (plugin_info->init (GM_PLUGIN_MANAGER_SERVICES (*self))) {

	  g_module_make_resident (module);
	  g_print ("Managed to load %s\n", g_module_name (module));
	  self->priv->plugins
	    = g_slist_prepend (self->priv->plugins, module);
	  success = TRUE;
	} else /* init failed */
	  failed_plugins = g_slist_prepend (failed_plugins, module);
      }
    }

    /* now we have tried each and every pending plugin : we have
     * only a list of failed plugins. If we still managed at least one
     * initialization, then perhaps we can still satisfy a new dependancy
     * so we will loop again
     */
    self->priv->pending_plugins = failed_plugins;
    failed_plugins = NULL;

  }
}

static void
preload_plugin (GmPluginManager *self,
		const gchar *filename)
{
  GModule *module = NULL;
  GmPluginInfo *(*gm_get_plugin_info) () = NULL;
  GmPluginInfo *plugin_info = NULL;

  module = g_module_open (filename, (GModuleFlags)0);

  if (module == NULL) {

    g_warning ("Couldn't open %s: %s", filename, g_module_error ());
    return;
  }

  if (already_loaded (self, g_module_name (module))) {

    g_module_close (module);
    return;
  }

  if (g_module_symbol (module, "gm_get_plugin_info",
		       (gpointer)&gm_get_plugin_info) == FALSE) {

    g_warning ("Couldn't recognize %s as a plugin", filename);
    g_module_close (module);
    return;
  }

  plugin_info = gm_get_plugin_info ();

  if (plugin_info->version != GM_PLUGIN_VERSION) {

    g_warning ("Plugin %s doesn't have the right version!", filename);
    g_module_close (module);
    return;
  }

  self->priv->pending_plugins = g_slist_prepend (self->priv->pending_plugins,
						 module);
}


/* public api code */

GmPluginManager *
gm_plugin_manager_new (GmServices *services)
{
  GmPluginManager *result = NULL;

  result = (GmPluginManager *)g_object_new (GM_TYPE_PLUGIN_MANAGER,
					    "services", services,
					    NULL);

  return result;
}

void
gm_plugin_manager_load_directory (GmPluginManager *self,
				  const gchar *dirname)
{
  GDir *dir = NULL;
  const gchar *name = NULL;
  gchar *filename = NULL;

  g_return_if_fail (GM_IS_PLUGIN_MANAGER (self));
  g_return_if_fail (dirname != NULL);

  if (G_UNLIKELY (!g_module_supported ())) {

    g_warning ("Dynamic plugin not supported : can't load them from %s\n",
	       dirname);
    return;
  }

  dir = g_dir_open (dirname, 0, NULL); /* FIXME: handle errors */

  if (dir != NULL) {

    while ((name = g_dir_read_name (dir)) != NULL) {

      filename = g_build_filename (dirname, name, NULL);
      preload_plugin (self, filename);
      g_free (filename);
    }

    g_dir_close (dir);
    bootstrap_plugins (self);
    clean_pending (self);
  }
}

/* GObject boilerplate code */

static void
gm_plugin_manager_dispose (GObject *obj)
{
  GmPluginManager *self = NULL;

#ifdef __GNUC__
  g_print ("%s\n", __PRETTY_FUNCTION__);
#endif

  self = (GmPluginManager *)obj;

  /* FIXME: dispose the plugins */

  parent_class->dispose (obj);
}

static void
gm_plugin_manager_class_init (gpointer g_class,
			      gpointer class_data)
{
  GObjectClass *gobject_class = NULL;

  (void)class_data; /* -Wextra */

  parent_class = (GObjectClass *)g_type_class_peek_parent (g_class);

  g_type_class_add_private (g_class, sizeof (GmPluginManagerPrivate));

  gobject_class = (GObjectClass *)g_class;
  gobject_class->dispose = gm_plugin_manager_dispose;
}

static void
gm_plugin_manager_init (GTypeInstance *instance,
			gpointer g_class)
{
  GmPluginManager *self = NULL;

  (void)g_class; /* -Wextra */

  self = (GmPluginManager *)instance;
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                            GM_TYPE_PLUGIN_MANAGER,
                                            GmPluginManagerPrivate);
  self->priv->plugins = NULL;
}

GType
gm_plugin_manager_get_type ()
{
  static GType result = 0;

  if (result == 0) {

    static const GTypeInfo info = {
      sizeof (GmPluginManagerClass),
      NULL,
      NULL,
      gm_plugin_manager_class_init,
      NULL,
      NULL,
      sizeof (GmPluginManager),
      0,
      gm_plugin_manager_init,
      NULL
    };

    result = g_type_register_static (GM_TYPE_OBJECT,
				     "GmPluginManagerType",
				     &info, (GTypeFlags)0);
  }

  return result;
}
