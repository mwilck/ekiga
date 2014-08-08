#!/usr/bin/make -f

# RELEASE: 0=from git/svn, 1=release (tar.gz) for ekiga, ptlib and opal
RELEASE   := 0
# DEBUG: 0=release, 1=debug version
DEBUG     := 0
# MINGW32: 1=use old mingw32, 0=use new mingw-w64
MINGW32   := 0
# BIT32 (used only for mingw-w64): 1=generate 32bit, 0=generate 64bit
BIT32     := 1

# flags used by make for each of the libraries and ekiga
# using $(MAKEFLAGS) here might lead to interferences between variables in this
#   Makefile (e.g. DEBUG) and lib own flags
MAKEOPTS  := -j2
MAKE_LIB_OPTS:=

BUILDROOT := $(CURDIR)
TARGETDIR := $(BUILDROOT)/dist
SRCDIR    := $(BUILDROOT)/src
INCLUDEDIR:= $(BUILDROOT)/include
LIBDIR    := $(BUILDROOT)/lib
BINDIR    := $(BUILDROOT)/bin

export DEB_BUILD_GNU_TYPE:=$(shell dpkg-architecture -qDEB_BUILD_GNU_TYPE)

WGET := wget -nv -T 60 -N

ifeq ($(MINGW32), 1)
HOST_TOOL_PREFIX:=i586-mingw32msvc
export DEB_HOST_GNU_TYPE:="i586-pc-mingw32"
else
ifeq ($(BIT32), 1)
HOST_TOOL_PREFIX:=i686-w64-mingw32
export DEB_HOST_GNU_TYPE:="i686-pc-mingw32"
else
HOST_TOOL_PREFIX:=x86_64-w64-mingw32
export DEB_HOST_GNU_TYPE:="x86_64-pc-mingw32"
endif
endif
REALCC:=$(HOST_TOOL_PREFIX)-gcc
REALCXX:=$(HOST_TOOL_PREFIX)-g++
export CC=$(REALCC) -g -O2
export CXX=$(REALCXX) -g -O2
export LD:=$(HOST_TOOL_PREFIX)-ld
export NM:=$(HOST_TOOL_PREFIX)-nm
export AR:=$(HOST_TOOL_PREFIX)-ar
export RANLIB:=$(HOST_TOOL_PREFIX)-ranlib
export DLLTOOL:=$(HOST_TOOL_PREFIX)-dlltool
export DLLWRAP:=$(HOST_TOOL_PREFIX)-dllwrap
export OBJDUMP:=$(HOST_TOOL_PREFIX)-objdump
export WINDRES:=$(HOST_TOOL_PREFIX)-windres
export STRIP:=$(HOST_TOOL_PREFIX)-strip
export AS:=$(HOST_TOOL_PREFIX)-as
export LDD:=$(BUILDROOT)/$(HOST_TOOL_PREFIX)-ldd.sh

export CFLAGS:=-I$(INCLUDEDIR) -march=pentium-mmx
export CXXFLAGS:=-I$(INCLUDEDIR) -march=pentium-mmx
export LDFLAGS:=-L$(LIBDIR)
export PKG_CONFIG_PATH:=$(LIBDIR)/pkgconfig
export PKG_CONFIG_LIBDIR:=$(LIBDIR)
export PATH:=$(BINDIR):$(PATH)


ifeq ($(RELEASE),1)
	EKIGA_VER := 4.0.0
	EKIGA_URL := http://ftp.gnome.org/pub/gnome/sources/ekiga/4.0
	EKIGA_ARCHIVE := ekiga-$(EKIGA_VER).tar.xz
	EKIGA_DIR := $(BUILDROOT)/ekiga-$(EKIGA_VER)
else
	GIT_EKIGA_REV = reset --hard HEAD
	#GIT_EKIGA_REV := checkout -b v4_0 origin/v4_0
	EKIGA_URL := git://git.gnome.org/ekiga
	EKIGA_ARCHIVE := ekiga
	EKIGA_DIR = $(BUILDROOT)/ekiga
	EKIGA_VER := $(strip $(shell grep AC_INIT $(SRCDIR)/$(EKIGA_ARCHIVE)/configure.ac | awk 'BEGIN { FS = "," } ; { print $$2 }' | tr -d '[]'))
	EKIGA_REV := $(strip $(shell (cd $(SRCDIR)/$(EKIGA_ARCHIVE); git describe | awk 'BEGIN { FS = "-" } ; { print $$2 "_" $$3 }' )))
ifneq ($(EKIGA_REV),)
	EKIGA_VER := $(EKIGA_VER)-git-$(EKIGA_REV)
endif
endif
EKIGA_PREFIX := $(TARGETDIR)/Ekiga
ifeq ($(DEBUG),1)
EKIGA_INSTALLER := ekiga-setup-$(EKIGA_VER)-debug.exe
else
EKIGA_INSTALLER := ekiga-setup-$(EKIGA_VER).exe
endif
#EKIGA_WIN32_DIFF_DIR := $(CURDIR)/diff
EKIGA_INSTALLER_DIR := $(EKIGA_DIR)/win32/nsisinstaller

ifeq ($(RELEASE),1)
	OPAL_VER := 3.14.2
	OPAL_URL := http://ftp.gnome.org/pub/gnome/sources/opal/3.14
	OPAL_ARCHIVE := opal-$(OPAL_VER).tar.xz
	OPAL_DIR := $(BUILDROOT)/opal-$(OPAL_VER)
else
	OPAL_VER := 3.14-beta3
	# OPAL_REV: HEAD or revision number
	OPAL_REV := HEAD
	OPAL_URL := svn://svn.code.sf.net/p/opalvoip/code/opal/branches/v3_14
	OPAL_ARCHIVE := opal
	OPAL_DIR := $(BUILDROOT)/opal
endif
OPAL_LIB := libopal.$(OPAL_VER).dll

ifeq ($(RELEASE),1)
	PTLIB_VER := 2.14.2
	PTLIB_URL := http://ftp.gnome.org/pub/gnome/sources/ptlib/2.14
	PTLIB_ARCHIVE := ptlib-$(PTLIB_VER).tar.xz
	PTLIB_DIR := $(BUILDROOT)/ptlib-$(PTLIB_VER)
else
	PTLIB_VER := 2.14-beta3
	# PTLIB_REV: HEAD or revision number
	PTLIB_REV := HEAD
	PTLIB_URL := svn://svn.code.sf.net/p/opalvoip/code/ptlib/branches/v2_14
	PTLIB_ARCHIVE := ptlib
	PTLIB_DIR := $(BUILDROOT)/ptlib
endif
PTLIB_LIB := libpt.$(PTLIB_VER).dll

##############   ekiga/ptlib/opal library and headers dependencies
OPENLDAP_VER := 2.4.31
OPENLDAP_DIR := $(BUILDROOT)/openldap-$(OPENLDAP_VER)
OPENLDAP_ARCHIVE := openldap-$(OPENLDAP_VER).tgz
OPENLDAP_URL := http://www.openldap.org/software/download/OpenLDAP/openldap-release

LIBSASL2_VER := 2.1.22
LIBSASL2_DIR := $(BUILDROOT)/cyrus-sasl-$(LIBSASL2_VER)
LIBSASL2_ARCHIVE := cyrus-sasl-$(LIBSASL2_VER).zip
LIBSASL2_URL := http://developer.pidgin.im/static/win32

OGG_VER := 1.3.1
OGG_DIR := $(BUILDROOT)/libogg-$(OGG_VER)
OGG_ARCHIVE := libogg-$(OGG_VER).tar.xz
OGG_URL := http://downloads.xiph.org/releases/ogg

