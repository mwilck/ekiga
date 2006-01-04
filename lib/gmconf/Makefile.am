noinst_LTLIBRARIES = libgmconf.la

libgmconf_la_SOURCES = 	\
	gmconf.h

if DISABLE_GNOME
libgmconf_la_SOURCES += gmconf-glib.c
else
libgmconf_la_SOURCES +=	gmconf-gconf.c
endif

INCLUDES = $(GNOMEMEETING_LIB_CFLAGS)

libgmconf_la_LDFLAGS = -export-dynamic
