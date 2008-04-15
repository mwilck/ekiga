noinst_LTLIBRARIES = libavahipublisher.la

avahi_publisher_dir = $(top_srcdir)/lib/engine/components/avahi-publisher/

AM_CXXFLAGS = $(SIGC_CFLAGS) $(GLIB_CFLAGS)

INCLUDES = 						\
	-I$(top_srcdir)/lib/gmconf	 		\
	-I$(top_srcdir)/lib/engine/include 		\
	-I$(top_srcdir)/lib/engine/framework		\
	-I$(top_srcdir)/lib/engine/presence/skel 	\
	-I$(top_srcdir)/lib/engine/protocol/skel 

libavahipublisher_la_SOURCES = 						\
	$(avahi_publisher_dir)/avahi-publisher-main.cpp			\
	$(avahi_publisher_dir)/avahi-publisher-main.h			\
	$(avahi_publisher_dir)/avahi-publisher.cpp			\
	$(avahi_publisher_dir)/avahi-publisher.h

libavahipublisher_la_LIBADD = 						\
	$(top_builddir)/lib/engine/protocol/skel/libgmprotocol.la	\
	$(top_builddir)/lib/engine/framework/libgmframework.la		\
	$(top_builddir)/lib/engine/presence/skel/libgmpresence.la

libavahipublisher_la_LDFLAGS = -export-dynamic -no-undefined $(SIGC_LIBS) $(GLIB_LIBS)
