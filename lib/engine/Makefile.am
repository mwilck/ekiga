SUBDIRS = src addressbook plugins gui protocol

noinst_LTLIBRARIES = libekiga_engine.la

INCLUDES = 								\
	-I$(top_srcdir)/lib/engine/	 				\
	-I$(top_srcdir)/lib/engine/gui/ 				\
	-I$(top_srcdir)/lib/engine/include/ 				\
	-I$(top_srcdir)/lib/engine/include/framework/	 		\
	-I$(top_srcdir)/lib/engine/protocol/sip				\
	-I$(top_srcdir)/lib/engine/plugins/				\
	-I$(top_srcdir)/lib/engine/addressbook/				\
	-I$(top_srcdir)/lib/engine/addressbook/evolution		\
	-I$(top_srcdir)/lib/engine/plugins/presence

libekiga_engine_la_SOURCES = engine.h engine.cpp

AM_CXXFLAGS = $(SIGC_CFLAGS) 
AM_LIBS = $(SIGC_LIBS) 

libekiga_engine_la_LDFLAGS = -export-dynamic
libekiga_engine_la_LIBADD = gui/libgmgui.la			 		\
			    protocol/sip/libgmsip.la				\
			    src/framework/libgmframework.la			\
			    src/addressbook/libgmaddressbook.la			\
			    src/presence/libgmpresence.la			\
			    plugins/presence/localroster/libgmlocalroster.la	\
			    $(AM_LIBS)

if HAVE_EDS
libekiga_engine_la_LIBADD += addressbook/evolution/libgmevolution.la
endif