SPEEX_VER := 1.2rc1
SPEEX_DIR := $(BUILDROOT)/speex-$(SPEEX_VER)
SPEEX_ARCHIVE := speex-$(SPEEX_VER).tar.gz
SPEEX_URL := http://downloads.xiph.org/releases/speex

# libexpat is needed by ptlib
EXPAT_VER = 2.1.0
EXPAT_DIR = $(BUILDROOT)/expat-$(EXPAT_VER)
EXPAT_ARCHIVE := expat-$(EXPAT_VER).tar.gz
EXPAT_URL := https://downloads.sourceforge.net/project/expat/expat/$(EXPAT_VER)

# choose commit 203e34 (2007-09-30), since since then a few modifications
# have been done and the compilation needs to be updated
REGEX_REV := 203e34
REGEX_DIR := $(BUILDROOT)/regex
REGEX_ARCHIVE := regex

BOOST_VER := 1.54.0
BOOST_URL := http://downloads.sourceforge.net/project/boost/boost/$(BOOST_VER)
BOOST_ARCHIVE := boost_$(shell echo $(BOOST_VER)|tr . _).tar.gz
BOOST_DIR := $(BUILDROOT)/boost_$(shell echo $(BOOST_VER)|tr . _)

LIBAV_VER := 9.9
LIBAV_URL := http://www.libav.org/releases
LIBAV_DIR := $(BUILDROOT)/libav-$(LIBAV_VER)
LIBAV_ARCHIVE := libav-$(LIBAV_VER).tar.xz

X264_VER := a5831aa
X264_URL := git://git.videolan.org/x264.git
X264_DIR := $(BUILDROOT)/x264
X264_ARCHIVE := x264

# note that every celt release might break API
CELT_VER := 0.7.1
CELT_URL := http://downloads.xiph.org/releases/celt
CELT_DIR := $(BUILDROOT)/celt-$(CELT_VER)
CELT_ARCHIVE := celt-$(CELT_VER).tar.gz

THEORA_VER := 1.1.1
THEORA_URL := http://downloads.xiph.org/releases/theora
THEORA_DIR := $(BUILDROOT)/libtheora-$(THEORA_VER)
THEORA_ARCHIVE := libtheora-$(THEORA_VER).tar.xz

NSIS_VER := 2.46
NSIS_URL := http://downloads.sourceforge.net/nsis
NSISPROCESSES_URL := http://nsis.sourceforge.net/mediawiki/images/f/f3/
NSISSYTEM_DIR := /usr/share/nsis/Contrib/Modern\ UI
NSISPLUGIN_DIR := $(BUILDROOT)/nsisplugins

COGL_VER := 1.14.0
COGL_URL := http://ftp.gnome.org/pub/GNOME/sources/cogl/$(shell echo $(COGL_VER)|cut -f1-2 -d.)
COGL_DIR := $(BUILDROOT)/cogl-$(COGL_VER)
COGL_ARCHIVE := cogl-$(COGL_VER).tar.xz

JSONGLIB_VER := 0.16.2
JSONGLIB_URL := http://ftp.gnome.org/pub/GNOME/sources/json-glib/$(shell echo $(JSONGLIB_VER)|cut -f1-2 -d.)
JSONGLIB_DIR := $(BUILDROOT)/json-glib-$(JSONGLIB_VER)
JSONGLIB_ARCHIVE := json-glib-$(JSONGLIB_VER).tar.xz

CLUTTER_VER := 1.14.6
CLUTTER_URL := http://ftp.gnome.org/pub/GNOME/sources/clutter/$(shell echo $(CLUTTER_VER)|cut -f1-2 -d.)
CLUTTER_DIR := $(BUILDROOT)/clutter-$(CLUTTER_VER)
CLUTTER_ARCHIVE := clutter-$(CLUTTER_VER).tar.xz

CLUTTERGTK_VER := 1.4.4
CLUTTERGTK_URL := http://ftp.gnome.org/pub/GNOME/sources/clutter-gtk/$(shell echo $(CLUTTERGTK_VER)|cut -f1-2 -d.)
CLUTTERGTK_DIR := $(BUILDROOT)/clutter-gtk-$(CLUTTERGTK_VER)
CLUTTERGTK_ARCHIVE := clutter-gtk-$(CLUTTERGTK_VER).tar.xz

GSTREAMER_VER := 1.2.2
GSTREAMER_URL := http://ftp.gnome.org/pub/GNOME/sources/gstreamer/$(shell echo $(GSTREAMER_VER)|cut -f1-2 -d.)
GSTREAMER_DIR := $(BUILDROOT)/gstreamer-$(GSTREAMER_VER)
GSTREAMER_ARCHIVE := gstreamer-$(GSTREAMER_VER).tar.xz

GSTPLUGINSBASE_VER := 1.2.2
GSTPLUGINSBASE_URL := http://ftp.gnome.org/pub/GNOME/sources/gst-plugins-base/$(shell echo $(GSTPLUGINSBASE_VER)|cut -f1-2 -d.)
GSTPLUGINSBASE_DIR := $(BUILDROOT)/gst-plugins-base-$(GSTPLUGINSBASE_VER)
GSTPLUGINSBASE_ARCHIVE := gst-plugins-base-$(GSTPLUGINSBASE_VER).tar.xz

CLUTTERGST_VER := 2.0.10
CLUTTERGST_URL := http://ftp.gnome.org/pub/GNOME/sources/clutter-gst/$(shell echo $(CLUTTERGST_VER)|cut -f1-2 -d.)
CLUTTERGST_DIR := $(BUILDROOT)/clutter-gst-$(CLUTTERGST_VER)
CLUTTERGST_ARCHIVE := clutter-gst-$(CLUTTERGST_VER).tar.xz

# gtk headers + libraries
GTK_VER := 3.6.4-20130921
GTK_URL := http://win32builder.gnome.org
GTK_ARCHIVE := gtk+-bundle_$(GTK_VER)_win32.zip

LN:=ln
RM:=rm
CP:=cp
MV:=mv
SVN:=svn
GIT:=git

# write country codes you want to build help for (in addition to C),
#   or leave blank to build help for all available languages;
#   e.g. for French and C put: DOC_LINGUAS := fr
DOC_LINGUAS :=

confflags := --prefix=$(BUILDROOT) --build=$(DEB_BUILD_GNU_TYPE) --host=$(DEB_HOST_GNU_TYPE)

# without --disable-sasl there is a compile error
# libsasl2.a is found, but sasl.h is not
# the question is whether we need sasl support or not
# sasl does not seem to be needed by ptlib
confptlib := $(confflags) --enable-vfw \
	--disable-sasl --disable-alsa --disable-esd --disable-oss --disable-sunaudio --disable-shmaudio \
	--disable-v4l --disable-v4l2 --disable-bsdvideo --disable-avc --disable-dc
ifeq ($(MINGW32), 1)
confptlib := $(confptlib) --with-directx-includedir=$(EKIGA_DIR)/win32/directx
endif

# disable silk non-free codec
confopal := $(confflags) --disable-t38 --disable-fax --disable-silk

confekiga:= $(confflags) \
	--prefix=$(EKIGA_PREFIX) --bindir=$(EKIGA_PREFIX) --datadir=$(EKIGA_PREFIX) \
	--sysconfdir=$(EKIGA_PREFIX) --libdir=$(EKIGA_PREFIX) \
	--with-ldap-dir=$(OPENLDAP_DIR) --with-libsasl2-dir=$(LIBSASL2_DIR) --with-boost=$(BUILDROOT) # --enable-static-libs
