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
 *                         gtk-text-buffer-extensions.c  -  description
 *                         --------------------------------------------
 *   begin                : Tue May 13 2003
 *   copyright            : (C) 2003 by Miguel Rodríguez,
 *                                      StÃ©phane Wirtel
 *                                      Kenneth Christiansen
 *                                      and Julien Puydt
 *   description          : This file contains the smiles detection code,
 *                          and the uri open/copy popup code
 *
 */

#include "../config.h"
#include "gtk-text-buffer-extentions.h"
#include <glib.h>
#include <string.h>
#include <regex.h>
#include "../pixmaps/inline_emoticons.h"
#ifndef DISABLE_GNOME
#include <gnome.h>
#endif

#ifndef _
#ifdef DISABLE_GNOME
#include <libintl.h>
#define _(x) gettext(x)
#ifdef gettext_noop
#define N_(String) gettext_noop (String)
#else
#define N_(String) (String)
#endif
#endif
#endif

// here is the code of the smiles' detection:

/**
 * struct smile_detect
 * @code : the code of the symbol
 * @data : the representation of the symbol
 * @n_children : number of children
 * @children : next node in the tree
 **/
typedef struct smile_detect_ {
  char code;  // The code of the symbol
  GdkPixbuf *data;
  int n_children;
  struct smile_detect_ **children;
} smile_detect;


/**
 * add_new_children
 * @code : the code of the symbol
 * @parent : the pointer to the parent of the new node
 *
 * Adds a new node in the tree, it's very important to insert 
 * a new smiley.
 **/
static smile_detect *
add_new_children (char code,    
		  smile_detect *parent) 
{
  smile_detect *node = NULL;
  
  parent->children = 
    (smile_detect **) g_realloc (parent->children, 
				 (parent->n_children + 1) * 
				 sizeof (smile_detect *));
  node = (smile_detect *) g_malloc (sizeof (smile_detect));
  parent->children[parent->n_children++] = node;
    
  node->code = code;
  node->data = NULL;
  node->n_children = 0;
  node->children = NULL;

  return node;
}


/**
 * locate_code
 * @code : the code of symbol
 * @iter : a node of the tree
 *
 * Look for a code of a symbol in the tree.
 **/
static smile_detect *
locate_code (char code, 
	     const smile_detect *iter)
{
  int i;

  for (i = 0; i < iter->n_children; i++) {
    if (iter->children[i]->code == code)
      return iter->children[i];
  }

  return NULL;
}


/**
 * add_smile
 * @root : the root of the tree != NULL ( if NULL --> not allocated )
 * @pattern : the pattern is the symbol by ex : ':-)', ':D', etc...
 * @data : contains the file name of the emoticon
 *
 * If it did not find a copy of the symbol, thus this functions add this symbol
 * in the tree with the add_new_children function.
 **/
static void 
add_smile (smile_detect *root,
	   const char *pattern, 
	   GdkPixbuf *data)
{
  int i;
  smile_detect *iter = root;
  
  for (i= 0 ; i < strlen (pattern); i++) {
    smile_detect *node = locate_code (pattern[i], iter);
    if (node == NULL)
      node = add_new_children (pattern[i], iter);

    iter = node;
  }

  iter->data = data;
  g_object_ref (G_OBJECT (data));
}


/*
 * Initialize the tree
 */
