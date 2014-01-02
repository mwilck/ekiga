
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
#include <sstream>

#include <glib.h>
#include <glib/gi18n.h>

#include <ptlib.h>
#include <ptclib/guid.h>

#include <opal/opal.h>
#include <sip/sippres.h>

#include "opal-account.h"

#include "robust-xml.h"
#include "form-request-simple.h"
#include "menu-builder-tools.h"
#include "platform.h"

#include "opal-presentity.h"
#include "sip-endpoint.h"

// remove leading and trailing spaces and tabs (useful for copy/paste)
// also, if no protocol specified, add leading "sip:"
static std::string
canonize_uri (std::string uri)
{
  const size_t begin_str = uri.find_first_not_of (" \t");
  if (begin_str == std::string::npos)  // there is no content
    return "";

  const size_t end_str = uri.find_last_not_of (" \t");
  const size_t range = end_str - begin_str + 1;
  uri = uri.substr (begin_str, range);
  const size_t pos = uri.find (":");
  if (pos == std::string::npos)
    uri = uri.insert (0, "sip:");
  return uri;
}

xmlNodePtr
Opal::Account::build_node(Opal::Account::Type typus,
			  std::string name,
			  std::string host,
			  std::string user,
			  std::string auth_user,
			  std::string password,
			  bool enabled,
			  unsigned timeout)
{
  xmlNodePtr node = xmlNewNode (NULL, BAD_CAST "account");

  xmlNewChild (node, NULL, BAD_CAST "name",
	       BAD_CAST robust_xmlEscape (node->doc, name).c_str ());
  xmlNewChild (node, NULL, BAD_CAST "host",
	       BAD_CAST robust_xmlEscape (node->doc, host).c_str ());
  xmlNewChild (node, NULL, BAD_CAST "user",
	       BAD_CAST robust_xmlEscape (node->doc, user).c_str ());
  xmlNewChild (node, NULL, BAD_CAST "auth_user",
	       BAD_CAST robust_xmlEscape (node->doc, auth_user).c_str ());
  xmlNewChild (node, NULL, BAD_CAST "password",
	       BAD_CAST robust_xmlEscape (node->doc, password).c_str ());
  if (enabled) {

    xmlSetProp (node, BAD_CAST "enabled", BAD_CAST "true");
  } else {

    xmlSetProp (node, BAD_CAST "enabled", BAD_CAST "false");
  }
  {
    std::stringstream sstream;
    sstream << timeout;
    xmlSetProp (node, BAD_CAST "timeout", BAD_CAST sstream.str ().c_str ());
  }

  switch (typus) {

  case Ekiga:
    xmlSetProp (node, BAD_CAST "type", BAD_CAST "Ekiga");
    break;

  case DiamondCard:
    xmlSetProp (node, BAD_CAST "type", BAD_CAST "DiamondCard");
    break;

  case H323:
    xmlSetProp (node, BAD_CAST "type", BAD_CAST "H323");
    break;

  case SIP:
  default:
    xmlSetProp (node, BAD_CAST "type", BAD_CAST "SIP");
    break;
  }

  xmlNewChild(node, NULL, BAD_CAST "roster", NULL);

  return node;
}


Opal::Account::Account (boost::shared_ptr<Opal::Sip::EndPoint> _sip_endpoint,
			boost::weak_ptr<Ekiga::PresenceCore> _presence_core,
			boost::shared_ptr<Ekiga::NotificationCore> _notification_core,
			boost::shared_ptr<Ekiga::PersonalDetails> _personal_details,
			boost::shared_ptr<Ekiga::AudioOutputCore> _audiooutput_core,
			boost::shared_ptr<CallManager> _call_manager,
			boost::function0<std::set<std::string> > _existing_groups,
			xmlNodePtr _node):
  existing_groups(_existing_groups),
  node(_node),
  sip_endpoint(_sip_endpoint),
  presence_core(_presence_core),
  notification_core(_notification_core),
  personal_details(_personal_details),
  audiooutput_core(_audiooutput_core),
  call_manager(_call_manager)
{
  state = Unregistered;
  status = _("Unregistered");
  message_waiting_number = 0;
  failed_registration_already_notified = false;
  dead = false;

  decide_type ();

  if (type != Account::H323) {

    const std::string name = get_name ();
    if (name.find ("%limit") != std::string::npos)
      compat_mode = SIPRegister::e_CannotRegisterMultipleContacts;  // start registration in this compat mode
    else
      compat_mode = SIPRegister::e_FullyCompliant;
  }

  for (xmlNodePtr child = node->children; child != NULL; child = child->next) {

    if (child->type == XML_ELEMENT_NODE && child->name != NULL && xmlStrEqual (BAD_CAST "roster", child->name)) {

      roster_node = child;
      for (xmlNodePtr presnode = roster_node->children; presnode != NULL; presnode = presnode->next) {

	Opal::PresentityPtr pres(new Presentity (presence_core,
						 existing_groups,
						 presnode));

	pres->trigger_saving.connect (boost::ref (trigger_saving));
	pres->removed.connect (boost::bind (boost::ref (presentity_removed), pres));
	pres->updated.connect (boost::bind (boost::ref (presentity_updated), pres));
	add_object (pres);
	presentity_added (pres);
      }
    }
  }
  setup_presentity ();
}


