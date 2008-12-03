noinst_LTLIBRARIES = libgmloudmouth.la

loudmouth_dir = $(top_srcdir)/lib/engine/components/loudmouth

AM_CXXFLAGS = $(SIGC_CFLAGS) $(LOUDMOUTH_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/engine/framework \
	-I$(top_srcdir)/lib/engine/account/skel \
	-I$(top_srcdir)/lib/engine/presence/skel \
	-I$(top_srcdir)/lib/engine/chat/skel

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
	$(loudmouth_dir)/loudmouth-chat-simple.cpp

libgmloudmouth_la_LDFLAGS = -export-dynamic -no-undefined $(SIGC_LIBS) $(LOUDMOUTH_LIBS)