stacklib_LTLIBRARIES = libgmfriendorfoe.la

friend_or_foe_dir = $(top_srcdir)/lib/engine/friend-or-foe

AM_CXXFLAGS = $(BOOST_CPPFLAGS) $(SIGC_CFLAGS) $(GLIB_CFLAGS)

INCLUDES = -I$(top_srcdir)/lib/engine/framework

libgmfriendorfoe_la_SOURCES = \
	$(friend_or_foe_dir)/friend-or-foe.h \
	$(friend_or_foe_dir)/friend-or-foe.cpp

libgmfriendorfoe_la_LDFLAGS = $(STACKLIB_LDFLAGS)