
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
#include <lib/dialog.h>

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
  GtkWidget *druid_window = NULL;

  GMH323EndPoint *endpoint = NULL;
  PSTUNClient *stun = NULL;

  BOOL regist = FALSE;
  
  endpoint = GnomeMeeting::Process ()->Endpoint ();
  history_window = GnomeMeeting::Process ()->GetHistoryWindow ();
  druid_window = GnomeMeeting::Process ()->GetDruidWindow ();

  gchar *prefered_method = NULL;
  gchar *primary_text = NULL;
  gchar *dialog_text = NULL;
  gboolean stun_dialog = FALSE;

  GtkWidget *dialog = NULL;
  GtkWidget *dialog_label = NULL;

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

  if (!regist && reg) {

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

      nat_type = name [stun->GetNatType ()];
      gnomemeeting_threads_enter ();
      gm_history_window_insert (history_window, _("Set STUN server to %s (%s)"), (const char *) stun_host, (const char *) nat_type);
      gnomemeeting_threads_leave ();
    }
  } 
  /* Only detects and configure */
  else if (!reg && !stun_host.IsEmpty ()) {

    PSTUNClient stun (stun_host,
		      endpoint->GetUDPPortBase(), 
		      endpoint->GetUDPPortMax(),
		      endpoint->GetRtpIpPortBase(), 
		      endpoint->GetRtpIpPortMax());

    nat_type = name [stun.GetNatType ()];

    switch (stun.GetNatType ())
      {
      case 0:
      case 7:
      case 8:
	prefered_method = g_strdup_printf (_("STUN test result: %s.\n\nGnomeMeeting could not detect the type of NAT you are using. The most appropriate method, if your router does not natively support H.323, is probably to forward the required ports to your internal machine and use IP translation if you are behind a NAT router. Please also make sure you are not running a local firewall."), (const char *) nat_type);
	stun_dialog = FALSE;
	break;
	
      case 1:
      case 6:
	prefered_method = g_strdup_printf (_("STUN test result: %s.\n\nYour system does not need any specific configuration as long as you do not have a local firewall blocking the ports required by GnomeMeeting."), (const char *) nat_type);
	stun_dialog = FALSE;
	break;

      case 5:
	prefered_method = g_strdup_printf (_("GnomeMeeting detected Symmetric NAT. The most appropriate method, if your router does not natively support H.323, is to forward the required ports to your internal machine in order to change your Symmetric NAT into Cone NAT. Running this test again after the port forwarding has been done should report Cone NAT and allow GnomeMeeting to be used with STUN support enabled. If it does not report Cone NAT, then it means that there is a problem in your forwarding rules."));
	stun_dialog = FALSE;
	break;
	
      default:
	prefered_method = g_strdup_printf (_("STUN test result: %s.\n\nUsing a STUN server is most probably the most appropriate method if your router does not natively support H.323.\nNotice that STUN support is not sufficient if you want to contact H.323 clients that do not support H.245 Tunneling like Netmeeting. In that case you will have to use the classical IP translation and port forwarding.\n\nEnable STUN Support?"), (const char *) nat_type);
	stun_dialog = TRUE;
	break;
      }

    if (stun_dialog) {
      
      gnomemeeting_threads_enter ();
      dialog = 
	gtk_dialog_new_with_buttons (_("NAT Detection Successfull"),
				     GTK_WINDOW (druid_window),
				     GTK_DIALOG_MODAL,
				     GTK_STOCK_NO,
				     GTK_RESPONSE_NO,
				     GTK_STOCK_YES,
				     GTK_RESPONSE_YES,
				     NULL);
      gnomemeeting_threads_leave ();
    }
    else {

      gnomemeeting_threads_enter ();
      dialog = 
	gtk_dialog_new_with_buttons (_("NAT Detection Successfull"),
				     GTK_WINDOW (druid_window),
				     GTK_DIALOG_MODAL,
				     GTK_STOCK_OK,
				     GTK_RESPONSE_ACCEPT,
				     NULL);
      gnomemeeting_threads_leave ();
    }
    
    primary_text =
      g_strdup_printf ("<span weight=\"bold\" size=\"larger\">%s</span>",
		       _("The type of NAT was successfully detected"));

    dialog_text =
      g_strdup_printf ("%s\n\n%s", primary_text, prefered_method);

    gnomemeeting_threads_enter ();
    gtk_window_set_title (GTK_WINDOW (dialog), "");
    dialog_label = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (dialog_label),
			  dialog_text);
    gtk_label_set_line_wrap (GTK_LABEL (dialog_label), TRUE);
    gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox),
		       dialog_label);
    gtk_widget_show (dialog_label);
    gnomemeeting_threads_dialog_show (dialog);

    switch (gtk_dialog_run (GTK_DIALOG (dialog))) {

    case GTK_RESPONSE_YES:

      gm_conf_set_string (NAT_KEY "stun_server", "stun.voxgratia.org");
      gm_conf_set_bool (NAT_KEY "enable_stun_support", TRUE);
      
      ((H323EndPoint *) endpoint)->SetSTUNServer ("stun.voxgratia.org");
      
      gm_history_window_insert (history_window, _("Set STUN server to %s"), 
				"stun.voxgratia.org");
      
      break;

    case GTK_RESPONSE_NO:
      
      gm_conf_set_bool (NAT_KEY "enable_stun_support", FALSE);
      
      ((H323EndPoint *) endpoint)->SetSTUNServer (PString ());

      gm_history_window_insert (history_window, _("Removed STUN server"));

      break;
    }
    gtk_widget_destroy (dialog);
    gnomemeeting_threads_leave ();

    g_free (primary_text);
    g_free (dialog_text);
    g_free (prefered_method);
  }
}
