noinst_LTLIBRARIES = libgmgstreamer.la

gstreamer_dir = $(top_srcdir)/lib/engine/components/gstreamer/

AM_CXXFLAGS = $(SIGC_CFLAGS) $(GSTREAMER_CFLAGS)

INCLUDES = 						\
	-I$(top_srcdir)/lib/engine/framework

libgmgstreamer_la_SOURCES = \
	$(gstreamer_dir)/gst-main.h \
	$(gstreamer_dir)/gst-main.cpp

libgmgstreamer_la_LDFLAGS = -export-dynamic -no-undefined $(SIGC_LIBS) $(GSTREAMER_LIBS)
