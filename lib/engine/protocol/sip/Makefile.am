noinst_LTLIBRARIES = libgmsip.la

sip_dir = $(top_srcdir)/lib/engine/protocol/sip

AM_CPPFLAGS = $(SIGC_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/engine/include \
	-I$(top_srcdir)/lib/engine/include/framework \
	-I$(top_srcdir)/lib/engine/include/presence \
	-I$(top_srcdir)/lib/engine/include/addressbook

libgmsip_la_SOURCES = \
	$(sip_dir)/sip-basic.h \
	$(sip_dir)/sip-basic.cpp \
	$(sip_dir)/sip-presence.h \
	$(sip_dir)/sip-presence.cpp \
	$(sip_dir)/sip-main.h \
	$(sip_dir)/sip-main.cpp

libgmsip_la_LIBADD = \
	$(top_srcdir)/lib/engine/src/presence/libgmpresence.la \
	$(top_srcdir)/lib/engine/src/addressbook/libgmaddressbook.la

libgmsip_la_LDFLAGS = -export-dynamic -no-undefined $(SIGC_LIBS)