std::set<std::string>
Opal::Account::get_groups () const
{
  std::set<std::string> result;

  for (const_iterator iter = begin (); iter != end (); ++iter) {

    std::set<std::string> groups = (*iter)->get_groups ();
    result.insert (groups.begin (), groups.end ());
  }

  return result;
}


const std::string
Opal::Account::get_name () const
{
  std::string result;
  xmlChar* xml_str = NULL;

  for (xmlNodePtr child = node->children; child != NULL; child = child->next) {

    if (child->type == XML_ELEMENT_NODE && child->name != NULL && xmlStrEqual (BAD_CAST "name", child->name)) {

      xml_str = xmlNodeGetContent (child);
      if (xml_str != NULL) {

	result = (const char*)xml_str;
	xmlFree (xml_str);
      } else {
	result = _("Unnamed");
      }
    }
  }

  return result;
}

const std::string
Opal::Account::get_status () const
{
  std::string result;
  if (message_waiting_number > 0) {

    gchar* str = NULL;
    /* translators : the result will look like :
     * "registered (with 2 voice mail messages)"
     */
    str = g_strdup_printf (ngettext ("%s (with %d voice mail message)",
                                     "%s (with %d voice mail messages)",
                                     message_waiting_number),
                           status.c_str (), message_waiting_number);
    result = str;
    g_free (str);
  } else {

    result = status;
  }

  return result;
}

const std::string
Opal::Account::get_aor () const
{
  std::stringstream str;

  str << (get_protocol_name () == "SIP" ? "sip:" : "h323:") << get_username ();

  if (get_username ().find ("@") == string::npos)
    str << "@" << get_host ();

  return str.str ();
}

const std::string
Opal::Account::get_protocol_name () const
{
  std::string result = "SIP";
  xmlChar* xml_str = xmlGetProp (node, BAD_CAST "type");

  if (xml_str != NULL) {

    result = (const char*)xml_str;
    xmlFree (xml_str);
  }

  return result;
}


const std::string
Opal::Account::get_host () const
{
  std::string result;
  xmlChar* xml_str = NULL;

  for (xmlNodePtr child = node->children; child != NULL; child = child->next) {

    if (child->type == XML_ELEMENT_NODE && child->name != NULL && xmlStrEqual (BAD_CAST "host", child->name)) {

      xml_str = xmlNodeGetContent (child);
      if (xml_str != NULL) {

	result = (const char*)xml_str;
	xmlFree (xml_str);
      }
    }
  }

  return result;
}


const std::string
Opal::Account::get_username () const
{
  std::string result;
  xmlChar* xml_str = NULL;

  for (xmlNodePtr child = node->children; child != NULL; child = child->next) {

    if (child->type == XML_ELEMENT_NODE && child->name != NULL && xmlStrEqual (BAD_CAST "user", child->name)) {

      xml_str = xmlNodeGetContent (child);
      if (xml_str != NULL) {

	result = (const char*)xml_str;
	xmlFree (xml_str);
      }
    }
  }

  return result;
}


const std::string
Opal::Account::get_authentication_username () const
{
  std::string result;
  xmlChar* xml_str = NULL;

  for (xmlNodePtr child = node->children; child != NULL; child = child->next) {

    if (child->type == XML_ELEMENT_NODE && child->name != NULL && xmlStrEqual (BAD_CAST "auth_user", child->name)) {

      xml_str = xmlNodeGetContent (child);
      if (xml_str != NULL) {

	result = (const char*)xml_str;
	xmlFree (xml_str);
      }
    }
  }

  return result;
}


const std::string
Opal::Account::get_password () const
{
  std::string result;
  xmlChar* xml_str = NULL;

  for (xmlNodePtr child = node->children; child != NULL; child = child->next) {

    if (child->type == XML_ELEMENT_NODE && child->name != NULL && xmlStrEqual (BAD_CAST "password", child->name)) {

      xml_str = xmlNodeGetContent (child);
      if (xml_str != NULL) {

	result = (const char*)xml_str;
	xmlFree (xml_str);
      }
    }
  }

  return result;
}


