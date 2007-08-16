SUBDIRS = common searchwindow roster

noinst_LTLIBRARIES = libgmgui.la

gui_dir = $(top_srcdir)/lib/engine/gui

AM_CPPFLAGS = $(GTK_CFLAGS) $(SIGC_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/engine \
	-I$(top_srcdir)/lib/gui \
	-I$(top_srcdir)/lib/engine/gui \
	-I$(top_srcdir)/lib/engine/gui/common \
	-I$(top_srcdir)/lib/engine/include \
	-I$(top_srcdir)/lib/engine/include/framework \
	-I$(top_srcdir)/lib/engine/include/addressbook \
	-I$(top_srcdir)/lib/engine/include/presence

libgmgui_la_SOURCES = \
		      $(gui_dir)/gtk-core.h \
		      $(gui_dir)/gtk-core.cpp \
		      $(gui_dir)/gtk-core-main.h \
		      $(gui_dir)/gtk-core-main.cpp

libgmgui_la_LIBADD = \
	$(top_builddir)/lib/gui/libgmwidgets.la						\
	$(top_builddir)/lib/engine/gui/common/libgmguicommon.la 			\
	$(top_builddir)/lib/engine/gui/searchwindow/libgmguisearchwindow.la 		\
	$(top_builddir)/lib/engine/gui/roster/libgmguiroster.la				\
	$(top_builddir)/lib/engine/src/framework/libgmframework.la

libgmgui_la_LDFLAGS = -export-dynamic -no-undefined $(GTK_LIBS) $(GLIB_LIBS) $(SIGC_LIBS)
