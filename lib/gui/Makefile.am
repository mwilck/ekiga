SUBDIRS = gmtray

noinst_LTLIBRARIES = libgmwidgets.la

libgmwidgets_la_SOURCES =	\
	gmwindow.c		\
	gmwindow.h		\
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
	gmstatusbar.c		\
	gmstatusbar.h		\
	gmstockicons.c		\
	gmstockicons.h		\
	gmtextbufferaddon.c	\
	gmtextbufferaddon.h	\
	gmtexttagaddon.c	\
	gmtexttagaddon.h	\
	gmtextviewaddon.c	\
	gmtextviewaddon.h	\
	gmpowermeter.c		\
	gmpowermeter.h		\
	gmgroupseditor.c	\
	gmgroupseditor.h	\
	gmcellrendererexpander.c  \
	gmcellrendererexpander.h  \
	gm-cell-renderer-bitext.c \
	gm-cell-renderer-bitext.h


if HAVE_XV
libgmwidgets_la_SOURCES +=	\
	xvwindow.cpp		\
	xvwindow.h
endif

if HAVE_DX
libgmwidgets_la_SOURCES +=	\
	dxwindow.cpp		\
	dxwindow.h
endif

INCLUDES =						\
	-DDATA_DIR=\""$(datadir)"\"			\
	-I$(top_srcdir)/lib/gmcontacts			\
	-I$(top_srcdir)/lib/gmconf			\
	-I$(top_srcdir)/lib				\
	-I$(top_srcdir)					\
	-I$(top_builddir)

AM_CXXFLAGS = $(GTK_CFLAGS) $(GLIB_CFLAGS) $(OPAL_CFLAGS) $(PWLIB_CFLAGS) $(XV_CFLAGS) $(DX_CFLAGS)
AM_CFLAGS = $(GTK_CFLAGS) $(GLIB_CFLAGS) $(GNOME_CFLAGS)
AM_LIBS = $(GTK_LIBS) $(GLIB_LIBS) $(OPAL_LIBS) $(PWLIB_LIBS) $(XV_LIBS) $(DX_LIBS) $(GNOME_LIBS)

libgmwidgets_la_LDFLAGS = -export-dynamic
libgmwidgets_la_LIBADD = gmtray/libgmtray.la $(AM_LIBS)
