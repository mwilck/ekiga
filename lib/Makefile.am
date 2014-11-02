stacklib_LTLIBRARIES = libekiga.la
stacklib_dir = $(stacklibdir)

libekiga_la_LDFLAGS = $(STACKLIB_LDFLAGS) $(GLIB_LIBS) $(XML_LIBS) $(GTK_LIBS)

if !WIN32
libekiga_la_LDFLAGS += -lX11
endif

CLEANFILES =

AM_CPPFLAGS = \
	$(BOOST_CPPFLAGS) $(GLIB_CFLAGS) $(XML_CFLAGS) $(GTK_CFLAGS) \
	$(DBUS_CFLAGS) $(PTLIB_CFLAGS) $(OPAL_CFLAGS) \
	-DSYSCONFDIR=\""$(sysconfdir)"\" \
	-DEKIGA_PLUGIN_DIR=\"$(PLUGIN_DIR)\" \
	-I$(top_srcdir) \
	-I$(top_srcdir)/lib \
	-I$(top_srcdir)/lib/gui \
	-I$(top_srcdir)/lib/platform \
	-I$(top_srcdir)/lib/settings \
	-I$(top_srcdir)/lib/engine \
	-I$(top_srcdir)/lib/engine/account \
	-I$(top_srcdir)/lib/engine/action \
	-I$(top_srcdir)/lib/engine/addressbook \
	-I$(top_srcdir)/lib/engine/audioinput \
	-I$(top_srcdir)/lib/engine/audiooutput \
	-I$(top_srcdir)/lib/engine/chat \
	-I$(top_srcdir)/lib/engine/framework \
	-I$(top_srcdir)/lib/engine/friend-or-foe \
	-I$(top_srcdir)/lib/engine/gui/gtk-core \
	-I$(top_srcdir)/lib/engine/gui/gtk-frontend \
	-I$(top_srcdir)/lib/engine/hal \
	-I$(top_srcdir)/lib/engine/notification \
	-I$(top_srcdir)/lib/engine/plugin \
	-I$(top_srcdir)/lib/engine/presence \
	-I$(top_srcdir)/lib/engine/protocol \
	-I$(top_srcdir)/lib/engine/videoinput \
	-I$(top_srcdir)/lib/engine/videooutput \
	-I$(top_srcdir)/lib/engine/components/call-history \
	-I$(top_srcdir)/lib/engine/components/echo \
	-I$(top_srcdir)/lib/engine/components/gmconf-personal-details \
	-I$(top_srcdir)/lib/engine/components/hal-dbus \
	-I$(top_srcdir)/lib/engine/components/mlogo-videoinput \
	-I$(top_srcdir)/lib/engine/components/null-audioinput \
	-I$(top_srcdir)/lib/engine/components/null-audiooutput \
	-I$(top_srcdir)/lib/engine/components/opal \
	-I$(top_srcdir)/lib/engine/components/ptlib

AM_CFLAGS = \
	$(GLIB_CFLAGS) $(GTK_CFLAGS) $(GCONF_CFLAGS) $(DBUS_CFLAGS)

AM_LIBS = $(GLIB_LIBS) $(XML_LIBS) $(GTK_LIBS)

##
# Sources out of this directory
##

libekiga_la_SOURCES =

EXTRA_DIST =

nodist_libekiga_la_SOURCES =

BUILT_SOURCES =

##
# Sources out of the platform directory
##

libekiga_la_SOURCES += \
	platform/platform.h \
	platform/platform.c

if WIN32
libekiga_la_SOURCES += platform/winpaths.h
endif


##
# Sources out of the settings directory
##

libekiga_la_SOURCES += \
	settings/settings-mappings.h \
        settings/settings-mappings.c \
	settings/ekiga-settings.h


##
# Sources out of the pixops directory
##
if !WIN32

AM_CPPFLAGS += -I$(top_srcdir)/lib/pixops
EXTRA_DIST += $(top_srcdir)/lib/pixops/README

