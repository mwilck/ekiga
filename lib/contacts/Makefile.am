noinst_LTLIBRARIES = libgmcontacts.la

libgmcontacts_la_SOURCES = \
			gm_contacts.h \
			gm_contacts.cpp \
			gm_contacts-remote.h \
			gm_contacts-remote.cpp \
			gm_contacts-local.h \
			gm_contacts-convert.h \
			gm_contacts-convert.c \
			e-vcard.h \
			e-vcard.c

if DISABLE_GNOME
libgmcontacts_la_SOURCES += gm_contacts-gm_conf.cpp
else
libgmcontacts_la_SOURCES += gm_contacts-eds.cpp
endif

INCLUDES =						\
	-DGNOMELOCALEDIR=\""$(datadir)/locale/"\"        \
	-DG_LOG_DOMAIN=\"GnomeMeeting\"		 	\
	-DSHARE_DIR=\"$(pkgdatadir)\"			\
	-I$(top_srcdir) 				\
	$(GNOMEMEETING_LIB_CFLAGS)				

libgmcontacts_la_LDFLAGS = -export-dynamic
libgmcontacts_la_LIDADD = 
