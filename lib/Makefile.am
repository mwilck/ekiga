stacklib_LTLIBRARIES = libekiga.la
stacklib_dir = $(stacklibdir)

libekiga_la_SOURCES =
libekiga_la_LDFLAGS = $(STACKLIB_LDFLAGS) $(BOOST_LIBS) $(GLIB_LIBS) $(XML_LIBS) $(GTK_LIBS)

if !WIN32
libekiga_la_LDFLAGS += -lX11
endif

CLEANFILES =

INCLUDES = \
	-DSYSCONFDIR=\""$(sysconfdir)"\" \
	-DEKIGA_PLUGIN_DIR=\"$(PLUGIN_DIR)\" \
	-I$(top_srcdir) \
	-I$(top_srcdir)/lib \
	-I$(top_srcdir)/lib/gmconf \
	-I$(top_srcdir)/lib/gui \
	-I$(top_srcdir)/lib/engine \
	-I$(top_srcdir)/lib/engine/account \
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
	-I$(top_srcdir)/lib/engine/components/common-videooutput \
	-I$(top_srcdir)/lib/engine/components/dx-videooutput \
	-I$(top_srcdir)/lib/engine/components/echo \
	-I$(top_srcdir)/lib/engine/components/gmconf-personal-details \
	-I$(top_srcdir)/lib/engine/components/hal-dbus \
	-I$(top_srcdir)/lib/engine/components/local-roster \
	-I$(top_srcdir)/lib/engine/components/mlogo-videoinput \
	-I$(top_srcdir)/lib/engine/components/null-audioinput \
	-I$(top_srcdir)/lib/engine/components/null-audiooutput \
	-I$(top_srcdir)/lib/engine/components/opal \
	-I$(top_srcdir)/lib/engine/components/ptlib

AM_CFLAGS = \
	$(GLIB_CFLAGS) $(GTK_CFLAGS) $(GCONF_CFLAGS) $(DBUS_CFLAGS)

AM_CPPFLAGS = \
	$(BOOST_CPPFLAGS) $(GLIB_CFLAGS) $(XML_CFLAGS) $(GTK_CFLAGS) \
	$(DBUS_CFLAGS) $(PTLIB_CFLAGS) $(OPAL_CFLAGS)
AM_LIBS = \
	$(BOOST_LIBS) $(GLIB_LIBS) $(XML_LIBS) $(GTK_LIBS)

engine_dir = $(top_srcdir)/lib/engine
components_dir = $(top_srcdir)/lib/engine/components

##
# Sources out of this directory
##

libekiga_la_SOURCES += \
	$(top_srcdir)/lib/toolbox.h \
	$(top_srcdir)/lib/toolbox.c

EXTRA_DIST =

nodist_libekiga_la_SOURCES =

BUILT_SOURCES =

##
# Sources out of the platform directory
##

libekiga_la_SOURCES += \
	$(top_srcdir)/lib/platform/platform.h \
	$(top_srcdir)/lib/platform/platform.c

if WIN32
libekiga_la_SOURCES += $(top_srcdir)/lib/platform/winpaths.h
endif

##
# Sources out of the pixops directory
##
if !WIN32

INCLUDES += -I$(top_srcdir)/lib/pixops
EXTRA_DIST += $(top_srcdir)/lib/pixops/README

libekiga_la_SOURCES += \
	$(top_srcdir)/lib/pixops/pixops.h \
	$(top_srcdir)/lib/pixops/pixops.c \
	$(top_srcdir)/lib/pixops/pixops-internal.h

if USE_MMX
libekiga_la_SOURCES += \
	$(top_srcdir)/lib/pixops/have_mmx.S \
	$(top_srcdir)/lib/pixops/scale_line_22_33_mmx.S \
	$(top_srcdir)/lib/pixops/composite_line_22_4a4_mmx.S \
	$(top_srcdir)/lib/pixops/composite_line_color_22_4a4_mmx.S
endif

endif

##
# Sources out of the gmconf directory
##
libekiga_la_SOURCES += \
	$(top_srcdir)/lib/gmconf/gmconf.h \
	$(top_srcdir)/lib/gmconf/gmconf-ekiga-keys.h