libekiga_la_SOURCES += \
	pixops/pixops.h \
	pixops/pixops.c \
	pixops/pixops-internal.h

if USE_MMX
libekiga_la_SOURCES += \
	pixops/have_mmx.S \
	pixops/scale_line_22_33_mmx.S \
	pixops/composite_line_22_4a4_mmx.S \
	pixops/composite_line_color_22_4a4_mmx.S
endif

endif

##
# Sources out of the gui directory
# (those should probably be moved elsewhere like engine/gui/gtk-core/ )
##
libekiga_la_SOURCES += \
	gui/gmvideowidget.c \
	gui/gmvideowidget.h \
	gui/dialpad.h \
	gui/dialpad.c \
	gui/gm-smileys.h \
	gui/gm-smileys.c \
	gui/gm-entry.h \
	gui/gm-entry.c \
	gui/gmwindow.c \
	gui/gmwindow.h \
	gui/gmmenuaddon.c \
	gui/gmmenuaddon.h \
	gui/gmstatusbar.c \
	gui/gmstatusbar.h \
	gui/gmstockicons.c \
	gui/gmstockicons.h \
	gui/gmpowermeter.c \
	gui/gmpowermeter.h \
	gui/gm-cell-renderer-expander.c \
	gui/gm-cell-renderer-expander.h \
	gui/gm-cell-renderer-bitext.c \
	gui/gm-cell-renderer-bitext.h \
	gui/gm-text-buffer-enhancer-helper-interface.c \
	gui/gm-text-buffer-enhancer-helper-interface.h \
	gui/gm-text-anchored-tag.c \
	gui/gm-text-anchored-tag.h \
	gui/gm-text-smiley.c \
	gui/gm-text-smiley.h \
	gui/gm-text-extlink.c \
	gui/gm-text-extlink.h \
	gui/gm-text-buffer-enhancer.c \
	gui/gm-text-buffer-enhancer.h \
	gui/gm-smiley-chooser-button.c \
	gui/gm-smiley-chooser-button.h

if !HAVE_SIDEBAR
libekiga_la_SOURCES += \
	gui/gm-sidebar.h \
	gui/gm-sidebar.c
endif

##
# Sources of the main engine code
##

libekiga_la_SOURCES += \
	engine/engine.h \
	engine/engine.cpp

##
# Sources of the engine framework code
##

libekiga_la_SOURCES += \
	engine/framework/boost-exceptions.cpp \
	engine/framework/services.h \
	engine/framework/null-deleter.h \
	engine/framework/map-key-iterator.h \
	engine/framework/map-key-const-iterator.h \
	engine/framework/reflister.h \
	engine/framework/chain-of-responsibility.h \
	engine/framework/device-def.h \
	engine/framework/form-builder.h \
	engine/framework/form-dumper.h \
	engine/framework/form.h \
	engine/framework/form.cpp \
	engine/framework/form-request.h \
	engine/framework/form-request-simple.h \
	engine/framework/robust-xml.h \
	engine/framework/robust-xml.cpp \
	engine/framework/form-visitor.h \
	engine/framework/runtime.h \
	engine/framework/menu-builder.h \
	engine/framework/menu-builder.cpp \
	engine/framework/menu-builder-tools.h \
	engine/framework/menu-builder-tools.cpp \
	engine/framework/form-builder.cpp \
	engine/framework/form-dumper.cpp \
	engine/framework/form-request-simple.cpp \
	engine/framework/runtime-glib.cpp \
	engine/framework/services.cpp \
	engine/framework/trigger.h \
	engine/framework/menu-xml.h \
	engine/framework/menu-xml.cpp \
	engine/framework/kickstart.h \
	engine/framework/kickstart.cpp \
	engine/framework/personal-details.h \
	engine/framework/ptr_array.h \
	engine/framework/ptr_array_iterator.h \
	engine/framework/ptr_array_const_iterator.h \
	engine/framework/live-object.h \
	engine/framework/filterable.h \
	engine/framework/scoped-connections.h

