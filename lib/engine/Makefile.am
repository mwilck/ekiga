SUBDIRS = src plugins gui

noinst_LTLIBRARIES = libekiga_engine.la

INCLUDES = 							\
	-I$(top_srcdir)/lib/engine/	 			\
	-I$(top_srcdir)/lib/engine/gui/ 			\
	-I$(top_srcdir)/lib/engine/include/ 			\
	-I$(top_srcdir)/lib/engine/include/framework/	 	\
	-I$(top_srcdir)/lib/engine/plugins/

libekiga_engine_la_SOURCES = include/engine.h 			\
			     engine.cpp

AM_CXXFLAGS = $(SIGC_CFLAGS) 
AM_LIBS = $(SIGC_LIBS) 

libekiga_engine_la_LDFLAGS = -export-dynamic
libekiga_engine_la_LIBADD = gui/searchbook/libekiga_gtk_searchbook.la 	\
			    src/framework/libgmframework.la		\
			    src/addressbook/libgmaddressbook.la		\
			    $(AM_LIBS)

if HAVE_EDS
libekiga_engine_la_LIBADD += plugins/evolution/libgmevolution.la
endif
