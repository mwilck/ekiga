/*  history-combo.h
 *
 *  GnomeMeeting -- A Video-Conferencing application
 *  Copyright (C) 2000-2001 Damien Sandras
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  Started: 18 June 2002, based one code from Thu Nov 22 2001
 *
 *  Authors: Damien Sandras <dsandras@seconix.com>
 *           Kenneth Christiansen <kenneth@gnu.org>
 *           Miguel Rodríguez <migrax@terra.es>
 *           De Michele Cristiano
 */

#ifndef __GM_HISTORY_COMBO_H
#define __GM_HISTORY_COMBO_H

#include <glib-object.h>
#include <gtk/gtk.h>
#include <gconf/gconf-client.h>

G_BEGIN_DECLS

#define GM_TYPE_HISTORY_COMBO         (gm_history_combo_get_type ())
#define GM_HISTORY_COMBO(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), GM_TYPE_HISTORY_COMBO, GmHistoryCombo))
#define GM_HISTORY_COMBO_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), GM_TYPE_HISTORY_COMBO, GmHistoryComboClass))
#define GM_IS_HISTORY_COMBO(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), GM_TYPE_HISTORY_COMBO))
#define GM_IS_HISTORY_COMBO_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), GM_TYPE_HISTORY_COMBO))
#define GM_HISTORY_COMBO_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), GM_TYPE_HISTORY_COMBO, GmHistoryComboClass))

typedef struct GmHistoryComboPrivate GmHistoryComboPrivate;

typedef struct
{
  GtkCombo parent;
  GList   *contact_list;
  
  GmHistoryComboPrivate *priv;
} GmHistoryCombo;

typedef struct
{
  GtkComboClass parent_class;
  
  /* signals */
  /* implementation */
} GmHistoryComboClass;

GType gm_history_combo_get_type (void);
GtkWidget *gm_history_combo_new (const char *key);

/**
 * gm_history_combo_add_entry:
 *
 * @key is the gconf key used to store the history.
 * 
 * Add a new entry to the history combo and saves it
 * in the GConf database.
 **/
void gm_history_combo_add_entry (GmHistoryCombo *combo, 
                                 const char *key,
                                 const char *new_entry);

void gm_history_combo_update (GmHistoryCombo *combo);

G_END_DECLS

#endif /* __GM_HISTORY_COMBO_H */
