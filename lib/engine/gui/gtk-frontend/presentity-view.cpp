
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
 *                        presentity-view.cpp  -  description
 *                         --------------------------------
 *   begin                : written in july 2008 by Julien Puydt
 *   copyright            : (C) 2008 by Julien Puydt
 *   description          : Implementation of a Chat area (view and control)
 *
 */

#include "presentity-view.h"
#include "menu-builder-gtk.h"

struct _PresentityViewPrivate
{
  Ekiga::Presentity* presentity;
  boost::signals2::connection updated_conn;
  boost::signals2::connection removed_conn;

  /* we contain those, so no need to unref them */
  GtkWidget* presence_image;
  GtkWidget* name_status;
};

enum {
  PRESENTITY_VIEW_PROP_PRESENTITY = 1
};

G_DEFINE_TYPE (PresentityView, presentity_view, GTK_TYPE_EVENT_BOX);

/* declaration of callbacks */

static void on_presentity_updated (PresentityView* self);

static void on_presentity_removed (PresentityView* self);

/* declaration of internal api */

static void presentity_view_set_presentity (PresentityView* self,
					    Ekiga::Presentity* presentity);

static void presentity_view_unset_presentity (PresentityView* self);

/* implementation of callbacks */

static void
on_presentity_updated (PresentityView* self)
{
  gchar *txt = NULL;

  gtk_image_set_from_icon_name (GTK_IMAGE (self->priv->presence_image),
				("user-" + self->priv->presentity->get_presence ()).c_str (),
				GTK_ICON_SIZE_MENU);
  if (!self->priv->presentity->get_status ().empty ())
    txt = g_markup_printf_escaped ("<span weight=\"bold\">%s</span>\n<span size=\"small\">%s</span>",
                                   self->priv->presentity->get_name ().c_str (),
                                   self->priv->presentity->get_status ().c_str ());
  else
    txt = g_markup_printf_escaped ("<span weight=\"bold\">%s</span>",
                                   self->priv->presentity->get_name ().c_str ());

  gtk_label_set_markup (GTK_LABEL (self->priv->name_status), txt);
  g_free (txt);
}

static void
on_presentity_removed (PresentityView* self)
{
  presentity_view_unset_presentity (self);
}


/* implementation of internal api */

static void
presentity_view_set_presentity (PresentityView* self,
				Ekiga::Presentity* presentity)
{
  g_return_if_fail ( !self->priv->presentity);

  self->priv->presentity = presentity;
  self->priv->updated_conn = self->priv->presentity->updated.connect (boost::bind (&on_presentity_updated, self));
  self->priv->removed_conn = self->priv->presentity->removed.connect (boost::bind (&on_presentity_removed, self));

  on_presentity_updated (self);
}

static void
presentity_view_unset_presentity (PresentityView* self)
{
  if (self->priv->presentity) {

    self->priv->presentity = NULL;
    self->priv->updated_conn.disconnect ();
    self->priv->removed_conn.disconnect ();
  }
}

/* GObject code */

static void
presentity_view_finalize (GObject* obj)
{
  PresentityView* self = (PresentityView*)obj;

  presentity_view_unset_presentity (self);

  delete self->priv;
  self->priv = NULL;

  G_OBJECT_CLASS (presentity_view_parent_class)->finalize (obj);
}

static void
presentity_view_set_property (GObject* obj,
			      guint prop_id,
			      const GValue* value,
			      GParamSpec* spec)
{
  PresentityView* self = NULL;
  Ekiga::Presentity* presentity = NULL;

  self = (PresentityView* )obj;

  switch (prop_id) {

  case PRESENTITY_VIEW_PROP_PRESENTITY:
    presentity = (Ekiga::Presentity*)g_value_get_pointer (value);
    presentity_view_set_presentity (self, presentity);

    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, spec);
    break;
  }
}

static void
presentity_view_class_init (PresentityViewClass* klass)
{
  GObjectClass* gobject_class = G_OBJECT_CLASS (klass);
  GParamSpec* spec = NULL;

  gobject_class->finalize = presentity_view_finalize;
  gobject_class->set_property = presentity_view_set_property;

  spec = g_param_spec_pointer ("presentity",
			       "displayed presentity",
			       "Displayed presentity",
			       (GParamFlags)(G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
  g_object_class_install_property (gobject_class,
				   PRESENTITY_VIEW_PROP_PRESENTITY,
				   spec);
}

static void
presentity_view_init (PresentityView* self)
{
  GtkWidget* box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);

  gtk_container_add (GTK_CONTAINER (self), box);
  gtk_widget_show (box);

  self->priv = new PresentityViewPrivate;
  self->priv->presentity = NULL;


  self->priv->presence_image = gtk_image_new ();
  gtk_box_pack_start (GTK_BOX (box), self->priv->presence_image,
		      FALSE, FALSE, 2);
  gtk_widget_show (self->priv->presence_image);

  self->priv->name_status = gtk_label_new (NULL);
  gtk_box_pack_start (GTK_BOX (box), self->priv->name_status,
		      FALSE, FALSE, 2);
  gtk_widget_show (self->priv->name_status);
}

/* public api */

GtkWidget*
presentity_view_new (Ekiga::PresentityPtr presentity)
{
  return (GtkWidget*)g_object_new (TYPE_PRESENTITY_VIEW,
				   "presentity", presentity.get (),
				   NULL);
}
