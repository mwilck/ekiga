#include "gtk-text-buffer-extentions.h"


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
  typedef enum {
    GM_EMOTICON_STATE_NO_SMILEY,
    GM_EMOTICON_EYES_NORMAL,
    GM_EMOTICON_EYES_ONE_CLOSED,
    GM_EMOTICON_HUGE_EYES,
        
    /* Noses and tears are ignored */
        
    GM_EMOTICON_STATE_BIG_SMILE,  /* ;D  */
    GM_EMOTICON_STATE_SHOCKED,    /* ;O  */
    GM_EMOTICON_STATE_SMILING,    /* :)  */
    GM_EMOTICON_STATE_EYE_BLINK,  /* ;)  */
    GM_EMOTICON_STATE_BIG_EYES,   /* 8)  */
    GM_EMOTICON_STATE_LOOKING_SAD /* :(  */
  } GmEmoticonState;
    
  GdkPixbuf *emoticon;
  GmEmoticonState state;
    
  char *iter = NULL;
  char *str = NULL;
  char *orig_str = NULL;
  char *emoticon_start = NULL;
  char *emoticon_end = NULL;
  gboolean end = FALSE;

  str = g_strdup_printf ("%s", text);
  orig_str = str;
    
  for (iter = str, state = GM_EMOTICON_STATE_NO_SMILEY;
       end == FALSE; iter++)
    {
      switch (state)
        {
        case GM_EMOTICON_EYES_NORMAL:
        case GM_EMOTICON_EYES_ONE_CLOSED:
        case GM_EMOTICON_HUGE_EYES:
	  switch (*iter)
            {
            case '-':
	      /* ignoring noses and tears*/
	      break;
                
            case 'D':
	      emoticon_end = iter + 1;
	      state = GM_EMOTICON_STATE_BIG_SMILE;
	      break;
                
            case '0': case 'O':
	      emoticon_end = iter + 1;
	      state = GM_EMOTICON_STATE_SHOCKED;
	      break;
                
            case '(':
	      emoticon_end = iter + 1;
	      state = GM_EMOTICON_STATE_LOOKING_SAD;
	      break;
                
            case ')':
	      switch (state)
                {
                case GM_EMOTICON_EYES_NORMAL:
		  emoticon_end = iter + 1;
		  state = GM_EMOTICON_STATE_SMILING;
		  break;
                    
                case GM_EMOTICON_EYES_ONE_CLOSED:
		  emoticon_end = iter + 1;
		  state = GM_EMOTICON_STATE_EYE_BLINK;
		  break;
                                        
                case GM_EMOTICON_HUGE_EYES:
		  emoticon_end = iter + 1;
		  state = GM_EMOTICON_STATE_BIG_EYES;
		  break;
                }
	      break;
                
            default:
	      state = GM_EMOTICON_STATE_NO_SMILEY;
	      break;
            }
	  break;
            
        case GM_EMOTICON_STATE_NO_SMILEY:
	  switch (*iter)
            {
                
            case ';':
	      emoticon_start = iter;
	      state = GM_EMOTICON_EYES_ONE_CLOSED;
	      break;
                
            case ':':
	      emoticon_start = iter;
	      state = GM_EMOTICON_EYES_NORMAL;
	      break;
                
            case '8':
	      emoticon_start = iter;
	      state = GM_EMOTICON_HUGE_EYES;
	      break;       
                
            default:
	      break;        
            }
	  break;
            
        case GM_EMOTICON_STATE_BIG_SMILE:   /* ;D  */
	  *emoticon_start = '\0';
            
	  gtk_text_buffer_insert (buf, bufiter, str, -1);
	  emoticon = gdk_pixbuf_new_from_inline (-1, inline_smiley_one, FALSE, NULL);
	  gtk_text_buffer_insert_pixbuf (buf, bufiter, emoticon);
            
	  str = emoticon_end;
	  state = GM_EMOTICON_STATE_NO_SMILEY;
	  break;
            
        case GM_EMOTICON_STATE_SHOCKED:     /* ;O  */
	  *emoticon_start = '\0';
            
	  gtk_text_buffer_insert (buf, bufiter, str, -1);
	  emoticon = gdk_pixbuf_new_from_inline (-1, inline_smiley_two, FALSE, NULL);
	  gtk_text_buffer_insert_pixbuf (buf, bufiter, emoticon);
            
	  str = emoticon_end;
	  state = GM_EMOTICON_STATE_NO_SMILEY;
	  break;
            
        case GM_EMOTICON_STATE_SMILING:     /* :)  */
	  *emoticon_start = '\0';
            
	  gtk_text_buffer_insert (buf, bufiter, str, -1);
	  emoticon = gdk_pixbuf_new_from_inline (-1, inline_smiley_three, FALSE, NULL);
	  gtk_text_buffer_insert_pixbuf (buf, bufiter, emoticon);
            
	  str = emoticon_end;
	  state = GM_EMOTICON_STATE_NO_SMILEY;
	  break;
            
        case GM_EMOTICON_STATE_EYE_BLINK:   /* ;)  */
	  *emoticon_start = '\0';
                        
	  gtk_text_buffer_insert (buf, bufiter, str, -1);
	  emoticon = gdk_pixbuf_new_from_inline (-1, inline_smiley_four, FALSE, NULL);
	  gtk_text_buffer_insert_pixbuf (buf, bufiter, emoticon);
            
	  str = emoticon_end;
	  state = GM_EMOTICON_STATE_NO_SMILEY;
	  break;
            
        case GM_EMOTICON_STATE_BIG_EYES:    /* 8)  */
	  *emoticon_start = '\0';
            
	  gtk_text_buffer_insert (buf, bufiter, str, -1);
	  emoticon = gdk_pixbuf_new_from_inline (-1, inline_smiley_five, FALSE, NULL);
	  gtk_text_buffer_insert_pixbuf (buf, bufiter, emoticon);
            
	  str = emoticon_end;
	  state = GM_EMOTICON_STATE_NO_SMILEY;
	  break;
            
        case GM_EMOTICON_STATE_LOOKING_SAD: /* :(  */
	  *emoticon_start = '\0';
            
	  gtk_text_buffer_insert (buf, bufiter, str, -1);
	  emoticon = gdk_pixbuf_new_from_inline (-1, inline_smiley_six, FALSE, NULL);
	  gtk_text_buffer_insert_pixbuf (buf, bufiter, emoticon);
            
	  str = emoticon_end;
	  state = GM_EMOTICON_STATE_NO_SMILEY;
	  break;
            
        default:
	  break;
        }
    
      if (*iter == '\0')
	end = TRUE;
    }

  gtk_text_buffer_insert (buf, bufiter, str, -1);
  free (orig_str);
}