##
# Sources of the plugin loader code
##

libekiga_la_SOURCES += \
	engine/plugin/plugin-core.h \
	engine/plugin/plugin-core.cpp

##
# Sources of the account stack
##

libekiga_la_SOURCES += \
	engine/account/account.h \
	engine/account/bank.h \
	engine/account/bank-impl.h \
	engine/account/account-core.h \
	engine/account/account-core.cpp

##
# Actions
##

libekiga_la_SOURCES += \
	engine/action/action.h \
	engine/action/action.cpp \
	engine/action/action-provider.h \
	engine/action/action-provider.cpp \
	engine/action/actor.h \
	engine/action/actor.cpp

##
#  Sources of the addressbook stack
##

libekiga_la_SOURCES += \
	engine/addressbook/contact.h \
	engine/addressbook/book.h \
	engine/addressbook/book-impl.h \
	engine/addressbook/source.h \
	engine/addressbook/source-impl.h \
	engine/addressbook/contact-core.h \
	engine/addressbook/contact-core.cpp

##
# Sources of the chat stack
##

libekiga_la_SOURCES += \
	engine/chat/chat-core.h \
	engine/chat/chat-core.cpp \
	engine/chat/dialect.h \
	engine/chat/dialect-impl.h \
	engine/chat/chat.h \
	engine/chat/chat-simple.h \
	engine/chat/chat-multiple.h

##
# Sources of the friend-or-foe stack
##

libekiga_la_SOURCES += \
	engine/friend-or-foe/friend-or-foe.h \
	engine/friend-or-foe/friend-or-foe.cpp

##
# Sources of the notification stack
##

libekiga_la_SOURCES += \
	engine/notification/notification-core.h \
	engine/notification/notification-dummy.cpp

##
# Sources of the presence stack
##

libekiga_la_SOURCES += \
	engine/presence/presentity.h \
	engine/presence/heap.h \
	engine/presence/heap-impl.h \
	engine/presence/cluster.h \
	engine/presence/cluster-impl.h \
	engine/presence/uri-presentity.h \
	engine/presence/proxy-presentity.cpp \
	engine/presence/proxy-presentity.h \
	engine/presence/uri-presentity.cpp \
	engine/presence/presence-core.h \
	engine/presence/presence-core.cpp

##
# Sources of the protocol stack
##

libekiga_la_SOURCES += \
	engine/protocol/call-core.h \
	engine/protocol/call-manager.h \
	engine/protocol/call-manager.cpp \
	engine/protocol/call.h \
	engine/protocol/call-core.cpp \
	engine/protocol/call-protocol-manager.h \
	engine/protocol/codec-description.h \
	engine/protocol/codec-description.cpp

##
# Sources of the video output stack
##

libekiga_la_SOURCES += \
	engine/videooutput/videooutput-info.h \
	engine/videooutput/videooutput-manager.h \
	engine/videooutput/videooutput-core.h \
	engine/videooutput/videooutput-core.cpp

##
# Sources of the video input stack
##

libekiga_la_SOURCES += \
	engine/videoinput/videoinput-manager.h \
	engine/videoinput/videoinput-info.h \
	engine/videoinput/videoinput-core.h \
	engine/videoinput/videoinput-core.cpp

##
# Sources of the audio output stack
##

libekiga_la_SOURCES += \
	engine/audiooutput/audiooutput-manager.h \
	engine/audiooutput/audiooutput-info.h \
	engine/audiooutput/audiooutput-scheduler.h \
	engine/audiooutput/audiooutput-scheduler.cpp \
	engine/audiooutput/audiooutput-core.h \
	engine/audiooutput/audiooutput-core.cpp

##
# Sources of the audio input stack
##

libekiga_la_SOURCES += \
	engine/audioinput/audioinput-manager.h	\
	engine/audioinput/audioinput-info.h	\
	engine/audioinput/audioinput-core.h	\
	engine/audioinput/audioinput-core.cpp

