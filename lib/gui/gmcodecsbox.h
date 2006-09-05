
/* Ekiga -- A VoIP and Video-Conferencing application
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
 *                         gmcodecsbox.h  -  description
 *                         -------------------------------
 *   begin                : Sat Sep 02 2006
 *   copyright            : (C) 2000-2006 by Damien Sandras 
 *   description          : Contains a codecs box
 *
 */



#ifndef __GM_CODECS_BOX_H
#define __GM_CODECS_BOX_H

#include <glib-object.h>
#include <gtk/gtk.h>

#include <ptlib.h>
#include <opal/manager.h>


G_BEGIN_DECLS

#define GM_CODECS_BOX_TYPE (gm_codecs_box_get_type ())
#define GM_CODECS_BOX(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), GM_CODECS_BOX_TYPE, GmCodecsBox))
#define GM_CODECS_BOX_CLASS(k) (G_TYPE_CHECK_CLASS_CAST((k), GM_CODECS_BOX_TYPE, GmCodecsBoxClass))
#define GM_IS_CODECS_BOX(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), GM_CODECS_BOX_TYPE))
#define GM_IS_CODECS_BOX_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), GM_CODECS_BOX_TYPE))
#define GM_CODECS_BOX_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), GM_CODECS_BOX_TYPE, GmConnectButtonClass))


typedef struct
{
  GtkHBox parent;

  OpalMediaFormatList *media_formats;
  
  GtkWidget *codecs_list;
  
  gchar *conf_key;
  gboolean activatable_codecs;

} GmCodecsBox;


typedef struct
{
  GtkHBoxClass parent_class;
  
} GmCodecsBoxClass;


/* The functions */

/* DESCRIPTION  :  /
 * BEHAVIOR     :  Returns the GType for the GmCodecsBox.
 * PRE          :  /
 */
GType gm_codecs_box_get_type (void);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Creates a new GmCodecsBox. If activatable_codecs is TRUE,
 *                 codecs can be enabled or disabled. conf_key is the key
 *                 where to store results.
 * PRE          :  A valid GMConf key.
 */
GtkWidget *gm_codecs_box_new (gboolean activatable_codecs,
                              const char *conf_key);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Update the codecs list in the GmCodecsBox.
 *                 When codecs are enabled/disabled/moved or when
 *                 the codecs list is updated, the signal "codecs-box-changed"
 *                 is emitted.
 * PRE          :  /
 */
void gm_codecs_box_set_codecs (GmCodecsBox *cb,
                               const OpalMediaFormatList & l);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Returns the list of OpalMediaFormats to enable.
 * PRE          :  /
 */
void gm_codecs_box_get_codecs (GmCodecsBox *cb,
                               PStringArray & order);

G_END_DECLS

#endif /* __GM_CODECS_BOX_H */
