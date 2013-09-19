plugin_LTLIBRARIES = libgmkab.la

AM_CPPFLAGS = \
	$(BOOST_CPPFLAGS) $(KAB_CFLAGS) \
	-I$(top_srcdir)/lib/engine/framework \
	-I$(top_srcdir)/lib/engine/addressbook

libgmkab_la_SOURCES = \
	kab-contact.h \
	kab-contact.cpp \
	kab-book.h \
	kab-book.cpp \
	kab-source.h \
	kab-source.cpp \
	kab-main.h \
	kab-main.cpp

libgmkab_la_LDFLAGS = $(PLUGINS_LIBTOOL_FLAGS)
libgmkab_la_LIBADD = \
	$(KAB_LIBS)
