
/* e-splash.cpp
 *
 * Copyright (C) 2000, 2001 Ximian, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Author: Ettore Perazzoli
 * Adapted to GnomeMeeting: Miguel Rodríguez
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "e-splash.h"

#include <gdk/gdkx.h>
#include <glib-object.h>
#include <X11/Xlib.h>


#define PARENT_TYPE gtk_window_get_type ()
static GtkWindowClass *parent_class = NULL;

struct _Icon {
	GdkPixbuf *dark_pixbuf;
	GdkPixbuf *light_pixbuf;
	GnomeCanvasItem *canvas_item;
};
typedef struct _Icon Icon;

struct _ESplashPrivate {
	GnomeCanvas *canvas;
	GdkPixbuf *splash_image_pixbuf;

	GList *icons;		/* (Icon *) */
	int num_icons;

	int layout_idle_id;
};


/* Layout constants.  These need to be changed if the splash changes.  */

#define ICON_Y    280
#define ICON_SIZE 48


/* BackingStore support.  */

static void
widget_realize_callback_for_backing_store (GtkWidget *widget,
					   void *data)
{
	XSetWindowAttributes attributes;
	GdkWindow *window;

	if (GTK_IS_LAYOUT (widget))
		window = GTK_LAYOUT (widget)->bin_window;
	else
		window = widget->window;

	attributes.backing_store = Always;
	XChangeWindowAttributes (GDK_WINDOW_XDISPLAY (window), GDK_WINDOW_XWINDOW (window),
				 CWBackingStore, &attributes);
}

/**
 * e_make_widget_backing_stored:
 * @widget: A GtkWidget
 * 
 * Make sure that the window for @widget has the BackingStore attribute set to
 * Always when realized.  This will allow the widget to be refreshed by the X
 * server even if the application is currently not responding to X events (this
 * is e.g. very useful for the splash screen).
 *
 * Notice that this will not work 100% in all cases as the server might not
 * support that or just refuse to do so.
 **/
void
e_make_widget_backing_stored  (GtkWidget *widget)
{
	g_signal_connect (G_OBJECT (widget), "realize",
			  G_CALLBACK (widget_realize_callback_for_backing_store), NULL);
}


/* Icon management.  */

static GdkPixbuf *
create_darkened_pixbuf (GdkPixbuf *pixbuf)
{
	GdkPixbuf *newpix;
	unsigned char *rowp;
	int width, height;
	int rowstride;
	int i, j;

	newpix = gdk_pixbuf_copy (pixbuf);
	if (! gdk_pixbuf_get_has_alpha (newpix))
		return newpix;

	width     = gdk_pixbuf_get_width (newpix);
	height    = gdk_pixbuf_get_height (newpix);
	rowstride = gdk_pixbuf_get_rowstride (newpix);

	rowp = gdk_pixbuf_get_pixels (newpix);
	for (i = 0; i < height; i ++) {
		unsigned char *p;

		p = rowp;
		for (j = 0; j < width; j++) {
			p[3] = static_cast<unsigned char> (p[3] * .25);
			p += 4;
		}

		rowp += rowstride;
	}

	return newpix;
}

static Icon *
icon_new (ESplash *splash,
	  GdkPixbuf *image_pixbuf)
{
	ESplashPrivate *priv;
	GnomeCanvasGroup *canvas_root_group;
	Icon *icon;

	priv = splash->priv;

	icon = g_new (Icon, 1);

	icon->light_pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8, ICON_SIZE, ICON_SIZE);
	gdk_pixbuf_scale (image_pixbuf, icon->light_pixbuf,
			  0, 0,
			  ICON_SIZE, ICON_SIZE,
			  0, 0,
			  (double) ICON_SIZE / gdk_pixbuf_get_width (image_pixbuf),
			  (double) ICON_SIZE / gdk_pixbuf_get_height (image_pixbuf),
			  GDK_INTERP_HYPER);

	icon->dark_pixbuf  = create_darkened_pixbuf (icon->light_pixbuf);

	/* Set up the canvas item to point to the dark pixbuf initially.  */

	canvas_root_group = GNOME_CANVAS_GROUP (GNOME_CANVAS (priv->canvas)->root);

	icon->canvas_item = gnome_canvas_item_new (canvas_root_group,
						   GNOME_TYPE_CANVAS_PIXBUF,
						   "pixbuf", icon->dark_pixbuf,
						   NULL);

	return icon;
}