static void 
smiley_tree_init (smile_detect *root)
{
  struct _table_smiley
  {
    const char *symbol;
    GdkPixbuf **data;
  };
  
  GdkPixbuf *pixbufs[28];

  const struct _table_smiley table_smiley [] = 
    {
      {":)", &pixbufs[0]},
      {"8)", &pixbufs[1]},
      {"8-)", &pixbufs[1]},
      {";)", &pixbufs[2]},
      {";-)", &pixbufs[2]},
      {":(", &pixbufs[3]},
      {":-(", &pixbufs[3]},
      {":0", &pixbufs[4]},
      {":o", &pixbufs[4]},
      {":-0", &pixbufs[4]},
      {":-o", &pixbufs[4]},
      {":-D", &pixbufs[5]},
      {":D", &pixbufs[5]},
      {":-)", &pixbufs[6]},
      {":|", &pixbufs[7]},
      {":-|", &pixbufs[7]},
      {":-/", &pixbufs[8]},
      /*      {":/", &pixbufs[8]},*/
      {":-P", &pixbufs[9]},
      {":-p", &pixbufs[9]},
      {":P", &pixbufs[9]},
      {":p", &pixbufs[9]},
      {":'(", &pixbufs[10]},
      {":[", &pixbufs[11]},
      {":-*", &pixbufs[12]},
      {":-x", &pixbufs[13]},
      {"B-)", &pixbufs[14]},
      {"B)", &pixbufs[14]},
      {"x*O", &pixbufs[15]},
      {"(.)", &pixbufs[16]},
      {"(|)", &pixbufs[17]},
      {":-.", &pixbufs[18]},
      {"X)", &pixbufs[19]},
      {"X|", &pixbufs[20]},
      {"X(", &pixbufs[21]},
      {"}:)", &pixbufs[22]},
      {"|)", &pixbufs[23]},
      {"}:(", &pixbufs[24]},
      {"|(", &pixbufs[25]},
      {"|-(", &pixbufs[26]},
      {"|-)", &pixbufs[27]},
      NULL
    };
  const struct _table_smiley *tmp;
 
  pixbufs[0] = gdk_pixbuf_new_from_inline (-1, gm_emoticon_face1, FALSE, NULL);
  pixbufs[1] = gdk_pixbuf_new_from_inline (-1, gm_emoticon_face2, FALSE, NULL);
  pixbufs[2] = gdk_pixbuf_new_from_inline (-1, gm_emoticon_face3, FALSE, NULL);
  pixbufs[3] = gdk_pixbuf_new_from_inline (-1, gm_emoticon_face4, FALSE, NULL);
  pixbufs[4] = gdk_pixbuf_new_from_inline (-1, gm_emoticon_face5, FALSE, NULL);
  pixbufs[5] = gdk_pixbuf_new_from_inline (-1, gm_emoticon_face6, FALSE, NULL);
  pixbufs[6] = gdk_pixbuf_new_from_inline (-1, gm_emoticon_face7, FALSE, NULL);
  pixbufs[7] = gdk_pixbuf_new_from_inline (-1, gm_emoticon_face8, FALSE, NULL);
  pixbufs[8] = gdk_pixbuf_new_from_inline (-1, gm_emoticon_face9, FALSE, NULL);
  pixbufs[9] = gdk_pixbuf_new_from_inline (-1, gm_emoticon_face10, 
					   FALSE, NULL);
  pixbufs[10] = gdk_pixbuf_new_from_inline (-1, gm_emoticon_face11, 
					    FALSE, NULL);
  pixbufs[11] = gdk_pixbuf_new_from_inline (-1, gm_emoticon_face12, 
					    FALSE, NULL);
  pixbufs[12] = gdk_pixbuf_new_from_inline (-1, gm_emoticon_face13, 
					    FALSE, NULL);
  pixbufs[13] = gdk_pixbuf_new_from_inline (-1, gm_emoticon_face14, 
					    FALSE, NULL);
  pixbufs[14] = gdk_pixbuf_new_from_inline (-1, gm_emoticon_face15, 
					    FALSE, NULL);
  pixbufs[15] = gdk_pixbuf_new_from_inline (-1, gm_emoticon_face16, 
					    FALSE, NULL);
  pixbufs[16] = gdk_pixbuf_new_from_inline (-1, gm_emoticon_face17, 
					    FALSE, NULL);
  pixbufs[17] = gdk_pixbuf_new_from_inline (-1, gm_emoticon_face18, 
					    FALSE, NULL);
  pixbufs[18] = gdk_pixbuf_new_from_inline (-1, gm_emoticon_face19, 
					    FALSE, NULL);
  pixbufs[19] = gdk_pixbuf_new_from_inline (-1, gm_emoticon_dead_happy, 
					    FALSE, NULL);
  pixbufs[20] = gdk_pixbuf_new_from_inline (-1, gm_emoticon_dead, 
					    FALSE, NULL);
  pixbufs[21] = gdk_pixbuf_new_from_inline (-1, gm_emoticon_dead_sad, 
					    FALSE, NULL);
  pixbufs[22] = gdk_pixbuf_new_from_inline (-1, gm_emoticon_happy_devil, 
					    FALSE, NULL);
  pixbufs[23] = gdk_pixbuf_new_from_inline (-1, gm_emoticon_nose_glasses, 
					    FALSE, NULL);
  pixbufs[24] = gdk_pixbuf_new_from_inline (-1, gm_emoticon_sad_devil, 
					    FALSE, NULL);
  pixbufs[25] = gdk_pixbuf_new_from_inline (-1, gm_emoticon_sad_glasses, 
					    FALSE, NULL);
  pixbufs[26] = gdk_pixbuf_new_from_inline (-1, gm_emoticon_sad_nose_glasses, 
					    FALSE, NULL);
  pixbufs[27] = gdk_pixbuf_new_from_inline (-1, gm_emoticon_happy_nose_glasses, 
					    FALSE, NULL);

  for (tmp = table_smiley; tmp->symbol && tmp->data; tmp++) {
    add_smile (root, tmp->symbol, *(tmp->data));
  }
}


