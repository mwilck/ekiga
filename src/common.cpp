
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2003 Damien Sandras
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
 *                         common.cpp  -  description
 *                         --------------------------
 *   begin                : Sat Dec 23 2000
 *   copyright            : (C) 2000-2003 by Damien Sandras
 *   description          : This file contains things common to the whole soft.
 *
 */


#include "common.h"


GmWindow *
gnomemeeting_get_main_window (GtkWidget *gm)
{
  GmWindow *gw = (GmWindow *) g_object_get_data (G_OBJECT (gm), "gw");

  return gw;
}


GmPrefWindow *
gnomemeeting_get_pref_window (GtkWidget *gm)
{
  GmPrefWindow *pw = (GmPrefWindow *) g_object_get_data (G_OBJECT (gm), "pw");

  return pw;
}


GmLdapWindow *
gnomemeeting_get_ldap_window (GtkWidget *gm)
{
  GmLdapWindow *lw = (GmLdapWindow *) g_object_get_data (G_OBJECT (gm), "lw");

  return lw;
}


GmDruidWindow *
gnomemeeting_get_druid_window (GtkWidget *gm)
{
  GmDruidWindow *dw =
    (GmDruidWindow *) g_object_get_data (G_OBJECT (gm), "dw");

  return dw;
}


GmCallsHistoryWindow *
gnomemeeting_get_calls_history_window (GtkWidget *gm)
{
  GmCallsHistoryWindow *chw =
    (GmCallsHistoryWindow *) g_object_get_data (G_OBJECT (gm), "chw");

  return chw;
}


GmTextChat *
gnomemeeting_get_chat_window (GtkWidget *gm)
{
  GmTextChat *chat = (GmTextChat *) g_object_get_data (G_OBJECT (gm), "chat");

  return chat;
}


GmRtpData *
gnomemeeting_get_rtp_data (GtkWidget *gm)
{
  GmRtpData *rtp = (GmRtpData *) g_object_get_data (G_OBJECT (gm), "rtp");

  return rtp;
}
