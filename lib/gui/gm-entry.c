
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2014 Damien Sandras <dsandras@seconix.com>
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
 *                         gm-entry.c  -  description
 *                         --------------------------
 *   begin                : Sat Oct 25 2014
 *   copyright            : (C) 2014 by Damien Sandras
 *   description          : Contains a GmEntry implementation handling
 *                          quick delete and basic validation depending
 *                          on the entry type.
 *
 */


#include "gm-entry.h"

#include <regex.h>

struct _GmEntryPrivate {

  GRegex *regex;
  gchar *regex_string;
  gboolean is_valid;
  gboolean allow_empty;
};

enum {
  GM_ENTRY_REGEX = 1,
  GM_ENTRY_ALLOW_EMPTY = 2,
};

enum {
  VALIDITY_CHANGED_SIGNAL,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (GmEntry, gm_entry, GTK_TYPE_ENTRY);


/* Callbacks */
static void gm_entry_changed_cb (GmEntry *self,
                                 G_GNUC_UNUSED gpointer data);

static void gm_entry_icon_release_cb (GtkEntry *self,
                                      G_GNUC_UNUSED gpointer data);


/* Static GObject functions and declarations */
static void gm_entry_class_init (GmEntryClass *);

static void gm_entry_init (GmEntry *);

static void gm_entry_dispose (GObject *);

static void gm_entry_get_property (GObject *obj,
                                   guint prop_id,
                                   GValue *value,
                                   GParamSpec *spec);

static void gm_entry_set_property (GObject *obj,
                                   guint prop_id,
                                   const GValue *value,
                                   GParamSpec *spec);


/* Callbacks */
static void
gm_entry_changed_cb (GmEntry *self,
                     G_GNUC_UNUSED gpointer data)
{
  gboolean empty = (gtk_entry_get_text_length (GTK_ENTRY (self)) == 0);
  gboolean rtl = (gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL);
  gboolean is_valid = gm_entry_text_is_valid (self);

  g_object_set (self,
                "secondary-icon-name", !empty ? (rtl ? "edit-clear-rtl-symbolic" : "edit-clear-symbolic") : NULL,
                "secondary-icon-activatable", !empty,
                "secondary-icon-sensitive", !empty,
                NULL);

  if (is_valid != self->priv->is_valid) {
    self->priv->is_valid = is_valid;
    g_signal_emit (self, signals[VALIDITY_CHANGED_SIGNAL], 0);
  }
}


static void
gm_entry_icon_release_cb (GtkEntry *self,
                          G_GNUC_UNUSED gpointer data)
{
  gtk_entry_set_text (self, "");
  gtk_widget_grab_focus (GTK_WIDGET (self));
}


/* Implementation of GObject stuff */
static void
gm_entry_dispose (GObject* obj)
{
  GmEntryPrivate *priv = GM_ENTRY (obj)->priv;

  if (priv->regex) {
    g_regex_unref (priv->regex);
    priv->regex = NULL;
  }
  if (priv->regex_string) {
    g_free (priv->regex_string);
    priv->regex_string = NULL;
  }

  G_OBJECT_CLASS (gm_entry_parent_class)->dispose (obj);
}


static void
gm_entry_get_property (GObject *obj,
                       guint prop_id,
                       GValue *value,
                       GParamSpec *spec)
{
  GmEntry *self = NULL;

  self = GM_ENTRY (obj);
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GM_TYPE_ENTRY, GmEntryPrivate);

  switch (prop_id) {

  case GM_ENTRY_REGEX:
    g_value_set_string (value, self->priv->regex_string);
    break;

  case GM_ENTRY_ALLOW_EMPTY:
    g_value_set_boolean (value, self->priv->allow_empty);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, spec);
    break;
  }
}


