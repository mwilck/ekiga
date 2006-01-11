INCLUDES = \
	-I clients/			\
	-I components/			\
	-I devices/			\
	-I endpoints/			\
	-I gui/				\
	-I$(top_srcdir)/src		\
	-I$(top_srcdir)/lib		\
	-I$(top_srcdir)/lib/gmconf	\
	-I$(top_srcdir)/lib/gmcontacts	\
	-I$(top_srcdir)/lib/gui		\
	-I$(top_srcdir)/lib/xdap	\
	$(GNOME_INCLUDEDIR)		\
	@EKIGA_CFLAGS@		\
	-DDATA_DIR=\""$(datadir)"\"	\
	-DSCHEMA_AGE=@SCHEMA_AGE@ 

#-DG_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED -DGDK_PIXBUF_DISABLE_DEPRECATED -DGNOME_DISABLE_DEPRECATED

bin_SCRIPTS = ekiga-config-tool
bin_PROGRAMS = ekiga

EXTRA_PROGRAMS =

# Common stuff
ekiga_SOURCES =			\
	common.h		

# Graphical User Interface 
ekiga_SOURCES +=			\
	gui/accounts.h    		\
	gui/accounts.cpp		\
	gui/addressbook.h    		\
	gui/addressbook.cpp		\
	gui/callbacks.h			\
	gui/callbacks.cpp		\
	gui/callshistory.h		\
	gui/callshistory.cpp		\
	gui/chat.h			\
	gui/chat.cpp			\
	gui/config.h			\
	gui/config.cpp			\
	gui/druid.h			\
	gui/druid.cpp			\
	gui/history.h			\
	gui/history.cpp			\
	gui/main.h			\
	gui/main.cpp			\
	gui/misc.h			\
	gui/misc.cpp			\
	gui/preferences.h		\
	gui/preferences.cpp		\
	gui/tools.h			\
	gui/tools.cpp			\
	gui/statusicon.h		\
	gui/statusicon.cpp

# Devices 
ekiga_SOURCES +=			\
	devices/audio.h			\
	devices/audio.cpp		\
	devices/fakevideoinput.h	\
	devices/fakevideoinput.cpp	\
	devices/videoinput.h		\
	devices/videoinput.cpp		\
	devices/videooutput.h		\
	devices/videooutput.cpp		

# Endpoints 
ekiga_SOURCES +=			\
	endpoints/accountshandler.h	\
	endpoints/accountshandler.cpp	\
	endpoints/ekiga.h		\
	endpoints/ekiga.cpp		\
	endpoints/manager.h		\
	endpoints/manager.cpp		\
	endpoints/h323.h		\
	endpoints/h323.cpp		\
	endpoints/pcss.h		\
	endpoints/pcss.cpp		\
	endpoints/sip.h			\
	endpoints/sip.cpp		\
	endpoints/urlhandler.h		\
	endpoints/urlhandler.cpp		

# Clients
ekiga_SOURCES +=			\
	clients/stun.h			\
	clients/stun.cpp

	
if !DISABLE_GNOME
ekiga_SOURCES += 		\
	components/bonobo.h		\
	components/bonobo.cpp 
serverdir = $(libdir)/bonobo/servers
server_DATA = components/@PACKAGE_NAME@.server
endif

if HAS_DBUS 
ekiga_SOURCES +=		\
	components/dbus.h	\
	components/dbus.cpp	\
	components/dbus_stub.h

bin_PROGRAMS += ekiga-helper

ekiga_helper_SOURCES = 		\
	components/dbus-helper-stub.h 	\
	components/dbus-helper.cpp

ekiga_helper_LDADD = @DBUS_LIBS@

BUILT_SOURCES = dbus-helper-stub.h dbus_stub.h
dbus-helper-stub.h: components/dbus-helper-stub.xml
	$(LIBTOOL) --mode=execute dbus-binding-tool --prefix=helper --mode=glib-server --output=$@ $^

dbus_stub.h: components/dbus_stub.xml
	$(LIBTOOL) --mode=execute dbus-binding-tool --prefix=dbus_component --mode=glib-server --output=$@ $^
endif

if HAS_HOWL
ekiga_SOURCES += 		\
	clients/zeroconf.h		\
	clients/zeroconf.cpp 
endif

if HAS_AVAHI
ekiga_SOURCES += 		\
	clients/avahi.h		\
	clients/avahi.cpp 	
endif

ekiga_LDADD = \
	$(top_builddir)/lib/libekiga.la \
	@EKIGA_LIBS@


