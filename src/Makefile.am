
INCLUDES = @GNOMEMEETING_INCLUDES@ -DGNOMELOCALEDIR=\""$(datadir)/locale"\" -I$(top_srcdir)/src $(GNOME_INCLUDEDIR)


bin_PROGRAMS = gnomemeeting

gnomemeeting_SOURCES =		\
	callbacks.cpp		\
	callbacks.h		\
	gdkvideoio.cpp		\
	gdkvideo.h		\
	main.cpp		\
	main.h			\
	connection.cpp		\
	connection.h		\
	endpoint.cpp		\
	endpoint.h		\
	menu.cpp		\
	menu.h			\
	toolbar.cpp		\
	toolbar.h		\
	preferences.cpp		\
	preferences.h		\
	webcam.cpp		\
	config.cpp		\
	config.h		\
	ldap.cpp		\
	ldap.h			\
	splash.cpp		\
	splash.h		\
	applet.cpp		\
	applet.h		\
	main_interface.cpp		\
	main_interface.h		\
	audio.cpp				\
	audio.h


gnomemeeting_LDADD =  @GNOMEMEETING_LDADD@ -lpanel_applet

#Iconsdir = $(datadir)/pixmaps
#Icons_DATA =    gnome-napster.png


#gnorbadir   = $(sysconfdir)/CORBA/servers
#gnorba_DATA = Gnapster.gnorba
