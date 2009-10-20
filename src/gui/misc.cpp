
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
 *                         misc.cpp  -  description
 *                         ------------------------
 *   begin                : Thu Nov 22 2001
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : This file contains miscellaneous functions.
 *   Additional Code      : De Michele Cristiano, Miguel Rodríguez 
 *
 */


#include "config.h"

#include "misc.h"
#include "ekiga.h"
#include "callbacks.h"

#include "gmdialog.h"
#include "gmwindow.h"
#include "gmconf.h"

#include <glib/gi18n.h>


/* The functions */
GtkWidget *
gnomemeeting_button_new (const char *lbl, 
			 GtkWidget *pixmap)
{
  GtkWidget *button = NULL;
  GtkWidget *hbox2 = NULL;
  GtkWidget *label = NULL;

  button = gtk_button_new ();
  label = gtk_label_new_with_mnemonic (lbl);
  hbox2 = gtk_hbox_new (FALSE, 0);

  gtk_box_pack_start(GTK_BOX (hbox2), pixmap, TRUE, TRUE, 0);  
  gtk_box_pack_start(GTK_BOX (hbox2), label, TRUE, TRUE, 0);

  gtk_container_add (GTK_CONTAINER (button), hbox2);

  return button;
}
