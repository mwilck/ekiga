noinst_LTLIBRARIES = libdruid.la

libdruid_la_SOURCES =					\
			gnome-druid.c			\
			gnome-druid.h			\
			gnome-druid-page.c		\
			gnome-druid-page.h		\
			gnome-druid-page-edge.c		\
			gnome-druid-page-edge.h		\
			gnome-druid-page-standard.c	\
			gnome-druid-page-standard.h

INCLUDES =						\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"        \
	-DGNOMEMEETING_IMAGES=\""$(datadir)/pixmaps"\"	\
	-DSHARE_DIR=\"$(pkgdatadir)\"			\
	-I$(top_srcdir) 				\
	$(GNOMEMEETING_LIB_CFLAGS)				

libdruid_la_LDFLAGS = -export-dynamic
libdruid_la_LIDADD = 
