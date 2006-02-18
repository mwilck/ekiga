
/* GnomeMeeting -- A Video-Conferencing application
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
 *                         chat_window.cpp  -  description
 *                         -------------------------------
 *   begin                : Wed Jan 23 2002
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : This file contains functions to build the chat
 *                          window. It uses DTMF tones.
 *   Additional code      : Kenneth Christiansen  <kenneth@gnu.org>
 *
 */


#include "../../config.h"

#include "chat.h"
#include "ekiga.h"
#include "callbacks.h"
#include "misc.h"
#include "main.h"
#include "addressbook.h"
#include "callshistory.h"
#include "urlhandler.h"
#include "toolbox/toolbox.h"

#include "gmconf.h"
#include "gmtexttagaddon.h"
#include "gmtextbufferaddon.h"
#include "gmtextviewaddon.h"
#include "gmmenuaddon.h"
#include "gmconnectbutton.h"
#include "gmstatusbar.h"
#include "gmstockicons.h"

#include <gdk/gdkkeysyms.h>


#ifdef WIN32
#include "winpaths.h"
#endif

/* Internal structures used by the text chat */
struct GmTextChatWindow_
{
  GtkWidget	*notebook;	 /* The notebook containing 
				    the conversations */
  GtkWidget 	*statusbar;	 /* A simple status bar */
  GtkTooltips   *tips;           /* Tooltips */
};

struct GmTextChatWindowPage_
{
  GtkWidget	*connect_button; /* The connect button */
  GtkWidget	*send_button;    /* The send button */
  GtkWidget	*remote_url;     /* Where to send the message */ 
  GtkWidget     *conversation;   /* The conversation history */
  GtkWidget	*message;        /* The message to send */
  GtkWidget	*tab_label;      /* The notebook tab label */
  GtkWidget     *bold_button;    /* The buttons */
  GtkWidget     *italic_button;
  GtkWidget     *underline_button;
  int           last_user;       /* 0 = local, 1 = remote, -1 = nobody */
};

typedef struct GmTextChatWindow_ GmTextChatWindow;
typedef struct GmTextChatWindowPage_ GmTextChatWindowPage;

#define GM_TEXT_CHAT_WINDOW(x) (GmTextChatWindow *) (x)
#define GM_TEXT_CHAT_WINDOW_PAGE(x) (GmTextChatWindowPage *) (x)

/* the list of magic values for the various styles */
enum {
  STYLE_FIRST,
  STYLE_BOLD,
  STYLE_ITALIC,
  STYLE_UNDERLINE,
  STYLE_LAST,
};


/* Declarations */

/* GUI functions */

/* DESCRIPTION  : / 
 * BEHAVIOR     : Frees a GmTextChatWindowPage and its content.
 * PRE          : A non-NULL pointer to a GmTextChatWindowPage.
 */
static void gm_twp_destroy (gpointer);


/* DESCRIPTION  : / 
 * BEHAVIOR     : Frees a GmTextChatWindow and its content.
 * PRE          : A non-NULL pointer to a GmTextChatWindow.
 */
static void gm_tw_destroy (gpointer);


/* DESCRIPTION  : / 
 * BEHAVIOR     : Returns a pointer to the private GmTextChatWindow
 * 		  used by the text chat window GMObject.
 * PRE          : The given GtkWidget pointer must be a text chat GMObject.
 */
static GmTextChatWindow *gm_tw_get_tw (GtkWidget *);


/* DESCRIPTION  : / 
 * BEHAVIOR     : Returns a pointer to the private GmTextChatWindowPage
 * 		  used by any page of the internal GtkNotebook of the 
 * 		  text chat book GMObject.
 * PRE          : The given GtkWidget pointer must point to a page
 * 		  of the internal GtkNotebook of the text chat GMObject.
 */
static GmTextChatWindowPage *gm_tw_get_twp (GtkWidget *);


/* DESCRIPTION  : / 
 * BEHAVIOR     : Returns a pointer to the private GmTextChatWindowPage
 * 		  used by the current page of the internal GtkNotebook of the 
 * 		  text chat GMObject.
 * PRE          : The given GtkWidget pointer must point to the text chat
 * 		  GMObject.
 */