if HAVE_GCONF
libekiga_la_SOURCES += $(top_srcdir)/lib/gmconf/gmconf-gconf.c
libekiga_la_LDFLAGS += $(GCONF_LIBS)
else
libekiga_la_SOURCES += $(top_srcdir)/lib/gmconf/gmconf-glib.c
endif

##
# Sources out of the gui directory
# (those should probably be moved elsewhere like lib/engine/gui/gtk-core/ )
##
libekiga_la_SOURCES += \
	$(top_srcdir)/lib/gui/dialpad.h \
	$(top_srcdir)/lib/gui/dialpad.c \
	$(top_srcdir)/lib/gui/gm-smileys.h \
	$(top_srcdir)/lib/gui/gm-smileys.c \
	$(top_srcdir)/lib/gui/gmwindow.c \
	$(top_srcdir)/lib/gui/gmwindow.h \
	$(top_srcdir)/lib/gui/gmcallbacks.c \
	$(top_srcdir)/lib/gui/gmcallbacks.h \
	$(top_srcdir)/lib/gui/gmconfwidgets.c \
	$(top_srcdir)/lib/gui/gmconfwidgets.h \
	$(top_srcdir)/lib/gui/gmdialog.c \
	$(top_srcdir)/lib/gui/gmdialog.h \
	$(top_srcdir)/lib/gui/gmentrydialog.c \
	$(top_srcdir)/lib/gui/gmentrydialog.h \
	$(top_srcdir)/lib/gui/gmlevelmeter.c \
	$(top_srcdir)/lib/gui/gmlevelmeter.h \
	$(top_srcdir)/lib/gui/gmmenuaddon.c \
	$(top_srcdir)/lib/gui/gmmenuaddon.h \
	$(top_srcdir)/lib/gui/gmpreferences.c \
	$(top_srcdir)/lib/gui/gmpreferences.h \
	$(top_srcdir)/lib/gui/gmstatusbar.c \
	$(top_srcdir)/lib/gui/gmstatusbar.h \
	$(top_srcdir)/lib/gui/gmstockicons.c \
	$(top_srcdir)/lib/gui/gmstockicons.h \
	$(top_srcdir)/lib/gui/gmpowermeter.c \
	$(top_srcdir)/lib/gui/gmpowermeter.h \
	$(top_srcdir)/lib/gui/gmcellrendererexpander.c \
	$(top_srcdir)/lib/gui/gmcellrendererexpander.h \
	$(top_srcdir)/lib/gui/gm-cell-renderer-bitext.c \
	$(top_srcdir)/lib/gui/gm-cell-renderer-bitext.h \
	$(top_srcdir)/lib/gui/gm-text-buffer-enhancer-helper-interface.c \
	$(top_srcdir)/lib/gui/gm-text-buffer-enhancer-helper-interface.h \
	$(top_srcdir)/lib/gui/gm-text-anchored-tag.c \
	$(top_srcdir)/lib/gui/gm-text-anchored-tag.h \
	$(top_srcdir)/lib/gui/gm-text-smiley.c \
	$(top_srcdir)/lib/gui/gm-text-smiley.h \
	$(top_srcdir)/lib/gui/gm-text-extlink.c \
	$(top_srcdir)/lib/gui/gm-text-extlink.h \
	$(top_srcdir)/lib/gui/gm-text-buffer-enhancer.c \
	$(top_srcdir)/lib/gui/gm-text-buffer-enhancer.h \
	$(top_srcdir)/lib/gui/gm-smiley-chooser-button.c \
	$(top_srcdir)/lib/gui/gm-smiley-chooser-button.h

if !WIN32
libekiga_la_SOURCES +=	\
	$(top_srcdir)/lib/gui/xwindow.cpp		\
	$(top_srcdir)/lib/gui/xwindow.h
endif

if HAVE_XV
libekiga_la_SOURCES +=	\
	$(top_srcdir)/lib/gui/xvwindow.cpp		\
	$(top_srcdir)/lib/gui/xvwindow.h

libekiga_la_LDFLAGS += $(XV_LIBS)

endif

if HAVE_DX
libekiga_la_SOURCES +=	\
	$(top_srcdir)/lib/gui/dxwindow.cpp		\
	$(top_srcdir)/lib/gui/dxwindow.h
endif

##
# Sources of the main engine code
##

libekiga_la_SOURCES += \
	$(engine_dir)/engine.h \
	$(engine_dir)/engine.cpp

