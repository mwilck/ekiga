SUBDIRS = widgets

noinst_LTLIBRARIES = libgm.la

libgm_la_SOURCES =					\
	dialog.c					\
	dialog.h					\
	stock-icons.c					\
	stock-icons.h					\
	e-splash.c					\
	e-splash.h					

INCLUDES =						\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"        \
	-DGNOMEMEETING_IMAGES=\""$(datadir)/pixmaps"\"	\
	-DG_LOG_DOMAIN=\"GnomeMeeting\"		 	\
	-DSHARE_DIR=\"$(pkgdatadir)\"			\
	-I$(top_srcdir) 				\
	$(GNOMEMEETING_CFLAGS)

libgm_la_LDFLAGS = -export-dynamic
libgm_la_LIBADD = widgets/libgmwidgets.la
