#include <gtk/gtk.h>

#ifndef __GTK_TEXT_BUFFER_EXT_H
#define __GTK_TEXT_BUFFER_EXT_H

G_BEGIN_DECLS

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
                                       const char    *text);

G_END_DECLS

#endif
