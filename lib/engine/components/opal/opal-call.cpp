
/*
 * Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2009 Damien Sandras <dsandras@seconix.com>

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
 *                         opal-call.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Damien Sandras
 *   copyright            : (c) 2007 by Damien Sandras
 *   description          : declaration of the interface of a call handled by
 *                          Opal.
 *
 */


#include <cctype>
#include <algorithm>

#include <glib/gi18n.h>
#include <opal/opal.h>
#include <ep/pcss.h>
#include <sip/sippdu.h>

#include "call.h"
#include "opal-call.h"
#include "opal-call-manager.h"
#include "notification-core.h"
#include "call-core.h"
#include "runtime.h"

using namespace Opal;

static void
strip_special_chars (std::string& str, char* special_chars, bool start)
{
  std::string::size_type idx;

  unsigned i = 0;
  while (i < strlen (special_chars)) {
    idx = str.find_first_of (special_chars[i]);
    if (idx != std::string::npos) {
      if (start)
        str = str.substr (idx+1);
      else
        str = str.substr (0, idx);
    }
    i++;
  }
}

class CallSetup : public PThread
{
  PCLASSINFO(CallSetup, PThread);

public:
  CallSetup (Opal::Call & _call,
             OpalConnection & _connection)
    : PThread (1000, AutoDeleteThread),
      call (_call),
      connection (_connection)
  {
    this->Resume ();
  }

  void Main ()
  {
    call.DoSetUp (connection);
  }

private:
  Opal::Call & call;
  OpalConnection & connection;
};


Opal::Call::Call (Opal::CallManager& _manager,
		  const std::string& uri)
  : OpalCall (_manager), Ekiga::Call (), manager(_manager), remote_uri (uri),
    call_setup(false), jitter(0), outgoing(false)
{
  re_a_bytes = tr_a_bytes = re_v_bytes = tr_v_bytes = 0.0;
  last_v_tick = last_a_tick = PTime ();
  total_a =
    total_v =
    lost_a =
    too_late_a =
    out_of_order_a =
    lost_v =
    too_late_v =
    out_of_order_v = 0;
  lost_packets = late_packets = out_of_order_packets = 0.0;
  re_a_bw = tr_a_bw = re_v_bw = tr_v_bw = 0.0;

  NoAnswerTimer.SetNotifier (PCREATE_NOTIFIER (OnNoAnswerTimeout));

  add_action (Ekiga::ActionPtr (new Ekiga::Action ("hangup", _("Hangup"),
                                                   boost::bind (&Call::hang_up, this))));
  if (!is_outgoing () && !IsEstablished ()) {
    add_action (Ekiga::ActionPtr (new Ekiga::Action ("answer", _("Answer"),
                                                     boost::bind (&Call::answer, this))));
    add_action (Ekiga::ActionPtr (new Ekiga::Action ("reject", _("Reject"),
                                                     boost::bind (&Call::hang_up, this))));
  }
}

Opal::Call::~Call ()
{
  remove_action ("hangup");
  remove_action ("reject");
  remove_action ("answer");
  remove_action ("transfer");
  remove_action ("hold");
  remove_action ("transmit-video");
}

void
Opal::Call::set_engine_services (boost::shared_ptr<Ekiga::NotificationCore> _notification_core,
				 boost::shared_ptr<Ekiga::CallCore> _call_core)
{
  notification_core = _notification_core;
  call_core = _call_core;
}

void
Opal::Call::hang_up ()
{
  if (!is_outgoing () && !IsEstablished ())
    Clear (OpalConnection::EndedByAnswerDenied);
  else
    Clear ();
}


void
Opal::Call::answer ()
{
  if (!is_outgoing () && !IsEstablished ()) {
    PSafePtr<OpalPCSSConnection> connection = GetConnectionAs<OpalPCSSConnection>();
    if (connection != NULL) {
      connection->AcceptIncoming ();
      remove_action ("reject");
      remove_action ("answer");
    }
  }
}


