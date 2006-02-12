
/* GnomeMeeting -- A Video-Conferencing application
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
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : Multithreaded class for the stun client.
 *
 */


#include "../../config.h" 

#include "stun.h"
#include "ekiga.h"
#include "manager.h"
#include "history.h"
#include "misc.h"

#include "gmconf.h"
#include "gmdialog.h"

#include <ptclib/http.h>
#include <ptclib/html.h>
#include <ptclib/pstun.h>


/* Declarations */

/* GUI Functions */
/* DESCRIPTION  :  /
 * BEHAVIOR     :  Create a dialog that presents the result of the STUN test.
 * 		   If the NAT type permits it, ask to the user if he wants
 * 		   to enable STUN support or not.
 * PRE          :  /
 */
static GtkWidget *
gm_sw_stun_result_window_new (GtkWidget *parent,
			      int nat_type);


/* Callbacks */

/* DESCRIPTION  :  This callback is called when the user validates an answer
 *                 to the stun dialog.
 * BEHAVIOR     :  Destroy the dialog and set/unset the STUN server.
 * PRE          :  /
 */
static void stun_dialog_response_cb (GtkDialog *dialog, 
				     gint response,
				     gpointer data);


/* Implementation */
static void 
stun_dialog_response_cb (GtkDialog *dialog, 
			 gint response,
			 gpointer data)
{
  GMManager *ep = NULL;

  GtkWidget *history_window = NULL;

  
  ep = GnomeMeeting::Process ()->GetManager ();
  history_window = GnomeMeeting::Process ()->GetHistoryWindow ();
  
  switch (response) {

  case GTK_RESPONSE_YES:

    gm_conf_set_string (NAT_KEY "stun_server", "stun.ekiga.net");
    gm_conf_set_int (NAT_KEY "method", 1);

    gm_history_window_insert (history_window, 
			      _("STUN server set to %s"), 
			      "stun.ekiga.net");

    break;

  case GTK_RESPONSE_NO:

    ((OpalManager *) ep)->SetSTUNServer (PString ());

    gm_history_window_insert (history_window, _("Removed STUN server"));

    break;
  }
  
  gtk_widget_destroy (GTK_WIDGET (dialog));
}


static GtkWidget *
gm_sw_stun_result_window_new (GtkWidget *parent,
			      int nat_type_index)
{
  GtkWidget *dialog = NULL;
  GtkWidget *dialog_label = NULL;

  gchar *prefered_method = NULL;
  gchar *primary_text = NULL;
  gchar *dialog_text = NULL;

  gboolean stun_dialog = TRUE;

  PString nat_type = GMStunClient::GetNatName (nat_type_index);
  
  switch (nat_type_index)
    {
    case 0:
    case 7:
    case 8:
      prefered_method = g_strdup_printf (_("STUN test result: %s.\n\nEkiga could not detect the type of NAT you are using. The most appropriate method, if your router does not natively support SIP or H.323, is probably to forward the required ports to your internal machine and use IP translation if you are behind a NAT router. Please also make sure you are not running a local firewall."), (const char *) nat_type);
      stun_dialog = FALSE;
      break;

    case 1:
    case 6:
      prefered_method = g_strdup_printf (_("STUN test result: %s.\n\nYour system does not need any specific configuration as long as you do not have a local firewall blocking the ports required by Ekiga."), (const char *) nat_type);
      stun_dialog = FALSE;
      break;

    case 5:
      prefered_method = g_strdup_printf (_("Ekiga detected Symmetric NAT. The most appropriate method, if your router does not natively support SIP or H.323, is to forward the required ports to your internal machine in order to change your Symmetric NAT into Cone NAT. If you run this test again after the port forwarding has been done, it should report Cone NAT. This should allow Ekiga to be used with STUN support enabled. If it does not report Cone NAT, then it means that there is a problem in your forwarding rules."));
      stun_dialog = FALSE;
      break;
    case 9:
      prefered_method = g_strdup_printf (_("STUN test result: %s.\n\nYou do not seem to be using a NAT router. STUN support is not required."), (const char *) nat_type);
      stun_dialog = FALSE;
      break;

    default:
      prefered_method = g_strdup_printf (_("STUN test result: %s.\n\nUsing a STUN server is most probably the most appropriate method if your router does not natively support SIP or H.323.\n\nEnable STUN Support?"), (const char *) nat_type);
      stun_dialog = TRUE;
      break;
    }


  if (stun_dialog) {

    dialog = 
      gtk_dialog_new_with_buttons (_("NAT Detection Finished"),
				   GTK_WINDOW (parent),
				   GTK_DIALOG_MODAL,
				   GTK_STOCK_NO,
				   GTK_RESPONSE_NO,
				   GTK_STOCK_YES,
				   GTK_RESPONSE_YES,
				   NULL);
    gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_YES);
  }
  else {

    dialog = 
      gtk_dialog_new_with_buttons (_("NAT Detection Finished"),
				   GTK_WINDOW (parent),
				   GTK_DIALOG_MODAL,
				   GTK_STOCK_OK,
				   GTK_RESPONSE_ACCEPT,
				   NULL);
    gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);
  }

  primary_text =
    g_strdup_printf ("<span weight=\"bold\" size=\"larger\">%s</span>",
		     _("The detection of your NAT type is finished"));

  dialog_text =
    g_strdup_printf ("%s\n\n%s", primary_text, prefered_method);

  gtk_window_set_title (GTK_WINDOW (dialog), "");
  dialog_label = gtk_label_new (NULL);
  gtk_label_set_markup (GTK_LABEL (dialog_label),
			dialog_text);
  gtk_label_set_line_wrap (GTK_LABEL (dialog_label), TRUE);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox),
		     dialog_label);

  g_signal_connect (GTK_WINDOW (dialog), "response",
		    G_CALLBACK (stun_dialog_response_cb), NULL);

  g_free (prefered_method);
  g_free (primary_text);
  g_free (dialog_text);

  return dialog;
}


