INCLUDES = \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" -I$(top_srcdir)/src -I$(top_srcdir)/lib -I$(top_srcdir)/lib/widgets -I$(top_srcdir)/lib/xdap $(GNOME_INCLUDEDIR) @GNOMEMEETING_CFLAGS@ -DGNOMEMEETING_IMAGES=\""$(datadir)/pixmaps/"\" -DGNOMEMEETING_SOUNDS=\""$(datadir)/sounds/gnomemeeting/"\" -DSCHEMA_AGE=@SCHEMA_AGE@ -DGNOMEMEETING_DATADIR=\""$(datadir)"\" 

#-DG_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED -DGDK_PIXBUF_DISABLE_DEPRECATED -DGNOME_DISABLE_DEPRECATED

bin_SCRIPTS = gnomemeeting-config-tool
bin_PROGRAMS = gnomemeeting 
EXTRA_PROGRAMS =

gnomemeeting_SOURCES =		\
	addressbook_window.cpp	\
	addressbook_window.h    \
	callbacks.cpp		\
	callbacks.h		\
	calls_history_window.cpp\
	calls_history_window.h	\
	chat_window.cpp		\
	chat_window.h		\
	common.h		\
	config.h		\
	config.cpp		\
	druid.cpp		\
	druid.h			\
	endpoint.cpp		\
	endpoint.h		\
	gdkvideoio.cpp		\
	gdkvideoio.h		\
	gnomemeeting.cpp	\
	gnomemeeting.h		\
	h323endpoint.cpp	\
	h323endpoint.h		\
	h323gatekeeper.cpp	\
	h323gatekeeper.h	\
	ils.cpp			\
	ils.h			\
	log_window.cpp		\
	log_window.h		\
	main_window.cpp		\
	main_window.h		\
	misc.cpp		\
	misc.h			\
	pcssendpoint.cpp	\
	pcssendpoint.h		\
	pref_window.cpp		\
	pref_window.h		\
	sound_handling.cpp	\
	sound_handling.h	\
	stunclient.cpp		\
	stunclient.h		\
	tools.cpp		\
	tools.h			\
	tray.h			\
	tray.cpp		\
	urlhandler.cpp		\
	urlhandler.h		\
	vfakeio.cpp		\
	vfakeio.h		\
	videograbber.h		\
	videograbber.cpp	
	

if ! DISABLE_GNOME
gnomemeeting_SOURCES += bonobo_component.cpp bonobo_component.h
endif

if HAS_DBUS 
gnomemeeting_SOURCES += dbus_component.cpp dbus_component.h
endif

if HAS_HOWL
gnomemeeting_SOURCES += zeroconf_publisher.cpp zeroconf_publisher.h
endif

gnomemeeting_LDADD = \
	@GNOMEMEETING_LIBS@ \
	$(top_builddir)/lib/libgnomemeeting.la

serverdir = $(libdir)/bonobo/servers
server_DATA = @PACKAGE_NAME@.server
