SUBDIRS = src plugins gui

noinst_LTLIBRARIES = libekiga_engine.la

libekiga_engine_la_SOURCES =

libekiga_engine_la_LDFLAGS = -export-dynamic
libekiga_engine_la_LIBADD = gui/searchbook/libekiga_gtk_searchbook.la 	\
			    plugins/evolution/libgmevolution.la 	\
			    src/framework/libgmframework.la		\
			    src/addressbook/libgmaddressbook.la
