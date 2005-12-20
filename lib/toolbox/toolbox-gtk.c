#include "toolbox.h"

void
gm_open_uri (const gchar *uri)
{
  static gchar *command = "sensible-browser %s";
  gchar *commandline = NULL;

  g_return_if_fail (uri != NULL);

  commandline = g_strdup_printf (command, uri);
  g_spawn_command_line_async (commandline, NULL);
  g_free (commandline);
}
