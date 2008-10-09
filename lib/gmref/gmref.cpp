/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2008 Damien Sandras
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
 *                         gmref.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : Reference-counted memory management helpers
 *
 */

#include "gmref.h"

#include <map>

static std::map<GmRefCounted*, int> refcounts;

void
gmref_init ()
{
  /* the goal is to prevent the static initialization fiasco */
  refcounts.clear ();
}

void
gmref_inc (GmRefCounted* obj)
{
  std::map<GmRefCounted*, int>::iterator iter = refcounts.find (obj);
  if (iter == refcounts.end ()) {

    refcounts[obj] = 1;
  } else
    refcounts[obj]++;
}

void
gmref_dec (GmRefCounted* obj)
{
  refcounts[obj]--;

  if (refcounts[obj] <= 0) {

    refcounts.erase (obj);
    delete obj;
  }
}
