#ifndef _GCONF_SIMU_H
#define _GCONF_SIMU_H

#include <glib.h>

#define FILE_CONFIG "gnomemeeting.schemas"
#define MAX_LIST_ELEMENT 10

typedef enum { /*< prefix=GCONF_VALUE >*/
  GCONF_VALUE_INVALID,
  GCONF_VALUE_STRING,
  GCONF_VALUE_INT,
  GCONF_VALUE_FLOAT,
  GCONF_VALUE_BOOL,
  GCONF_VALUE_SCHEMA,

  /* unfortunately these aren't really types; we want list_of_string,
     list_of_int, etc.  but it's just too complicated to implement.
     instead we'll complain in various places if you do something
     moronic like mix types in a list or treat pair<string,int> and
     pair<float,bool> as the same type. */
  GCONF_VALUE_LIST,
  GCONF_VALUE_PAIR
  
} GConfValueType;

typedef struct GConfValue_ {
	gchar *value;
  GConfValueType type;
} GConfValue;

typedef struct GConfEntry_ {
  gchar *key;
  GConfValue *value;
} GConfEntry;


typedef enum { /*< prefix=GCONF_ERROR >*/
  GCONF_ERROR_SUCCESS = 0,
  GCONF_ERROR_FAILED = 1,        /* Something didn't work, don't know why, probably unrecoverable
                                    so there's no point having a more specific errno */

  GCONF_ERROR_NO_SERVER = 2,     /* Server can't be launched/contacted */
  GCONF_ERROR_NO_PERMISSION = 3, /* don't have permission for that */
  GCONF_ERROR_BAD_ADDRESS = 4,   /* Address couldn't be resolved */
  GCONF_ERROR_BAD_KEY = 5,       /* directory or key isn't valid (contains bad
                                    characters, or malformed slash arrangement) */
  GCONF_ERROR_PARSE_ERROR = 6,   /* Syntax error when parsing */
  GCONF_ERROR_CORRUPT = 7,       /* Fatal error parsing/loading information inside the backend */
  GCONF_ERROR_TYPE_MISMATCH = 8, /* Type requested doesn't match type found */
  GCONF_ERROR_IS_DIR = 9,        /* Requested key operation on a dir */
  GCONF_ERROR_IS_KEY = 10,       /* Requested dir operation on a key */
  GCONF_ERROR_OVERRIDDEN = 11,   /* Read-only source at front of path has set the value */
  GCONF_ERROR_OAF_ERROR = 12,    /* liboaf error */
  GCONF_ERROR_LOCAL_ENGINE = 13, /* Tried to use remote operations on a local engine */
  GCONF_ERROR_LOCK_FAILED = 14,  /* Failed to get a lockfile */
  GCONF_ERROR_NO_WRITABLE_DATABASE = 15, /* nowhere to write a value */
  GCONF_ERROR_IN_SHUTDOWN = 16   /* server is shutting down */
} GConfError;

typedef enum { /*< prefix=GCONF_CLIENT >*/
  GCONF_CLIENT_HANDLE_NONE,
  GCONF_CLIENT_HANDLE_UNRETURNED,
  GCONF_CLIENT_HANDLE_ALL
} GConfClientErrorHandlingMode;

// Class GConfClient
typedef struct GConfClient_ {
	gint id;
} GConfClient;

#define GCONF_CLIENT (GConfClient*)

typedef void (*GConfClientNotifyFunc)(GConfClient *client,guint cnxn_id,GConfEntry *entry,gpointer user_data);
typedef void (*GConfClientErrorHandlerFunc)(GConfClient*,GError*);

typedef struct GConfIdle_ {
	GConfClientNotifyFunc notify_func;
	GConfEntry *entry;
	gpointer data;
} GConfIdle;

// Cache for read value 
#define CACHE_SIZE 5
typedef struct GConfCache_ {
	gint size;
	GSList *key;
	GHashTable *cache;
} GConfCache;

void			gconf_insert_to_cache(const gchar* key,gchar* data);

// List of functions

/* DESCRIPTION  :  
 * BEHAVIOR     :  Check if string is started by prefix
 * PRE          :  /
 */
gboolean 		gconf_str_start_at			(const gchar* str,const gchar* prefix);

