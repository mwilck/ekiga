SUBDIRS = searchwindow

noinst_LTLIBRARIES = libgmgui.la

gui_dir = $(top_srcdir)/lib/engine/gui

AM_CPPFLAGS = $(GTK_CFLAGS) $(SIGC_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/engine/include \
	-I$(top_srcdir)/lib/engine/include/framework \
	-I$(top_srcdir)/lib/engine/include/addressbook

libgmgui_la_SOURCES = \
		      $(gui_dir)/menu-builder-gtk.h \
		      $(gui_dir)/menu-builder-gtk.cpp \
		      $(gui_dir)/form-dialog.h \
		      $(gui_dir)/form-dialog.cpp \
		      $(gui_dir)/gtk-core.h \
		      $(gui_dir)/gtk-core.cpp \
		      $(gui_dir)/gtk-core-main.h \
		      $(gui_dir)/gtk-core-main.cpp

libgmgui_la_LIBADD = \
	$(top_builddir)/lib/engine/gui/searchwindow/libekiga_gtk_searchwindow.la \
	$(top_builddir)/lib/engine/src/framework/libgmframework.la

libgmgui_la_LDFLAGS = -export-dynamic -no-undefined $(GTK_LIBS) $(GLIB_LIBS) $(SIGC_LIBS)
