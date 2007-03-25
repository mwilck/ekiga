
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
 *                         gm-object.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2006 by Julien Puydt
 *   copyright            : (c) 2006-2007 by Julien Puydt
 *   description          : interface for a base object in ekiga
 *
 */

/*!\defgroup Base Ekiga base services and objects
 */

/*!\file gm-object.h
 * \brief interface for a base object in ekiga
 * \author Julien Puydt
 * \date 2006-2007
 * \ingroup Base
 */

#ifndef __GM_OBJECT_H__
#define __GM_OBJECT_H__

#include "gm-services.h"

G_BEGIN_DECLS

typedef struct _GmObject GmObject;
typedef struct _GmObjectClass GmObjectClass;

#define GM_OBJECT_SERVICES(obj) ((obj).services)

struct _GmObject {
  GObject parent;

  /* readonly outside! */
  GmServices *services; /*!< services hint */
};

struct _GmObjectClass {
  GObjectClass parent;
};

#define GM_TYPE_OBJECT (gm_object_get_type ())

#define GM_OBJECT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GM_TYPE_OBJECT, GmObject))

#define GM_IS_OBJECT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GM_TYPE_OBJECT))

#define GM_OBJECT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GM_TYPE_OBJECT, GmObjectClass))

#define GM_OBJECT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GM_TYPE_OBJECT, GmObjectClass))

/*!
 * \brief GTyping function for GmObject
 */
GType gm_object_get_type ();


G_END_DECLS

#endif