static GmTextChatWindowPage *gm_tw_get_current_twp (GtkWidget *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Delete the current tab if it is not the last one. Hide
 * 		   the chat window otherwise.
 * PRE          :  The pointer must be a valid pointer to the chat window 
 * 		   GMObject. The integer indicates a valid page.
 */
static void gm_tw_close_tab (GtkWidget *,
			     int);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Get the current tab number.
 * PRE          :  The pointer must be a valid pointer to the chat window 
 * 		   GMObject. 
 */
static int gm_tw_get_current_tab (GtkWidget *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Get the first free page of the text chat window.
 * PRE          :  The text chat window.
 */
static GtkWidget *gm_tw_get_first_free_tab (GtkWidget *, 
					    int &);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Build a tab for the text chat window.
 * PRE          :  The text chat window.
 */
static GtkWidget *gm_tw_build_tab (GtkWidget *,
				   int &);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Clears the current tab.
 * PRE          :  The pointer must be a valid pointer to the chat window 
 * 		   GMObject.
 */
static void gm_tw_clear_current_tab (GtkWidget *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Get the message in the current tab, and delete the content.
 * PRE          :  The pointer must be a valid pointer to the chat window 
 * 		   GMObject.
 */
static gchar *gm_tw_get_message_body (GtkWidget *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Get the URL in the current tab.
 * PRE          :  The pointer must be a valid pointer to the chat window 
 * 		   GMObject.
 */
static const char *gm_tw_get_url (GtkWidget *);


/* Callbacks */

/* DESCRIPTION  :  Called when somebody types something in the chat entry.
 * BEHAVIOR     :  Send the given message to the remote user if return is
 * 		   pressed, closes the tab if ESC is pressed, clears the
 * 		   tab if Ctrl-L is pressed.
 * PRE          :  The pointer must be a valid pointer to the chat window
 * 		   GMObject.
 */
static gboolean chat_entry_key_pressed_cb (GtkWidget *,
					   GdkEventKey *,
					   gpointer);


/* DESCRIPTION  :  This callback is called when the user selects a match in the
 * 		   possible URLs list.
 * BEHAVIOR     :  It udpates the entry.
 * PRE          :  /
 */
static gboolean entry_completion_url_selected_cb (GtkEntryCompletion *,
						  GtkTreeModel *,
						  GtkTreeIter *,
						  gpointer);


/* DESCRIPTION  :  Called when the close button of a tab is clicked.
 * BEHAVIOR     :  Calls gm_tw_close_tab. 
 * PRE          :  The pointer must be a valid pointer to the chat window 
 * 		   GMObject.
 */
static void close_button_clicked_cb (GtkWidget *, 
				     gpointer);


/* DESCRIPTION  :  Called when a style button of a tab is clicked.
 * BEHAVIOR     :  Update the other buttons and insert the necessary markup
 *                 in the message tab.
 * PRE          :  The pointer must be a valid GINT_TO_POINTER, corresponding
 *                 to a valid text style (STYLE_*)
 */
static void style_button_toggled_cb (GtkToggleButton *widget, 
				     gpointer data);


/* DESCRIPTION  :  Called when the send button of a tab is clicked.
 * BEHAVIOR     :  Send the message to the remote user if there is an url
 *                 where to send it.
 * PRE          :  The pointer must be a valid pointer to the chat window 
 * 		   GMObject.
 */
static void send_button_clicked_cb (GtkWidget *widget, 
				    gpointer data);


/* DESCRIPTION  :  Called when the new button of a tab is clicked.
 * BEHAVIOR     :  Creates a new tab. If we are in a call, create a new
 *                 tab with the current call information if it doesn't
 *                 exist yet.
 * PRE          :  The pointer must be a valid pointer to the chat window 
 * 		   GMObject.
 */
static void new_button_clicked_cb (GtkWidget *widget, 
				   gpointer data);


/* DESCRIPTION  :  Called when the URL entry is modified in a page.
 * BEHAVIOR     :  Update the tab label. If we are in a call and if the
 * 		   URL corresponds to the call URL, then update the current
 * 		   tab state to the "connected" state.
 * PRE          :  The pointer must be a valid pointer to the chat window.
 */
static void url_entry_changed_cb (GtkWidget *, 
				  gpointer);


/* DESCRIPTION  :  This callback is called when idle.
 * BEHAVIOR     :  Do the job of gm_text_chat_window_urls_history_update
 *                 async.
 * PRE          :  /
 */
static gboolean gm_tw_urls_history_update_cb (gpointer);

	
/* DESCRIPTION  :  Called when an URL is clicked.
 * BEHAVIOR     :  Set the text in the clipboard.
 * PRE          :  /
 */
static void copy_uri_cb (const gchar *);


/* DESCRIPTION  :  Called when an URL is clicked.
 * BEHAVIOR     :  Connect to the given URL or transfer the call to that URL.
 * PRE          :  /
 */
static void connect_uri_cb (const gchar *);


/* DESCRIPTION  :  Called when an URL has to be added to the addressbook.
 * BEHAVIOR     :  Displays the popup.
 * PRE          :  /
 */
static void add_uri_cb (const gchar *);


/* Implementation */
static void
gm_twp_destroy (gpointer t)
{
  GmTextChatWindowPage *twp = NULL;

  g_return_if_fail (t != NULL);
  
  twp = GM_TEXT_CHAT_WINDOW_PAGE (t); 
  
  delete (twp);
}


static void
gm_tw_destroy (gpointer tw)
{
  g_return_if_fail (tw != NULL);
  
  delete (GM_TEXT_CHAT_WINDOW (tw));
}


static GmTextChatWindow *
gm_tw_get_tw (GtkWidget *text_chat_window)
{
  g_return_val_if_fail (text_chat_window != NULL, NULL);

  return GM_TEXT_CHAT_WINDOW (g_object_get_data (G_OBJECT (text_chat_window), "GMObject"));
}


static GmTextChatWindowPage *
gm_tw_get_twp (GtkWidget *p)
{
  g_return_val_if_fail (p != NULL, NULL);

  return GM_TEXT_CHAT_WINDOW_PAGE (g_object_get_data (G_OBJECT (p), "GMObject"));
}


static GmTextChatWindowPage *
gm_tw_get_current_twp (GtkWidget *t)
{
  GmTextChatWindow *tw = NULL;
  
  int page_num = 0;
  GtkWidget *p = NULL;
  
  g_return_val_if_fail (t != NULL, NULL);

  /* Get the required data from the GtkNotebook page */
  tw = gm_tw_get_tw (t);

  g_return_val_if_fail (tw != NULL, NULL);

  page_num = gtk_notebook_get_current_page (GTK_NOTEBOOK (tw->notebook));
  p = gtk_notebook_get_nth_page (GTK_NOTEBOOK (tw->notebook), page_num);

  return GM_TEXT_CHAT_WINDOW_PAGE (g_object_get_data (G_OBJECT (p), 
						      "GMObject"));
}


static void 
gm_tw_close_tab (GtkWidget *chat_window,
		 int tab_number)
{
  GmTextChatWindow *tw = NULL;
  GmTextChatWindowPage *twp = NULL;
  
  GtkWidget *page = NULL;
  int nbr = 0;
  
  g_return_if_fail (chat_window != NULL);

  tw = gm_tw_get_tw (GTK_WIDGET (chat_window));

  /* Remove the tab */
  page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (tw->notebook), tab_number);
  nbr = gtk_notebook_get_n_pages (GTK_NOTEBOOK (tw->notebook));

  if (nbr <= 1) {

    gnomemeeting_window_hide (GTK_WIDGET (chat_window));
    gm_text_chat_window_add_tab (GTK_WIDGET (chat_window), NULL, NULL);
  }

  gtk_notebook_remove_page (GTK_NOTEBOOK (tw->notebook), tab_number);

  /* Update the focus */
  twp = gm_tw_get_current_twp (GTK_WIDGET (chat_window));
  gtk_widget_grab_focus (twp->message);
}


static int
gm_tw_get_current_tab (GtkWidget *chat_window)
{
  GmTextChatWindow *tw = NULL;

  g_return_val_if_fail (chat_window != NULL, 0);

  tw = gm_tw_get_tw (GTK_WIDGET (chat_window));

  return gtk_notebook_get_current_page (GTK_NOTEBOOK (tw->notebook));
}


static GtkWidget *
gm_tw_get_first_free_tab (GtkWidget *chat_window,
			  int & pos)
{
  GmTextChatWindow *tw = NULL;
  GmTextChatWindowPage *twp = NULL;

  GtkWidget *page = NULL;

  const char *contact_url = NULL;

  g_return_val_if_fail (chat_window != NULL, NULL);

  /* Get the internal data */
  tw = gm_tw_get_tw (chat_window);

  for (pos = 0 ; pos < gtk_notebook_get_n_pages (GTK_NOTEBOOK (tw->notebook)) ; pos++) {

    page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (tw->notebook), pos);

    if (page) {

      /* Get the internal data */
      twp = gm_tw_get_twp (page);
      
      contact_url = gtk_entry_get_text (GTK_ENTRY (twp->remote_url));

      if (GMURL (contact_url).IsEmpty ())
	return page;
    }
  }

  return NULL;
}


static GtkWidget * 
gm_tw_build_tab (GtkWidget *chat_window,
		 int & pos)
{
  GmTextChatWindow *tw = NULL;
  GmTextChatWindowPage *twp = NULL;

  GtkWidget *scr = NULL;
  GtkWidget *hbox = NULL;
  GtkWidget *hbox2 = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *label = NULL;
  GtkWidget *image = NULL;
  GtkWidget *close_image = NULL;
  GtkWidget *close_button = NULL;
  GtkWidget *new_button_image = NULL;
  GtkWidget *new_button = NULL;
  GtkWidget *separator = NULL;
  GtkWidget *frame = NULL;
  GtkWidget *page = NULL;
  GtkWidget *vpane = NULL;

  GtkEntryCompletion *completion = NULL;
  GtkListStore *list_store = NULL;
  
  GtkTextIter iter;
  GtkTextBuffer *buffer = NULL;
  GtkTextMark *mark = NULL;
  GtkTextTag *regex_tag = NULL;

  int x = 0, y = 0;

  /* Get the data */
  g_return_val_if_fail (chat_window != NULL, NULL);

  tw = gm_tw_get_tw (chat_window);
  
  g_return_val_if_fail (tw != NULL, NULL);
 
  /* Set the data */
  twp = new GmTextChatWindowPage ();
  twp->remote_url = NULL;

  /* Build the tab */
  hbox = gtk_hbox_new (FALSE, 0);	   
  twp->tab_label = gtk_label_new (NULL);
  gtk_box_pack_start (GTK_BOX (hbox), twp->tab_label, TRUE, TRUE, 0);

  close_button = gtk_button_new ();
  gtk_button_set_relief (GTK_BUTTON (close_button), GTK_RELIEF_NONE);
  close_image = gtk_image_new_from_stock (GTK_STOCK_CLOSE,
					  GTK_ICON_SIZE_MENU); 
  gtk_container_add (GTK_CONTAINER (close_button), close_image);
  gtk_widget_set_size_request (close_button, 17, 17);
  gtk_box_pack_start (GTK_BOX (hbox), close_button, FALSE, FALSE, 0);
  gtk_widget_show_all (hbox);

  /* Build the page content */
  vpane = gtk_vpaned_new ();
  gtk_container_set_border_width (GTK_CONTAINER (vpane), 4);
  pos = gtk_notebook_append_page (GTK_NOTEBOOK (tw->notebook), vpane, hbox);
  page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (tw->notebook), pos);
  g_object_set_data_full (G_OBJECT (page), "GMObject", 
			  twp, (GDestroyNotify) gm_twp_destroy);

  /* The URL entry and the conversation history */
  vbox = gtk_vbox_new (FALSE, 4);

  /* New button and URL entry */
  hbox = gtk_hbox_new (FALSE, 4);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
  
  new_button = gtk_button_new ();
  new_button_image = gtk_image_new_from_stock (GTK_STOCK_NEW,
					       GTK_ICON_SIZE_LARGE_TOOLBAR);
  gtk_container_add (GTK_CONTAINER (new_button), new_button_image);
  gtk_box_pack_start (GTK_BOX (hbox), new_button, FALSE, FALSE, 0);

  separator = gtk_vseparator_new ();
  gtk_box_pack_start (GTK_BOX (hbox), separator, FALSE, FALSE, 0);

  twp->remote_url = gtk_entry_new ();
  completion = gtk_entry_completion_new ();
  list_store = gtk_list_store_new (3, 
				   G_TYPE_STRING,
				   G_TYPE_STRING,
				   G_TYPE_STRING);
  gtk_entry_completion_set_model (GTK_ENTRY_COMPLETION (completion),
				  GTK_TREE_MODEL (list_store));
  gtk_entry_completion_set_text_column (GTK_ENTRY_COMPLETION (completion), 2);
  gtk_entry_set_completion (GTK_ENTRY (twp->remote_url), completion);
  gtk_entry_set_text (GTK_ENTRY (twp->remote_url), GMURL ().GetDefaultURL ());
  gtk_entry_completion_set_match_func (GTK_ENTRY_COMPLETION (completion),
				       entry_completion_url_match_cb,
				       (gpointer) list_store,
				       NULL);
  gtk_box_pack_start (GTK_BOX (hbox), twp->remote_url, TRUE, TRUE, 0);
  
  gm_text_chat_window_urls_history_update (chat_window);

  /* Text buffer */
  twp->conversation = gtk_text_view_new_with_regex ();
  gtk_text_view_set_editable (GTK_TEXT_VIEW (twp->conversation), FALSE);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (twp->conversation),
			       GTK_WRAP_WORD);

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (twp->conversation));
  gtk_text_buffer_get_end_iter (buffer, &iter);
  gtk_text_view_set_cursor_visible  (GTK_TEXT_VIEW (twp->conversation), FALSE);
  mark = gtk_text_buffer_create_mark (buffer, "current-position", &iter, FALSE);
  gtk_text_buffer_create_tag (buffer, "remote-user",
			      "foreground", "red", 
			      "weight", 900, NULL);
  gtk_text_buffer_create_tag (buffer, "local-user",
			      "foreground", "darkblue", 
			      "weight", 900, NULL);
  gtk_text_buffer_create_tag (buffer, "timestamp",
			      "foreground", "darkgray", 
			      "left-margin", 15,
			      "stretch", PANGO_STRETCH_CONDENSED, NULL);
  gtk_text_buffer_create_tag (buffer, "error",
			      "foreground", "red", NULL); 

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  scr = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (scr), twp->conversation);
  gtk_container_add (GTK_CONTAINER (frame), scr);
  gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);
  
  gtk_paned_add1 (GTK_PANED (vpane), vbox);

  /* The message text view */
  hbox = gtk_hbox_new (FALSE, 4);

  twp->message = gtk_text_view_new ();
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (twp->message), 
			       GTK_WRAP_WORD_CHAR);
  twp->last_user = -1;
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  scr = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (scr), twp->message);
  gtk_container_add (GTK_CONTAINER (frame), scr);
  gtk_box_pack_start (GTK_BOX (hbox), frame, TRUE, TRUE, 0);
  
  vbox = gtk_vbox_new (FALSE, 4);
					       
  twp->connect_button = gm_connect_button_new (GM_STOCK_STATUS_IN_A_CALL,
					       GM_STOCK_STATUS_AVAILABLE,
					       GTK_ICON_SIZE_MENU,
					       _("Hang _up"),
					       _("_Call user"));
  gtk_tooltips_set_tip (tw->tips, twp->connect_button, _("Call this user"), 0); 
  gtk_box_pack_start (GTK_BOX (vbox), twp->connect_button, TRUE, FALSE, 0);
  
  twp->send_button = gtk_button_new ();
  image = gtk_image_new_from_stock (GTK_STOCK_JUMP_TO, 
				    GTK_ICON_SIZE_MENU);
  hbox2 = gtk_hbox_new (FALSE, 0);
  label = gtk_label_new (NULL);
  gtk_label_set_markup_with_mnemonic (GTK_LABEL (label), _("_Send"));
  gtk_box_pack_start (GTK_BOX (hbox2), image, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox2), label, FALSE, FALSE, 6);
  gtk_container_add (GTK_CONTAINER (twp->send_button), hbox2);
  
  gtk_tooltips_set_tip (tw->tips, twp->send_button, _("Send message"), NULL); 
  gtk_box_pack_start (GTK_BOX (vbox), twp->send_button, TRUE, FALSE, 0);

  hbox2 = gtk_hbox_new (FALSE, 0);
  twp->bold_button = gtk_toggle_button_new (); 
  image = gtk_image_new_from_stock (GTK_STOCK_BOLD,
				    GTK_ICON_SIZE_MENU);
  gtk_container_add (GTK_CONTAINER (twp->bold_button), image);
  gtk_box_pack_start (GTK_BOX (hbox2), twp->bold_button, FALSE, FALSE, 0);

  twp->italic_button = gtk_toggle_button_new (); 
  image = gtk_image_new_from_stock (GTK_STOCK_ITALIC,
				    GTK_ICON_SIZE_MENU);
  gtk_container_add (GTK_CONTAINER (twp->italic_button), image);
  gtk_box_pack_start (GTK_BOX (hbox2), twp->italic_button, FALSE, FALSE, 0);
  
  twp->underline_button = gtk_toggle_button_new (); 
  image = gtk_image_new_from_stock (GTK_STOCK_UNDERLINE,
				    GTK_ICON_SIZE_MENU);
  gtk_container_add (GTK_CONTAINER (twp->underline_button), image);
  gtk_box_pack_start (GTK_BOX (hbox2), twp->underline_button, FALSE, FALSE, 0);

  gtk_box_pack_start (GTK_BOX (vbox), hbox2, TRUE, FALSE, 0);
  
  gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);

  gtk_widget_realize (GTK_WIDGET (chat_window));
  gtk_widget_show (twp->message);
  gtk_widget_grab_focus (twp->message);
  gnomemeeting_window_get_size (GTK_WIDGET (chat_window), x, y);
  gtk_paned_set_position (GTK_PANED (vpane), y - 150);
  gtk_paned_add2 (GTK_PANED (vpane), hbox);
  
  /* Create the various tags for the different urls types */
  /* HTTP/FTP/HTTPS/SFTP */
  regex_tag = gtk_text_buffer_create_tag (buffer, "uri-http", "foreground", "blue", "underline", PANGO_UNDERLINE_SINGLE,  NULL);
  if (gtk_text_tag_set_regex (regex_tag, "\\<(http[s]?|[s]?ftp)://[^[:blank:]]+\\>")) 
    gtk_text_tag_add_actions_to_regex (regex_tag, _("_Open URL"), gm_open_uri, _("_Copy URL to Clipboard"), copy_uri_cb, NULL);
  
  /* H323/SIP/CALLTO */
  regex_tag = gtk_text_buffer_create_tag (buffer, "uri-gm", "foreground", "blue", "underline", PANGO_UNDERLINE_SINGLE, NULL);
  if (gtk_text_tag_set_regex (regex_tag, "\\<((h323|sip|callto):[^[:blank:]]+)\\>"))
    gtk_text_tag_add_actions_to_regex (regex_tag, _("C_all Contact"), connect_uri_cb, _("Add Contact to _Address Book"), add_uri_cb, _("_Copy URL to Clipboard"), copy_uri_cb, NULL);
  
  /* Smileys */
  regex_tag = gtk_text_buffer_create_tag (buffer, "smileys", "foreground", "grey", NULL);
  if (gtk_text_tag_set_regex (regex_tag, "(:[-]?(\\)|\\(|o|O|p|P|D|\\||/)|\\}:(\\(|\\))|\\|[-]?(\\(|\\))|:'\\(|:\\[|:-(\\.|\\*|x)|;[-]?\\)|(8|B)[-]?\\)|X(\\(|\\||\\))|\\((\\.|\\|)\\)|x\\*O)"))
    gtk_text_tag_set_regex_display (regex_tag, gtk_text_buffer_insert_smiley);
  
  /* Bold, Italic and Underline */
  regex_tag = gtk_text_buffer_create_tag (buffer, "bold", "weight", PANGO_WEIGHT_BOLD, NULL);
  if (gtk_text_tag_set_regex (regex_tag, "(<b>.*</b>|<B>.*</B>)"))
    gtk_text_tag_set_regex_display (regex_tag, gtk_text_buffer_insert_markup);
  
  regex_tag = gtk_text_buffer_create_tag (buffer, "italic", "style", PANGO_STYLE_ITALIC, NULL);
  if (gtk_text_tag_set_regex (regex_tag, "(<i>.*</i>|<I>.*</I>)"))
    gtk_text_tag_set_regex_display (regex_tag, gtk_text_buffer_insert_markup);
  
  regex_tag = gtk_text_buffer_create_tag (buffer, "underline", "underline", PANGO_UNDERLINE_SINGLE, NULL);
  if (gtk_text_tag_set_regex (regex_tag, "(<u>.*</u>|<U>.*</U>)"))
    gtk_text_tag_set_regex_display (regex_tag, gtk_text_buffer_insert_markup);

  /* Latex */
  regex_tag = gtk_text_buffer_create_tag (buffer, "latex", "foreground", "grey",NULL);
  if (gtk_text_tag_set_regex (regex_tag, "(\\$[^$]*\\$|\\$\\$[^$]*\\$\\$)"))
    gtk_text_tag_add_actions_to_regex (regex_tag, _("_Copy Equation"), copy_uri_cb, NULL);

  
  /* Signals */
  g_signal_connect (G_OBJECT (twp->bold_button), "toggled",
                    G_CALLBACK (style_button_toggled_cb), 
		    GINT_TO_POINTER (STYLE_BOLD));
  g_signal_connect (G_OBJECT (twp->italic_button), "toggled",
                    G_CALLBACK (style_button_toggled_cb), 
		    GINT_TO_POINTER (STYLE_ITALIC));
  g_signal_connect (G_OBJECT (twp->underline_button), "toggled",
                    G_CALLBACK (style_button_toggled_cb), 
		    GINT_TO_POINTER (STYLE_UNDERLINE));
  g_signal_connect (G_OBJECT (twp->connect_button), "released",
                    G_CALLBACK (connect_button_clicked_cb), 
		    GTK_ENTRY (twp->remote_url));
  g_signal_connect (GTK_OBJECT (twp->send_button), "clicked",
		    G_CALLBACK (send_button_clicked_cb), 
		    chat_window);
  g_signal_connect (GTK_OBJECT (close_button), "clicked",
		    G_CALLBACK (close_button_clicked_cb), 
		    page);
  g_signal_connect (GTK_OBJECT (new_button), "clicked",
		    G_CALLBACK (new_button_clicked_cb), 
		    chat_window);
  g_signal_connect (GTK_OBJECT (twp->message), "key-press-event",
		    G_CALLBACK (chat_entry_key_pressed_cb), chat_window);
  g_signal_connect (G_OBJECT (completion), "match-selected", 
		    GTK_SIGNAL_FUNC (entry_completion_url_selected_cb), 
		    chat_window);
  g_signal_connect (G_OBJECT (twp->remote_url), "changed", 
		    GTK_SIGNAL_FUNC (url_entry_changed_cb), chat_window);

  return page;
}