static void
icon_free (Icon *icon)
{
	g_object_unref (G_OBJECT (icon->dark_pixbuf));
	g_object_unref (G_OBJECT (icon->light_pixbuf));
  	g_object_unref (G_OBJECT (icon->canvas_item)); 

	g_free (icon);
}


/* Icon layout management.  */

static void
layout_icons (ESplash *splash)
{
	ESplashPrivate *priv;
	GList *p;
	double x_step;
	double x, y;

	priv = splash->priv;

	x_step = ((double) gdk_pixbuf_get_width (priv->splash_image_pixbuf)) / priv->num_icons;

	x = (x_step - ICON_SIZE) / 2.0;
	y = ICON_Y;

	for (p = priv->icons; p != NULL; p = p->next) {
		Icon *icon;

		icon = (Icon *) p->data;

		g_object_set (G_OBJECT (icon->canvas_item),
			      "x", (double) x,
			      "y", (double) ICON_Y,
			      NULL);

		x += x_step;
	}
}

static int
layout_idle_cb (void *data)
{
	ESplash *splash;
	ESplashPrivate *priv;

	splash = E_SPLASH (data);
	priv = splash->priv;

	layout_icons (splash);

	priv->layout_idle_id = 0;

	return FALSE;
}

static void
schedule_relayout (ESplash *splash)
{
	ESplashPrivate *priv;

	priv = splash->priv;

	if (priv->layout_idle_id != 0)
		return;

	priv->layout_idle_id = gtk_idle_add (layout_idle_cb, splash);
}


/* GtkObject methods.  */

static void
impl_destroy (GtkObject *object)
{
	ESplash *splash;
	ESplashPrivate *priv;
	GList *p;

	splash = E_SPLASH (object);
	priv = splash->priv;

	if (priv->splash_image_pixbuf != NULL)
		g_object_unref (G_OBJECT (priv->splash_image_pixbuf));

	for (p = priv->icons; p != NULL; p = p->next) {
		Icon *icon;

		icon = (Icon *) p->data;
		icon_free (icon);
	}

	g_list_free (priv->icons);

	if (priv->layout_idle_id != 0)
		gtk_idle_remove (priv->layout_idle_id);

	g_free (priv);

	(* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}


static void
class_init (ESplashClass *klass)
{
	GtkObjectClass *object_class;

	object_class = GTK_OBJECT_CLASS (klass);
	object_class->destroy = impl_destroy;
  
	parent_class = reinterpret_cast<GtkWindowClass *>
	  (gtk_type_class (gtk_window_get_type ()));
}

static void
init (ESplash *splash)
{
	ESplashPrivate *priv;

	priv = g_new (ESplashPrivate, 1);
	priv->canvas              = NULL;
	priv->splash_image_pixbuf = NULL;
	priv->icons               = NULL;
	priv->num_icons           = 0;
	priv->layout_idle_id      = 0;

	splash->priv = priv;
}

static gboolean
button_press_event (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	ESplash *splash;

	splash = E_SPLASH (data);

	gtk_widget_hide (GTK_WIDGET (splash));
	
	return TRUE;
}


/**
 * e_splash_construct:
 * @splash: A pointer to an ESplash widget
 * @splash_image_pixbuf: The pixbuf for the image to appear in the splash dialog
 * 
 * Construct @splash with @splash_image_pixbuf as the splash image.
 **/
void
e_splash_construct (ESplash *splash,
		    GdkPixbuf *splash_image_pixbuf)
{
	ESplashPrivate *priv;
	GtkWidget *canvas, *frame;
	int image_width, image_height;

	g_return_if_fail (splash != NULL);
	g_return_if_fail (E_IS_SPLASH (splash));
	g_return_if_fail (splash_image_pixbuf != NULL);

	priv = splash->priv;

	priv->splash_image_pixbuf = GDK_PIXBUF (g_object_ref (splash_image_pixbuf));

	canvas = gnome_canvas_new_aa ();
	priv->canvas = GNOME_CANVAS (canvas);

	e_make_widget_backing_stored (canvas);

	image_width = gdk_pixbuf_get_width (splash_image_pixbuf);
	image_height = gdk_pixbuf_get_height (splash_image_pixbuf);

	gtk_widget_set_size_request (canvas, image_width, image_height);
	gnome_canvas_set_scroll_region (GNOME_CANVAS (canvas), 0, 0, image_width, image_height);
	gtk_widget_show (canvas);

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_OUT);
	gtk_container_add (GTK_CONTAINER (frame), canvas);
	gtk_widget_show (frame);

	gtk_container_add (GTK_CONTAINER (splash), frame);

	gnome_canvas_item_new (GNOME_CANVAS_GROUP (priv->canvas->root),
			       GNOME_TYPE_CANVAS_PIXBUF,
			       "pixbuf", splash_image_pixbuf,
			       NULL);
	
	g_signal_connect (G_OBJECT (splash), "button-press-event",
			  G_CALLBACK (button_press_event), splash);
	
	g_object_set (G_OBJECT (splash), "type", GTK_WINDOW_POPUP, NULL);
	gtk_window_set_position (GTK_WINDOW (splash), GTK_WIN_POS_CENTER);
	gtk_window_set_resizable (GTK_WINDOW (splash), FALSE);
	gtk_window_set_default_size (GTK_WINDOW (splash), image_width, image_height);
	gnome_window_icon_set_from_file (GTK_WINDOW (splash), 
					 GNOMEMEETING_IMAGES "/gnomemeeting-logo-icon.png");
	gtk_window_set_title (GTK_WINDOW (splash), "GnomeMeeting");

}

