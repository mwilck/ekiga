INCLUDES = \
	@GNOMEMEETING_INCLUDES@ 			\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" 	\
	-I$(top_srcdir)/src 				\
	-I$(top_srcdir)/lib				\
	-I$(top_srcdir)/lib/widgets			\
	$(GNOME_INCLUDEDIR) 				\
	-DGNOMEMEETING_IMAGES=\""$(datadir)/pixmaps"\" 	\
	-DSCHEMA_AGE=@SCHEMA_AGE@ 			\
	-DDATADIR=\""$(datadir)"\" 			\
	-DG_DISABLE_DEPRECATED 				\
	-DGTK_DISABLE_DEPRECATED 			\
	-DGDK_DISABLE_DEPRECATED 			\
	-DGDK_DISABLE_DEPRECATED 			\
	-DGDK_PIXBUF_DISABLE_DEPRECATED 		\
	-DGNOME_DISABLE_DEPRECATED

bin_SCRIPTS = gnomemeeting-config-tool
bin_PROGRAMS = gnomemeeting

gnomemeeting_SOURCES =		\
	common.h		\
	callbacks.cpp		\
	callbacks.h		\
	gdkvideoio.cpp		\
	gdkvideoio.h		\
	gnomemeeting.cpp	\
	gnomemeeting.h		\
	connection.cpp		\
	connection.h		\
	endpoint.cpp		\
	endpoint.h		\
	menu.cpp		\
	menu.h			\
	toolbar.cpp		\
	toolbar.h		\
	pref_window.cpp		\
	pref_window.h		\
	videograbber.h		\
	videograbber.cpp	\
	config.h		\
	config.cpp		\
	ldap_window.cpp		\
	ldap_window.h		\
	docklet.cpp		\
	docklet.h		\
	main_window.cpp		\
	main_window.h		\
	audio.cpp		\
	audio.h			\
	gatekeeper.cpp		\
	gatekeeper.h		\
	ils.cpp			\
	ils.h			\
	cleaner.cpp		\
	cleaner.h		\
	misc.cpp		\
	misc.h			\
	chat_window.cpp		\
	chat_window.h		\
	druid.cpp		\
	druid.h			\
	urlhandler.cpp		\
	urlhandler.h


gnomemeeting_LDADD = \
	$(top_builddir)/lib/libgnomemeeting.la	\
	@GNOMEMEETING_LDADD@

serverdir = $(libdir)/bonobo/servers
server_DATA = gnomemeeting.server
