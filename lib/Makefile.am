SUBDIRS = widgets xdap contacts

if DISABLE_GNOME
SUBDIRS += about druid
endif

noinst_LTLIBRARIES = libgnomemeeting.la

libgnomemeeting_la_SOURCES =				\
	dialog.c					\
	dialog.h					\
	stock-icons.c					\
	stock-icons.h					\
	gtk-text-tag-addon.c                            \
	gtk-text-tag-addon.h                            \
	gtk-text-buffer-addon.c                         \
	gtk-text-buffer-addon.h                         \
	gtk-text-view-addon.c                           \
	gtk-text-view-addon.h                           \
	gtk_menu_extensions.c				\
	gtk_menu_extensions.h				\
	gm_conf.h				 	\
	gm_conf-widgets_extensions.h			\
	gm_conf-widgets_extensions.c			\
	gnome_prefs_window.c				\
	gnome_prefs_window.h				\
	e-splash.c					\
	e-splash.h					\
	eggtrayicon.h					\
	eggtrayicon.c					\
	stats_drawing_area.h				\
	stats_drawing_area.c				\
	gm_events.h					\
	gm_events.c					\
	gm_events_marshalers.h				\
	gm_events_marshalers.c

if DISABLE_GNOME
libgnomemeeting_la_SOURCES += gm_conf-glib.c
else
libgnomemeeting_la_SOURCES += gm_conf-gconf.c
endif

INCLUDES =						\
	-DGNOMELOCALEDIR=\""$(datadir)/locale/"\"       \
	-DGNOMEMEETING_IMAGES=\""$(datadir)/pixmaps/"\"	\
	-DG_LOG_DOMAIN=\"GnomeMeeting\"		 	\
	-DSHARE_DIR=\"$(pkgdatadir)\"			\
	-DSYSCONFDIR=\""$(sysconfdir)"\"		\
	-I$(top_srcdir)					\
        $(GNOMEMEETING_LIB_CFLAGS)

libgnomemeeting_la_LDFLAGS = -export-dynamic
libgnomemeeting_la_LIBADD = widgets/libgmwidgets.la xdap/libxdap.la contacts/libgmcontacts.la  
if DISABLE_GNOME
libgnomemeeting_la_LIBADD += about/libabout.la druid/libdruid.la
endif

gm_events_marshalers.h: gm_events_marshalers.list
	glib-genmarshal --prefix=gm_event --header $^ > $@

gm_events_marshalers.c: gm_events_marshalers.list
	glib-genmarshal --prefix=gm_event --body $^ > $@

gm_events.c: gm_events_marshalers.h