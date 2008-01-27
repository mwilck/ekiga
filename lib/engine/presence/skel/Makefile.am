noinst_LTLIBRARIES = libgmpresence.la

presence_dir = $(top_srcdir)/lib/engine/presence/skel

AM_CXXFLAGS = $(SIGC_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/engine/include \
	-I$(top_srcdir)/lib/engine/framework \
	-I$(top_srcdir)/lib/engine/presence/skel \
	-I$(top_srcdir)/lib/engine/protocol/skel

libgmpresence_la_SOURCES = \
	$(presence_dir)/presentity.h		\
	$(presence_dir)/heap.h			\
	$(presence_dir)/heap-impl.h		\
	$(presence_dir)/cluster.h		\
	$(presence_dir)/cluster-impl.h		\
	$(presence_dir)/presence-core.h		\
	$(presence_dir)/presence-core.cpp

libgmpresence_la_LDFLAGS = -export-dynamic -no-undefined $(SIGC_LIBS)
