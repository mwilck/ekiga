#include "toolbox.h"

#include <windows.h>

void
gm_open_uri (const gchar *uri)
{
  SHELLEXECUTEINFO sinfo;

  g_return_if_fail (uri != NULL);

  memset (&sinfo, 0, sizeof (sinfo));
  sinfo.cbSize = sizeof (sinfo);
  sinfo.fMask = SEE_MASK_CLASSNAME;
  sinfo.lpVerb = "open";
  sinfo.lpFile = uri;
  sinfo.nShow = SW_SHOWNORMAL;
  sinfo.lpClass = "http";

  (void)ShellExecuteEx (&sinfo); /* leave out any error */
}
