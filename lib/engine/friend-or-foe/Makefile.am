noinst_LTLIBRARIES = libgmfriendorfoe.la

friend_or_foe_dir = $(top_srcdir)/lib/engine/friend-or-foe

AM_CXXFLAGS = $(SIGC_CFLAGS) $(GLIB_CFLAGS)

INCLUDES = -I$(top_srcdir)/lib/engine/framework

libgmfriendorfoe_la_SOURCES = \
	$(friend_or_foe_dir)/friend-or-foe.h \
	$(friend_or_foe_dir)/friend-or-foe.cpp

libgmaddressbook_la_LDFLAGS = -export-dynamic -no-undefined