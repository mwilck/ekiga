SUBDIRS = po src pixmaps sounds

@INTLTOOL_DESKTOP_RULE@

DESKTOP_IN_FILES=gnomemeeting.desktop.in
DESKTOP_FILES=$(DESKTOP_IN_FILES:.desktop.in=.desktop)

SCHEMAS_FILE = gnomemeeting.schema

bin_SCRIPTS = gnomemeeting-config-tool

EXTRA_DIST = $(DESKTOP_IN_FILES) $(DESKTOP_FILES) pixmaps/  intltool-extract.in intltool-merge.in intltool-update.in FAQ $(SCHEMAS_FILE)
Applicationsdir = $(datadir)/applications
Applications_DATA = $(DESKTOP_FILES)

schemasdir = $(GCONF_SCHEMA_FILE_DIR)
schemas_DATA = $(SCHEMAS_FILE)

install-schemas: gnomemeeting.schema
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) gconftool --makefile-install-rule $(SCHEMAS_FILE)
	gconftool --shutdown

install-data-local: install-schemas

gnomemeeting-config-tool: gnomemeeting-config-tool.in Makefile
	sed -e "s:@GNOMEMEETING_SYSCONFDIR@:$(sysconfdir):;" < $< > $@
