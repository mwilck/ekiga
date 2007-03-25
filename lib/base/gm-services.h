
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
 *                         gm-services.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2006 by Julien Puydt
 *   copyright            : (c) 2006-2007 by Julien Puydt
 *   description          : interface for the services object,
 *                          which makes high-level stuff available
 *                          to low-level
 *
 */

/*!\file gm-services.h
 * \brief interface for the services object
 * \author Julien Puydt
 * \date 2006-2007
 * \ingroup Base
 * \see gm-services.c
 *
 * The basic idea of a GmServices object is that other objects are registered
 * to it under names ; this allows for example plugins to detect available
 * features and decide whether their dependencies are satisfied -- giving a
 * basic introspection.
 *
 * It avoids adding pointers with #ifdef in a global place, and replaces them
 * by an unconditional object, which is queried at runtime.
 */

#ifndef __GM_SERVICES_H__
#define __GM_SERVICES_H__

#include <glib-object.h>

G_BEGIN_DECLS

typedef struct _GmServices GmServices;
typedef struct _GmServicesPrivate GmServicesPrivate;
typedef struct _GmServicesClass GmServicesClass;

/* public api */

/*!
 * \brief Create a new GmServices object
 *
 * \returns A pointer to the created GmServices object
 */
GmServices *gm_services_new ();


/*!
 * \brief Register a GmObject in the GmServices
 *
 * \param services A valid GmServices object
 * \param name The string to associate with the object
 * \param object The GObject to register
 */
void gm_services_register (GmServices *services,
			   const gchar *name,
			   GObject *object);

/*!
 * \brief Return the GObject associated to a string
 *
 * \param services A valid GmServices object
 * \param name The string to query in the GmServices table
 *
 * \returns The associated GObject, or NULL if not found
 */
GObject *gm_services_peek (const GmServices *services,
			   const gchar *name);

/* GObject */

struct _GmServices {
  GObject parent;

  GmServicesPrivate *priv;
};

struct _GmServicesClass {
  GObjectClass parent;
};


#define GM_TYPE_SERVICES (gm_services_get_type ())

#define GM_SERVICES(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GM_TYPE_SERVICES, GmServices))

#define GM_IS_SERVICES(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GM_TYPE_SERVICES))

#define GM_SERVICES_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GM_TYPE_SERVICES, GmServicesClass))

#define GM_SERVICES_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GM_TYPE_SERVICES, GmServicesClass))

GType gm_services_get_type ();

G_END_DECLS

#endif
