SUBDIRS = man sounds pixmaps lib src po 

### Don't execute make in the help subdirectory if BULD_DOC is set
if HAVE_GNOME_DOC_UTILS
SUBDIRS += help
endif


### Intltool on the desktop and on the schemas files
@INTLTOOL_DESKTOP_RULE@
@INTLTOOL_SCHEMAS_RULE@

DESKTOP_IN_IN_FILES=@PACKAGE_NAME@.desktop.in.in
DESKTOP_IN_FILES=@PACKAGE_NAME@.desktop.in
DESKTOP_FILES=$(DESKTOP_IN_FILES:.desktop.in=.desktop)

SCHEMAS_IN_IN_FILE = @PACKAGE_NAME@.schemas.in.in
SCHEMAS_FILE = @PACKAGE_NAME@.schemas


### Win32 files
WIN32_DIST = \
	win32/ico/ekiga.ico					\
	win32/ico/ekiga-uninstall.ico				\
	win32/diff/opal_opal.pc.in				\
	win32/diff/ptlib_Makefile.am				\
        win32/diff/ptlib_ptlib.pc.in                            \
	win32/diff/opal_configure.ac				\
	win32/diff/opal_Makefile.am				\
	win32/diff/ptlib_configure.ac				\
	win32/diff/ptlib_pwlib.pc.in				\
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
	ekiga.rc						\
	win32/Makefile						\
	win32/nsisinstaller/ekiga.nsi				\
	win32/nsisinstaller/language_files/hungarian.nsh	\
	win32/nsisinstaller/language_files/english.nsh		\
	win32/nsisinstaller/language_files/french.nsh		\
	win32/nsisinstaller/langmacros.nsh


### Extra dist
EXTRA_DIST = \
	$(DESKTOP_IN_IN_FILES)					\
        doc/                                                    \
	gnome-doc-utils.make					\
	FAQ							\
	README							\
	$(SCHEMAS_IN_IN_FILE)					\
	gnome-doc-utils.make					\
	lib/gmmarshallers.list					\
	$(WIN32_DIST)

Applicationsdir = $(datadir)/applications
Applications_DATA = $(DESKTOP_FILES)


### Dist Clear
DISTCLEANFILES=gnome-doc-utils.make ekiga.desktop ekiga.schemas

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
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) gconftool-2 --makefile-install-rule $(SCHEMAS_FILE) 2>&1 > /dev/null
	gconftool-2 --shutdown

if GCONF_SCHEMAS_INSTALL
install-data-local: install-schemas 
else
install-data-local:
endif
