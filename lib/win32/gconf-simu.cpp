
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2003 Damien Sandras
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
 *                         gconf-simu.cpp  -  description
 *                         ------------------------------
 *   begin                : Thu Nov 21 2002
 *   copyright            : (C) 2002-2003 by Tuan
 *   description          : GConf simulation functions.
 *
 */


#ifdef WIN32
#include <windows.h>
#endif

#include "gconf-simu.h"

static gboolean content_changed=FALSE;
static GString *content=NULL;

static GConfCache gconf_cache;

static GAsyncQueue *queue_key=NULL,*queue_value=NULL,*queue_type=NULL;

static GHashTable *hashtable=NULL;

/* DESCRIPTION  :  
 * BEHAVIOR     :  Check if string is started by prefix
 * PRE          :  /
 */
gboolean gconf_str_start_at(const gchar* str,const gchar* prefix) {
	gboolean result;
	gchar *p;
	GString *g_str;
	g_str=g_string_new(str);
	p=g_strstr_len(g_str->str,g_str->len,prefix);
	if(p==g_str->str)
		result=TRUE;
	else
		result=FALSE;
	p=g_string_free(g_str,TRUE);
	return result;
};

/* DESCRIPTION  :  
 * BEHAVIOR     :  Save content of file
 * PRE          :  /
 */
void gconf_save_content_to_file() {
	GError **err=NULL;
	gboolean bResult=FALSE;
	if(content_changed) 
		bResult=gconf_write_file_content(FILE_CONFIG,content,err);
}

/* DESCRIPTION  :  
 * BEHAVIOR     :  Get file of content
 * PRE          :  /
 */
gchar* gconf_read_file_content(const gchar *filename,GError **err) {
	GIOChannel *f;
	GIOStatus status;
	gchar *file_buffer,*result;
	gsize size;
	result=NULL;
	f=g_io_channel_new_file(filename,"r",err);
	if(err!=NULL) {
		g_print("%s\n%s\n",filename,(*err)->message);
		return NULL;
	}
	if(f) {
		status=g_io_channel_read_to_end(f,&file_buffer,&size,err);
//		if(err!=NULL) g_print("%s\n%s\n",filename,(*err)->message);
		if(status==G_IO_STATUS_NORMAL) 	result=file_buffer;
		status=g_io_channel_shutdown(f,FALSE,NULL); 
	}
	return result;
};

/* DESCRIPTION  :  
 * BEHAVIOR     :  Put file of content
 * PRE          :  /
 */
gboolean gconf_write_file_content(const gchar *filename,const GString *content,GError **err) {
#ifndef WIN32
	GIOChannel *f;
	GIOStatus status;
	gsize size;
//	g_print("content=%s\n",content->str);
	f=g_io_channel_new_file(filename,"w",err);
	if(err!=NULL) {
		g_print("%s\n%s\n",filename,(*err)->message);
		return FALSE;
	}
	if(f) {
		status=g_io_channel_write_chars(f,content->str,content->len,&size,err);
		if(err!=NULL) {
			g_print("%s\n%s\n",filename,(*err)->message);
			return FALSE;
		}
//		g_print("size=%d\n",size);
		if(status==G_IO_STATUS_NORMAL) {
			status=g_io_channel_shutdown(f,TRUE,NULL);           
			return TRUE;
		}else {
			status=g_io_channel_shutdown(f,FALSE,NULL);           
			return FALSE;
		}
	}
	return FALSE;
#else
	HANDLE hFile;
	DWORD dwBytesWritten;
	hFile = CreateFile (filename,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		WriteFile (hFile, content->str, content->len,&dwBytesWritten, NULL);
		CloseHandle (hFile);  
		return TRUE;
	}
	return FALSE;
#endif
};

/* DESCRIPTION  :  
 * BEHAVIOR     :  Read value from key
 * PRE          :  /
 */
