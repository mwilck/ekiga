
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


#include <math.h>

#include "gmpowermeter.h"

struct _GmPowermeterPrivate {

  GmPowermeterIconset *iconset;
  /*!< used icons to draw the level, in a NULL terminated vector */

  gfloat level;
  /*!< the level to display, a float between 0.0 and 1.0 */
};

#define NUMBER_OF_PICTURES 5

#include "pixmaps/gm_powermeter_default_00.xpm"
#include "pixmaps/gm_powermeter_default_01.xpm"
#include "pixmaps/gm_powermeter_default_02.xpm"
#include "pixmaps/gm_powermeter_default_03.xpm"
#include "pixmaps/gm_powermeter_default_04.xpm"

G_DEFINE_TYPE (GmPowermeter, gm_powermeter, GTK_TYPE_IMAGE);

/* some helpers */

static guint
gm_powermeter_get_index_by_level (guint maxindex,
				  gfloat level)
{
  /* FIXME? */
  gfloat stepvalue = 0.0;
  gfloat stepnumber = 0.0;

  if (level <= 0.0)
    return 0;
  if (level >= 1.0)
    return maxindex;

  stepvalue = 1.0 / maxindex;
  stepnumber = level / stepvalue;

  return (guint) rintf ((float) stepnumber);
}

static void
gm_powermeter_redraw (GmPowermeter* powermeter)
{
  guint calculated_index = 0;

  g_return_if_fail (GM_IS_POWERMETER (powermeter));

  calculated_index =
    gm_powermeter_get_index_by_level (powermeter->priv->iconset->max_index,
				      powermeter->priv->level);

  gtk_image_set_from_pixbuf (GTK_IMAGE (powermeter),
			     powermeter->priv->iconset->iconv [calculated_index]);
}

/* GObject implementation */

static void
gm_powermeter_dispose (GObject *obj)
{
  int ii;

  for (ii = 0; ii < NUMBER_OF_PICTURES; ii++) {

  if (((GmPowermeter*)obj)->priv->iconset->iconv[ii])
    g_object_unref (((GmPowermeter*)obj)->priv->iconset->iconv[ii]);
  ((GmPowermeter*)obj)->priv->iconset->iconv[ii] = NULL;
  }

  G_OBJECT_CLASS (gm_powermeter_parent_class)->dispose (obj);
}

static void
gm_powermeter_finalize (GObject *obj)
{
  g_free (((GmPowermeter*)obj)->priv->iconset->iconv);
  g_free (((GmPowermeter*)obj)->priv->iconset);

  G_OBJECT_CLASS (gm_powermeter_parent_class)->finalize (obj);
}

static void
gm_powermeter_class_init (GmPowermeterClass* klass)
{
  GObjectClass* gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->dispose = gm_powermeter_dispose;
  gobject_class->finalize = gm_powermeter_finalize;

  g_type_class_add_private (klass, sizeof (GmPowermeterPrivate));
}


static void
gm_powermeter_init (GmPowermeter* powermeter)
{
  /* adjust that when you change the number of pictures for the default set! */
  char ** tmp_xmp = { NULL };

  powermeter->priv = G_TYPE_INSTANCE_GET_PRIVATE (powermeter,
						  GM_TYPE_POWERMETER, GmPowermeterPrivate);

  powermeter->priv->level = 0.0;

  /* set the default icon set FIXME isn't that ugly? */
  powermeter->priv->iconset = (GmPowermeterIconset*) g_malloc (sizeof (GmPowermeterIconset));
  powermeter->priv->iconset->max_index = NUMBER_OF_PICTURES - 1;

  /* allocate the vector table (plus 1 for NULL) */
  powermeter->priv->iconset->iconv = 
    (GdkPixbuf**) g_malloc (sizeof (GdkPixbuf*) * (NUMBER_OF_PICTURES + 1));

  /* populate the vector table and append NULL */
  /* append/remove lines when you change the number of
   * pictures for the default set! (and free them in dispose!)
   * FIXME FIXME
   * the way round char** tmp_xmp for temporary assignment is needed because
   * there seems to be no way to directly do
   *   foo = gdk_pixbuf_new_from_xpm_data ((const char **) xpm_data);
   * without compiler warnings!
   */
  tmp_xmp = (char **) gm_powermeter_default_00_xpm;
  powermeter->priv->iconset->iconv[0] = gdk_pixbuf_new_from_xpm_data ((const char**) tmp_xmp);

  tmp_xmp = (char **) gm_powermeter_default_01_xpm;
  powermeter->priv->iconset->iconv[1] = gdk_pixbuf_new_from_xpm_data ((const char**) tmp_xmp);

  tmp_xmp = (char **) gm_powermeter_default_02_xpm;
  powermeter->priv->iconset->iconv[2] = gdk_pixbuf_new_from_xpm_data ((const char**) tmp_xmp);

  tmp_xmp = (char **) gm_powermeter_default_03_xpm;
  powermeter->priv->iconset->iconv[3] = gdk_pixbuf_new_from_xpm_data ((const char**) tmp_xmp);

  tmp_xmp = (char **) gm_powermeter_default_04_xpm;
  powermeter->priv->iconset->iconv[4] = gdk_pixbuf_new_from_xpm_data ((const char**) tmp_xmp);

  powermeter->priv->iconset->iconv[NUMBER_OF_PICTURES] = NULL;

  gm_powermeter_redraw (powermeter);
}


GtkWidget*
gm_powermeter_new (void)
{
  return GTK_WIDGET (g_object_new (GM_TYPE_POWERMETER, NULL));
}

void
gm_powermeter_set_level (GmPowermeter* powermeter,
			 gfloat level)
{
  g_return_if_fail (GM_IS_POWERMETER (powermeter));

  /* don't bother if we're requested to display the same
   * level we already do */
  if (fabs (level - powermeter->priv->level) <= 0.0001)
    return;

  powermeter->priv->level = level;

  /* limit the level to values between 0 and 1, inclusive */
  if (powermeter->priv->level < 0.0)
    powermeter->priv->level = 0.0;
  if (powermeter->priv->level > 1.0)
    powermeter->priv->level = 1.0;

  gm_powermeter_redraw (powermeter);
}

gfloat
gm_powermeter_get_level (GmPowermeter* powermeter)
{
  g_return_val_if_fail (GM_IS_POWERMETER (powermeter), 0.0);

  return powermeter->priv->level;
}
