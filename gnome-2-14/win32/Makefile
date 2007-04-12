#!/usr/bin/make -f
SILENT:=1
MAKEFLAGS:=-s
MAKELIBOPTS:=-S
MAKEOPTS:=$(MAKEFLAGS)
DEBUG=1

BUILDROOT = $(shell pwd | sed "s/\/ekiga-.*\/win32//")
TARGETDIR := $(BUILDROOT)/dist
SRCDIR = $(BUILDROOT)/src
INCLUDEDIR:=$(BUILDROOT)/include
LIBDIR = $(BUILDROOT)/lib
BINDIR = $(BUILDROOT)/bin

export DEB_BUILD_GNU_TYPE:=$(shell dpkg-architecture -qDEB_BUILD_GNU_TYPE)
export DEB_HOST_GNU_TYPE:="i586-pc-mingw32"

WGET := wget -nv -T 60 -N --no-proxy

HOST_TOOL_PREFIX:=i586-mingw32msvc

REALCC:=$(HOST_TOOL_PREFIX)-gcc
REALCXX:=$(HOST_TOOL_PREFIX)-g++
CONCURRENCY:=2

MACH_FLAGS:=-march=pentium-mmx
OPTI_FLAGS:=-O3 -fomit-frame-pointer
#	IX86_TYPE according to Intel
#	0 : 8086/88, 2 = 80286, 3 = 80386
#	4 = 80486, 5 = Pentium(R) Processor, 6 = PentiumPro(R) Processor
#	7 or higher = Processor beyond the PentiumPro6(R) Processor
IX86_TYPE:=5


ifeq ($(DEBUG),1)
	export CC=$(REALCC) -mms-bitfields -g 
	export CXX=$(REALCXX) -mms-bitfields -g
else
	export CC=$(REALCC) -mms-bitfields 
	export CXX=$(REALCXX) -mms-bitfields 
endif

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
export SDL_CONFIG:=i386-mingw32msvc-sdl-config
export CFLAGS:=-I$(INCLUDEDIR) $(MACH_FLAGS) -DPTRACING
export CXXFLAGS:=-I$(INCLUDEDIR) $(MACH_FLAGS) -DPTRACING
export LDFLAGS:=-L$(LIBDIR)  $(MACH_FLAGS)
export PKG_CONFIG_PATH:=$(LIBDIR)/pkgconfig
export PATH:=$(BINDIR):$(PATH)

EKIGA_ORIG_VER = $(shell pwd | sed "s/^.*\/ekiga-\(.*\)\/win32/\1/")
EKIGA_VER = $(EKIGA_ORIG_VER)-BETA

EKIGA_DIR = $(BUILDROOT)/ekiga-$(EKIGA_ORIG_VER)
EKIGA_ARCHIVE := ekiga-$(EKIGA_ORIG_VER).tar.gz
EKIGA_WIN32_DIR := $(PWD)
EKIGA_WIN32_DIFF_DIR := $(EKIGA_WIN32_DIR)/diff
EKIGA_INSTALLER := ekiga-setup-$(EKIGA_VER).exe
EKIGA_INSTALLER_DIR := $(EKIGA_WIN32_DIR)/nsisinstaller
#EKIGA_URL := http://www.ekiga.org/admin/downloads/latest/sources/sources
EKIGA_URL := http://www.ekiga.net/misc

OPAL_VER:= 2.2.7
OPAL_DIR:= $(BUILDROOT)/opal-$(OPAL_VER)
OPAL_ARCHIVE := opal-$(OPAL_VER).tar.gz

PWLIB_VER:= 1.10.6
PWLIB_DIR:= $(BUILDROOT)/pwlib-$(PWLIB_VER)
PWLIB_ARCHIVE := pwlib-$(PWLIB_VER).tar.gz

OPENLDAP_VER:= 2.3.19
OPENLDAP_DIR:= $(BUILDROOT)/openldap-$(OPENLDAP_VER)
OPENLDAP_ARCHIVE := openldap-$(OPENLDAP_VER).tgz
OPENLDAP_URL := http://www.openldap.org/software/download/OpenLDAP/openldap-release

OGG_VER:= 1.1.3
OGG_DIR:= $(BUILDROOT)/libogg-$(OGG_VER)
OGG_ARCHIVE := libogg-$(OGG_VER).tar.gz
OGG_URL := http://downloads.xiph.org/releases/ogg

SPEEX_VER:= 1.1.11.1
SPEEX_DIR:= $(BUILDROOT)/speex-$(SPEEX_VER)
SPEEX_ARCHIVE := speex-$(SPEEX_VER).tar.gz
SPEEX_URL := http://downloads.us.xiph.org/releases/speex

GTK_VER:= 2.8.9
GTK_URL := http://ftp.gtk.org/pub/gtk/v2.8/win32
GTK_ARCHIVE := gtk+-dev-$(GTK_VER).zip
GTK_BIN_ARCHIVE := gtk+-$(GTK_VER)-setup.zip
GTK_INSTALLER_URL := http://kent.dl.sourceforge.net/sourceforge/gimp-win
GTK_BIN_EXE := gtk+-$(GTK_VER)-setup.exe

