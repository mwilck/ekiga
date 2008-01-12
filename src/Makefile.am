INCLUDES = \
	-I$(top_srcdir)/src				\
	-I$(top_srcdir)/src/clients/			\
	-I$(top_srcdir)/src/components/			\
	-I$(top_srcdir)/src/devices/			\
	-I$(top_srcdir)/src/endpoints/			\
	-I$(top_srcdir)/src/gui/			\
	-I$(top_srcdir)/lib				\
	-I$(top_srcdir)/lib/gmconf			\
	-I$(top_srcdir)/lib/gui				\
	-I$(top_srcdir)/lib/engine/			\
	-I$(top_srcdir)/lib/engine/gui/gtk-frontend	\
	-I$(top_srcdir)/lib/engine/addressbook/skel	\
	-I$(top_srcdir)/lib/engine/presence/skel	\
	-I$(top_srcdir)/lib/engine/protocol/skel	\
	-I$(top_srcdir)/lib/engine/protocol/sip		\
	-I$(top_srcdir)/lib/engine/display/skel		\
	-I$(top_srcdir)/lib/engine/framework		\
	-I$(top_builddir)				\
	$(GNOME_INCLUDEDIR)				\
	-DDATA_DIR=\""$(datadir)"\"			\
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
	gui/callbacks.h			\
	gui/callbacks.cpp		\
	gui/conf.h			\
	gui/conf.cpp			\
	gui/dialpad.h			\
	gui/dialpad.cpp			\
	gui/assistant.h			\
	gui/assistant.cpp		\
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
	devices/videooutput.h           \
	devices/videooutput.cpp         \
	devices/videodisplay.h          \
	devices/videodisplay.cpp

# XVideo/X
if !WIN32
ekiga_SOURCES +=                        \
	devices/videodisplay_x.h        \
	devices/videodisplay_x.cpp
endif

# DirectX
if HAVE_DX
ekiga_SOURCES +=                        \
	devices/videodisplay_dx.h       \
	devices/videodisplay_dx.cpp
endif

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
	endpoints/opal-call.h		\
	endpoints/opal-call.cpp		\
	endpoints/opal-codec-description.h	\
	endpoints/opal-codec-description.cpp	\
	endpoints/opal-gmconf-bridge.h	\
	endpoints/opal-gmconf-bridge.cpp\
	endpoints/opal-main.h		\
	endpoints/opal-main.cpp		\
	endpoints/urlhandler.h		\
	endpoints/urlhandler.cpp		

# Clients
ekiga_SOURCES +=			\
	clients/stun.h			\
	clients/stun.cpp

if HAVE_DBUS 
ekiga_SOURCES +=		\
	components/dbus-stub.h	\
	components/dbus.h	\
	components/dbus.cpp


bin_PROGRAMS += ekiga-helper

ekiga_helper_SOURCES = 		\
	components/dbus-helper-stub.h 	\
	components/dbus-helper.cpp

ekiga_helper_LDADD = $(DBUS_LIBS)

BUILT_SOURCES = components/dbus-helper-stub.h components/dbus-stub.h

components/dbus-helper-stub.h: components/dbus-helper-stub.xml
	$(LIBTOOL) --mode=execute dbus-binding-tool --prefix=helper --mode=glib-server --output=$@ $^

components/dbus-stub.h: components/dbus-stub.xml
	$(LIBTOOL) --mode=execute dbus-binding-tool --prefix=ekiga_dbus_component --mode=glib-server --output=$@ $^
endif

if INSTALL_DBUS_SERVICE
servicedir = $(datadir)/dbus-1/services
service_in_files = \
	components/org.ekiga.Ekiga.service.in \
	components/org.ekiga.Helper.service.in
service_DATA = $(service_in_files:.service.in=.service)

components/org.ekiga.Ekiga.service: components/org.ekiga.Ekiga.service.in
	(cd $(srcdir) && sed -e 's#\@bindir\@#@bindir@#' $<) > $@

components/org.ekiga.Helper.service: components/org.ekiga.Helper.service.in
	(cd $(srcdir) && sed -e 's#\@bindir\@#@bindir@#' $<) > $@
endif

if HAVE_AVAHI
ekiga_SOURCES += 		\
	clients/avahi.h		\
	clients/avahi.cpp 	
endif

AM_CXXFLAGS = $(GTK_CFLAGS) $(GLIB_CFLAGS) $(GNOME_CFLAGS) $(AVAHI_CFLAGS) $(DBUS_CFLAGS) $(BONOBO_CFLAGS) $(OPAL_CFLAGS) $(PTLIB_CFLAGS) $(SIGC_CFLAGS) $(XV_CFLAGS) $(DX_CFLAGS) $(XML_CFLAGS)
AM_LIBS = $(GTK_LIBS) $(GLIB_LIBS) $(GNOME_LIBS) $(AVAHI_LIBS) $(DBUS_LIBS) $(BONOBO_LIBS) $(OPAL_LIBS) $(PTLIB_LIBS) $(SIGC_LIBS) $(XML_LIBS)

ekiga_LDADD = \
	$(top_builddir)/lib/libekiga.la $(top_builddir)/lib/engine/libekiga_engine.la $(AM_LIBS) 

EXTRA_DIST = \
	$(service_in_files)		\
	@PACKAGE_NAME@.server		\
	components/dbus-stub.xml	\
	components/dbus-helper-stub.xml

CLEANFILES = \
	$(service_DATA)
