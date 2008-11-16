
/*
 * Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2008 Damien Sandras

 * This program is free software; you can  redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version. This program is distributed in the hope
 * that it will be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Ekiga is licensed under the GPL license and as a special exception, you
 * have permission to link or otherwise combine this program with the
 * programs OPAL, OpenH323 and PWLIB, and distribute the combination, without
 * applying the requirements of the GNU GPL to the OPAL, OpenH323 and PWLIB
 * programs, as long as you do follow the requirements of the GNU GPL for all
 * the rest of the software thus combined.
 */


/*
 *                         gm-refcounted-boxed.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : implementation of boxed GmRefCounted objects
 *
 */

#include "gm-refcounted-boxed.h"

#include "gmref.h"

static GmRefCounted*
gmrefcounted_boxed_copy (GmRefCounted* obj)
{
  gmref_inc (obj);
  return obj;
}

static void
gmrefcounted_boxed_free (GmRefCounted* obj)
{
  gmref_dec (obj);
}

GType
gm_refcounted_boxed_get_type ()
{
  static GType result = 0;

  if (result == 0) {

    result
      = g_boxed_type_register_static ("GmRefCountedBoxed",
				      (GBoxedCopyFunc)gmrefcounted_boxed_copy,
				      (GBoxedFreeFunc)gmrefcounted_boxed_free);
  }

  return result;
}
