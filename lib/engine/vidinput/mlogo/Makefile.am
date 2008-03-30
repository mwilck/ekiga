noinst_LTLIBRARIES = libgmvidinput-mlogo.la

vidinput_dir = $(top_srcdir)/lib/engine/vidinput/mlogo

AM_CXXFLAGS = $(SIGC_CFLAGS) $(GLIB_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/gmconf \
	-I$(top_srcdir)/pixmaps \
	-I$(top_srcdir)/lib/engine/include \
	-I$(top_srcdir)/lib/engine/framework \
	-I$(top_srcdir)/lib/engine/display/skel \
	-I$(top_srcdir)/lib/engine/vidinput/skel \
	-I$(top_srcdir)/lib/engine/vidinput/mlogo \
	-I$(top_srcdir)/lib/engine/hal/skel

libgmvidinput_mlogo_la_SOURCES = \
	$(vidinput_dir)/vidinput-manager-mlogo.h \
	$(vidinput_dir)/vidinput-manager-mlogo.cpp \
	$(vidinput_dir)/vidinput-main-mlogo.h \
	$(vidinput_dir)/vidinput-main-mlogo.cpp 

libgmvidinput_mlogo_la_LDFLAGS = -export-dynamic -no-undefined $(SIGC_LIBS) $(GLIB_LIBS)