void
Opal::Call::transfer ()
{
  boost::shared_ptr<Ekiga::FormRequestSimple> request =
    boost::shared_ptr<Ekiga::FormRequestSimple> (new Ekiga::FormRequestSimple (boost::bind (&Opal::Call::on_transfer_form_submitted, this, _1, _2, _3)));

  request->title (_("Transfer Call"));
  request->action (_("Transfer"));
  request->text ("uri", _("Remote URI"),
                 "",
                 _("sip:username@ekiga.net"),
                 Ekiga::FormVisitor::STANDARD,
                 false, false);

  Ekiga::Call::questions (request);
}


void
Opal::Call::transfer (std::string uri)
{
  PSafePtr<OpalConnection> connection = get_remote_connection ();
  if (connection != NULL)
    connection->TransferConnection (uri);
}


bool
Opal::Call::on_transfer_form_submitted (bool submitted,
                                        Ekiga::Form& result,
                                        std::string& error)
{
  std::string::size_type idx;

  if (!submitted)
    return false;

  std::string uri = result.text ("uri");

  /* If the user did not provide a full URI,
   * use the same "domain" than the call local address
   */
  idx = uri.find_first_of ("@");
  if (idx == std::string::npos)
    uri = uri + "@" + (const char *) PURL (remote_uri).GetHostName ();
  if (manager.is_supported_uri (uri)) {

    transfer (uri);
    return true;
  }
  else
    error = _("You supplied an unsupported address");

  return false;
}


void
Opal::Call::toggle_hold ()
{
  bool on_hold = false;
  PSafePtr<OpalConnection> connection = get_remote_connection ();
  if (connection != NULL) {

    on_hold = connection->IsOnHold (false);
    connection->HoldRemote (!on_hold);
  }
}


void
Opal::Call::toggle_stream_pause (StreamType type)
{
  OpalMediaStreamPtr stream = NULL;
  PString codec_name;
  std::string stream_name;

  bool paused = false;

  PSafePtr<OpalConnection> connection = get_remote_connection ();
  if (connection != NULL) {

    stream = connection->GetMediaStream ((type == Audio) ? OpalMediaType::Audio () : OpalMediaType::Video (), false);
    if (stream != NULL) {

      stream_name = std::string ((const char *) stream->GetMediaFormat ().GetEncodingName ());
      std::transform (stream_name.begin (), stream_name.end (), stream_name.begin (), (int (*) (int)) toupper);
      paused = stream->IsPaused ();
      stream->SetPaused (!paused);

      if (paused)
	Ekiga::Runtime::run_in_main (boost::bind (boost::ref (stream_resumed), stream_name, type));
      else
	Ekiga::Runtime::run_in_main (boost::bind (boost::ref (stream_paused), stream_name, type));
    }
  }
}


void
Opal::Call::send_dtmf (const char dtmf)
{
  PSafePtr<OpalConnection> connection = get_remote_connection ();
  if (connection != NULL) {
    connection->SendUserInputTone (dtmf, 180);
  }
}


void Opal::Call::set_no_answer_forward (unsigned delay, const std::string & uri)
{
  forward_uri = uri;

  NoAnswerTimer.SetInterval (0, std::min (delay, (unsigned) 299));
}


void Opal::Call::set_reject_delay (unsigned delay)
{
  NoAnswerTimer.SetInterval (0, std::min (delay, (unsigned) 299));
}


const std::string
Opal::Call::get_id () const
{
  return GetToken ();
}


const std::string
Opal::Call::get_local_party_name () const
{
  return local_party_name;
}


const std::string
Opal::Call::get_remote_party_name () const
{
  return remote_party_name;
}


const std::string
Opal::Call::get_remote_application () const
{
  return remote_application;
}


const std::string
Opal::Call::get_remote_uri () const
{
  return remote_uri;
}


