
/* Ekiga -- A VoIP and Video-Conferencing application
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
 *                         stunclient.cpp  -  description
 *                         ------------------------------
 *   begin                : Thu Sep 30 2004
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : Multithreaded class for the stun client.
 *
 */


#include "config.h"

#include "manager.h"
#include "stun.h"
#include "gmconf.h"

#include <ptclib/pstun.h>


/* Helper function */
static PString get_nat_name (int nat_type);

/* Implementation */

static PString
get_nat_name (int nat_type)
{
  static const gchar *name [] = 
    { 
      N_("Unknown NAT"), 
      N_("Open NAT"),
      N_("Cone NAT"), 
      N_("Restricted NAT"), 
      N_("Port Restricted NAT"),
      N_("Symmetric NAT"), 
      N_("Symmetric Firewall"), 
      N_("Blocked"),
      N_("Partially Blocked"),
      N_("No NAT"),
      NULL,
    };

  return PString (gettext (name [nat_type]));
}

/* The class */
GMStunClient::GMStunClient (bool display_progress_,
			    bool display_config_dialog_,
			    bool wait_,
			    GtkWidget *parent_window,
			    GMManager & endpoint)
  :PThread (1000, NoAutoDeleteThread), 
  ep (endpoint)
{
  gchar *conf_string = NULL;
  int nat_method = 0;
  
  nat_method = gm_conf_get_int (NAT_KEY "method");
  conf_string = gm_conf_get_string (NAT_KEY "stun_server");
  stun_host = conf_string;
  
  display_progress = display_progress_;
  display_config_dialog = display_config_dialog_;
  wait = wait_;

  parent = parent_window;
  
  this->Resume ();
  if (wait)
    sync.Wait ();
  
  g_free (conf_string);
}


GMStunClient::~GMStunClient ()
{
  /* Nothing to do here */
  PWaitAndSignal m(quit_mutex);
}


void GMStunClient::Main ()
{
  PSTUNClient *stun = NULL;

  PString html;
  PString public_ip;
  PString listener_ip;
  gboolean has_nat = FALSE;
  int nat_type_index = 0;

  PWaitAndSignal m(quit_mutex);

  /* Async remove the current stun server setting */
  if (stun_host.IsEmpty ()) {
    if (wait)
      sync.Signal ();
    ((OpalManager *) &ep)->SetSTUNServer (PString ());
    return;
  }

  /* Set the STUN server for the endpoint */
  std::cout << "Set STUN server" << std::endl << std::flush;
  ((OpalManager *) &ep)->SetSTUNServer (stun_host);

  stun = ep.GetSTUN ();
  if (stun) 
    nat_type_index = stun->GetNatType ();
  else if (!has_nat) 
    nat_type_index = 9; 
  nat_type = get_nat_name (nat_type_index);

  if (wait)
    sync.Signal ();
}
