noinst_LTLIBRARIES = libgmbase.la

INCLUDES = -I$(top_srcdir)/lib -I$(top_srcdir)/plugins \
           $(EKIGA_LIB_CFLAGS)

libgmbase_la_SOURCES = gm-object.c         \
                       gm-plugin-manager.c \
		       gm-services.c

libgmbase_la_LDFLAGS = -export-dynamic -no-undefined