GLIB_VER:= 2.8.5
GLIB_URL := $(GTK_URL)
GLIB_ARCHIVE := glib-dev-$(GLIB_VER).zip


ATK_VER:= 1.10.3
ATK_ARCHIVE := atk-dev-$(ATK_VER).zip
ATK_URL := $(GTK_URL)

CAIRO_VER:= 1.0.2
CAIRO_ARCHIVE := cairo-dev-$(CAIRO_VER).zip
CAIRO_URL := $(GTK_URL)

XML2_VER:= 2.6.23
XML2_DIR:= $(BUILDROOT)/libxml2-$(XML2_VER)
XML2_URL := ftp://xmlsoft.org/libxml2
XML2_ARCHIVE := libxml2-$(XML2_VER).tar.gz

PANGO_VER:= 1.10.1
PANGO_URL := $(GTK_URL)
PANGO_ARCHIVE := pango-dev-$(PANGO_VER).zip

LIBPNG_VER:= 1.2.8
GNUWIN32_URL := http://mesh.dl.sourceforge.net/gnuwin32

SDL_VER:= 1.2.9
SDL_DIR:= $(BUILDROOT)/SDL-$(SDL_VER)
SDL_URL := http://www.libsdl.org/release
SDL_ARCHIVE := SDL-devel-$(SDL_VER)-mingw32.tar.gz

ZLIB_VER:= 1.2.3
ZLIB_DIR:= $(BUILDROOT)/zlib-$(ZLIB_VER)

REGEXDIR := $(BUILDROOT)/regex
REGEX_ARCHIVE := regex.tar.gz

GIMP_URL := http://www.gimp.org/~tml/gimp/win32
GETTEXT_VER := 0.14.5
GETTEXT_ARCHIVE := gettext-dev-$(GETTEXT_VER).zip
ICONV_VER := 1.9.1
ICONV_ARCHIVE := libiconv-$(ICONV_VER).bin.woe32.zip

#DIRECTXDIR := $(BUILDROOT)/directx-dev
#DIRECTX_ARCHIVE := directx-dev.tar.gz

LN:=ln
RM:=rm
CP:=cp

DOC_LINGUAS=bg de es fr pt_BR sv uk

ifneq (,$(CONCURRENCY))
	MAKELIBOPTS+=-j$(CONCURRENCY)
endif

confflags:=--prefix=$(BUILDROOT) --build=$(DEB_BUILD_GNU_TYPE) --host=$(DEB_HOST_GNU_TYPE)
confekiga:= $(confflags) --with-pwlib-dir=$(BUILDROOT) --with-opal-dir=$(BUILDROOT) \
	--prefix=$(TARGETDIR) --bindir=$(TARGETDIR) --datadir=$(TARGETDIR) \
	--sysconfdir=$(TARGETDIR) --libdir=$(TARGETDIR) --disable-gnome \
	--disable-dbus --disable-avahi --disable-esd


all: binaries $(TARGETDIR)/$(EKIGA_INSTALLER)
update:
	$(WGET) -P $(SRCDIR) $(EKIGA_URL)/$(PWLIB_ARCHIVE)
	$(WGET) -P $(SRCDIR) $(EKIGA_URL)/$(OPAL_ARCHIVE)
	
binaries:
	echo Checking prerequisites...
	hash $(REALCC) $(REALCXX) $(LD) $(NM) $(AR) $(RANLIB) $(DLLTOOL) $(DLLWRAP) $(OBJDUMP) $(WINDRES) $(AS)||(echo You need to install mingw32;exit 1)
	hash tar unzip zip wget aclocal-1.9 autoheader libtoolize automake-1.9 autoconf pkg-config gnome-autogen.sh bison gdk-pixbuf-csource||(echo we need more binaries ;exit 1)
	hash makensis||(echo You need to install nsis;exit1)
	hash xmlto||(echo You need to install xmlto;exit1)
	
	[ -f /usr/share/gnome-doc-utils/gnome-doc-utils.make ]||(echo gnome-doc-tools not installed;exit 1)
	[ -f /usr/lib/pkgconfig/gconf-2.0.pc ]||(echo libgconf2-dev not installed;exit 1)
	[ -f /usr/lib/pkgconfig/avahi-glib.pc ]||(echo libavahi-glib-dev not installed;exit 1)
	[ -f /usr/lib/pkgconfig/avahi-client.pc ]||(echo libavahi-client-dev not installed;exit 1)
#	[ $(BUILDROOT) = $(PWD) ]||(echo You are supposed to run this as Makefile in $(BUILDROOT);exit 1)
	
	mkdir -p $(INCLUDEDIR) $(LIBDIR)/pkgconfig $(SRCDIR) $(BINDIR)
	$(RM) -f $(BINDIR)/cygpath
	ln -s /bin/true $(BINDIR)/cygpath
	touch $@

### REGEX

