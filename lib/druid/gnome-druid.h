/* gnome-druid.h
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
/* TODO: allow setting bgcolor for all pages globally */
#ifndef __GNOME_DRUID_H__
#define __GNOME_DRUID_H__

#include <gtk/gtk.h>
#include <glib-object.h>
#include "gnome-druid-page.h"

G_BEGIN_DECLS

#define GNOME_PAD_SMALL    4
#define GNOME_CALL_PARENT(parent_class_cast, name, args)               \
        ((parent_class_cast(parent_class)->name != NULL) ?              \
         parent_class_cast(parent_class)->name args : (void)0)
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

#define GNOME_TYPE_DRUID            (gnome_druid_get_type ())
#define GNOME_DRUID(obj)            (GTK_CHECK_CAST ((obj), GNOME_TYPE_DRUID, GnomeDruid))
#define GNOME_DRUID_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), GNOME_TYPE_DRUID, GnomeDruidClass))
#define GNOME_IS_DRUID(obj)         (GTK_CHECK_TYPE ((obj), GNOME_TYPE_DRUID))
#define GNOME_IS_DRUID_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GNOME_TYPE_DRUID))
#define GNOME_DRUID_GET_CLASS(obj)  (GTK_CHECK_GET_CLASS ((obj), GNOME_TYPE_DRUID, GnomeDruidClass))


typedef struct _GnomeDruid        GnomeDruid;
typedef struct _GnomeDruidPrivate GnomeDruidPrivate;
typedef struct _GnomeDruidClass   GnomeDruidClass;

struct _GnomeDruid
{
	GtkContainer parent;
	GtkWidget *help;
	GtkWidget *back;
	GtkWidget *next;
	GtkWidget *cancel;
	GtkWidget *finish;

	/*< private >*/
	GnomeDruidPrivate *_priv;
};
struct _GnomeDruidClass
{
	GtkContainerClass parent_class;
	
	void     (*cancel)	(GnomeDruid *druid);
	void     (*help)	(GnomeDruid *druid);

	/* Padding for possible expansion */
	gpointer padding1;
	gpointer padding2;
};


GType      gnome_druid_get_type              (void) G_GNUC_CONST;
GtkWidget *gnome_druid_new                   (void);
void	   gnome_druid_set_buttons_sensitive (GnomeDruid *druid,
					      gboolean back_sensitive,
					      gboolean next_sensitive,
					      gboolean cancel_sensitive,
					      gboolean help_sensitive);
void	   gnome_druid_set_show_finish       (GnomeDruid *druid, gboolean show_finish);
void	   gnome_druid_set_show_help         (GnomeDruid *druid, gboolean show_help);
void       gnome_druid_prepend_page          (GnomeDruid *druid, GnomeDruidPage *page);
void       gnome_druid_insert_page           (GnomeDruid *druid, GnomeDruidPage *back_page, GnomeDruidPage *page);
void       gnome_druid_append_page           (GnomeDruid *druid, GnomeDruidPage *page);
void	   gnome_druid_set_page              (GnomeDruid *druid, GnomeDruidPage *page);

/* Pure sugar, methods for making new druids with a window already */
GtkWidget *gnome_druid_new_with_window       (const char *title,
					      GtkWindow *parent,
					      gboolean close_on_cancel,
					      GtkWidget **window);
void       gnome_druid_construct_with_window (GnomeDruid *druid,
					      const char *title,
					      GtkWindow *parent,
					      gboolean close_on_cancel,
					      GtkWidget **window);

G_END_DECLS

#endif /* __GNOME_DRUID_H__ */
