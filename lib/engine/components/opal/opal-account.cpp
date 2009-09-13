
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2009 Damien Sandras <dsandras@seconix.com>
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
#include <algorithm>
#include <sstream>

#include <glib.h>
#include <glib/gi18n.h>
#include <ptlib.h>
#include <opal/opal.h>
#include <ptclib/guid.h>

#include "opal-account.h"
#include "form-request-simple.h"
#include "toolbox.h"

#include "presence-core.h"
#include "personal-details.h"
#include "audiooutput-core.h"

#include "sip-endpoint.h"

Opal::Account::Account (Ekiga::ServiceCore & _core,
                        const std::string & account)
  : core (_core)
{
  dead = false;
  state = Unregistered;
  message_waiting_number = 0;

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
      // Could be empty, it is the only field allowed to be empty
      if (password == " ")
        password = "";
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

  if (host == "ekiga.net")
    type = Account::Ekiga;
  else if (host == "sip.diamondcard.us")
    type = Account::DiamondCard;
  else if (protocol_name == "SIP")
    type = Account::SIP;
  else
    type = Account::H323;

  limited = (name.find ("%limit") != std::string::npos);
}


Opal::Account::Account (Ekiga::ServiceCore & _core,
                        Type t,
                        std::string _name,
                        std::string _host,
                        std::string _username,
                        std::string _auth_username,
                        std::string _password,
                        bool _enabled,
                        unsigned _timeout)
  : core (_core)
{
  dead = false;
  state = Unregistered;
  message_waiting_number = 0;
  enabled = _enabled;
  aid = (const char *) PGloballyUniqueID ().AsString ();
  name = _name;
  protocol_name = (t == H323) ? "H323" : "SIP";
  host = _host;
  username = _username;
  if (!_auth_username.empty())
    auth_username = _auth_username;
  else
    auth_username = _username;
  password = _password;
  timeout = _timeout;
  type = t;

  if (enabled)
    enable ();
}


Opal::Account::~Account ()
{
}


const std::string Opal::Account::as_string () const
{
  if (dead)
    return "";

  std::stringstream str;

  str << enabled << "|1|"
      << aid << "|"
      << name << "|"
      << protocol_name << "|"
      << host << "|"
      << host << "|"
      << username << "|"
      << auth_username << "|"
      << (password.empty () ? " " : password) << "|"
      << timeout;

  return str.str ();
}

const std::string Opal::Account::get_name () const
{
  return name;
}

const std::string
Opal::Account::get_status () const
{
  std::string result;
  if (message_waiting_number > 0) {

    gchar* str = NULL;
    /* translators : the result will look like :
     * "registered (with 2 voicemail messages)"
     */
    str = g_strdup_printf (ngettext ("%s (with %d voicemail message)",
				     "%s (with %d voicemail messages)",
				     message_waiting_number),
			   status.c_str (), message_waiting_number);
    result = str;
    g_free (str);
  } else {

    result = status;
  }

  return result;
}