##
# Sources of the engine framework code
##

libekiga_la_SOURCES += \
	$(engine_dir)/framework/boost-exceptions.cpp \
	$(engine_dir)/framework/services.h \
	$(engine_dir)/framework/map-key-iterator.h \
	$(engine_dir)/framework/map-key-const-iterator.h \
	$(engine_dir)/framework/reflister.h \
	$(engine_dir)/framework/chain-of-responsibility.h \
	$(engine_dir)/framework/device-def.h \
	$(engine_dir)/framework/form-builder.h \
	$(engine_dir)/framework/form-dumper.h \
	$(engine_dir)/framework/form.h \
	$(engine_dir)/framework/form.cpp \
	$(engine_dir)/framework/form-request.h \
	$(engine_dir)/framework/form-request-simple.h \
	$(engine_dir)/framework/robust-xml.h \
	$(engine_dir)/framework/robust-xml.cpp \
	$(engine_dir)/framework/form-visitor.h \
	$(engine_dir)/framework/gmconf-bridge.h \
	$(engine_dir)/framework/gmconf-bridge.cpp \
	$(engine_dir)/framework/runtime.h \
	$(engine_dir)/framework/menu-builder.h \
	$(engine_dir)/framework/menu-builder.cpp \
	$(engine_dir)/framework/menu-builder-tools.h \
	$(engine_dir)/framework/menu-builder-tools.cpp \
	$(engine_dir)/framework/form-builder.cpp \
	$(engine_dir)/framework/form-dumper.cpp \
	$(engine_dir)/framework/form-request-simple.cpp \
	$(engine_dir)/framework/runtime-glib.cpp \
	$(engine_dir)/framework/services.cpp \
	$(engine_dir)/framework/trigger.h \
	$(engine_dir)/framework/menu-xml.h \
	$(engine_dir)/framework/menu-xml.cpp \
	$(engine_dir)/framework/kickstart.h \
	$(engine_dir)/framework/kickstart.cpp \
	$(engine_dir)/framework/personal-details.h \
	$(engine_dir)/framework/ptr_array.h \
	$(engine_dir)/framework/ptr_array_iterator.h \
	$(engine_dir)/framework/ptr_array_const_iterator.h \
	$(engine_dir)/framework/live-object.h \
	$(engine_dir)/framework/filterable.h

##
# Sources of the plugin loader code
##

libekiga_la_SOURCES += \
	$(engine_dir)/plugin/plugin-core.h \
	$(engine_dir)/plugin/plugin-core.cpp

##
# Sources of the account stack
##

libekiga_la_SOURCES += \
	$(engine_dir)/account/account.h \
	$(engine_dir)/account/bank.h \
	$(engine_dir)/account/bank-impl.h \
	$(engine_dir)/account/account-core.h \
	$(engine_dir)/account/account-core.cpp

##
#  Sources of the addressbook stack
##

libekiga_la_SOURCES += \
	$(engine_dir)/addressbook/contact.h \
	$(engine_dir)/addressbook/book.h \
	$(engine_dir)/addressbook/book-impl.h \
	$(engine_dir)/addressbook/source.h \
	$(engine_dir)/addressbook/source-impl.h \
	$(engine_dir)/addressbook/contact-core.h \
	$(engine_dir)/addressbook/contact-core.cpp

##
# Sources of the chat stack
##

libekiga_la_SOURCES += \
	$(engine_dir)/chat/chat-core.h \
	$(engine_dir)/chat/chat-core.cpp \
	$(engine_dir)/chat/dialect.h \
	$(engine_dir)/chat/dialect-impl.h \
	$(engine_dir)/chat/chat.h \
	$(engine_dir)/chat/chat-simple.h \
	$(engine_dir)/chat/chat-multiple.h

##
# Sources of the friend-or-foe stack
##

libekiga_la_SOURCES += \
	$(engine_dir)/friend-or-foe/friend-or-foe.h \
	$(engine_dir)/friend-or-foe/friend-or-foe.cpp

##
# Sources of the notification stack
##

libekiga_la_SOURCES += \
	$(engine_dir)/notification/notification-core.h \
	$(engine_dir)/notification/notification-dummy.cpp

##
# Sources of the presence stack
##