static void 
gm_tw_clear_current_tab (GtkWidget *chat_window)
{
  GmTextChatWindowPage *twp = NULL;

  GtkTextIter start_iter, end_iter;
  GtkTextBuffer *buffer = NULL;

  g_return_if_fail (chat_window != NULL);

  twp = gm_tw_get_current_twp (GTK_WIDGET (chat_window));

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (twp->conversation));
  gtk_text_buffer_get_start_iter (buffer, &start_iter);
  gtk_text_buffer_get_end_iter (buffer, &end_iter);

  gtk_text_buffer_delete (buffer, &start_iter, &end_iter);
}


static gchar *
gm_tw_get_message_body (GtkWidget *chat_window)
{
  GmTextChatWindowPage *twp = NULL;

  GtkTextIter start_iter, end_iter;
  GtkTextBuffer *buffer = NULL;

  gchar *body = NULL;

  g_return_val_if_fail (chat_window != NULL, NULL);

  twp = gm_tw_get_current_twp (GTK_WIDGET (chat_window));

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (twp->message));
  gtk_text_buffer_get_start_iter (buffer, &start_iter);
  gtk_text_buffer_get_end_iter (buffer, &end_iter);

  body = gtk_text_buffer_get_text (GTK_TEXT_BUFFER (buffer),
				   &start_iter, &end_iter, FALSE);
  gtk_text_buffer_delete (GTK_TEXT_BUFFER (buffer), &start_iter, &end_iter);

  return body;  
}


