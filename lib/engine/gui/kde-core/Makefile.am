if WIN32
lib_LTLIBRARIES = libgmkde-core.la
else
noinst_LTLIBRARIES = libgmkde-core.la
endif

kde_core_dir = $(top_srcdir)/gui/kde-core

AM_CPPFLAGS = $(KAB_CFLAGS) $(GTK_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/framework

libgmkde_core_la_SOURCES = \
	$(kde_core_dir)/kde-main.h \
	$(kde_core_dir)/kde-main.cpp

libgmkde_core_la_LDFLAGS = -export-dynamic -no-undefined $(KAB_LIBS) $(GTK_LIBS)