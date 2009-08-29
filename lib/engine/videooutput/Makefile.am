stacklib_LTLIBRARIES = libgmvideooutput.la

videooutput_dir = $(top_srcdir)/lib/engine/videooutput

AM_CXXFLAGS = $(BOOST_CPPFLAGS) $(SIGC_CFLAGS) $(GLIB_CFLAGS) $(PTLIB_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/gmconf \
	-I$(top_srcdir)/lib/engine/framework \
	-I$(top_srcdir)/lib/engine/videooutput

libgmvideooutput_la_SOURCES = \
	$(videooutput_dir)/videooutput-info.h		\
	$(videooutput_dir)/videooutput-manager.h	\
	$(videooutput_dir)/videooutput-core.h		\
	$(videooutput_dir)/videooutput-core.cpp         \
	$(videooutput_dir)/videooutput-gmconf-bridge.h  \
	$(videooutput_dir)/videooutput-gmconf-bridge.cpp

libgmvideooutput_la_LDFLAGS = $(STACKLIB_LDFLAGS)
libgmvideooutput_la_LIBADD = \
	$(top_builddir)/lib/engine/framework/libgmframework.la \
	$(BOOST_LDFLAGS) $(SIGC_LIBS) $(GLIB_LIBS) $(PTLIB_LIBS)
