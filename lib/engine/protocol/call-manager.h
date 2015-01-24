
/*
 * Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2009 Damien Sandras <dsandras@seconix.com>

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
 *                         call-manager.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Damien Sandras 
 *   copyright            : (c) 2007 by Damien Sandras
 *   description          : Declaration of the interface of a call manager
 *                          implementation backend. A call manager handles calls,
 *                          sometimes simultaneously.
 *
 */


#ifndef __CALL_MANAGER_H__
#define __CALL_MANAGER_H__

#include <set>
#include <boost/signals2.hpp>
#include <boost/bind.hpp>

#include <boost/smart_ptr.hpp>

#include "call-protocol-manager.h"
#include "codec-description.h"

namespace Ekiga
{

/**
 * @addtogroup calls
 * @{
 */

  class CallManager
  {

    public:

    typedef std::set<boost::shared_ptr<CallProtocolManager> >::iterator iterator;
    typedef std::set<boost::shared_ptr<CallProtocolManager> >::const_iterator const_iterator;

    /* The constructor
     */
    CallManager () {};

    /* The destructor
     */
    virtual ~CallManager () {}

    /** Add a CallProtocolManager to the CallManager.
     * @param The manager to be added.
     */
    void add_protocol_manager (boost::shared_ptr<CallProtocolManager> manager);

    /** Remove a CallProtocolManager from the CallManager.
     * @param The manager to be removed.
     */
    void remove_protocol_manager (boost::shared_ptr<CallProtocolManager> manager);

    /** Return a pointer to a CallProtocolManager of the CallManager.
     * @param protocol is the protcol name.
     * @return a pointer to the CallProtocolManager or NULL if none.
     */
    boost::shared_ptr<CallProtocolManager> get_protocol_manager (const std::string &protocol) const;

    /** Return iterator to beginning
     * @return iterator to beginning
     */
    iterator begin ();
    const_iterator begin () const;

    /** Return iterator to end
     * @return iterator to end
     */
    iterator end ();
    const_iterator end () const;

    /** This signal is emitted when a Ekiga::CallProtocolManager has been
     * added to the CallManager.
     */
    boost::signals2::signal<void(boost::shared_ptr<CallProtocolManager>)> manager_added;

    /** This signal is emitted when a Ekiga::CallProtocolManager has been
     * removed from the CallManager.
     */
    boost::signals2::signal<void(boost::shared_ptr<CallProtocolManager>)> manager_removed;


    /*
     * CALL MANAGEMENT
     */

    /** Create a call based on the remote uri given as parameter
     * @param: An uri
     * @return: true if a Ekiga::Call could be created
     */
    virtual bool dial (const std::string & uri) = 0;

    /** Hang up all active calls (if any).
     */
    virtual void hang_up () = 0;

    /* Return true if URI can be handled by the CallCore,
     * false otherwise.
     * @param the URI to test
     * @return true of the URI can be handled, false otherwise
     */
    virtual bool is_supported_uri (const std::string & uri) = 0;


    /*
     * PROTOCOL INFORMATION
     */

    /**
     * @return the protocol name
     */
    const std::list<std::string> get_protocol_names () const;

    /**
     * @return the interface on which we are accepting calls. Generally,
     * under the form protocol:IP:port.
     */
    const CallProtocolManager::InterfaceList get_interfaces () const;


    /*
     * Misc
     */

    /** Enable the given codecs
     * @param codecs is a set of the codecs and their descriptions
     *        when the function returns, the list also contains disabled
     *        codecs supported by the CallManager. Unsupported codecs
     *        have been removed.
     */
    virtual void set_codecs (CodecList & codecs) = 0;

    /** Return the list of available codecs
     * @return a set of the codecs and their descriptions
     */
    virtual const Ekiga::CodecList & get_codecs () const = 0;

    /** Set the display name used on outgoing calls
     * @param name is the display name to use.
     */
    virtual void set_display_name (const std::string & name) = 0;

    /** Return the display name used on outgoing calls
     */
    virtual const std::string & get_display_name () const = 0;

    /** Enable echo cancellation
     * @param enabled is true if echo cancellation should be enabled, false
     * otherwise.
     */
    virtual void set_echo_cancellation (bool enabled) = 0;

    /** Get echo cancellation setting
     * @return true if echo cancellation is enabled.
     */
    virtual bool get_echo_cancellation () const = 0;

    /** Enable silence detection
     * @param enabled is true if silence detection should be enabled, false
     * otherwise.
     */
    virtual void set_silence_detection (bool enabled) = 0;

    /** Get silence detection setting
     * @return true if silence detection is enabled.
     */
    virtual bool get_silence_detection () const = 0;

    /** Set maximum jitter
     * @param max_val is the maximum jitter for calls in seconds.
     */
    virtual void set_maximum_jitter (unsigned max_val) = 0;

    /** Get maximum jitter
     * @return the maximum jitter for calls in seconds.
     */
    virtual unsigned get_maximum_jitter () const = 0;

    /** Set delay before dropping an incoming call
     * @param delay is the delay after which the call should be rejected
     * (or forwarded if supported by the CallManager).
     */
    virtual void set_reject_delay (unsigned delay) = 0;

    /** Get delay before dropping an incoming call
     * @return the delay in seconds after which a call should be rejected
     * (or forwarded if supported by the CallManager).
     */
    virtual unsigned get_reject_delay () const = 0;

    /** Set auto answer
     * @param true if incoming calls should be auto answered.
     */
    virtual void set_auto_answer (bool enabled) = 0;

    /** Get auto answer
     * @return true if incoming calls should be auto answered.
     */
    virtual bool get_auto_answer () const = 0;


    /*
     * MISC
     */
    boost::signals2::signal<void(void)> ready;

    private:
    std::set<boost::shared_ptr<CallProtocolManager> > managers;
  };

  /**
   * @}
   */
};
#endif
