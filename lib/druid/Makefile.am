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
	-I$(top_srcdir) 				

AM_CFLAGS = $(GTK_CFLAGS) $(GLIB_CFLAGS)

libdruid_la_LDFLAGS = -export-dynamic $(GTK_LIBS) $(GLIB_LIBS)
libdruid_la_LIDADD = 
