@INTLTOOL_SOUNDLIST_RULE@

SOUNDLIST_IN_FILES=GnomeMeeting.soundlist.in
SOUNDLIST_FILES=$(SOUNDLIST_IN_FILES:.soundlist.in=.soundlist)

gnomemeeting_SOUNDS = gnomemeeting.wav

EXTRA_DIST = $(gnomemeeting_SOUNDS) $(SOUNDLIST_IN_FILES) $(SOUNDLIST_FILES)

gnomemeetingsoundsdir = $(datadir)/sounds/gnomemeeting
gnomemeetingsounds_DATA = $(gnomemeeting_SOUNDS)

soundlistdir = $(sysconfdir)/sound/events
soundlist_DATA = $(SOUNDLIST_FILES)
