SUBDIRS = macros po intl src pixmaps sounds

## to automatically rebuild aclocal.m4 if any of the macros in
## `macros/' change
@MAINT@include macros/macros.dep
@MAINT@macros/macros.dep: macros/Makefile.am
@MAINT@		cd macros && $(MAKE) macros.dep

## We dist autogen.sh since this is an example program
## Real-world programs do not need to distribute autogen.sh
EXTRA_DIST = 			\
	autogen.sh		


Applicationsdir = $(datadir)/gnome/apps/Internet
Applications_DATA = gnomemeeting.desktop

dist-hook: 

