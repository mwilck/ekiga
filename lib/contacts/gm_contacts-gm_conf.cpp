

#include <lib/gm_conf.h>
#include "gm_contacts.h"
#ifndef _GM_CONTACTS_H_INSIDE__
#define _GM_CONTACTS_H_INSIDE__
#include "gm_contacts-local.h"
#include "gm_contacts-convert.h"
#undef _GM_CONTACTS_H_INSIDE__
#endif
#include <stdlib.h>

/*
 * The following is the implementation of the local addressbooks, when
 * gnomemeeting is compiled without evolution-data-server support: it uses
 * the configuration system (gm_conf) to store the data.
 *
 * Everything is stored under CONTACTS_KEY, in the following form:
 * - an addressbook identifier is an integer starting at 1 ;
 * (stored as a string) ;
 * - the integer key CONTACTS_KEY "max_aid" stores the maximum addressbook
 * identifier (to know when to stop when searching the list of addressbooks:
 * there may be an empty slot in the middle after a removal, so looking for
 * empty slot isn't a good idea) ;
 * - the addressbook is stored in the CONTACTS_KEY (string of the aid)
 * namespace ;
 * - the addressbook features are directly stored in the namespace: for example
 * CONTACTS_KEY (string of the aid)"/name" stores the name of the
 * group ;
 * - a contact identifier is an integer starting at 1 (and has a meaning in the
 * addressbook only) ;
 * - a contact is stored as a vcard in the addressbook namespace:
 * CONTACTS_KEY (string of the aid) "/" (string of the uid) ;
 * - the integer key CONTACTS_KEY (string of the aid)"/max_uid" stores the
 * maximum contact identifier (for the same reasons as above).
 *
 * NB: starting the counts at 1 has the bonus that a 0 can mean "doesn't exist"
 */

#define CONTACTS_KEY "/apps/gnomemeeting/contacts/"


/*
 * Declaration of the helper functions
 */


/* this function retrieves the contact with the given "coordinates"
 * from the configuration ; it wants and checks aid > 0 and uid > 0.
 */
static GmContact *get_contact (gint aid, 
			       gint uid);


/* this function stores the given contact in the configuration, at the
 * given "coordinates" ; it wants and checks aid > 0, and contact != NULL
 * with a valid (> 0) uid.
 */
static gboolean store_contact (GmContact *contact, 
			       gint aid); 


/* this function retrieves the addressbook with the given identifier
 * from the configuration ; it wants and checks aid > 0.
 */
static GmAddressbook *get_addressbook (gint aid);


/* this function stores the addressbook in the configuration, at the
 * given identifier ; it wants and checks addb != NULL with a valid aid.
 */
static gboolean store_addressbook (GmAddressbook *addb);


/* this function returns the first available addressbook identifier
 * (that can be max_aid+1 or the old identifier of a removed group)
 */
static gint get_available_aid ();


/* this function returns the first available contact identifier
 * available in the given group (again, that can be either max_uid+1 or
 * the old identifier of a removed contact) [it wants and checks aid > 0]
 */
static gint get_available_uid (gint aid);


/*
 * Implementation of the helper functions
 */


static GmContact *
get_contact (gint aid, 
	     gint uid)
{
  gchar *vcard = NULL;
  gchar *key = NULL;
  GmContact *contact = NULL;

  g_return_val_if_fail (aid > 0, NULL);
  g_return_val_if_fail (uid > 0, NULL);
  
  key = g_strdup_printf (CONTACTS_KEY "%d/%d", aid, uid);
  vcard = gm_conf_get_string (key);
  g_free (key);

  if (vcard != NULL) {
    contact = vcard_to_gmcontact (vcard);
    contact->uid = g_strdup_printf ("%d", uid);
  }

  return contact;
}


static gboolean
store_contact (GmContact *contact, 
	       gint aid)
{
  gchar *vcard = NULL;
  gchar *key = NULL;
  gint uid = 0;
  gint max_uid = 0;

  g_return_val_if_fail (contact != NULL, FALSE);
  g_return_val_if_fail (contact->uid != NULL, FALSE);
  g_return_val_if_fail (aid > 0, FALSE);

  uid = strtol (contact->uid, NULL, 10);
  g_return_val_if_fail (uid > 0, FALSE);

  vcard = gmcontact_to_vcard (contact);
  key = g_strdup_printf (CONTACTS_KEY "%d/%d", aid, uid);
  gm_conf_set_string (key, vcard);
  g_free (key);  
  g_free (vcard);

  key = g_strdup_printf (CONTACTS_KEY "%d/max_uid", aid);
  max_uid = gm_conf_get_int (key);
  if (uid > max_uid)
    gm_conf_set_int (key, uid);
  g_free (key);

  return TRUE;
}


