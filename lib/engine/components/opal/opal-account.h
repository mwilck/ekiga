
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2013 Damien Sandras <dsandras@seconix.com>
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
 *                         opal-account.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Damien Sandras
 *   copyright            : (c) 2008 by Damien Sandras
 *                          (c) 2013 by Julien Puydt
 *   description          : declaration of an OPAL account
 *
 */

#ifndef __OPAL_ACCOUNT_H__
#define __OPAL_ACCOUNT_H__

#include <libxml/tree.h>
#include <opal/pres_ent.h>
#include <sip/sippdu.h>

#include "notification-core.h"
#include "presence-core.h"
#include "personal-details.h"
#include "audiooutput-core.h"

#include "bank-impl.h"

#include "opal-presentity.h"

namespace Opal
{
  // forward declarations:
  class Bank;
  class CallManager;
  class Presentity;
  namespace Sip { class EndPoint; };

  /**
   * @addtogroup accounts
   * @internal
   * @{
   */
  class Account:
    public Ekiga::Account,
    public Ekiga::Heap,
    protected Ekiga::RefLister<Presentity>,
    public Ekiga::PresencePublisher
  {
    friend class Opal::Presentity;
public:

    typedef enum { SIP, Ekiga, DiamondCard, H323 } Type;

    static xmlNodePtr build_node (Opal::Account::Type typus,
				  std::string name,
				  std::string host,
				  std::string user,
				  std::string auth_user,
				  std::string password,
				  bool enabled,
				  unsigned timeout);

    Account (Opal::Bank & bank,
             boost::shared_ptr<Opal::Sip::EndPoint> _sip_endpoint,
	     boost::weak_ptr<Ekiga::PresenceCore> _presence_core,
	     boost::shared_ptr<Ekiga::NotificationCore> _notification_core,
	     boost::shared_ptr<Ekiga::PersonalDetails> _personal_details,
	     boost::shared_ptr<Ekiga::AudioOutputCore> _audiooutput_core,
	     boost::shared_ptr<CallManager> _call_manager,
	     boost::function0<std::list<std::string> > _existing_groups,
	     xmlNodePtr node_);

    ~Account ();

    const std::string get_name () const;

    const std::string get_status () const;

    Ekiga::Account::RegistrationState get_state () const;

    const std::string get_aor () const;

    Type get_type () const;

    std::list<std::string> get_groups () const;

    /** Returns the protocol name of the Opal::Account.
     * This function is purely virtual and should be implemented by the
     * Ekiga::Account descendant.
     * @return The protocol name of the Ekiga::Contact.
     */
    const std::string get_protocol_name () const;

    const std::string get_host () const;

    /** Returns the user name for the Opal::Account.
     * This function is purely virtual and should be implemented by the
     * Ekiga::Account descendant.
     * @return The user name of the Ekiga::Account.
     */
    const std::string get_username () const;

    /** Returns the authentication user name for the Opal::Account.
     * This function is purely virtual and should be implemented by the
     * Ekiga::Account descendant.
     * @return The authentication user name of the Ekiga::Account.
     */
    const std::string get_authentication_username () const;

    /** Returns the password for the Opal::Account.
     * This function is purely virtual and should be implemented by the
     * Ekiga::Account descendant.
     * @return The password of the Ekiga::Account.
     */
    const std::string get_password () const;

    void set_authentication_settings (const std::string & username,
                                      const std::string & password);

    /** Returns the registration timeout for the Opal::Account.
     * This function is purely virtual and should be implemented by the
     * Ekiga::Account descendant.
     * @return The timeout of the Ekiga::Account.
     */
    unsigned get_timeout () const;

    void enable ();

    void disable ();

    bool is_enabled () const;

    bool is_active () const;

    SIPRegister::CompatibilityModes get_compat_mode () const;

    void remove ();

    void edit ();

    boost::signals2::signal<void(void)> trigger_saving;

    /*
     * This is because an opal account is an Ekiga::PresencePublisher
     */
    void publish (const Ekiga::PersonalDetails& details);

    /* This method is public to be called by an opal endpoint, which will push
     * this Opal::Account's new registration state
     * Notice : it's very wrong to make that a const method, but Opal seems to
     * want its Register method to take a const account...
     */
    void handle_registration_event (RegistrationState state_,
				    const std::string info) const;

    /* This method is public to be called by an opal endpoint, which will push
     * this Opal::Account's message waiting information
     */
    void handle_message_waiting_information (const std::string info);

    /* This part of the api is the implementation of Ekiga::Heap */
    void visit_presentities (boost::function1<bool, Ekiga::PresentityPtr > visitor) const;

    /* This object is not an Ekiga::PresenceFetcher, but Opal::Bank is,
     * this is where the information comes from
     */
    boost::signals2::signal<void(std::string, std::string)> presence_received;
    boost::signals2::signal<void(std::string, std::string)> status_received;

protected:
    void on_rename_group (Opal::PresentityPtr pres);

private:
    void fetch (const std::string uri) const;
    void unfetch (const std::string uri) const;

    void decide_type ();

    void add_contact ();
    bool on_add_contact_form_submitted (bool submitted,
					Ekiga::Form& result,
                                        std::string& error);

    bool on_edit_form_submitted (bool submitted,
				 Ekiga::Form &result,
                                 std::string& error);
    void on_consult (const std::string url);
    bool is_myself (const std::string uri) const;

    bool on_rename_group_form_submitted (bool submitted,
                                         Ekiga::Form& result,
                                         std::string& error,
                                         const std::list<std::string> & groups);

    Type type;
    mutable RegistrationState state;
    bool dead;
    bool failed;
    mutable SIPRegister::CompatibilityModes compat_mode;
    std::string aid;
    mutable std::string status;  // the state, as a string
    int message_waiting_number;
    std::string protocol_name;

    mutable bool failed_registration_already_notified;

    PSafePtr<OpalPresentity> presentity;
    void setup_presentity ();

    PDECLARE_PresenceChangeNotifier (Account, OnPresenceChange);

    boost::function0<std::list<std::string> > existing_groups;
    xmlNodePtr node;
    xmlNodePtr roster_node;
    OpalPresenceInfo::State personal_state;
    std::string presence_status;
    void presence_status_in_main (std::string uri,
				  std::string presence,
				  std::string status) const;
    void when_presentity_removed (boost::shared_ptr<Opal::Presentity> pres);
    void when_presentity_updated (boost::shared_ptr<Opal::Presentity> pres);

    Opal::Bank & bank;

    boost::shared_ptr<Opal::Sip::EndPoint> sip_endpoint;
    boost::weak_ptr<Ekiga::PresenceCore> presence_core;
    boost::weak_ptr<Ekiga::NotificationCore> notification_core;
    boost::weak_ptr<Ekiga::PersonalDetails> personal_details;
    boost::weak_ptr<Ekiga::AudioOutputCore> audiooutput_core;
    boost::shared_ptr<CallManager> call_manager;
  };

  typedef boost::shared_ptr<Account> AccountPtr;

  /**
   * @}
   */
};

#endif
