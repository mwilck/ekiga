
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2006 Damien Sandras
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *
 * GnomeMeting is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OpenH323 and Pwlib, and distribute the combination, without
 * applying the requirements of the GNU GPL to the OpenH323 program, as long
 * as you do follow the requirements of the GNU GPL for all the rest of the
 * software thus combined.
 */


/*
 *                         gmcontacts-dnd.c  -  description
 *                         ------------------------------------------
 *   begin                : July 2004
 *   copyright            : (C) 2004 by Julien Puydt
 *   description          : Drag'n drop of contacts made easy (implementation)
 *
 */

#include <string.h>

#include "gmcontacts.h"
#ifndef _GM_CONTACTS_H_INSIDE__
#define _GM_CONTACTS_H_INSIDE__
#include "gmcontacts-convert.h"
#undef _GM_CONTACTS_H_INSIDE__
#endif
#include "e-vcard.h"

gchar *
gmcontact_to_vcard (GmContact *contact)
{
    GString *str = NULL;
    gchar *result = NULL;

    g_return_val_if_fail (contact != NULL, NULL);

    /* do the conversion the brute way, since a GmContact is a rather crude
     * thing! 
     */
    str = g_string_new ("BEGIN:VCARD\r\n");
    if (contact->fullname)
      g_string_sprintfa (str, "FN:%s\r\n", contact->fullname);
    if (contact->email)
      g_string_sprintfa (str, "EMAIL;INTERNET:%s\r\n", contact->email);
    if (contact->url)
      g_string_sprintfa (str, "X-EVOLUTION-VIDEO-URL:%s\r\n", contact->url);
    if (contact->categories)
      g_string_sprintfa (str, "CATEGORIES:%s\r\n", contact->categories);
    if (contact->speeddial)
      g_string_sprintfa (str, "X-GNOMEMEETING-SPEEDDIAL:%s\r\n",
			 contact->speeddial);   
    str = g_string_append (str, "END:VCARD\r\n\r\n");

    result = str->str;
    g_string_free (str, FALSE); /* keep the gchar* ! */
    return result;
}

GmContact *
vcard_to_gmcontact (const gchar *vcard)
{
  GmContact *contact = NULL;

  GList *attr_iter = NULL;
  EVCardAttribute *attr = NULL;
  const char *attr_name = NULL;
  GList *attr_values = NULL;
  EVCard *evc = NULL;

  g_return_val_if_fail (vcard != NULL, NULL);

  /* parsing a vcard may not be really simple, as it can have all fields
   * a GmContact uses, and much, much more: use the evolution-data-server
   * code to make the hard work, then loop over the attributes and see what
   * we have
   */
  evc = e_vcard_new ();
  e_vcard_construct (evc, vcard);
  contact = gmcontact_new ();
  attr_iter = e_vcard_get_attributes (evc);
  while (attr_iter != NULL) {
    attr = (EVCardAttribute *)attr_iter->data;
    attr_name = e_vcard_attribute_get_name (attr);
    if (!strcmp (attr_name, EVC_FN)) {
      attr_values = e_vcard_attribute_get_values (attr);
      if (attr_values)
	contact->fullname = g_strdup ((char *)attr_values->data);
    }
    if (!strcmp (attr_name, EVC_EMAIL)) {
      attr_values = e_vcard_attribute_get_values (attr);
      if (attr_values)
	contact->email = g_strdup ((char *)attr_values->data);
    }
    if (!strcmp (attr_name, EVC_X_VIDEO_URL)) {
      attr_values = e_vcard_attribute_get_values (attr);
      if (attr_values)
	contact->url = g_strdup ((char *)attr_values->data);
    }
    if (!strcmp (attr_name, EVC_CATEGORIES)) {
      attr_values = e_vcard_attribute_get_values (attr);
	if (attr_values)
	  contact->categories = g_strdup ((char *)attr_values->data);
    }
    if (!strcmp (attr_name, "X-GNOMEMEETING-SPEEDDIAL")) {
      attr_values = e_vcard_attribute_get_values (attr);
      if (attr_values)
	contact->speeddial = g_strdup ((char *)attr_values->data);
    }
    attr_iter = g_list_next (attr_iter);
  }
  g_object_unref (evc);
  
  return contact;
}
