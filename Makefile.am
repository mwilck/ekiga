SUBDIRS = macros po intl src pixmaps sounds

@INTLTOOL_DESKTOP_RULE@

DESKTOP_IN_FILES=gnomemeeting.desktop.in
DESKTOP_FILES=$(DESKTOP_IN_FILES:.desktop.in=.desktop)
EXTRA_DIST = $(DESKTOP_IN_FILES) $(DESKTOP_FILES) pixmaps/  intltool-extract.in intltool-merge.in intltool-update.in FAQ
Applicationsdir = $(datadir)/gnome/apps/Internet
Applications_DATA = $(DESKTOP_FILES)

dist-hook: 

