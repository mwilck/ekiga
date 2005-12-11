#include <windows.h>

#include "winpaths.h"

static const gchar *
win32_basedir ()
{
  static gchar *result = NULL;

  if (!result) {

    gchar *fullpath = NULL;

    if (G_WIN32_HAVE_WIDECHAR_API ()) {

      wchar_t winstall_dir [MAXPATHLEN];

      if (GetModuleFileNameW (NULL, winstall_dir,
			      MAXPATHLEN) > 0) {

	fullpath = g_utf16_to_utf8 (winstall_dir, -1,
				    NULL, NULL, NULL);
      }
    } else {

      gchar ainstall_dir [MAXPATHLEN];

      if (GetModuleFileNameA (NULL, ainstall_dir,
			      MAXPATHLEN) > 0) {

	fullpath = g_locale_to_utf8 (ainstall_dir, -1,
				     NULL, NULL, NULL);
      }
    }

    if (fullpath) {

      result = g_path_get_dirname (fullpath);
      g_free (fullpath);
    }
  }

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
