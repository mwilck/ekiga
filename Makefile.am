SUBDIRS = man sounds pixmaps lib src plugins po

ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}

### Don't execute make in the help subdirectory if BULD_DOC is set
if HAVE_GNOME_DOC_UTILS
SUBDIRS += help
endif


### Intltool on the desktop and on the schemas files
@INTLTOOL_DESKTOP_RULE@
@INTLTOOL_SCHEMAS_RULE@

DESKTOP_IN_FILE=@PACKAGE_NAME@.desktop.in.in
DESKTOP_FILE=@PACKAGE_NAME@.desktop

SCHEMAS_IN_IN_FILE = @PACKAGE_NAME@.schemas.in.in
SCHEMAS_FILE = @PACKAGE_NAME@.schemas

gsettings_SCHEMAS = org.gnome.ekiga.gschema.xml
@INTLTOOL_XML_NOMERGE_RULE@

@GSETTINGS_RULES@


### Win32 files
WIN32_DIST = \
	win32/ico/ekiga.ico					\
	win32/ico/ekiga-uninstall.ico				\
        win32/directx/Amvideo.h                                 \
        win32/directx/control.h                                 \
        win32/directx/ddraw.h                                   \
        win32/directx/dshow.h                                   \
        win32/directx/dsound.h                                  \
        win32/directx/dxerr9.h                                  \
        win32/directx/ksuuids.h                                 \
        win32/directx/mingw_dshow_port.h                        \
        win32/directx/strmif.h                                  \
        win32/directx/uuids.h                                   \
        win32/diff/README                                       \
        win32/diff/ekiga_sticonmenu1.patch                      \
        win32/diff/ekiga_sticonmenu2.patch                      \
	win32/ekiga-rc.rc					\
	win32/Makefile						\
	win32/nsisinstaller/ekiga.nsi				\
	win32/nsisinstaller/language_files/hungarian.nsh	\
	win32/nsisinstaller/language_files/english.nsh		\
	win32/nsisinstaller/language_files/french.nsh		\
	win32/nsisinstaller/language_files/dutch.nsh		\
	win32/nsisinstaller/language_files/romanian.nsh		\
	win32/nsisinstaller/language_files/russian.nsh		\
	win32/nsisinstaller/language_files/german.nsh		\
	win32/nsisinstaller/langmacros.nsh


### Extra dist
EXTRA_DIST = \
	$(DESKTOP_FILE)					        \
        doc/using_dbus.html                                     \
	gnome-doc-utils.make					\
	FAQ							\
	README							\
        LICENSE                                                 \
        MAINTAINERS                                             \
	$(gsettings_SCHEMAS) 					\
	$(SCHEMAS_FILE) 					\
	$(WIN32_DIST)

Applicationsdir = $(datadir)/applications
Applications_DATA = $(DESKTOP_FILE)


### Dist Clear
DISTCLEANFILES=gnome-doc-utils.make ekiga.desktop ekiga.schemas org.gnome.ekiga.gschema

MAINTAINERCLEANFILES= \
	Makefile.in aclocal.m4 compile config.guess config.sub \
	depcomp install-sh ltmain.sh missing config.h.in configure \
	m4/libtool.m4 m4/ltoptions.m4 m4/ltsugar.m4 m4/ltversion.m4 \
	m4/lt~obsolete.m4 m4/gnome-doc-utils.m4

### Install the schemas file in /etc if GCONF support is not compiled in,
### and do not install the schemas at all
if HAVE_GCONF
schemasdir = $(GCONF_SCHEMA_FILE_DIR)
schemas_DATA = $(SCHEMAS_FILE)
else
schemasdir = $(sysconfdir)/@PACKAGE_NAME@
schemas_DATA = $(SCHEMAS_FILE)
endif

install-schemas: @PACKAGE_NAME@.schemas
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) gconftool-2 --makefile-install-rule $(SCHEMAS_FILE) >/dev/null 2>&1
	gconftool-2 --shutdown

if GCONF_SCHEMAS_INSTALL
install-data-local: install-schemas 
else
install-data-local:
endif
