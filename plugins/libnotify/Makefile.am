plugin_LTLIBRARIES = libgmlibnotify.la

libnotify_dir = $(top_srcdir)/plugins/libnotify

AM_CXXFLAGS = $(BOOST_CPPFLAGS) $(NOTIFY_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/engine/framework \
	-I$(top_srcdir)/lib/engine/notification \
	-I$(top_srcdir)/lib/engine/protocol

libgmlibnotify_la_SOURCES = \
	$(libnotify_dir)/libnotify-main.cpp

libgmlibnotify_la_LDFLAGS = $(PLUGINS_LIBTOOL_FLAGS)
libgmlibnotify_la_LIBADD = \
	$(top_builddir)/lib/libekiga.la \
	$(BOOST_LDFLAGS) $(BOOST_LIBS) $(NOTIFY_LIBS)
