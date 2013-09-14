plugin_LTLIBRARIES = libgmxcap.la

xcap_dir = $(top_srcdir)/plugins/xcap

AM_CXXFLAGS = \
	$(BOOST_CPPFLAGS) $(SOUP_CFLAGS) \
	-I$(top_srcdir)/lib/engine/framework

libgmxcap_la_SOURCES = \
	$(xcap_dir)/xcap-path.h \
	$(xcap_dir)/xcap-path.cpp \
	$(xcap_dir)/xcap-core.h \
	$(xcap_dir)/xcap-core.cpp \
	$(xcap_dir)/xcap-main.h \
	$(xcap_dir)/xcap-main.cpp

libgmxcap_la_LDFLAGS = $(PLUGINS_LIBTOOL_FLAGS)
libgmxcap_la_LIBADD = \
	$(top_builddir)/lib/libekiga.la \
	$(BOOST_LDFLAGS) $(SOUP_LIBS)
