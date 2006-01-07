#include <gtk/gtk.h>

#ifndef __GMTRAY_H__
#define __GMTRAY_H__

G_BEGIN_DECLS

/* our opaque data structure */
typedef struct _GmTray GmTray;

/* DESCRIPTION : /
 * BEHAVIOR    : Creates a tray with the given image
 * PRE         : image should be a valid stock id
 */
GmTray *os_tray_new (const gchar *image);


/* DESCRIPTION : /
 * BEHAVIOR    : Deletes the tray
 * PRE         : tray shouldn't be NULL
 */
void os_tray_delete (GmTray *tray);


/* DESCRIPTION : /
 * BEHAVIOR    : Sets the tray to the given image
 * PRE         : tray shouldn't be NULL and image should be a valid stock id
 */
void os_tray_set_image (GmTray *tray, const gchar *image);


/* DESCRIPTION : /
 * BEHAVIOR    : Checks if the tray isn't already blinking
 * PRE         : tray shouldn't be NULL
 */
gboolean os_tray_is_blinking (GmTray *tray);


/* DESCRIPTION : /
 * BEHAVIOR    : Make the tray blink between its base image and the blink_image
 *               (changing which is shown every interval)
 * PRE         : tray and blink_image shouldn't be NULL, interval shouldn't be
 *               zero and the tray shouldn't already be blinking
 */
void os_tray_blink (GmTray *tray, const gchar *blink_image, guint interval);


/* DESCRIPTION : /
 * BEHAVIOR    : Make the tray stop blinking, if it already was (ok if not)
 * PRE         : tray shouldn't be NULL
 */
void os_tray_stop_blink (GmTray *tray);


/* DESCRIPTION : /
 * BEHAVIOR    : Define which function to call when the tray is clicked
 * PRE         : tray shouldn't be NULL
 */
void os_tray_set_clicked_callback (GmTray *tray, void (*callback)(void));


/* DESCRIPTION : /
 * BEHAVIOR    : Define which function to call to obtain a menu for the tray
 *               (notice : you'll be responsible with disposing from it)
 * PRE         : tray shouldn't be NULL
 */
void os_tray_set_menu_callback (GmTray *tray, GtkMenu *(*callback)(void));

G_END_DECLS

#endif /* __GMTRAY_H__ */
