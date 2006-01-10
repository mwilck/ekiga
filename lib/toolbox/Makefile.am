noinst_LTLIBRARIES = libtoolbox.la

libtoolbox_la_SOURCES = toolbox.h

if WIN32
libtoolbox_la_SOURCES += toolbox-win32.c
else
if DISABLE_GNOME
libtoolbox_la_SOURCES += toolbox-gtk.c
else
libtoolbox_la_SOURCES += toolbox-gnome.c
endif
endif

INCLUDES = $(EKIGA_LIB_CFLAGS)

libtoolbox_la_LDFLAGS = -export-dynamic