static void
gm_entry_set_property (GObject *obj,
                       guint prop_id,
                       const GValue *value,
                       GParamSpec *spec)
{
  GmEntry *self = NULL;
  GError *error = NULL;
  const gchar *str = NULL;

  self = GM_ENTRY (obj);
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GM_TYPE_ENTRY, GmEntryPrivate);

  switch (prop_id) {

  case GM_ENTRY_REGEX:
    if (self->priv->regex_string)
      g_free (self->priv->regex_string);
    if (self->priv->regex)
      g_regex_unref (self->priv->regex);
    self->priv->regex_string = NULL;
    self->priv->regex = NULL;

    str = g_value_get_string (value);
    if (g_strcmp0 (str, "")) {
      self->priv->regex = g_regex_new (str, 0, 0, &error);
      if (!self->priv->regex) {
        g_warning ("Failed to create regex: %s", error->message);
        g_error_free (error);
        break;
      }
      self->priv->regex_string = g_strdup (str);
    }
    break;

  case GM_ENTRY_ALLOW_EMPTY:
    self->priv->allow_empty = g_value_get_boolean (value);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, spec);
    break;
  }
}


static void
gm_entry_class_init (GmEntryClass *klass)
{
  GParamSpec *spec = NULL;

  g_type_class_add_private (klass, sizeof (GmEntryPrivate));

  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->dispose = gm_entry_dispose;
  gobject_class->get_property = gm_entry_get_property;
  gobject_class->set_property = gm_entry_set_property;

  spec = g_param_spec_string ("regex", "Regex", "Validation Regex",
                              NULL, (GParamFlags) G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, GM_ENTRY_REGEX, spec);


  spec = g_param_spec_boolean ("allow-empty", "Allow Empty", "Allow empty GmEntry",
                               TRUE, (GParamFlags) G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, GM_ENTRY_ALLOW_EMPTY, spec);


  signals[VALIDITY_CHANGED_SIGNAL] =
    g_signal_new ("validity-changed",
		  G_OBJECT_CLASS_TYPE (gobject_class),
		  G_SIGNAL_RUN_LAST,
		  0, NULL, NULL,
		  g_cclosure_marshal_generic,
		  G_TYPE_NONE, 0);
}


static void
gm_entry_init (GmEntry* self)
{
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                            GM_TYPE_ENTRY,
					    GmEntryPrivate);

  self->priv->regex = NULL;
  self->priv->regex_string = NULL;
  self->priv->is_valid = gm_entry_text_is_valid (self);

  g_signal_connect (self, "changed",
                    G_CALLBACK (gm_entry_changed_cb), NULL);
  g_signal_connect (self, "icon-release",
                    G_CALLBACK (gm_entry_icon_release_cb), NULL);
}


/* public api */
GtkWidget *
gm_entry_new (const gchar *regex)
{
  if (regex)
    return GTK_WIDGET (g_object_new (GM_TYPE_ENTRY, "regex", regex, NULL));

  return GTK_WIDGET (g_object_new (GM_TYPE_ENTRY, NULL));
}


gboolean
gm_entry_text_is_valid (GmEntry *self)
{
  GMatchInfo *match_info = NULL;
  gboolean success = FALSE;
  gchar *value = NULL;

  g_return_val_if_fail (GM_IS_ENTRY (self), success);
  if (!self->priv->regex)
    return TRUE;

  const char *content = gtk_entry_get_text (GTK_ENTRY (self));
  if (self->priv->allow_empty) {
    value = g_strdup (content);
    value = g_strstrip (value);
    if (!g_strcmp0 (value, "")) {
      g_free (value);
      return TRUE;
    }
    g_free (value);
  }

  g_regex_match (self->priv->regex, content, 0, &match_info);

  if (g_match_info_matches (match_info))
    success = TRUE;

  g_match_info_free (match_info);

  return success;
}


void
gm_entry_set_allow_empty (GmEntry *self,
                          gboolean allow_empty)
{
  g_return_if_fail (GM_IS_ENTRY (self));

  g_object_set (self, "allow-empty", allow_empty, NULL);
}


gboolean
gm_entry_get_allow_empty (GmEntry *self)
{
  g_return_val_if_fail (GM_IS_ENTRY (self), TRUE);

  return self->priv->allow_empty;
}