libekiga_la_SOURCES += \
	$(engine_dir)/presence/presentity.h \
	$(engine_dir)/presence/heap.h \
	$(engine_dir)/presence/heap-impl.h \
	$(engine_dir)/presence/cluster.h \
	$(engine_dir)/presence/cluster-impl.h \
	$(engine_dir)/presence/uri-presentity.h \
	$(engine_dir)/presence/proxy-presentity.cpp \
	$(engine_dir)/presence/proxy-presentity.h \
	$(engine_dir)/presence/uri-presentity.cpp \
	$(engine_dir)/presence/presence-core.h \
	$(engine_dir)/presence/presence-core.cpp

##
# Sources of the protocol stack
##

libekiga_la_SOURCES += \
	$(engine_dir)/protocol/call-core.h \
	$(engine_dir)/protocol/call-manager.h \
	$(engine_dir)/protocol/call-manager.cpp \
	$(engine_dir)/protocol/call.h \
	$(engine_dir)/protocol/call-core.cpp \
	$(engine_dir)/protocol/call-protocol-manager.h \
	$(engine_dir)/protocol/codec-description.h \
	$(engine_dir)/protocol/codec-description.cpp

##
# Sources of the video output stack
##

libekiga_la_SOURCES += \
	$(engine_dir)/videooutput/videooutput-info.h \
	$(engine_dir)/videooutput/videooutput-manager.h \
	$(engine_dir)/videooutput/videooutput-core.h \
	$(engine_dir)/videooutput/videooutput-core.cpp \
	$(engine_dir)/videooutput/videooutput-gmconf-bridge.h \
	$(engine_dir)/videooutput/videooutput-gmconf-bridge.cpp

##
# Sources of the video input stack
##

libekiga_la_SOURCES += \
	$(engine_dir)/videoinput/videoinput-manager.h \
	$(engine_dir)/videoinput/videoinput-info.h \
	$(engine_dir)/videoinput/videoinput-core.h \
	$(engine_dir)/videoinput/videoinput-core.cpp \
	$(engine_dir)/videoinput/videoinput-gmconf-bridge.h \
	$(engine_dir)/videoinput/videoinput-gmconf-bridge.cpp

##
# Sources of the audio output stack
##

libekiga_la_SOURCES += \
	$(engine_dir)/audiooutput/audiooutput-manager.h \
	$(engine_dir)/audiooutput/audiooutput-info.h \
	$(engine_dir)/audiooutput/audiooutput-scheduler.h \
	$(engine_dir)/audiooutput/audiooutput-scheduler.cpp \
	$(engine_dir)/audiooutput/audiooutput-core.h \
	$(engine_dir)/audiooutput/audiooutput-core.cpp \
	$(engine_dir)/audiooutput/audiooutput-gmconf-bridge.h \
	$(engine_dir)/audiooutput/audiooutput-gmconf-bridge.cpp

##
# Sources of the audio input stack
##

libekiga_la_SOURCES += \
	$(engine_dir)/audioinput/audioinput-manager.h	\
	$(engine_dir)/audioinput/audioinput-info.h	\
	$(engine_dir)/audioinput/audioinput-core.h	\
	$(engine_dir)/audioinput/audioinput-core.cpp       \
	$(engine_dir)/audioinput/audioinput-gmconf-bridge.h \
	$(engine_dir)/audioinput/audioinput-gmconf-bridge.cpp

##
# Sources of the hardware abstraction layer (HAL) stack
##

libekiga_la_SOURCES += \
	$(engine_dir)/hal/hal-manager.h \
	$(engine_dir)/hal/hal-core.h \
	$(engine_dir)/hal/hal-core.cpp

##
# Sources of the gtk+ core
##

libekiga_la_SOURCES += \
	$(engine_dir)/gui/gtk-core/menu-builder-gtk.h \
	$(engine_dir)/gui/gtk-core/menu-builder-gtk.cpp \
	$(engine_dir)/gui/gtk-core/form-dialog-gtk.h \
	$(engine_dir)/gui/gtk-core/form-dialog-gtk.cpp \
	$(engine_dir)/gui/gtk-core/optional-buttons-gtk.h \
	$(engine_dir)/gui/gtk-core/optional-buttons-gtk.cpp \
	$(engine_dir)/gui/gtk-core/codecsbox.cpp \
	$(engine_dir)/gui/gtk-core/codecsbox.h \
	$(engine_dir)/gui/gtk-core/gtk-core.h \
	$(engine_dir)/gui/gtk-core/gtk-core.cpp \
	$(engine_dir)/gui/gtk-core/gtk-core-main.h \
	$(engine_dir)/gui/gtk-core/gtk-core-main.cpp