ifeq ($(MINGW32), 1)
confekiga := $(confekiga) --with-directx-dir=$(EKIGA_DIR)/win32/directx
endif

# h264 decoding is provided by libav, while encoding by x264 lib
conflibav := --cross-prefix=$(HOST_TOOL_PREFIX)- --enable-shared --disable-static \
	--enable-cross-compile --arch=`echo $(HOST_TOOL_PREFIX)|cut -d- -f1` --target-os=mingw32 \
	--prefix=$(BUILDROOT) --libdir=$(LIBDIR) \
	--disable-programs --enable-runtime-cpudetect \
	--disable-network --disable-swscale --disable-avfilter --disable-avformat --disable-avdevice \
	--disable-everything --enable-decoder=mpeg4 --enable-decoder=h263 --enable-encoder=mpeg4 --enable-encoder=h263 --enable-encoder=h263p --enable-decoder=h264 --enable-hwaccel=h264_dxva2

confx264 := $(confflags) --enable-static --disable-cli --disable-avs

conftheora := $(confflags) --disable-vorbistest --disable-sdltest --disable-examples --disable-spec

# only signals2 is needed, which is a include-only "library"
#confboost := --user-config=user-config.jam --layout=system variant=release threading=multi link=shared runtime-link=shared target-os=windows threadapi=win32 --with-signals -q

all: binaries $(TARGETDIR)/$(EKIGA_INSTALLER)
	@echo "Built on $(shell date)"

update-sources::
	@echo "Updating on $(shell date)"

update: update-sources

binaries:
	@echo Checking prerequisites...
	hash $(REALCC) $(REALCXX) $(LD) $(NM) $(AR) $(RANLIB) $(DLLTOOL) $(DLLWRAP) $(OBJDUMP) $(WINDRES) $(AS)||(echo You need to install mingw-w64 or mingw32; exit 1)
	hash tar unzip zip xz wget aclocal autoheader libtoolize automake autoconf pkg-config gnome-autogen.sh bison glib-compile-schemas gdk-pixbuf-csource || (echo we need more binaries ; exit 1)
	[ -f /usr/share/gnome-doc-utils/gnome-doc-utils.make ] || (echo gnome-doc-tools not installed; exit 1)
	[ -f /usr/share/icons/gnome/index.theme ] || (echo gnome-icon-theme not installed; exit 1)
	hash makensis || (echo nsis not installed; exit 1)
	hash xmlto || (echo xmlto not installed; exit 1)
	hash yasm || (echo yasm not installed, mandatory for x264; exit 1)
#	[ $(BUILDROOT) = $(CURDIR) ] || (echo You are supposed to run this as Makefile in $(BUILDROOT); exit 1)
ifeq ($(MINGW32), 1)
	[ `grep define.*MINGW32_MINOR_VERSION /usr/i586-mingw32msvc/include/_mingw.h|tr -s " "|cut -d" " -f3` -ge 15 ] || (echo 'You need mingw32-runtime at version >= 3.15'; exit 1)
endif
	mkdir -p $(INCLUDEDIR) $(LIBDIR)/pkgconfig $(SRCDIR) $(BINDIR)
	$(RM) -f $(BINDIR)/cygpath
	$(LN) -s /bin/true $(BINDIR)/cygpath
	touch $@

### REGEX
update-sources::
	@echo "--- Getting libregex..."
	mkdir -p $(SRCDIR)/$(REGEX_ARCHIVE)
	cd $(SRCDIR)/$(REGEX_ARCHIVE); \
	for i in regex.c regexec.c regex.h regex_internal.c regex_internal.h regcomp.c alloca_.h alloca.c localcharset.c localcharset.h; do \
	  $(WGET) http://git.savannah.gnu.org/cgit/gnulib.git/plain/lib/$$i?id=$(REGEX_REV) -O $$i; \
	done

