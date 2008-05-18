noinst_LTLIBRARIES = libgmvideoinput.la

videoinput_dir = $(top_srcdir)/lib/engine/videoinput/skel

AM_CXXFLAGS = $(SIGC_CFLAGS) $(GLIB_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/gmconf \
	-I$(top_srcdir)/lib/engine/include \
	-I$(top_srcdir)/lib/engine/framework \
	-I$(top_srcdir)/lib/engine/videooutput/skel \
	-I$(top_srcdir)/lib/engine/videoinput/skel \
	-I$(top_srcdir)/lib/engine/hal/skel

libgmvideoinput_la_SOURCES = \
	$(videoinput_dir)/vidinput-manager.h	\
	$(videoinput_dir)/vidinput-info.h		\
	$(videoinput_dir)/vidinput-core.h		\
	$(videoinput_dir)/vidinput-core.cpp       \
	$(videoinput_dir)/vidinput-gmconf-bridge.h \
	$(videoinput_dir)/vidinput-gmconf-bridge.cpp

libgmvideoinput_la_LDFLAGS = -export-dynamic -no-undefined $(SIGC_LIBS) $(GLIB_LIBS)
