plugin_LTLIBRARIES = libgmkde.la

AM_CXXFLAGS = \
	$(BOOST_CPPFLAGS) $(KDE_CFLAGS) \
	-I$(top_srcdir)/lib/engine/framework

libgmkde_la_SOURCES = \
	kde-main.h \
	kde-main.cpp

libgmkde_la_LDFLAGS = $(PLUGINS_LIBTOOL_FLAGS)
libgmkde_la_LIBADD = \
	$(BOOST_LDFLAGS) $(KDE_LIBS)
