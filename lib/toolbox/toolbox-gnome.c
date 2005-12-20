#include "toolbox.h"

#include <gnome.h>

void
gm_open_uri (const gchar *uri)
{
  g_return_if_fail (uri != NULL);

  gnome_url_show (uri, NULL);
}
