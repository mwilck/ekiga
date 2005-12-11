noinst_LTLIBRARIES = libgmcontacts.la

libgmcontacts_la_SOURCES = \
			gm_contacts.h \
			gm_contacts.cpp \
			gm_contacts-remote.h \
			gm_contacts-remote.cpp \
			gm_contacts-ldap.h \
			gm_contacts-ldap.cpp \
			gm_contacts-local.h \
			gm_contacts-dnd.h \
			gm_contacts-dnd.cpp \
			gm_contacts-convert.h \
			gm_contacts-convert.cpp \
			e-vcard.h \
			e-vcard.c

if DISABLE_GNOME
libgmcontacts_la_SOURCES += gm_contacts-gm_conf.cpp
else
libgmcontacts_la_SOURCES += gm_contacts-eds.cpp
endif

if HAS_HOWL
libgmcontacts_la_SOURCES += gm_contacts-zeroconf.cpp gm_contacts-zeroconf.h
endif

if HAS_AVAHI
libgmcontacts_la_SOURCES += gm_contacts-avahi.cpp gm_contacts-avahi.h
endif

INCLUDES =						\
	-DG_LOG_DOMAIN=\"GnomeMeeting\"		 	\
	-DSHARE_DIR=\"$(pkgdatadir)\"			\
	-I$(top_srcdir) 				\
	@GNOMEMEETING_CFLAGS@				

libgmcontacts_la_LDFLAGS = -export-dynamic
libgmcontacts_la_LIDADD = 
