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
 *                         loudmouth-account.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : declaration of a loudmouth account
 *
 */

#ifndef __LOUDMOUTH_ACCOUNT_H__
#define __LOUDMOUTH_ACCOUNT_H__

#include <libxml/tree.h>

#include "account.h"

#include "loudmouth-dialect.h"
#include "loudmouth-cluster.h"

namespace LM
{

  class Account: public Ekiga::Account
  {
  public:
    Account (boost::shared_ptr<Ekiga::PersonalDetails> details_,
	     boost::shared_ptr<Dialect> dialect_,
	     boost::shared_ptr<Cluster> cluster_,
	     xmlNodePtr node_);

    Account (boost::shared_ptr<Ekiga::PersonalDetails> details_,
	     boost::shared_ptr<Dialect> dialect_,
	     boost::shared_ptr<Cluster> cluster_,
	     const std::string name, const std::string user,
	     const std::string server, int port,
	     const std::string resource, const std::string password,
	     bool enable_on_startup);

    ~Account ();

    void enable ();

    void disable ();

    xmlNodePtr get_node () const;

    boost::signals2::signal<void(void)> trigger_saving;

    const std::string get_name () const;

    const std::string get_status () const;

    bool is_enabled () const;

    bool is_active () const;

    bool populate_menu (Ekiga::MenuBuilder& builder);

    /* public only to be called by C callbacks */
    void on_connection_opened (bool result);

    void on_disconnected (LmDisconnectReason reason);

    void on_authenticate (bool result);

    /* LM::Handler-like interface
     * but not exactly, since it's the hub from which all information flows
     * to the real handlers
     */
    void handle_up ();
    void handle_down ();
    LmHandlerResult handle_iq (LmMessage* message);
    LmHandlerResult handle_message (LmMessage* message);
    LmHandlerResult handle_presence (LmMessage* message);

  private:

    void edit ();
    void on_edit_form_submitted (bool submitted,
				 Ekiga::Form &result);

    void remove ();

    boost::shared_ptr<Ekiga::PersonalDetails> details;
    boost::shared_ptr<Dialect> dialect;
    boost::shared_ptr<Cluster> cluster;
    xmlNodePtr node;

    std::string status;

    LmConnection* connection;
  };
};

#endif
