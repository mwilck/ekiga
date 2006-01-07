lib_LTLIBRARIES = libgmtray.la

libgmtray_la_SOURCES =		\
	gmtray.h		\
	gmtray-internal.h	\
	gmtray-common.c

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

INCLUDES = $(GNOMEMEETING_LIB_CFLAGS)