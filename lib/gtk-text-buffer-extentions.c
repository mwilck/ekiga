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
 *                                      and Kenneth Christiansen
 *   description          : This file contains the smiles detection code.
 *
 */

#include "../config.h"
#include "gtk-text-buffer-extentions.h"
#include <glib.h>
#include <string.h>
#include "../pixmaps/inline_emoticons.h"


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
void
gtk_text_buffer_insert_with_emoticons (GtkTextBuffer *buf,
                                       GtkTextIter   *bufiter, 
                                       const char    *text)
{
  GdkPixbuf *emoticon;
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