##
# Sources of the hardware abstraction layer (HAL) stack
##

libekiga_la_SOURCES += \
	engine/hal/hal-manager.h \
	engine/hal/hal-core.h \
	engine/hal/hal-core.cpp

##
# Sources of the gtk+ core
##

libekiga_la_SOURCES += \
	engine/gui/gtk-core/menu-builder-gtk.h \
	engine/gui/gtk-core/menu-builder-gtk.cpp \
	engine/gui/gtk-core/form-dialog-gtk.h \
	engine/gui/gtk-core/form-dialog-gtk.cpp \
	engine/gui/gtk-core/gactor-menu.h \
	engine/gui/gtk-core/gactor-menu.cpp \
	engine/gui/gtk-core/optional-buttons-gtk.h \
	engine/gui/gtk-core/optional-buttons-gtk.cpp \
	engine/gui/gtk-core/codecsbox.cpp \
	engine/gui/gtk-core/codecsbox.h \
	engine/gui/gtk-core/gtk-core.h \
	engine/gui/gtk-core/gtk-core.cpp \
	engine/gui/gtk-core/gtk-core-main.h \
	engine/gui/gtk-core/gtk-core-main.cpp

##
# Sources of the gtk+ frontend
##

libekiga_la_SOURCES += \
	engine/gui/gtk-frontend/addressbook-window.h \
	engine/gui/gtk-frontend/addressbook-window.cpp \
	engine/gui/gtk-frontend/accounts-window.h \
	engine/gui/gtk-frontend/accounts-window.cpp \
	engine/gui/gtk-frontend/assistant-window.h \
	engine/gui/gtk-frontend/assistant-window.cpp \
	engine/gui/gtk-frontend/main_window.h \
	engine/gui/gtk-frontend/main_window.cpp \
	engine/gui/gtk-frontend/book-view-gtk.h \
	engine/gui/gtk-frontend/book-view-gtk.cpp \
	engine/gui/gtk-frontend/call-window.h \
	engine/gui/gtk-frontend/call-window.cpp \
	engine/gui/gtk-frontend/roster-view-gtk.h \
	engine/gui/gtk-frontend/roster-view-gtk.cpp \
	engine/gui/gtk-frontend/call-history-view-gtk.h \
	engine/gui/gtk-frontend/call-history-view-gtk.cpp \
	engine/gui/gtk-frontend/default_devices.cpp \
	engine/gui/gtk-frontend/default_devices.h \
	engine/gui/gtk-frontend/presentity-view.h \
	engine/gui/gtk-frontend/presentity-view.cpp \
	engine/gui/gtk-frontend/heap-view.h \
	engine/gui/gtk-frontend/heap-view.cpp \
	engine/gui/gtk-frontend/chat-area.h \
	engine/gui/gtk-frontend/chat-area.cpp \
	engine/gui/gtk-frontend/simple-chat-page.h \
	engine/gui/gtk-frontend/simple-chat-page.cpp \
	engine/gui/gtk-frontend/multiple-chat-page.h \
	engine/gui/gtk-frontend/multiple-chat-page.cpp \
	engine/gui/gtk-frontend/preferences-window.cpp \
	engine/gui/gtk-frontend/preferences-window.h \
	engine/gui/gtk-frontend/statusicon.cpp \
	engine/gui/gtk-frontend/statusicon.h \
	engine/gui/gtk-frontend/statusmenu.h \
	engine/gui/gtk-frontend/statusmenu.cpp \
	engine/gui/gtk-frontend/chat-window.h \
	engine/gui/gtk-frontend/chat-window.cpp \
	engine/gui/gtk-frontend/ekiga-app.h \
	engine/gui/gtk-frontend/ekiga-app.cpp

##
# Sources of the ptlib component
##