const std::string Opal::Account::get_aor () const
{
  std::stringstream str;

  str << (protocol_name == "SIP" ? "sip:" : "h323:") << username;

  if (username.find ("@") == string::npos)
    str << "@" << host;

  return str.str ();
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


void Opal::Account::set_authentication_settings (const std::string & _username,
                                                 const std::string & _password)
{
  username = _username;
  auth_username = _username;
  password = _password;

  enable ();
}


void Opal::Account::enable ()
{
  enabled = true;

  boost::shared_ptr<Sip::EndPoint> endpoint = core.get<Sip::EndPoint> ("opal-sip-endpoint");
  endpoint->subscribe (*this);

  updated ();
  trigger_saving ();
}


void Opal::Account::disable ()
{
  enabled = false;

  boost::shared_ptr<Sip::EndPoint> endpoint = core.get<Sip::EndPoint> ("opal-sip-endpoint");
  endpoint->unsubscribe (*this);

  updated ();
  trigger_saving ();
}


bool Opal::Account::is_enabled () const
{
  return enabled;
}


bool Opal::Account::is_limited () const
{
  return limited;
}


bool Opal::Account::is_active () const
{
  return enabled;
}


void Opal::Account::remove ()
{
  enabled = false;
  dead = true;

  boost::shared_ptr<Sip::EndPoint> endpoint = core.get<Sip::EndPoint> ("opal-sip-endpoint");
  endpoint->unsubscribe (*this);

  trigger_saving ();

  removed ();
}


bool Opal::Account::populate_menu (Ekiga::MenuBuilder &builder)
{
  if (enabled)
    builder.add_action ("disable", _("_Disable"),
                        boost::bind (&Opal::Account::disable, this));
  else
    builder.add_action ("enable", _("_Enable"),
                        boost::bind (&Opal::Account::enable, this));

  builder.add_separator ();

  builder.add_action ("edit", _("_Edit"),
		      boost::bind (&Opal::Account::edit, this));
  builder.add_action ("remove", _("_Remove"),
		      boost::bind (&Opal::Account::remove, this));

  if (type == DiamondCard) {

    std::stringstream str;
    std::stringstream url;
    str << "https://www.diamondcard.us/exec/voip-login?accId=" << get_username () << "&pinCode=" << get_password () << "&spo=ekiga";

    builder.add_separator ();

    url.str ("");
    url << str.str () << "&act=rch";
    builder.add_action ("recharge",
			_("Recharge the account"),
                        boost::bind (&Opal::Account::on_consult, this, url.str ()));
    url.str ("");
    url << str.str () << "&act=bh";
    builder.add_action ("balance",
                        _("Consult the balance history"),
                        boost::bind (&Opal::Account::on_consult, this, url.str ()));
    url.str ("");
    url << str.str () << "&act=ch";
    builder.add_action ("history",
                        _("Consult the call history"),
                        boost::bind (&Opal::Account::on_consult, this, url.str ()));
  }

  return true;
}


void Opal::Account::edit ()
{
  boost::shared_ptr<Ekiga::FormRequestSimple> request = boost::shared_ptr<Ekiga::FormRequestSimple> (new Ekiga::FormRequestSimple (boost::bind (&Opal::Account::on_edit_form_submitted, this, _1, _2)));
  std::stringstream str;

  str << get_timeout ();

  request->title (_("Edit account"));

  request->instructions (_("Please update the following fields:"));

  request->text ("name", _("Name:"), get_name ());
  if (get_protocol_name () == "SIP")
    request->text ("host", _("Registrar:"), get_host ());
  else
    request->text ("host", _("Gatekeeper:"), get_host ());
  request->text ("user", _("User:"), get_username ());
  if (get_protocol_name () == "SIP")
    /* Translators:
     * SIP knows two usernames: The name for the client ("User") and the name
     * for the authentication procedure ("Authentication User") */
    request->text ("authentication_user", _("Authentication User:"), get_authentication_username ());
  request->private_text ("password", _("Password:"), get_password ());
  request->text ("timeout", _("Timeout:"), str.str ());
  request->boolean ("enabled", _("Enable Account"), enabled);

  questions (request);
}


void Opal::Account::on_edit_form_submitted (bool submitted,
					    Ekiga::Form &result)
{
  if (!submitted)
    return;

  std::string new_name = result.text ("name");
  std::string new_host = result.text ("host");
  std::string new_user = result.text ("user");
  std::string new_authentication_user;
  if (get_protocol_name () == "SIP")
    new_authentication_user = result.text ("authentication_user");
  if (new_authentication_user.empty ())
    new_authentication_user = new_user;
  std::string new_password = result.private_text ("password");
  bool new_enabled = result.boolean ("enabled");
  unsigned new_timeout = atoi (result.text ("timeout").c_str ());
  std::string error;

  if (new_name.empty ())
    error = _("You did not supply a name for that account.");
  else if (new_host.empty ())
    error = _("You did not supply a host to register to.");
  else if (new_user.empty ())
    error = _("You did not supply a user name for that account.");
  else if (new_timeout < 10)
    error = _("The timeout should have a bigger value.");

  if (!error.empty ()) {

    boost::shared_ptr<Ekiga::FormRequestSimple> request = boost::shared_ptr<Ekiga::FormRequestSimple> (new Ekiga::FormRequestSimple (boost::bind (&Opal::Account::on_edit_form_submitted, this, _1, _2)));
    result.visit (*request);
    request->error (error);

    questions (request);
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
    if (enabled)
      enable ();

    updated ();
    trigger_saving ();
  }
}


void
Opal::Account::on_consult (const std::string url)
{
  gm_open_uri (url.c_str ());
}

void
Opal::Account::handle_registration_event (RegistrationState state_,
					  const std::string info) const
{
  switch (state_) {

  case Registered:

    if (state != Registered) {

      status = _("Registered");
      boost::shared_ptr<Ekiga::PresenceCore> presence_core = core.get<Ekiga::PresenceCore> ("presence-core");
      boost::shared_ptr<Ekiga::PersonalDetails> personal_details = core.get<Ekiga::PersonalDetails> ("personal-details");
      if (presence_core && personal_details) {

	presence_core->publish (personal_details);
      }
      state = state_;
      updated ();
    } 
    break;

  case Unregistered:

    status = _("Unregistered");
    updated ();
    break;

  case UnregistrationFailed:

    status = _("Could not unregister");
    if (!info.empty ())
      status = status + " (" + info + ")";
    updated ();
    break;

  case RegistrationFailed:

    if (!limited) {
      limited = true;
      boost::shared_ptr<Sip::EndPoint> endpoint = core.get<Sip::EndPoint> ("opal-sip-endpoint");
      endpoint->subscribe (*this);
    } else {
      status = _("Could not register");
      if (!info.empty ())
        status = status + " (" + info + ")";
      updated ();
    }
    break;

  case Processing:

    status = _("Processing...");
    updated ();
  default:
    break;
  }

  state = state_;
}

void
Opal::Account::handle_message_waiting_information (const std::string info)
{
  std::string::size_type loc = info.find ("/", 0);

  if (loc != std::string::npos) {

    boost::shared_ptr<Ekiga::AudioOutputCore> audiooutput_core = core.get<Ekiga::AudioOutputCore> ("audiooutput-core");
    std::stringstream new_messages;
    new_messages << info.substr (0, loc);
    new_messages >> message_waiting_number;
    if (message_waiting_number > 0)
      audiooutput_core->play_event ("new_voicemail_sound");
    updated ();
  }
}

Opal::Account::Type
Opal::Account::get_type () const
{
  return type;
}
