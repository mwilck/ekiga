### Don't execute make in the help subdirectory if DISABLE_GNOME is set
if DISABLE_GNOME
SUBDIRS = man sounds xdap_data pixmaps lib src po 
else
SUBDIRS = help man sounds xdap_data pixmaps lib src po 
endif


### Intltool on the desktop and on the schemas files
@INTLTOOL_DESKTOP_RULE@
@INTLTOOL_SCHEMAS_RULE@

DESKTOP_IN_FILES=@PACKAGE_NAME@.desktop.in
DESKTOP_FILES=$(DESKTOP_IN_FILES:.desktop.in=.desktop)

SCHEMAS_FILE = @PACKAGE_NAME@.schemas


### Extra dist
EXTRA_DIST = $(DESKTOP_IN_FILES) $(DESKTOP_FILES) intltool-extract.in intltool-merge.in intltool-update.in FAQ README $(SCHEMAS_FILE) src/@PACKAGE_NAME@.server lib/gm_events_marshalers.list
Applicationsdir = $(datadir)/applications
Applications_DATA = $(DESKTOP_FILES)


### Install the schemas file in /etc if GNOME support is not compiled in,
### and do not install the schemas at all
if DISABLE_GNOME
schemasdir = $(sysconfdir)/@PACKAGE_NAME@
schemas_DATA = $(SCHEMAS_FILE)
else
schemasdir = $(GCONF_SCHEMA_FILE_DIR)
schemas_DATA = $(SCHEMAS_FILE)
endif

install-schemas: @PACKAGE_NAME@.schemas
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) gconftool-2 --makefile-install-rule $(SCHEMAS_FILE) 2>&1 > /dev/null
	gconftool-2 --shutdown

install-services: @PACKAGE_NAME@.service
	cp @PACKAGE_NAME@.service $(libdir)/dbus-1.0/services/

if GCONF_SCHEMAS_INSTALL
install-data-local: install-schemas 
else
install-data-local:
endif

if DBUS_SERVICES_INSTALL
install-data-local: install-services
endif
