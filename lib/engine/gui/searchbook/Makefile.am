noinst_LTLIBRARIES = libekiga_gtk_searchbook.la

gtk_searchbook_dir = $(top_srcdir)/lib/engine/gui/searchbook

AM_CPPFLAGS = $(GTK_CFLAGS) $(SIGC_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/engine/include \
	-I$(top_srcdir)/lib/engine/include/framework \
	-I$(top_srcdir)/lib/engine/include/addressbook

libekiga_gtk_searchbook_la_SOURCES = \
	$(gtk_searchbook_dir)/addressbook-window.h \
	$(gtk_searchbook_dir)/addressbook-window.cpp \
	$(gtk_searchbook_dir)/book-view-gtk.h \
	$(gtk_searchbook_dir)/book-view-gtk.cpp \
	$(gtk_searchbook_dir)/menu-builder-gtk.h \
	$(gtk_searchbook_dir)/menu-builder-gtk.cpp 

libekiga_gtk_searchbook_la_LIBADD = \
				$(top_srcdir)/lib/engine/src/addressbook/libgmaddressbook.la 

libekiga_gtk_searchbook_la_LDFLAGS = -export-dynamic -no-undefined $(GTK_LIBS) $(GLIB_LIBS) $(SIGC_LIBS)
