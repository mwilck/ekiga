#define __GMTRAY_IMPLEMENTATION__

#include "../../../config.h"

#include <windows.h>

#include "gmtray-internal.h"

#include "pixbuf_to_hicon.h"

struct _GmTraySpecific
{
  NOTIFYICONDATA nid; /* this is the windows tray */
  GtkWidget *stupid_platform; /* we need this widget we never show to render
			       * the icons in it... */
};


/* helper functions */


/* this function receives events on the tray, and decides whether to call
 * the click callback, show the menu or ignore
 */
static LRESULT CALLBACK
message_handler (HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  GmTray *tray = NULL;

  tray = (GmTray *)wparam;

  if (msg == WM_USER) {

    if (lparam == WM_LBUTTONDOWN) {

      if (tray->clicked_callback)
	tray->clicked_callback ();
    } else if (lparam == WM_RBUTTONDOWN) {

      os_tray_menu (tray);
    }
    return 0;
  }

  return DefWindowProc (hwnd, msg, wparam, lparam);
}


/* creates a sort of win32 event box, associated with
 * our message callback function
 */
static HWND
create_message_window ()
{
  WNDCLASS wclass;
  ATOM klass;
  HINSTANCE hmodule = GetModuleHandle (NULL);

  memset (&wclass, 0, sizeof (WNDCLASS));
  wclass.lpszClassName = PACKAGE_NAME "-tray";
  wclass.lpfnWndProc = message_handler;

  klass = RegisterClass (&wclass);

  return CreateWindow (MAKEINTRESOURCE (klass), NULL, WS_POPUP,
		       0, 0, 1, 1, NULL, NULL,
		       hmodule, NULL);

}


/* public api implementation */


GmTray *
os_tray_new (const gchar *image)
{
  GmTray    *result = NULL;
  GdkPixbuf *pixbuf = NULL;
  GtkWidget *stupid = NULL;

  stupid = gtk_label_new ("");
  pixbuf = gtk_widget_render_icon (stupid, image,
				   GTK_ICON_SIZE_SMALL_TOOLBAR, NULL);

  result = os_tray_new_common (image);

  result->specific = g_new (GmTraySpecific, 1);

  result->specific->stupid_platform = stupid;

  memset (&result->specific->nid, 0, sizeof (result->specific->nid));
  result->specific->nid.hWnd = create_message_window ();
  result->specific->nid.uID = GPOINTER_TO_UINT (result);
  result->specific->nid.uCallbackMessage = WM_USER;
  result->specific->nid.uFlags = NIF_ICON | NIF_MESSAGE;
  result->specific->nid.hIcon = _gdk_win32_pixbuf_to_hicon (pixbuf);

  Shell_NotifyIcon (NIM_ADD, &result->specific->nid);

  g_object_unref (pixbuf);

  return result;
}


void
os_tray_delete (GmTray *tray)
{
  g_return_if_fail (tray != NULL);

  gtk_widget_destroy (tray->specific->stupid_platform);
  Shell_NotifyIcon (NIM_DELETE, &tray->specific->nid);
  g_free (tray->specific);
  os_tray_delete_common (tray);
}


void
os_tray_show_image (GmTray *tray, const gchar *image)
{
  GdkPixbuf *pixbuf = NULL;

  g_return_if_fail (tray != NULL);

  pixbuf = gtk_widget_render_icon (tray->specific->stupid_platform,
				   image, GTK_ICON_SIZE_SMALL_TOOLBAR, NULL);

  tray->specific->nid.hIcon = _gdk_win32_pixbuf_to_hicon (pixbuf);
  tray->specific->nid.uFlags |= NIF_ICON;

  Shell_NotifyIcon (NIM_MODIFY, &tray->specific->nid);

  g_object_unref (pixbuf);
}


void
os_tray_menu (GmTray *tray)
{
  GtkMenu *menu = NULL;

  g_return_if_fail (tray != NULL);

  if (tray->menu_callback == NULL)
    return;

  menu = tray->menu_callback ();

  gtk_widget_show_all (GTK_WIDGET (menu));

  gtk_menu_popup (menu, NULL, NULL, NULL, NULL,
		  0, gtk_get_current_event_time ());
}
