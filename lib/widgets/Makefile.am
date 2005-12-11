noinst_LTLIBRARIES = libgmwidgets.la

libgmwidgets_la_SOURCES =				\
	connectbutton.c					\
	connectbutton.h					\
	gmentrydialog.c					\
	gmentrydialog.h					\
	gmstatusbar.c					\
	gmstatusbar.h					\
	gtklevelmeter.c					\
	gtklevelmeter.h					

INCLUDES =						\
	-DG_LOG_DOMAIN=\"GnomeMeeting\"		 	\
	-DSHARE_DIR=\"$(pkgdatadir)\"			\
	-I$(top_srcdir) 				\
	$(GNOMEMEETING_LIB_CFLAGS)

libgmwidgets_la_LDFLAGS = -export-dynamic
libgmwidgets_la_LIDADD = 
