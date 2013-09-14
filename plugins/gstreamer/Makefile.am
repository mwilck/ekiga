plugin_LTLIBRARIES = libgmgstreamer.la

gstreamer_dir = $(top_srcdir)/plugins/gstreamer/

AM_CXXFLAGS = \
	$(BOOST_CPPFLAGS) $(GSTREAMER_CFLAGS) $(PTLIB_CFLAGS) \
	-I$(top_srcdir)/lib/gmconf \
	-I$(top_srcdir)/lib/engine/framework \
	-I$(top_srcdir)/lib/engine/notification	\
	-I$(top_srcdir)/lib/engine/videoinput \
	-I$(top_srcdir)/lib/engine/videooutput \
	-I$(top_srcdir)/lib/engine/audioinput \
	-I$(top_srcdir)/lib/engine/audiooutput \
	-I$(top_srcdir)/lib/engine/hal

libgmgstreamer_la_SOURCES = \
	$(gstreamer_dir)/gst-helper.h \
	$(gstreamer_dir)/gst-helper.cpp \
	$(gstreamer_dir)/gst-main.h \
	$(gstreamer_dir)/gst-main.cpp \
	$(gstreamer_dir)/gst-videoinput.h \
	$(gstreamer_dir)/gst-videoinput.cpp \
	$(gstreamer_dir)/gst-audioinput.h \
	$(gstreamer_dir)/gst-audioinput.cpp \
	$(gstreamer_dir)/gst-audiooutput.h \
	$(gstreamer_dir)/gst-audiooutput.cpp

libgmgstreamer_la_LDFLAGS = $(PLUGINS_LIBTOOL_FLAGS)
libgmgstreamer_la_LIBADD = \
	$(BOOST_LDFLAGS) $(GSTREAMER_LIBS) $(PTLIB_LIBS)