static const char *
gm_tw_get_url (GtkWidget *chat_window)
{
  GmTextChatWindowPage *twp = NULL;

  g_return_val_if_fail (chat_window != NULL, NULL);

  twp = gm_tw_get_current_twp (GTK_WIDGET (chat_window));

  return gtk_entry_get_text (GTK_ENTRY (twp->remote_url));
}


static gboolean
chat_entry_key_pressed_cb (GtkWidget *w,
			   GdkEventKey *key,
			   gpointer data)
{
  GmTextChatWindowPage *twp = NULL;

  g_return_val_if_fail (data != NULL, FALSE);

  if (key->keyval == GDK_Return) {

    /* Simulate a click on the send button */
    twp = gm_tw_get_current_twp (GTK_WIDGET (data));
    gtk_button_clicked (GTK_BUTTON (twp->send_button));

    return TRUE;
  }
  else if (key->keyval == GDK_Escape) {

    gm_tw_close_tab (GTK_WIDGET (data), 
		     gm_tw_get_current_tab (GTK_WIDGET (data)));

    return TRUE;
  }
  else if ((key->state & GDK_CONTROL_MASK) && key->keyval == GDK_l) {

    gm_tw_clear_current_tab (GTK_WIDGET (data));

    return TRUE;
  }

  return FALSE;
}


