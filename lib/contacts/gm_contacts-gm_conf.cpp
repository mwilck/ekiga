

#include <lib/gm_conf.h>
#include "gm_contacts.h"
#ifndef _GM_CONTACTS_H_INSIDE__
#define _GM_CONTACTS_H_INSIDE__
#include "gm_contacts-local.h"
#undef _GM_CONTACTS_H_INSIDE__
#endif


GmContact *
gm_contact_new ()
{
  GmContact *contact = NULL;

  contact = g_new (GmContact, 1);


  contact->fullname = NULL;
  contact->categories = NULL;
  contact->url = NULL;
  contact->location = NULL;
  contact->speeddial = NULL;
  contact->comment = NULL;
  contact->software = NULL;
  contact->email = NULL;
  contact->state = 0;
  contact->video_capable = FALSE;
  contact->uid = NULL; 

  return contact;
}


void
gm_contact_delete (GmContact *contact)
{
  if (!contact)
    return;

  g_free (contact->uid);
  g_free (contact->fullname);
  g_free (contact->url);
  g_free (contact->speeddial);
  g_free (contact->categories);
  g_free (contact->comment);
  g_free (contact->software);
  g_free (contact->email);

  g_free (contact);
}


GmAddressbook *
gm_addressbook_new ()
{
  GmAddressbook *addressbook = NULL;

  addressbook = g_new (GmAddressbook, 1);
  
  addressbook->name = NULL;
  addressbook->url = NULL;
  addressbook->aid = NULL;
  addressbook->call_attribute = NULL;

  return addressbook;
}


void 
gm_addressbook_delete (GmAddressbook *addressbook)
{
  if (!addressbook)
    return;

  g_free (addressbook->url);
  g_free (addressbook->aid);
  g_free (addressbook->name);
  g_free (addressbook->call_attribute);

  g_free (addressbook);  
}


GSList *
gnomemeeting_get_local_addressbooks ()
{
  return NULL;
}


GSList *
gnomemeeting_local_addressbook_get_contacts (GmAddressbook *,
					     gboolean,
					     gchar *,
					     gchar *,
					     gchar *)
{
  return NULL;
}

gboolean
gnomemeeting_local_addressbook_add (GmAddressbook *)
{
  return FALSE;
}


gboolean
gnomemeeting_local_addressbook_delete (GmAddressbook *)
{
  return FALSE;
}


gboolean
gnomemeeting_local_addressbook_modify (GmAddressbook *)
{
  return FALSE;
}


gboolean
gnomemeeting_local_addressbook_delete_contact (GmAddressbook *,
					       GmContact *)
{
  return FALSE;
}

gboolean
gnomemeeting_local_addressbook_add_contact (GmAddressbook *,
					    GmContact *)
{
  return FALSE;
}

gboolean
gnomemeeting_local_addressbook_modify_contact (GmAddressbook *,
					       GmContact *)
{
  return FALSE;
}

