noinst_LTLIBRARIES = libgmpresence.la

presence_incdir = $(top_srcdir)/lib/engine/include/presence
presence_srcdir = $(top_srcdir)/lib/engine/src/presence

AM_CXXFLAGS = $(SIGC_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/engine/include \
	-I$(top_srcdir)/lib/engine/include/framework \
	-I$(top_srcdir)/lib/engine/include/presence

presence_generics = \
	$(presence_incdir)/presentity.h			\
	$(presence_incdir)/heap.h			\
	$(presence_incdir)/heap-impl.h			\
	$(presence_incdir)/cluster.h			\
	$(presence_incdir)/cluster-impl.h		\
	$(presence_incdir)/presence-core.h		\
	$(presence_srcdir)/presence-core.cpp


libgmpresence_la_SOURCES = \
	$(presence_generics)

libgmpresence_la_LDFLAGS = -export-dynamic -no-undefined $(SIGC_LIBS)
