
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
 *                         gmentrydialog.c  -  description
 *                         -------------------------------
 *   begin                : Sat Jan 03 2004
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : Contains a gmentrydialog widget permitting to
 *                          quickly build GtkDialogs with a +rw GtkEntry
 *                          field.
 *
 */


#include "gmentrydialog.h"


struct _GmEntryDialogPrivate {

  GtkWidget *field_entry;
  GtkWidget *label;
};

G_DEFINE_TYPE (GmEntryDialog, gm_entry_dialog, GTK_TYPE_DIALOG);

/* Static functions and declarations */
static void gm_entry_dialog_class_init (GmEntryDialogClass *);

static void gm_entry_dialog_init (GmEntryDialog *);


static void
gm_entry_dialog_class_init (GmEntryDialogClass *klass)
{
  g_type_class_add_private (klass, sizeof (GmEntryDialogPrivate));
}


static void
gm_entry_dialog_init (GmEntryDialog* self)
{
  GtkWidget *hbox = NULL;

  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
					    GM_TYPE_ENTRY_DIALOG,
					    GmEntryDialogPrivate);
  self->priv->field_entry = gtk_entry_new ();
  gtk_entry_set_activates_default (GTK_ENTRY (self->priv->field_entry), TRUE);

  self->priv->label = gtk_label_new (NULL);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);
  gtk_box_pack_start (GTK_BOX (hbox), self->priv->label, FALSE, FALSE, 6);
  gtk_box_pack_start (GTK_BOX (hbox), self->priv->field_entry, FALSE, FALSE, 6);

  gtk_container_add (GTK_CONTAINER (gtk_dialog_get_content_area (GTK_DIALOG (self))), hbox);

  gtk_dialog_add_buttons (GTK_DIALOG (self),
			  GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL);

  gtk_window_set_modal (GTK_WINDOW (self), TRUE);
}


/* public api */

GtkWidget *
gm_entry_dialog_new (const char *label,
		     const char *button_label)
{
  GmEntryDialog *ed = NULL;

  ed =
    GM_ENTRY_DIALOG (g_object_new (GM_TYPE_ENTRY_DIALOG, NULL));

  gtk_label_set_text (GTK_LABEL (GM_ENTRY_DIALOG (ed)->priv->label), label);

  gtk_dialog_add_buttons (GTK_DIALOG (ed),
			  button_label, GTK_RESPONSE_ACCEPT, NULL);

  gtk_widget_show_all (GTK_WIDGET (ed));

  return GTK_WIDGET (ed);
}


void
gm_entry_dialog_set_text (GmEntryDialog *ed,
			  const char *text)
{
  g_return_if_fail (GM_ENTRY_DIALOG (ed));
  g_return_if_fail (text != NULL);

  gtk_entry_set_text (GTK_ENTRY (ed->priv->field_entry), text);
}


const gchar *
gm_entry_dialog_get_text (GmEntryDialog *ed)
{
  g_return_val_if_fail (GM_ENTRY_DIALOG (ed), NULL);

  return gtk_entry_get_text (GTK_ENTRY (ed->priv->field_entry));
}
