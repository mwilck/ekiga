plugin_LTLIBRARIES = libgmevolution.la

AM_CXXFLAGS = \
	$(BOOST_CPPFLAGS) $(EDS_CFLAGS) $(GLIB_CFLAGS) \
	-I$(top_srcdir)/lib/engine/framework \
	-I$(top_srcdir)/lib/engine/addressbook

libgmevolution_la_SOURCES = \
	evolution-contact.h \
	evolution-contact.cpp \
	evolution-book.h \
	evolution-book.cpp \
	evolution-source.h \
	evolution-source.cpp \
	evolution-main.h \
	evolution-main.cpp

libgmevolution_la_LDFLAGS = $(PLUGINS_LIBTOOL_FLAGS)
libgmevolution_la_LIBADD =  \
	$(top_builddir)/lib/libekiga.la \
	$(BOOST_LDFLAGS) $(EDS_LIBS) $(GLIB_LIBS)