update-source:: $(SRCDIR)/$(REGEX_ARCHIVE)
$(SRCDIR)/$(REGEX_ARCHIVE):
	echo "--- Getting libregex..."
	rm -fr $(LIBDIR)/libregex.a $(REGEXDIR)/regex.h $(INCLUDEDIR)/regex.h
	mkdir -p $(REGEXDIR)
	cd $(REGEXDIR)/;for i in regex.c regexec.c regex.h regex_internal.c regex_internal.h regcomp.c alloca_.h alloca.c strcase.h localcharset.c localcharset.h;do \
		$(WGET) http://cvs.savannah.nongnu.org/viewcvs/*checkout*/gnulib/gnulib/lib/$$i ;\
	done
	mv $(REGEXDIR)/alloca_.h $(REGEXDIR)/alloca.h
	echo '//' >> $(REGEXDIR)/config.h
	(cd $(BUILDROOT); \
	 tar cfz $(SRCDIR)/$(REGEX_ARCHIVE) regex;)
	rm -rf $(REGEXDIR)
	touch $@

$(REGEXDIR)/regex.h: $(SRCDIR)/$(REGEX_ARCHIVE)
	rm -f $(LIBDIR)/libregex.a $(INCLUDEDIR)/regex.h
	(cd $(BUILDROOT); \
	 tar xfz $(SRCDIR)/$(REGEX_ARCHIVE))
	touch $@

$(INCLUDEDIR)/regex.h: $(REGEXDIR)/regex.h
	rm -f $(LIBDIR)/libregex.a
	@(cp $(REGEXDIR)/regex.h $(INCLUDEDIR);cp $(REGEXDIR)/alloca.h $(INCLUDEDIR);cp $(REGEXDIR)/localcharset.h $(INCLUDEDIR))||\
	(echo Copy regex.h, localcharset.h and alloca.h to $(INCLUDEDIR), and try again !;exit 1)
	touch $@

$(LIBDIR)/libregex.a: binaries $(INCLUDEDIR)/regex.h $(REGEXDIR)/regex.h
	rm -f $(LIBDIR)/libregex.a
	cd $(REGEXDIR) ;\
	$(CC) -I$(INCLUDEDIR) -I. -c regex.c ;\
	$(CC) -I$(INCLUDEDIR) -I. -c alloca.c ;\
	$(CC) -I$(INCLUDEDIR) -I. -DLIBDIR=$(INCLUDEDIR) -c localcharset.c ;\
	$(AR) r libregex.a regex.o alloca.o localcharset.o
	cp $(REGEXDIR)/libregex.a $(LIBDIR)
	touch $@


### OPENLDAP
update-sources:: $(SRCDIR)/$(OPENLDAP_ARCHIVE)
$(SRCDIR)/$(OPENLDAP_ARCHIVE): binaries
	echo --- Getting OpenLDAP...
	$(WGET) -P $(SRCDIR) $(OPENLDAP_URL)/$(OPENLDAP_ARCHIVE)
	touch $@

$(OPENLDAP_DIR)/configure: $(LIBDIR)/libregex.a $(SRCDIR)/$(OPENLDAP_ARCHIVE)
	rm -f $(LIBDIR)/libldap_r.dll
	tar xfz $(SRCDIR)/$(OPENLDAP_ARCHIVE) -C $(BUILDROOT)
	sed -i -e 's#windres#$(WINDRES)#g' $(OPENLDAP_DIR)/libraries/liblutil/Makefile.in
	touch $@

$(OPENLDAP_DIR)/config.status: binaries $(OPENLDAP_DIR)/configure
	#-$(MAKE) -C $(OPENLDAP_DIR) clean
	rm -f $(LIBDIR)/libldap_r.dll
	ln -sf $(INCLUDEDIR)/regex.h $(OPENLDAP_DIR)/include/
	cd $(OPENLDAP_DIR);./configure --with-cyrus-sasl=no --enable-bdb=no --enable-hdb=no $(confflags)
	$(MAKE) $(MAKEOPTS) -C $(OPENLDAP_DIR) depend
	touch $@

$(LIBDIR)/libldap_r.dll: binaries $(OPENLDAP_DIR)/config.status
	$(MAKE) $(MAKEOPTS) -C $(OPENLDAP_DIR)/libraries/liblutil
	$(CP) -f $(OPENLDAP_DIR)/libraries/liblutil/liblutil.a $(LIBDIR)
	$(MAKE) -C $(OPENLDAP_DIR)/libraries/liblber all install 
	$(MAKE) -C $(OPENLDAP_DIR)/include install
	$(MAKE) -C $(OPENLDAP_DIR)/libraries/libldap MOD_LIBS="-llutil" all install
	$(MAKE) -C $(OPENLDAP_DIR)/libraries/libldap_r MOD_LIBS="-llutil" all install
	touch $@

### PWLib
update-sources:: $(SRCDIR)/$(PWLIB_ARCHIVE)
$(SRCDIR)/$(PWLIB_ARCHIVE): binaries 
	echo --- Getting PWLib ...
	$(WGET) -P $(SRCDIR) $(EKIGA_URL)/$(PWLIB_ARCHIVE)
	touch $@