##
# Sources of the gtk+ frontend
##

libekiga_la_SOURCES += \
	$(engine_dir)/gui/gtk-frontend/addressbook-window.h \
	$(engine_dir)/gui/gtk-frontend/addressbook-window.cpp \
	$(engine_dir)/gui/gtk-frontend/accounts-window.h \
	$(engine_dir)/gui/gtk-frontend/accounts-window.cpp \
	$(engine_dir)/gui/gtk-frontend/book-view-gtk.h \
	$(engine_dir)/gui/gtk-frontend/book-view-gtk.cpp \
	$(engine_dir)/gui/gtk-frontend/call-window.h \
	$(engine_dir)/gui/gtk-frontend/call-window.cpp \
	$(engine_dir)/gui/gtk-frontend/ext-window.h \
	$(engine_dir)/gui/gtk-frontend/ext-window.cpp \
	$(engine_dir)/gui/gtk-frontend/roster-view-gtk.h \
	$(engine_dir)/gui/gtk-frontend/roster-view-gtk.cpp \
	$(engine_dir)/gui/gtk-frontend/call-history-view-gtk.h \
	$(engine_dir)/gui/gtk-frontend/call-history-view-gtk.cpp \
	$(engine_dir)/gui/gtk-frontend/default_devices.cpp \
	$(engine_dir)/gui/gtk-frontend/default_devices.h \
	$(engine_dir)/gui/gtk-frontend/presentity-view.h \
	$(engine_dir)/gui/gtk-frontend/presentity-view.cpp \
	$(engine_dir)/gui/gtk-frontend/heap-view.h \
	$(engine_dir)/gui/gtk-frontend/heap-view.cpp \
	$(engine_dir)/gui/gtk-frontend/chat-area.h \
	$(engine_dir)/gui/gtk-frontend/chat-area.cpp \
	$(engine_dir)/gui/gtk-frontend/simple-chat-page.h \
	$(engine_dir)/gui/gtk-frontend/simple-chat-page.cpp \
	$(engine_dir)/gui/gtk-frontend/multiple-chat-page.h \
	$(engine_dir)/gui/gtk-frontend/multiple-chat-page.cpp \
	$(engine_dir)/gui/gtk-frontend/preferences-window.cpp \
	$(engine_dir)/gui/gtk-frontend/preferences-window.h \
	$(engine_dir)/gui/gtk-frontend/statusicon.cpp \
	$(engine_dir)/gui/gtk-frontend/statusicon.h \
	$(engine_dir)/gui/gtk-frontend/chat-window.h \
	$(engine_dir)/gui/gtk-frontend/chat-window.cpp \
	$(engine_dir)/gui/gtk-frontend/gtk-frontend.h \
	$(engine_dir)/gui/gtk-frontend/gtk-frontend.cpp

##
# Sources of the ptlib component
##

libekiga_la_SOURCES += \
	$(components_dir)/ptlib/utils.h \
	$(components_dir)/ptlib/utils.cpp \
	$(components_dir)/ptlib/audioinput-manager-ptlib.h \
	$(components_dir)/ptlib/audioinput-manager-ptlib.cpp \
	$(components_dir)/ptlib/audioinput-main-ptlib.h \
	$(components_dir)/ptlib/audioinput-main-ptlib.cpp \
	$(components_dir)/ptlib/videoinput-manager-ptlib.h \
	$(components_dir)/ptlib/videoinput-manager-ptlib.cpp \
	$(components_dir)/ptlib/videoinput-main-ptlib.h \
	$(components_dir)/ptlib/videoinput-main-ptlib.cpp \
	$(components_dir)/ptlib/audiooutput-manager-ptlib.h \
	$(components_dir)/ptlib/audiooutput-manager-ptlib.cpp \
	$(components_dir)/ptlib/audiooutput-main-ptlib.h \
	$(components_dir)/ptlib/audiooutput-main-ptlib.cpp

libekiga_la_LDFLAGS += $(PTLIB_LIBS)

