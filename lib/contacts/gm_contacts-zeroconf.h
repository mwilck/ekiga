
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
 *                         gm_contacts-zeroconf.h  -  description
 *                         --------------------------------------
 *   begin                : Thu Nov 4 2004
 *   copyright            : (C) 2004 by Sebastien Estienne
 *   				        Benjamin Leviant
 *   description          : This file contains zeroconf browser related headers
 *
 */


#ifndef _GM_CONTACTS_ZEROCONF_H_
#define _GM_CONTACTS_ZEROCONF_H_

#define ZC_H323 "_h323._tcp."

#include <gm_contacts.h>
#include <ptlib.h>
#include <lib/gm_conf.h>


/* 
 * Redefine some howl struct because of a bug in howl 
 * including config.h that conflish with GM config.h
 */
struct				_sw_discovery;
typedef struct _sw_discovery*	sw_discovery;
typedef unsigned int		sw_discovery_oid;

#define PERSONAL_DATA_KEY "/apps/gnomemeeting/general/personal_data/"
#define PORTS_KEY "/apps/gnomemeeting/protocols/h323/ports/"


GSList *gnomemeeting_zero_addressbook_get_contacts (GmAddressbook *,
						    int &,
						    gboolean,
						    gchar *,
						    gchar *,
						    gchar *,
						    gchar *);


void gnomemeeting_zero_addressbook_init ();


class GMZeroconfBrowser : public PThread
{
  PCLASSINFO(GMZeroconfBrowser, PThread);

public:

  /* DESCRIPTION  : / 
   * BEHAVIOR     : ZeroconfBrowse constructor :
   *		    * initialization of the thread
   *		    * initialization of the discovery zeroconf session
   * PRE          : /
   */
  GMZeroconfBrowser ();

  /* DESCRIPTION  : / 
   * BEHAVIOR     : ZeroconfBrowse destructor.
   *		    Releases the discovery zeroconf session.
   * PRE          : /
   */
  ~GMZeroconfBrowser ();

  /* DESCRIPTION  : / 
   * BEHAVIOR     : Returns -1 when error occured, 0 else.
   *		    Starts the thread
   * PRE          : /
   */
  int Start ();


  /* DESCRIPTION  : / 
   * BEHAVIOR     : Returns -1 when error occurs, 0 else.
   *		    Browse the gnomemeeting zeroconf service
   *		    to retrieve neighborhood contacts.
   * PRE          : Start () method must be called before Browse ().
   */
  int Browse ();


  /* DESCRIPTION  : / 
   * BEHAVIOR     : Returns -1 when error occured, 0 else.
   *		    Cancel browsing for the GM zeroconf service.
   * PRE          : /
   * 		  
   */
  int Stop ();


  /* DESCRIPTION  : / 
   * BEHAVIOR     : Returns NULL when an error occurs or when the contacts GList
   * 		    is empty.
   *		    Returns a copy of the list.
   * PRE          : /
   */
  GSList *GetContacts ();

  GSList *contacts;
  PMutex mutex;

private:

  /* DESCRIPTION  : / 
   * BEHAVIOR     : Yields control of the CPU to Howl
   * PRE          : Called by Resume()
   * 		  
   */
  void Main();

  sw_discovery discovery; /* HOWL discovery session */
  sw_discovery_oid browse_id; /* Id of the publisher */

  PMutex quit_mutex;
  PSyncPoint thread_sync_point;
};
#endif
