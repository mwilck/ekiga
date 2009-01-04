INCLUDES = \
	-I$(top_srcdir)/lib				\
	-I$(top_srcdir)/lib/gmconf			\
	-I$(top_srcdir)/lib/toolbox			\
	-I$(top_srcdir)/lib/gui				\
	-I$(top_srcdir)/lib/engine/			\
	-I$(top_srcdir)/lib/engine/framework		\
	-I$(top_srcdir)/lib/engine/gui/gtk-frontend	\
	-I$(top_srcdir)/lib/engine/account		\
	-I$(top_srcdir)/lib/engine/addressbook	\
	-I$(top_srcdir)/lib/engine/chat/skel		\
	-I$(top_srcdir)/lib/engine/presence	\
	-I$(top_srcdir)/lib/engine/protocol	\
	-I$(top_srcdir)/lib/engine/protocol/sip		\
	-I$(top_srcdir)/lib/engine/videooutput/skel	\
	-I$(top_srcdir)/lib/engine/videoinput/skel	\
	-I$(top_srcdir)/lib/engine/audioinput/skel	\
	-I$(top_srcdir)/lib/engine/audiooutput/skel	\
	-I$(top_srcdir)/lib/engine/hal/skel		\
	-I$(top_srcdir)/lib/engine/framework		\
	-I$(top_srcdir)/lib/engine/gui/gtk-core		\
	-I$(top_srcdir)/lib/engine/components/opal	\
	-I$(top_srcdir)/src				\
	-I$(top_srcdir)/src/dbus-helper/		\
	-I$(top_srcdir)/src/gui/			\
	-I$(top_builddir)				\
	$(GNOME_INCLUDEDIR)				\
	-DSCHEMA_AGE=@SCHEMA_AGE@ 

BUILT_SOURCES = src/revision.h

# XVideo/X
if !WIN32
INCLUDES +=                   				\
	-I$(top_srcdir)/lib/engine/videooutput/common   \
	-I$(top_srcdir)/lib/engine/videooutput/x
endif

# DirectX
if HAVE_DX
INCLUDES +=                   				\
	-I$(top_srcdir)/lib/engine/videooutput/common   \
	-I$(top_srcdir)/lib/engine/videooutput/dx
endif


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
	gui/statusicon.h		\
	gui/statusicon.cpp		\
	gui/statusmenu.h		\
	gui/statusmenu.cpp

# Endpoints 
ekiga_SOURCES +=				\
	ekiga.h			                \
	ekiga.cpp

if HAVE_DBUS 
ekiga_SOURCES +=		\
	dbus-helper/dbus-stub.h	\
	dbus-helper/dbus.h	\
	dbus-helper/dbus.cpp


bin_PROGRAMS += ekiga-helper

ekiga_helper_SOURCES = 		\
	dbus-helper/dbus-helper-stub.h 	\
	dbus-helper/dbus-helper.cpp

ekiga_helper_LDADD = $(DBUS_LIBS)

BUILT_SOURCES += dbus-helper/dbus-helper-stub.h dbus-helper/dbus-stub.h

dbus-helper/dbus-helper-stub.h: dbus-helper/dbus-helper-stub.xml
	$(LIBTOOL) --mode=execute dbus-binding-tool --prefix=helper --mode=glib-server --output=$@ $^

dbus-helper/dbus-stub.h: dbus-helper/dbus-stub.xml
	$(LIBTOOL) --mode=execute dbus-binding-tool --prefix=ekiga_dbus_component --mode=glib-server --output=$@ $^
endif

if INSTALL_DBUS_SERVICE

%.service: %.service.in
	$(mkdir_p) dbus-helper # this is ugly...
	sed -e "s#\@bindir\@#@bindir@#" $< > $@

servicedir = $(datadir)/dbus-1/services
service_DATA = dbus-helper/org.ekiga.Ekiga.service dbus-helper/org.ekiga.Helper.service
service_in_files = $(service_DATA:.service=.service.in)

endif

SVN_REVISION=$(shell head ../.svn/entries -n11 2>/dev/null | tail -n1)
CACHED_REVISION=$(shell cat revision.h 2>/dev/null | cut -c24-)

src/revision.h:
$(shell if test -e "../.svn/entries"; then \
          if test "x$(SVN_REVISION)" != "x$(CACHED_REVISION)"; then \
            echo "#define EKIGA_REVISION $(SVN_REVISION)" > revision.h; \
          fi \
        else \
          if !(test -e "./revision.h"); then\
            echo "#define EKIGA_REVISION \"unknown\"" > revision.h; \
          fi \
        fi)

AM_CXXFLAGS = $(GTK_CFLAGS) $(GLIB_CFLAGS) $(GNOME_CFLAGS) $(DBUS_CFLAGS) $(BONOBO_CFLAGS) $(OPAL_CFLAGS) $(PTLIB_CFLAGS) $(SIGC_CFLAGS) $(XML_CFLAGS) $(NOTIFY_CFLAGS)
AM_LIBS = $(GTK_LIBS) $(GLIB_LIBS) $(GNOME_LIBS) $(DBUS_LIBS) $(BONOBO_LIBS) $(OPAL_LIBS) $(PTLIB_LIBS) $(SIGC_LIBS) $(XML_LIBS) $(NOTIFY_LIBS)

ekiga_LDADD = \
	$(top_builddir)/lib/libekiga.la $(top_builddir)/lib/engine/libekiga_engine.la $(AM_LIBS) 

EXTRA_DIST = \
	$(service_in_files)		\
	dbus-helper/dbus-stub.xml	\
	dbus-helper/dbus-helper-stub.xml

CLEANFILES = \
	$(service_DATA)

DISTCLEANFILES = revision.h