##
# Sources of the opal component
##

libekiga_la_SOURCES += \
	$(components_dir)/opal/opal-call-manager.h \
	$(components_dir)/opal/opal-call-manager.cpp \
	$(components_dir)/opal/pcss-endpoint.h \
	$(components_dir)/opal/pcss-endpoint.cpp \
	$(components_dir)/opal/opal-account.h \
	$(components_dir)/opal/opal-account.cpp \
	$(components_dir)/opal/opal-bank.h \
	$(components_dir)/opal/opal-bank.cpp \
	$(components_dir)/opal/opal-call.h \
	$(components_dir)/opal/opal-call.cpp \
	$(components_dir)/opal/opal-codec-description.h \
	$(components_dir)/opal/opal-codec-description.cpp \
	$(components_dir)/opal/opal-gmconf-bridge.h \
	$(components_dir)/opal/opal-gmconf-bridge.cpp \
	$(components_dir)/opal/opal-main.h \
	$(components_dir)/opal/opal-main.cpp \
	$(components_dir)/opal/opal-audio.h \
	$(components_dir)/opal/opal-audio.cpp \
	$(components_dir)/opal/opal-videoinput.h \
	$(components_dir)/opal/opal-videoinput.cpp \
	$(components_dir)/opal/opal-videooutput.h \
	$(components_dir)/opal/opal-videooutput.cpp \
	$(components_dir)/opal/opal-plugins-hook.h \
	$(components_dir)/opal/opal-plugins-hook.cpp

if HAVE_H323
libekiga_la_SOURCES += \
	$(components_dir)/opal/h323-endpoint.h \
	$(components_dir)/opal/h323-endpoint.cpp
endif

libekiga_la_SOURCES += \
	$(components_dir)/opal/sip-chat-simple.h \
	$(components_dir)/opal/sip-chat-simple.cpp \
	$(components_dir)/opal/sip-dialect.h \
	$(components_dir)/opal/sip-dialect.cpp \
	$(components_dir)/opal/sip-endpoint.h \
	$(components_dir)/opal/sip-endpoint.cpp

libekiga_la_LDFLAGS += $(OPAL_LIBS)

##
# sources of the call history component
##

libekiga_la_SOURCES += \
	$(components_dir)/call-history/history-contact.h \
	$(components_dir)/call-history/history-contact.cpp \
	$(components_dir)/call-history/history-book.h \
	$(components_dir)/call-history/history-book.cpp \
	$(components_dir)/call-history/history-source.h \
	$(components_dir)/call-history/history-source.cpp \
	$(components_dir)/call-history/history-main.h \
	$(components_dir)/call-history/history-main.cpp

##
# Sources of the text chat echo component
##

libekiga_la_SOURCES += \
	$(components_dir)/echo/echo-presentity.h \
	$(components_dir)/echo/echo-presentity.cpp \
	$(components_dir)/echo/echo-simple.h \
	$(components_dir)/echo/echo-simple.cpp \
	$(components_dir)/echo/echo-dialect.h \
	$(components_dir)/echo/echo-dialect.cpp \
	$(components_dir)/echo/echo-main.h \
	$(components_dir)/echo/echo-main.cpp

##
# Sources of the gmconf personal details component
##

libekiga_la_SOURCES += \
	$(components_dir)/gmconf-personal-details/gmconf-personal-details.cpp \
	$(components_dir)/gmconf-personal-details/gmconf-personal-details.h \
	$(components_dir)/gmconf-personal-details/gmconf-personal-details-main.cpp \
	$(components_dir)/gmconf-personal-details/gmconf-personal-details-main.h

##
# Sources of the local roster component
##

libekiga_la_SOURCES += \
	$(components_dir)/local-roster/local-presentity.h \
	$(components_dir)/local-roster/local-presentity.cpp \
	$(components_dir)/local-roster/local-heap.h \
	$(components_dir)/local-roster/local-heap.cpp \
	$(components_dir)/local-roster/local-cluster.h \
	$(components_dir)/local-roster/local-cluster.cpp \
	$(components_dir)/local-roster/local-roster-main.h \
	$(components_dir)/local-roster/local-roster-main.cpp \
	$(components_dir)/local-roster/local-roster-bridge.h \
	$(components_dir)/local-roster/local-roster-bridge.cpp

