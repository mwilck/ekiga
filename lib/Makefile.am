SUBDIRS = gmconf gui toolbox platform engine

if !HAVE_GNOME
SUBDIRS += druid
endif

noinst_LTLIBRARIES = libekiga.la

libekiga_la_SOURCES =			\
	gmmarshallers.c				\
	gmmarshallers.h

# forces the HAVE :
BUILT_SOURCES = gmmarshallers.h gmmarshallers.c

INCLUDES =						\
	-DSYSCONFDIR=\""$(sysconfdir)"\"		\
	-I$(top_srcdir)					

libekiga_la_LDFLAGS = -export-dynamic
libekiga_la_LIBADD = gmconf/libgmconf.la \
		     gui/libgmwidgets.la \
		     toolbox/libtoolbox.la \
		     platform/libgmplatform.la

if !HAVE_GNOME
libekiga_la_LIBADD += druid/libdruid.la
endif

AM_CFLAGS = $(GLIB_CFLAGS) $(GTK_CFLAGS) $(DBUS_CFLAGS)
AM_LIBS = $(GLIB_LIBS) $(GTK_LIBS) $(DBUS_LIBS)


gmmarshallers.h: gmmarshallers.list
	$(LIBTOOL) --mode=execute glib-genmarshal --prefix=gm_marshal $^ --header > $@.tmp && mv $@.tmp $@

gmmarshallers.c: gmmarshallers.list
	$(LIBTOOL) --mode=execute glib-genmarshal --prefix=gm_marshal $^ --body > $@.tmp && mv $@.tmp $@
