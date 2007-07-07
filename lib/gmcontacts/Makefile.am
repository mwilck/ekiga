noinst_LTLIBRARIES = libgmcontacts.la

libgmcontacts_la_SOURCES = \
			gmcontacts.h \
			gmcontacts.cpp \
			gmcontacts-remote.h \
			gmcontacts-remote.cpp \
			gmcontacts-ldap.h \
			gmcontacts-ldap.cpp \
			gmcontacts-local.h \
			gmcontacts-dnd.h \
			gmcontacts-dnd.cpp \
			gmcontacts-convert.h \
			gmcontacts-convert.cpp \
			e-vcard.h \
			e-vcard.c

INCLUDES =						\
	-DG_LOG_DOMAIN=\"Ekiga\"		 	\
	-DSHARE_DIR=\"$(pkgdatadir)\"			\
	-I../gmconf/					\
	-I../toolbox/					\
	-I$(top_srcdir) 

if HAVE_EDS
libgmcontacts_la_SOURCES += gmcontacts-eds.cpp
else
libgmcontacts_la_SOURCES += gmcontacts-gmconf.cpp
endif

if HAVE_AVAHI
libgmcontacts_la_SOURCES += gmcontacts-avahi.cpp gmcontacts-avahi.h
endif

AM_CXXFLAGS = $(GTK_CFLAGS) $(GLIB_CFLAGS) $(EDS_CFLAGS) $(AVAHI_CFLAGS)
AM_CFLAGS = $(GTK_CFLAGS) $(GLIB_CFLAGS) $(EDS_CFLAGS) $(AVAHI_CFLAGS)
AM_LIBS = $(GTK_LIBS) $(GLIB_LIBS) $(EDS_LIBS) $(AVAHI_LIBS)

libgmcontacts_la_LDFLAGS = -export-dynamic $(AM_LIBS)
