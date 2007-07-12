
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


/*
 *                         gtk-text-buffer-addon.c  -  description 
 *                         ------------------------------------
 *   begin                : Sat Nov 29 2003, but based on older code
 *   copyright            : (C) 2000-2006 by Julien Puydt
 *                                           Miguel Rodríguez,
 *                                           Stéphane Wirtel
 *                                           Kenneth Christiansen
 *   description          : Add-on functions for regex-based context menus
 *
 */


#include "gmtextbufferaddon.h"

#if defined(P_FREEBSD)
#include <sys/types.h>
#endif

#include <regex.h>

#define GM_REGEX_SPECIAL_CHARS "\\.[]()*+?{}|^$"


struct regex_match
{
  const gchar *buffer;
  regoff_t start;
  regoff_t end;
  GtkTextTag *tag;
};

static void
find_match (GtkTextTag *tag, gpointer data)
{
  gint match;
  regmatch_t regmatch;
  struct regex_match *smatch = data;
  
  regex_t *regex = g_object_get_data (G_OBJECT(tag), "regex");

  if(regex != NULL) { // we are concerned only by the tags that use the regex addon!
    match = regexec (regex, smatch->buffer, 1, &regmatch, 0);
    if (!match) { // there's an uri that matches
      if (smatch->tag == NULL
	  || (smatch->tag != NULL
	      && regmatch.rm_so < smatch->start)) {
	smatch->start = regmatch.rm_so;
	smatch->end = regmatch.rm_eo;
	smatch->tag = tag;
      }
    }
  }
}

/**
 * gtk_text_buffer_insert_with_regex:
 * @buf: A pointer to a GtkTextBuffer
 * @bufiter: An iterator for the buffer
 * @text: A text string
 *
 * Inserts @text into the @buf, but with all matching regex shown
 * in a beautified way
 **/
void
gtk_text_buffer_insert_with_regex (GtkTextBuffer *buf,
				   GtkTextIter *bufiter,
				   const gchar *text)
{
  gchar *string = NULL;
  struct regex_match match;

  match.buffer = text;
  match.tag = NULL;
  gtk_text_tag_table_foreach (gtk_text_buffer_get_tag_table (buf),
			      find_match, &match);
  while (match.tag != NULL) { /* as long as there is an url to treat */
    RegexDisplayFunc func;

    /* if the match isn't at the beginning, we treat that beginning as simple text */
    if (match.start) {
      g_assert (match.start <= strlen (text));
      gtk_text_buffer_insert (buf, bufiter, text, match.start);
    }/*  */

    /* treat the uri we found */
    string = g_strndup (text + match.start,
			match.end - match.start);

    func = g_object_get_data (G_OBJECT(match.tag),
					       "regex-display");
    if (func == NULL) { /* this doesn't need any fancy stuff to be displayed */
      func = gtk_text_buffer_insert_plain;
    }
    (*func)(buf, bufiter, match.tag, string);
    g_free (string);
    /* the rest will be our new buffer on next loop */
    text += match.end;
    match.buffer = text;
    match.tag = NULL;
    gtk_text_tag_table_foreach (gtk_text_buffer_get_tag_table (buf),
				find_match, &match);
  }

  /* we treat what's left after we found all uris */
  gtk_text_buffer_insert (buf, bufiter, text, -1);
}

/**
 * gtk_text_buffer_insert_plain:
 * @buf: A pointer to a GtkTextBuffer
 * @iter: An iterator for the buffer
 * @tag: A pointer to a GtkTextTag
 * @text: A text string
 *
 * Inserts @text into the @buf, with the given tag, as simple text
 **/
void
gtk_text_buffer_insert_plain (GtkTextBuffer *buf,
			      GtkTextIter *iter,
			      GtkTextTag *tag,
			      const gchar *text)
{
  gtk_text_buffer_insert_with_tags (buf, iter, text, -1, tag, NULL);
}


/* here is the code used to display an emoticon/smiley, through the use
 * of a specialized function
 */

/* needed to store the association of a string with an image */
struct _smiley
{
  const char *smile;
  const char *icon_name;
};

