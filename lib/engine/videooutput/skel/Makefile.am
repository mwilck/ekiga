noinst_LTLIBRARIES = libgmvideooutput.la

videooutput_dir = $(top_srcdir)/lib/engine/videooutput/skel

AM_CXXFLAGS = $(SIGC_CFLAGS) $(GLIB_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/gmconf \
	-I$(top_srcdir)/lib/engine/include \
	-I$(top_srcdir)/lib/engine/framework \
	-I$(top_srcdir)/lib/engine/videooutput/skel

libgmvideooutput_la_SOURCES = \
	$(videooutput_dir)/display-manager.h	\
	$(videooutput_dir)/display-core.h		\
	$(videooutput_dir)/display-core.cpp         \
	$(videooutput_dir)/display-gmconf-bridge.h  \
	$(videooutput_dir)/display-gmconf-bridge.cpp

libgmvideooutput_la_LDFLAGS = -export-dynamic -no-undefined $(SIGC_LIBS) $(GLIB_LIBS)