static GmAddressbook *
get_addressbook (gint aid)
{
  GmAddressbook *addb = NULL;
  gchar *key = NULL;
  gchar *str = NULL;

  g_return_val_if_fail (aid > 0, NULL);

  addb = gm_addressbook_new ();

  addb->aid = g_strdup_printf ("%d", aid);

  key = g_strdup_printf (CONTACTS_KEY "%d/name", aid);
  str = gm_conf_get_string (key);
  if (str != NULL)
    addb->name = g_strdup (str);
  g_free (key);

  key = g_strdup_printf (CONTACTS_KEY "%d/url", aid);
  str = gm_conf_get_string (key);
  if (str != NULL)
    addb->url = g_strdup (str);
  g_free (key);

  key = g_strdup_printf (CONTACTS_KEY "%d/call_attribute", aid);
  str = gm_conf_get_string (key);
  if (str != NULL)
    addb->call_attribute = g_strdup (str);
  g_free (key);

  if (addb->name == NULL
      && addb->url == NULL 
      && addb->call_attribute == NULL) {
    gm_addressbook_delete (addb);
    addb = NULL;
  }

  return addb;
}


static gboolean
store_addressbook (GmAddressbook *addb)
{
  gchar *key = NULL;
  gint max_aid = 0;
  gint aid = 0;

  g_return_val_if_fail (addb != NULL, FALSE);
  g_return_val_if_fail (addb->aid != NULL, FALSE);

  aid = strtol (addb->aid, NULL, 10);
  
  g_return_val_if_fail (aid > 0, FALSE);

  key = g_strdup_printf (CONTACTS_KEY "%d/name", aid);
  if (addb->name != NULL)
    gm_conf_set_string (key, addb->name);
  else
    gm_conf_destroy (key);
  g_free (key);

  key = g_strdup_printf (CONTACTS_KEY "%d/url", aid);
  if (addb->url != NULL)
    gm_conf_set_string (key, addb->url);
  else
    gm_conf_destroy (key);
  g_free (key);

  key = g_strdup_printf (CONTACTS_KEY "%d/call_attribute", aid);
  if (addb->call_attribute != NULL)
    gm_conf_set_string (key, addb->call_attribute);
  else
    gm_conf_destroy (key);
  g_free (key);

  max_aid = gm_conf_get_int (CONTACTS_KEY "max_aid");
  if (aid > max_aid)
    gm_conf_set_int (CONTACTS_KEY "max_aid", aid);

  return TRUE;
}


static gint
get_available_aid ()
{
  gint aid = 1;
  gint max_aid = 1;
  GmAddressbook *addb = NULL;

  max_aid = gm_conf_get_int (CONTACTS_KEY "max_aid");
  
  for (aid = 1; aid < max_aid + 1; aid++) {
    addb = get_addressbook (aid);
    if (addb == NULL)
      break;
    gm_addressbook_delete (addb);
  }
  
  return aid;
}


static gint
get_available_uid (gint aid)
{
  gint uid = 1;
  gint max_uid = 1;
  gchar *key = NULL;
  GmContact *contact = NULL;

  g_return_val_if_fail (aid > 0, 1);

  key = g_strdup_printf (CONTACTS_KEY "%d/max_uid", aid);
  max_uid = gm_conf_get_int (key);
  g_free (key);
  
  for (uid = 1; uid < max_uid + 1; uid++) {
    contact = get_contact (aid, uid);
    if (contact == NULL)
      break;
    gm_contact_delete (contact);
  }

  return uid;
}


/*
 * Implementation of the public api
 */


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

  g_free (contact->fullname);
  g_free (contact->url);
  g_free (contact->speeddial);
  g_free (contact->categories);
  g_free (contact->comment);
  g_free (contact->software);
  g_free (contact->email);
  g_free (contact->uid);

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
  gint i = 0;
  gint max_aid = 0;
  GmAddressbook *addb = NULL;
  GSList *result = NULL;

  max_aid = gm_conf_get_int (CONTACTS_KEY "max_aid");

  for (i = 1; i <= max_aid ; i++) {
    addb = get_addressbook (i);
    if (addb != NULL)
      result = g_slist_append (result, (gpointer)addb);
  }

  return result;
}