/* the table that associates each smiley-as-a-text with a smiley-as-a-picture */
static const struct _smiley table_smiley [] =
  {
    {"O:-)", "face-angel"},
    {"0:-)", "face-angel"},
    {"O:)", "face-angel"},
    {"0:)", "face-angel"},
    {":'(", "face-crying"},
    {">:-)", "face-devil-grin"},
    {">:)", "face-devil-grin"},
    {"}:-)", "face-devil-grin"},
    {"}:)", "face-devil-grin"},
    {">:-(", "face-devil-sad"},
    {">:(", "face-devil-sad"},
    {"}:-(", "face-devil-sad"},
    {"}:(", "face-devil-sad"},
    {"B-)", "face-glasses"},
    {"B)", "face-glasses"},
    {":-*", "face-kiss"},
    {":*", "face-kiss"},
    {":-(|)", "face-monkey"},
    {":(|)", "face-monkey"},
    {":-|", "face-plain"},
    {":|", "face-plain"},
    {":-(", "face-sad"},
    {":(", "face-sad"},
    {":-)", "face-smile"},
    {":)", "face-smile"},
    {":-D", "face-smile-big"},
    {":D", "face-smile-big"},
    {":-!", "face-smirk"},
    {":!", "face-smirk"},
    {":-O", "face-surprise"},
    {":O", "face-surprise"},
    {":-0", "face-surprise"},
    {":0", "face-surprise"},
    {":-o", "face-surprise"},
    {":o", "face-surprise"},
    {";-)", "face-wink"},
    {";)", "face-wink"},
    {NULL}
  };


void
gtk_text_buffer_insert_smiley (GtkTextBuffer *buf,
			       GtkTextIter *iter,
			       GtkTextTag *tag,
			       const gchar *smile)
{
  GtkIconTheme *theme = gtk_icon_theme_get_default();
  GdkPixbuf *pixbuf = NULL;
  const struct _smiley *tmp;

  for (tmp = table_smiley;
       tmp->smile != NULL && tmp->icon_name != NULL;
       tmp++) {

    if (strcmp (smile, tmp->smile) == 0) {
      pixbuf = gtk_icon_theme_load_icon (theme, tmp->icon_name, 22, 0, NULL);
      break;
    }
  }

  if (pixbuf != NULL)
    gtk_text_buffer_insert_pixbuf (buf, iter, pixbuf);
  else 
    gtk_text_buffer_insert (buf, iter, smile, -1); 
}

/**
 * gtk_text_buffer_get_smiley_regex:
 *
 * Returns a regex suitable for matching smilies recognized by
 * gtk_text_buffer_insert_smiley ().
 **/
const gchar *
gtk_text_buffer_get_smiley_regex ()
{
  static gchar *regex = NULL;
  const struct _smiley *tmp = NULL;
  size_t regex_len = 0;
  size_t i, j;
  
  /* If we've built this before, just return it */
  if (regex != NULL)
    return regex;
  
  /* Discover total smiley length */
  for (tmp = table_smiley;
       tmp->smile != NULL;
       tmp++) {

    /* Twice len for backslashes + 3 for control characters ')', '(', and '|' */
    regex_len += strlen (tmp->smile) * 2 + 3;
  }
  
  /* Would normally add 1 for ending null, but we'll skip one of the allocated
     '|' characters, so there's an extra space available. */
  regex = (gchar *) g_malloc (regex_len);

  /* Now fill in regex string */
  j = 0;
  for (tmp = table_smiley;
       tmp->smile != NULL;
       tmp++) {

    if (j)
      regex[j++] = '|';

    regex[j++] = '(';
    for (i = 0; tmp->smile[i]; ++i) {
      if (strchr (GM_REGEX_SPECIAL_CHARS, tmp->smile[i]) != NULL)
      	regex[j++] = '\\';
      regex[j++] = tmp->smile[i];
    }
    regex[j++] = ')';
  }
  regex[j] = 0;

  return regex;
}

void
gtk_text_buffer_insert_markup (GtkTextBuffer *buf,
			       GtkTextIter *iter,
			       GtkTextTag *tag,
			       const gchar *markup)
{
  gchar *text = g_strdup (&markup [3]);
  text [strlen (text) - 4] = '\0';

  gtk_text_buffer_insert_with_tags (buf, iter, text, -1, tag, NULL); 

  g_free (text);
}
