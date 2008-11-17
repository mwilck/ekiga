noinst_LTLIBRARIES = libopal.la

opal_dir = $(top_srcdir)/lib/engine/components/opal

AM_CXXFLAGS = $(SIGC_CFLAGS) $(GLIB_CFLAGS) $(OPAL_CFLAGS) $(PTLIB_CFLAGS)

INCLUDES = 					        \
	-I$(top_srcdir)/lib				\
	-I$(top_srcdir)/lib/gmconf			\
	-I$(top_srcdir)/lib/gmref			\
	-I$(top_srcdir)/lib/toolbox/			\
	-I$(top_srcdir)/lib/engine/			\
	-I$(top_srcdir)/lib/engine/account/skel		\
	-I$(top_srcdir)/lib/engine/addressbook/skel	\
	-I$(top_srcdir)/lib/engine/chat/skel		\
	-I$(top_srcdir)/lib/engine/presence/skel	\
	-I$(top_srcdir)/lib/engine/protocol/skel	\
	-I$(top_srcdir)/lib/engine/videooutput/skel	\
	-I$(top_srcdir)/lib/engine/videoinput/skel	\
	-I$(top_srcdir)/lib/engine/audioinput/skel	\
	-I$(top_srcdir)/lib/engine/audiooutput/skel	\
	-I$(top_srcdir)/lib/engine/framework

# Endpoints 
libopal_la_SOURCES = 			\
	opal-call-manager.h		\
	opal-call-manager.cpp		\
	pcss-endpoint.h		        \
	pcss-endpoint.cpp		\
	opal-account.h		        \
	opal-account.cpp		\
	opal-bank.h			\
	opal-bank.cpp			\
	opal-call.h			\
	opal-call.cpp			\
	opal-codec-description.h	\
	opal-codec-description.cpp	\
	opal-gmconf-bridge.h		\
	opal-gmconf-bridge.cpp	        \
	opal-main.h			\
	opal-main.cpp

if HAVE_H323
libopal_la_SOURCES += 			\
	h323-endpoint.h		        \
	h323-endpoint.cpp
endif

if HAVE_SIP
libopal_la_SOURCES += 			\
	sip-chat-simple.h		\
	sip-chat-simple.cpp		\
	sip-dialect.h			\
	sip-dialect.cpp		        \
	sip-endpoint.h		        \
	sip-endpoint.cpp
endif

libopal_la_LIBADD = 						\
	$(top_builddir)/lib/engine/framework/libgmframework.la

libopal_la_LDFLAGS = -export-dynamic -no-undefined $(SIGC_LIBS) $(GLIB_LIBS) $(OPAL_LIBS) $(PTLIB_LIBS)
