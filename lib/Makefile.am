SUBDIRS =						\
	widgets						\
	xdap						

noinst_LTLIBRARIES = libgnomemeeting.la

libgnomemeeting_la_SOURCES =				\
	dialog.c					\
	dialog.h					\
	stock-icons.c					\
	stock-icons.h					\
	gtk-text-buffer-extentions.c			\
	gtk-text-buffer-extentions.h			\
	e-splash.c					\
	e-splash.h					\
	eggtrayicon.h					\
	eggtrayicon.c

INCLUDES =						\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"        \
	-DGNOMEMEETING_IMAGES=\""$(datadir)/pixmaps"\"	\
	-DG_LOG_DOMAIN=\"GnomeMeeting\"		 	\
	-DSHARE_DIR=\"$(pkgdatadir)\"			\
	-I$(top_srcdir) 				\
	$(GNOMEMEETING_CFLAGS)

libgnomemeeting_la_LDFLAGS = -export-dynamic
libgnomemeeting_la_LIBADD = widgets/libgmwidgets.la xdap/libxdap.la 
