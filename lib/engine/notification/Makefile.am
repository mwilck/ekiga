stacklib_LTLIBRARIES = libnotification.la

notification_dir = $(top_srcdir)/lib/engine/notification

AM_CPPFLAGS = $(BOOST_CPPFLAGS) $(SIGC_CFLAGS) $(GLIB_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/gmconf \
	-I$(top_srcdir)/lib/engine/framework

libnotification_la_SOURCES = \
	$(notification_dir)/notification-core.h \
	$(notification_dir)/notification-dummy.cpp

libnotification_la_LDFLAGS = $(STACKLIB_LDFLAGS) $(BOOST_LDFLAGS) $(BOOST_LIBS) $(GLIB_LIBS)
