
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
 *                         callinfo.cpp  -  description
 *                         ----------------------------
 *   begin                : Sun Sep 09 2007
 *   copyright            : (C) 2000-2007 by Damien Sandras
 *   description          : This file contains a class containing information
 *                          about a call.
 *
 */

#include "config.h"


#include "callinfo.h"


std::string
Ekiga::CallInfo::get_remote_party_name ()
{
  char special_chars [] = "([@";
  int i = 0;
  std::string::size_type idx;
  std::string remote_party_name = (const char *) connection.GetRemotePartyName ();

  while (i < 3) {

    idx = remote_party_name.find_first_of (special_chars [i]);
    if (idx != std::string::npos)
      remote_party_name = remote_party_name.substr (0, idx);

    i++;
  }

  return remote_party_name;
}


std::string
Ekiga::CallInfo::get_remote_application ()
{
  char special_chars [] = "([@";
  int i = 0;
  std::string::size_type idx;
  std::string remote_application = (const char *) connection.GetRemoteApplication (); 

  while (i < 3) {

    idx = remote_application.find_first_of (special_chars [i]);
    if (idx != std::string::npos)
      remote_application = remote_application.substr (0, idx);
    
    i++;
  }

  return remote_application;
}


std::string
Ekiga::CallInfo::get_remote_uri ()
{
  std::string remote_uri = (const char *) connection.GetRemotePartyCallbackURL ();

  return remote_uri;
}


std::string
Ekiga::CallInfo::get_call_end_reason ()
{
  std::string call_end_reason;
  gchar *end_reason = NULL;

  if (connection.GetPhase () < OpalConnection::ReleasedPhase)
    return call_end_reason;

  switch (connection.GetCallEndReason ()) {

  case OpalConnection::EndedByLocalUser :
    end_reason = g_strdup (_("Local user cleared the call"));
    break;
  case OpalConnection::EndedByNoAccept :
    end_reason = g_strdup (_("Local user rejected the call"));
    break;
  case OpalConnection::EndedByAnswerDenied :
    end_reason = g_strdup (_("Local user rejected the call"));
    break;
  case OpalConnection::EndedByRemoteUser :
    end_reason = g_strdup (_("Remote user cleared the call"));
    break;
  case OpalConnection::EndedByRefusal :
    end_reason = g_strdup (_("Remote user rejected the call"));
    break;
  case OpalConnection::EndedByCallerAbort :
    end_reason = g_strdup (_("Remote user has stopped calling"));
    break;
  case OpalConnection::EndedByTransportFail :
    end_reason = g_strdup (_("Abnormal call termination"));
    break;
  case OpalConnection::EndedByConnectFail :
    end_reason = g_strdup (_("Could not connect to remote host"));
    break;
  case OpalConnection::EndedByGatekeeper :
    end_reason = g_strdup (_("The Gatekeeper cleared the call"));
    break;
  case OpalConnection::EndedByNoUser :
    end_reason = g_strdup (_("User not found"));
    break;
  case OpalConnection::EndedByNoBandwidth :
    end_reason = g_strdup (_("Insufficient bandwidth"));
    break;
  case OpalConnection::EndedByCapabilityExchange :
    end_reason = g_strdup (_("No common codec"));
    break;
  case OpalConnection::EndedByCallForwarded :
    end_reason = g_strdup (_("Call forwarded"));
    break;
  case OpalConnection::EndedBySecurityDenial :
    end_reason = g_strdup (_("Security check failed"));
    break;
  case OpalConnection::EndedByLocalBusy :
    end_reason = g_strdup (_("Local user is busy"));
    break;
  case OpalConnection::EndedByLocalCongestion :
    end_reason = g_strdup (_("Congested link to remote party"));
    break;
  case OpalConnection::EndedByRemoteBusy :
    end_reason = g_strdup (_("Remote user is busy"));
    break;
  case OpalConnection::EndedByRemoteCongestion :
    end_reason = g_strdup (_("Congested link to remote party"));
    break;
  case OpalConnection::EndedByHostOffline :
    end_reason = g_strdup (_("Remote host is offline"));
    break;
  case OpalConnection::EndedByTemporaryFailure :
  case OpalConnection::EndedByUnreachable :
  case OpalConnection::EndedByNoEndPoint :
  case OpalConnection::EndedByNoAnswer :
    if (connection.IsOriginating ())
      end_reason = g_strdup (_("Remote user is not available at this time"));
    else
      end_reason = g_strdup (_("Local user is not available at this time"));
    break;

  default :
    end_reason = g_strdup (_("Call completed"));
  }

  call_end_reason = end_reason;
  g_free (end_reason);

  return call_end_reason;
}


int
Ekiga::CallInfo::get_call_duration ()
{
  return 3600;
}

Ekiga::CallInfo::CallType
Ekiga::CallInfo::get_call_type ()
{
  return type;
}
