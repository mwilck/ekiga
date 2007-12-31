noinst_LTLIBRARIES = libgmprotocol.la

protocol_dir = $(top_srcdir)/lib/engine/protocol/skel

AM_CPPFLAGS = $(SIGC_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/engine/include \
	-I$(top_srcdir)/lib/engine/framework

libgmprotocol_la_SOURCES = \
	$(protocol_dir)/call-core.h		\
	$(protocol_dir)/call-manager.h		\
	$(protocol_dir)/call.h			\
	$(protocol_dir)/call-core.cpp

libgmprotocol_la_LDFLAGS = -export-dynamic -no-undefined $(SIGC_LIBS)
