
INCLUDES = @GNOMEMEETING_INCLUDES@ -DGNOMELOCALEDIR=\""$(datadir)/locale"\" -I$(top_srcdir)/src $(GNOME_INCLUDEDIR) -DGNOMEMEETING_IMAGES=\""$(datadir)/pixmaps"\" -DBUILD_ID=\""@BUILD_ID@"\" -DDATADIR=\""$(datadir)"\" -DG_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED -DGDK_PIXBUF_DISABLE_DEPRECATED -DGNOME_DISABLE_DEPRECATED

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
	e-splash.cpp		\
	e-splash.h		\
	chat_window.cpp		\
	chat_window.h		\
	druid.cpp		\
	druid.h


gnomemeeting_LDADD =  @GNOMEMEETING_LDADD@

