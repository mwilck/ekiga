
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2004 Damien Sandras
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
 *                         zeroconf_publish.h  -  description
 *                         ----------------------------------
 *   begin                : Thu Nov 4 2004
 *   copyright            : (C) 2004 by Sebastien Estienne 
 *   				        Benjamin Leviant
 *   description          : This file contains the zeroconf publisher 
 *   			    and resolver
 *
 */


#include "common.h"


#ifndef _ZEROCONF_PUBLISHER_H_
#define _ZEROCONF_PUBLISHER_H_


/* Zeroconf Service Type */
#define ZC_H323 "_h323._tcp."


/* 
 * Redefine some howl struct because of a bug in howl 
 * including config.h that conflicts with GM config.h
 */
struct _sw_discovery;
typedef struct _sw_discovery *sw_discovery;
typedef unsigned int sw_discovery_oid;
struct _sw_text_record;
typedef struct _sw_text_record	*sw_text_record;


class GMZeroconfPublisher : public PThread
{
  PCLASSINFO (GMZeroconfPublisher, PThread);

 public:

  /* DESCRIPTION  : / 
   * BEHAVIOR     : ZeroconfPublisher constructor
   *		    - initialization of the thread
   *		    - initialization of discovery zeroconf session
   * PRE          : /
   */
  GMZeroconfPublisher ();


  /* DESCRIPTION  : / 
   * BEHAVIOR     : ZeroconfPublisher destructor
   *		    - release the discovery zeroconf session
   * PRE          : /
   */
  ~GMZeroconfPublisher ();


  /* DESCRIPTION  : / 
   * BEHAVIOR     : Return -1 when error occured, 0 else.
   *		    Start the thread
   * PRE          : /
   * 		  
   */
  int Start ();


  /* DESCRIPTION  : / 
   * BEHAVIOR     : Return -1 when error occured, 0 if no error.
   *		    Publish the gnomemeeting zeroconf service
   *		    with data store in class attributes info.
   * PRE          : Start() method must be called before Publish ().
   */
  int Publish ();
  
  
  /* DESCRIPTION  : / 
   * BEHAVIOR     : Return err
   *		    - Stop publishing the gnomemeeting zeroconf service
   *		    - free text_record.
   * PRE          : No comment.
   */
  int Stop ();


 private:

  /* DESCRIPTION  : / 
   * BEHAVIOR     : Return err=SW_OKAY when no error occured.
   *		    Retrieve user personal data from gmconf 
   *		    to class attributes info.
   * PRE          : must be call to update personal data
   */
  int GetPersonalData();
  

  /* DESCRIPTION  : / 
   * BEHAVIOR     : Yields control of the CPU to Howl
   * PRE          : Called by Resume()
   * 		  
   */
  void Main();

  PMutex quit_mutex;
  PSyncPoint thread_sync_point;

  sw_discovery discovery; /* HOWL discovery session */
  sw_discovery_oid publish_id; /* Id of the publisher */
  sw_text_record text_record; /* Txt Record */
  gchar	*name; /* Srv Record */
  int port; /* port number of Srv Record */
};
#endif
