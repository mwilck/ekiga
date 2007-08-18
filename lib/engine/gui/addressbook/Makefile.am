noinst_LTLIBRARIES = libgmguiaddressbook.la

gmguiaddressbook_dir = $(top_srcdir)/lib/engine/gui/addressbook

AM_CPPFLAGS = $(GTK_CFLAGS) $(SIGC_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/gui/ 			\
	-I$(top_srcdir)/lib/engine/gui/			\
	-I$(top_srcdir)/lib/engine/gui/common		\
	-I$(top_srcdir)/lib/engine/include 		\
	-I$(top_srcdir)/lib/engine/framework 		\
	-I$(top_srcdir)/lib/engine/addressbook/skel

libgmguiaddressbook_la_SOURCES = \
	$(gmguiaddressbook_dir)/addressbook.h \
	$(gmguiaddressbook_dir)/addressbook.cpp \
	$(gmguiaddressbook_dir)/book-view-gtk.h \
	$(gmguiaddressbook_dir)/book-view-gtk.cpp 

libgmguiaddressbook_la_LIBADD = \
	$(top_builddir)/lib/engine/addressbook/skel/libgmaddressbook.la

libgmguiaddressbook_la_LDFLAGS = -export-dynamic -no-undefined $(GTK_LIBS) $(GLIB_LIBS) $(SIGC_LIBS)
