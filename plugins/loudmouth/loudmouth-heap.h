/*
 * Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2014 Damien Sandras <dsandras@seconix.com>

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
 *                         loudmouth-heap.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2014 by Julien Puydt
 *   copyright            : (c) 2014 by Julien Puydt
 *   description          : declaration of a Heap-as-a-structure
 *                          to be used in Conversation instances
 *
 */

#ifndef __LOUDMOUTH_HEAP_H__
#define __LOUDMOUTH_HEAP_H__

#include "heap-impl.h"
#include "loudmouth-presentity.h"

namespace LM
{

  class Heap: public Ekiga::HeapImpl<Presentity>
  {
  public:

    using Ekiga::HeapImpl<Presentity>::add_connection;
    using Ekiga::HeapImpl<Presentity>::add_presentity;
    using Ekiga::HeapImpl<Presentity>::remove_presentity;
  };

  typedef typename boost::shared_ptr<Heap> HeapPtr;
};

#endif
