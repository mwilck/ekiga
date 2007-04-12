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

if DISABLE_GNOME
libgmcontacts_la_SOURCES += gmcontacts-gmconf.cpp
else
libgmcontacts_la_SOURCES += gmcontacts-eds.cpp
endif

if HAS_AVAHI
libgmcontacts_la_SOURCES += gmcontacts-avahi.cpp gmcontacts-avahi.h
endif

INCLUDES =						\
	-DG_LOG_DOMAIN=\"Ekiga\"		 	\
	-DSHARE_DIR=\"$(pkgdatadir)\"			\
	-I../gmconf/					\
	-I$(top_srcdir) 				\
	@EKIGA_CFLAGS@				

libgmcontacts_la_LDFLAGS = -export-dynamic
libgmcontacts_la_LIBADD = 
