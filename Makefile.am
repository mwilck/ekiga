SUBDIRS = macros po intl src pixmaps sounds

@XML_I18N_MERGE_DESKTOP_RULE@

DESKTOP_IN_FILES=gnomemeeting.desktop.in
DESKTOP_FILES=$(DESKTOP_IN_FILES:.desktop.in=.desktop)
EXTRA_DIST = $(DESKTOP_IN_FILES) $(DESKTOP_FILES)
Applicationsdir = $(datadir)/gnome/apps/Internet
Applications_DATA = $(DESKTOP_FILES)

dist-hook: 

