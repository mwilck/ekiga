plugin_LTLIBRARIES = libgmavahi.la

AM_CXXFLAGS = \
	$(BOOST_CPPFLAGS) $(AVAHI_CFLAGS) \
	-I$(top_srcdir)/lib/engine \
	-I$(top_srcdir)/lib/engine/framework \
	-I$(top_srcdir)/lib/engine/account \
	-I$(top_srcdir)/lib/engine/presence \
	-I$(top_srcdir)/lib/engine/protocol

libgmavahi_la_SOURCES = \
	avahi-main.cpp \
	avahi-publisher.cpp \
	avahi-publisher.h \
	avahi-heap.h \
	avahi-heap.cpp \
	avahi-cluster.h \
	avahi-cluster.cpp

libgmavahi_la_LDFLAGS = $(PLUGINS_LIBTOOL_FLAGS)
libgmavahi_la_LIBADD = \
	$(top_builddir)/lib/libekiga.la \
	$(BOOST_LDFLAGS) $(AVAHI_LIBS)
