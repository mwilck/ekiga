noinst_LTLIBRARIES = libopal.la

opal_dir = $(top_srcdir)/lib/engine/components/opal

AM_CXXFLAGS = $(SIGC_CFLAGS) $(GLIB_CFLAGS) $(OPAL_CFLAGS) $(PTLIB_CFLAGS)

INCLUDES = 					        \
	-I$(top_srcdir)/lib				\
	-I$(top_srcdir)/lib/gmconf			\
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
libopal_la_SOURCES = \
	$(opal_dir)/opal-call-manager.h \
	$(opal_dir)/opal-call-manager.cpp \
	$(opal_dir)/pcss-endpoint.h \
	$(opal_dir)/pcss-endpoint.cpp \
	$(opal_dir)/opal-account.h \
	$(opal_dir)/opal-account.cpp \
	$(opal_dir)/opal-bank.h \
	$(opal_dir)/opal-bank.cpp \
	$(opal_dir)/opal-call.h \
	$(opal_dir)/opal-call.cpp \
	$(opal_dir)/opal-codec-description.h \
	$(opal_dir)/opal-codec-description.cpp \
	$(opal_dir)/opal-gmconf-bridge.h \
	$(opal_dir)/opal-gmconf-bridge.cpp \
	$(opal_dir)/opal-main.h \
	$(opal_dir)/opal-main.cpp

if HAVE_H323
libopal_la_SOURCES += \
	$(opal_dir)/h323-endpoint.h \
	$(opal_dir)/h323-endpoint.cpp
endif

if HAVE_SIP
libopal_la_SOURCES += \
	$(opal_dir)/sip-chat-simple.h \
	$(opal_dir)/sip-chat-simple.cpp \
	$(opal_dir)/sip-dialect.h \
	$(opal_dir)/sip-dialect.cpp \
	$(opal_dir)/sip-endpoint.h \
	$(opal_dir)/sip-endpoint.cpp
endif

libopal_la_LIBADD = \
	$(top_builddir)/lib/engine/framework/libgmframework.la

libopal_la_LDFLAGS = -export-dynamic -no-undefined $(SIGC_LIBS) $(GLIB_LIBS) $(OPAL_LIBS) $(PTLIB_LIBS)
