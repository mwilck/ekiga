noinst_LTLIBRARIES = libgmgstreamer.la

gstreamer_dir = $(top_srcdir)/lib/engine/components/gstreamer/

AM_CXXFLAGS = $(SIGC_CFLAGS) $(GSTREAMER_CFLAGS) $(PTLIB_CFLAGS)

INCLUDES = 						\
	-I$(top_srcdir)/lib/gmconf			\
	-I$(top_srcdir)/lib/engine/framework		\
	-I$(top_srcdir)/lib/engine/videoinput/skel	\
	-I$(top_srcdir)/lib/engine/videooutput/skel	\
	-I$(top_srcdir)/lib/engine/audioinput/skel	\
	-I$(top_srcdir)/lib/engine/audiooutput/skel	\
	-I$(top_srcdir)/lib/engine/hal/skel

libgmgstreamer_la_SOURCES = \
	$(gstreamer_dir)/gst-main.h \
	$(gstreamer_dir)/gst-main.cpp \
	$(gstreamer_dir)/gst-videoinput.h \
	$(gstreamer_dir)/gst-videoinput.cpp \
	$(gstreamer_dir)/gst-audioinput.h \
	$(gstreamer_dir)/gst-audioinput.cpp

libgmgstreamer_la_LDFLAGS = -export-dynamic -no-undefined $(SIGC_LIBS) $(GSTREAMER_LIBS) $(PTLIB_LIBS)
