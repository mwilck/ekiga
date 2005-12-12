#include <glib.h>

const gchar *win32_sysconfdir (void);
const gchar *win32_datadir (void);

#undef SYSCONFDIR
#define SYSCONFDIR win32_sysconfdir()

#undef DATA_DIR
#define DATA_DIR win32_datadir()
