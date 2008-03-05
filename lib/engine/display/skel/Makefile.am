noinst_LTLIBRARIES = libgmdisplay.la

display_dir = $(top_srcdir)/lib/engine/display/skel

AM_CXXFLAGS = $(SIGC_CFLAGS) $(GLIB_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/gmconf \
	-I$(top_srcdir)/lib/engine/include \
	-I$(top_srcdir)/lib/engine/framework \
	-I$(top_srcdir)/lib/engine/display/skel

libgmdisplay_la_SOURCES = \
	$(display_dir)/display-manager.h	\
	$(display_dir)/display-core.h		\
	$(display_dir)/display-core.cpp         \
	$(display_dir)/display-gmconf-bridge.h  \
	$(display_dir)/display-gmconf-bridge.cpp

libgmdisplay_la_LDFLAGS = -export-dynamic -no-undefined $(SIGC_LIBS) $(GLIB_LIBS)