/**
 * e_splash_new:
 *
 * Create a new ESplash widget.
 * 
 * Return value: A pointer to the newly created ESplash widget.
 **/
GtkWidget *
e_splash_new (void)
{
	ESplash *newsp;
	GdkPixbuf *splash_image_pixbuf;

 	splash_image_pixbuf = gdk_pixbuf_new_from_file (GNOMEMEETING_IMAGES "/gnomemeeting-splash.png", 0);
	g_return_val_if_fail (splash_image_pixbuf != NULL, NULL);

	newsp = reinterpret_cast<ESplash *> (g_object_new (e_splash_get_type (), 0));
	e_splash_construct (newsp, splash_image_pixbuf);

	g_object_unref (splash_image_pixbuf);

	return GTK_WIDGET (newsp);
}


/**
 * e_splash_add_icon:
 * @splash: A pointer to an ESplash widget
 * @icon_pixbuf: Pixbuf for the icon to be added
 * 
 * Add @icon_pixbuf to the @splash.
 * 
 * Return value: The total number of icons in the splash after the new icon has
 * been added.
 **/
int
e_splash_add_icon (ESplash *splash,
		   GdkPixbuf *icon_pixbuf)
{
	ESplashPrivate *priv;
	Icon *icon;

	g_return_val_if_fail (splash != NULL, 0);
	g_return_val_if_fail (E_IS_SPLASH (splash), 0);
	g_return_val_if_fail (icon_pixbuf != NULL, 0);

	priv = splash->priv;

	icon = icon_new (splash, icon_pixbuf);
	priv->icons = g_list_append (priv->icons, icon);

	priv->num_icons ++;

	schedule_relayout (splash);

	return priv->num_icons;
}

/**
 * e_splash_set_icon_highlight:
 * @splash: A pointer to an ESplash widget
 * @num: Number of the icon whose highlight state must be changed
 * @highlight: Whether the icon must be highlit or not
 * 
 * Change the highlight state of the @num-th icon.
 **/
void
e_splash_set_icon_highlight  (ESplash *splash,
			      int num,
			      gboolean highlight)
{
	ESplashPrivate *priv;
	Icon *icon;

	g_return_if_fail (splash != NULL);
	g_return_if_fail (E_IS_SPLASH (splash));

	priv = splash->priv;

	icon = (Icon *) g_list_nth_data (priv->icons, num);

	g_object_set (G_OBJECT (icon->canvas_item),
		      "pixbuf", highlight ? icon->light_pixbuf : icon->dark_pixbuf,
		      NULL);
}

#define E_MAKE_TYPE(l,str,t,ci,i,parent) \
GType l##_get_type(void)\
{\
        static GType type = 0;                          \
        if (!type){                                     \
                static GTypeInfo const object_info = {  \
                        sizeof (t##Class),              \
                                                        \
                        (GBaseInitFunc) NULL,           \
                        (GBaseFinalizeFunc) NULL,       \
                                                        \
                        (GClassInitFunc) ci,            \
                        (GClassFinalizeFunc) NULL,      \
                        NULL,   /* class_data */        \
                                                        \
                        sizeof (t),                     \
                        0,      /* n_preallocs */       \
                        (GInstanceInitFunc) i,          \
                };                                      \
                type = g_type_register_static (parent, str, &object_info, (GTypeFlags) 0);   \
        }                                               \
        return type;                                    \
}


E_MAKE_TYPE (e_splash, "ESplash", ESplash, class_init, init, PARENT_TYPE)
