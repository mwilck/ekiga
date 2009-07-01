plugin_LTLIBRARIES = libgmxcap.la

xcap_dir = $(top_srcdir)/lib/engine/components/xcap/

AM_CXXFLAGS = $(SIGC_CFLAGS) $(SOUP_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/engine/framework

libgmxcap_la_SOURCES = \
	$(xcap_dir)/xcap-path.h \
	$(xcap_dir)/xcap-path.cpp \
	$(xcap_dir)/xcap-core.h \
	$(xcap_dir)/xcap-core.cpp \
	$(xcap_dir)/xcap-main.h \
	$(xcap_dir)/xcap-main.cpp

libgmxcap_la_LDFLAGS = $(PLUGINS_LIBTOOL_FLAGS)
libgmxcap_la_LIBADD = $(SIGC_LIBS) $(SOUP_LIBS)