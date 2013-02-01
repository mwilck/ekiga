
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2009 Damien Sandras <dsandras@seconix.com>
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
 *                         opal-bank.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Damien Sandras
 *   copyright            : (c) 2008 by Damien Sandras
 *   description          : declaration of an OPAL bank 
 *
 */

#ifndef __OPAL_BANK_H__
#define __OPAL_BANK_H__

#include "contact-core.h"
#include "presence-core.h"

#include "opal-account.h"

namespace Opal
{
  /**
   * @addtogroup accounts
   * @internal
   * @{
   */
  class Bank: 
      public Ekiga::BankImpl<Account>,
      public Ekiga::PresencePublisher,
      public Ekiga::PresenceFetcher,
      public Ekiga::ContactDecorator,
      public Ekiga::PresentityDecorator,
      public Ekiga::Service
  {
public:

    Bank (Ekiga::ServiceCore &_core);

    virtual ~Bank () { }

    bool populate_menu (Ekiga::MenuBuilder & builder);

    const std::string get_name () const
    { return "opal-account-store"; }

    const std::string get_description () const
    { return "\tStores the opal accounts"; }

    /*
     * this object is an Ekiga::PresenceFetcher and an Ekiga::PresencePublisher
     */
    void publish (const Ekiga::PersonalDetails& details);

    void fetch (const std::string uri);

    void unfetch (const std::string uri);

    void new_account (Account::Type acc_type,
                      std::string username = "",
                      std::string password = "");

    /*
     * this object is an Ekiga::ContactDecorator and an Ekiga::PresentityDecorator
     */
    bool populate_menu (Ekiga::ContactPtr contact,
			const std::string uri,
			Ekiga::MenuBuilder& builder);

    bool populate_menu (Ekiga::PresentityPtr presentity,
			const std::string uri,
			Ekiga::MenuBuilder& builder);

    /** Find the account with the given address of record in the Bank
     * @param aor is the address of record of the Account or the host to look 
     *        for
     * @return The Opal::Account if an Account was found, false otherwise.
     *         The returned account should not be freed.
     */
    AccountPtr find_account (const std::string& aor);


    void call_manager_ready ();

private:
    boost::shared_ptr<Opal::Sip::EndPoint> sip_endpoint;
#ifdef HAVE_H323
    boost::shared_ptr<Opal::H323::EndPoint> h323_endpoint;
#endif
    boost::weak_ptr<Ekiga::PresenceCore> presence_core;
    boost::shared_ptr<Ekiga::NotificationCore> notification_core;
    boost::shared_ptr<Ekiga::PersonalDetails> personal_details;
    boost::shared_ptr<Ekiga::AudioOutputCore> audiooutput_core;
    boost::weak_ptr<CallManager> opal_component;

    bool populate_menu_helper (const std::string fullname,
			       const std::string& uri,
			       Ekiga::MenuBuilder& builder);

    void on_new_account_form_submitted (bool submitted,
					Ekiga::Form& form,
					Account::Type acc_type);

    void add (Account::Type acc_type,
              std::string name, 
              std::string host,
              std::string user,
              std::string auth_user,
              std::string password,
              bool enabled,
              unsigned timeout);

    void save () const;

  };

  /**
   * @}
   */
};

#endif