void* gconf_read_value(const GString* content,const gchar* key,const GConfValueType g_type) {
	gchar *str_value=NULL;
//	str_value=(gchar*)g_hash_table_lookup(gconf_cache.cache,key);
	if(str_value==NULL) {
		str_value=gconf_read_str_value(content,key,g_type);
//		gconf_insert_to_cache(key,str_value);		
	}
	if(g_type!=GCONF_VALUE_LIST) 
		return str_value;
	else 
		return gconf_str_to_list(str_value);
};
//----------------------------------------------------------------------------------
void gconf_insert_to_cache(const gchar* key,gchar* data) {
	gboolean b;
	gchar *key_remove=NULL;
	if(gconf_cache.size==CACHE_SIZE) {
		key_remove=(gchar*)(gconf_cache.key)->data;
		b=g_hash_table_remove(gconf_cache.cache,key_remove);
		gconf_cache.key=g_slist_remove(gconf_cache.key,(gpointer)key_remove);
		gconf_cache.size--;
		if(b) g_print("Remove key cache: %s\n",key_remove);
	}
	g_hash_table_insert(gconf_cache.cache,(gpointer)key,data);
	gconf_cache.key=g_slist_append(gconf_cache.key,(gpointer)key);
	gconf_cache.size++;

	g_print("Add key cache: %s\n",key);
};
//----------------------------------------------------------------------------------
GSList* gconf_str_to_list(gchar* str_value) {
	gchar **arr=NULL;
	GSList *gs_list=NULL;
	if(str_value==NULL) return NULL;
	arr=g_strsplit(str_value,",",(gint)MAX_LIST_ELEMENT);
	while(*arr) {
		gs_list=g_slist_append(gs_list,*arr);
//			g_print("value=%s\n",*arr);
		arr++;
	}
	return gs_list;
};
//----------------------------------------------------------------------------------
gchar* gconf_read_str_value(const GString* content,const gchar* key,const GConfValueType g_type) {
	gchar *start=NULL,*pos;
	GString *result=NULL;

	result=g_string_new("");
	start=g_strstr_len(content->str,content->len,"?>");
	if(start==NULL)
		start=g_strstr_len(content->str,content->len,key);
	else
		start=g_strstr_len(start,content->len,key);
//	g_print("key=%s\n",key);
	if(start==NULL) {
		g_print("In function: gconf_read_value(...)\n%s is not exist.\n",key);
		return NULL;
	}
	if(g_type!=GCONF_VALUE_LIST) {
		pos=g_strstr_len(start,content->len,"<default>");
		pos+=8;
//		g_print("value=%s\n",pos);
		while(!gconf_str_start_at(++pos,"</default>")) result=g_string_append_c(result,*pos);	
	} else {
		pos=g_strstr_len(start,content->len,"<default>[");
		pos+=9;
		while(!gconf_str_start_at(++pos,"]</default>")) result=g_string_append_c(result,*pos);
	}
	return result->str;
};

/* DESCRIPTION  :  
 * BEHAVIOR     :  Write new value into content
 * PRE          :  /
 */
//----------------------------------------------------------------------------------
gchar* gconf_list_to_str(GSList *list) {
	GString  *gs_str;
	gs_str=g_string_new("");
	while(list) {
		if(gs_str->len>0) gs_str=g_string_append(gs_str,",");
		gs_str=g_string_append(gs_str,(gchar*)list->data);
		list=list->next;
	}
	return gs_str->str;
};
//----------------------------------------------------------------------------------
GString* gconf_write_value(GString* content,const gchar* key,
											 const GConfValueType g_type,const void* value) {
	gchar *start,*p_pos;
	GSList *list=NULL;
	start=g_strstr_len(content->str,content->len,"?>");
	if(start==NULL)
		start=g_strstr_len(content->str,content->len,key);
	else
		start=g_strstr_len(start,content->len,key);
	if(start==NULL) {
		g_print("%s is not exist.\n",key);
		return NULL;
	}
	if(g_type!=GCONF_VALUE_LIST) {
		p_pos=g_strstr_len(start,content->len,"<default>");
		p_pos+=8;
		content=gconf_rep_content(content,p_pos,(gchar*)value,"</default>");
	}else {
		p_pos=g_strstr_len(start,content->len,"<default>[");
		p_pos+=9;
		content=gconf_rep_content(content,p_pos,gconf_list_to_str((GSList*)value),"]</default>");
	}
	return content;
};

/* DESCRIPTION	: When we need set new value 
 * BEHAVIOR			:	replace current content with new content
 * PRE					: /	
 */
GString* gconf_rep_content(GString* content,gchar* p_pos,const gchar* value,const gchar* fin) {
	gssize pos;
	gchar* p;
	GString *tail,*old_value;
	
	old_value=g_string_new("");
	tail=g_string_new(++p_pos);
	pos=content->len-tail->len;
	while(!gconf_str_start_at(p_pos,fin)) 
		old_value=g_string_append_c(old_value,*p_pos++);
	content=g_string_erase(content,pos,old_value->len);
	content=g_string_insert(content,pos,value);
	p=g_string_free(tail,TRUE);
	return content;
};

/* DESCRIPTION  :  
 * BEHAVIOR     :  Get value from string key
 * PRE          :  /
 */
void* gconf_client_get_value(const gchar *key,const GConfValueType g_type,GError **err) {
  gboolean bResult=FALSE;
  gchar *str=NULL;
  void *result=NULL;
//	g_print("Get: %s\n",key);
  if(content==NULL) {
    str=gconf_read_file_content(FILE_CONFIG,err);
   	if(err!=NULL) return NULL;
    content=g_string_new(str);
  }
  result=gconf_read_value(content,key,g_type);
  return result;
};