static gboolean
entry_completion_url_selected_cb (GtkEntryCompletion *completion,
				  GtkTreeModel *model,
				  GtkTreeIter *iter,
				  gpointer data)
{
  GmTextChatWindowPage *twp = NULL;
  
  GtkWidget *entry = NULL;
  
  gchar *url = NULL;
  gchar *name = NULL;
  
  g_return_val_if_fail (data != NULL, TRUE);

  twp = gm_tw_get_current_twp (GTK_WIDGET (data));
  
  gtk_tree_model_get (GTK_TREE_MODEL (model), iter, 0, &name, -1);
  gtk_tree_model_get (GTK_TREE_MODEL (model), iter, 1, &url, -1);

  entry = gtk_entry_completion_get_entry (completion);
  gtk_entry_set_text (GTK_ENTRY (entry), url);
  gtk_label_set_text (GTK_LABEL (twp->tab_label), name);

  g_free (url);
  g_free (name);

  return TRUE;
}


static void 
close_button_clicked_cb (GtkWidget *, 
			 gpointer data)
{
  GmTextChatWindow *tw = NULL;
  GtkWidget *chat_window = NULL;

  chat_window = GnomeMeeting::Process ()->GetChatWindow ();
  tw = gm_tw_get_tw (chat_window);
  g_return_if_fail (tw != NULL);
  g_return_if_fail (data != NULL);

  for (int i = 0 ; i < gtk_notebook_get_n_pages (GTK_NOTEBOOK (tw->notebook)) ; i++) {

    if (gtk_notebook_get_nth_page (GTK_NOTEBOOK (tw->notebook), i) == GTK_WIDGET (data)) {
      
      gm_tw_close_tab (chat_window, i);
      return;
    }
  }
}


static void
style_button_toggled_cb (GtkToggleButton *button,
			 gpointer data)
{
  GtkWidget *chat_window = NULL;
  
  GtkTextBuffer *buffer = NULL;
  GtkTextIter iter;
  gint style = 0;
  
  GmTextChatWindowPage *twp = NULL;
  
  chat_window = GnomeMeeting::Process ()->GetChatWindow ();
  twp = gm_tw_get_current_twp (chat_window);
  style = GPOINTER_TO_INT (data);

  g_return_if_fail (STYLE_FIRST < style && style < STYLE_LAST);

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (twp->message));
  gtk_text_buffer_get_end_iter (buffer, &iter);
  
  switch (style) {
  case STYLE_BOLD:
    if (gtk_toggle_button_get_active (button)) {
      
      if (GTK_TOGGLE_BUTTON (twp->italic_button)->active)
	gtk_text_buffer_insert (buffer, &iter, "</i>", -1);
      if (GTK_TOGGLE_BUTTON (twp->underline_button)->active)
	gtk_text_buffer_insert (buffer, &iter, "</u>", -1);
	
      GTK_TOGGLE_BUTTON (twp->italic_button)->active = FALSE;
      gtk_widget_set_state (twp->italic_button, GTK_STATE_NORMAL);
      GTK_TOGGLE_BUTTON (twp->underline_button)->active = FALSE;
      gtk_widget_set_state (twp->underline_button, GTK_STATE_NORMAL);
      gtk_text_buffer_insert (buffer, &iter, "<b>", -1);
    }
    else
      gtk_text_buffer_insert (buffer, &iter, "</b>", -1);
      
    break;
  case STYLE_ITALIC:
    if (gtk_toggle_button_get_active (button)) {
      
      if (GTK_TOGGLE_BUTTON (twp->bold_button)->active)
	gtk_text_buffer_insert (buffer, &iter, "</b>", -1);
      if (GTK_TOGGLE_BUTTON (twp->underline_button)->active)
	gtk_text_buffer_insert (buffer, &iter, "</u>", -1);

      GTK_TOGGLE_BUTTON (twp->bold_button)->active = FALSE;
      gtk_widget_set_state (twp->bold_button,GTK_STATE_NORMAL);
      GTK_TOGGLE_BUTTON (twp->underline_button)->active = FALSE;
      gtk_widget_set_state (twp->underline_button, GTK_STATE_NORMAL);
      gtk_text_buffer_insert (buffer, &iter, "<i>", -1);
    }
    else
      gtk_text_buffer_insert (buffer, &iter, "</i>", -1);
    
    break;
  case STYLE_UNDERLINE:
    if (gtk_toggle_button_get_active (button)) {
      
      if (GTK_TOGGLE_BUTTON (twp->italic_button)->active)
	gtk_text_buffer_insert (buffer, &iter, "</i>", -1);
      if (GTK_TOGGLE_BUTTON (twp->bold_button)->active)
	gtk_text_buffer_insert (buffer, &iter, "</b>", -1);
      
      GTK_TOGGLE_BUTTON (twp->italic_button)->active = FALSE;
      gtk_widget_set_state (twp->italic_button, GTK_STATE_NORMAL);
      GTK_TOGGLE_BUTTON (twp->bold_button)->active = FALSE;
      gtk_widget_set_state (twp->bold_button, GTK_STATE_NORMAL);
      gtk_text_buffer_insert (buffer, &iter, "<u>", -1);
    }
    else
      gtk_text_buffer_insert (buffer, &iter, "</u>", -1);
    
    break;
  }

  gtk_widget_grab_focus (twp->message);
}