##
# Sources of the moving logo component
##

libekiga_la_SOURCES += \
	$(components_dir)/mlogo-videoinput/videoinput-manager-mlogo.h \
	$(components_dir)/mlogo-videoinput/videoinput-manager-mlogo.cpp \
	$(components_dir)/mlogo-videoinput/videoinput-main-mlogo.h \
	$(components_dir)/mlogo-videoinput/videoinput-main-mlogo.cpp

##
# Sources of the null audio input component
##

libekiga_la_SOURCES += \
	$(components_dir)/null-audioinput/audioinput-manager-null.h \
	$(components_dir)/null-audioinput/audioinput-manager-null.cpp \
	$(components_dir)/null-audioinput/audioinput-main-null.h \
	$(components_dir)/null-audioinput/audioinput-main-null.cpp

##
# Sources of the null audio output component
##

libekiga_la_SOURCES += \
	$(components_dir)/null-audiooutput/audiooutput-manager-null.h \
	$(components_dir)/null-audiooutput/audiooutput-manager-null.cpp \
	$(components_dir)/null-audiooutput/audiooutput-main-null.h \
	$(components_dir)/null-audiooutput/audiooutput-main-null.cpp

##
# Sources of the hal dbus component
##
if HAVE_DBUS

libekiga_la_SOURCES += \
	$(components_dir)/hal-dbus/hal-manager-dbus.h \
	$(components_dir)/hal-dbus/hal-manager-dbus.cpp \
	$(components_dir)/hal-dbus/hal-main-dbus.h \
	$(components_dir)/hal-dbus/hal-main-dbus.cpp

if HAVE_V4L
libekiga_la_SOURCES += \
	$(components_dir)/hal-dbus/hal-v4l-helper.h \
	$(components_dir)/hal-dbus/hal-v4l-helper.c
endif

nodist_libekiga_la_SOURCES += \
	$(builddir)/hal-marshal.h \
	$(builddir)/hal-marshal.c

libekiga_la_LDFLAGS += $(DBUS_LIBS)

# forces the HAVE :
BUILT_SOURCES += $(builddir)/hal-marshal.h $(builddir)/hal-marshal.c
CLEANFILES += $(builddir)/hal-marshal.h $(builddir)/hal-marshal.c
EXTRA_DIST += $(components_dir)/hal-dbus/hal-marshal.list

$(builddir)/hal-marshal.h: $(components_dir)/hal-dbus/hal-marshal.list
	$(AM_V_GEN)$(LIBTOOL) --mode=execute glib-genmarshal --prefix=gm_hal_dbus_marshal $^ --header > $@.tmp && mv $@.tmp $@

$(builddir)/hal-marshal.c: $(components_dir)/hal-dbus/hal-marshal.list
	$(AM_V_GEN)$(LIBTOOL) --mode=execute glib-genmarshal --prefix=gm_hal_dbus_marshal $^ --body > $@.tmp && mv $@.tmp $@

endif

##
# Support for shared memory
##
if HAVE_SHM

libekiga_la_LDFLAGS += $(SHM_LIBS)

endif

##
# Common files for several video output components
##

libekiga_la_SOURCES += \
	$(components_dir)/common-videooutput/videooutput-manager-common.cpp \
	$(components_dir)/common-videooutput/videooutput-manager-common.h

##
# Sources of the X video output component
##

if !WIN32

INCLUDES += \
	-I$(top_srcdir)/lib/engine/components/x-videooutput

libekiga_la_SOURCES += \
	$(components_dir)/x-videooutput/videooutput-manager-x.cpp \
	$(components_dir)/x-videooutput/videooutput-manager-x.h \
	$(components_dir)/x-videooutput/videooutput-main-x.cpp \
	$(components_dir)/x-videooutput/videooutput-main-x.h

endif

##
# Sources of the Direct X video output component
##

if HAVE_DX

libekiga_la_SOURCES += \
	$(components_dir)/dx-videooutput/videooutput-manager-dx.cpp \
	$(components_dir)/dx-videooutput/videooutput-manager-dx.h \
	$(components_dir)/dx-videooutput/videooutput-main-dx.cpp \
	$(components_dir)/dx-videooutput/videooutput-main-dx.h

libekiga_la_LDFLAGS += $(DX_LIBS)
endif
