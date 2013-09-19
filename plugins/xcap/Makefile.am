plugin_LTLIBRARIES = libgmxcap.la

AM_CXXFLAGS = \
	$(BOOST_CPPFLAGS) $(SOUP_CFLAGS) \
	-I$(top_srcdir)/lib/engine/framework

libgmxcap_la_SOURCES = \
	xcap-path.h \
	xcap-path.cpp \
	xcap-core.h \
	xcap-core.cpp \
	xcap-main.h \
	xcap-main.cpp

libgmxcap_la_LDFLAGS = $(PLUGINS_LIBTOOL_FLAGS)
libgmxcap_la_LIBADD = \
	$(top_builddir)/lib/libekiga.la \
	$(BOOST_LDFLAGS) $(SOUP_LIBS)
