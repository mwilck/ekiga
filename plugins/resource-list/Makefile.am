plugin_LTLIBRARIES = libgmresource_list.la

AM_CXXFLAGS = \
	$(BOOST_CPPFLAGS) $(GLIB_CFLAGS) $(XML_CFLAGS) \
	-I$(top_srcdir)/lib/				\
	-I$(top_srcdir)/lib/engine/framework		\
	-I$(top_srcdir)/lib/engine/presence		\
	-I$(top_srcdir)/lib/engine/account		\
	-I$(top_srcdir)/plugins/xcap

libgmresource_list_la_SOURCES = \
	resource-list-main.h \
	resource-list-main.cpp \
	rl-presentity.h \
	rl-presentity.cpp \
	rl-entry.h \
	rl-entry.cpp \
	rl-entry-ref.h \
	rl-entry-ref.cpp \
	rl-list.h \
	rl-list.cpp \
	rl-heap.h \
	rl-heap.cpp \
	rl-cluster.h \
	rl-cluster.cpp

libgmresource_list_la_LDFLAGS = \
	-L$(top_builddir)/plugins/xcap \
	$(PLUGINS_LIBTOOL_FLAGS)
libgmresource_list_la_LIBADD = \
	$(top_builddir)/lib/libekiga.la \
	$(top_builddir)/plugins/xcap/libgmxcap.la \
	$(BOOST_LDFLAGS) $(GLIB_LIBS) $(XML_LIBS)
