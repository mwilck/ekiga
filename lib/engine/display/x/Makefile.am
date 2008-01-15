noinst_LTLIBRARIES = libgmdisplay-x.la

display_dir = $(top_srcdir)/lib/engine/display/x
common_dir = $(top_srcdir)/lib/engine/display/common

AM_CXXFLAGS = $(SIGC_CFLAGS) $(PTLIB_CFLAGS) $(GLIB_CFLAGS) $(XV_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/engine/include \
	-I$(top_srcdir)/lib/engine/framework \
	-I$(top_srcdir)/lib/engine/display/skel \
	-I$(top_srcdir)/lib/engine/display/common

libgmdisplay_x_la_SOURCES = \
	$(common_dir)/display-manager-common.cpp\
	$(common_dir)/display-manager-common.h  \
	$(display_dir)/display-manager-x.cpp    \
	$(display_dir)/display-manager-x.h      \
	$(display_dir)/display-main-x.cpp    \
	$(display_dir)/display-main-x.h

libgmdisplay_x_la_LDFLAGS = -export-dynamic -no-undefined $(SIGC_LIBS) $(PTLIB_LIBS) $(GLIB_LIBS) $(XV_LIBS)