libekiga_la_SOURCES += \
	engine/components/ptlib/utils.h \
	engine/components/ptlib/utils.cpp \
	engine/components/ptlib/audioinput-manager-ptlib.h \
	engine/components/ptlib/audioinput-manager-ptlib.cpp \
	engine/components/ptlib/audioinput-main-ptlib.h \
	engine/components/ptlib/audioinput-main-ptlib.cpp \
	engine/components/ptlib/videoinput-manager-ptlib.h \
	engine/components/ptlib/videoinput-manager-ptlib.cpp \
	engine/components/ptlib/videoinput-main-ptlib.h \
	engine/components/ptlib/videoinput-main-ptlib.cpp \
	engine/components/ptlib/audiooutput-manager-ptlib.h \
	engine/components/ptlib/audiooutput-manager-ptlib.cpp \
	engine/components/ptlib/audiooutput-main-ptlib.h \
	engine/components/ptlib/audiooutput-main-ptlib.cpp

libekiga_la_LDFLAGS += $(PTLIB_LIBS)

##
# Sources of the opal component
##

libekiga_la_SOURCES += \
	engine/components/opal/opal-call-manager.h \
	engine/components/opal/opal-call-manager.cpp \
	engine/components/opal/pcss-endpoint.h \
	engine/components/opal/pcss-endpoint.cpp \
	engine/components/opal/opal-account.h \
	engine/components/opal/opal-account.cpp \
	engine/components/opal/opal-bank.h \
	engine/components/opal/opal-bank.cpp \
	engine/components/opal/opal-call.h \
	engine/components/opal/opal-call.cpp \
	engine/components/opal/opal-codec-description.h \
	engine/components/opal/opal-codec-description.cpp \
	engine/components/opal/opal-main.h \
	engine/components/opal/opal-main.cpp \
	engine/components/opal/opal-audio.h \
	engine/components/opal/opal-audio.cpp \
	engine/components/opal/opal-videoinput.h \
	engine/components/opal/opal-videoinput.cpp \
	engine/components/opal/opal-videooutput.h \
	engine/components/opal/opal-videooutput.cpp \
	engine/components/opal/opal-plugins-hook.h \
	engine/components/opal/opal-plugins-hook.cpp \
	engine/components/opal/opal-presentity.h \
	engine/components/opal/opal-presentity.cpp \
	engine/components/opal/opal-process.h \
	engine/components/opal/opal-process.cpp

if HAVE_H323
libekiga_la_SOURCES += \
	engine/components/opal/h323-endpoint.h \
	engine/components/opal/h323-endpoint.cpp
endif

libekiga_la_SOURCES += \
	engine/components/opal/sip-chat-simple.h \
	engine/components/opal/sip-chat-simple.cpp \
	engine/components/opal/sip-dialect.h \
	engine/components/opal/sip-dialect.cpp \
	engine/components/opal/sip-endpoint.h \
	engine/components/opal/sip-endpoint.cpp

libekiga_la_LDFLAGS += $(OPAL_LIBS)

##
# sources of the call history component
##

libekiga_la_SOURCES += \
	engine/components/call-history/history-contact.h \
	engine/components/call-history/history-contact.cpp \
	engine/components/call-history/history-book.h \
	engine/components/call-history/history-book.cpp \
	engine/components/call-history/history-source.h \
	engine/components/call-history/history-source.cpp \
	engine/components/call-history/history-main.h \
	engine/components/call-history/history-main.cpp

##
# Sources of the text chat echo component
##

libekiga_la_SOURCES += \
	engine/components/echo/echo-presentity.h \
	engine/components/echo/echo-presentity.cpp \
	engine/components/echo/echo-simple.h \
	engine/components/echo/echo-simple.cpp \
	engine/components/echo/echo-dialect.h \
	engine/components/echo/echo-dialect.cpp \
	engine/components/echo/echo-main.h \
	engine/components/echo/echo-main.cpp

##
# Sources of the gmconf personal details component
##

libekiga_la_SOURCES += \
	engine/components/gmconf-personal-details/gmconf-personal-details.cpp \
	engine/components/gmconf-personal-details/gmconf-personal-details.h


