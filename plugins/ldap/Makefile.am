plugin_LTLIBRARIES = libgmldap.la

AM_CPPFLAGS = \
	$(LDAP_CFLAGS) $(BOOST_CPPFLAGS) $(GLIB_CFLAGS) $(XML_CFLAGS) \
	-I$(top_srcdir)/lib/ 			\
	-I$(top_srcdir)/lib/settings    	\
	-I$(top_srcdir)/lib/engine/framework 	\
	-I$(top_srcdir)/lib/engine/addressbook

libgmldap_la_SOURCES = \
	ldap-contact.h 	\
	ldap-contact.cpp 	\
	ldap-book.h 	\
	ldap-book.cpp 	\
	ldap-source.h 	\
	ldap-source.cpp 	\
	ldap-main.h 	\
	ldap-main.cpp

libgmldap_la_LDFLAGS = $(PLUGINS_LIBTOOL_FLAGS)

libgmldap_la_LIBADD = \
	$(top_builddir)/lib/libekiga.la \
	$(LDAP_LIBS) $(BOOST_LDFLAGS) $(GLIB_LIBS) $(XML_LIBS)
