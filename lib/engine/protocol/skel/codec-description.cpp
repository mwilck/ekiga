
/*
 * Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2008 Damien Sandras

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
 *                         codec-description.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in January 2008 by Damien Sandras 
 *   copyright            : (c) 2008 by Damien Sandras
 *   description          : declaration of the interface of a codec description.
 *
 */

#include <iostream>
#include <sstream>

#include "codec-description.h"


using namespace Ekiga;

CodecDescription::CodecDescription ()
: rate (0), active (false), audio (false)
{
}


CodecDescription::CodecDescription (std::string _name,
                                    unsigned _rate,
                                    bool _audio,
                                    std::string _protocols,
                                    bool _active)
: name (_name), rate (_rate), active (_active), audio (_audio)
{
  char *pch = NULL;

  pch = strtok ((char *) _protocols.c_str (), ",");
  while (pch != NULL) {

    std::string protocol = pch;
    protocol = protocol.substr (protocol.find_first_not_of (" "));
    protocols.push_back (protocol);
    pch = strtok (NULL, ",");
  }

  protocols.unique ();
  protocols.sort ();
}


CodecDescription::CodecDescription (std::string codec)
{
  int i = 0;
  char *pch = NULL;

  std::string tmp [5];

  pch = strtok ((char *) codec.c_str (), "*");
  while (pch != NULL) {

    tmp [i] = pch;
    pch = strtok (NULL, "*");

    i++;
  }

  if (i < 4)
    return;

  pch = strtok ((char *) tmp [3].c_str (), " ");
  while (pch != NULL) {

    protocols.push_back (pch);
    pch = strtok (NULL, " ");
  }

  name = tmp [0];
  rate = atoi (tmp [1].c_str ());
  audio = atoi (tmp [2].c_str ());
  active = atoi (tmp [4].c_str ());
}


std::string CodecDescription::str ()
{
  std::stringstream val;
  std::stringstream proto;

  val << name << "*" << rate << "*" << audio << "*";
  protocols.sort ();
  for (std::list<std::string>::iterator iter = protocols.begin ();
       iter != protocols.end ();
       iter++) {

    if (iter != protocols.begin ())
      proto << " ";

    proto << *iter;
  }
  val << proto.str () << "*" << (active ? "1" : "0");

  return val.str ();
}