$(PWLIB_DIR)/configure: binaries $(LIBDIR)/libldap_r.dll $(LIBDIR)/libogg.a $(LIBDIR)/libspeex.a $(SRCDIR)/$(PWLIB_ARCHIVE) $(EKIGA_WIN32_DIFF_DIR)/ptlib_Makefile.am $(EKIGA_WIN32_DIFF_DIR)/ptlib_configure.ac $(EKIGA_WIN32_DIFF_DIR)/ptlib_pwlib.pc.in $(EKIGA_WIN32_DIFF_DIR)/ptlib_vfw.patch 
	rm -f $(LIBDIR)/libpt.a
	rm -rf $(PWLIB_DIR)
	tar xfz $(SRCDIR)/$(PWLIB_ARCHIVE) -C $(BUILDROOT)
	$(RM) -f $(PWLIB_DIR)/configure.exe $(PWLIB_DIR)/configure $(PWLIB_DIR)/configure.ac $(PWLIB_DIR)/Makefile.in
	$(CP) -fl $(EKIGA_WIN32_DIFF_DIR)/ptlib_configure.ac $(PWLIB_DIR)/configure.ac
	$(CP) -fl $(EKIGA_WIN32_DIFF_DIR)/ptlib_Makefile.am $(PWLIB_DIR)/Makefile.am
	$(CP) -fl $(EKIGA_WIN32_DIFF_DIR)/ptlib_pwlib.pc.in $(PWLIB_DIR)/pwlib.pc.in
	
	(cd $(PWLIB_DIR); \
	patch -p1 < $(EKIGA_WIN32_DIFF_DIR)/ptlib_vfw.patch ;\
	sed -i "s/\$${PWLIB_VER}/${PWLIB_VER}/" configure.ac; \
	aclocal-1.9; autoheader; libtoolize --force; \
	touch NEWS README AUTHORS ChangeLog unused.h.in; \
	automake-1.9 -a -c; autoconf; \
	)

$(PWLIB_DIR)/config.status: binaries $(PWLIB_DIR)/configure
	rm -f $(LIBDIR)/libpt.a
	(cd $(PWLIB_DIR)/;./configure $(confflags) )

$(LIBDIR)/libpt.a: binaries $(PWLIB_DIR)/config.status
	$(MAKE) $(MAKELIBOPTS) -C $(PWLIB_DIR)
	$(MAKE) -C $(PWLIB_DIR) install
	sed -i -e 's#^prefix=.*$$#prefix=$(BUILDROOT)#g' $(LIBDIR)/pkgconfig/pwlib.pc

### libogg
update-sources:: $(SRCDIR)/$(OGG_ARCHIVE)
$(SRCDIR)/$(OGG_ARCHIVE): binaries
	echo --- Getting Ogg ...
	$(WGET) -P $(SRCDIR) $(OGG_URL)/$(OGG_ARCHIVE)
	touch $@

$(OGG_DIR)/config.status: binaries $(SRCDIR)/$(OGG_ARCHIVE)
	rm -f $(LIBDIR)/libogg.a
	tar xfz $(SRCDIR)/$(OGG_ARCHIVE) -C $(BUILDROOT)
	(cd $(OGG_DIR)/;./configure $(confflags) \
	)

$(LIBDIR)/libogg.a: binaries $(OGG_DIR)/config.status
	rm -f $(LIBDIR)/libogg.a
	$(MAKE) $(MAKELIBOPTS) -C $(OGG_DIR)
	$(MAKE) -C $(OGG_DIR) install

### libspeex
update-sources:: $(SRCDIR)/$(SPEEX_ARCHIVE)
$(SRCDIR)/$(SPEEX_ARCHIVE): binaries
	echo --- Getting Vorbis ...
	$(WGET) -P $(SRCDIR) $(SPEEX_URL)/$(SPEEX_ARCHIVE)
	touch $@

$(SPEEX_DIR)/config.status: binaries $(SRCDIR)/$(SPEEX_ARCHIVE)
	rm -f $(LIBDIR)/_ibspeex.a
	tar xfz $(SRCDIR)/$(SPEEX_ARCHIVE) -C $(BUILDROOT)
	(cd $(SPEEX_DIR)/;LDFLAGS="$$LDFLAGS -no-undefined" ./configure $(confflags) \
	)

$(LIBDIR)/libspeex.a: binaries $(SPEEX_DIR)/config.status
	rm -f $(LIBDIR)/libspeex.a
	$(MAKE) $(MAKEOPTS) -C $(SPEEX_DIR)
	$(MAKE) -C $(SPEEX_DIR) install

### OPAL
update-sources:: $(SRCDIR)/$(OPAL_ARCHIVE)
$(SRCDIR)/$(OPAL_ARCHIVE): binaries
	echo --- Getting OPAL ...
	$(WGET) -P $(SRCDIR) $(EKIGA_URL)/$(OPAL_ARCHIVE)
	touch $@

