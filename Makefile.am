SUBDIRS = man sounds pixmaps lib src po 

### Don't execute make in the help subdirectory if BULD_DOC is set
if HAVE_GNOME_DOC_UTILS
SUBDIRS += help
endif


### Intltool on the desktop and on the schemas files
@INTLTOOL_DESKTOP_RULE@
@INTLTOOL_SCHEMAS_RULE@

DESKTOP_IN_FILES=@PACKAGE_NAME@.desktop.in
DESKTOP_FILES=$(DESKTOP_IN_FILES:.desktop.in=.desktop)

SCHEMAS_FILE = @PACKAGE_NAME@.schemas


### Win32 files
WIN32_DIST = \
	win32/ico/ekiga.ico					\
	win32/ico/ekiga-uninstall.ico					\
	win32/diff/opal_opal.pc.in				\
	win32/diff/ptlib_Makefile.am				\
	win32/diff/ptlib_vfw.patch				\
	win32/diff/opal_configure.ac				\
	win32/diff/opal_Makefile.am				\
	win32/diff/ptlib_configure.ac				\
	win32/diff/ptlib_pwlib.pc.in				\
	ekiga.rc						\
	win32/Makefile						\
	win32/README.win32					\
	win32/nsisinstaller/ekiga.nsi				\
	win32/nsisinstaller/language_files/hungarian.nsh	\
	win32/nsisinstaller/language_files/english.nsh		\
	win32/nsisinstaller/language_files/french.nsh		\
	win32/nsisinstaller/langmacros.nsh


### Extra dist
EXTRA_DIST = $(DESKTOP_IN_FILES) $(DESKTOP_FILES) intltool-extract.in intltool-merge.in intltool-update.in gnome-doc-utils.make FAQ README $(SCHEMAS_FILE) src/components/@PACKAGE_NAME@.server gnome-doc-utils.make lib/gmmarshallers.list src/components/dbus-helper-stub.xml src/components/dbus_stub.xml $(WIN32_DIST)
Applicationsdir = $(datadir)/applications
Applications_DATA = $(DESKTOP_FILES)


### Dist Clear
DISTCLEANFILES=gnome-doc-utils.make


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

if DBUS_SERVICES_INSTALL
servicedir = ${datadir}/dbus-1/services
service_DATA = \
	net.ekiga.instance.service	\
	net.ekiga.helper.service
endif
