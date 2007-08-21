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
 *                         sip-basic.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : code to provide basic SIP (call & text messages)
 *
 */

#include <iostream>

#include "config.h"

#include "sip-basic.h"

static bool
is_sip_address (const std::string uri)
{
  return (uri.find ("sip:") == 0);
}


SIP::Basic::~Basic ()
{
#ifdef __GNUC__
  std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
}


bool
SIP::Basic::populate_menu (Ekiga::Contact &contact,
			   Ekiga::MenuBuilder &builder)
{
  bool populated = false;

  std::map<std::string, std::string> uris = contact.get_uris ();

  for (std::map<std::string, std::string>::const_iterator iter
	 = uris.begin ();
       iter != uris.end ();
       iter++) {

    populated = populate_for_precision_and_uri (iter->first,
						iter->second, 
                                                builder);
  }

  return populated;
}


bool
SIP::Basic::populate_menu (const std::string uri,
			   Ekiga::MenuBuilder &builder)
{
  return populate_for_precision_and_uri ("", uri, builder);
}


void
SIP::Basic::on_call (std::string uri)
{
  ep.call (uri);
}

void
SIP::Basic::on_message (std::string uri)
{
  std::cout << "Ekiga should message " << uri << std::endl; 
}

bool
SIP::Basic::populate_for_precision_and_uri (const std::string precision,
                                            const std::string uri,
                                            Ekiga::MenuBuilder &builder)
{
  if (is_sip_address (uri)) {

    builder.add_action ("call", _("_Call"),
			sigc::bind (sigc::mem_fun (this, &SIP::Basic::on_call), uri));
    builder.add_action ("message", _("Send _Message"),
			sigc::bind (sigc::mem_fun (this, &SIP::Basic::on_message), uri));

    return true;
  } else
    return false;
}