const std::string
Opal::Call::get_duration () const
{
  std::stringstream duration;

  if (start_time.IsValid () && IsEstablished ()) {

    PTimeInterval t = PTime () - start_time;

    duration << setfill ('0') << setw (2) << t.GetHours () << ":";
    duration << setfill ('0') << setw (2) << (t.GetMinutes () % 60) << ":";
    duration << setfill ('0') << setw (2) << (t.GetSeconds () % 60);
  }

  return duration.str ();
}


time_t
Opal::Call::get_start_time () const
{
  return start_time.GetTimeInSeconds ();
}


bool
Opal::Call::is_outgoing () const
{
  return outgoing;
}


// if the parameter is not valid utf8, remove from it all the chars
//   after the first invalid utf8 char, so that it becomes valid utf8
static void
make_valid_utf8 (string & str)
{
  const char *pos;
  if (!g_utf8_validate (str.c_str(), -1, &pos)) {
    PTRACE (4, "Ekiga\tTrimming invalid UTF-8 string: " << str.c_str());
    str = str.substr (0, pos - str.c_str()).append ("...");
  }
}


void
Opal::Call::parse_info (OpalConnection & connection)
{
  char start_special_chars [] = "$";
  char end_special_chars [] = "([;=";

  std::string l_party_name;
  std::string r_party_name;
  std::string app;

  if (!PIsDescendant(&connection, OpalPCSSConnection)) {

    remote_uri = (const char *) connection.GetRemotePartyURL ();

    l_party_name = (const char *) connection.GetLocalPartyName ();
    r_party_name = (const char *) connection.GetRemotePartyName ();
    app = (const char *) connection.GetRemoteProductInfo ().AsString ();
    start_time = connection.GetConnectionStartTime ();
    if (!start_time.IsValid ())
      start_time = PTime ();

    if (!l_party_name.empty ())
      local_party_name = (const char *) SIPURL (l_party_name).GetUserName ();
    if (!r_party_name.empty ())
      remote_party_name = r_party_name;
    if (!app.empty ())
      remote_application = app;

    make_valid_utf8 (remote_party_name);
    make_valid_utf8 (remote_application);
    make_valid_utf8 (remote_uri);

    strip_special_chars (remote_party_name, end_special_chars, false);
    strip_special_chars (remote_application, end_special_chars, false);
    strip_special_chars (remote_uri, end_special_chars, false);

    strip_special_chars (remote_party_name, start_special_chars, true);
    strip_special_chars (remote_uri, start_special_chars, true);
  }
}


PBoolean
Opal::Call::OnEstablished (OpalConnection & connection)
{
  OpalRTPSession *session = NULL;
  OpalMediaStreamPtr stream;

  NoAnswerTimer.Stop (false);

  if (!PIsDescendant(&connection, OpalPCSSConnection)) {

    add_action (Ekiga::ActionPtr (new Ekiga::Action ("hold", _("Hold"),
                                                     boost::bind (&Call::toggle_hold, this))));
    add_action (Ekiga::ActionPtr (new Ekiga::Action ("transfer", _("Transfer"),
                                                     boost::bind (&Call::transfer, this))));
    remove_action ("answer");
    remove_action ("reject");

    parse_info (connection);
    Ekiga::Runtime::run_in_main (boost::bind (&Opal::Call::emit_established_in_main, this));
  }

  if (PIsDescendant(&connection, OpalRTPConnection)) {

    stream = connection.GetMediaStream (OpalMediaType::Audio (), false);
    if (stream != NULL) {

      session = (OpalRTPSession*)PDownCast (OpalRTPConnection, &connection)->GetMediaSession (stream->GetSessionID ());
      if (session) {

        session->SetIgnorePayloadTypeChanges (TRUE);
        session->SetRxStatisticsInterval(50);
        session->SetTxStatisticsInterval(50);
      }
    }

    stream = connection.GetMediaStream (OpalMediaType::Video (), false);
    if (stream != NULL) {

      session = (OpalRTPSession*)PDownCast (OpalRTPConnection, &connection)->GetMediaSession (stream->GetSessionID ());
      if (session) {

        session->SetIgnorePayloadTypeChanges (TRUE);
        session->SetRxStatisticsInterval(50);
        session->SetTxStatisticsInterval(50);
      }
    }
  }

  return OpalCall::OnEstablished (connection);
}


