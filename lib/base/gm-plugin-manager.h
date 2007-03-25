
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
 *                         gm-plugin-manager.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2006 by Julien Puydt
 *   copyright            : (c) 2006-2007 by Julien Puydt
 *   description          : interface of the plugin managing code
 *
 */

/*!\file gm-plugin-manager.h
 * \brief interface of the plugin managing code
 * \author Julien Puydt
 * \date 2006-2007
 * \ingroup Plugins
 */

#ifndef __GM_PLUGIN_MANAGER_H__
#define __GM_PLUGIN_MANAGER_H__

#include "gm-plugin.h"
#include "gm-object.h"

G_BEGIN_DECLS

typedef struct _GmPluginManager GmPluginManager;
typedef struct _GmPluginManagerPrivate GmPluginManagerPrivate;
typedef struct _GmPluginManagerClass GmPluginManagerClass;

/* public normal api */

#define GM_PLUGIN_MANAGER_SERVICES(obj) (GM_OBJECT_SERVICES ((obj).parent))

/*!
 * \brief Creates a GmPluginManager
 *
 * \param services A ready-to-work GmServices object
 *
 * \returns A pointer to a GmPluginManager
 */
GmPluginManager *gm_plugin_manager_new (GmServices *services);

/*!
 * \brief Attemts to load plugins from a specified directory
 *
 * \param self The GmPluginManager to operate on
 * \param dirname The pathname to scan and load files from
 */
void gm_plugin_manager_load_directory (GmPluginManager *self,
				       const gchar *dirname);

/* GObject thingies */

struct _GmPluginManager {
  GmObject parent;

  GmPluginManagerPrivate *priv;
};

struct _GmPluginManagerClass {
  GmObjectClass parent;

};

#define GM_TYPE_PLUGIN_MANAGER (gm_plugin_manager_get_type ())

#define GM_PLUGIN_MANAGER(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GM_TYPE_PLUGIN_MANAGER, GmPluginManager))

#define GM_IS_PLUGIN_MANAGER(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GM_TYPE_PLUGIN_MANAGER))

#define GM_PLUGIN_MANAGER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GM_TYPE_PLUGIN_MANAGER, GmPluginManagerClass))

#define GM_PLUGIN_MANAGER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GM_TYPE_PLUGIN_MANAGER, GmPluginManagerClass))

GType gm_plugin_manager_get_type ();

G_END_DECLS

#endif
