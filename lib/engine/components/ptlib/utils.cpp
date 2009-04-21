#include <glib.h>

#include "utils.h"

const std::string
latin2utf (const std::string str)
{
  gchar *utf8_str;
  std::string result;

  if (g_utf8_validate (str.c_str (), -1, NULL))
    utf8_str = g_strdup (str.c_str ());
  else
    utf8_str = g_convert (str.c_str (), -1,
                          "UTF-8", "ISO-8859-1",
                          NULL, NULL, NULL);

  result = std::string (utf8_str);

  g_free (utf8_str);

  return result;
}


const std::string
utf2latin (const std::string str)
{
  gchar *utf8_str;
  std::string result;

  g_warn_if_fail (g_utf8_validate (str.c_str (), -1, NULL));
  utf8_str = g_convert (str.c_str (), -1,
                        "ISO-8859-1", "UTF-8",
                        NULL, NULL, NULL);

  result = std::string (utf8_str);

  g_free (utf8_str);

  return result;
}
