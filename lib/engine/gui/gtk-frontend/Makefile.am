noinst_LTLIBRARIES = libgmgtk-frontend.la

gtk_frontend_dir = $(top_srcdir)/lib/engine/gui/gtk-frontend

AM_CPPFLAGS = $(SIGC_CFLAGS) $(GTK_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/gui \
	-I$(top_srcdir)/lib/gmconf \
	-I$(top_srcdir)/lib/engine/framework \
	-I$(top_srcdir)/lib/engine/presence/skel \
	-I$(top_srcdir)/lib/engine/addressbook/skel \
	-I$(top_srcdir)/lib/engine/gui/gtk-core \
	-I$(top_srcdir)/lib/engine/protocol/sip


#### FIXME should be removed at a later stage
INCLUDES += \
	-I$(top_srcdir)/src                             \
	-I$(top_srcdir)/src/clients/                    \
	-I$(top_srcdir)/src/components/                 \
	-I$(top_srcdir)/src/devices/                    \
	-I$(top_srcdir)/src/endpoints/                  \
	-I$(top_srcdir)/src/gui/                  

AM_CPPFLAGS += $(GTK_CFLAGS) $(GLIB_CFLAGS) $(OPAL_CFLAGS) $(PTLIB_CFLAGS)

libgmgtk_frontend_la_SOURCES = \
	$(gtk_frontend_dir)/addressbook-window.h \
	$(gtk_frontend_dir)/addressbook-window.cpp \
	$(gtk_frontend_dir)/book-view-gtk.h \
	$(gtk_frontend_dir)/book-view-gtk.cpp \
	$(gtk_frontend_dir)/roster-view-gtk.h \
	$(gtk_frontend_dir)/roster-view-gtk.cpp \
	$(gtk_frontend_dir)/chat-window.h \
	$(gtk_frontend_dir)/chat-window.cpp \
	$(gtk_frontend_dir)/chat-window-page.h \
	$(gtk_frontend_dir)/chat-window-page.cpp \
	$(gtk_frontend_dir)/gtk-frontend.h \
	$(gtk_frontend_dir)/gtk-frontend.cpp

libgmgtk_frontend_la_LIBADD = \
	$(top_builddir)/lib/gui/libgmwidgets.la \
	$(top_builddir)/lib/engine/presence/skel/libgmpresence.la \
	$(top_builddir)/lib/engine/addressbook/skel/libgmaddressbook.la \
	$(top_builddir)/lib/engine/gui/gtk-core/libgmgtk-core.la

libgmgtk_frontend_la_LDFLAGS = -export-dynamic -no-undefined $(GTK_LIBS)

libgmgtk_frontend_la_LDFLAGS += $(GTK_LIBS) $(GLIB_LIBS) $(OPAL_LIBS) $(PTLIB_LIBS)
#### end

