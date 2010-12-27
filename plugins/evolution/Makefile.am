plugin_LTLIBRARIES = libgmevolution.la

evolution_dir = $(top_srcdir)/plugins/evolution

AM_CXXFLAGS = $(BOOST_CPPFLAGS) $(EDS_CFLAGS) $(GLIB_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/engine/framework \
	-I$(top_srcdir)/lib/engine/addressbook

libgmevolution_la_SOURCES = \
	$(evolution_dir)/evolution-contact.h \
	$(evolution_dir)/evolution-contact.cpp \
	$(evolution_dir)/evolution-book.h \
	$(evolution_dir)/evolution-book.cpp \
	$(evolution_dir)/evolution-source.h \
	$(evolution_dir)/evolution-source.cpp \
	$(evolution_dir)/evolution-main.h \
	$(evolution_dir)/evolution-main.cpp

libgmevolution_la_LDFLAGS = $(PLUGINS_LIBTOOL_FLAGS)
libgmevolution_la_LIBADD =  \
	$(top_builddir)/lib/libekiga.la \
	$(BOOST_LDFLAGS) $(BOOST_LIBS) $(EDS_LIBS) $(GLIB_LIBS)
