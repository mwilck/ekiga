
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
 *                         stunclient.cpp  -  description
 *                         ------------------------------
 *   begin                : Thu Sep 30 2004
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : Multithreaded class for the stun client.
 *
 */


#include "../config.h" 

#include "stunclient.h"
#include "gnomemeeting.h"
#include "endpoint.h"
#include "log_window.h"
#include "misc.h"

#include <lib/gm_conf.h>

#include <ptclib/pstun.h>



/* The class */
GMStunClient::GMStunClient (BOOL r)
  :PThread (1000, NoAutoDeleteThread)
{
  gchar *conf_string = NULL;
  
  reg = r;

  gnomemeeting_threads_enter ();
  conf_string = gm_conf_get_string (NAT_KEY "stun_server");
  stun_host = conf_string;
  g_free (conf_string);
  gnomemeeting_threads_leave ();
  
  
  this->Resume ();
}


GMStunClient::~GMStunClient ()
{
  /* Nothing to do here */
  PWaitAndSignal m(quit_mutex);
}


void GMStunClient::Main ()
{
  GtkWidget *history_window = NULL;

  GMH323EndPoint *endpoint = NULL;
  PSTUNClient *stun = NULL;

  BOOL regist = FALSE;
  
  endpoint = GnomeMeeting::Process ()->Endpoint ();
  history_window = GnomeMeeting::Process ()->GetHistoryWindow ();

  char *name [] = { N_("Unknown NAT"), N_("Open NAT"),
    N_("Cone NAT"), N_("Restricted NAT"), N_("Port Restricted NAT"),
    N_("Symmetric NAT"), N_("Symmetric Firewall"), N_("Blocked"),
    N_("Partially Blocked")};

  for (int i = 0 ; i < 9 ; i++)
    name [i] = gettext (name [i]);

  PWaitAndSignal m(quit_mutex);

  gnomemeeting_threads_enter ();
  regist = gm_conf_get_bool (NAT_KEY "enable_stun_support");
  gnomemeeting_threads_leave ();

  if (!regist) {

    ((H323EndPoint *) endpoint)->SetSTUNServer (PString ());
    gnomemeeting_threads_enter ();
    gm_history_window_insert (history_window, _("Removed STUN server"));
    gnomemeeting_threads_leave ();
      
    return;
  }
    
  
  /* Set the STUN server for the endpoint */
  if (!stun_host.IsEmpty () && reg) {
    
    ((H323EndPoint *) endpoint)->SetSTUNServer (stun_host);
    stun = endpoint->GetSTUN ();

    if (stun) {

      gnomemeeting_threads_enter ();
      nat_type = name [stun->GetNatType ()];
      gm_history_window_insert (history_window, _("Set STUN server to %s (%s)"), (const char *) stun_host, (const char *) nat_type);
      gnomemeeting_threads_leave ();
    }
  } 
  /* Only detects */
  else if (!reg && !stun_host.IsEmpty ()) {

    PSTUNClient stun (stun_host,
		      endpoint->GetUDPPortBase(), 
		      endpoint->GetUDPPortMax(),
		      endpoint->GetRtpIpPortBase(), 
		      endpoint->GetRtpIpPortMax());

    nat_type = name [stun.GetNatType ()];
  }
}


PString GMStunClient::GetNatType ()
{
  return nat_type;
}

