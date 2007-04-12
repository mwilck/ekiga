noinst_LTLIBRARIES = libgmconf.la

libgmconf_la_SOURCES = 	\
	gmconf.h

if DISABLE_GNOME
libgmconf_la_SOURCES += gmconf-glib.c
else
libgmconf_la_SOURCES +=	gmconf-gconf.c
endif

INCLUDES = $(EKIGA_LIB_CFLAGS) \
	   -DSYSCONFDIR=\""$(sysconfdir)"\"		\
	   -I$(top_srcdir)/lib

libgmconf_la_LDFLAGS = -export-dynamic
