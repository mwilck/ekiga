plugin_LTLIBRARIES = libgmgstreamer.la

AM_CXXFLAGS = \
	$(BOOST_CPPFLAGS) $(GSTREAMER_CFLAGS) $(PTLIB_CFLAGS) \
	-I$(top_srcdir)/lib \
	-I$(top_srcdir)/lib/settings \
	-I$(top_srcdir)/lib/engine/framework \
	-I$(top_srcdir)/lib/engine/notification	\
	-I$(top_srcdir)/lib/engine/videoinput \
	-I$(top_srcdir)/lib/engine/videooutput \
	-I$(top_srcdir)/lib/engine/audioinput \
	-I$(top_srcdir)/lib/engine/audiooutput \
	-I$(top_srcdir)/lib/engine/hal

libgmgstreamer_la_SOURCES = \
	gst-helper.h \
	gst-helper.cpp \
	gst-main.h \
	gst-main.cpp \
	gst-videoinput.h \
	gst-videoinput.cpp \
	gst-audioinput.h \
	gst-audioinput.cpp \
	gst-audiooutput.h \
	gst-audiooutput.cpp

libgmgstreamer_la_LDFLAGS = $(PLUGINS_LIBTOOL_FLAGS)
libgmgstreamer_la_LIBADD = \
	$(BOOST_LDFLAGS) $(GSTREAMER_LIBS) $(PTLIB_LIBS)