void
Opal::Call::OnReleased (OpalConnection & connection)
{
  parse_info (connection);

  OpalCall::OnReleased (connection);
}


void
Opal::Call::OnCleared ()
{
  std::string reason;

  NoAnswerTimer.Stop (false);

  // TODO find a better way than that
  while (!call_setup)
    PThread::Current ()->Sleep (100);

  OpalCall::OnCleared ();

    switch (GetCallEndReason ()) {

    case OpalConnection::EndedByLocalUser :
      reason = _("Local user cleared the call");
      break;
    case OpalConnection::EndedByNoAccept :
      reason = _("Local user rejected the call");
      break;
    case OpalConnection::EndedByAnswerDenied :
      reason = _("Local user rejected the call");
      break;
    case OpalConnection::EndedByRemoteUser :
      reason = _("Remote user cleared the call");
      break;
    case OpalConnection::EndedByRefusal :
      reason = _("Remote user rejected the call");
      break;
    case OpalConnection::EndedByCallerAbort :
      reason = _("Remote user has stopped calling");
      break;
    case OpalConnection::EndedByTransportFail :
      reason = _("Abnormal call termination");
      break;
    case OpalConnection::EndedByConnectFail :
      reason = _("Could not connect to remote host");
      break;
    case OpalConnection::EndedByGatekeeper :
    case OpalConnection::EndedByGkAdmissionFailed :
      reason = _("The Gatekeeper cleared the call");
      break;
    case OpalConnection::EndedByNoUser :
      reason = _("User not found");
      break;
    case OpalConnection::EndedByNoBandwidth :
      reason = _("Insufficient bandwidth");
      break;
    case OpalConnection::EndedByCapabilityExchange :
      reason = _("No common codec");
      break;
    case OpalConnection::EndedByCallForwarded :
      reason = _("Call forwarded");
      break;
    case OpalConnection::EndedBySecurityDenial :
      reason = _("Security check failed");
      break;
    case OpalConnection::EndedByLocalBusy :
      reason = _("Local user is busy");
      break;
    case OpalConnection::EndedByLocalCongestion :
      reason = _("Congested link to remote party");
      break;
    case OpalConnection::EndedByRemoteBusy :
      reason = _("Remote user is busy");
      break;
    case OpalConnection::EndedByRemoteCongestion :
      reason = _("Congested link to remote party");
      break;
    case OpalConnection::EndedByHostOffline :
      reason = _("Remote host is offline");
      break;
    case OpalConnection::EndedByTemporaryFailure :
      reason = _("User is offline");
      break;
    case OpalConnection::EndedByUnreachable :
      reason = _("Remote host not found");
      break;
    case OpalConnection::EndedByNoEndPoint :
    case OpalConnection::EndedByNoAnswer :
      reason = _("User is not available");
      break;
    case OpalConnection::EndedByOutOfService:
      reason = _("Service unavailable");  // this appears when 500 does not work
      break;
    case OpalConnection::EndedByCertificateAuthority:
      reason = _("Remote certificate not authenticated");
      break;
    case OpalConnection::EndedByQ931Cause:
    case OpalConnection::EndedByDurationLimit:
    case OpalConnection::EndedByInvalidConferenceID:
    case OpalConnection::EndedByNoDialTone:
    case OpalConnection::EndedByNoRingBackTone:
    case OpalConnection::EndedByAcceptingCallWaiting:
    case OpalConnection::EndedByMediaFailed:
    case OpalConnection::EndedByCallCompletedElsewhere:
    case OpalConnection::EndedByIllegalAddress:
    case OpalConnection::NumCallEndReasons:
    default:
      reason = _("Call completed");
    }

    if (IsEstablished () || is_outgoing ())
      Ekiga::Runtime::run_in_main (boost::bind (&Opal::Call::emit_cleared_in_main, this, reason));
    else
      Ekiga::Runtime::run_in_main (boost::bind (&Opal::Call::emit_missed_in_main, this));
}