/* DESCRIPTION  :  
 * BEHAVIOR     :  Set value into file config
 * PRE          :  /
 */
gboolean gconf_client_set_value(const gchar *key,const GConfValueType g_type,const void* value,GError **err) {
	gchar *str=NULL;
	GString *g_value;

	if(content==NULL) {
	    str=gconf_read_file_content(FILE_CONFIG,err);
    	if(err!=NULL) return FALSE;
	    content=g_string_new(str);
	}
	
//	g_print("Key: %s - Value: %s\n",key,(gchar*)value);

	content=gconf_write_value(content,key,g_type,value);
	content_changed=TRUE;

	g_async_queue_push(queue_key,(gchar*)key);
	if(g_type!=GCONF_VALUE_LIST)		
	    g_async_queue_push(queue_value,(gchar*)value);
	else
	    g_async_queue_push(queue_value,(gchar*)"");
	g_value=g_string_new("");
	g_string_printf(g_value,"%d\0",g_type);
	g_async_queue_push(queue_type,g_value->str);

	gconf_event_loop(NULL);
	return TRUE;
};

//----------------------------------------------------------------------------------
// Liste des fonctions publiques
//----------------------------------------------------------------------------------
GConfClient* 	gconf_client_get_default() {
	return NULL;
};
//----------------------------------------------------------------------------------
gdouble gconf_client_get_float(GConfClient *client,const gchar *key,GError **err) {
	gchar *value;
	if((value=(gchar*)gconf_client_get_value(key,GCONF_VALUE_FLOAT,err))!=NULL) {
		return g_strtod(value,NULL);
	}
	return 0;
};
//----------------------------------------------------------------------------------
gboolean gconf_client_set_float(GConfClient *client,const gchar *key,gdouble val,GError **err) {
	GString *value;
	value=g_string_new("");
	g_string_printf(value,"%f\0",val);
	return gconf_client_set_value(key,GCONF_VALUE_FLOAT,value->str,err);
};
//----------------------------------------------------------------------------------
gint gconf_client_get_int(GConfClient *client,const gchar *key,GError **err) {
	gchar *value;
	if((value=(gchar*)gconf_client_get_value(key,GCONF_VALUE_INT,err))!=NULL) {
		return (gint)g_strtod(value,NULL);
	}
	return 0;
};
//----------------------------------------------------------------------------------
gboolean gconf_client_set_int(GConfClient *client,const gchar *key,gint val,GError **err) {
	GString *value;
	value=g_string_new("");
	g_string_printf(value,"%d\0",val);
	return gconf_client_set_value(key,GCONF_VALUE_INT,value->str,err);
};
//----------------------------------------------------------------------------------
gboolean gconf_client_get_bool(GConfClient *client,const gchar *key,GError **err) {
	gboolean bResult=FALSE;
	gchar *value;
	GString *value1,*value2;
	if((value=(gchar*)gconf_client_get_value(key,GCONF_VALUE_BOOL,err))!=NULL) {
		value1=g_string_new("true");
		value2=g_string_new(value);
		if(g_string_equal(value1,value2))	bResult=TRUE;
		g_string_free(value1,TRUE);
		g_string_free(value2,TRUE);
	}
	return bResult;
};
//----------------------------------------------------------------------------------
gboolean gconf_client_set_bool(GConfClient *client,const gchar *key,gboolean val,GError **err) {
	if(val)
		return gconf_client_set_value(key,GCONF_VALUE_BOOL,(gchar*)"true",err);
	else
		return gconf_client_set_value(key,GCONF_VALUE_BOOL,(gchar*)"false",err);
};
//----------------------------------------------------------------------------------
gchar* gconf_client_get_string(GConfClient *client,const gchar *key,GError **err) {
	return (gchar*)gconf_client_get_value(key,GCONF_VALUE_STRING,err);
};
//----------------------------------------------------------------------------------
gboolean gconf_client_set_string(GConfClient *client,const gchar *key,const gchar *val,GError **err) {
	return gconf_client_set_value(key,GCONF_VALUE_STRING,val,err);
};
//----------------------------------------------------------------------------------
GSList* gconf_client_get_list(GConfClient *client,const gchar *key,GConfValueType list_type,GError **err) {
	GSList *gs_list;
	gs_list=(GSList*)gconf_client_get_value(key,GCONF_VALUE_LIST,err);
//	g_print("Get list ...\n");
//	g_slist_foreach(gs_list,g_print_slist,NULL);
	return gs_list;
};
//----------------------------------------------------------------------------------
gboolean gconf_client_set_list(GConfClient *client,const gchar *key,
															 GConfValueType list_type,GSList* list,GError **err) {
//	g_print("Set list ...\n");
//	g_slist_foreach(list,g_print_slist,NULL);
	return gconf_client_set_value(key,GCONF_VALUE_LIST,list,err);
;
};

