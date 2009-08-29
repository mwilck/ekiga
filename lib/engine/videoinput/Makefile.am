stacklib_LTLIBRARIES = libgmvideoinput.la

videoinput_dir = $(top_srcdir)/lib/engine/videoinput

AM_CXXFLAGS = $(BOOST_CPPFLAGS) $(SIGC_CFLAGS) $(GLIB_CFLAGS) $(PTLIB_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/gmconf \
	-I$(top_srcdir)/lib/engine/framework \
	-I$(top_srcdir)/lib/engine/videooutput \
	-I$(top_srcdir)/lib/engine/videoinput \
	-I$(top_srcdir)/lib/engine/hal

libgmvideoinput_la_SOURCES = \
	$(videoinput_dir)/videoinput-manager.h	\
	$(videoinput_dir)/videoinput-info.h		\
	$(videoinput_dir)/videoinput-core.h		\
	$(videoinput_dir)/videoinput-core.cpp       \
	$(videoinput_dir)/videoinput-gmconf-bridge.h \
	$(videoinput_dir)/videoinput-gmconf-bridge.cpp

libgmvideoinput_la_LDFLAGS = $(STACKLIB_LDFLAGS)
libgmvideoinput_la_LIBADD = \
	$(top_builddir)/lib/engine/framework/libgmframework.la \
	$(top_builddir)/lib/engine/videooutput/libgmvideooutput.la \
	$(BOOST_LDFLAGS) $(BOOST_LIBS) $(GLIB_LIBS) $(PTLIB_LIBS)
