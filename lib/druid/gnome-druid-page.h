/* gnome-druid-page.h
 * Copyright (C) 1999  Red Hat, Inc.
 * All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
/*
  @NOTATION@
*/
#ifndef __GNOME_DRUID_PAGE_H__
#define __GNOME_DRUID_PAGE_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GNOME_REGISTER_TYPE(type, type_as_function, corba_type,         \
                            parent_type, parent_type_macro)             \
        g_type_register_static (parent_type_macro, #type, &object_info, 0)
#define GNOME_CLASS_BOILERPLATE(type, type_as_function,          \
                           parent_type, parent_type_macro,              \
                           register_type_macro)                         \
static void type_as_function ## _class_init    (type ## Class *klass);  \
static void type_as_function ## _instance_init (type          *object); \
static parent_type ## Class *parent_class = NULL;                       \
static void                                                             \
type_as_function ## _class_init_trampoline (gpointer klass,             \
                                            gpointer data)              \
{                                                                       \
        parent_class = (parent_type ## Class *)g_type_class_ref (       \
                parent_type_macro);                                     \
        type_as_function ## _class_init ((type ## Class *)klass);       \
}                                                                       \
GType                                                                   \
type_as_function ## _get_type (void)                                    \
{                                                                       \
        static GType object_type = 0;                                   \
        if (object_type == 0) {                                         \
                static const GTypeInfo object_info = {                  \
                    sizeof (type ## Class),                             \
                    NULL,               /* base_init */                 \
                    NULL,               /* base_finalize */             \
                    type_as_function ## _class_init_trampoline,         \
                    NULL,               /* class_finalize */            \
                    NULL,               /* class_data */                \
                    sizeof (type),                                      \
                    0,                  /* n_preallocs */               \
                    (GInstanceInitFunc) type_as_function ## _instance_init \
                };                                                      \
                object_type = register_type_macro                       \
                        (type, type_as_function, type,            \
                         parent_type, parent_type_macro);               \
        }                                                               \
        return object_type;                                             \
}
#define GNOME_CALL_PARENT_WITH_DEFAULT(parent_class_cast,              \
                                        name, args, def_return)         \
        ((parent_class_cast(parent_class)->name != NULL) ?              \
         parent_class_cast(parent_class)->name args : def_return)



#define GNOME_TYPE_DRUID_PAGE            (gnome_druid_page_get_type ())
#define GNOME_DRUID_PAGE(obj)            (GTK_CHECK_CAST ((obj), GNOME_TYPE_DRUID_PAGE, GnomeDruidPage))
#define GNOME_DRUID_PAGE_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), GNOME_TYPE_DRUID_PAGE, GnomeDruidPageClass))
#define GNOME_IS_DRUID_PAGE(obj)         (GTK_CHECK_TYPE ((obj), GNOME_TYPE_DRUID_PAGE))
#define GNOME_IS_DRUID_PAGE_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GNOME_TYPE_DRUID_PAGE))
#define GNOME_DRUID_PAGE_GET_CLASS(obj)  (GTK_CHECK_GET_CLASS ((obj), GNOME_TYPE_DRUID_PAGE, GnomeDruidPageClass))


typedef struct _GnomeDruidPage        GnomeDruidPage;
typedef struct _GnomeDruidPagePrivate GnomeDruidPagePrivate;
typedef struct _GnomeDruidPageClass   GnomeDruidPageClass;

struct _GnomeDruidPage
{
	GtkBin parent;

	/*< private >*/
	GnomeDruidPagePrivate *_priv;
};
struct _GnomeDruidPageClass
{
	GtkBinClass parent_class;

	gboolean (*next)	(GnomeDruidPage *druid_page, GtkWidget *druid);
	void     (*prepare)	(GnomeDruidPage *druid_page, GtkWidget *druid);
	gboolean (*back)	(GnomeDruidPage *druid_page, GtkWidget *druid);
	void     (*finish)	(GnomeDruidPage *druid_page, GtkWidget *druid);
	gboolean (*cancel)	(GnomeDruidPage *druid_page, GtkWidget *druid);

	/* Signal used for relaying out the canvas */
	void     (*configure_canvas) (GnomeDruidPage *druid_page);

	/* virtual */
	void	 (*set_sidebar_shown) (GnomeDruidPage *druid_page,
				       gboolean sidebar_shown);

	/* Padding for possible expansion */
	gpointer padding1;
	gpointer padding2;
};


GType      gnome_druid_page_get_type            (void) G_GNUC_CONST;
GtkWidget *gnome_druid_page_new                 (void);
/* These are really to be only called from GnomeDruid */
gboolean   gnome_druid_page_next                (GnomeDruidPage *druid_page);
void       gnome_druid_page_prepare             (GnomeDruidPage *druid_page);
gboolean   gnome_druid_page_back                (GnomeDruidPage *druid_page);
gboolean   gnome_druid_page_cancel              (GnomeDruidPage *druid_page);
void       gnome_druid_page_finish              (GnomeDruidPage *druid_page);

G_END_DECLS

#endif /* __GNOME_DRUID_PAGE_H__ */




