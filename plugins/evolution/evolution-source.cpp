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
 *                         evolution-source.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : implementation of an evolution source
 *
 */

#include <iostream>

#include "config.h"

#include "evolution-source.h"

#if EDS_CHECK_VERSION(3,5,3)
#else
#define GCONF_PATH "/apps/evolution/addressbook/sources"
#endif

#if EDS_CHECK_VERSION(3,5,3)
static void
on_registry_source_added_c (ESourceRegistry */*registry*/,
                            ESource *source,
                            gpointer data)
{
  Evolution::Source *self = NULL;

  self = (Evolution::Source *)data;

  if (e_source_has_extension (source, E_SOURCE_EXTENSION_ADDRESS_BOOK))
    self->add_source (source);
}
#else
static void
on_source_list_group_added_c (ESourceList */*source_list*/,
			      ESourceGroup *group,
			      gpointer data)
{
  Evolution::Source *self = NULL;

  self = (Evolution::Source *)data;

  self->add_group (group);
}
#endif

#if EDS_CHECK_VERSION(3,5,3)
void
Evolution::Source::add_source (ESource *source)
{
  EBook *ebook = NULL;
  ebook = e_book_new (source, NULL);
  BookPtr book = Evolution::Book::create (services, ebook);
  g_object_unref (ebook);
  add_book (book);
}
#else
void
Evolution::Source::add_group (ESourceGroup *group)
{
  GSList *sources = NULL;

  sources = e_source_group_peek_sources (group);

  for ( ; sources != NULL; sources = g_slist_next (sources)) {

    ESource *source = NULL;
    ESource *s = NULL;
    gchar *uri = NULL;
    EBook *ebook = NULL;

    source = E_SOURCE (sources->data);

    s = e_source_copy (source);

    uri = g_strdup_printf("%s/%s",
			  e_source_group_peek_base_uri (group),
			  e_source_peek_relative_uri (source));
    e_source_set_absolute_uri (s, uri);
    g_free (uri);

    ebook = e_book_new (s, NULL);
    g_object_unref (s);

    BookPtr book (new Evolution::Book (services, ebook));

    g_object_unref (ebook);

    add_book (book);
  }
}
#endif

#if EDS_CHECK_VERSION(3,5,3)
static void
on_registry_source_removed_c (ESourceRegistry */*registry*/,
                              ESource *source,
                              gpointer data)
{
  Evolution::Source *self = (Evolution::Source *)data;
  if (e_source_has_extension (source, E_SOURCE_EXTENSION_ADDRESS_BOOK))
    self->remove_source (source);
}
#else
static void
on_source_list_group_removed_c (ESourceList */*source_list*/,
				ESourceGroup *group,
				gpointer data)
{
  Evolution::Source *self = NULL;

  self = (Evolution::Source *)data;

  self->remove_group (group);
}
#endif

class remove_helper
{
public :

#if EDS_CHECK_VERSION(3,5,3)
  remove_helper (ESource* source_): source(source_)
#else
  remove_helper (ESourceGroup* group_): group(group_)
#endif
  { ready (); }

  inline void ready ()
  { found = false; }

  bool operator() (Ekiga::BookPtr book_)
  {
    Evolution::BookPtr book = boost::dynamic_pointer_cast<Evolution::Book> (book_);
    if (book) {

      EBook *book_ebook = book->get_ebook ();
      ESource *book_source = e_book_get_source (book_ebook);
#if EDS_CHECK_VERSION(3,5,3)
      if (e_source_equal (source, book_source)) {
#else
      ESourceGroup *book_group = e_source_peek_group (book_source);

      if (book_group == group) {
#endif
        book->removed (book);
        found = true;
      }
    }

    return !found;
  }

  bool has_found () const
  { return found; }

private:
#if EDS_CHECK_VERSION(3,5,3)
  ESource* source;
#else
  ESourceGroup* group;
#endif
  bool found;
};

void
#if EDS_CHECK_VERSION(3,5,3)
Evolution::Source::remove_source (ESource *source)
{
  remove_helper helper (source);
#else
Evolution::Source::remove_group (ESourceGroup *group)
{
  remove_helper helper (group);
#endif

  do {

    helper.ready ();
    visit_books (boost::ref (helper));

  } while (helper.has_found ());
}

boost::shared_ptr<Evolution::Source>
Evolution::Source::create (Ekiga::ServiceCore &_services)
{
  boost::shared_ptr<Evolution::Source> source = boost::shared_ptr<Evolution::Source> (new Evolution::Source (_services));
  source->load ();

  return source;
}

Evolution::Source::Source (Ekiga::ServiceCore &_services)
  : services(_services)
{
}

Evolution::Source::~Source ()
{
#if EDS_CHECK_VERSION(3,5,3)
  g_object_unref (registry);
#else
  g_object_unref (source_list);
#endif
#if DEBUG
  std::cout << "Evolution::Source: Destructor invoked" << std::endl;
#endif
}

bool
Evolution::Source::populate_menu (Ekiga::MenuBuilder &/*builder*/)
{
  /* FIXME: add back creating a new addressbook later */
  return false;
}

void
Evolution::Source::load ()
{
#if EDS_CHECK_VERSION(3,5,3)
  GList *list, *link;
  const gchar *extension_name;
  GError *error = NULL;

  /* XXX This blocks.  Should it be created asynchronously? */
  registry = e_source_registry_new_sync (NULL, &error);

  if (error != NULL) {
    g_warning ("%s", error->message);
    g_error_free (error);
    return;
  }

  extension_name = E_SOURCE_EXTENSION_ADDRESS_BOOK;
  list = e_source_registry_list_sources (registry, extension_name);

  for (link = list; link != NULL; link = g_list_next (link)) {
    ESource *source = E_SOURCE (link->data);
    add_source (source);
  }

  g_list_foreach (list, (GFunc) g_object_unref, NULL);
  g_list_free (list);

  g_signal_connect (registry, "source-added",
                    G_CALLBACK (on_registry_source_added_c), this);
  g_signal_connect (registry, "source-removed",
		    G_CALLBACK (on_registry_source_removed_c), this);
#else
  GSList *groups = NULL;
  ESourceGroup *group = NULL;

  source_list = e_source_list_new_for_gconf_default (GCONF_PATH);

  groups = e_source_list_peek_groups (source_list);

  for ( ; groups != NULL; groups = g_slist_next (groups)) {

    group = E_SOURCE_GROUP (groups->data);
    add_group (group);
  }

  g_signal_connect (source_list, "group-added",
		    G_CALLBACK (on_source_list_group_added_c), this);
  g_signal_connect (source_list, "group-removed",
		    G_CALLBACK (on_source_list_group_removed_c), this);
#endif
}
