plugin_LTLIBRARIES = libgmlibnotify.la

AM_CXXFLAGS = \
	$(BOOST_CPPFLAGS) $(NOTIFY_CFLAGS) \
	-I$(top_srcdir)/lib/engine/framework \
	-I$(top_srcdir)/lib/engine/notification \
	-I$(top_srcdir)/lib/engine/action 	\
	-I$(top_srcdir)/lib/engine/addressbook \
	-I$(top_srcdir)/lib/engine/protocol

libgmlibnotify_la_SOURCES = \
	libnotify-main.cpp

libgmlibnotify_la_LDFLAGS = $(PLUGINS_LIBTOOL_FLAGS)
libgmlibnotify_la_LIBADD = \
	$(top_builddir)/lib/libekiga.la \
	$(BOOST_LDFLAGS) $(NOTIFY_LIBS)
