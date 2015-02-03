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
 *                         loudmouth-cluster.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : declaration of a loudmouth cluster
 *
 */

#ifndef __LOUDMOUTH_CLUSTER_H__
#define __LOUDMOUTH_CLUSTER_H__

#include "cluster-impl.h"

#include "loudmouth-handler.h"
#include "loudmouth-heap-roster.h"

namespace LM
{
  class Cluster:
    public Ekiga::ClusterImpl<HeapRoster>,
    public LM::Handler
  {
  public:

    Cluster (boost::shared_ptr<LM::Dialect> dialect_,
	     boost::shared_ptr<Ekiga::PersonalDetails> details_);

    ~Cluster ();

    using Ekiga::ClusterImpl<HeapRoster>::add_heap;

    bool populate_menu (Ekiga::MenuBuilder& builder);

    /* LM::Handler implementation */
    void handle_up (LmConnection* connection,
		    const std::string name);
    void handle_down (LmConnection* connection);
    LmHandlerResult handle_iq (LmConnection* connection,
			       LmMessage* message);
    LmHandlerResult handle_message (LmConnection* connection,
				    LmMessage* message);
    LmHandlerResult handle_presence (LmConnection* connection,
				     LmMessage* message);

  private:

    boost::shared_ptr<LM::Dialect> dialect;
    boost::shared_ptr<Ekiga::PersonalDetails> details;
  };

  typedef boost::shared_ptr<Cluster> ClusterPtr;

};

#endif