OpalConnection::AnswerCallResponse
Opal::Call::OnAnswerCall (OpalConnection & connection,
			  const PString & caller)
{
  remote_party_name = (const char *) caller;

  parse_info (connection);

  if (manager.get_auto_answer ())
    return OpalConnection::AnswerCallNow;

  return OpalCall::OnAnswerCall (connection, caller);
}


PBoolean
Opal::Call::OnSetUp (OpalConnection & connection)
{
  outgoing = !IsNetworkOriginated ();
  parse_info (connection);

  call_setup = true;
  Ekiga::Runtime::run_in_main (boost::bind (&Opal::Call::emit_setup_in_main, this));

  new CallSetup (*this, connection);

  return true;
}


PBoolean
Opal::Call::OnAlerting (OpalConnection & connection)
{
  if (!PIsDescendant(&connection, OpalPCSSConnection))
    Ekiga::Runtime::run_in_main (boost::bind (&Opal::Call::emit_ringing_in_main, this));

  return OpalCall::OnAlerting (connection);
}


void
Opal::Call::OnHold (OpalConnection & /*connection*/,
                    bool /*from_remote*/,
                    bool on_hold)
{
  if (on_hold)
    Ekiga::Runtime::run_in_main (boost::bind (&Opal::Call::emit_held_in_main, this));
  else
    Ekiga::Runtime::run_in_main (boost::bind (&Opal::Call::emit_retrieved_in_main, this));
}


void
Opal::Call::OnOpenMediaStream (OpalMediaStream & stream)
{
  StreamType type = (stream.GetMediaFormat().GetMediaType() == OpalMediaType::Audio ()) ? Audio : Video;
  bool is_transmitting = false;
  std::string stream_name;

  stream_name = std::string ((const char *) stream.GetMediaFormat ().GetEncodingName ());
  std::transform (stream_name.begin (), stream_name.end (), stream_name.begin (), (int (*) (int)) toupper);
  is_transmitting = !stream.IsSource ();

  Ekiga::Runtime::run_in_main (boost::bind (boost::ref (stream_opened), stream_name, type, is_transmitting));

  if (type == Ekiga::Call::Video)
    add_action (Ekiga::ActionPtr (new Ekiga::Action ("transmit-video", _("Transmit Video"),
                                                     boost::bind (&Call::toggle_stream_pause, this, Ekiga::Call::Video))));
}


void
Opal::Call::OnClosedMediaStream (OpalMediaStream & stream)
{
  StreamType type = (stream.GetMediaFormat().GetMediaType() == OpalMediaType::Audio ()) ? Audio : Video;
  bool is_transmitting = false;
  std::string stream_name;

  stream_name = std::string ((const char *) stream.GetMediaFormat ().GetEncodingName ());
  std::transform (stream_name.begin (), stream_name.end (), stream_name.begin (), (int (*) (int)) toupper);
  is_transmitting = !stream.IsSource ();

  Ekiga::Runtime::run_in_main (boost::bind (boost::ref (stream_closed), stream_name, type, is_transmitting));
}


