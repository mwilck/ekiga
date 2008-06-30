
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2007 Damien Sandras
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
 *                         opal-account.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Damien Sandras
 *   copyright            : (c) 2008 by Damien Sandras
 *   description          : implementation of an opal account
 *
 */

#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>

#include "config.h"

#include "common.h"

#include "opal-account.h"
#include "form-request-simple.h"


Opal::Account::Account (Ekiga::ServiceCore & core,
                        const std::string & account)
{
  int i = 0;
  char *pch = strtok ((char *) account.c_str (), "|");
  while (pch != NULL) {

    switch (i) {

    case 0:
      enabled = atoi (pch);
      break;

    case 2:
      aid = pch;
      break;

    case 3:
      name = pch;
      break;

    case 4:
      protocol_name = pch;
      break;

    case 5:
      host = pch;
      break;

    case 7:
      username = pch;
      break;

    case 8:
      auth_username = pch;
      break;

    case 9:
      password = pch;
      break;

    case 10:
      timeout = atoi (pch);
      break;

    case 1:
    case 6:
    case 11:
    default:
      break;
    }
    pch = strtok (NULL, "|");
    i++;
  }

  account_core = dynamic_cast<Ekiga::AccountCore*>(core.get ("account-core"));

  if (enabled)
    enable ();
}


Opal::Account::Account (Ekiga::ServiceCore & core,
                        std::string _name, 
                        std::string _host,
                        std::string _username,
                        std::string _auth_username,
                        std::string _password,
                        bool _enabled,
                        unsigned _timeout)
{
  enabled = _enabled;
  aid = (const char *) PGloballyUniqueID ().AsString ();
  name = _name;
  protocol_name = "SIP";
  host = _host;
  username = _username;
  auth_username = _auth_username;
  password = _password;
  timeout = _timeout;

  account_core = dynamic_cast<Ekiga::AccountCore*>(core.get ("account-core"));

  if (enabled)
    enable ();
}


Opal::Account::~Account ()
{
}


const std::string Opal::Account::as_string () const
{
  std::stringstream str;

  str << enabled << "|1|" 
      << aid << "|" 
      << name << "|" 
      << protocol_name << "|" 
      << host << "|" 
      << host << "|" 
      << username << "|" 
      << auth_username << "|" 
      << password << "|" 
      << timeout; 

  return str.str ();
}

const std::string Opal::Account::get_name () const
{
  return name;
}


const std::string Opal::Account::get_protocol_name () const
{
  return protocol_name;
}


const std::string Opal::Account::get_host () const
{
  return host;
}


const std::string Opal::Account::get_username () const
{
  return username;
}


const std::string Opal::Account::get_authentication_username () const
{
  return auth_username;
}


const std::string Opal::Account::get_password () const
{
  return password;
}


unsigned Opal::Account::get_timeout () const
{
  return timeout;
}


void Opal::Account::enable ()
{
  enabled = true;

  account_core->subscribe_account (*this);

  updated.emit ();
  trigger_saving.emit ();
}


void Opal::Account::disable ()
{
  enabled = false;

  account_core->unsubscribe_account (*this);

  updated.emit ();
  trigger_saving.emit ();
}


bool Opal::Account::is_enabled () const
{
  return enabled;
}

    
void Opal::Account::remove ()
{
}

bool Opal::Account::populate_menu (Ekiga::MenuBuilder &builder)
{
  if (enabled)
    builder.add_action ("disable", _("_Disable"),
                        sigc::mem_fun (this, &Opal::Account::disable));
  else
    builder.add_action ("enable", _("_Enable"),
                        sigc::mem_fun (this, &Opal::Account::enable));

  builder.add_separator ();

  builder.add_action ("edit", _("_Edit"),
		      sigc::mem_fun (this, &Opal::Account::edit));
  builder.add_action ("remove", _("_Remove"),
		      sigc::mem_fun (this, &Opal::Account::remove));

  return true;
}


void Opal::Account::edit ()
{
  Ekiga::FormRequestSimple request;
  std::stringstream str;

  str << get_timeout ();
  
  request.title (_("Edit account"));

  request.instructions (_("Please update the following fields:"));

  request.text ("name", _("Name:"), get_name ());
  request.text ("host", _("Host"), get_host ());
  request.text ("user", _("User:"), get_username ());
  request.text ("authentication_user", _("Authentication User:"), get_authentication_username ());
  request.private_text ("password", _("Password:"), get_password ());
  request.text ("timeout", _("Timeout:"), str.str ());
  request.boolean ("enabled", _("Enable Account"), enabled);

  request.submitted.connect (sigc::mem_fun (this, &Opal::Account::on_edit_form_submitted));

  if (!questions.handle_request (&request)) {

    // FIXME: better error reporting
#ifdef __GNUC__
    std::cout << "Unhandled form request in "
	      << __PRETTY_FUNCTION__ << std::endl;
#endif
  }
}


void Opal::Account::on_edit_form_submitted (Ekiga::Form &result)
{
  try {

    Ekiga::FormRequestSimple request;

    std::string error;
    std::string new_name = result.text ("name");
    std::string new_host = result.text ("host");
    std::string new_user = result.text ("user");
    std::string new_authentication_user = result.text ("authentication_user");
    std::string new_password = result.private_text ("password");
    bool new_enabled = result.boolean ("enabled");
    unsigned new_timeout = atoi (result.text ("timeout").c_str ());

    result.visit (request);

    if (new_name.empty ()) 
      error = _("You did not supply a name for that account.");
    else if (new_host.empty ()) 
      error = _("You did not supply a host to register to.");
    else if (new_user.empty ())
      error = _("You did not supply a user name for that account.");

    if (!error.empty ()) {
      request.error (error);
      request.submitted.connect (sigc::mem_fun (this, &Opal::Account::on_edit_form_submitted));

      if (!questions.handle_request (&request)) {
#ifdef __GNUC__
        std::cout << "Unhandled form request in "
          << __PRETTY_FUNCTION__ << std::endl;
#endif
      }
    }
    else {

      disable ();
      name = new_name;
      host = new_host;
      username = new_user;
      auth_username = new_authentication_user;
      password = new_password;
      timeout = new_timeout;
      enabled = new_enabled;
      enable ();

      updated.emit ();
      trigger_saving.emit ();
    }

  } catch (Ekiga::Form::not_found) {

    std::cerr << "Invalid result form" << std::endl; // FIXME: do better
  }
}