$(OPAL_DIR)/configure: binaries $(LIBDIR)/libogg.a $(LIBDIR)/libspeex.a $(LIBDIR)/libpt.a $(SRCDIR)/$(OPAL_ARCHIVE) $(EKIGA_WIN32_DIFF_DIR)/opal_configure.ac $(EKIGA_WIN32_DIFF_DIR)/opal_Makefile.am $(EKIGA_WIN32_DIFF_DIR)/opal_opal.pc.in 
	rm -rf $(OPAL_DIR) $(LIBDIR)/libopal.a
	tar xfz $(SRCDIR)/$(OPAL_ARCHIVE) -C $(BUILDROOT)
	$(RM) -f $(OPAL_DIR)/configure.exe $(OPAL_DIR)/configure $(OPAL_DIR)/configure.ac $(OPAL_DIR)/Makefile.in
	$(CP) -fl $(EKIGA_WIN32_DIFF_DIR)/opal_configure.ac $(OPAL_DIR)/configure.ac
	$(CP) -fl $(EKIGA_WIN32_DIFF_DIR)/opal_Makefile.am $(OPAL_DIR)/Makefile.am
	$(CP) -fl $(EKIGA_WIN32_DIFF_DIR)/opal_opal.pc.in $(OPAL_DIR)/opal.pc.in
	
	(cd $(OPAL_DIR); \
	sed -i "s/\$${OPAL_VER}/${OPAL_VER}/" configure.ac; \
	aclocal-1.9; autoheader; libtoolize --force; \
	touch NEWS README AUTHORS ChangeLog unused.h.in; \
	automake-1.9 -a -c; autoconf; \
	)

$(OPAL_DIR)/config.status: binaries $(OPAL_DIR)/configure
	rm -f $(LIBDIR)/libopal.a
	(cd $(OPAL_DIR)/; CXXFLAGS="$(CXXFLAGS) -DPTRACING" ; \
	./configure $(confflags) \
	)

$(LIBDIR)/libopal.a: binaries $(OPAL_DIR)/config.status
	rm -f $(LIBDIR)/libopal.a
	$(MAKE) $(MAKELIBOPTS) -C $(OPAL_DIR) 
	$(MAKE) -C $(OPAL_DIR) install 
	sed -i -e 's#^prefix=.*$$#prefix=$(BUILDROOT)#g' $(LIBDIR)/pkgconfig/opal*.pc
	
### libgtk
update-sources:: $(SRCDIR)/$(GTK_ARCHIVE)
$(SRCDIR)/$(GTK_ARCHIVE): binaries
	echo --- Getting GTK2+ ...
	$(WGET) -P $(SRCDIR) $(GTK_URL)/$(GTK_ARCHIVE)
	touch $@

$(LIBDIR)/pkgconfig/gtk+-2.0.pc: $(SRCDIR)/$(GTK_ARCHIVE)
	unzip -u $(SRCDIR)/$(GTK_ARCHIVE) -d $(BUILDROOT)
	sed -i -e 's#^prefix=.*$$#prefix=$(BUILDROOT)#g' $(LIBDIR)/pkgconfig/g*.pc
	touch $@

### GTK Runtime
update-sources:: $(SRCDIR)/$(GTK_BIN_ARCHIVE)
$(SRCDIR)/$(GTK_BIN_ARCHIVE) :
	echo --- Getting GTK2+ Runtime Library ...
	$(WGET) -P $(SRCDIR) $(GTK_INSTALLER_URL)/$(GTK_BIN_ARCHIVE)
	touch $@

$(LIBDIR)/$(GTK_BIN_EXE): $(SRCDIR)/$(GTK_BIN_ARCHIVE)
	unzip $(SRCDIR)/$(GTK_BIN_ARCHIVE) -d $(LIBDIR)
	touch $@

### libglib
update-sources:: $(SRCDIR)/$(GLIB_ARCHIVE)
$(SRCDIR)/$(GLIB_ARCHIVE): binaries
	echo --- Getting GLib ...
	$(WGET) -P $(SRCDIR) $(GLIB_URL)/$(GLIB_ARCHIVE)
	touch $@

$(LIBDIR)/pkgconfig/glib-2.0.pc: $(SRCDIR)/$(GLIB_ARCHIVE)
	unzip -u $(SRCDIR)/$(GLIB_ARCHIVE) -d $(BUILDROOT)
	sed -i -e 's#^prefix=.*$$#prefix=$(BUILDROOT)#g' $(LIBDIR)/pkgconfig/g*.pc $(BINDIR)/glib-gettextize
	chmod +x $(BINDIR)/glib-gettextize
	touch $@

### atk
update-sources:: $(SRCDIR)/$(ATK_ARCHIVE)
$(SRCDIR)/$(ATK_ARCHIVE): binaries
	echo --- Getting ATK ...
	$(WGET) -P  $(SRCDIR) $(ATK_URL)/$(ATK_ARCHIVE)
	touch $@

