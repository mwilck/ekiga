plugin_LTLIBRARIES = libgmkab.la

kab_dir = $(top_srcdir)/plugins/kab

AM_CPPFLAGS = \
	$(BOOST_CPPFLAGS) $(KAB_CFLAGS) \
	-I$(top_srcdir)/lib/engine/framework \
	-I$(top_srcdir)/lib/engine/addressbook

libgmkab_la_SOURCES = \
	$(kab_dir)/kab-contact.h \
	$(kab_dir)/kab-contact.cpp \
	$(kab_dir)/kab-book.h \
	$(kab_dir)/kab-book.cpp \
	$(kab_dir)/kab-source.h \
	$(kab_dir)/kab-source.cpp \
	$(kab_dir)/kab-main.h \
	$(kab_dir)/kab-main.cpp

libgmkab_la_LDFLAGS = $(PLUGINS_LIBTOOL_FLAGS)
libgmkab_la_LIBADD = \
	$(KAB_LIBS)
