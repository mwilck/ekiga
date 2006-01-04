SUBDIRS = gmconf gmcontacts gui xdap toolbox

if DISABLE_GNOME
SUBDIRS += about druid
endif

noinst_LTLIBRARIES = libgnomemeeting.la

libgnomemeeting_la_SOURCES =				\
	gm_marshallers.c				\
	gm_marshallers.h

# forces the build :
BUILT_SOURCES = gm_marshallers.h gm_marshallers.c

if WIN32
libgnomemeeting_la_SOURCES += winpaths.c winpaths.h
endif

if !DISABLE_GNOME
libgnomemeeting_la_SOURCES +=				\
	eggtrayicon.h					\
	eggtrayicon.c
endif

INCLUDES =						\
	-DG_LOG_DOMAIN=\"GnomeMeeting\"		 	\
	-DSHARE_DIR=\"$(pkgdatadir)\"			\
	-DSYSCONFDIR=\""$(sysconfdir)"\"		\
	-I$(top_srcdir)					\
        $(GNOMEMEETING_LIB_CFLAGS)

libgnomemeeting_la_LDFLAGS = -export-dynamic
libgnomemeeting_la_LIBADD = gmconf/libgmconf.la gmcontacts/libgmcontacts.la gui/libgmwidgets.la toolbox/libtoolbox.la xdap/libxdap.la 
if DISABLE_GNOME
libgnomemeeting_la_LIBADD += about/libabout.la druid/libdruid.la
endif

gm_marshallers.h: gm_marshallers.list
	$(LIBTOOL) --mode=execute glib-genmarshal --prefix=gm_marshal $^ --header > $@.tmp && mv $@.tmp $@

gm_marshallers.c: gm_marshallers.list
	$(LIBTOOL) --mode=execute glib-genmarshal --prefix=gm_marshal $^ --body > $@.tmp && mv $@.tmp $@