/**
 * lookup_smile
 * @root : root of the tree
 * @input : 
 * @s_len : 
 * Search a smiley in the tree, if it does not exist, this function returns NULL
 **/
static GdkPixbuf *
lookup_smile (const smile_detect *root,
	      const char *input, 
	      int *s_len)
{
  const smile_detect *iter;
  int i;

  for (iter = root, i = 1; iter != NULL; input++, i++) {
    iter = locate_code (*input, iter);
    if (iter && iter->data) {
      *s_len = i;
      return iter->data;
    }
  }
  
  return NULL;
}


/*
 * Search a symbol 
 * @input : the text to check
 * @ini : the start of the selection
 * @end : the end of the selection
 */
/*
 * BUGS: If one symbol is a prefix of another symbol, the prefix symbol is 
 * always returned.
 */
static GdkPixbuf *
search_symbol (const smile_detect *root, 
	       const char *input, 
	       int *ini, 
	       int *end)
{
  int i;
  int len = strlen (input);

  for (i = 0; i < len; i++) {
    int s_len;

    GdkPixbuf *smile = lookup_smile (root, input + i, &s_len);
    if (smile != NULL) {
      *ini = i;
      *end = i + s_len;
      return smile;
    }
  }

  *end = len;
  return NULL;
}


/**
 * gtk_text_buffer_insert_with_emoticons:
 * @buf: A pointer to a GtkTextBuffer
 * @bufiter: An iterator for the buffer
 * @text: A text string
 *
 * Inserts @test into the @buf, but with all smilies shown
 * as pictures and not text.
 **/
static void
gtk_text_buffer_insert_with_emoticons (GtkTextBuffer *buf,
                                       GtkTextIter   *bufiter, 
                                       const char    *text)
{
  char *i, *buffer;
  static smile_detect root = { 0, NULL, 0, NULL };

  if (!root.n_children)
    smiley_tree_init (&root);

  buffer = g_strdup (text);
  for (i = buffer; *i != '\0'; ) {
      int ini, end;

      GdkPixbuf *emoticon = search_symbol (&root, i, &ini, &end);
      if (emoticon == NULL) 	{
	gtk_text_buffer_insert (buf, bufiter, i, -1);
      }
      else {
	i[ini] = '\0';
	gtk_text_buffer_insert (buf, bufiter, i, -1);
	gtk_text_buffer_insert_pixbuf (buf, bufiter, emoticon);
      }
      i += end;
  }
  
  g_free (buffer);
}


/* here is the code of the uri popup code */
static regex_t *
uri_regexp_new ()
{
  regex_t *reg = NULL;

  reg = (regex_t *) g_malloc (sizeof(regex_t));

  if(regcomp (reg, "\\<([s]?(ht|f)tp://[^[:blank:]]+)\\>",
	      REG_EXTENDED) != 0) {

      regfree (reg);
      reg = NULL; /* will serve as a signal of our failure */
  }

  return reg;
}


static gboolean
uri_event (GtkTextTag *texttag,
	   GObject *arg1,
	   GdkEvent *event,
	   GtkTextIter *iter,
	   gpointer user_data)
{
  GtkTextIter *start = NULL;
  GtkTextIter *end = NULL; 

  gchar *uri = NULL;

  if (event->type == GDK_BUTTON_PRESS && event->button.button == 3) {

    start = gtk_text_iter_copy (iter);
    end = gtk_text_iter_copy (iter);

    gtk_text_iter_backward_to_tag_toggle (start, texttag);
    gtk_text_iter_forward_to_tag_toggle (end, texttag);

    uri = gtk_text_buffer_get_slice (gtk_text_iter_get_buffer (iter),
				     start, end, FALSE); 

    g_object_set_data_full (G_OBJECT (user_data), "clicked-uri", 
			    uri, g_free);

    gtk_menu_popup ((GtkMenu *) user_data, NULL, NULL, NULL, NULL,
		    event->button.button, event->button.time);

    gtk_text_iter_free (start);
    gtk_text_iter_free (end);

    return TRUE;
  }

  return FALSE;
}


static GtkTextTag *
uri_tag_new (GtkTextBuffer *buffer, GtkWidget *popup)
{
  GtkTextTag *tag = NULL;

  tag = gtk_text_buffer_create_tag (buffer, "uri-tag",
				    "foreground", "blue",
				    "underline", TRUE,
				    NULL);

  g_signal_connect (tag, "event", (GtkSignalFunc) uri_event, popup);

  return tag;
}


#ifndef DISABLE_GNOME
static void
open_url_callback (GtkWidget *widget, gpointer data)
{
  gchar *uri = NULL;

  uri = g_object_get_data (G_OBJECT(data), "clicked-uri");

  if(uri != NULL)
    gnome_url_show (uri, NULL);
}
#endif