static void 
send_button_clicked_cb (GtkWidget *, 
			gpointer data)
{
  GMManager *ep = NULL;
  
  const char *url = NULL;
  gchar *body = NULL;
  gboolean success = FALSE;

  g_return_if_fail (data != NULL);
  
  ep = GnomeMeeting::Process ()->GetManager ();
  
  url = gm_tw_get_url (GTK_WIDGET (data));
  
  if (GMURL (url).IsEmpty ()) /* No URL, ignore return key press */
    return;

  body = gm_tw_get_message_body (GTK_WIDGET (data));

  if (!body || !strcmp (body, ""))
    return;

  ep = GnomeMeeting::Process ()->GetManager ();

  gm_text_chat_window_insert (GTK_WIDGET (data),
			      url,
			      ep->GetDefaultDisplayName (), 
			      body, 
			      0);
  
  gdk_threads_leave ();
  success = ep->SendTextMessage (GMURL(url).GetURL (), body);
  gdk_threads_enter ();

  if (!success)
    gm_text_chat_window_insert (GTK_WIDGET (data), url, NULL, 
				_("Error: Failed to transmit message"), 2);
}


static void 
new_button_clicked_cb (GtkWidget *widget, 
		       gpointer data)
{
  GmTextChatWindow *tw = NULL;

  GMManager *ep = NULL;

  gchar *name = NULL;
  gchar *url = NULL;


  ep = GnomeMeeting::Process ()->GetManager ();
  
  tw = gm_tw_get_tw (GTK_WIDGET (data));
  
  g_return_if_fail (tw != NULL);
  g_return_if_fail (data != NULL);


  /* Check if there is an active call */
  gdk_threads_leave ();
  ep->GetCurrentConnectionInfo (name, url);
  gdk_threads_enter ();

  /* If we are in a call, add a tab with the given URL if there
   * is none.
   */
  if (url && !gm_text_chat_window_has_tab (GTK_WIDGET (data), url)) {

    gm_text_chat_window_add_tab (GTK_WIDGET (data), url, name);
    if (url)
      gm_chat_window_update_calling_state (GTK_WIDGET (data), 
					   name,
					   url, 
					   GMManager::Connected);
  }
  else
    gm_text_chat_window_add_tab (GTK_WIDGET (data), NULL, NULL);
}


static void 
url_entry_changed_cb (GtkWidget *entry, 
		      gpointer data)
{
  GMManager *ep = NULL;
  GmTextChatWindowPage *twp = NULL;
  
  const char *value = NULL;

  PSafePtr <OpalCall> call = NULL;
  PSafePtr <OpalConnection> connection = NULL;

  gchar *utf8_name = NULL;
  gchar *utf8_app = NULL;
  gchar *utf8_url = NULL;
  
  g_return_if_fail (data != NULL);

  twp = gm_tw_get_current_twp (GTK_WIDGET (data));
  ep = GnomeMeeting::Process ()->GetManager ();

  value = gtk_entry_get_text (GTK_ENTRY (entry));
  gtk_label_set_text (GTK_LABEL (twp->tab_label), value);

  gdk_threads_leave ();
  if (strcmp (value, "")) { 

    call = ep->FindCallWithLock (ep->GetCurrentCallToken ());
    if (call != NULL) {

      connection = ep->GetConnection (call, TRUE);

      if (connection != NULL) 
	ep->GetRemoteConnectionInfo (*connection, utf8_name, 
				     utf8_app, utf8_url);
    }
  }
  gdk_threads_enter ();

  if (utf8_url && GMURL (value) == GMURL (utf8_url)) {
    
    gm_chat_window_update_calling_state (GTK_WIDGET (data), 
					 utf8_name,
					 utf8_url, 
					 GMManager::Connected);
  }

  g_free (utf8_app);
  g_free (utf8_name);
  g_free (utf8_url);
}


static gboolean 
gm_tw_urls_history_update_cb (gpointer data)
{
  GmTextChatWindow *tw = NULL;
  GmTextChatWindowPage *twp = NULL;

  GmContact *c = NULL;

  GtkWidget *chat_window = NULL;
  GtkWidget *page = NULL;
  GtkTreeModel *cache_model = NULL;
  GtkEntryCompletion *completion = NULL;
  
  GtkTreeIter tree_iter;
  
  GSList *c1 = NULL;
  GSList *c2 = NULL;
  GSList *contacts = NULL;
  GSList *iter = NULL;

  gchar *entry = NULL;
  int i = 0;
  int nbr = 0;
  
  chat_window = GTK_WIDGET (data);

  g_return_val_if_fail (chat_window != NULL, FALSE);
  
  tw = gm_tw_get_tw (chat_window);

  /* Get the full address book */
  c1 = gnomemeeting_addressbook_get_contacts (NULL,
					      nbr,
					      FALSE,
					      NULL,
					      NULL,
					      NULL,
					      NULL,
					      NULL);
  /* Get the full calls history */
  c2 = gm_calls_history_get_calls (MAX_VALUE_CALL, 25, TRUE, FALSE);
  contacts = g_slist_concat (c1, c2);

  /* Update all of them */
  gdk_threads_enter ();
  for (i=0 ; i<gtk_notebook_get_n_pages (GTK_NOTEBOOK (tw->notebook)) ; i++) {

    page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (tw->notebook), i);

    if (page) {

      /* Get the internal data */
      twp = gm_tw_get_twp (page);
      
      if (twp->remote_url) {

	completion = gtk_entry_get_completion (GTK_ENTRY (twp->remote_url));
	cache_model = 
	  gtk_entry_completion_get_model (GTK_ENTRY_COMPLETION (completion));
	gtk_list_store_clear (GTK_LIST_STORE (cache_model));

	iter = contacts;
	gdk_threads_leave ();
	while (iter) {

	  c = GM_CONTACT (iter->data);
	  if (c->url && strcmp (c->url, "")) {

	    entry = NULL;
	    if (c->fullname && strcmp (c->fullname, ""))
	      entry = g_strdup_printf ("%s [%s]",
				       c-> url, 
				       c->fullname);
	    else
	      entry = g_strdup (c->url);

	    gdk_threads_enter ();
	    gtk_list_store_append (GTK_LIST_STORE (cache_model), &tree_iter);
	    gtk_list_store_set (GTK_LIST_STORE (cache_model), &tree_iter, 
				0, c->fullname,
				1, c->url,
				2, (char *) entry, -1);
	    gdk_threads_leave ();

	    g_free (entry);
	  }

	  iter = g_slist_next (iter);
	}
	gdk_threads_enter ();
      }
    }
  }
  gdk_threads_leave ();

  g_slist_foreach (contacts, (GFunc) gmcontact_delete, NULL);
  g_slist_free (contacts);

  return FALSE;
}


