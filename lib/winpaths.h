#include <glib.h>

const gchar *win32_sysconfdir (void);
const gchar *win32_datadir (void);

#undef SYSCONFDIR
#define SYSCONFDIR win32_sysconfdir()

#undef DATADIR
#define DATADIR win32_datadir()
