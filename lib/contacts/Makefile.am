noinst_LTLIBRARIES = libgmcontacts.la

libgmcontacts_la_SOURCES =				\
	gm_contacts-eds.cpp				\
	gm_contacts.h					

INCLUDES =						\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"        \
	-DG_LOG_DOMAIN=\"GnomeMeeting\"		 	\
	-DSHARE_DIR=\"$(pkgdatadir)\"			\
	-I$(top_srcdir) 				\
	$(GNOMEMEETING_LIB_CFLAGS)				

libgmcontacts_la_LDFLAGS = -export-dynamic
libgmcontacts_la_LIDADD = 