$(REGEX_DIR)/regex.h: $(SRCDIR)/$(REGEX_ARCHIVE)
	$(RM) -rf $(REGEX_DIR)
	mkdir -p $(REGEX_DIR)
	$(CP) $(SRCDIR)/$(REGEX_ARCHIVE)/* $(REGEX_DIR)
	$(MV) $(REGEX_DIR)/alloca_.h $(REGEX_DIR)/alloca.h
	echo '//' >> $(REGEX_DIR)/config.h
	touch $@

$(LIBDIR)/libregex.a: binaries $(REGEX_DIR)/regex.h
	cd $(REGEX_DIR) ;\
	$(CC) -I$(INCLUDEDIR) -I. -c regex.c ;\
	$(CC) -I$(INCLUDEDIR) -I. -c alloca.c ;\
	$(CC) -I$(INCLUDEDIR) -I. -DLIBDIR=$(INCLUDEDIR) -c localcharset.c ;\
	$(AR) r libregex.a regex.o alloca.o localcharset.o
	$(CP) $(REGEX_DIR)/libregex.a $(LIBDIR)
	$(CP) $(REGEX_DIR)/regex.h $(INCLUDEDIR)
	$(CP) $(REGEX_DIR)/alloca.h $(INCLUDEDIR)
	$(CP) $(REGEX_DIR)/localcharset.h $(INCLUDEDIR)
	touch $@

### boost
update-sources::
	@echo --- Getting boost ...
	$(WGET) -P src $(BOOST_URL)/$(BOOST_ARCHIVE)

$(INCLUDEDIR)/boost/signals2.hpp: $(SRCDIR)/$(BOOST_ARCHIVE)
	tar xfz $(SRCDIR)/$(BOOST_ARCHIVE)
	cp -r $(BOOST_DIR)/boost $(INCLUDEDIR)

#$(BOOST_DIR)/config.status: $(SRCDIR)/$(BOOST_ARCHIVE)
#	tar xfz $(SRCDIR)/$(BOOST_ARCHIVE)
#	# avoid configuring error at -g
#	cd $(BOOST_DIR); CC="$(REALCC)" ./bootstrap.sh --prefix=$(BUILDROOT)
#	touch $@
#
#$(LIBDIR)/libboost_signals.dll: $(BOOST_DIR)/config.status
#	echo "using gcc : : $(HOST_TOOL_PREFIX)-g++ : ;" >$(BOOST_DIR)/user-config.jam
#	cd $(BOOST_DIR); ./b2 $(confboost)
#	cd $(BOOST_DIR); ./b2 $(confboost) install

### expat
update-sources::
	@echo --- Getting EXPAT ...
	$(WGET) -P src $(EXPAT_URL)/$(EXPAT_ARCHIVE)

$(EXPAT_DIR)/config.status: $(SRCDIR)/$(EXPAT_ARCHIVE)
	tar xfz $(SRCDIR)/$(EXPAT_ARCHIVE)
	cd $(EXPAT_DIR); ./configure $(confflags)

$(LIBDIR)/libexpat.a: $(EXPAT_DIR)/config.status
	$(MAKE) $(MAKEOPTS) -C $(EXPAT_DIR)
	$(MAKE) $(MAKEOPTS) -C $(EXPAT_DIR) install
	touch $@

### OPENLDAP
update-sources::
	@echo --- Getting OpenLDAP ...
	$(WGET) -P $(SRCDIR) $(OPENLDAP_URL)/$(OPENLDAP_ARCHIVE)

$(OPENLDAP_DIR)/configure: $(LIBDIR)/libregex.a $(SRCDIR)/$(OPENLDAP_ARCHIVE)
	rm -f $(LIBDIR)/libldap.dll
	tar xfz $(SRCDIR)/$(OPENLDAP_ARCHIVE) -C $(BUILDROOT)
	sed -i -e 's#windres#$(WINDRES)#g' $(OPENLDAP_DIR)/libraries/liblutil/Makefile.in
	touch $@

$(OPENLDAP_DIR)/config.status: binaries $(OPENLDAP_DIR)/configure
	#-$(MAKE) -C $(OPENLDAP_DIR) clean
	rm -f $(LIBDIR)/libldap.dll
	$(LN) -sf $(INCLUDEDIR)/regex.h $(OPENLDAP_DIR)/include/
	cd $(OPENLDAP_DIR); ./configure --with-cyrus-sasl=no --enable-bdb=no --enable-hdb=no --with-tls=no $(confflags)
	$(MAKE) $(MAKEOPTS) -C $(OPENLDAP_DIR) depend
	touch $@

$(LIBDIR)/libldap.dll: binaries $(OPENLDAP_DIR)/config.status
	$(MAKE) $(MAKE_LIB_OPTS) -C $(OPENLDAP_DIR)/libraries/liblutil
	$(CP) -f $(OPENLDAP_DIR)/libraries/liblutil/liblutil.a $(LIBDIR)
	$(MAKE) $(MAKE_LIB_OPTS) -C $(OPENLDAP_DIR)/libraries/liblber all install
	$(MAKE) $(MAKE_LIB_OPTS) -C $(OPENLDAP_DIR)/include install
	$(MAKE) $(MAKE_LIB_OPTS) -C $(OPENLDAP_DIR)/libraries/libldap MOD_LIBS="-llutil" all install
	touch $@

### libsasl2
update-sources::
	@echo --- Getting libsasl2 ...
	$(WGET) -P $(SRCDIR) $(LIBSASL2_URL)/$(LIBSASL2_ARCHIVE)

$(LIBDIR)/libsasl2.a: $(SRCDIR)/$(LIBSASL2_ARCHIVE)
	rm -f $(LIBDIR)/libsasl2.a
	rm -rf $(LIBSASL2_DIR)
	unzip -u $(SRCDIR)/$(LIBSASL2_ARCHIVE) -d $(BUILDROOT)
	cp $(LIBSASL2_DIR)/lib/libsasl.lib $(LIBDIR)/libsasl2.a

### PTLib
update-sources::
	@echo --- Getting PTLib ...
ifeq ($(RELEASE),1)
	$(WGET) -P $(SRCDIR) $(PTLIB_URL)/$(PTLIB_ARCHIVE)
else
	$(SVN) co $(PTLIB_URL) $(SRCDIR)/$(PTLIB_ARCHIVE) -r $(PTLIB_REV)
endif

$(PTLIB_DIR)/configure: binaries $(LIBDIR)/libldap.dll $(LIBDIR)/libogg.a $(LIBDIR)/libspeex.a $(SRCDIR)/$(PTLIB_ARCHIVE) $(LIBDIR)/libexpat.a $(LIBDIR)/libregex.a
	rm -rf $(PTLIB_DIR) $(LIBDIR)/$(PTLIB_LIB) $(INCLUDEDIR)/ptclib $(INCLUDEDIR)/ptlib
ifeq ($(RELEASE),1)
	tar Jxf $(SRCDIR)/$(PTLIB_ARCHIVE) -C $(BUILDROOT)
else
	$(CP) -r $(SRCDIR)/$(PTLIB_ARCHIVE) $(PTLIB_DIR)
endif
	# regenerate configure, since it might be older than configure.ac
	cd $(PTLIB_DIR); aclocal && autoconf
ifeq ($(MINGW32), 1)
	# mingw32 wrongly defines DnsSectionAdditional
	sed -i -e 's#\#include <ntverp.h># #g' $(PTLIB_DIR)/include/ptclib/pdns.h
	sed -i -e 's#enum { DnsSectionAdditional = DnsSectionAddtional };# #g' $(PTLIB_DIR)/include/ptclib/pdns.h
endif
	touch $@

$(PTLIB_DIR)/config.status: binaries $(PTLIB_DIR)/configure
	rm -f $(LIBDIR)/$(PTLIB_LIB)
	cd $(PTLIB_DIR); ./configure $(confptlib)

$(LIBDIR)/$(PTLIB_LIB): binaries $(PTLIB_DIR)/config.status
	# use make opt instead of make until ptlib fixes bug where plugins are not built
	$(MAKE) opt $(MAKEOPTS) VERBOSE=1 -C $(PTLIB_DIR)
	$(MAKE) -C $(PTLIB_DIR) install

### libogg
update-sources::
	@echo --- Getting Ogg ...
	$(WGET) -P $(SRCDIR) $(OGG_URL)/$(OGG_ARCHIVE)

$(OGG_DIR)/config.status: binaries $(SRCDIR)/$(OGG_ARCHIVE)
	rm -f $(LIBDIR)/libogg.a
	tar xfJ $(SRCDIR)/$(OGG_ARCHIVE) -C $(BUILDROOT)
	cd $(OGG_DIR); ./configure $(confflags)

$(LIBDIR)/libogg.a: binaries $(OGG_DIR)/config.status
	rm -f $(LIBDIR)/libogg.a
	$(MAKE) $(MAKEOPTS) -C $(OGG_DIR)
	$(MAKE) -C $(OGG_DIR) install

### libspeex
update-sources::
	@echo --- Getting Speex ...
	$(WGET) -P $(SRCDIR) $(SPEEX_URL)/$(SPEEX_ARCHIVE)

$(SPEEX_DIR)/config.status: binaries $(SRCDIR)/$(SPEEX_ARCHIVE)
	rm -f $(LIBDIR)/libspeex.a
	tar xfz $(SRCDIR)/$(SPEEX_ARCHIVE) -C $(BUILDROOT)
	cd $(SPEEX_DIR); ./configure $(confflags)

$(LIBDIR)/libspeex.a: binaries $(SPEEX_DIR)/config.status
	rm -f $(LIBDIR)/libspeex.a
	$(MAKE) $(MAKE_LIB_OPTS) -C $(SPEEX_DIR)
	$(MAKE) -C $(SPEEX_DIR) install

### libav
update-sources::
	@echo --- Getting libav ...
	$(WGET) -P $(SRCDIR) $(LIBAV_URL)/$(LIBAV_ARCHIVE)
	#touch $(SRCDIR)/$(LIBAV_ARCHIVE)

$(LIBAV_DIR)/config.status: $(SRCDIR)/$(LIBAV_ARCHIVE)
	rm -rf $(LIBAV_DIR)
	tar Jxf $(SRCDIR)/$(LIBAV_ARCHIVE) -C $(BUILDROOT)
ifeq ($(MINGW32), 1)
	# workaround a bug in gcc 4.2.x (http://code.google.com/p/chromium/issues/detail?id=37905)
	sed -i -e 's#check_cflags -Werror=missing-prototypes##g' $(LIBAV_DIR)/configure
endif
	cd $(LIBAV_DIR); ./configure $(conflibav)
	touch $@

$(BINDIR)/avcodec.dll: $(LIBAV_DIR)/config.status
	$(MAKE) V=1 $(MAKE_LIB_OPTS) -C $(LIBAV_DIR)
	$(MAKE) -C $(LIBAV_DIR) install
	touch $@

### x264
update-sources::
	@echo --- Getting x264 ...
ifeq ($(wildcard $(SRCDIR)/$(X264_ARCHIVE)), )
	$(GIT) clone $(X264_URL) $(SRCDIR)/$(X264_ARCHIVE)
endif
	cd $(SRCDIR)/$(X264_ARCHIVE); git reset --hard $(X264_VER)
	#touch $(SRCDIR)/$(X264_ARCHIVE)

$(X264_DIR)/config.status: $(SRCDIR)/$(X264_ARCHIVE)
	rm -rf $(X264_DIR)
	$(CP) -r $(SRCDIR)/$(X264_ARCHIVE) $(X264_DIR)
	cd $(X264_DIR); ./configure $(confx264)
	touch $@

$(LIBDIR)/libx264.a: $(X264_DIR)/config.status
	$(MAKE) $(MAKE_LIB_OPTS) -C $(X264_DIR)
	$(MAKE) -C $(X264_DIR) install

### celt
update-sources::
	@echo --- Getting CELT ...
	$(WGET) -P $(SRCDIR) $(CELT_URL)/$(CELT_ARCHIVE)

$(CELT_DIR)/config.status: $(SRCDIR)/$(CELT_ARCHIVE) $(LIBDIR)/libogg.a
	rm -rf $(CELT_DIR) $(INCLUDEDIR)/celt
	tar xfz $(SRCDIR)/$(CELT_ARCHIVE) -C $(BUILDROOT)
	cd $(CELT_DIR); ./configure $(confflags) --disable-oggtest

$(LIBDIR)/libcelt0.a: $(CELT_DIR)/config.status
	$(MAKE) $(MAKE_LIB_OPTS) -C $(CELT_DIR)
	$(MAKE) -C $(CELT_DIR) install

### theora
update-sources::
	@echo --- Getting theora ...
	$(WGET) -P $(SRCDIR) $(THEORA_URL)/$(THEORA_ARCHIVE)

$(THEORA_DIR)/config.status: $(SRCDIR)/$(THEORA_ARCHIVE) $(LIBDIR)/libogg.a
	rm -rf $(THEORA_DIR)
	tar xfJ $(SRCDIR)/$(THEORA_ARCHIVE) -C $(BUILDROOT)
	# .def files of theora use CRLF line terminators, which makes the new
	#   binutils trigger a linker error:
	# /usr/bin/i686-w64-mingw32-ld: .libs/libtheoradec-1.dll.def:3: syntax error
	sed -i -e 's#\r##g' $(THEORA_DIR)/win32/xmingw32/libtheoradec-all.def
	sed -i -e 's#\r##g' $(THEORA_DIR)/win32/xmingw32/libtheoraenc-all.def
	cd $(THEORA_DIR); ./autogen.sh $(conftheora)

$(LIBDIR)/libtheora.a: $(THEORA_DIR)/config.status
	$(MAKE) $(MAKE_LIB_OPTS) -C $(THEORA_DIR)
	$(MAKE) -C $(THEORA_DIR) install
	sed -i -e "s#library_names=''#library_names='libtheora\.a'#g" $(LIBDIR)/libtheora.la

### OPAL
update-sources::
	@echo --- Getting OPAL ...
ifeq ($(RELEASE),1)
	$(WGET) -P $(SRCDIR) $(OPAL_URL)/$(OPAL_ARCHIVE)
else
	$(SVN) co $(OPAL_URL) $(SRCDIR)/$(OPAL_ARCHIVE) -r $(OPAL_REV)
endif

$(OPAL_DIR)/configure: binaries $(LIBDIR)/libogg.a $(LIBDIR)/libspeex.a $(LIBDIR)/$(PTLIB_LIB) $(SRCDIR)/$(OPAL_ARCHIVE) $(BINDIR)/avcodec.dll $(LIBDIR)/libx264.a $(LIBDIR)/libcelt0.a $(LIBDIR)/libtheora.a
	rm -rf $(OPAL_DIR) $(LIBDIR)/$(OPAL_LIB) $(INCLUDEDIR)/opal
ifeq ($(RELEASE),1)
	tar Jxf $(SRCDIR)/$(OPAL_ARCHIVE) -C $(BUILDROOT)
else
	$(CP) -r $(SRCDIR)/$(OPAL_ARCHIVE) $(OPAL_DIR)
endif
	# fix build error with isac
	sed -i -e 's/#elif (defined(WEBRTC_ARCH_X86) \&\& defined(WIN32))/#elif 0 \&\& (defined(WEBRTC_ARCH_X86) \&\& defined(WIN32))/g' $(OPAL_DIR)/plugins/audio/iSAC/iSAC/os_specific_inline.h
	# fix build errors with libg722.1
	##sed -i -e 's#\#\! /bin/sh#\#\! /bin/sh\nLD=#g' $(OPAL_DIR)/plugins/audio/G.722.1/libg722_1/configure
	###sed -i -e 's#rpl_# #g' $(OPAL_DIR)/plugins/audio/G.722.1/libg722_1/configure
	###sed -i -e 's#am__api_version='1.13'#am__api_version='1.14'#g' $(OPAL_DIR)/plugins/audio/G.722.1/libg722_1/configure
	# try also touch aclocal.m4
	# fix build errors with opus
	###sed -i -e 's#\#\! /bin/sh#\#\! /bin/sh\nLD=#g' $(OPAL_DIR)/plugins/audio/Opus/opus/configure
	# regenerate configure, since it might be older than configure.ac
	cd $(OPAL_DIR); aclocal && autoconf
	cd $(OPAL_DIR)/plugins; aclocal && autoconf
	#cd $(OPAL_DIR)/plugins/audio/G.722.1/libg722_1; ./autogen.sh
	#cd $(OPAL_DIR)/plugins/audio/Opus/opus; aclocal && autoconf
	touch $@

$(OPAL_DIR)/config.status: binaries $(OPAL_DIR)/configure
	rm -f $(LIBDIR)/$(OPAL_LIB)
	# g.722.1 gives complex build errors, disable it until it gets fixed
	cd $(OPAL_DIR); ./configure --disable-g.722.1 $(confopal)

$(LIBDIR)/$(OPAL_LIB): binaries $(OPAL_DIR)/config.status
	rm -f $(LIBDIR)/$(OPAL_LIB)
	# use make opt instead of make until opal fixes bug where plugins are not built
	$(MAKE) opt $(MAKEOPTS) VERBOSE=1 -C $(OPAL_DIR)
	$(MAKE) -C $(OPAL_DIR) install

### gtk bundle
update-sources::
	@echo --- Getting GTK3 libs + headers bundle...
	$(WGET) -P $(SRCDIR) $(GTK_URL)/$(GTK_ARCHIVE)

$(LIBDIR)/pkgconfig/gtk+-3.0.pc: $(SRCDIR)/$(GTK_ARCHIVE)
	unzip -o $(SRCDIR)/$(GTK_ARCHIVE) -d $(BUILDROOT)
	sed -i -e 's#^prefix=.*$$#prefix=$(BUILDROOT)#g' $(BINDIR)/glib-gettextize
	sed -i -e 's#^prefix=.*$$#prefix=$(BUILDROOT)#g' $(LIBDIR)/pkgconfig/g*.pc
	sed -i -e 's#^prefix=.*$$#prefix=$(BUILDROOT)#g' $(LIBDIR)/pkgconfig/atk*.pc
	sed -i -e 's#^prefix=.*$$#prefix=$(BUILDROOT)#g' $(LIBDIR)/pkgconfig/pango*.pc
	sed -i -e 's#^prefix=.*$$#prefix=$(BUILDROOT)#g' $(LIBDIR)/pkgconfig/cairo*.pc
	sed -i -e 's#^prefix=.*$$#prefix=$(BUILDROOT)#g' $(LIBDIR)/pkgconfig/libxml*.pc
	touch $@

### cogl
update-sources::
	@echo --- Getting cogl ...
	$(WGET) -P $(SRCDIR) $(COGL_URL)/$(COGL_ARCHIVE)
	# without gl.h, cogl gives compile (not configure!) errors
	# mingw-w64 too provides gl.h, but perhaps it is too old?
	$(WGET) -P $(SRCDIR) http://cgit.freedesktop.org/mesa/mesa/plain/include/GL/gl.h

$(COGL_DIR)/config.status: $(SRCDIR)/$(COGL_ARCHIVE)
	rm -rf $(COGL_DIR)
	tar xf $(SRCDIR)/$(COGL_ARCHIVE) -C $(BUILDROOT)
	mkdir -p $(INCLUDEDIR)/GL; cp $(SRCDIR)/gl.h $(INCLUDEDIR)/GL
	cd $(COGL_DIR); ./configure $(confflags)

$(LIBDIR)/pkgconfig/cogl-1.0.pc: $(COGL_DIR)/config.status
	$(MAKE) $(MAKE_LIB_OPTS) -C $(COGL_DIR)
	$(MAKE) -C $(COGL_DIR) install

### json glib
update-sources::
	@echo --- Getting json glib ...
	$(WGET) -P $(SRCDIR) $(JSONGLIB_URL)/$(JSONGLIB_ARCHIVE)

$(JSONGLIB_DIR)/config.status: $(SRCDIR)/$(JSONGLIB_ARCHIVE)
	rm -rf $(JSONGLIB_DIR)
	tar xf $(SRCDIR)/$(JSONGLIB_ARCHIVE) -C $(BUILDROOT)
	cd $(JSONGLIB_DIR); ./configure $(confflags)

$(LIBDIR)/pkgconfig/json-glib-1.0.pc: $(JSONGLIB_DIR)/config.status
	$(MAKE) $(MAKE_LIB_OPTS) -C $(JSONGLIB_DIR)
	$(MAKE) -C $(JSONGLIB_DIR) install

### clutter
update-sources::
	@echo --- Getting clutter ...
	$(WGET) -P $(SRCDIR) $(CLUTTER_URL)/$(CLUTTER_ARCHIVE)

$(CLUTTER_DIR)/config.status: $(SRCDIR)/$(CLUTTER_ARCHIVE) $(LIBDIR)/pkgconfig/cogl-1.0.pc $(LIBDIR)/pkgconfig/json-glib-1.0.pc
	rm -rf $(CLUTTER_DIR) $(INCLUDEDIR)/clutter
	tar xf $(SRCDIR)/$(CLUTTER_ARCHIVE) -C $(BUILDROOT)
	cd $(CLUTTER_DIR); ./configure $(confflags) --disable-examples

$(LIBDIR)/pkgconfig/clutter-1.0.pc: $(CLUTTER_DIR)/config.status
	$(MAKE) $(MAKE_LIB_OPTS) -C $(CLUTTER_DIR)
	$(MAKE) -C $(CLUTTER_DIR) install

### clutter gtk
update-sources::
	@echo --- Getting clutter gtk ...
	$(WGET) -P $(SRCDIR) $(CLUTTERGTK_URL)/$(CLUTTERGTK_ARCHIVE)

$(CLUTTERGTK_DIR)/config.status: $(SRCDIR)/$(CLUTTERGTK_ARCHIVE)
	rm -rf $(CLUTTERGTK_DIR) $(INCLUDEDIR)/clutter-gtk-1.0
	tar xf $(SRCDIR)/$(CLUTTERGTK_ARCHIVE) -C $(BUILDROOT)
	# without the sed there is a compile error
	sed -i -e 's#<clutter/clutter-win32.h>#<clutter/win32/clutter-win32.h>#g' $(CLUTTERGTK_DIR)/clutter-gtk/gtk-clutter-actor.c
	# without the sed no .dll file is generated, which fools the linker
	sed -i -e 's#$$(CLUTTER_LT_LDFLAGS)#$$(CLUTTER_LT_LDFLAGS) -no-undefined#g' $(CLUTTERGTK_DIR)/clutter-gtk/Makefile.in
	cd $(CLUTTERGTK_DIR); ./configure $(confflags)

$(LIBDIR)/pkgconfig/clutter-gtk-1.0.pc: $(CLUTTERGTK_DIR)/config.status
	$(MAKE) $(MAKE_LIB_OPTS) -C $(CLUTTERGTK_DIR)
	$(MAKE) -C $(CLUTTERGTK_DIR) install

### gstreamer
update-sources::
	@echo --- Getting gstreamer ...
	$(WGET) -P $(SRCDIR) $(GSTREAMER_URL)/$(GSTREAMER_ARCHIVE)

$(GSTREAMER_DIR)/config.status: $(SRCDIR)/$(GSTREAMER_ARCHIVE)
	rm -rf $(GSTREAMER_DIR) $(INCLUDEDIR)/gstreamer-1.0
	tar xf $(SRCDIR)/$(GSTREAMER_ARCHIVE) -C $(BUILDROOT)
	cd $(GSTREAMER_DIR); ./configure $(confflags) --disable-gtk-doc-html

$(LIBDIR)/pkgconfig/gstreamer-1.0.pc: $(GSTREAMER_DIR)/config.status
	$(MAKE) $(MAKE_LIB_OPTS) -C $(GSTREAMER_DIR)
	$(MAKE) -C $(GSTREAMER_DIR) install

### gst plugins base
update-sources::
	@echo --- Getting gst plugins base ...
	$(WGET) -P $(SRCDIR) $(GSTPLUGINSBASE_URL)/$(GSTPLUGINSBASE_ARCHIVE)

$(GSTPLUGINSBASE_DIR)/config.status: $(SRCDIR)/$(GSTPLUGINSBASE_ARCHIVE) $(LIBDIR)/pkgconfig/gstreamer-1.0.pc
	rm -rf $(GSTPLUGINSBASE_DIR)
	tar xf $(SRCDIR)/$(GSTPLUGINSBASE_ARCHIVE) -C $(BUILDROOT)
	# version 1.2.2 compile error with gcc 4.9: https://github.com/mxe/mxe/commit/34cc814
	sed -i -e 's#libgstaudioresample_la_CFLAGS =#libgstaudioresample_la_CFLAGS =-msse2#g' $(GSTPLUGINSBASE_DIR)/gst/audioresample/Makefile.in
	cd $(GSTPLUGINSBASE_DIR); ./configure $(confflags) --disable-gtk-doc-html

$(LIBDIR)/pkgconfig/gstreamer-plugins-base-1.0.pc: $(GSTPLUGINSBASE_DIR)/config.status
	$(MAKE) $(MAKE_LIB_OPTS) -C $(GSTPLUGINSBASE_DIR)
	$(MAKE) -C $(GSTPLUGINSBASE_DIR) install

### clutter gst
update-sources::
	@echo --- Getting clutter gst ...
	$(WGET) -P $(SRCDIR) $(CLUTTERGST_URL)/$(CLUTTERGST_ARCHIVE)

$(CLUTTERGST_DIR)/config.status: $(SRCDIR)/$(CLUTTERGST_ARCHIVE) $(LIBDIR)/pkgconfig/gstreamer-1.0.pc $(LIBDIR)/pkgconfig/gstreamer-plugins-base-1.0.pc
	rm -rf $(CLUTTERGST_DIR) $(INCLUDEDIR)/clutter-gst-2.0
	tar xf $(SRCDIR)/$(CLUTTERGST_ARCHIVE) -C $(BUILDROOT)
	cd $(CLUTTERGST_DIR); ./configure $(confflags) --disable-gtk-doc-html

$(LIBDIR)/pkgconfig/clutter-gst-2.0.pc: $(CLUTTERGST_DIR)/config.status
	$(MAKE) $(MAKE_LIB_OPTS) -C $(CLUTTERGST_DIR)
	$(MAKE) -C $(CLUTTERGST_DIR) install

### ekiga
update-sources::
	@echo --- Getting Ekiga ...
ifeq ($(RELEASE),1)
	$(WGET) -P $(SRCDIR) $(EKIGA_URL)/$(EKIGA_ARCHIVE)
else
ifeq ($(wildcard $(SRCDIR)/$(EKIGA_ARCHIVE)), )
	$(GIT) clone $(EKIGA_URL) $(SRCDIR)/$(EKIGA_ARCHIVE)
	# switch to branch
	cd $(SRCDIR)/$(EKIGA_ARCHIVE); git $(GIT_EKIGA_REV)
else
	cd $(SRCDIR)/$(EKIGA_ARCHIVE); $(GIT) pull
endif
endif

$(EKIGA_DIR)/configure: binaries $(SRCDIR)/$(EKIGA_ARCHIVE)
	rm -rf $(EKIGA_DIR)
ifeq ($(RELEASE),1)
	tar Jxf $(SRCDIR)/$(EKIGA_ARCHIVE) -C $(BUILDROOT)
else
	$(CP) -r $(SRCDIR)/$(EKIGA_ARCHIVE) $(EKIGA_DIR)
	cd $(EKIGA_DIR); NOCONFIGURE=1 ./autogen.sh  # generate configure
endif
	# until gtk 3.10 bundle works on windows
	cd $(EKIGA_DIR); patch -p1 <$(EKIGA_DIR)/win32/usegtk3.6.patch
	sed -i -e 's/3.10.0/3.6.0/g' $(EKIGA_DIR)/configure
	touch $@

$(EKIGA_DIR)/config.status: binaries $(EKIGA_DIR)/configure \
		$(LIBDIR)/$(PTLIB_LIB) $(LIBDIR)/$(OPAL_LIB) \
		$(LIBDIR)/pkgconfig/gtk+-3.0.pc \
		$(INCLUDEDIR)/boost/signals2.hpp $(LIBDIR)/libsasl2.a \
		$(LIBDIR)/pkgconfig/clutter-1.0.pc $(LIBDIR)/pkgconfig/clutter-gtk-1.0.pc \
		$(LIBDIR)/pkgconfig/clutter-gst-2.0.pc
	rm -f $(EKIGA_DIR)/src/ekiga.exe
	cd $(EKIGA_DIR); ACLOCAL_FLAGS="-I $(BUILDROOT)/share/aclocal" LDFLAGS="$(LDFLAGS) -Wl,-subsystem,windows" ./configure $(confekiga)

$(EKIGA_DIR)/src/ekiga.exe: binaries $(EKIGA_DIR)/config.status
	$(MAKE) $(MAKEOPTS) V=1 -C $(EKIGA_DIR)
	$(MAKE) $(MAKEOPTS) V=1 -C $(EKIGA_DIR)/help

$(TARGETDIR)/zips: binaries $(EKIGA_DIR)/src/ekiga.exe
	$(RM) -rf $(TARGETDIR)
	mkdir $(TARGETDIR)
	make -C $(EKIGA_DIR) install
# remove unneeded files, if any
	rm -f $(EKIGA_PREFIX)/*.dll.a
	rm -f $(EKIGA_PREFIX)/*.la
	rm -f $(EKIGA_PREFIX)/plugins/*.dll.a
	rm -f $(EKIGA_PREFIX)/plugins/*.la

	mkdir -p $(EKIGA_PREFIX)/plugins  # create directory, if does not exist
	for plugin in $(LIBDIR)/opal*/codecs/audio/*; do \
		cp -r $$plugin $(EKIGA_PREFIX)/plugins ;\
	done
	for plugin in $(LIBDIR)/opal*/codecs/video/*; do \
		cp -r $$plugin $(EKIGA_PREFIX)/plugins ;\
	done
	for lib in $(PTLIB_LIB) $(OPAL_LIB); do \
		cp -r $(LIBDIR)/$$lib $(EKIGA_PREFIX) ;\
	done
	for lib in libexpat-1.dll liblber.dll libldap.dll libspeex-1.dll libspeexdsp-1.dll libtheora-0.dll avcodec-54.dll avutil-52.dll libogg-0.dll \
		libclutter-1.0-0.dll libclutter-gtk-1.0-0.dll libcogl-pango-12.dll libcogl-12.dll libjson-glib-1.0-0.dll; do \
		cp -r $(BINDIR)/$$lib $(EKIGA_PREFIX) ;\
	done

	# gstreamer
	# for the moment, copy everything, except theora, which adds external dependencies (libtheoraenc-1.dll and libtheoradec-1.dll)
	cp $(BINDIR)/libgst*.dll $(EKIGA_PREFIX)
	mkdir -p $(EKIGA_PREFIX)/lib/gstreamer-1.0
	cp -r $(LIBDIR)/gstreamer-1.0/*.dll $(EKIGA_PREFIX)/lib/gstreamer-1.0
	rm $(EKIGA_PREFIX)/lib/gstreamer-1.0/libgsttheora.dll

	# locale for ekiga are not found if they are in /share
	mv $(EKIGA_PREFIX)/share/locale $(EKIGA_PREFIX)

	# libraries from GTK bundle
	for lib in libatk-1.0-0.dll libcairo-2.dll libcairo-gobject-2.dll libffi-6.dll libfreetype-6.dll libgdk_pixbuf-2.0-0.dll libgdk-3-0.dll libgio-2.0-0.dll libglib-2.0-0.dll libgmodule-2.0-0.dll libgobject-2.0-0.dll libgtk-3-0.dll libiconv-2.dll libintl-8.dll libpango-1.0-0.dll libpangocairo-1.0-0.dll libpangowin32-1.0-0.dll libpixman-1-0.dll libpng15-15.dll libxml2-2.dll pthreadGC2.dll zlib1.dll libfontconfig-1.dll libpangoft2-1.0-0.dll liblzma-5.dll; do \
		cp -r $(BINDIR)/$$lib $(EKIGA_PREFIX) ;\
	done
	cp -r $(BUILDROOT)/share/locale $(EKIGA_PREFIX)/share
	mkdir -p $(EKIGA_PREFIX)/share/glib-2.0; cp -r $(BUILDROOT)/share/glib-2.0/schemas $(EKIGA_PREFIX)/share/glib-2.0
	cd $(EKIGA_PREFIX); mv -f glib-2.0/schemas/* share/glib-2.0/schemas; rmdir glib-2.0/schemas; rmdir glib-2.0; glib-compile-schemas share/glib-2.0/schemas

ifeq ($(MINGW32), 1)
	cd $(EKIGA_PREFIX); cp /usr/share/doc/mingw32-runtime/mingwm10.dll.gz . && gunzip mingwm10.dll.gz
else
	cp /usr/lib/gcc/i686-w64-mingw32/4.9-win32/libgcc_s_sjlj-1.dll /usr/lib/gcc/i686-w64-mingw32/4.9-win32/libstdc++-6.dll /usr/i686-w64-mingw32/lib/libwinpthread-1.dll $(EKIGA_PREFIX)
endif

ifeq ($(DEBUG),0)
	$(STRIP) $(EKIGA_PREFIX)/*.dll
	$(STRIP) $(EKIGA_PREFIX)/*.exe
	$(STRIP) $(EKIGA_PREFIX)/plugins/*.dll
	$(STRIP) $(EKIGA_PREFIX)/plugins/*.exe
	$(STRIP) $(EKIGA_PREFIX)/lib/gstreamer-1.0/*.dll
endif
	# we cannot strip MS compiled dlls
	cp -r $(LIBSASL2_DIR)/bin/libsasl.dll $(EKIGA_PREFIX)

	# for RELEASE=0 ekiga.xml files do not exist, so create them
	D=$(DOC_LINGUAS); test -z "$(DOC_LINGUAS)" && D=`grep DOC_LINGUAS $(EKIGA_DIR)/help/Makefile.am | cut -d= -f2`; for lc in C $$D; do \
		mkdir -p $(EKIGA_PREFIX)/help/$$lc; \
		test $$lc != "C" -a ! -f $(EKIGA_DIR)/help/$$lc/ekiga.xml && xml2po -m docbook -e -p $(EKIGA_DIR)/help/$$lc/$$lc.po $(EKIGA_DIR)/help/C/ekiga.xml >$(EKIGA_DIR)/help/$$lc/ekiga.xml ; \
		if test -f $(EKIGA_DIR)/help/$$lc/ekiga.xml; then \
			xmlto -o $(EKIGA_PREFIX)/help/$$lc/ --skip-validation html $(EKIGA_DIR)/help/$$lc/ekiga.xml; \
		fi ;\
		cp -r $(EKIGA_DIR)/help/$$lc/ekiga.xml $(EKIGA_PREFIX)/help/$$lc;\
		if test -d $(EKIGA_DIR)/help/$$lc/figures; then \
			mkdir -p $(EKIGA_PREFIX)/help/$$lc/figures; \
			cp -r $(EKIGA_DIR)/help/$$lc/figures/*.png $(EKIGA_PREFIX)/help/$$lc/figures; \
		fi \
	done

	# install icons not found in GTK Windows file
# some icons are found inside (e.g. document-open-recent), others are found
#   in ekiga code (e.g. phone-pick-up); the others must be installed manually
# I still do not understand why document-open-recent is found and avatar-default is not (both are used similarly in ekiga code)
	for i in camera-web avatar-default input-dialpad user-away user-busy user-available user-offline x-office-address-book gtk-clear gtk-add gtk-remove computer network-server audio-input-microphone back forward gtk-close mail-forward exit \
	face-smile face-angel face-cool face-crying face-embarrassed face-devilish face-kiss face-monkey face-plain face-raspberry face-sad face-smile face-smile-big face-smirk face-surprise face-wink; do \
		for j in `echo /usr/share/icons/gnome/*/*/$${i}.png`; do \
			[ $$j != '/usr/share/icons/gnome/*/*/'$$i.png ] || (echo ERROR: icon $$i not found, exiting; exit 1); \
			dir=`echo $$j|cut -d/ -f6-7`; \
			mkdir -p $(EKIGA_PREFIX)/share/icons/hicolor/$$dir; \
			cp $$j $(EKIGA_PREFIX)/share/icons/hicolor/$$dir; \
		done; \
	done
	cp /usr/share/icons/gnome/index.theme $(EKIGA_PREFIX)/share/icons/hicolor
	touch $@

# Two additional plugins are needed from nsis:
# - System.dll, included in nsis distribution
# - Processes.dll, not included in nsis distribution
update-sources::
	@echo --- Getting NSIS Installer ...
	$(WGET) -P $(SRCDIR) $(NSIS_URL)/nsis-$(NSIS_VER).zip

$(NSISPLUGIN_DIR)/SystemLocal.dll: $(SRCDIR)/nsis-$(NSIS_VER).zip
	mkdir -p $(NSISPLUGIN_DIR)
	unzip -u -j $(SRCDIR)/nsis-$(NSIS_VER).zip nsis-$(NSIS_VER)/Plugins/System.dll -d $(NSISPLUGIN_DIR)
	$(MV) $(NSISPLUGIN_DIR)/System.dll $(NSISPLUGIN_DIR)/SystemLocal.dll
	touch $@

update-sources::
	@echo --- Getting NSIS Processes Plugin ...
	$(WGET) -P $(SRCDIR) $(NSISPROCESSES_URL)/Processes.zip

$(NSISPLUGIN_DIR)/Processes.dll: $(SRCDIR)/Processes.zip
	mkdir -p $(NSISPLUGIN_DIR)
	unzip -u -j $(SRCDIR)/Processes.zip setup-processes/bin/Processes.dll -d $(NSISPLUGIN_DIR)
	touch $@

$(TARGETDIR)/$(EKIGA_INSTALLER): $(TARGETDIR)/zips $(NSISPLUGIN_DIR)/SystemLocal.dll $(NSISPLUGIN_DIR)/Processes.dll
	makensis -DEKIGA_VERSION=$(EKIGA_VER) -DEKIGA_DIR=$(EKIGA_DIR) \
		-DINSTALLER_DIR=$(EKIGA_INSTALLER_DIR) -DLIB_DIR=$(LIBDIR) \
		-DTARGET_DIR=$(TARGETDIR) \
		-DNSISSYSTEMDIR=$(NSISSYTEM_DIR) -DNSISPLUGINDIR=$(NSISPLUGIN_DIR) \
		-DDEBUG=$(DEBUG) $(EKIGA_INSTALLER_DIR)/ekiga.nsi

clean:
	$(RM) -rf $(TARGETDIR)

distclean:
	$(RM) -rf $(OPENLDAP_DIR)
	$(RM) -rf $(LIBSASL2_DIR)
	$(RM) -rf $(BUILDROOT)/share/
	$(RM) -rf $(BINDIR)
	$(RM) -rf $(BUILDROOT)/etc
	$(RM) -rf $(INCLUDEDIR)
	$(RM) -rf $(LIBDIR)
	$(RM) -rf $(OGG_DIR)
	$(RM) -rf $(BUILDROOT)/manifest
	$(RM) -rf $(OPAL_DIR)
	$(RM) -rf $(EKIGA_DIR)
	$(RM) -rf $(PTLIB_DIR)
	$(RM) -rf $(REGEX_DIR)
	$(RM) -rf $(BOOST_DIR)
	$(RM) -rf $(SPEEX_DIR)
	$(RM) -rf $(EXPAT_DIR)
	$(RM) -rf $(LIBAV_DIR)
	$(RM) -rf $(X264_DIR)
	$(RM) -rf $(THEORA_DIR)
	$(RM) -rf $(CELT_DIR)
	$(RM) -rf $(COGL_DIR)
	$(RM) -rf $(JSONGLIB_DIR)
	$(RM) -rf $(CLUTTER_DIR)
	$(RM) -rf $(CLUTTERGTK_DIR)
	$(RM) -rf $(GSTREAMER_DIR)
	$(RM) -rf $(GSTPLUGINSBASE_DIR)
	$(RM) -rf $(BUILDROOT)/libexec
	$(RM) -rf $(CLUTTERGST_DIR)
	$(RM) -rf $(NSISPLUGIN_DIR)
	$(RM) -f binaries
#	$(RM) -rf $(SRCDIR)

.PHONY: clean distclean update update-sources
