plugin_LTLIBRARIES = libgmavahi.la

avahi_dir = $(top_srcdir)/plugins/avahi

AM_CXXFLAGS = \
	$(BOOST_CPPFLAGS) $(AVAHI_CFLAGS) \
	-I$(top_srcdir)/lib/engine/framework \
	-I$(top_srcdir)/lib/engine/account \
	-I$(top_srcdir)/lib/engine/presence \
	-I$(top_srcdir)/lib/engine/protocol

libgmavahi_la_SOURCES = \
	$(avahi_dir)/avahi-main.cpp \
	$(avahi_dir)/avahi-publisher.cpp \
	$(avahi_dir)/avahi-publisher.h \
	$(avahi_dir)/avahi-heap.h \
	$(avahi_dir)/avahi-heap.cpp \
	$(avahi_dir)/avahi-cluster.h \
	$(avahi_dir)/avahi-cluster.cpp

libgmavahi_la_LDFLAGS = $(PLUGINS_LIBTOOL_FLAGS)
libgmavahi_la_LIBADD = \
	$(top_builddir)/lib/libekiga.la \
	$(BOOST_LDFLAGS) $(AVAHI_LIBS)
