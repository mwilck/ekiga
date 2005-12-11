noinst_LTLIBRARIES = libabout.la

libabout_la_SOURCES =	gnome-about.c			\
			gnome-about.h

INCLUDES =						\
	-DGNOMELOCALEDIR=\""$(datadir)/locale/"\"        \
	-DSHARE_DIR=\"$(pkgdatadir)\"			\
	-I$(top_srcdir) 				\
	$(GNOMEMEETING_LIB_CFLAGS)				

libabout_la_LDFLAGS = -export-dynamic
libabout_la_LIDADD = 