//----------------------------------------------------------------------------------
const char* gconf_value_get_string(const GConfValue *value) {
	return value->value;
};
//----------------------------------------------------------------------------------
int gconf_value_get_int(const GConfValue *value) {
	return (gint)g_strtod(value->value,NULL);
};
//----------------------------------------------------------------------------------
double gconf_value_get_float(const GConfValue *value) {
	return g_strtod(value->value,NULL);
};
//----------------------------------------------------------------------------------
gboolean gconf_value_get_bool(const GConfValue *value) {
	gboolean bResult;
	GString *value1,*value2;
	bResult=FALSE;
	value1=g_string_new("true");
	value2=g_string_new(value->value);
	if(g_string_equal(value1,value2))	bResult=TRUE;
	g_string_free(value1,TRUE);
	g_string_free(value2,TRUE);
	return bResult;
};
//----------------------------------------------------------------------------------
const char* gconf_entry_get_key (const GConfEntry *entry) {
	return entry->key;
};
//----------------------------------------------------------------------------------
gboolean gconf_init(int argc,char **argv,GError **err) {
	
	queue_key=g_async_queue_new();
	queue_value=g_async_queue_new();
	queue_type=g_async_queue_new();
	
	gconf_cache.size=0;
	gconf_cache.key=NULL;
	gconf_cache.cache=g_hash_table_new( g_str_hash,g_str_equal);

	hashtable=g_hash_table_new( g_str_hash,g_str_equal);

	return TRUE;
};
//----------------------------------------------------------------------------------
void gconf_client_set_error_handling (GConfClient *client,GConfClientErrorHandlingMode mode) {
//	Have not anything
};
//----------------------------------------------------------------------------------
void gconf_client_set_global_default_error_handler(GConfClientErrorHandlerFunc error_handle_func) {
//	Have not anything	
};

//----------------------------------------------------------------------------------
gchar*	gconf_queue_key_pop(){
	gchar* result=NULL;
	if(g_async_queue_length(queue_key)>0) result=(gchar*)g_async_queue_pop(queue_key);
	return result;
};
//----------------------------------------------------------------------------------
gchar*	gconf_queue_value_pop(){
	gchar* result=NULL;
	if(g_async_queue_length(queue_value)>0) result=(gchar*)g_async_queue_pop(queue_value);
	return result;
};
//----------------------------------------------------------------------------------
gint	gconf_queue_type_pop(){
	gint result=-1;
	gchar *str;
	if(g_async_queue_length(queue_type)>0) {
	    str=(gchar*)g_async_queue_pop(queue_type);
	    result=(gint)g_strtod(str,NULL);
	}
	return result;
};

#ifdef DISABLE_GCONF

/**
 * gconf_event_loop
 */

gpointer gconf_event_loop(gpointer data) {
	gchar *key=NULL;
	GConfValue g_value;
	GConfEntry g_entry;
	GSList *list=NULL,*list_data=NULL;

//	while(TRUE) {
		key=gconf_queue_key_pop();
		if(key!=NULL) {
//			g_print("Key: %s\n",key);
			g_value.value=gconf_queue_value_pop();
			g_value.type=(GConfValueType)gconf_queue_type_pop();
			g_entry.key=(gchar*)key;
			g_entry.value=&g_value;
			list=(GSList*)g_hash_table_lookup(hashtable,key);
			if(list!=NULL) {
				while(list!=NULL) {
					list_data=(GSList*)list->data;
					((GConfClientNotifyFunc)(list_data->data))(NULL,0,&g_entry,list_data->next->data);
					list=list->next;
				}
			}		
		}	
//	}
	return NULL;
};
/**
 * gconf_client_notify_add
 */
guint gconf_client_notify_add (GConfClient *client,
				const gchar *namespace_section,
				GConfClientNotifyFunc notify_func,
				gpointer user_data,
				GFreeFunc destroy_notify,
				GError **err) {																
	gboolean bAdd=FALSE;
	GSList *list=NULL,*list_data=NULL;

	if(hashtable==NULL) hashtable=g_hash_table_new( g_str_hash,g_str_equal);
	list=(GSList*)g_hash_table_lookup(hashtable,namespace_section);
	if(list==NULL) {
		bAdd=TRUE;
	}
	list_data=g_slist_append(list_data,notify_func);
	list_data=g_slist_append(list_data,user_data);
	list=g_slist_append(list,list_data);
	if(bAdd)g_hash_table_insert(hashtable,(gpointer)namespace_section,list);	
	return 0;
};

#endif
