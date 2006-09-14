
/* Ekiga -- A VoIP and Video-Conferencing application
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


#include "gmcontacts.h"

#ifndef _GM_CONTACTS_H_INSIDE__
#define _GM_CONTACTS_H_INSIDE__
#include "gmcontacts-local.h"
#include "gmcontacts-remote.h"
#undef _GM_CONTACTS_H_INSIDE__
#endif

#include "toolbox.h"

GSList *
gmcontact_enum_categories (const GmContact * contact)
{
  GSList * categorylist = NULL;
  gchar ** split_categories = { NULL };
  gchar ** split_categories_iter = { NULL };

  g_return_val_if_fail (contact != NULL, NULL);

  if (!contact->categories)
    return NULL;

  if (g_ascii_strcasecmp (contact->categories, "") == 0)
    return NULL;

  split_categories =
    g_strsplit_set (contact->categories,
		    ",",
		    -1);

  for (split_categories_iter = split_categories;
       *split_categories_iter != NULL;
       split_categories_iter++) {
    if (g_ascii_strcasecmp (*split_categories_iter,"") != 0)
      categorylist =
	g_slist_append (categorylist, g_strdup (*split_categories_iter));
  }
  
  g_strfreev (split_categories);

  categorylist = gm_string_gslist_remove_dups (categorylist);

  return categorylist;
}


gboolean
gmcontact_is_in_category (const GmContact * contact,
			  const gchar * category)
{
  GSList * categorylist = NULL;
  GSList * categorylist_iter = NULL;
  
  g_return_val_if_fail (contact != NULL, FALSE);

  categorylist = gmcontact_enum_categories (contact);

  if (!categorylist)
    return FALSE;

  for (categorylist_iter = categorylist;
       categorylist_iter != NULL;
       categorylist_iter = g_slist_next (categorylist_iter))
    {
      if (g_ascii_strcasecmp ((const gchar*) categorylist_iter->data,
			      category) == 0)
	{
	  g_slist_free (categorylist);
	  return TRUE;
	}
    }

  g_slist_free (categorylist);
  return FALSE;
}


GSList *
gnomemeeting_addressbook_get_contacts (GmAddressbook *addressbook,
				       int &nbr,
				       gboolean partial_match,
				       gchar *fullname,
				       gchar *url,
				       gchar *categorie,
				       gchar *location,
				       gchar *speeddial)
{
  if (addressbook && !gnomemeeting_addressbook_is_local (addressbook)) 
    return gnomemeeting_remote_addressbook_get_contacts (addressbook,
							 nbr,
							 partial_match,
							 fullname,
							 url,
							 categorie,
							 location,
							 speeddial);
  else
    return gnomemeeting_local_addressbook_get_contacts (addressbook,
							nbr,
							partial_match,
							fullname,
							url, 
							categorie,
							location,
							speeddial);
}


gboolean 
gnomemeeting_addressbook_add (GmAddressbook *addressbook)
{
  g_return_val_if_fail (addressbook != NULL, FALSE);

  if (gnomemeeting_addressbook_is_local (addressbook))
    return gnomemeeting_local_addressbook_add (addressbook);
  else
    return gnomemeeting_remote_addressbook_add (addressbook);
}


gboolean 
gnomemeeting_addressbook_delete (GmAddressbook *addressbook)
{
  g_return_val_if_fail (addressbook != NULL, FALSE);

  if (gnomemeeting_addressbook_is_local (addressbook))
    return gnomemeeting_local_addressbook_delete (addressbook);
  else
    return gnomemeeting_remote_addressbook_delete (addressbook);
}


gboolean 
gnomemeeting_addressbook_modify (GmAddressbook *addressbook)
{
  g_return_val_if_fail (addressbook != NULL, FALSE);

  if (gnomemeeting_addressbook_is_local (addressbook))
    return gnomemeeting_local_addressbook_modify (addressbook);
  else
    return gnomemeeting_remote_addressbook_modify (addressbook);
}


gboolean 
gnomemeeting_addressbook_is_local (GmAddressbook *addressbook)
{
  g_return_val_if_fail (addressbook != NULL, TRUE);

  if (addressbook->url == NULL)
    return TRUE; 

  if (addressbook->url 
      && g_str_has_prefix (addressbook->url, "file:"))
    return TRUE;


  return FALSE;
}


gboolean
gnomemeeting_addressbook_add_contact (GmAddressbook *addressbook,
                                      GmContact *ctact)
{
  g_return_val_if_fail (ctact != NULL, FALSE);
  g_return_val_if_fail (addressbook != NULL, FALSE);

  if (gnomemeeting_addressbook_is_local (addressbook))
    return gnomemeeting_local_addressbook_add_contact (addressbook, ctact);

  return FALSE;
}


gboolean
gnomemeeting_addressbook_delete_contact (GmAddressbook *addressbook,
					 GmContact *ctact)
{
  g_return_val_if_fail (ctact != NULL, FALSE);
  g_return_val_if_fail (addressbook != NULL, FALSE);

  if (gnomemeeting_addressbook_is_local (addressbook))
    return gnomemeeting_local_addressbook_delete_contact (addressbook, ctact);

  return FALSE;
}


gboolean
gnomemeeting_addressbook_modify_contact (GmAddressbook *addressbook,
					 GmContact *ctact)
{
  g_return_val_if_fail (ctact != NULL, FALSE);
  g_return_val_if_fail (addressbook != NULL, FALSE);

  if (gnomemeeting_addressbook_is_local (addressbook))
    return gnomemeeting_local_addressbook_modify_contact (addressbook, ctact);

  return FALSE;
}


gboolean
gnomemeeting_addressbook_is_editable (GmAddressbook *addressbook)
{
  g_return_val_if_fail (addressbook != NULL, FALSE);

  if (gnomemeeting_addressbook_is_local (addressbook))
    return gnomemeeting_local_addressbook_is_editable (addressbook);
  else
    return gnomemeeting_remote_addressbook_is_editable (addressbook);
}


void
gnomemeeting_addressbook_init (gchar *group_name, 
			       gchar *addressbook_name)
{
  g_return_if_fail (group_name != NULL && addressbook_name != NULL);
  
  gnomemeeting_local_addressbook_init (group_name, addressbook_name);
  gnomemeeting_remote_addressbook_init ();
}


gboolean 
gnomemeeting_addressbook_has_fullname (GmAddressbook *addressbook)
{
  g_return_val_if_fail (addressbook != NULL, FALSE);

  if (gnomemeeting_addressbook_is_local (addressbook))
    return gnomemeeting_local_addressbook_has_fullname (addressbook);
  else
    return gnomemeeting_remote_addressbook_has_fullname (addressbook);
}


gboolean 
gnomemeeting_addressbook_has_url (GmAddressbook *addressbook)
{
  g_return_val_if_fail (addressbook != NULL, FALSE);

  if (gnomemeeting_addressbook_is_local (addressbook))
    return gnomemeeting_local_addressbook_has_url (addressbook);
  else
    return gnomemeeting_remote_addressbook_has_url (addressbook);
}


gboolean 
gnomemeeting_addressbook_has_speeddial (GmAddressbook *addressbook)
{
  g_return_val_if_fail (addressbook != NULL, FALSE);

  if (gnomemeeting_addressbook_is_local (addressbook))
    return gnomemeeting_local_addressbook_has_speeddial (addressbook);
  else
    return gnomemeeting_remote_addressbook_has_speeddial (addressbook);
}


gboolean 
gnomemeeting_addressbook_has_categories (GmAddressbook *addressbook)
{
  g_return_val_if_fail (addressbook != NULL, FALSE);

  if (gnomemeeting_addressbook_is_local (addressbook))
    return gnomemeeting_local_addressbook_has_categories (addressbook);
  else
    return gnomemeeting_remote_addressbook_has_categories (addressbook);
}


gboolean 
gnomemeeting_addressbook_has_location (GmAddressbook *addressbook)
{
  g_return_val_if_fail (addressbook != NULL, FALSE);

  if (gnomemeeting_addressbook_is_local (addressbook))
    return gnomemeeting_local_addressbook_has_location (addressbook);
  else
    return gnomemeeting_remote_addressbook_has_location (addressbook);
}


gboolean 
gnomemeeting_addressbook_has_comment (GmAddressbook *addressbook)
{
  g_return_val_if_fail (addressbook != NULL, FALSE);

  if (gnomemeeting_addressbook_is_local (addressbook))
    return gnomemeeting_local_addressbook_has_comment (addressbook);
  else
    return gnomemeeting_remote_addressbook_has_comment (addressbook);
}


gboolean 
gnomemeeting_addressbook_has_software (GmAddressbook *addressbook)
{
  g_return_val_if_fail (addressbook != NULL, FALSE);

  if (gnomemeeting_addressbook_is_local (addressbook))
    return gnomemeeting_local_addressbook_has_software (addressbook);
  else
    return gnomemeeting_remote_addressbook_has_software (addressbook);
}


gboolean 
gnomemeeting_addressbook_has_email (GmAddressbook *addressbook)
{
  g_return_val_if_fail (addressbook != NULL, FALSE);

  if (gnomemeeting_addressbook_is_local (addressbook))
    return gnomemeeting_local_addressbook_has_email (addressbook);
  else
    return gnomemeeting_remote_addressbook_has_email (addressbook);
}


gboolean 
gnomemeeting_addressbook_has_state (GmAddressbook *addressbook)
{
  g_return_val_if_fail (addressbook != NULL, FALSE);

  if (gnomemeeting_addressbook_is_local (addressbook))
    return gnomemeeting_local_addressbook_has_state (addressbook);
  else
    return gnomemeeting_remote_addressbook_has_state (addressbook);
}
