plugin_LTLIBRARIES = libgmloudmouth.la

AM_CXXFLAGS = \
	$(BOOST_CPPFLAGS) $(XML_CFLAGS) $(LOUDMOUTH_CFLAGS) $(GLIB_CFLAGS) \
	-I$(top_srcdir)/lib/ \
	-I$(top_srcdir)/lib/settings/ \
	-I$(top_srcdir)/lib/engine/framework \
	-I$(top_srcdir)/lib/engine/account \
	-I$(top_srcdir)/lib/engine/presence \
	-I$(top_srcdir)/lib/engine/chat

libgmloudmouth_la_SOURCES = \
	loudmouth-main.cpp \
	loudmouth-handler.h \
	loudmouth-helpers.h \
	loudmouth-helpers.cpp \
	loudmouth-bank.h \
	loudmouth-bank.cpp \
	loudmouth-account.h \
	loudmouth-account.cpp \
	loudmouth-cluster.h \
	loudmouth-cluster.cpp \
	loudmouth-heap-roster.h \
	loudmouth-heap-roster.cpp \
	loudmouth-presentity.h \
	loudmouth-presentity.cpp \
	loudmouth-chat-simple.h \
	loudmouth-chat-simple.cpp \
	loudmouth-chat-multiple.h \
	loudmouth-chat-multiple.cpp \
	loudmouth-dialect.h \
	loudmouth-dialect.cpp

libgmloudmouth_la_LDFLAGS = $(PLUGINS_LIBTOOL_FLAGS)
libgmloudmouth_la_LIBADD = \
	$(top_builddir)/lib/libekiga.la \
	$(BOOST_LDFLAGS) $(XML_LIBS) $(LOUDMOUTH_LIBS) $(GLIB_LIBS)
