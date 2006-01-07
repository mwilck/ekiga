#include "gmtray.h"

#ifndef __GMTRAY_INTERNAL_H__
#define __GMTRAY_INTERNAL_H__

#ifndef __GMTRAY_IMPLEMENTATION__
#error "Shouldn't be used outside of the implementation of libgmtray !"
#endif

G_BEGIN_DECLS

/* declare -- but don't define -- the structure that will hold all
 * implementation-specific details */
typedef struct _GmTraySpecific GmTraySpecific;

/* this structure makes available the os-independent part of the data
 */
struct _GmTray
{
  gchar *base_image; /* the stock-id of the image supposedly shown */

  gchar *blink_image; /* the stock-id of the image shown half of the time
		       * when blinking
		       */
  gboolean blink_shown; /* do we show the blink image or the base image ? */
  guint blink_id; /* the id of the timeout function -- kept to be able to
		   * disable it whenever we want */

  void (*clicked_callback) (void); /* the callback the user said to call when
				    * the tray is clicked (the fact that we're
				    * not a real GObject makes so that there
				    * can be only one, but that terrible
				    * restriction will go away when the gtk+
				    * team will have released a version with
				    * GtkStatusIcon)
				    */

  GtkMenu *(*menu_callback) (void); /* the callback which tells us which menu
				     * to show when the tray is right-clicked
				     * (should hopefully allow to make said
				     * menu more context-sensitive)
				     */

  GmTraySpecific *specific; /* to let each implementation keep what it needs */
};


/* DESCRIPTION : /
 * BEHAVIOR    : creates a new common tray -- used by the implementations
 * PRE         : image should be a valid stock id
 * NOTICE      : implemented in gmtray-common.c
 */
GmTray *gmtray_new_common (const gchar *image);


/* DESCRIPTION : /
 * BEHAVIOR    : creates a new common tray -- used by the implementations
 * PRE         : image should be a valid stock id
 * NOTICE      : implemented in gmtray-common.c
 */
void gmtray_delete_common (GmTray *tray);


/* DESCRIPTION : /
 * BEHAVIOR    : Sets the tray to the given image
 *               (changes what is shown only, contrary to gmtray_set_image)
 * PRE         : tray and image shouldn't be NULL
 * NOTICE      : this is os-specific
 */
void gmtray_show_image (GmTray *tray, const gchar *image);


/* DESCRIPTION : /
 * BEHAVIOR    : Prompts the tray to show its associated menu
 * PRE         : tray shouldn't be NULL
 * NOTICE      : this is os-specific
 */
void gmtray_menu (GmTray *tray);

G_END_DECLS

#endif /* __GMTRAY_INTERNAL_H__ */
