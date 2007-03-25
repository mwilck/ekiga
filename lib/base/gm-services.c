
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
 *                         gm-services.c  -  description
 *                         ------------------------------------------
 *   begin                : written in 2006 by Julien Puydt
 *   copyright            : (c) 2006-2007 by Julien Puydt
 *   description          : implementation for the services object,
 *                          which makes high-level stuff available
 *                          to low-level
 *
 */

/*!\file gm-services.c
 * \brief implementation for the services object
 * \author Julien Puydt
 * \date 2006-2007
 * \ingroup Base
 * \see gm-services.h
 */

#include "gm-services.h"

struct _GmServicesPrivate
{
  GHashTable *hash;
};

static GObjectClass *parent_class = NULL;

/* public api */

GmServices *
gm_services_new ()
{
  return (GmServices *)g_object_new (GM_TYPE_SERVICES, NULL);
}

void
gm_services_register (GmServices *services,
		      const gchar *name,
		      GObject *object)
{
  g_return_if_fail (GM_IS_SERVICES (services));
  g_return_if_fail (name != NULL);
  g_return_if_fail (G_IS_OBJECT (object));

  g_hash_table_replace (services->priv->hash,
			g_strdup (name),
			g_object_ref (object));
}

GObject *
gm_services_peek (const GmServices *services,
		 const gchar *name)
{
  g_return_val_if_fail (GM_IS_SERVICES (services), NULL);
  g_return_val_if_fail (name != NULL, NULL);

  return (GObject *)g_hash_table_lookup (services->priv->hash, name);
}

/* GObject code */

static void
gm_services_dispose (GObject *obj)
{
  GmServices *self = NULL;

#ifdef __GNUC__
  g_print ("%s\n", __PRETTY_FUNCTION__);
#endif

  self = (GmServices *)obj;

  if (self->priv->hash) {

    g_hash_table_unref (self->priv->hash);
    self->priv->hash = NULL;
  }

  parent_class->dispose (obj);
}

static void
gm_services_class_init (gpointer g_class,
			gpointer class_data)
{
  GObjectClass *gobject_class = NULL;

  (void)class_data; /* -Wextra */

  parent_class = (GObjectClass *)g_type_class_peek_parent (g_class);

  g_type_class_add_private (g_class, sizeof (GmServicesPrivate));

  gobject_class = (GObjectClass *)g_class;
  gobject_class->dispose = gm_services_dispose;
}

static void
gm_services_init (GTypeInstance *instance,
		  gpointer g_class)
{
  GmServices *self = NULL;

  (void)g_class; /* -Wextra */

  self = (GmServices *)instance;
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                            GM_TYPE_SERVICES,
                                            GmServicesPrivate);
  self->priv->hash = g_hash_table_new_full (g_str_hash,
					    g_str_equal,
					    g_free,
					    g_object_unref);
}

GType
gm_services_get_type ()
{
  static GType result = 0;

  if (result == 0) {

    static const GTypeInfo info = {
      sizeof (GmServicesClass),
      NULL,
      NULL,
      gm_services_class_init,
      NULL,
      NULL,
      sizeof (GmServices),
      0,
      gm_services_init,
      NULL
    };

    result = g_type_register_static (G_TYPE_OBJECT,
				     "GmServicesType",
				     &info, (GTypeFlags)0);
  }

  return result;
}
