noinst_LTLIBRARIES = libgmlocalroster.la

localroster_dir = $(top_srcdir)/lib/engine/plugins/presence/localroster

AM_CPPFLAGS = $(XML_CFLAGS) $(SIGC_CFLAGS) $(GLIB_CFLAGS) $(GTK_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/gmconf	 		\
	-I$(top_srcdir)/lib/engine/include 		\
	-I$(top_srcdir)/lib/engine/framework 		\
	-I$(top_srcdir)/lib/engine/addressbook/skel 	\
	-I$(top_srcdir)/lib/engine/presence/skel

libgmlocalroster_la_SOURCES = \
	$(localroster_dir)/gmconf-presentity.h \
	$(localroster_dir)/gmconf-presentity.cpp \
	$(localroster_dir)/gmconf-heap.h \
	$(localroster_dir)/gmconf-heap.cpp \
	$(localroster_dir)/gmconf-cluster.h \
	$(localroster_dir)/gmconf-cluster.cpp \
	$(localroster_dir)/gmconf-roster-main.h \
	$(localroster_dir)/gmconf-roster-main.cpp \
	$(localroster_dir)/gmconf-roster-bridge.h \
	$(localroster_dir)/gmconf-roster-bridge.cpp

libgmlocalroster_la_LIBADD = \
	$(top_builddir)/lib/engine/addressbook/skel/libgmaddressbook.la \
	$(top_srcdir)/lib/engine/presence/skel/libgmpresence.la 

libgmlocalroster_la_LDFLAGS = -export-dynamic -no-undefined $(XML_LIBS) $(SIGC_LIBS) $(GLIB_LIBS) $(GTK_LIBS)
