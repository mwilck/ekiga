if HAVE_DBUS
DBUS_DIR = hal-dbus

if HAVE_GNOME
GNOME_SESSION_DIR = gnome-session
endif

endif

if HAVE_DX
DX_DIR = dx-videooutput
endif

if !WIN32
X_DIR = x-videooutput
endif

if HAVE_AVAHI
AVAHI_DIR = avahi-publisher avahi
endif

if HAVE_NOTIFY
LIBNOTIFY_DIR = libnotify
endif

SUBDIRS =                       \
	call-history	        \
        echo                    \
	gmconf-personal-details \
        local-roster            \
        mlogo-videoinput        \
        null-audioinput         \
        null-audiooutput        \
        opal                    \
        ptlib                   \
	$(GNOME_SESSION_DIR)	\
	$(AVAHI_DIR)            \
	$(DBUS_DIR)             \
	$(LIBNOTIFY_DIR)	\
	$(DX_DIR)               \
	$(X_DIR)