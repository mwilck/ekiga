noinst_LTLIBRARIES = libgmsip.la

sip_dir = $(top_srcdir)/lib/engine/protocol/sip

AM_CPPFLAGS = $(SIGC_CFLAGS) $(OPAL_CFLAGS) $(PWLIB_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/engine/include 		\
	-I$(top_srcdir)/lib/engine/framework 		\
	-I$(top_srcdir)/lib/engine/presence/skel 	\
	-I$(top_srcdir)/lib/engine/addressbook/skel

libgmsip_la_SOURCES = \
	$(sip_dir)/sip-basic.h \
	$(sip_dir)/sip-basic.cpp \
	$(sip_dir)/sip-endpoint.h \
	$(sip_dir)/sip-endpoint.cpp \
	$(sip_dir)/sip-presence.h \
	$(sip_dir)/sip-presence.cpp \
	$(sip_dir)/sip-main.h \
	$(sip_dir)/sip-main.cpp

libgmsip_la_LIBADD = \
	$(top_srcdir)/lib/engine/presence/skel/libgmpresence.la \
	$(top_srcdir)/lib/engine/addressbook/skel/libgmaddressbook.la

libgmsip_la_LDFLAGS = -export-dynamic -no-undefined $(SIGC_LIBS) $(OPAL_LIBS) $(PWLIB_LIBS)

#### FIXME should be removed at a later stage
INCLUDES += \
	-I$(top_srcdir)/src                             \
	-I$(top_srcdir)/src/clients/                    \
	-I$(top_srcdir)/src/components/                 \
	-I$(top_srcdir)/src/devices/                    \
	-I$(top_srcdir)/src/endpoints/                  \
	-I$(top_srcdir)/src/gui/                        

AM_CPPFLAGS += $(GTK_CFLAGS) $(GLIB_CFLAGS) $(OPAL_CFLAGS) $(PWLIB_CFLAGS)

libgmsip_la_LDFLAGS += $(GTK_LIBS) $(GLIB_LIBS) $(OPAL_LIBS) $(PWLIB_LIBS)

#### End