$(LIBDIR)/pkgconfig/atk-2.0.pc: $(SRCDIR)/$(ATK_ARCHIVE)
	unzip -u $(SRCDIR)/$(ATK_ARCHIVE) -d $(BUILDROOT)
	sed -i -e 's#^prefix=.*$$#prefix=$(BUILDROOT)#g' $(LIBDIR)/pkgconfig/atk*.pc
	touch $@

### libpng
update-sources:: $(SRCDIR)/stamp_png
$(SRCDIR)/stamp_png: binaries
	echo --- Getting PNG ...
	$(WGET) -P $(SRCDIR) $(GNUWIN32_URL)/libpng-$(LIBPNG_VER)-bin.zip
	$(WGET) -P $(SRCDIR) $(GNUWIN32_URL)/libpng-$(LIBPNG_VER)-dep.zip
	$(WGET) -P $(SRCDIR) $(GNUWIN32_URL)/libpng-$(LIBPNG_VER)-lib.zip
	touch $@

$(LIBDIR)/libpng.a: $(SRCDIR)/stamp_png
	unzip -u $(SRCDIR)/libpng-$(LIBPNG_VER)-bin.zip -d $(BUILDROOT)
	unzip -u $(SRCDIR)/libpng-$(LIBPNG_VER)-dep.zip -d $(BUILDROOT)
	unzip -u $(SRCDIR)/libpng-$(LIBPNG_VER)-lib.zip -d $(BUILDROOT)
	#sed -i -e 's#^prefix=.*$$#prefix=$(BUILDROOT)#g' lib/pkgconfig/libpng*.pc
	touch $@

### libintl
update-sources:: $(SRCDIR)/$(GETTEXT_ARCHIVE)
$(SRCDIR)/$(GETTEXT_ARCHIVE): binaries
	echo --- Getting Gettext ...
	$(WGET) -P $(SRCDIR) $(GIMP_URL)/$(GETTEXT_ARCHIVE)
	touch $@

$(LIBDIR)/libintl.a: $(SRCDIR)/$(GETTEXT_ARCHIVE)
	unzip -u $(SRCDIR)/$(GETTEXT_ARCHIVE) -d $(BUILDROOT)
	touch $@

### libiconv
update-sources:: $(SRCDIR)/$(ICONV_ARCHIVE)
$(SRCDIR)/$(ICONV_ARCHIVE): binaries
	echo --- Getting Iconv ...
	$(WGET) -P $(SRCDIR) $(GIMP_URL)/$(ICONV_ARCHIVE)
	touch $@

$(LIBDIR)/libiconv.a: $(SRCDIR)/$(ICONV_ARCHIVE)
	unzip -u $(SRCDIR)/$(ICONV_ARCHIVE) -d $(BUILDROOT)
	$(RM) -f README.libiconv
	touch $@

### pango
update-sources:: $(SRCDIR)/$(PANGO_ARCHIVE)
$(SRCDIR)/$(PANGO_ARCHIVE): binaries
	echo --- Getting Pango ...
	$(WGET) -P $(SRCDIR) $(PANGO_URL)/$(PANGO_ARCHIVE)
	touch $@

$(LIBDIR)/pkgconfig/pango32.pc: $(SRCDIR)/$(PANGO_ARCHIVE)
	unzip -u $(SRCDIR)/$(PANGO_ARCHIVE) -d $(BUILDROOT)
	sed -i -e 's#^prefix=.*$$#prefix=$(BUILDROOT)#g' $(LIBDIR)/pkgconfig/pango*.pc
	touch $@

### cairo
update-sources:: $(SRCDIR)/$(CAIRO_ARCHIVE)
$(SRCDIR)/$(CAIRO_ARCHIVE): binaries
	echo --- Getting Cairo ...
	$(WGET) -P $(SRCDIR) $(CAIRO_URL)/$(CAIRO_ARCHIVE)
	touch $@

$(LIBDIR)/pkgconfig/cairo.pc: $(SRCDIR)/$(CAIRO_ARCHIVE)
	unzip -u $(SRCDIR)/$(CAIRO_ARCHIVE) -d $(BUILDROOT)
	sed -i -e 's#^prefix=.*$$#prefix=$(BUILDROOT)#g' $(LIBDIR)/pkgconfig/cairo*.pc
	touch $@

### libxml2
update-sources:: $(SRCDIR)/$(XML2_ARCHIVE)
$(SRCDIR)/$(XML2_ARCHIVE): binaries
	echo --- Getting libXML2 ...
	$(WGET) -P $(SRCDIR) $(XML2_URL)/$(XML2_ARCHIVE)
	touch $@

$(XML2_DIR)/config.status: binaries $(SRCDIR)/$(XML2_ARCHIVE)
	rm -f $(LIBDIR)/libxml2.a
	tar xfz $(SRCDIR)/$(XML2_ARCHIVE) -C $(BUILDROOT)
	(cd $(XML2_DIR)/;./configure $(confflags) --without-python \
	)

$(LIBDIR)/libxml2.a: binaries $(XML2_DIR)/config.status
	rm -f $(LIBDIR)/libxml2.a
	$(MAKE) $(MAKELIBOPTS) -C $(XML2_DIR)
	$(MAKE) $(MAKEOPTS) -C $(XML2_DIR) install

