noinst_LTLIBRARIES = libgmaddressbook.la

addressbook_incdir = $(top_srcdir)/lib/engine/include/addressbook
addressbook_srcdir = $(top_srcdir)/lib/engine/src/addressbook

AM_CXXFLAGS = $(SIGC_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/engine/include \
	-I$(top_srcdir)/lib/engine/include/framework \
	-I$(top_srcdir)/lib/engine/include/addressbook 

addressbook_generics = \
	$(addressbook_incdir)/contact.h			\
	$(addressbook_incdir)/book.h			\
	$(addressbook_incdir)/book-impl.h		\
	$(addressbook_incdir)/source.h			\
	$(addressbook_incdir)/source-impl.h		\
	$(addressbook_incdir)/contact-core.h		\
	$(addressbook_srcdir)/contact-core.cpp


libgmaddressbook_la_SOURCES = \
	$(addressbook_generics)

libgmaddressbook_la_LDFLAGS = -export-dynamic -no-undefined $(SIGC_LIBS)
