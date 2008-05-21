noinst_LTLIBRARIES = libgmchat.la

chat_dir = $(top_srcdir)/lib/engine/chat/skel

AM_CPPFLAGS = $(SIGC_CFLAGS) $(GLIB_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/engine/include 		\
	-I$(top_srcdir)/lib/engine/framework

libgmchat_la_SOURCES = \
	$(chat_dir)/chat-core.h			\
	$(chat_dir)/chat-core.cpp		\
	$(chat_dir)/chat-manager.h

libgmchat_la_LDFLAGS = -export-dynamic -no-undefined $(SIGC_LIBS)