static void
copy_uri_cb (const gchar *uri)
{
  GtkClipboard *cb = NULL;

  g_return_if_fail (uri != NULL);

  cb = gtk_clipboard_get (GDK_NONE);
  gtk_clipboard_set_text (cb, uri, -1);
}


static void
connect_uri_cb (const gchar *uri)
{
  GMManager *ep = NULL;
  
  g_return_if_fail (uri != NULL);

  ep =  GnomeMeeting::Process ()->GetManager ();

  if (ep->GetCallingState () == GMManager::Standby) 
    GnomeMeeting::Process ()->Connect (uri);
}


static void
add_uri_cb (const gchar *uri)
{
  GmContact *contact = NULL;
  
  GtkWidget *addressbook_window = NULL;
  GtkWidget *chat_window = NULL;
  
  g_return_if_fail (uri != NULL);

  addressbook_window = GnomeMeeting::Process ()->GetAddressbookWindow ();
  chat_window = GnomeMeeting::Process ()->GetChatWindow ();
  
  contact = gmcontact_new ();
  contact->url = g_strdup (uri);
  
  gm_addressbook_window_edit_contact_dialog_run (addressbook_window,
						 NULL,
						 contact,
						 FALSE,
						 chat_window);
  gmcontact_delete (contact);
}  


void 
gm_text_chat_window_insert (GtkWidget *chat_window, 
			    const char *url,
			    const char *name,
			    const char *body, 
			    int user)
{
  GmTextChatWindow *tw = NULL;
  GmTextChatWindowPage *twp = NULL;

  GtkTextIter iter;
  GtkTextBuffer *buffer = NULL;
  GtkTextMark *mark = NULL;
  
  GtkWidget *page = NULL;
  
  const char *label = NULL;
  const char *contact_url = NULL;
  gchar *msg = NULL;
  gboolean found = FALSE;
  int i = 0;

  g_return_if_fail (chat_window != NULL);

  /* Get the internal data */
  tw = gm_tw_get_tw (chat_window);

  for (i=0 ; i<gtk_notebook_get_n_pages (GTK_NOTEBOOK (tw->notebook)) ; i++) {

    page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (tw->notebook), i);

    if (page) {

      /* Get the internal data */
      twp = gm_tw_get_twp (page);
      
      contact_url = gtk_entry_get_text (GTK_ENTRY (twp->remote_url));

      if (GMURL (contact_url) == GMURL (url)) {

	found = TRUE;
	break;
      }
    }
  }

  if (!found) {
    
    page = gm_text_chat_window_add_tab (chat_window, url, name);

    /* Get the internal data */
    twp = gm_tw_get_twp (page);
  }

  /* Update the page with confirmed information */
  if (user == 1) {

    g_signal_handlers_block_matched (G_OBJECT (twp->remote_url),
				     G_SIGNAL_MATCH_FUNC,
				     0, 0, NULL,
				     (gpointer) url_entry_changed_cb,
				     chat_window);
    gtk_entry_set_text (GTK_ENTRY (twp->remote_url), url);
    gtk_editable_set_editable (GTK_EDITABLE (twp->remote_url), FALSE);
    g_signal_handlers_unblock_matched (G_OBJECT (twp->remote_url),
				       G_SIGNAL_MATCH_FUNC,
				       0, 0, NULL,
				       (gpointer) url_entry_changed_cb,
				       chat_window);
    if (name) label = name;
    else if (url) label = url;
    gtk_label_set_text (GTK_LABEL (twp->tab_label), label);
  }

  /* Get iter */
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (twp->conversation));
  gtk_text_buffer_get_end_iter (buffer, &iter);

  /* Insert user name */
  msg = g_strdup_printf ("%s %s\n", 
			 (user == 1) ? name : _("You"),
			 /* Translators: "He says", "You say" */
			 (user == 1) ? _("says:") : _("say:"));
  if (user == 1 && (twp->last_user == -1 || twp->last_user == 0)) {
   
    gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, msg, 
					      -1, "remote-user", NULL);
  }
  else if (user == 0 && (twp->last_user == -1 || twp->last_user == 1)) {
    
    gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, msg, 
					      -1, "local-user", NULL);
  }
  g_free (msg);

  /* Insert body */
  if (user == 2) 
    gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, body, 
					      -1, "error", NULL);
  else {

    msg = g_strdup_printf ("[%s]: ",(const char *) PTime ().AsString ("hh:mm"));
    gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, msg, 
					      -1, "timestamp", NULL);
    gtk_text_buffer_insert_with_regex (buffer, &iter, body);
    g_free (msg);
  }
  gtk_text_buffer_insert (buffer, &iter, "\n", -1);
  twp->last_user = user;
  mark = gtk_text_buffer_get_mark (buffer, "current-position");

  /* Auto-scroll */
  gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (twp->conversation), mark, 
				0.0, FALSE, 0,0);
}


GtkWidget *
gm_text_chat_window_new ()
{
  GdkPixbuf *pixbuf = NULL;
  
  GtkWidget *chat_window = NULL;
  GtkWidget *vbox = NULL;
  
  GmTextChatWindow *tw = NULL;

  gchar *filename = NULL;

  /* The window */
  chat_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  g_object_set_data_full (G_OBJECT (chat_window), "window_name",
			  g_strdup ("chat_window"), g_free);
  
  gtk_window_set_title (GTK_WINDOW (chat_window), _("Chat Window"));

  filename = g_build_filename (DATA_DIR, "pixmaps", PACKAGE_NAME ".png", NULL);
  pixbuf = gdk_pixbuf_new_from_file (filename, NULL);
  g_free (filename);
  if (pixbuf) {

    gtk_window_set_icon (GTK_WINDOW (chat_window), pixbuf);
    g_object_unref (pixbuf);
  }

  gtk_window_set_position (GTK_WINDOW (chat_window), GTK_WIN_POS_CENTER);

  /* Set the internal data */
  tw = new GmTextChatWindow ();
  g_object_set_data_full (G_OBJECT (chat_window), "GMObject",
                          (gpointer) tw,
			  (GDestroyNotify) (gm_tw_destroy));

  /* Build the window */
  tw->tips = gtk_tooltips_new ();
  vbox = gtk_vbox_new (FALSE, 0);
  tw->notebook = gtk_notebook_new ();
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (tw->notebook), GTK_POS_BOTTOM);
  gtk_notebook_set_scrollable (GTK_NOTEBOOK (tw->notebook), TRUE);
  gtk_box_pack_start (GTK_BOX (vbox), tw->notebook, TRUE, TRUE, 0);
  tw->statusbar = gm_statusbar_new ();
  gtk_box_pack_start (GTK_BOX (vbox), tw->statusbar, FALSE, FALSE, 0);

  gtk_container_add (GTK_CONTAINER (chat_window), vbox);
  gm_text_chat_window_add_tab (chat_window, NULL, NULL);
  gtk_widget_show_all (vbox);


  /* Signals */
  g_signal_connect (G_OBJECT (chat_window), "delete_event",
		    G_CALLBACK (delete_window_cb), NULL);

  return chat_window;
}


