if HAVE_LDAP
LDAP_DIR = ldap
endif

if HAVE_KAB
KAB_DIR = kab
endif

if HAVE_EDS
EDS_DIR = evolution
endif

if HAVE_AVAHI
AVAHI_DIR = avahi-publisher
endif

if HAVE_GSTREAMER
GSTREAMER_DIR = gstreamer
endif

if HAVE_KDE
KDE_DIR = kde
endif

if HAVE_XCAP
XCAP_DIRS = xcap resource-list
endif

if HAVE_AVAHI
AVAHI_DIR += avahi
endif

SUBDIRS =                       \
	call-history	        \
	gmconf-personal-details \
        local-roster            \
        opal                    \
        ptlib                   \
	$(AVAHI_DIR)            \
	$(GSTREAMER_DIR)        \
	$(KDE_DIR)              \
	$(EDS_DIR) 	        \
	$(LDAP_DIR)	        \
	$(KAB_DIR)              \
	$(XCAP_DIRS)
