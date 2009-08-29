stacklib_LTLIBRARIES = libplugin.la

plugin_dir = $(top_srcdir)/lib/engine/plugin

AM_CPPFLAGS = \
	-DEKIGA_PLUGIN_DIR=\"$(PLUGIN_DIR)\" \
	$(BOOST_CPPFLAGS) $(SIGC_CFLAGS) $(GLIB_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/gmconf	 		\
	-I$(top_srcdir)/lib/engine/framework

libplugin_la_SOURCES = \
	$(plugin_dir)/plugin-core.h			\
	$(plugin_dir)/plugin-core.cpp

libplugin_la_LDFLAGS = $(STACKLIB_LDFLAGS)
libplugin_la_LIBADD = $(BOOST_LDFLAGS) $(BOOST_LIBS) $(GLIB_LIBS)