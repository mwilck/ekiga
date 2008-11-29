noinst_LTLIBRARIES = libgmloudmouth.la

loudmouth_dir = $(top_srcdir)/lib/engine/components/loudmouth

AM_CXXFLAGS = $(SIGC_CFLAGS) $(LOUDMOUTH_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/engine/framework

libgmloudmouth_la_SOURCES = \
	$(loudmouth_dir)/loudmouth-main.h \
	$(loudmouth_dir)/loudmouth-main.cpp

libgmloudmouth_la_LDFLAGS = -export-dynamic -no-undefined $(SIGC_LIBS) $(LOUDMOUTH_LIBS)