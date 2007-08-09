noinst_LTLIBRARIES = libgmguicommon.la

gtk_common_dir = $(top_srcdir)/lib/engine/gui/common

AM_CPPFLAGS = $(GTK_CFLAGS) $(SIGC_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/gui 			\
	-I$(top_srcdir)/lib/engine/include/framework	\
	-I$(top_srcdir)/lib/engine/include/addressbook

libgmguicommon_la_SOURCES = \
	$(gtk_common_dir)/menu-builder-gtk.h 		\
	$(gtk_common_dir)/menu-builder-gtk.cpp 		\
	$(gtk_common_dir)/form-dialog-gtk.cpp 		\
	$(gtk_common_dir)/form-dialog-gtk.h

libgmguicommon_la_LDFLAGS = -export-dynamic -no-undefined $(GTK_LIBS) $(GLIB_LIBS) $(SIGC_LIBS)
