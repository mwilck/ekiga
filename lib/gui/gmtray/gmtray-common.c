#define __GMTRAY_IMPLEMENTATION__

#include "gmtray-internal.h"


/* helper function that makes the tray blink
 * it always returns TRUE, because its id is kept
 * within the tray, and can else be cancelled whenever
 * we want
 */
static gboolean
blink_timeout (gpointer data)
{
  GmTray *tray = data;

  g_return_val_if_fail (tray != NULL, FALSE);

  if (tray->blink_shown) {

    os_tray_show_image (tray, tray->base_image);
    tray->blink_shown = FALSE;
  } else {

    os_tray_show_image (tray, tray->blink_image);
    tray->blink_shown = TRUE;
  }

  return TRUE;
}


/* implementation of public functions */


GmTray *
os_tray_new_common (const gchar *image)
{
  GmTray *result = NULL;

  result = g_new (GmTray, 1);

  result->base_image = g_strdup (image);
  result->blink_image = NULL;
  result->blink_shown = FALSE;
  result->blink_id = -1;
  result->clicked_callback = NULL;
  result->menu_callback = NULL;

  return result;
}


void
os_tray_delete_common (GmTray *tray)
{
  g_return_if_fail (tray != NULL);

  os_tray_stop_blink (tray);

  g_free (tray->base_image);
  tray->base_image = NULL;

  g_free (tray);
}


void
os_tray_set_image (GmTray *tray, const gchar *image)
{
  gchar *old_image = NULL;

  g_return_if_fail (tray != NULL);

  old_image = tray->base_image;
  tray->base_image = g_strdup (image);
  g_free (old_image);

  os_tray_show_image (tray, tray->base_image);
}


gboolean
os_tray_is_blinking (GmTray *tray)
{
  g_return_val_if_fail (tray != NULL, FALSE);

  if (tray->blink_id == -1)
    return FALSE;
  else
    return TRUE;
}


void
os_tray_blink (GmTray *tray, const gchar *blink_image, guint interval)
{
  g_return_if_fail (tray != NULL);
  g_return_if_fail (tray->blink_image == NULL);
  g_return_if_fail (interval > 0);

  tray->blink_image = g_strdup (blink_image);
  tray->blink_id = g_timeout_add (interval, blink_timeout, (gpointer)tray);
}


void
os_tray_stop_blink (GmTray *tray)
{
  g_return_if_fail (tray != NULL);

  if (tray->blink_id != -1) {

    g_source_remove (tray->blink_id);
    tray->blink_id = -1;
    g_free (tray->blink_image);
    tray->blink_image = NULL;
  }

  os_tray_show_image (tray, tray->base_image);
  tray->blink_shown = FALSE;
}


void
os_tray_set_clicked_callback (GmTray *tray, void (*callback)(void))
{
  g_return_if_fail (tray != NULL);

  tray->clicked_callback = callback;
}


void
os_tray_set_menu_callback (GmTray *tray, GtkMenu *(*callback)(void))
{
  g_return_if_fail (tray != NULL);

  tray->menu_callback = callback;
}
