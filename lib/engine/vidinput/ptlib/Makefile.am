noinst_LTLIBRARIES = libgmvidinput-ptlib.la

vidinput_dir = $(top_srcdir)/lib/engine/vidinput/ptlib

AM_CXXFLAGS = $(SIGC_CFLAGS) $(GLIB_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/gmconf \
	-I$(top_srcdir)/lib/engine/include \
	-I$(top_srcdir)/lib/engine/framework \
	-I$(top_srcdir)/lib/engine/display/skel \
	-I$(top_srcdir)/lib/engine/vidinput/skel \
	-I$(top_srcdir)/lib/engine/vidinput/ptlib \
	-I$(top_srcdir)/lib/engine/hal/skel

libgmvidinput_ptlib_la_SOURCES = \
	$(vidinput_dir)/vidinput-manager-ptlib.h \
	$(vidinput_dir)/vidinput-manager-ptlib.cpp \
	$(vidinput_dir)/vidinput-main-ptlib.h \
	$(vidinput_dir)/vidinput-main-ptlib.cpp 

libgmvidinput_ptlib_la_LDFLAGS = -export-dynamic -no-undefined $(SIGC_LIBS) $(GLIB_LIBS)
