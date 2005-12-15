#include <glib/gwin32.h>

#include "winpaths.h"

static const gchar *
win32_basedir ()
{
  static gchar *result = NULL;

  if (!result)
    result = g_win32_get_package_installation_directory (NULL, NULL);

  return result;
}

const gchar *
win32_sysconfdir ()
{
  return win32_basedir ();
}

const gchar *
win32_datadir ()
{
  return win32_basedir ();
}