void
Opal::Call::OnRTPStatistics2 (const OpalConnection & /* connection */,
			     const OpalRTPSession & session)
{
  PWaitAndSignal m(stats_mutex); // The stats are computed from two different threads

  if (session.IsAudio ()) {

    PTimeInterval t = PTime () - last_a_tick;
    if (t.GetMilliSeconds () < 500)
      return;

    unsigned elapsed_seconds = max ((unsigned long) t.GetMilliSeconds (), (unsigned long) 1);
    double octets_received = session.GetOctetsReceived ();
    double octets_sent = session.GetOctetsSent ();

    re_a_bw = max ((octets_received - re_a_bytes) / elapsed_seconds, 0.0);
    tr_a_bw = max ((octets_sent - tr_a_bytes) / elapsed_seconds, 0.0);

    re_a_bytes = octets_received;
    tr_a_bytes = octets_sent;
    last_a_tick = PTime ();

    total_a = session.GetPacketsReceived ();
    lost_a = session.GetPacketsLost ();
    too_late_a = session.GetPacketsTooLate ();
    out_of_order_a = session.GetPacketsOutOfOrder ();

    jitter = session.GetJitterBufferSize () / max ((unsigned) session.GetJitterTimeUnits (), (unsigned) 8);
  }
  else {

    PTimeInterval t = PTime () - last_v_tick;
    if (t.GetMilliSeconds () < 500)
      return;

    unsigned elapsed_seconds = max ((unsigned long) t.GetMilliSeconds (), (unsigned long) 1);
    double octets_received = session.GetOctetsReceived ();
    double octets_sent = session.GetOctetsSent ();

    re_v_bw = max ((octets_received - re_v_bytes) / elapsed_seconds, 0.0);
    tr_v_bw = max ((octets_sent - tr_v_bytes) / elapsed_seconds, 0.0);

    re_v_bytes = octets_received;
    tr_v_bytes = octets_sent;
    last_v_tick = PTime ();

    total_v = session.GetPacketsReceived ();
    lost_v = session.GetPacketsLost ();
    too_late_v = session.GetPacketsTooLate ();
    out_of_order_v = session.GetPacketsOutOfOrder ();
  }

  lost_packets = (lost_a + lost_v) / max ((unsigned long)(total_a + total_v), (unsigned long) 1);
  late_packets = (too_late_a + too_late_v) / max ((unsigned long)(total_a + total_v), (unsigned long) 1);
  out_of_order_packets = (out_of_order_a + out_of_order_v) / max ((unsigned long)(total_a + total_v), (unsigned long) 1);
}


void
Opal::Call::DoSetUp (OpalConnection & connection)
{
  OpalCall::OnSetUp (connection);
}


void
Opal::Call::OnNoAnswerTimeout (PTimer &,
                               INT)
{
  if (!is_outgoing ()) {

    if (!forward_uri.empty ()) {

      PSafePtr<OpalConnection> connection = get_remote_connection ();
      if (connection != NULL)
        connection->ForwardCall (forward_uri);
    }
    else
      Clear (OpalConnection::EndedByNoAnswer);
  }
}

void
Opal::Call::emit_established_in_main ()
{
  established ();
}

void
Opal::Call::emit_missed_in_main ()
{
  boost::shared_ptr<Ekiga::CallCore> ccore = call_core.lock ();
  if (!ccore)
    return;

  std::stringstream msg;

  missed ();
  msg << _("Missed call from") << " " << get_remote_party_name ();
  boost::shared_ptr<Ekiga::Notification> notif (new Ekiga::Notification (Ekiga::Notification::Warning,
                                                                         _("Missed call"), msg.str (),
                                                                         _("Call"),
                                                                         boost::bind (&Ekiga::CallCore::dial, ccore,
                                                                                      get_remote_uri ())));
  notification_core->push_notification (notif);
}

void
Opal::Call::emit_cleared_in_main (const std::string reason)
{
  cleared (reason);
  removed ();
}

void
Opal::Call::emit_setup_in_main ()
{
  setup ();
}

void
Opal::Call::emit_ringing_in_main ()
{
  ringing ();
}

void
Opal::Call::emit_held_in_main ()
{
  held ();
}

void
Opal::Call::emit_retrieved_in_main ()
{
  retrieved ();
}
