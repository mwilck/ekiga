plugin_LTLIBRARIES = libgmloudmouth.la

loudmouth_dir = $(top_srcdir)/plugins/loudmouth

AM_CXXFLAGS = \
	$(BOOST_CPPFLAGS) $(XML_CFLAGS) $(LOUDMOUTH_CFLAGS) \
	-I$(top_srcdir)/lib/gmconf \
	-I$(top_srcdir)/lib/engine/framework \
	-I$(top_srcdir)/lib/engine/account \
	-I$(top_srcdir)/lib/engine/presence \
	-I$(top_srcdir)/lib/engine/chat

libgmloudmouth_la_SOURCES = \
	$(loudmouth_dir)/loudmouth-main.cpp \
	$(loudmouth_dir)/loudmouth-handler.h \
	$(loudmouth_dir)/loudmouth-helpers.h \
	$(loudmouth_dir)/loudmouth-helpers.cpp \
	$(loudmouth_dir)/loudmouth-bank.h \
	$(loudmouth_dir)/loudmouth-bank.cpp \
	$(loudmouth_dir)/loudmouth-account.h \
	$(loudmouth_dir)/loudmouth-account.cpp \
	$(loudmouth_dir)/loudmouth-cluster.h \
	$(loudmouth_dir)/loudmouth-cluster.cpp \
	$(loudmouth_dir)/loudmouth-heap-roster.h \
	$(loudmouth_dir)/loudmouth-heap-roster.cpp \
	$(loudmouth_dir)/loudmouth-presentity.h \
	$(loudmouth_dir)/loudmouth-presentity.cpp \
	$(loudmouth_dir)/loudmouth-chat-simple.h \
	$(loudmouth_dir)/loudmouth-chat-simple.cpp \
	$(loudmouth_dir)/loudmouth-chat-multiple.h \
	$(loudmouth_dir)/loudmouth-chat-multiple.cpp \
	$(loudmouth_dir)/loudmouth-dialect.h \
	$(loudmouth_dir)/loudmouth-dialect.cpp

libgmloudmouth_la_LDFLAGS = $(PLUGINS_LIBTOOL_FLAGS)
libgmloudmouth_la_LIBADD = \
	$(top_builddir)/lib/libekiga.la \
	$(BOOST_LDFLAGS) $(XML_LIBS) $(LOUDMOUTH_LIBS)
