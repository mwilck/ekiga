noinst_LTLIBRARIES = libgmguiroster.la

gmguiroster_dir = $(top_srcdir)/lib/engine/gui/roster

AM_CPPFLAGS = $(XML_CFLAGS) $(SIGC_CFLAGS) $(GLIB_CFLAGS) $(GTK_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/gui 			\
	-I$(top_srcdir)/lib/engine/gui 			\
	-I$(top_srcdir)/lib/engine/gui/common		\
	-I$(top_srcdir)/lib/engine/include 		\
	-I$(top_srcdir)/lib/engine/include/framework 	\
	-I$(top_srcdir)/lib/engine/include/presence

libgmguiroster_la_SOURCES = \
	$(gmguiroster_dir)/roster-view-gtk.h \
	$(gmguiroster_dir)/roster-view-gtk.cpp 

libgmguiroster_la_LIBADD = \
	$(top_srcdir)/lib/gui/libgmwidgets.la				\
	$(top_srcdir)/lib/engine/src/presence/libgmpresence.la

libgmguiroster_la_LDFLAGS = -export-dynamic -no-undefined $(XML_LIBS) $(SIGC_LIBS) $(GLIB_LIBS) $(GTK_LIBS)
