
/*
 * Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2008 Damien Sandras

 * This program is free software; you can  redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version. This program is distributed in the hope
 * that it will be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Ekiga is licensed under the GPL license and as a special exception, you
 * have permission to link or otherwise combine this program with the
 * programs OPAL, OpenH323 and PWLIB, and distribute the combination, without
 * applying the requirements of the GNU GPL to the OPAL, OpenH323 and PWLIB
 * programs, as long as you do follow the requirements of the GNU GPL for all
 * the rest of the software thus combined.
 */


/*
 *                         gm-refcounted-boxed.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : interface of boxed GmRefCounted objects
 *
 */

#ifndef __GM_REFCOUNTED_BOXED_H__
#define __GM_REFCOUNTED_BOXED_H__

#include <glib-object.h>

#define GM_TYPE_REFCOUNTED (gm_refcounted_boxed_get_type ())

/* What does this code do?
 *
 * The basic idea is that it's possible to make gtk+ behave well with our
 * own reference counting, when we give it some GmRefCounted objects.
 *
 * Here is an example ; you create a store with a column containing GmRefCounted
 * objects :
 *
 * store = gtk_list_store_new (1, GM_TYPE_REFCOUNTED);
 *
 * later on you do :
 *
 * {
 *   gmref_ptr<Foo> foo(new Foo(data));
 *   gtk_list_store_set (store, &iter, 0, &*foo, -1);
 * }
 *
 * Here foo gets out of scope at the second } and should be freed...
 * it isn't, because we told gtk+ that column was GM_TYPE_REFCOUNTED, so it
 * incremented the reference count automatically... and will decrease it when
 * that position in the store is cleared.
 *
 * There's something to beware though : gtk+ will give you ownership of what
 * you get out of the GtkTreeModel, so you'll have to gmref_dec after use,
 * like this :
 *
 * GmRefCounted* foo = NULL;
 * gtk_tree_model_get (model, &iter, 0, &foo, -1);
 * do_something (foo);
 * gmref_dec (foo);
 *
 */

GType gm_refcounted_boxed_get_type () G_GNUC_CONST;

#endif