static void
copy_url_callback (GtkWidget *widget, gpointer data)
{ 
  gchar *uri = NULL;

  uri = g_object_get_data (G_OBJECT(data), "clicked-uri");

  if (uri != NULL)
    gtk_clipboard_set_text (gtk_clipboard_get (GDK_SELECTION_PRIMARY),
			    uri, -1);
}


static GtkWidget *
uri_popup_new ()
{
  GtkWidget *menu_item = NULL;
  GtkWidget *popup = NULL;

  popup = gtk_menu_new ();

#ifndef DISABLE_GNOME
  menu_item =
    gtk_menu_item_new_with_label (_("Open Link"));
  gtk_widget_show (menu_item);
  gtk_menu_shell_append (GTK_MENU_SHELL(popup), menu_item);
  g_signal_connect_after (GTK_OBJECT(menu_item), "activate",
			  (GCallback) open_url_callback, popup);
#endif
  menu_item =
    gtk_menu_item_new_with_label (_("Copy Link Address"));
  gtk_widget_show (menu_item);
  gtk_menu_shell_append (GTK_MENU_SHELL(popup), menu_item);
  g_signal_connect_after (GTK_OBJECT(menu_item), "activate",
			  (GCallback) copy_url_callback, popup); 
  return popup;
}


/**
 * gtk_text_buffer_insert_with_uris:
 * @buf: A pointer to a GtkTextBuffer
 * @bufiter: An iterator for the buffer
 * @text: A text string
 *
 * Inserts @test into the @buf, but with all uris shown
 * as links and not text.
 **/
static void
gtk_text_buffer_insert_with_uris (GtkTextBuffer *buf,
				  GtkTextIter *bufiter,
				  const char *text)
{
  gint match;
  regmatch_t uri_match;

  regex_t *uri_reg = NULL; 

  gchar *buffer = NULL; 
  gchar *beginning = NULL;
  gchar *uri = NULL;
  gchar *rest = NULL;

  GtkWidget *uri_popup = NULL;
  GtkTextTag *uri_tag = NULL;

  buffer = g_strdup (text);
  uri_reg = g_object_get_data (G_OBJECT (buf), "uri-reg");

  if (!uri_reg) {

      uri_reg = uri_regexp_new ();
      g_object_set_data_full (G_OBJECT (buf), "uri-reg", uri_reg,
			      (GDestroyNotify) regfree);
  } 

  if (!uri_reg) { /* we failed to compile the regex! */

    gtk_text_buffer_insert_with_emoticons (buf, bufiter, buffer);
    g_free (buffer);

    return;
  }

  uri_popup = g_object_get_data (G_OBJECT (buf), "uri-popup");
  if (!uri_popup) {

      uri_popup = uri_popup_new ();
      g_object_set_data_full (G_OBJECT(buf), "uri-popup", uri_popup,
			      (GDestroyNotify) gtk_widget_destroy);
  }

  uri_tag = gtk_text_tag_table_lookup (gtk_text_buffer_get_tag_table (buf), 
				       "uri-tag");
  if (!uri_tag)
    uri_tag = uri_tag_new (buf, uri_popup);


  match = regexec (uri_reg, buffer, 1, &uri_match, 0);
  while (!match) { /* as long as there is an url to treat */
      
    /* if the match isn't at the beginning, we treat it as simple text */
    if (uri_match.rm_so) {
 
	  beginning = g_strndup (buffer, uri_match.rm_so);
	  gtk_text_buffer_insert_with_emoticons (buf, bufiter, beginning);
	  g_free (beginning);
    }/*  */

    /* treat the uri we found */
    uri = g_strndup (buffer + uri_match.rm_so,
		     uri_match.rm_eo - uri_match.rm_so);

    gtk_text_buffer_insert_with_tags (buf, bufiter, uri, 
				      -1, uri_tag, NULL);

    /* the rest will be our new buffer on next loop */
    rest = g_strdup (buffer + uri_match.rm_eo);
    g_free (buffer);
    buffer = rest;
    match = regexec (uri_reg, buffer, 1, &uri_match, 0);
  }
  /* we treat what's left after we found all uris */
  gtk_text_buffer_insert_with_emoticons (buf, bufiter, buffer);

  g_free (buffer); 
}


/**
 * gtk_text_buffer_insert_with_addons:
 * @buf: A pointer to a GtkTextBuffer
 * @bufiter: An iterator for the buffer
 * @text: A text string
 *
 * Inserts @test into the @buf, but with some parts shown
 * in a more userfriendly fashion (emoticons, uris, ...).
 **/
void
gtk_text_buffer_insert_with_addons (GtkTextBuffer *buf,
				    GtkTextIter *bufiter,
				    const char *text)
{
  gtk_text_buffer_insert_with_uris (buf, bufiter, text);
}
