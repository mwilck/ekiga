SUBDIRS = framework addressbook presence gui protocol display vidinput audioinput audiooutput hal components

noinst_LTLIBRARIES = libekiga_engine.la

INCLUDES = 								\
	-I$(top_srcdir)/lib/gmconf/	 				\
	-I$(top_srcdir)/lib/engine/	 				\
	-I$(top_srcdir)/lib/engine/gui/gtk-core				\
	-I$(top_srcdir)/lib/engine/gui/gtk-frontend			\
	-I$(top_srcdir)/lib/engine/framework/		 		\
	-I$(top_srcdir)/lib/engine/protocol/skel			\
	-I$(top_srcdir)/lib/engine/addressbook/skel			\
	-I$(top_srcdir)/lib/engine/addressbook/call-history		\
	-I$(top_srcdir)/lib/engine/addressbook/evolution		\
	-I$(top_srcdir)/lib/engine/addressbook/ldap			\
	-I$(top_srcdir)/lib/engine/presence/skel			\
	-I$(top_srcdir)/lib/engine/presence/avahi			\
	-I$(top_srcdir)/lib/engine/presence/local-roster		\
	-I$(top_srcdir)/lib/engine/display/skel				\
	-I$(top_srcdir)/lib/engine/vidinput/skel			\
	-I$(top_srcdir)/lib/engine/audioinput/skel			\
	-I$(top_srcdir)/lib/engine/audiooutput/skel			\
	-I$(top_srcdir)/lib/engine/hal/skel				\
	-I$(top_srcdir)/lib/engine/components/gmconf-personal-details

# XVideo/X
if !WIN32
INCLUDES +=                                             \
	-I$(top_srcdir)/lib/engine/display/common       \
	-I$(top_srcdir)/lib/engine/display/x
endif

# DirectX
if HAVE_DX
INCLUDES +=                                             \
	-I$(top_srcdir)/lib/engine/display/common       \
	-I$(top_srcdir)/lib/engine/display/dx
endif
		
# Default Input Devices
INCLUDES +=                                             \
	-I$(top_srcdir)/lib/engine/vidinput/mlogo	\
	-I$(top_srcdir)/lib/engine/audioinput/null      \
	-I$(top_srcdir)/lib/engine/audiooutput/null

# Default PTLIB Input Devices
INCLUDES +=                                             \
	-I$(top_srcdir)/lib/engine/vidinput/ptlib       \
	-I$(top_srcdir)/lib/engine/audioinput/ptlib     \
	-I$(top_srcdir)/lib/engine/audiooutput/ptlib

# DBus / HALd
if HAVE_DBUS
INCLUDES +=                                             \
	-I$(top_srcdir)/lib/engine/hal/dbus
endif

libekiga_engine_la_SOURCES = engine.h engine.cpp

AM_CXXFLAGS = $(SIGC_CFLAGS) $(GTK_CFLAGS)
AM_LIBS = $(SIGC_LIBS) $(GTK_LIBS)

libekiga_engine_la_LDFLAGS = -export-dynamic
libekiga_engine_la_LIBADD = \
	$(top_builddir)/lib/engine/gui/gtk-core/libgmgtk-core.la 					\
	$(top_builddir)/lib/engine/gui/gtk-frontend/libgmgtk-frontend.la 				\
	$(top_builddir)/lib/engine/protocol/skel/libgmprotocol.la 					\
	$(top_builddir)/lib/engine/framework/libgmframework.la 						\
	$(top_builddir)/lib/engine/addressbook/skel/libgmaddressbook.la 				\
	$(top_builddir)/lib/engine/addressbook/call-history/libcall-history.la 				\
	$(top_builddir)/lib/engine/presence/skel/libgmpresence.la 					\
	$(top_builddir)/lib/engine/presence/local-roster/liblocal-roster.la 				\
	$(top_builddir)/lib/engine/display/skel/libgmdisplay.la						\
	$(top_builddir)/lib/engine/vidinput/skel/libgmvidinput.la					\
	$(top_builddir)/lib/engine/audioinput/skel/libgmaudioinput.la					\
	$(top_builddir)/lib/engine/audiooutput/skel/libgmaudiooutput.la					\
	$(top_builddir)/lib/engine/hal/skel/libgmhal.la					\
	$(top_builddir)/lib/engine/components/gmconf-personal-details/libgmconfpersonaldetails.la	\
	$(AM_LIBS)

# Default Input Devices
libekiga_engine_la_LIBADD += \
	$(top_builddir)/lib/engine/vidinput/mlogo/libgmvidinput-mlogo.la \
	$(top_builddir)/lib/engine/audioinput/null/libgmaudioinput-null.la \
	$(top_builddir)/lib/engine/audiooutput/null/libgmaudiooutput-null.la

# PTLIB Input Devices
libekiga_engine_la_LIBADD += \
	$(top_builddir)/lib/engine/vidinput/ptlib/libgmvidinput-ptlib.la \
	$(top_builddir)/lib/engine/audioinput/ptlib/libgmaudioinput-ptlib.la \
	$(top_builddir)/lib/engine/audiooutput/ptlib/libgmaudiooutput-ptlib.la

# DBus/HALd
if HAVE_DBUS
libekiga_engine_la_LIBADD += \
	$(top_builddir)/lib/engine/hal/dbus/libgmhal-dbus.la
endif

# XVideo/X
if !WIN32
libekiga_engine_la_LIBADD += \
	$(top_builddir)/lib/engine/display/x/libgmdisplay-x.la \
	$(top_builddir)/lib/pixops/libpixops.la
endif

# DirectX
if HAVE_DX
libekiga_engine_la_LIBADD += \
	$(top_builddir)/lib/engine/display/dx/libgmdisplay-dx.la
endif


if HAVE_AVAHI
libekiga_engine_la_LIBADD += presence/avahi/libgmavahi.la
endif

if HAVE_EDS
libekiga_engine_la_LIBADD += addressbook/evolution/libgmevolution.la
endif

if HAVE_LDAP
libekiga_engine_la_LIBADD += addressbook/ldap/libgmldap.la
endif

#### FIXME should be removed at a later stage
INCLUDES += \
	-I$(top_srcdir)/lib/gui/	                \
	-I$(top_srcdir)/src                             \
	-I$(top_srcdir)/src/clients/                    \
	-I$(top_srcdir)/src/components/                 \
	-I$(top_srcdir)/src/devices/                    \
	-I$(top_srcdir)/src/endpoints/                  \
	-I$(top_srcdir)/src/gui/                        

AM_CXXFLAGS += $(GTK_CFLAGS) $(GLIB_CFLAGS) $(OPAL_CFLAGS) $(PTLIB_CFLAGS)

libekiga_engine_la_LDFLAGS += $(GTK_LIBS) $(GLIB_LIBS) $(OPAL_LIBS) $(PTLIB_LIBS)

#### End