/* DESCRIPTION  :  
 * BEHAVIOR     :  
 * PRE          :  /
 */
GString* 		gconf_rep_content				(GString* content,gchar* p_pos,const gchar* value,const gchar* fin);

/* DESCRIPTION  :  
 * BEHAVIOR     :  
 * PRE          :  /
 */
void			gconf_save_content_to_file();
gchar* 			gconf_read_file_content	(const gchar *filename,GError **err);
/* DESCRIPTION  :  
 * BEHAVIOR     :  
 * PRE          :  /
 */
gboolean 		gconf_write_file_content(const gchar *filename,const GString *content,GError **err);
/* DESCRIPTION  :  
 * BEHAVIOR     :  
 * PRE          :  /
 */
void* 			gconf_read_value				(const GString* content,const gchar* key,const GConfValueType g_type);
gchar* 			gconf_read_str_value		(const GString* content,const gchar* key,const GConfValueType g_type);
GSList* 		gconf_str_to_list				(gchar* str_value);
/* DESCRIPTION  :  
 * BEHAVIOR     :  
 * PRE          :  /
 */

GString* 		gconf_write_value				(GString* content,const gchar* key,const GConfValueType g_type,const void* value);
gchar* 			gconf_list_to_str				(GSList *list);

/* DESCRIPTION  :  
 * BEHAVIOR     :  
 * PRE          :  /
 */
void* 			gconf_client_get_value	(const gchar *key,const GConfValueType g_type,GError **err);
/* DESCRIPTION  :  
 * BEHAVIOR     :  
 * PRE          :  /
 */
gboolean		gconf_client_set_value	(const gchar *key,const GConfValueType g_type,const void* value,GError **err);

// Liste des fonctions remplacant

//void        (*GConfClientNotifyFunc)(GConfClient *client,guint cnxn_id,GConfEntry *entry,gpointer user_data);
//void        (*GConfClientErrorHandlerFunc)  (GConfClient *client,GError *error);

GConfClient*	gconf_client_get_default(); 

gdouble			gconf_client_get_float	(GConfClient *client,const gchar *key,GError **err);
gboolean 		gconf_client_set_float	(GConfClient *client,const gchar *key,gdouble val,GError **err);
gint 				gconf_client_get_int		(GConfClient *client,const gchar *key,GError **err);
gboolean 		gconf_client_set_int		(GConfClient *client,const gchar *key,gint val,GError **err);
gboolean 		gconf_client_get_bool		(GConfClient *client,const gchar *key,GError **err);
gboolean 		gconf_client_set_bool		(GConfClient *client,const gchar *key,gboolean val,GError **err);
gchar* 			gconf_client_get_string	(GConfClient *client,const gchar *key,GError **err);
gboolean 		gconf_client_set_string	(GConfClient *client,const gchar *key,const gchar *val,GError **err);
GSList*			gconf_client_get_list		(GConfClient *client,const gchar *key,GConfValueType list_type,GError **err);
gboolean 		gconf_client_set_list		(GConfClient *client,const gchar *key,GConfValueType list_type,GSList* list,GError **err);

const char* gconf_value_get_string   (const GConfValue *value);
int         gconf_value_get_int      (const GConfValue *value);
double      gconf_value_get_float    (const GConfValue *value);
gboolean    gconf_value_get_bool     (const GConfValue *value);
GSList*     gconf_value_get_list     (const GConfValue *value);

const char* gconf_entry_get_key      (const GConfEntry *entry);

gboolean    gconf_init              (int argc,char **argv,GError **err);
													
void        gconf_client_set_error_handling (GConfClient *client,GConfClientErrorHandlingMode mode);
void        gconf_client_set_global_default_error_handler(GConfClientErrorHandlerFunc error_handle_func);

gchar*			gconf_queue_key_pop();
gchar*			gconf_queue_value_pop();
gint				gconf_queue_type_pop();

gpointer gconf_event_loop(gpointer data);

guint gconf_client_notify_add (GConfClient *client,
				const gchar *namespace_section,
				GConfClientNotifyFunc notify_func,
				gpointer user_data,
				GFreeFunc destroy_notify,
				GError **err);

#endif

