noinst_LTLIBRARIES = libgmplatform.la

INCLUDES = -I$(top_srcdir)/lib \
           $(EKIGA_LIB_CFLAGS)

libgmplatform_la_SOURCES = 

if WIN32
libgmplatform_la_SOURCES += platform-win32.c winpaths.h
else
libgmplatform_la_SOURCES += platform-generic.c
endif

libgmplatform_la_LDFLAGS = -export-dynamic -no-undefined