GtkWidget *
gm_text_chat_window_add_tab (GtkWidget *chat_window, 
			     const char *contact_url,
			     const char *contact_name)
{
  GmTextChatWindow *tw = NULL;
  GmTextChatWindowPage *twp = NULL;

  GtkWidget *page = NULL;

  int pos = 0;
  
  /* Get the Data */
  tw = gm_tw_get_tw (chat_window);
  g_return_val_if_fail (chat_window != NULL, NULL);

  if (contact_url != NULL)
    page = gm_tw_get_first_free_tab (chat_window, pos);
  if (page == NULL)
    page = gm_tw_build_tab (chat_window, pos);
  g_return_val_if_fail (page != NULL, NULL);

  twp = gm_tw_get_twp (page);
  g_return_val_if_fail (twp != NULL, NULL);

  if (contact_name) 
    gtk_label_set_text (GTK_LABEL (twp->tab_label), contact_name);
  else if (contact_url)
    gtk_label_set_text (GTK_LABEL (twp->tab_label), contact_url);
  else 
    gtk_label_set_text (GTK_LABEL (twp->tab_label), _("New Remote User"));

  if (contact_url) {

    g_signal_handlers_block_matched (G_OBJECT (twp->remote_url),
				     G_SIGNAL_MATCH_FUNC,
				     0, 0, NULL,
				     (gpointer) url_entry_changed_cb,
				     chat_window);
    gtk_entry_set_text (GTK_ENTRY (twp->remote_url), contact_url);
    gtk_editable_set_editable (GTK_EDITABLE (twp->remote_url), FALSE);
    g_signal_handlers_unblock_matched (G_OBJECT (twp->remote_url),
				       G_SIGNAL_MATCH_FUNC,
				       0, 0, NULL,
				       (gpointer) url_entry_changed_cb,
				       chat_window);
  }

  gtk_widget_show_all (tw->notebook);
  gtk_notebook_set_current_page (GTK_NOTEBOOK (tw->notebook), pos);

  return page;
}


gboolean 
gm_text_chat_window_has_tab (GtkWidget *chat_window, 
			     const char *url)
{
  GmTextChatWindow *tw = NULL;
  GmTextChatWindowPage *twp = NULL;

  GtkWidget *page = NULL;
  
  const char *contact_url = NULL;
  int i = 0;

  g_return_val_if_fail (chat_window != NULL, FALSE);
  
  /* Get the internal data */
  tw = gm_tw_get_tw (chat_window);

  for (i=0 ; i<gtk_notebook_get_n_pages (GTK_NOTEBOOK (tw->notebook)) ; i++) {

    page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (tw->notebook), i);

    if (page) {

      /* Get the internal data */
      twp = gm_tw_get_twp (page);
      
      contact_url = gtk_entry_get_text (GTK_ENTRY (twp->remote_url));

      if (GMURL (contact_url) == GMURL (url)) {

	return TRUE;
	break;
      }
    }
  }

  return FALSE;
}


void 
gm_text_chat_window_urls_history_update (GtkWidget *chat_window)
{
  g_idle_add (gm_tw_urls_history_update_cb, chat_window);
}


void
gm_chat_window_update_calling_state (GtkWidget *chat_window,
				     const char *name,
				     const char *url,
				     unsigned calling_state)
{
  GmTextChatWindow *tw = NULL;
  GmTextChatWindowPage *twp = NULL;

  GmConnectButton *b = NULL;

  GtkWidget *page = NULL;

  const char *contact_url = NULL;
  int i = 0;

  g_return_if_fail (chat_window != NULL);

  /* Get the internal data */
  tw = gm_tw_get_tw (chat_window);

  
  /* Update them all */
  for (i=0 ; i < gtk_notebook_get_n_pages (GTK_NOTEBOOK (tw->notebook)) ; i++) {

    page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (tw->notebook), i);

    if (page) {

      /* Get the internal data */
      twp = gm_tw_get_twp (page);

      contact_url = gtk_entry_get_text (GTK_ENTRY (twp->remote_url));
      b = GM_CONNECT_BUTTON (twp->connect_button);

      /* When we are in a call, and changing the state of the corresponding
       * tab, then prevent editing the url.
       */
      if (url 
	  && GMURL (contact_url) == GMURL (url)
	  && calling_state != GMManager::Standby)
	gtk_editable_set_editable (GTK_EDITABLE (twp->remote_url), FALSE);
      else
	gtk_editable_set_editable (GTK_EDITABLE (twp->remote_url), TRUE);

      if (!url || GMURL (contact_url) == GMURL (url)) {
	
	if (name)
	  gtk_label_set_text (GTK_LABEL (twp->tab_label), name);

	switch (calling_state) {
	case GMManager::Standby:

	  gm_connect_button_set_connected (b, FALSE);
	  break;


	case GMManager::Calling:

	  gm_connect_button_set_connected (b, TRUE);
	  break;


	case GMManager::Connected:

	  gm_connect_button_set_connected (b, TRUE);
	  break;


	case GMManager::Called:

	  gm_connect_button_set_connected (b, FALSE);
	  break;
	}
      }
    }
  }
}


void 
gm_chat_window_push_info_message (GtkWidget *chat_window, 
				  const char *url,
				  const char *msg, 
				  ...)
{
  GmTextChatWindow *tw = NULL;
  GmTextChatWindowPage *twp = NULL;
  
  const char *contact_url = NULL;

  g_return_if_fail (chat_window != NULL);

  tw = gm_tw_get_tw (chat_window);
  twp = gm_tw_get_current_twp (chat_window);

  va_list args;

  va_start (args, msg);

  contact_url = gtk_entry_get_text (GTK_ENTRY (twp->remote_url));
  
  if (!url || GMURL (contact_url) == GMURL (url))
    gm_statusbar_push_info_message (GM_STATUSBAR (tw->statusbar), msg, args);
  va_end (args);
}
