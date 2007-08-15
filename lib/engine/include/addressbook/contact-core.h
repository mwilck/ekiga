
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
 *                         contact-core.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : interface of the main contact managing object
 *
 */

#ifndef __CONTACT_CORE_H__
#define __CONTACT_CORE_H__

#include "services.h"
#include "source.h"

/* declaration of a few helper classes */
namespace Ekiga
{
  class ContactDecorator
    {
  public:

      virtual ~ContactDecorator ()
        {}

      virtual bool populate_menu (Contact &/*contact*/,
                                  MenuBuilder &/*builder*/) = 0;
    };
};


/* notice that you give sources to this object as pointers, and this
 * object then assumes the ownership of the source : it will call delete
 * on each of them when it is destroyed.
 */
namespace Ekiga
{
  class ContactCore: public Service
  {
public:

    /** The constructor.
     */
    ContactCore ()
      {}

    /** The destructor.
     */
    ~ContactCore ();


    /*** Service Implementation ***/

    /** Returns the name of the service.
     * @return The service name.
     */
    const std::string get_name () const
      { return "contact-core"; }


    /** Returns the description of the service.
     * @return The service description.
     */
    const std::string get_description () const
      { return "\tContact managing object"; }


    /*** Public API ***/

    /** Adds a source to the ContactCore service.
     * @param The source to be added.
     */
    void add_source (Source &source);


    /** Triggers a callback for all Ekiga::Source sources of the
     * ContactCore service.
     * @return The callback.
     */
    void visit_sources (sigc::slot<void, Source &> visitor);


    /** This signal is emitted when a Ekiga::Source has been 
     * added to the ContactCore Service.
     */
    sigc::signal<void, Source &> source_added;


    /*** Contact Helpers ***/

    void add_contact_decorator (ContactDecorator &decorator);


    /** Create the menu for a given Contact and its actions.
     * @param The Ekiga::Contact and a MenuBuilder object to populate.
     */
    bool populate_contact_menu (Contact &contact,
                                MenuBuilder &builder);

    /*** Misc ***/

    /** Create the menu for the ContactCore and its actions.
     * @param A MenuBuilder object to populate.
     */
    bool populate_menu (MenuBuilder &builder);

    /** This signal is emitted when the ContactCore Service has been 
     * updated.
     */
    sigc::signal<void> updated;


private:

    std::list<ContactDecorator *> contact_decorators;

    std::list<Source *> sources;
  };

};
#endif
