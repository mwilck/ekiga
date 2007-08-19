
/*
 * Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2007 Damien Sandras

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
 *                         local-presentity.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : declaration of a presentity for the local roster
 *
 */


/**
 * This class implements an Ekiga::Presentity.
 * 
 * The Presentity is represented by an internal XML document.
 *
 * There are also 2 private signals:
 * - 'save_me' : this will be emitted when appropriate to signal
 *   other parts to save the new representation.
 * - 'remove_me' : this will be emitted by a Presentity when
 *   it wishes to be removed. This signal will usually be catched
 *   by the GmConf::Heap that will remove it from the GmConf::Heap
 *   and emit the 'removed' and 'presentity_removed' signal 
 *   appropriately.
 */

#ifndef __LOCAL_PRESENTITY_H__
#define __LOCAL_PRESENTITY_H__

#include "form.h"
#include "presence-core.h"
#include "presentity.h"


namespace Local
{
  class Presentity: public Ekiga::Presentity
  {
  public:

    /**
     * Constructors
     */
    Presentity (Ekiga::ServiceCore &_core,
		xmlNodePtr _node);

    Presentity (Ekiga::ServiceCore &_core,
		const std::string _name,
		const std::string _uri,
		const std::set<std::string> _groups);

    ~Presentity ();


    /* 
     * Get elements of the Presentity
     */
    const std::string get_name () const;

    const std::string get_presence () const;

    const std::string get_status () const;

    const std::string get_avatar () const;

    const std::set<std::string> get_groups () const;

    const std::string get_uri () const;


    /**
     * This will set a new presence string
     * and emit the 'updated' signal to announce
     * to the various components that the GmConf::Presentity
     * has been updated.
     */
    void set_presence (const std::string _presence);


    /**
     * This will set a new status string
     * and emit the 'updated' signal to announce
     * to the various components that the GmConf::Presentity
     * has been updated.
     */
    void set_status (const std::string _status);


    /** Populates the given Ekiga::MenuBuilder with the actions.
     * Inherits from Ekiga::Presentity.
     * @param: A MenuBuilder.
     * @return: A populated menu.
     */
    bool populate_menu (Ekiga::MenuBuilder &);


    /** Return the current node in the XML document
     * describing the Presentity.
     * @return: A pointer to the node.
     */
    xmlNodePtr get_node () const;


    /** Private signals.
     * Those signals are usually associated to Action objects
     * to signal other parts like the GmConf::Heap that 
     * a GmConf::Presentity wishes to be 'removed' or 'saved'.
     */
    sigc::signal<void> remove_me;
    sigc::signal<void> save_me;


  private:

    /** This function should be called when a presentity has
     * to be edited. It builds a form with the known
     * fields already filled in.
     */
    void build_edit_presentity_form ();


    /** This should be triggered when an edit Presentity form
     * built with build_edit_presentity_form has been submitted.
     *
     * It does error checking and edits the Presentity.
     * It will also emit the 'updated' signal and the
     * private 'save_me' signal to trigger saving
     * from the GmConf::Heap.
     */
    void edit_presentity_form_submitted (Ekiga::Form &result);


    Ekiga::ServiceCore &core;
    Ekiga::PresenceCore *presence_core;

    xmlNodePtr node;
    xmlNodePtr name_node;
    
    std::string name;
    std::string uri;
    std::string presence;
    std::string status;
    std::string avatar;
    
    std::map<std::string, xmlNodePtr> group_nodes;
    std::set<std::string> groups;
  };
};
#endif