/* The class */
GMStunClient::GMStunClient (BOOL d,
			    BOOL c,
			    BOOL w,
			    GtkWidget *parent_window,
			    GMManager & endpoint)
  :PThread (1000, NoAutoDeleteThread), 
  ep (endpoint)
{
  gchar *conf_string = NULL;
  int nat_method = 0;
  
  gnomemeeting_threads_enter ();
  nat_method = gm_conf_get_int (NAT_KEY "method");
  conf_string = gm_conf_get_string (NAT_KEY "stun_server");
  stun_host = conf_string;
  gnomemeeting_threads_leave ();
  
  display_progress = d;
  display_config_dialog = c;
  wait = w;

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


PString GMStunClient::GetNatName (int i)
{
  char *name [] = 
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

  return PString (gettext (name [i]));
}


void GMStunClient::Main ()
{
  GtkWidget *history_window = NULL;
  GtkWidget *main_window = NULL;

  PSTUNClient *stun = NULL;

  PHTTPClient web_client ("GnomeMeeting");
  PString html;
  PString public_ip;
  PString listener_ip;
  PINDEX pos = 0;

  gchar *ip_detector = NULL;
  gboolean has_nat = FALSE;
  int nat_type_index = 0;

  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  history_window = GnomeMeeting::Process ()->GetHistoryWindow ();

  GtkWidget *progress_dialog = NULL;
  GtkWidget *dialog = NULL;


  PWaitAndSignal m(quit_mutex);

  /* Async remove the current stun server setting */
  if (stun_host.IsEmpty ()) {

    if (wait)
      sync.Signal ();

    ((OpalManager *) &ep)->SetSTUNServer (PString ());
    
    return;
  }

  /* Display a progress dialog */
  if (display_progress) {

    gnomemeeting_threads_enter ();
    progress_dialog = 
      gnomemeeting_progress_dialog (GTK_WINDOW (parent),
				    _("Detection in progress"), 
				    _("Please wait while your type of NAT is being detected."));
    gnomemeeting_threads_dialog_show_all (progress_dialog);
    gnomemeeting_threads_leave ();
  }

  /* Are we listening on a public IP address? */
  gnomemeeting_threads_enter ();
  ip_detector = gm_conf_get_string (NAT_KEY "public_ip_detector");
  gnomemeeting_threads_leave ();

  if (ip_detector != NULL
      && web_client.GetTextDocument (ip_detector, html)) {

    if (!html.IsEmpty ()) {

      PRegularExpression regex ("[0-9]*[.][0-9]*[.][0-9]*[.][0-9]*");
      PINDEX len;

      if (html.FindRegEx (regex, pos, len)) 
	public_ip = html.Mid (pos,len);
    }
  }

  listener_ip = ep.GetCurrentAddress ();
  pos = listener_ip.Find (":");
  if (pos != P_MAX_INDEX)
    listener_ip = listener_ip.Left (pos);
  has_nat = (listener_ip != public_ip);
  g_free (ip_detector);

  /* Set the STUN server for the endpoint */
  if (has_nat) {

    ((OpalManager *) &ep)->SetSTUNServer (stun_host);

    stun = ep.GetSTUN ();
  }

  if (stun) 
    nat_type_index = stun->GetNatType ();
  else if (!has_nat) 
    nat_type_index = 9; 
  nat_type = GetNatName (nat_type_index);

  if (wait)
    sync.Signal ();

  if (!display_config_dialog) {

    gnomemeeting_threads_enter ();
    if (has_nat)
      gm_history_window_insert (history_window, _("Set STUN server to %s (%s)"), (const char *) stun_host, (const char *) nat_type);
    else
      gm_history_window_insert (history_window, _("Ignored STUN server (%s)"), (const char *) nat_type);
    gnomemeeting_threads_leave ();
  }

  /* Show the config dialog */
  gnomemeeting_threads_enter ();
  if (display_config_dialog) {

    dialog = gm_sw_stun_result_window_new (parent, nat_type_index);
    gnomemeeting_threads_dialog_show_all (dialog);
  }
  gnomemeeting_threads_leave ();


  /* Delete the progress if any */
  gnomemeeting_threads_enter ();
  if (display_progress) 
    gnomemeeting_threads_widget_destroy (progress_dialog);
  gnomemeeting_threads_leave ();
}
