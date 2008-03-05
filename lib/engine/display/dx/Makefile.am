noinst_LTLIBRARIES = libgmdisplay-dx.la

display_dir = $(top_srcdir)/lib/engine/display/dx
common_dir = $(top_srcdir)/lib/engine/display/common

AM_CXXFLAGS = $(SIGC_CFLAGS) $(PTLIB_CFLAGS) $(DX_CFLAGS) $(GLIB_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/gmconf \
	-I$(top_srcdir)/lib/engine/include \
	-I$(top_srcdir)/lib/engine/framework \
	-I$(top_srcdir)/lib/engine/display/skel \
	-I$(top_srcdir)/lib/engine/display/common \
	-I$(top_srcdir)/lib/gui

libgmdisplay_dx_la_SOURCES = \
	$(common_dir)/display-manager-common.cpp\
	$(common_dir)/display-manager-common.h  \
	$(display_dir)/display-manager-dx.cpp    \
	$(display_dir)/display-manager-dx.h      \
	$(display_dir)/display-main-dx.cpp    \
	$(display_dir)/display-main-dx.h

libgmdisplay_dx_la_LDFLAGS = -export-dynamic -no-undefined $(SIGC_LIBS) $(PTLIB_LIBS) $(DX_LIBS) $(GLIB_LIBS)
