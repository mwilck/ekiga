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
    GM_EMOTICON_EYES_BLINKY,
    GM_EMOTICON_EYES_HUGE,
        
    GM_EMOTICON_EYES_NORMAL_WITH_NOSE,
    GM_EMOTICON_EYES_NORMAL_WITH_TEAR,
        
    GM_EMOTICON_STATE_FACE1,  /* :)  */
    GM_EMOTICON_STATE_FACE2,  /* 8)  */
    GM_EMOTICON_STATE_FACE3,  /* ;)  */
    GM_EMOTICON_STATE_FACE4,  /* :(  */
    GM_EMOTICON_STATE_FACE5,  /* :0  */
    GM_EMOTICON_STATE_FACE6,  /* :D  */
    GM_EMOTICON_STATE_FACE7,  /* :-) */
    GM_EMOTICON_STATE_FACE8,  /* :|  */
    GM_EMOTICON_STATE_FACE9,  /* :/  */
    GM_EMOTICON_STATE_FACE10, /* :P  */
    GM_EMOTICON_STATE_FACE11, /* :'( */


    GM_EMOTICON_STATE_FACE12,  /* ;D  */
    GM_EMOTICON_STATE_FACE13,  /* ;O  */
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
        case GM_EMOTICON_EYES_NORMAL_WITH_NOSE:
        case GM_EMOTICON_EYES_NORMAL_WITH_TEAR:      
        case GM_EMOTICON_EYES_BLINKY:
        case GM_EMOTICON_EYES_HUGE:
	  switch (*iter)
            {
            case '-':
              if (state == GM_EMOTICON_EYES_NORMAL)
                      state = GM_EMOTICON_EYES_NORMAL_WITH_NOSE;
	      break;

            case '\'':
              if (state == GM_EMOTICON_EYES_NORMAL)
                      state = GM_EMOTICON_EYES_NORMAL_WITH_TEAR;
	      break;
                
            case '(':
              emoticon_end = iter + 1;
              if (state == GM_EMOTICON_EYES_NORMAL_WITH_TEAR)
                      state = GM_EMOTICON_STATE_FACE11;
              else
                      state = GM_EMOTICON_STATE_FACE4;
	      break;
                
            case '0': case 'O': case 'o':
	      emoticon_end = iter + 1;
	      state = GM_EMOTICON_STATE_FACE5;
	      break;
                
            case 'D':
	      emoticon_end = iter + 1;
	      state = GM_EMOTICON_STATE_FACE6;
	      break;

            case '|':
	      emoticon_end = iter + 1;
	      state = GM_EMOTICON_STATE_FACE8;
	      break;
                
            case '\\': case '/':
	      emoticon_end = iter + 1;
	      state = GM_EMOTICON_STATE_FACE9;
	      break;
                
            case 'P': case 'p':
	      emoticon_end = iter + 1;
	      state = GM_EMOTICON_STATE_FACE10;
	      break;
                
            case ')':
	      switch (state)
                {
                case GM_EMOTICON_EYES_NORMAL:
		  emoticon_end = iter + 1;
		  state = GM_EMOTICON_STATE_FACE1;
		  break;
                    
                case GM_EMOTICON_EYES_BLINKY:
		  emoticon_end = iter + 1;
		  state = GM_EMOTICON_STATE_FACE3;
		  break;
                                        
                case GM_EMOTICON_EYES_HUGE:
		  emoticon_end = iter + 1;
		  state = GM_EMOTICON_STATE_FACE2;
		  break;

                case GM_EMOTICON_EYES_NORMAL_WITH_NOSE:
                  emoticon_end = iter + 1;  
                  state = GM_EMOTICON_STATE_FACE7;
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
	      state = GM_EMOTICON_EYES_BLINKY;
	      break;
                
            case ':':
	      emoticon_start = iter;
	      state = GM_EMOTICON_EYES_NORMAL;
	      break;
                
            case '8':
	      emoticon_start = iter;
	      state = GM_EMOTICON_EYES_HUGE;
	      break;       
                
            default:
	      break;        
            }
	  break;
            

        case GM_EMOTICON_STATE_FACE1:  /* :)  */
	  *emoticon_start = '\0';
            
	  gtk_text_buffer_insert (buf, bufiter, str, -1);
	  emoticon = gdk_pixbuf_new_from_inline (-1, gm_emoticon_face1, FALSE, NULL);
	  gtk_text_buffer_insert_pixbuf (buf, bufiter, emoticon);
            
	  str = emoticon_end;
	  state = GM_EMOTICON_STATE_NO_SMILEY;
	  break;

        case GM_EMOTICON_STATE_FACE2:  /* 8)  */
	  *emoticon_start = '\0';
            
	  gtk_text_buffer_insert (buf, bufiter, str, -1);
	  emoticon = gdk_pixbuf_new_from_inline (-1, gm_emoticon_face2, FALSE, NULL);
	  gtk_text_buffer_insert_pixbuf (buf, bufiter, emoticon);
            
	  str = emoticon_end;
	  state = GM_EMOTICON_STATE_NO_SMILEY;
	  break;

        case GM_EMOTICON_STATE_FACE3:  /* ;)  */
	  *emoticon_start = '\0';
            
	  gtk_text_buffer_insert (buf, bufiter, str, -1);
	  emoticon = gdk_pixbuf_new_from_inline (-1, gm_emoticon_face3, FALSE, NULL);
	  gtk_text_buffer_insert_pixbuf (buf, bufiter, emoticon);
            
	  str = emoticon_end;
	  state = GM_EMOTICON_STATE_NO_SMILEY;
	  break;

        case GM_EMOTICON_STATE_FACE4:  /* :(  */
	  *emoticon_start = '\0';
            
	  gtk_text_buffer_insert (buf, bufiter, str, -1);
	  emoticon = gdk_pixbuf_new_from_inline (-1, gm_emoticon_face4, FALSE, NULL);
	  gtk_text_buffer_insert_pixbuf (buf, bufiter, emoticon);
            
	  str = emoticon_end;
	  state = GM_EMOTICON_STATE_NO_SMILEY;
	  break;

        case GM_EMOTICON_STATE_FACE5:  /* :0  */
	  *emoticon_start = '\0';
            
	  gtk_text_buffer_insert (buf, bufiter, str, -1);
	  emoticon = gdk_pixbuf_new_from_inline (-1, gm_emoticon_face5, FALSE, NULL);
	  gtk_text_buffer_insert_pixbuf (buf, bufiter, emoticon);
            
	  str = emoticon_end;
	  state = GM_EMOTICON_STATE_NO_SMILEY;
	  break;

        case GM_EMOTICON_STATE_FACE6:  /* :D  */
	  *emoticon_start = '\0';
            
	  gtk_text_buffer_insert (buf, bufiter, str, -1);
	  emoticon = gdk_pixbuf_new_from_inline (-1, gm_emoticon_face6, FALSE, NULL);
	  gtk_text_buffer_insert_pixbuf (buf, bufiter, emoticon);
            
	  str = emoticon_end;
	  state = GM_EMOTICON_STATE_NO_SMILEY;
	  break;

        case GM_EMOTICON_STATE_FACE7:  /* :-) */
	  *emoticon_start = '\0';
            
	  gtk_text_buffer_insert (buf, bufiter, str, -1);
	  emoticon = gdk_pixbuf_new_from_inline (-1, gm_emoticon_face7, FALSE, NULL);
	  gtk_text_buffer_insert_pixbuf (buf, bufiter, emoticon);
            
	  str = emoticon_end;
	  state = GM_EMOTICON_STATE_NO_SMILEY;
	  break;

        case GM_EMOTICON_STATE_FACE8:  /* :|  */
	  *emoticon_start = '\0';
            
	  gtk_text_buffer_insert (buf, bufiter, str, -1);
	  emoticon = gdk_pixbuf_new_from_inline (-1, gm_emoticon_face8, FALSE, NULL);
	  gtk_text_buffer_insert_pixbuf (buf, bufiter, emoticon);
            
	  str = emoticon_end;
	  state = GM_EMOTICON_STATE_NO_SMILEY;
	  break;

        case GM_EMOTICON_STATE_FACE9:  /* :/  */
	  *emoticon_start = '\0';
            
	  gtk_text_buffer_insert (buf, bufiter, str, -1);
	  emoticon = gdk_pixbuf_new_from_inline (-1, gm_emoticon_face9, FALSE, NULL);
	  gtk_text_buffer_insert_pixbuf (buf, bufiter, emoticon);
            
	  str = emoticon_end;
	  state = GM_EMOTICON_STATE_NO_SMILEY;
	  break;

        case GM_EMOTICON_STATE_FACE10: /* :P  */
	  *emoticon_start = '\0';
            
	  gtk_text_buffer_insert (buf, bufiter, str, -1);
	  emoticon = gdk_pixbuf_new_from_inline (-1, gm_emoticon_face10, FALSE, NULL);
	  gtk_text_buffer_insert_pixbuf (buf, bufiter, emoticon);
            
	  str = emoticon_end;
	  state = GM_EMOTICON_STATE_NO_SMILEY;
	  break;

        case GM_EMOTICON_STATE_FACE11: /* :'( */
	  *emoticon_start = '\0';
            
	  gtk_text_buffer_insert (buf, bufiter, str, -1);
	  emoticon = gdk_pixbuf_new_from_inline (-1, gm_emoticon_face11, FALSE, NULL);
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
