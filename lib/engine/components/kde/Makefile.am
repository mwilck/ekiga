plugindir = $(PLUGINDIR)/kde
plugin_LTLIBRARIES = libgmkde.la

kde_dir = $(top_srcdir)/lib/engine/components/kde

AM_CXXFLAGS = $(SIGC_CFLAGS) $(KDE_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/engine/framework

libgmkde_la_SOURCES = \
	$(kde_dir)/kde-main.h \
	$(kde_dir)/kde-main.cpp

libgmkde_la_LDFLAGS = $(PLUGINS_LIBTOOL_FLAGS)
libgmkde_la_LIBADD = \
	$(top_builddir)/lib/engine/framework/libgmframework.la \
	$(SIGC_LIBS) $(KDE_LIBS)