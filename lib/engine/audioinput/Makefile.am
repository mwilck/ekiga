stacklib_LTLIBRARIES = libgmaudioinput.la

audioinput_dir = $(top_srcdir)/lib/engine/audioinput

AM_CXXFLAGS = $(BOOST_CPPFLAGS) $(SIGC_CFLAGS) $(GLIB_CFLAGS) $(PTLIB_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/gmconf \
	-I$(top_srcdir)/lib/engine/framework \
	-I$(top_srcdir)/lib/engine/audiooutput \
	-I$(top_srcdir)/lib/engine/audioinput \
	-I$(top_srcdir)/lib/engine/hal

libgmaudioinput_la_SOURCES = \
	$(audioinput_dir)/audioinput-manager.h	\
	$(audioinput_dir)/audioinput-info.h	\
	$(audioinput_dir)/audioinput-core.h	\
	$(audioinput_dir)/audioinput-core.cpp       \
	$(audioinput_dir)/audioinput-gmconf-bridge.h \
	$(audioinput_dir)/audioinput-gmconf-bridge.cpp

libgmaudioinput_la_LDFLAGS = $(STACKLIB_LDFLAGS)
libgmaudioinput_la_LIBADD = \
	$(top_builddir)/lib/engine/framework/libgmframework.la \
	$(BOOST_LDFLAGS) $(SIGC_LIBS) $(GLIB_LIBS) $(PTLIB_LIBS)