### libsdl
update-sources:: $(SRCDIR)/$(SDL_ARCHIVE)
$(SRCDIR)/$(SDL_ARCHIVE): binaries
	echo --- Getting libSDL ...
	$(WGET) -P $(SRCDIR) $(SDL_URL)/$(SDL_ARCHIVE)
	touch $@

$(LIBDIR)/libSDL.dll.a: binaries $(SRCDIR)/$(SDL_ARCHIVE)
	rm -f $(LIBDIR)/libSDL.dll.a
	tar xfz $(SRCDIR)/$(SDL_ARCHIVE) -C $(BUILDROOT)
	cp -r $(SDL_DIR)/bin $(SDL_DIR)/lib $(SDL_DIR)/include $(SDL_DIR)/share $(BUILDROOT)
	sed -i -e 's#^prefix=.*$$#prefix=$(BUILDROOT)#g' $(BINDIR)/i386-mingw32msvc-sdl-config
	chmod +x $(BINDIR)/i386-mingw32msvc-sdl-config
	ln -sf i386-mingw32msvc-sdl-config $(BINDIR)/sdl-config
	sed -i -e 's#^libdir=.*$$#libdir=$(BUILDROOT)/lib#g' -e 's#$$##' $(LIBDIR)/libSDL.la

### DirectX
#$(DIRECTXDIR)/stamp_directx: $(SRCDIR)/$(DIRECTX_ARCHIVE)
#	tar xzvf $(SRCDIR)/$(DIRECTX_ARCHIVE) -d $(BUILDROOT) 
#	touch $@

#$(LIBDIR)/directx/dsound.lib: $(DIRECTXDIR)/stamp_directx
#	mkdir -p $(LIBDIR)/directx
#	for lib in $(DIRECTXDIR)/lib/x86/*lib; do
#		install -m 755 $$lib $(LIBDIR)/directx/
#	done
#	touch $@

#$(INCLUDEDIR)/directx/dsound.h: $(DIRECTXDIR)/stamp_directx 
#	mkdir -p $(INCLUDEDIR)/directx
#	for header in $(DIRECTXDIR)/include/*h; do
#		cp $$header $(INCLUDEDIR)/directx/
#	done
#	touch $@

### GnomeMeeting / Ekiga

$(EKIGA_DIR)/autogen.sh: binaries $(EKIGA_WIN32_DIFF_DIR)/ekiga_pkgconfig.patch $(EKIGA_WIN32_DIFF_DIR)/ekiga.rc $(EKIGA_WIN32_DIFF_DIR)/Makefile.ekiga-win32
	(cd $(EKIGA_DIR);\
	 patch -p1 < $(EKIGA_WIN32_DIFF_DIR)/ekiga_pkgconfig.patch;\
	 patch -p1 < $(EKIGA_WIN32_DIFF_DIR)/ekiga_configure.diff; \
	 sed -i "s/${EKIGA_ORIG_VER}/${EKIGA_VER}/" $(EKIGA_DIR)/configure;)
#Not need as it will be set durring configure
#	 sed -i "s/^PACKAGE_STRING = ${EKIGA_ORIG_VER}/PACKAGE_STRING = ${EKIGA_VER}/" $(EKIGA_DIR)/Makefile; 
#	 sed -i "s/^PACKAGE_VERSION = ${EKIGA_ORIG_VER}/PACKAGE_VERSION = ${EKIGA_VER}/" $(EKIGA_DIR)/Makefile; 
#	 sed -i "s/^VERSION = ${EKIGA_ORIG_VER}/VERSION = ${EKIGA_VER}/" $(EKIGA_DIR)/Makefile;)
	#### we would need to immport these still...
	# [X] icon we'll generate live
	#cp -r figures-thesi $(EKIGA_DIR)/help/de/figures
	touch $@

$(EKIGA_DIR)/config.status: binaries $(EKIGA_DIR)/autogen.sh $(LIBDIR)/libpt.a $(LIBDIR)/libopal.a \
	$(LIBDIR)/pkgconfig/gtk+-2.0.pc $(LIBDIR)/pkgconfig/atk-2.0.pc $(LIBDIR)/pkgconfig/glib-2.0.pc \
	$(LIBDIR)/libpng.a $(LIBDIR)/pkgconfig/pango32.pc $(LIBDIR)/pkgconfig/cairo.pc $(LIBDIR)/libxml2.a \
	$(LIBDIR)/libintl.a $(LIBDIR)/libiconv.a $(LIBDIR)/libSDL.dll.a
	rm -f $(EKIGA_DIR)/src/ekiga.exe
	(cd $(EKIGA_DIR)/; ACLOCAL_FLAGS="-I $(BUILDROOT)/share/aclocal" \
	./configure $(confekiga) )

$(EKIGA_DIR)/src/ekiga.exe: binaries $(EKIGA_DIR)/config.status
	$(WINDRES) ekiga.rc ekiga_rc.o
	$(MAKE) $(MAKEOPTS) -C $(EKIGA_DIR)
	$(MAKE) $(MAKEOPTS) -C $(EKIGA_DIR)/help

