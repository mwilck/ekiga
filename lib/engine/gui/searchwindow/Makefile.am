noinst_LTLIBRARIES = libgmguisearchwindow.la

gmguisearchwindow_dir = $(top_srcdir)/lib/engine/gui/searchwindow

AM_CPPFLAGS = $(GTK_CFLAGS) $(SIGC_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/gui/ 			\
	-I$(top_srcdir)/lib/engine/gui/			\
	-I$(top_srcdir)/lib/engine/include 		\
	-I$(top_srcdir)/lib/engine/include/framework 	\
	-I$(top_srcdir)/lib/engine/include/addressbook

libgmguisearchwindow_la_SOURCES = \
	$(gmguisearchwindow_dir)/search-window.h \
	$(gmguisearchwindow_dir)/search-window.cpp \
	$(gmguisearchwindow_dir)/book-view-gtk.h \
	$(gmguisearchwindow_dir)/book-view-gtk.cpp 

libgmguisearchwindow_la_LIBADD = \
	$(top_builddir)/lib/engine/src/addressbook/libgmaddressbook.la

libgmguisearchwindow_la_LDFLAGS = -export-dynamic -no-undefined $(GTK_LIBS) $(GLIB_LIBS) $(SIGC_LIBS)
