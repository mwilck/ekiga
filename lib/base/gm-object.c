
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
 *                         gm-object.c  -  description
 *                         ------------------------------------------
 *   begin                : written in 2006 by Julien Puydt
 *   copyright            : (c) 2006-2007 by Julien Puydt
 *   description          : implementation for a base object in ekiga
 *
 */

/*!\file gm-object.c
 * \brief implementation for a base object in ekiga
 * \author Julien Puydt
 * \date 2006-2007
 * \ingroup Base
 */

#include "gm-object.h"

enum {

  GM_OBJECT_PROP_SERVICES = 1
};

static GObjectClass *parent_class = NULL;

static void
gm_object_set_property (GObject *obj,
			guint prop_id,
			const GValue *value,
			GParamSpec *spec)
{
  GmObject *self = NULL;

  self = (GmObject *)obj;

  switch (prop_id) {

  case GM_OBJECT_PROP_SERVICES:
    self->services = g_value_get_object (value);
    g_object_ref (self->services);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, spec);
    break;
  }
}

static void
gm_object_dispose (GObject *obj)
{
  GmObject *self = NULL;

  self = (GmObject *)obj;

  if (self->services) {

    g_object_unref (self->services);
    self->services = NULL;
  }

  parent_class->dispose (obj);
}

static void
gm_object_class_init (gpointer g_class,
		      gpointer class_data)
{
  GObjectClass *gobject_class = NULL;
  GParamSpec *spec = NULL;

  (void)class_data; /* -Wextra */

  parent_class = (GObjectClass *)g_type_class_peek_parent (g_class);

  gobject_class = (GObjectClass *)g_class;
  gobject_class->dispose = gm_object_dispose;
  gobject_class->set_property = gm_object_set_property;

  spec = g_param_spec_object ("services",
                              "services",
                              "Set services",
                              G_TYPE_OBJECT,
                              (GParamFlags)G_PARAM_CONSTRUCT_ONLY
			      | G_PARAM_WRITABLE);
  g_object_class_install_property (gobject_class,
                                   GM_OBJECT_PROP_SERVICES,
                                   spec);
}

static void
gm_object_init (GTypeInstance *instance,
		gpointer g_class)
{
  GmObject *self = NULL;

  (void)g_class; /* -Wextra */

  self = (GmObject *)instance;
  self->services = NULL;
}

GType
gm_object_get_type ()
{
  static GType result = 0;

  if (result == 0) {

    static const GTypeInfo info = {
      sizeof (GmObjectClass),
      NULL,
      NULL,
      gm_object_class_init,
      NULL,
      NULL,
      sizeof (GmObject),
      0,
      gm_object_init,
      NULL
    };

    result = g_type_register_static (G_TYPE_OBJECT,
				     "GmObjectType",
				     &info, (GTypeFlags)0);

  }

  return result;
}
