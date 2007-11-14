noinst_LTLIBRARIES = libcall-history.la

call_history_dir = $(top_srcdir)/lib/engine/presence/call-history

AM_CPPFLAGS = $(XML_CFLAGS) $(SIGC_CFLAGS) $(GLIB_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/engine/framework \
	-I$(top_srcdir)/lib/gmconf \
	-I$(top_srcdir)/lib/engine/addressbook/skel

libcall_history_la_SOURCES = \
	$(call_history_dir)/history-presentity.h \
	$(call_history_dir)/history-presentity.cpp \
	$(call_history_dir)/history-heap.h \
	$(call_history_dir)/history-heap.cpp \
	$(call_history_dir)/history-cluster.h \
	$(call_history_dir)/history-cluster.cpp \
	$(call_history_dir)/history-main.h \
	$(call_history_dir)/history-main.cpp

libcall_history_la_LIBADD = \
	$(top_builddir)/lib/engine/addressbook/skel/libgmaddressbook.la

libcall_history_la_LDFLAGS = -export-dynamic -no-undefined
