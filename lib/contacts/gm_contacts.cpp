#include "gm_contacts.h"

#ifndef _GM_CONTACTS_H_INSIDE__
#define _GM_CONTACTS_H_INSIDE__
#include "gm_contacts-local.h"
#include "gm_contacts-remote.h"
#undef _GM_CONTACTS_H_INSIDE__
#endif


GSList *
gnomemeeting_addressbook_get_contacts (GmAddressbook *addressbook,
				       gboolean partial_match,
				       gchar *fullname,
				       gchar *url,
				       gchar *categorie,
				       gchar *speeddial)
{
  if (addressbook && !gnomemeeting_addressbook_is_local (addressbook)) 
    return gnomemeeting_remote_addressbook_get_contacts (addressbook,
							 partial_match,
							 fullname,
							 url,
							 categorie,
							 speeddial);
  else
    return gnomemeeting_local_addressbook_get_contacts (addressbook,
							partial_match,
							fullname,
							url, 
							categorie,
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
    return FALSE; //gnomemeeting_local_addressbook_modify (addressbook);
  else
    return gnomemeeting_remote_addressbook_modify (addressbook);
}


gboolean 
gnomemeeting_addressbook_is_local (GmAddressbook *addressbook)
{
  g_return_val_if_fail (addressbook != NULL, TRUE);

  if (!addressbook->url 
      || g_str_has_prefix (addressbook->url, "file:"))
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


void
gnomemeeting_addressbook_init (gchar *group_name, 
			       gchar *addressbook_name)
{
  g_return_if_fail (group_name != NULL && addressbook_name != NULL);
  
  gnomemeeting_local_addressbook_init (group_name, addressbook_name);
}
