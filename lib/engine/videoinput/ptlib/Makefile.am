noinst_LTLIBRARIES = libgmvideoinput-ptlib.la

videoinput_dir = $(top_srcdir)/lib/engine/videoinput/ptlib

AM_CXXFLAGS = $(SIGC_CFLAGS) $(GLIB_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/gmconf \
	-I$(top_srcdir)/lib/engine/include \
	-I$(top_srcdir)/lib/engine/framework \
	-I$(top_srcdir)/lib/engine/videooutput/skel \
	-I$(top_srcdir)/lib/engine/videoinput/skel \
	-I$(top_srcdir)/lib/engine/videoinput/ptlib \
	-I$(top_srcdir)/lib/engine/hal/skel

libgmvideoinput_ptlib_la_SOURCES = \
	$(videoinput_dir)/vidinput-manager-ptlib.h \
	$(videoinput_dir)/vidinput-manager-ptlib.cpp \
	$(videoinput_dir)/vidinput-main-ptlib.h \
	$(videoinput_dir)/vidinput-main-ptlib.cpp 

libgmvideoinput_ptlib_la_LDFLAGS = -export-dynamic -no-undefined $(SIGC_LIBS) $(GLIB_LIBS)
