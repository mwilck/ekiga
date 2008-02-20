noinst_LTLIBRARIES = libgmvidinput.la

vidinput_dir = $(top_srcdir)/lib/engine/vidinput/skel

AM_CXXFLAGS = $(SIGC_CFLAGS) $(GLIB_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/gmconf \
	-I$(top_srcdir)/lib/engine/include \
	-I$(top_srcdir)/lib/engine/framework \
	-I$(top_srcdir)/lib/engine/display/skel \
	-I$(top_srcdir)/lib/engine/vidinput/skel

libgmvidinput_la_SOURCES = \
	$(vidinput_dir)/vidinput-manager.h	\
	$(vidinput_dir)/vidinput-info.h		\
	$(vidinput_dir)/vidinput-core.h		\
	$(vidinput_dir)/vidinput-core.cpp       \
	$(vidinput_dir)/vidinput-gmconf-bridge.h \
	$(vidinput_dir)/vidinput-gmconf-bridge.cpp

libgmvidinput_la_LDFLAGS = -export-dynamic -no-undefined $(SIGC_LIBS) $(GLIB_LIBS)
