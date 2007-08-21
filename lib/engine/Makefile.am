SUBDIRS = framework addressbook presence gui protocol

noinst_LTLIBRARIES = libekiga_engine.la

INCLUDES = 								\
	-I$(top_srcdir)/lib/engine/	 				\
	-I$(top_srcdir)/lib/engine/gui/gtk-core				\
	-I$(top_srcdir)/lib/engine/gui/gtk-frontend			\
	-I$(top_srcdir)/lib/engine/framework/		 		\
	-I$(top_srcdir)/lib/engine/protocol/sip				\
	-I$(top_srcdir)/lib/engine/addressbook/skel			\
	-I$(top_srcdir)/lib/engine/addressbook/evolution		\
	-I$(top_srcdir)/lib/engine/addressbook/ldap			\
	-I$(top_srcdir)/lib/engine/presence/skel			\
	-I$(top_srcdir)/lib/engine/presence/local-roster

libekiga_engine_la_SOURCES = engine.h engine.cpp

AM_CXXFLAGS = $(SIGC_CFLAGS) $(GTK_CFLAGS)
AM_LIBS = $(SIGC_LIBS) $(GTK_LIBS)

libekiga_engine_la_LDFLAGS = -export-dynamic
libekiga_engine_la_LIBADD = \
	$(top_builddir)/lib/engine/gui/gtk-core/libgmgtk-core.la \
	$(top_builddir)/lib/engine/gui/gtk-frontend/libgmgtk-frontend.la \
	$(top_builddir)/lib/engine/protocol/sip/libgmsip.la \
	$(top_builddir)/lib/engine/framework/libgmframework.la \
	$(top_builddir)/lib/engine/addressbook/skel/libgmaddressbook.la \
	$(top_builddir)/lib/engine/presence/skel/libgmpresence.la \
	$(top_builddir)/lib/engine/presence/local-roster/liblocal-roster.la \
	$(AM_LIBS)

if HAVE_EDS
libekiga_engine_la_LIBADD += addressbook/evolution/libgmevolution.la
endif

if HAVE_LDAP
libekiga_engine_la_LIBADD += addressbook/ldap/libgmldap.la
endif
