#include "gm_contacts.h"

#ifndef _GM_CONTACTS_H_INSIDE__
#define _GM_CONTACTS_H_INSIDE__
#include "gm_contacts-local.h"
#include "gm_contacts-remote.h"
#include "gm_contacts-dnd.h"
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
    return gnomemeeting_local_addressbook_modify (addressbook);
  else
    return gnomemeeting_remote_addressbook_modify (addressbook);
}


gboolean 
gnomemeeting_addressbook_is_local (GmAddressbook *addressbook)
{
  g_return_val_if_fail (addressbook != NULL, TRUE);

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


void
gnomemeeting_addressbook_init (gchar *group_name, 
			       gchar *addressbook_name)
{
  g_return_if_fail (group_name != NULL && addressbook_name != NULL);
  
  gnomemeeting_local_addressbook_init (group_name, addressbook_name);
}


void
gm_contacts_dnd_set_source (GtkWidget *widget,
			    GmDndGetContact helper, 
			    gpointer data)
{
  g_return_if_fail (widget != NULL);
  g_return_if_fail (helper != NULL);

  gtk_drag_source_set (widget, GDK_BUTTON1_MASK,
                       dnd_targets, DND_NUMBER_OF_TARGETS,
                       GDK_ACTION_COPY);

  g_signal_connect (G_OBJECT (widget), "drag_data_get",
                    G_CALLBACK (drag_data_get_cb), (gpointer)helper);

  g_object_set_data (G_OBJECT (widget), "GmDnD-Source", data);
}


void
gm_contacts_dnd_set_dest (GtkWidget *widget, 
			  GmDndPutContact helper,
			  gpointer data)
{
  g_return_if_fail (widget != NULL);
  g_return_if_fail (helper != NULL);

  gtk_drag_dest_set (widget,
                     GTK_DEST_DEFAULT_ALL,
                     dnd_targets, DND_NUMBER_OF_TARGETS,
                     GDK_ACTION_COPY);
  
  g_signal_connect (G_OBJECT (widget), "drag_data_received",
                    G_CALLBACK (drag_data_received_cb), (gpointer)helper);

  g_object_set_data (G_OBJECT (widget), "GmDnD-Target", data);
}


void
gm_contacts_dnd_set_dest_conditional (GtkWidget *widget, 
				      GmDndPutContact helper,
				      GmDndAllowDrop checker,
				      gpointer data)
{
  g_return_if_fail (widget != NULL);
  g_return_if_fail (helper != NULL);
  g_return_if_fail (checker != NULL);

  gm_contacts_dnd_set_dest (widget, helper, data);

  g_signal_connect (G_OBJECT (widget), "drag_motion",
		    G_CALLBACK (drag_motion_cb), (gpointer)checker);
}