unsigned
Opal::Account::get_timeout () const
{
  unsigned result = 0;
  xmlChar* xml_str = xmlGetProp (node, BAD_CAST "timeout");

  if (xml_str != NULL) {

    result = std::strtoul ((const char*)xml_str, NULL, 0);
    xmlFree (xml_str);
  }

  return result;
}


void
Opal::Account::set_authentication_settings (const std::string& username,
					    const std::string& password)
{
  for (xmlNodePtr child = node->children; child != NULL; child = child->next) {

    if (child->type == XML_ELEMENT_NODE && child->name != NULL) {

      if (xmlStrEqual (BAD_CAST "user", child->name)) {

	robust_xmlNodeSetContent (node, &child, "user", username);
      }
      if (xmlStrEqual (BAD_CAST "auth_user", child->name)) {

	robust_xmlNodeSetContent (node, &child, "auth_user", username);
      }
      if (xmlStrEqual (BAD_CAST "password", child->name)) {

	robust_xmlNodeSetContent (node, &child, "password", password);
      }
    }
  }

  enable ();
}


void
Opal::Account::enable ()
{
  xmlSetProp (node, BAD_CAST "enabled", BAD_CAST "true");

  state = Processing;
  status = _("Processing...");
  call_manager->subscribe (*this, presentity);

  updated ();
  trigger_saving ();
}


void
Opal::Account::disable ()
{
  xmlSetProp (node, BAD_CAST "enabled", BAD_CAST "false");

  if (presentity) {

    for (std::set<std::string>::iterator iter = watched_uris.begin ();
         iter != watched_uris.end (); ++iter) {
      presentity->UnsubscribeFromPresence (PString (*iter));
      Ekiga::Runtime::run_in_main (boost::bind (&Opal::Account::presence_status_in_main, this, *iter, "unknown", ""));
    }
  }

  if (type == Account::H323)
    call_manager->unsubscribe (*this, presentity);
  else {

      call_manager->unsubscribe (*this, presentity);
      sip_endpoint->Unsubscribe (SIPSubscribe::MessageSummary, get_aor ());
    }

  // Translators: this is a state, not an action, i.e. it should be read as
  // "(you are) unregistered", and not as "(you have been) unregistered"
  status = _("Unregistered");

  updated ();
  trigger_saving ();
}


bool
Opal::Account::is_enabled () const
{
  bool result = false;
  xmlChar* xml_str = xmlGetProp (node, BAD_CAST "enabled");

  if (xml_str != NULL) {

    if (xmlStrEqual (xml_str, BAD_CAST "true")) {

      result = true;
    } else {

      result = false;
    }
    xmlFree (xml_str);
  }

  return result;
}


bool
Opal::Account::is_active () const
{
  if (!is_enabled ())
    return false;

  return (state == Registered);
}


SIPRegister::CompatibilityModes
Opal::Account::get_compat_mode () const
{
  return compat_mode;
}


void
Opal::Account::remove ()
{
  dead = true;
  if (state == Registered || state == Processing) {
    disable();
    return;
  }

  trigger_saving ();
  removed ();
}


