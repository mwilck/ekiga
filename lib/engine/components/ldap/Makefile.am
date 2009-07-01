plugin_LTLIBRARIES = libgmldap.la

ldap_dir = $(top_srcdir)/lib/engine/components/ldap

AM_CPPFLAGS = $(LDAP_CFLAGS) $(SIGC_CFLAGS) $(GLIB_CFLAGS) $(XML_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/gmconf 			\
	-I$(top_srcdir)/lib/engine/framework 		\
	-I$(top_srcdir)/lib/engine/addressbook

libgmldap_la_SOURCES = \
	$(ldap_dir)/ldap-contact.h 	\
	$(ldap_dir)/ldap-contact.cpp 	\
	$(ldap_dir)/ldap-book.h 	\
	$(ldap_dir)/ldap-book.cpp 	\
	$(ldap_dir)/ldap-source.h 	\
	$(ldap_dir)/ldap-source.cpp 	\
	$(ldap_dir)/ldap-main.h 	\
	$(ldap_dir)/ldap-main.cpp

libgmldap_la_LDFLAGS = $(PLUGINS_LIBTOOL_FLAGS)
libgmldap_la_LIBADD =  $(LDAP_LIBS) $(SIGC_LIBS) $(GLIB_LIBS) $(XML_LIBS)
