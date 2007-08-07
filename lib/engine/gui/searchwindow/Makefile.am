noinst_LTLIBRARIES = libekiga_gtk_searchwindow.la

gtk_searchwindow_dir = $(top_srcdir)/lib/engine/gui/searchwindow

AM_CPPFLAGS = $(GTK_CFLAGS) $(SIGC_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/gui \
	-I$(top_srcdir)/lib/engine/include \
	-I$(top_srcdir)/lib/engine/include/framework \
	-I$(top_srcdir)/lib/engine/include/addressbook

libekiga_gtk_searchwindow_la_SOURCES = \
	$(gtk_searchwindow_dir)/search-window.h \
	$(gtk_searchwindow_dir)/search-window.cpp \
	$(gtk_searchwindow_dir)/book-view-gtk.h \
	$(gtk_searchwindow_dir)/book-view-gtk.cpp \
	$(gtk_searchwindow_dir)/menu-builder-gtk.h \
	$(gtk_searchwindow_dir)/menu-builder-gtk.cpp 

libekiga_gtk_searchwindow_la_LIBADD = \
	$(top_builddir)/lib/engine/src/addressbook/libgmaddressbook.la

libekiga_gtk_searchwindow_la_LDFLAGS = -export-dynamic -no-undefined $(GTK_LIBS) $(GLIB_LIBS) $(SIGC_LIBS)
