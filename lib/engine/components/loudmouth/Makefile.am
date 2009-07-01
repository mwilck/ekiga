plugin_LTLIBRARIES = libgmloudmouth.la

loudmouth_dir = $(top_srcdir)/lib/engine/components/loudmouth

AM_CXXFLAGS = $(SIGC_CFLAGS) $(XML_CFLAGS) $(LOUDMOUTH_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/gmconf \
	-I$(top_srcdir)/lib/engine/framework \
	-I$(top_srcdir)/lib/engine/account \
	-I$(top_srcdir)/lib/engine/presence \
	-I$(top_srcdir)/lib/engine/chat

libgmloudmouth_la_SOURCES = \
	$(loudmouth_dir)/loudmouth-main.h \
	$(loudmouth_dir)/loudmouth-main.cpp \
	$(loudmouth_dir)/loudmouth-bank.h \
	$(loudmouth_dir)/loudmouth-bank.cpp \
	$(loudmouth_dir)/loudmouth-account.h \
	$(loudmouth_dir)/loudmouth-account.cpp \
	$(loudmouth_dir)/loudmouth-cluster.h \
	$(loudmouth_dir)/loudmouth-cluster.cpp \
	$(loudmouth_dir)/loudmouth-heap.h \
	$(loudmouth_dir)/loudmouth-heap.cpp \
	$(loudmouth_dir)/loudmouth-presentity.h \
	$(loudmouth_dir)/loudmouth-presentity.cpp \
	$(loudmouth_dir)/loudmouth-chat-simple.h \
	$(loudmouth_dir)/loudmouth-chat-simple.cpp \
	$(loudmouth_dir)/loudmouth-dialect.h \
	$(loudmouth_dir)/loudmouth-dialect.cpp

libgmloudmouth_la_LDFLAGS = $(PLUGINS_LIBTOOL_FLAGS)
libgmloudmouth_la_LIBADD = $(SIGC_LIBS) $(XML_LIBS) $(LOUDMOUTH_LIBS)