
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2001 Damien Sandras
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
 */

/*
 *                         misc.cpp  -  description
 *                         ------------------------
 *   begin                : Thu Nov 22 2001
 *   copyright            : (C) 2000-2001 by Damien Sandras
 *   description          : This file contains miscellaneous functions.
 *   email                : dsandras@seconix.com
 *
 */


#include <ptlib.h>
#include <gnome.h>

#include "misc.h"


void gnomemeeting_threads_enter () {


  if (PThread::Current ()->GetThreadName () != "gnomemeeting") {
    
    PTRACE(3, "Will Take GDK Lock");
    gdk_threads_enter ();
    PTRACE(3, "GDK Lock Taken");
  }
  else {

    PTRACE(3, "Ignore GDK Lock request : Main Thread");
  }
    
}


void gnomemeeting_threads_leave () {

  if (PThread::Current ()->GetThreadName () != "gnomemeeting") {

    PTRACE(3, "Will Release GDK Lock");
    gdk_threads_leave ();
    PTRACE(3, "GDK Lock Released");
  }
  else {

    PTRACE(3, "Ignore GDK UnLock request : Main Thread");
  }
    
}


GtkWidget *gnomemeeting_button (gchar *lbl, GtkWidget *pixmap)
{
  GtkWidget *button;
  GtkWidget *hbox2;
  GtkWidget *label;
  
  button = gtk_button_new ();
  label = gtk_label_new (N_(lbl));
  hbox2 = gtk_hbox_new (FALSE, 0);

  gtk_box_pack_start(GTK_BOX (hbox2), pixmap, TRUE, TRUE, GNOME_PAD_SMALL);  
  gtk_box_pack_start(GTK_BOX (hbox2), label, TRUE, TRUE, GNOME_PAD_SMALL);
  
  gtk_container_add (GTK_CONTAINER (button), hbox2);
  
  gtk_widget_set_usize (GTK_WIDGET (button), 40, 25);
  
  gtk_widget_show (pixmap);
  gtk_widget_show (label);
  gtk_widget_show (hbox2);
		
  return button;
}
