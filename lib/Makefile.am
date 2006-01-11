SUBDIRS = gmconf gmcontacts gui toolbox

if DISABLE_GNOME
SUBDIRS += about druid
endif

noinst_LTLIBRARIES = libekiga.la

libekiga_la_SOURCES =			\
	gmmarshallers.c				\
	gmmarshallers.h

# forces the build :
BUILT_SOURCES = gmmarshallers.h gmmarshallers.c

if WIN32
libekiga_la_SOURCES += winpaths.c winpaths.h
endif

INCLUDES =						\
	-DG_LOG_DOMAIN=\"Ekiga\"		 	\
	-DSHARE_DIR=\"$(pkgdatadir)\"			\
	-DSYSCONFDIR=\""$(sysconfdir)"\"		\
	-I$(top_srcdir)					\
        $(EKIGA_LIB_CFLAGS)

libekiga_la_LDFLAGS = -export-dynamic
libekiga_la_LIBADD = gmconf/libgmconf.la gmcontacts/libgmcontacts.la gui/libgmwidgets.la toolbox/libtoolbox.la 
if DISABLE_GNOME
libekiga_la_LIBADD += about/libabout.la druid/libdruid.la
endif

gmmarshallers.h: gmmarshallers.list
	$(LIBTOOL) --mode=execute glib-genmarshal --prefix=gm_marshal $^ --header > $@.tmp && mv $@.tmp $@

gmmarshallers.c: gmmarshallers.list
	$(LIBTOOL) --mode=execute glib-genmarshal --prefix=gm_marshal $^ --body > $@.tmp && mv $@.tmp $@