GSList *
gnomemeeting_local_addressbook_get_contacts (GmAddressbook *addb,
					     int & nbr,
					     gboolean partial_match,
					     gchar *fullname,
					     gchar *url,
					     gchar *categorie,
					     gchar *speeddial)
{
  /* FIXME: no search implemented! */
  gchar *key = NULL;
  gint aid = 0;
  gint uid = 0;
  gint max_uid = 0;
  gint max_aid = 0;
  GmContact *contact = NULL;
  GSList *result = NULL;
  GmAddressbook *addb_loop = NULL;

  if (addb != NULL) {
    
    aid = strtol (addb->aid, NULL, 10);
 
    g_return_val_if_fail (aid > 0, NULL);
   
    key = g_strdup_printf (CONTACTS_KEY "%d/max_uid", aid);
    max_uid = gm_conf_get_int (key);
    g_free (key);
    
    for (uid = 1; uid <= max_uid ; uid++) {
      contact = get_contact (aid, uid);
      if (contact != NULL)
	result = g_slist_append (result, (gpointer)contact);
    }
  }
  else {
    max_aid = gm_conf_get_int (CONTACTS_KEY "max_aid");
    for (aid = 1; aid <= max_aid; aid++) {
      addb_loop = get_addressbook (aid);
      if (addb_loop != NULL)
	result = g_slist_concat (result, gnomemeeting_local_addressbook_get_contacts (addb_loop, nbr, partial_match, fullname, url, categorie, speeddial));
    }
  }

  nbr = g_slist_length (result);

  return result;
}


gboolean
gnomemeeting_local_addressbook_add (GmAddressbook *addb)
{
  gint aid = 0;
  
  g_return_val_if_fail (addb != NULL, FALSE);

  aid = get_available_aid ();

  if (addb->aid) {

    g_free (addb->aid);
  }
  addb->aid = g_strdup_printf ("%d", aid);

  return store_addressbook (addb);
}


gboolean
gnomemeeting_local_addressbook_delete (GmAddressbook *addb)
{
  gint aid = 0;
  gint max_aid = 0;
  gchar *namespc = NULL;

  g_return_val_if_fail (addb != NULL, FALSE);

  aid = strtol (addb->aid, NULL, 10);

  g_return_val_if_fail (aid > 0, FALSE);

  namespc = g_strdup_printf (CONTACTS_KEY "%d", aid);
  gm_conf_destroy (namespc);
  g_free (namespc);

  max_aid = gm_conf_get_int (CONTACTS_KEY "max_aid");

  if (max_aid == aid) {
    /* FIXME: bad! Need a proper loop to detect the last really
     * used aid (another helper function would be nice, I think)
     */
    gm_conf_set_int (CONTACTS_KEY "max_aid", aid - 1);
  }

  return TRUE;
}


gboolean
gnomemeeting_local_addressbook_modify (GmAddressbook *addb)
{
  g_return_val_if_fail (addb != NULL, FALSE);
 
  return store_addressbook (addb);
}


gboolean
gnomemeeting_local_addressbook_delete_contact (GmAddressbook *addb,
					       GmContact *contact)
{
  gint aid = 0;
  gint uid = 0;
  gint max_uid = 0;
  gchar *namespc = NULL;
  gchar *key = NULL;

  g_return_val_if_fail (addb != NULL, FALSE);
  g_return_val_if_fail (contact != NULL, FALSE);

  aid = strtol (addb->aid, NULL, 10);
  g_return_val_if_fail (aid > 0, FALSE);

  uid = strtol (contact->uid, NULL, 10);
  g_return_val_if_fail (uid > 0, FALSE);

  namespc = g_strdup_printf (CONTACTS_KEY "%d/%d", aid, uid);
  gm_conf_destroy (namespc);
  g_free (namespc);

  key = g_strdup_printf (CONTACTS_KEY "%d/max_uid", aid);
  max_uid = gm_conf_get_int (key);
  g_free (key);

  if (max_uid == uid) {
    /* FIXME: bad! Need a proper loop to detect the last really
     * used uid (another helper function would be nice, I think)
     */
    key = g_strdup_printf (CONTACTS_KEY "%d/max_uid", aid);
    gm_conf_set_int (key, uid - 1);
    g_free (key);
  }

  return TRUE;
}


gboolean
gnomemeeting_local_addressbook_add_contact (GmAddressbook *addb,
					    GmContact *contact)
{
  gint aid = 0;
  gint uid = 0;

  g_return_val_if_fail (addb != NULL, FALSE);
  g_return_val_if_fail (contact != NULL, FALSE);

  aid = strtol (addb->aid, NULL, 10);
  g_return_val_if_fail (aid > 0, FALSE);

  uid = get_available_uid (aid);
  contact->uid = g_strdup_printf ("%d", uid);

  return store_contact (contact, aid);
}


gboolean
gnomemeeting_local_addressbook_modify_contact (GmAddressbook *addb,
					       GmContact *contact)
{
  gint aid = 0;

  g_return_val_if_fail (addb != NULL, FALSE);
  g_return_val_if_fail (contact != NULL, FALSE);

  aid = strtol (addb->aid, NULL, 10);
  g_return_val_if_fail (aid > 0, FALSE);

  return store_contact (contact, aid);
}


gboolean 
gnomemeeting_local_addressbook_is_editable (GmAddressbook *)
{
  return TRUE;
}


void
gnomemeeting_local_addressbook_init (gchar *group_name,
				     gchar *source_name)
{
}
