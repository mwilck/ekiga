noinst_LTLIBRARIES = libabout.la

libabout_la_SOURCES =	gnome-about.c			\
			gnome-about.h

INCLUDES =						\
	-DSHARE_DIR=\"$(pkgdatadir)\"			\
	-I$(top_srcdir) 				\
	$(EKIGA_LIB_CFLAGS)				

libabout_la_LDFLAGS = -export-dynamic
libabout_la_LIDADD = 
