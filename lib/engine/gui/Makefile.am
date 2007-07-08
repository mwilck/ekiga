SUBDIRS = searchbook

noinst_LTLIBRARIES = libgmgui.la

gui_dir = $(top_srcdir)/lib/engine/gui

AM_CPPFLAGS = $(GTK_CFLAGS) $(SIGC_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/engine/include \
	-I$(top_srcdir)/lib/engine/include/framework \
	-I$(top_srcdir)/lib/engine/include/addressbook

libgmgui_la_SOURCES = \
	$(gui_dir)/gui-gtk.h \
	$(gui_dir)/gui-gtk.cpp \
	$(gui_dir)/gui-gtk-main.h \
	$(gui_dir)/gui-gtk-main.cpp

libgmgui_la_LIBADD = \
		     $(top_srcdir)/lib/engine/gui/searchbook/libekiga_gtk_searchbook.la

libgmgui_la_LDFLAGS = -export-dynamic -no-undefined $(GTK_LIBS) $(GLIB_LIBS) $(SIGC_LIBS)
