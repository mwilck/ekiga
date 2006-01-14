
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2006 Damien Sandras
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *
 * GnomeMeting is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OpenH323 and Pwlib, and distribute the combination, without
 * applying the requirements of the GNU GPL to the OpenH323 program, as long
 * as you do follow the requirements of the GNU GPL for all the rest of the
 * software thus combined.
 */

/*
 *                         avahi_publish.h  -  description
 *                         ------------------------------------
 *   begin                : Sun Aug 21 2005
 *   copyright            : (C) 2005 by Sebastien Estienne 
 *   description          : This file contains the Avahi zeroconf publisher. 
 *
 */


#include "common.h"

#ifndef _AVAHI_PUBLISHER_H_
#define _AVAHI_PUBLISHER_H_

#include <avahi-client/client.h>
#include <avahi-client/publish.h>
#include <avahi-common/alternative.h>
#include <avahi-common/malloc.h>
#include <avahi-common/error.h>
#include <avahi-common/timeval.h>
#include <avahi-glib/glib-watch.h>


/* Zeroconf Service Type */
#define ZC_H323 "_h323._tcp"
#define ZC_SIP "_sip._udp"

class GMManager;


class GMZeroconfPublisher
{

 public:

  /* DESCRIPTION  : / 
   * BEHAVIOR     : ZeroconfPublisher constructor
   *		    - insert Avahi in Gnomemeeting Mainloop
   *		    - initialization of some variables
   * PRE          : /
   */
  GMZeroconfPublisher (GMManager &);


  /* DESCRIPTION  : ZeroconfPublisher destructor 
   * BEHAVIOR     : 
   *		    - Free avahi Client and Entry group
   *		    - Stop publishing the gnomemeeting zeroconf service
   *		    - free text_record.
   * PRE          : /
   */
  ~GMZeroconfPublisher ();


  /* DESCRIPTION  : / 
   * BEHAVIOR     : Return -1 when error occured, 0 if no error.
   *		    Publish the gnomemeeting zeroconf service
   *		    with data store in class attributes info.
   * PRE          : Start() method must be called before Publish ().
   */
  int Publish ();
  
  
  void ClientCallback (AvahiClient *c, 
		       AvahiClientState state, 
		       void *userdata);
  
  
  int CreateServices (AvahiClient *c, 
		      void *userdata);

  
  void EntryGroupCallback (AvahiEntryGroup *group, 
			   AvahiEntryGroupState state, 
			   void *userdata);

 private:

  AvahiClient *client;
  AvahiEntryGroup *group;
  
  char *name;                         /* Srv Record */
  
  AvahiStringList *h323_text_record;  /* H323 Txt Record */
  AvahiStringList *sip_text_record;   /* Sip Txt Record */
  
  uint16_t h323_port;                 /* port number of Srv Record */
  uint16_t sip_port;                  /* port number of Srv Record */
  
  AvahiGLibPoll *glib_poll;
  const AvahiPoll *poll_api;

  /* DESCRIPTION  : / 
   * BEHAVIOR     : Return err=SW_OKAY when no error occured.
   *		    Retrieve user personal data from gmconf 
   *		    to class attributes info.
   * PRE          : must be call to update personal data
   */
  int GetPersonalData();

  GMManager & manager;
};

#endif