##
# Sources of the moving logo component
##

libekiga_la_SOURCES += \
	engine/components/mlogo-videoinput/videoinput-manager-mlogo.h \
	engine/components/mlogo-videoinput/videoinput-manager-mlogo.cpp \
	engine/components/mlogo-videoinput/videoinput-main-mlogo.h \
	engine/components/mlogo-videoinput/videoinput-main-mlogo.cpp

##
# Sources of the null audio input component
##

libekiga_la_SOURCES += \
	engine/components/null-audioinput/audioinput-manager-null.h \
	engine/components/null-audioinput/audioinput-manager-null.cpp \
	engine/components/null-audioinput/audioinput-main-null.h \
	engine/components/null-audioinput/audioinput-main-null.cpp

##
# Sources of the null audio output component
##

libekiga_la_SOURCES += \
	engine/components/null-audiooutput/audiooutput-manager-null.h \
	engine/components/null-audiooutput/audiooutput-manager-null.cpp \
	engine/components/null-audiooutput/audiooutput-main-null.h \
	engine/components/null-audiooutput/audiooutput-main-null.cpp

##
# Sources of the hal dbus component
##
if HAVE_DBUS

libekiga_la_SOURCES += \
	engine/components/hal-dbus/hal-manager-dbus.h \
	engine/components/hal-dbus/hal-manager-dbus.cpp \
	engine/components/hal-dbus/hal-main-dbus.h \
	engine/components/hal-dbus/hal-main-dbus.cpp

if HAVE_V4L
libekiga_la_SOURCES += \
	engine/components/hal-dbus/hal-v4l-helper.h \
	engine/components/hal-dbus/hal-v4l-helper.c
endif

nodist_libekiga_la_SOURCES += \
	engine/components/hal-dbus/hal-marshal.h \
	engine/components/hal-dbus/hal-marshal.c

libekiga_la_LDFLAGS += $(DBUS_LIBS)

# forces the HAVE :
BUILT_SOURCES += engine/components/hal-dbus/hal-marshal.h engine/components/hal-dbus/hal-marshal.c
CLEANFILES += engine/components/hal-dbus/hal-marshal.h engine/components/hal-dbus/hal-marshal.c
EXTRA_DIST += engine/components/hal-dbus/hal-marshal.list

engine/components/hal-dbus/hal-marshal.h: engine/components/hal-dbus/hal-marshal.list
	$(AM_V_GEN)$(LIBTOOL) --mode=execute glib-genmarshal --prefix=gm_hal_dbus_marshal $^ --header > $@.tmp && mv $@.tmp $@

engine/components/hal-dbus/hal-marshal.c: engine/components/hal-dbus/hal-marshal.list
	$(AM_V_GEN)$(LIBTOOL) --mode=execute glib-genmarshal --prefix=gm_hal_dbus_marshal $^ --body > $@.tmp && mv $@.tmp $@

endif

##
# Support for shared memory
##
if HAVE_SHM

libekiga_la_LDFLAGS += $(SHM_LIBS)

endif

##
# Sources of the Clutter video output component
##

AM_CPPFLAGS += $(CLUTTER_CFLAGS) $(CLUTTER_GTK_CFLAGS) $(CLUTTER_GST_CFLAGS) \
	-I$(top_srcdir)/lib/engine/components/clutter-gst-videooutput

libekiga_la_SOURCES += \
	engine/components/clutter-gst-videooutput/videooutput-manager-clutter-gst.cpp   \
	engine/components/clutter-gst-videooutput/videooutput-manager-clutter-gst.h     \
	engine/components/clutter-gst-videooutput/videooutput-main-clutter-gst.cpp      \
	engine/components/clutter-gst-videooutput/videooutput-main-clutter-gst.h

libekiga_la_LDFLAGS += $(CLUTTER_LIBS) $(CLUTTER_GTK_LIBS) $(CLUTTER_GST_LIBS)
