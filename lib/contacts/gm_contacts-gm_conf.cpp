#include "gm_contacts.h"

GmContact *
gm_contact_new ()
{
  GmContact *contact = NULL;
 
  contact = g_new (GmContact, 1);
  
  contact->fullname = NULL;
  contact->categories = NULL;
  contact->url = NULL;
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
  return NULL;
}

void gm_addressbook_delete (GmAddressbook *)
{
  return;
}

GSList *
gnomemeeting_get_local_addressbooks ()
{
  return NULL;
}

GSList *
gnomemeeting_addressbook_get_contacts (GmAddressbook *,
				       gchar *)
{
  return NULL;
}

gboolean
gnomemeeting_addressbook_modify_contact (GmAddressbook *,
					 GmContact *)
{
  return FALSE;
}