bool
Opal::Account::populate_menu (Ekiga::MenuBuilder &builder)
{
  if (is_enabled ())
    builder.add_action ("user-offline", _("_Disable"),
                        boost::bind (&Opal::Account::disable, this));
  else
    builder.add_action ("user-available", _("_Enable"),
                        boost::bind (&Opal::Account::enable, this));

  builder.add_separator ();

  builder.add_action ("add", _("A_dd Contact"),
		      boost::bind (&Opal::Account::add_contact, this));

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

bool
Opal::Account::populate_menu (const std::string fullname,
			      const std::string uri,
			      Ekiga::MenuBuilder& builder)
{
  bool result = false;
  Ekiga::TemporaryMenuBuilder tmp_builder;
  std::string protocol;
  std::string complete_uri;

  if (!is_enabled ())
    return false;

  // if there is no protocol, add what we are
  if (uri.find (":") == string::npos) {

    if (type == H323)
      protocol = "h323:";
    else
      protocol = "sip:";
    complete_uri = protocol + uri;
  } else
    complete_uri = uri;

  // whatever the protocol was previously, check if it fits
  if (not(
      (type == H323 && complete_uri.find ("h323:" != 0))
      ||
      (type != H323 && complete_uri.find ("sip:" != 0))
	  ))
    return false;

  // from now on, we're sure we have an uri corresponding to the account

  // but does it have a domain?
  //
  // (it is supposed not to, but let's still test so the function
  // can be called from several places without problem)
  if (complete_uri.find ("@") == string::npos && type != H323)
    complete_uri = complete_uri + "@" + get_host ();

  call_manager->populate_menu (fullname, complete_uri, tmp_builder);

  if ( !tmp_builder.empty ()) {

    builder.add_ghost ("", get_name ());
    tmp_builder.populate_menu (builder);
    result = true;
  }

  return result;

}

void
Opal::Account::edit ()
{
  boost::shared_ptr<Ekiga::FormRequestSimple> request = boost::shared_ptr<Ekiga::FormRequestSimple> (new Ekiga::FormRequestSimple (boost::bind (&Opal::Account::on_edit_form_submitted, this, _1, _2)));
  std::stringstream str;

  str << get_timeout ();

  request->title (_("Edit account"));

  request->instructions (_("Please update the following fields:"));

  request->text ("name", _("Name:"), get_name (), _("Account name, e.g. MyAccount"));
  if (get_protocol_name () == "SIP")
    request->text ("host", _("Registrar:"), get_host (), _("The registrar, e.g. ekiga.net"));
  else
    request->text ("host", _("Gatekeeper:"), get_host (), _("The gatekeeper, e.g. ekiga.net"));
  request->text ("user", _("User:"), get_username (), _("The user name, e.g. jim"));
  if (get_protocol_name () == "SIP")
    /* Translators:
     * SIP knows two usernames: The name for the client ("User") and the name
     * for the authentication procedure ("Authentication user") */
    request->text ("authentication_user", _("Authentication user:"), get_authentication_username (), _("The user name used during authentication, if different than the user name; leave empty if you do not have one"));
  request->private_text ("password", _("Password:"), get_password (), _("Password associated to the user"));
  request->text ("timeout", _("Timeout:"), str.str (), _("Time in seconds after which the account registration is automatically retried"));
  request->boolean ("enabled", _("Enable account"), is_enabled ());

  questions (request);
}


void
Opal::Account::on_edit_form_submitted (bool submitted,
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
  bool should_enable = false;
  bool should_disable = false;
  unsigned new_timeout = atoi (result.text ("timeout").c_str ());
  std::string error;

  if (new_name.empty ())
    error = _("You did not supply a name for that account.");
  else if (new_host.empty ())
    error = _("You did not supply a host to register to.");
  else if (new_user.empty ())
    error = _("You did not supply a user name for that account.");
  else if (new_timeout < 10)
    error = _("The timeout should be at least 10 seconds.");

  if (!error.empty ()) {

    boost::shared_ptr<Ekiga::FormRequestSimple> request = boost::shared_ptr<Ekiga::FormRequestSimple> (new Ekiga::FormRequestSimple (boost::bind (&Opal::Account::on_edit_form_submitted, this, _1, _2)));
    result.visit (*request);
    request->error (error);

    questions (request);
  }
  else {

    // Account was enabled and is now disabled
    // Disable it
    if (is_enabled () != new_enabled && !new_enabled) {
      should_disable = true;
    }
    // Account was disabled and is now enabled
    // or account was already enabled
    else if (new_enabled) {
      // Some critical setting just changed
      if (get_host () != new_host
	  || get_username () != new_user
          || get_authentication_username () != new_authentication_user
          || get_password () != new_password
          || get_timeout () != new_timeout
          || is_enabled () != new_enabled) {

        should_enable = true;
      }
    }

    if (new_enabled)
      xmlSetProp (node, BAD_CAST "enabled", BAD_CAST "true");
    else
      xmlSetProp (node, BAD_CAST "enabled", BAD_CAST "false");

    {
      std::stringstream sstream;
      sstream << new_timeout;
      xmlSetProp (node, BAD_CAST "timeout", BAD_CAST sstream.str ().c_str ());
    }

    for (xmlNodePtr child = node->children; child != NULL; child = child->next) {

      if (child->type == XML_ELEMENT_NODE && child->name != NULL) {


	if (xmlStrEqual (BAD_CAST "name", child->name))
	  robust_xmlNodeSetContent (node, &child, "name", new_name);
	if (xmlStrEqual (BAD_CAST "host", child->name))
	  robust_xmlNodeSetContent (node, &child, "host", new_host);
	if (xmlStrEqual (BAD_CAST "user", child->name))
	  robust_xmlNodeSetContent (node, &child, "user", new_user);
	if (xmlStrEqual (BAD_CAST "auth_user", child->name))
	  robust_xmlNodeSetContent (node, &child, "auth_user", new_authentication_user);
	if (xmlStrEqual (BAD_CAST "password", child->name))
	  robust_xmlNodeSetContent (node, &child, "password", new_password);
      }
    }

    decide_type ();

    if (should_enable)
      enable ();
    else if (should_disable)
      disable ();

    updated ();
    trigger_saving ();
  }
}

void
Opal::Account::add_contact ()
{
  boost::shared_ptr<Ekiga::PresenceCore> pcore = presence_core.lock ();
  if (!pcore)
    return;

  boost::shared_ptr<Ekiga::FormRequestSimple> request = boost::shared_ptr<Ekiga::FormRequestSimple> (new Ekiga::FormRequestSimple (boost::bind (&Opal::Account::on_add_contact_form_submitted, this, _1, _2)));
  std::set<std::string> groups = existing_groups ();

  request->title (_("Add to account roster"));
  request->instructions (_("Please fill in this form to add a new contact "
			   "to this account's roster"));
  request->text ("name", _("Name:"), "", _("Name of the contact, as shown in your roster"));

  request->text ("uri", _("Address:"), "sip:", _("Address, e.g. sip:xyz@ekiga.net; if you do not specify the host part, e.g. sip:xyz, then you can choose it by right-clicking on the contact in roster")); // let's put a default

  request->editable_set ("groups",
			 _("Put contact in groups:"),
			 std::set<std::string>(), groups);

  questions (request);
}

void
Opal::Account::on_add_contact_form_submitted (bool submitted,
					      Ekiga::Form& result)
{
  if (!submitted)
    return;

  boost::shared_ptr<Ekiga::PresenceCore> pcore = presence_core.lock ();
  if (!pcore)
    return;

  const std::string name = result.text ("name");
  std::string uri;
  const std::set<std::string> groups = result.editable_set ("groups");

  uri = result.text ("uri");
  uri = canonize_uri (uri);

  if (pcore->is_supported_uri (uri)) {

    xmlNodePtr presnode = Opal::Presentity::build_node (name, uri, groups);
    xmlAddChild (roster_node, presnode);
    trigger_saving ();

    Opal::PresentityPtr pres(new Presentity (presence_core, existing_groups, presnode));
    pres->trigger_saving.connect (boost::ref (trigger_saving));
    pres->removed.connect (boost::bind (boost::ref (presentity_removed), pres));
    pres->updated.connect (boost::bind (boost::ref (presentity_updated), pres));
    add_object (pres);
    presentity_added (pres);

  } else {

    boost::shared_ptr<Ekiga::FormRequestSimple> request = boost::shared_ptr<Ekiga::FormRequestSimple>(new Ekiga::FormRequestSimple (boost::bind (&Opal::Account::on_add_contact_form_submitted, this, _1, _2)));

    result.visit (*request);
    if (!pcore->is_supported_uri (uri))
      request->error (_("You supplied an unsupported address"));
    else
      request->error (_("You already have a contact with this address!"));

    questions (request);
  }
}

void
Opal::Account::on_consult (const std::string url)
{
  gm_platform_open_uri (url.c_str ());
}


bool
Opal::Account::is_myself (const std::string uri) const
{
  size_t pos = uri.find ("@");
  if (pos == string::npos)
    return false;

  std::string uri_host = uri.substr (++pos);
  if (uri_host != get_host ())
    return false;

  return true;
}


void
Opal::Account::publish (const Ekiga::PersonalDetails& details)
{
  std::string presence = details.get_presence ();

  if (presence == "available")
    personal_state = OpalPresenceInfo::Available;
  else if (presence == "away")
    personal_state = OpalPresenceInfo::Away;
  else if (presence == "busy")
    personal_state = OpalPresenceInfo::Busy;
  else {  // ekiga knows only these three presence types
    std::string s = "Warning: Unknown presence type ";
    s.append (presence);
    g_warning ("%s",s.data());
  }

  presence_status = details.get_status ();

  if (presentity) {
    presentity->SetLocalPresence (personal_state, presence_status);
    PTRACE (4, "Ekiga\tSent its own presence (publish) for " << get_aor() << ": " << presence << ", note " << presence_status);
  }
}


void
Opal::Account::fetch (const std::string uri)
{
  // Check if this is a presentity we watch
  if (!is_myself (uri))
    return;
  watched_uris.insert (uri);

  // Account is disabled, bye
  if (!is_enabled ())
    return;

  // Subscribe now
  if (state == Registered) {
    PTRACE(4, "Ekiga\tSubscribeToPresence for " << uri.c_str () << " (fetch)");
    presentity->SubscribeToPresence (PString (uri));
  }
}


void
Opal::Account::unfetch (const std::string uri)
{
  if (is_myself (uri) && presentity) {
    presentity->UnsubscribeFromPresence (PString (uri));
    watched_uris.erase (uri);
    Ekiga::Runtime::run_in_main (boost::bind (&Opal::Account::presence_status_in_main, this, uri, "unknown", ""));
  }
}


void
Opal::Account::handle_registration_event (RegistrationState state_,
					  const std::string info) const
{
  switch (state_) {

  case Registered:

    if (state != Registered) {

      // Translators: this is a state, not an action, i.e. it should be read as
      // "(you are) registered", and not as "(you have been) registered"
      status = _("Registered");
      if (presentity) {

        for (std::set<std::string>::iterator iter = watched_uris.begin ();
             iter != watched_uris.end (); ++iter) {
          PTRACE(4, "Ekiga\tSubscribeToPresence for " << iter->c_str () << " (Account Registered)");
          presentity->SubscribeToPresence (PString (*iter));
        }
        presentity->SetLocalPresence (personal_state, presence_status);
        if (type != Account::H323) {
          sip_endpoint->Subscribe (SIPSubscribe::MessageSummary, 3600, get_aor ());
        }
      }
      boost::shared_ptr<Ekiga::PersonalDetails> details = personal_details.lock ();
      if (details)
	const_cast<Account*>(this)->publish (*details);

      state = state_;
      failed_registration_already_notified = false;
      updated ();
    }
    break;

  case Unregistered:

    // Translators: this is a state, not an action, i.e. it should be read as
    // "(you are) unregistered", and not as "(you have been) unregistered"
    status = _("Unregistered");
    failed_registration_already_notified = false;
    state = state_;

    updated ();
    /* delay destruction of this account until the
       unsubscriber thread has called back */
    if (dead)
      removed ();
    break;

  case UnregistrationFailed:

    state = state_;
    status = _("Could not unregister");
    failed_registration_already_notified = false;
    if (!info.empty ())
      status = status + " (" + info + ")";
    updated ();
    break;

  case RegistrationFailed:

    state = state_;
    if (type == Account::H323) {
        std::stringstream msg;
        msg << _("Could not register to ") << get_name ();
        boost::shared_ptr<Ekiga::Notification> notif (new Ekiga::Notification (Ekiga::Notification::Warning, msg.str (), info, _("Edit"), boost::bind (&Opal::Account::edit, (Opal::Account*) this)));
	boost::shared_ptr<Ekiga::NotificationCore> ncore = notification_core.lock ();
	if (ncore)
	  ncore->push_notification (notif);
    }
    else {
      switch (compat_mode) {
      case SIPRegister::e_FullyCompliant:
        // FullyCompliant did not work, try next compat mode
        compat_mode = SIPRegister::e_CannotRegisterMultipleContacts;
        PTRACE (4, "Register failed in FullyCompliant mode, retrying in CannotRegisterMultipleContacts mode");
        call_manager->subscribe (*this, presentity);
        break;
      case SIPRegister::e_CannotRegisterMultipleContacts:
        // CannotRegMC did not work, try next compat mode
        compat_mode = SIPRegister::e_CannotRegisterPrivateContacts;
        PTRACE (4, "Register failed in CannotRegisterMultipleContacts mode, retrying in CannotRegisterPrivateContacts mode");
        call_manager->subscribe (*this, presentity);
        break;
      case SIPRegister::e_CannotRegisterPrivateContacts:
        // CannotRegPC did not work, try next compat mode
        compat_mode = SIPRegister::e_HasApplicationLayerGateway;
        PTRACE (4, "Register failed in CannotRegisterPrivateContacts mode, retrying in HasApplicationLayerGateway mode");
        call_manager->subscribe (*this, presentity);
        break;
      case SIPRegister::e_HasApplicationLayerGateway:
        // HasAppLG did not work, stop registration with error
        compat_mode = SIPRegister::e_FullyCompliant;
        PTRACE (4, "Register failed in HasApplicationLayerGateway mode, aborting registration");
        status = _("Could not register");
        if (!info.empty ())
          status = status + " (" + info + ")";
        if (!failed_registration_already_notified) {
          std::stringstream msg;
          msg << _("Could not register to ") << get_name ();
          boost::shared_ptr<Ekiga::Notification> notif (new Ekiga::Notification (Ekiga::Notification::Warning, msg.str (), info, _("Edit"), boost::bind (&Opal::Account::edit, (Opal::Account*) this)));
	boost::shared_ptr<Ekiga::NotificationCore> ncore = notification_core.lock ();
	if (ncore)
          ncore->push_notification (notif);
        }
        updated ();
        failed_registration_already_notified = true;
        break;
      default:

        state = state_;
        updated();
        break;
      }
    }
    break;

  case Processing:

    state = state_;
    status = _("Processing...");
    updated ();
  default:

    state = state_;
    updated();
    break;
  }
}


void
Opal::Account::handle_message_waiting_information (const std::string info)
{
  std::string::size_type loc = info.find ("/", 0);

  if (loc != std::string::npos) {

    std::stringstream new_messages;
    new_messages << info.substr (0, loc);
    new_messages >> message_waiting_number;
    if (message_waiting_number > 0) {
      boost::shared_ptr<Ekiga::AudioOutputCore> audiooutput = audiooutput_core.lock ();
      if (audiooutput)
	audiooutput->play_event ("new_voicemail_sound");
    }
    updated ();
  }
}


Opal::Account::Type
Opal::Account::get_type () const
{
  return type;
}

void
Opal::Account::setup_presentity ()
{
  PURL url = PString (get_aor ());
  presentity = call_manager->AddPresentity (url);

  if (presentity) {

    presentity->SetPresenceChangeNotifier (PCREATE_PresenceChangeNotifier (OnPresenceChange));
    presentity->GetAttributes().Set(OpalPresentity::AuthNameKey, get_authentication_username ());
    presentity->GetAttributes().Set(OpalPresentity::AuthPasswordKey, get_password ());
    if (type != H323) {
      presentity->GetAttributes().Set(SIP_Presentity::SubProtocolKey, "Agent");
    }
    PTRACE (4, "Created presentity for " << get_aor());
  } else
    PTRACE (4, "Error: cannot create presentity for " << get_aor());
}


void
Opal::Account::OnPresenceChange (OpalPresentity& /*presentity*/,
				 const OpalPresenceInfo& info)
{
  std::string new_presence;
  std::string new_status = "";

  SIPURL sip_uri = SIPURL (info.m_entity);
  sip_uri.Sanitise (SIPURL::ExternalURI);
  std::string uri = sip_uri.AsString ();
  PCaselessString note = info.m_note;

  PTRACE (4, "Ekiga\tReceived a presence change (notify) for " << info.m_entity << ": state " << info.m_state << ", note " << info.m_note);

  if (info.m_state == OpalPresenceInfo::Unchanged)
    return;

  if (!uri.compare (0, 5, "pres:"))
    uri.replace (0, 5, "sip:");  // replace "pres:" sith "sip:" FIXME

  new_status = (const char*) info.m_note;
  switch (info.m_state) {

  case OpalPresenceInfo::Unchanged:
    // do not change presence
    break;
  case OpalPresenceInfo::Available:
    new_presence = "available";
    if (!note.IsEmpty ()) {
      if (note.Find ("dnd") != P_MAX_INDEX
          || note.Find ("meeting") != P_MAX_INDEX
          || note.Find ("do not disturb") != P_MAX_INDEX
          || note.Find ("busy") != P_MAX_INDEX) {
        new_presence = "busy";
      }
      else if (note.Find ("away") != P_MAX_INDEX
               || note.Find ("out") != P_MAX_INDEX
               || note.Find ("vacation") != P_MAX_INDEX
               || note.Find ("holiday") != P_MAX_INDEX
               || note.Find ("lunch") != P_MAX_INDEX) {
        new_presence = "away";
      }
      else if (note.Find ("phone") != P_MAX_INDEX
               || note.Find ("ringing") != P_MAX_INDEX
               || note.Find ("call") != P_MAX_INDEX) {
        new_presence = "inacall";
      }
    }
    break;
  case OpalPresenceInfo::NoPresence:
    new_presence = "offline";
    break;
  case OpalPresenceInfo::InternalError:
  case OpalPresenceInfo::Forbidden:
  case OpalPresenceInfo::Unavailable:
  case OpalPresenceInfo::UnknownExtended:
    new_presence = "unknown";
    break;
  case OpalPresenceInfo::Away:
    new_presence = "away";
    break;
  case OpalPresenceInfo::Busy:
    new_presence = "busy";
    break;
  case OpalPresenceInfo::Appointment:
    new_presence = "away";
    // Translators: see RFC 4480 for more information about activities
    if (new_status.empty ())
      new_status = _("Appointment");
    break;
  case OpalPresenceInfo::Breakfast:
    new_presence = "away";
    if (new_status.empty ())
      new_status = _("Breakfast");
    break;
  case OpalPresenceInfo::Dinner:
    new_presence = "away";
    if (new_status.empty ())
      new_status = _("Dinner");
    break;
  case OpalPresenceInfo::Vacation:
  case OpalPresenceInfo::Holiday:
    new_presence = "away";
    if (new_status.empty ())
      new_status = _("Holiday");
    break;
  case OpalPresenceInfo::InTransit:
    new_presence = "away";
    if (new_status.empty ())
      new_status = _("In transit");
    break;
  case OpalPresenceInfo::LookingForWork:
    new_presence = "away";
    if (new_status.empty ())
      new_status = _("Looking for work");
    break;
  case OpalPresenceInfo::Lunch:
    new_presence = "away";
    if (new_status.empty ())
      new_status = _("Lunch");
    break;
  case OpalPresenceInfo::Meal:
    new_presence = "away";
    if (new_status.empty ())
      new_status = _("Meal");
    break;
  case OpalPresenceInfo::Meeting:
    new_presence = "away";
    if (new_status.empty ())
      new_status = _("Meeting");
    break;
  case OpalPresenceInfo::OnThePhone:
    new_presence = "inacall";
    if (new_status.empty ())
      new_status = _("On the phone");
    break;
  case OpalPresenceInfo::Playing:
    new_presence = "away";
    if (new_status.empty ())
      new_status = _("Playing");
    break;
  case OpalPresenceInfo::Shopping:
    new_presence = "away";
    if (new_status.empty ())
      new_status = _("Shopping");
    break;
  case OpalPresenceInfo::Sleeping:
    new_presence = "away";
    if (new_status.empty ())
      new_status = _("Sleeping");
    break;
  case OpalPresenceInfo::Working:
    new_presence = "busy";
    if (new_status.empty ())
      new_status = _("Working");
    break;
  case OpalPresenceInfo::Other:
  case OpalPresenceInfo::Performance:
  case OpalPresenceInfo::PermanentAbsence:
  case OpalPresenceInfo::Presentation:
  case OpalPresenceInfo::Spectator:
  case OpalPresenceInfo::Steering:
  case OpalPresenceInfo::Travel:
  case OpalPresenceInfo::TV:
  case OpalPresenceInfo::Worship:
    new_presence = "away";
    break;
  default:
    break;
  }

  Ekiga::Runtime::run_in_main (boost::bind (&Opal::Account::presence_status_in_main, this, uri, new_presence, new_status));
}


void
Opal::Account::presence_status_in_main (std::string uri,
					std::string uri_presence,
					std::string uri_status)
{
  presence_received (uri, uri_presence);
  status_received (uri, uri_status);
}


void
Opal::Account::visit_presentities (boost::function1<bool, Ekiga::PresentityPtr > visitor) const
{
  visit_objects (visitor);
}


bool
Opal::Account::populate_menu_for_group (const std::string name,
					Ekiga::MenuBuilder& builder)
{
  builder.add_action ("edit", _("Rename"),
		      boost::bind (&Opal::Account::on_rename_group, this, name));
  return true;
}


void
Opal::Account::on_rename_group (std::string name)
{
  boost::shared_ptr<Ekiga::FormRequestSimple> request = boost::shared_ptr<Ekiga::FormRequestSimple> (new Ekiga::FormRequestSimple (boost::bind (&Opal::Account::rename_group_form_submitted, this, name, _1, _2)));

  request->title (_("Rename group"));
  request->instructions (_("Please edit this group name"));
  request->text ("name", _("Name:"), name, std::string ());

  questions (request);
}


struct rename_group_form_submitted_helper
{
  rename_group_form_submitted_helper (const std::string old_name_,
				      const std::string new_name_):
    old_name(old_name_),
    new_name(new_name_)
  {}

  const std::string old_name;
  const std::string new_name;

  bool operator() (Ekiga::PresentityPtr pres)
  {
    Opal::PresentityPtr presentity = boost::dynamic_pointer_cast<Opal::Presentity> (pres);
    if (presentity)
      presentity->rename_group (old_name, new_name);
    return true;
  }
};


void
Opal::Account::rename_group_form_submitted (std::string old_name,
					    bool submitted,
					    Ekiga::Form& result)
{
  if (!submitted)
    return;

  const std::string new_name = result.text ("name");

  if ( !new_name.empty () && new_name != old_name) {

    rename_group_form_submitted_helper helper (old_name, new_name);
    visit_presentities (boost::ref (helper));
  }
}

void
Opal::Account::decide_type ()
{
  const std::string host = get_host ();

  if (host == "ekiga.net")
    type = Account::Ekiga;
  else if (host == "sip.diamondcard.us")
    type = Account::DiamondCard;
  else if (get_protocol_name () == "SIP")
    type = Account::SIP;
  else
    type = Account::H323;
}
