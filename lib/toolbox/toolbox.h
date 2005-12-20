#include <glib.h>

G_BEGIN_DECLS

/* DESCRIPTION  : /
 * BEHAVIOR     : Allows to open an uri in a browser,
 * 		  in a system-agnostic way
 * PRE		: Requires a non-NULL uri.
 */
void gm_open_uri (const gchar *uri);

G_END_DECLS
