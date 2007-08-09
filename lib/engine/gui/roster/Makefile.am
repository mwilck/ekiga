noinst_LTLIBRARIES = libgmconf-roster.la

gmconf_roster_dir = $(top_srcdir)/lib/engine/gui/roster

AM_CPPFLAGS = $(XML_CFLAGS) $(SIGC_CFLAGS) $(GLIB_CFLAGS) $(GTK_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/engine/include \
	-I$(top_srcdir)/lib/engine/include/framework \
	-I$(top_srcdir)/lib/gmconf \
	-I$(top_srcdir)/lib/engine/include/addressbook \
	-I$(top_srcdir)/lib/engine/include/presence

libgmconf_roster_la_SOURCES = \
	$(gmconf_roster_dir)/gmconf-presentity.h \
	$(gmconf_roster_dir)/gmconf-presentity.cpp \
	$(gmconf_roster_dir)/gmconf-heap.h \
	$(gmconf_roster_dir)/gmconf-heap.cpp \
	$(gmconf_roster_dir)/gmconf-cluster.h \
	$(gmconf_roster_dir)/gmconf-cluster.cpp \
	$(gmconf_roster_dir)/gmconf-roster-main.h \
	$(gmconf_roster_dir)/gmconf-roster-main.cpp \
	$(gmconf_roster_dir)/gmconf-roster-bridge.h \
	$(gmconf_roster_dir)/gmconf-roster-bridge.cpp

libgmconf_roster_la_LIBADD = \
	$(top_srcdir)/lib/engine/src/presence/libgmpresence.la \
	$(top_srcdir)/lib/engine/src/addressbook/libgmaddressbook.la

libgmconf_roster_la_LDFLAGS = -export-dynamic -no-undefined $(XML_LIBS) $(SIGC_LIBS) $(GLIB_LIBS) $(GTK_LIBS)
