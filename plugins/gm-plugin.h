
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
 *                         gm-plugin.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2006 by Julien Puydt
 *   copyright            : (c) 2006-2007 by Julien Puydt
 *   description          : common interfaces for plugins
 *
 */

/*!\defgroup Plugins Plugin management
 */

/*!\file gm-plugin.h
 * \brief common interfaces for plugins
 * \author Julien Puydt
 * \date 2006-2007
 * \ingroup Plugins
 */

#ifndef __GM_PLUGIN_H__
#define __GM_PLUGIN_H__

#include "base/gm-services.h"

G_BEGIN_DECLS

typedef struct _GmPluginInfo GmPluginInfo;

#define GM_PLUGIN_VERSION 0

/*!\brief Description block of a plugin
 */
struct _GmPluginInfo
{

  guint version; /*!< Version of the plugin system */
  gboolean (*init) (GmServices *); /*!< Initializer function of the plugin */
};

#define GM_PLUGIN_DEFINE_SIMPLE(init_function) \
static GmPluginInfo info = { \
                             \
  GM_PLUGIN_VERSION,         \
  &init_function             \
};                           \
                             \
GmPluginInfo *               \
gm_get_plugin_info ()        \
{                            \
  return &info;              \
}

G_END_DECLS

#endif
