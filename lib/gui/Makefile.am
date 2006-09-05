SUBDIRS = gmtray

noinst_LTLIBRARIES = libgmwidgets.la


libgmwidgets_la_SOURCES =	\
	gmconfwidgets.c		\
	gmconfwidgets.h		\
	gmcodecsbox.cpp		\
	gmcodecsbox.h		\
	gmconnectbutton.c	\
	gmconnectbutton.h	\
	gmdialog.c		\
	gmdialog.h		\
	gmentrydialog.c		\
	gmentrydialog.h		\
	gmlevelmeter.c		\
	gmlevelmeter.h		\
	gmmenuaddon.c		\
	gmmenuaddon.h		\
	gmpreferences.c		\
	gmpreferences.h		\
	gmroster.cpp		\
	gmroster.h		\
	gmstatsdrawingarea.c	\
	gmstatsdrawingarea.h	\
	gmstatusbar.c		\
	gmstatusbar.h		\
	gmstockicons.c		\
	gmstockicons.h		\
	gmtextbufferaddon.c	\
	gmtextbufferaddon.h	\
	gmtexttagaddon.c	\
	gmtexttagaddon.h	\
	gmtextviewaddon.c	\
	gmtextviewaddon.h

INCLUDES =						\
	-DDATA_DIR=\""$(datadir)"\"			\
	-DG_LOG_DOMAIN=\"Ekiga\"		 	\
	-DSHARE_DIR=\"$(pkgdatadir)\"			\
	-I../gmcontacts					\
	-I../gmconf					\
	-I$(top_srcdir) 				\
	@EKIGA_CFLAGS@				

libgmwidgets_la_LDFLAGS = -export-dynamic
libgmwidgets_la_LIBADD = gmtray/libgmtray.la