$(TARGETDIR)/zips: binaries $(EKIGA_DIR)/src/ekiga.exe
	-$(RM) -rf $(TARGETDIR)

	mkdir -p $(TARGETDIR)/Ekiga
	install -m 755 $(EKIGA_DIR)/src/ekiga.exe $(TARGETDIR)/Ekiga/
	for lib in liblber.dll libldap_r.dll libxml2-2.dll libspeex-1.dll ; do \
	install -m 755 $(BINDIR)/$$lib $(TARGETDIR)/Ekiga ;\
	done

ifeq ($(DEBUG),0)
	$(STRIP) $(TARGETDIR)/Ekiga/*
endif

	mkdir -p $(TARGETDIR)/Ekiga/ekiga \
		$(TARGETDIR)/Ekiga/sounds/ekiga \
		$(TARGETDIR)/Ekiga/pixmaps/ekiga
	install -m 644 $(EKIGA_DIR)/ekiga.schemas $(TARGETDIR)/Ekiga/ekiga
	install -m 644 $(EKIGA_DIR)/sounds/*.wav $(TARGETDIR)/Ekiga/sounds/ekiga
	install -m 644 $(EKIGA_DIR)/pixmaps/ekiga-logo.png $(TARGETDIR)/Ekiga/pixmaps/ekiga
	install -m 644 $(EKIGA_DIR)/pixmaps/ekiga.png $(TARGETDIR)/Ekiga/pixmaps/

	for loc in `ls $(EKIGA_DIR)/po/*.gmo | sed -e "s#$(EKIGA_DIR)\/po\/##" -e "s#.gmo##"`; do \
	dir=$(TARGETDIR)/Ekiga/locale/$$loc/LC_MESSAGES;\
	mkdir -p $$dir;\
	cp $(EKIGA_DIR)/po/$$loc.gmo $$dir/ekiga.mo;\
	done

	for lc in C $(DOC_LINGUAS); do \
		mkdir -p $(TARGETDIR)/Ekiga/help/$$lc; \
		if test -f $(EKIGA_DIR)/help/$$lc/ekiga.xml; then \
			xmlto -o $(TARGETDIR)/Ekiga/help/$$lc/ --skip-validation html $(EKIGA_DIR)/help/$$lc/ekiga.xml; \
		fi ;\
		install -m 644 $(EKIGA_DIR)/help/$$lc/ekiga.xml $(TARGETDIR)/Ekiga/help/$$lc/;\
		if test -d $(EKIGA_DIR)/help/$$lc/figures; then \
			mkdir -p $(TARGETDIR)/Ekiga/help/$$lc/figures; \
			install -m 644 $(EKIGA_DIR)/help/$$lc/figures/*.png $(TARGETDIR)/Ekiga/help/$$lc/figures; \
		fi \
	done
	(cd $(TARGETDIR)/;tar cfvz ekiga_help.tar.gz Ekiga/help)
	touch $@


$(TARGETDIR)/$(EKIGA_INSTALLER): $(TARGETDIR)/zips $(LIBDIR)/$(GTK_BIN_EXE) $(EKIGA_INSTALLER_DIR)/ekiga.nsi 
	(makensis -DEKIGA_VERSION=$(EKIGA_VER) -DEKIGA_DIR=$(EKIGA_DIR) -DBUILDROOT=$(BUILDROOT) \
	 -DINSTALLER_DIR=$(EKIGA_INSTALLER_DIR) -DLIB_DIR=$(LIBDIR) -DTARGET_DIR=$(TARGETDIR) \
	 -DCROSS_COMPILING=true -DWITH_GTK=true -DGTK_VERSION=$(GTK_VER) \
	 $(EKIGA_INSTALLER_DIR)/ekiga.nsi )
	touch $@


clean:
	-$(RM) -rf $(TARGETDIR)


$(TARGETDIR)clean: clean
	-$(RM) -rf $(OPENLDAP_DIR)
	-$(RM) -rf $(LIBDIR)/libspeex* $(BUILDROOT)/share/
	-$(RM) -rf $(BINDIR)
	-$(RM) -rf $(BUILDROOT)/contrib
	-$(RM) -rf $(BUILDROOT)/etc
	-$(RM) -rf $(INCLUDEDIR)
	-$(RM) -rf $(LIBDIR)
	-$(RM) -rf $(OGG_DIR)
	-$(RM) -rf $(XML2_DIR)
	-$(RM) -rf $(BUILDROOT)/man
	-$(RM) -rf $(BUILDROOT)/manifest
	-$(RM) -rf $(OPAL_DIR)
	-$(RM) -rf $(PWLIB_DIR)
	-$(RM) -rf $(REGEXDIR)
	-$(RM) -rf $(SPEEX_DIR)
	-$(RM) -rf $(SDL_DIR)
	-$(RM) -f binaries
	-$(RM) -f $(SRCDIR)/*


.PHONY: clean distclean update betarelease update-sources
