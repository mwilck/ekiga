noinst_LTLIBRARIES = libgmtray.la

libgmtray_la_SOURCES =		\
	gmtray.h		\
	gmtray-internal.h	\
	gmtray-common.c

if HAVE_GTK_2_10
libgmtray_la_SOURCES +=		\
	gmtray-gtk.c
else
if WIN32
libgmtray_la_SOURCES +=		\
	gmtray-win32.c		\
	pixbuf_to_hicon.h	\
	pixbuf_to_hicon.c
else
libgmtray_la_SOURCES +=		\
	gmtray-x11.c		\
	eggtrayicon.c		\
	eggtrayicon.h
endif
endif

AM_CFLAGS = $(GTK_CFLAGS) $(GLIB_CFLAGS)
AM_LIBS = $(GTK_LIBS) $(GLIB_LIBS)
