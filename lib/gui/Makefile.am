noinst_LTLIBRARIES = libgmwidgets.la


libgmwidgets_la_SOURCES =	\
	esplash.c		\
	esplash.h		\
	gmconfwidgets.c		\
	gmconfwidgets.h		\
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
	-DG_LOG_DOMAIN=\"GnomeMeeting\"		 	\
	-DSHARE_DIR=\"$(pkgdatadir)\"			\
	-I../gmconf					\
	-I$(top_srcdir) 				\
	$(GNOMEMEETING_LIB_CFLAGS)

libgmwidgets_la_LDFLAGS = -export-dynamic
libgmwidgets_la_LIBADD =
