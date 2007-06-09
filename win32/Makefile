#!/usr/bin/make -f

SILENT:=1
MAKEFLAGS:=-s
MAKEOPTS:=$(MAKEFLAGS)
MAKE_LIB_OPTS:=$(MAKEFLAGS)
DEBUG=1

BUILDROOT = $(shell pwd)
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
CONCURRENCY:=4

MACH_FLAGS:=-march=pentium-mmx
OPTI_FLAGS:=-O3 -fomit-frame-pointer
#	IX86_TYPE according to Intel
#	0 : 8086/88, 2 = 80286, 3 = 80386
#	4 = 80486, 5 = Pentium(R) Processor, 6 = PentiumPro(R) Processor
#       7 or higher = Processor beyond the PentiumPro6(R) Processor
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
export CFLAGS:=-I$(INCLUDEDIR) $(MACH_FLAGS) -DPTRACING
export CXXFLAGS:=-I$(INCLUDEDIR) $(MACH_FLAGS) -DPTRACING
export LDFLAGS:=-L$(LIBDIR)  $(MACH_FLAGS)
export PKG_CONFIG_PATH:=$(LIBDIR)/pkgconfig
export PATH:=$(BINDIR):$(PATH)


EKIGA_VER = cvs-$(shell date +"%Y%m%d")
EKIGA_DIR = $(BUILDROOT)/ekiga
EKIGA_PREFIX:= $(TARGETDIR)/Ekiga
EKIGA_ARCHIVE := ekiga-snapshot.tar.gz
EKIGA_WIN32_DIR := $(PWD)
EKIGA_WIN32_DIFF_DIR := $(EKIGA_WIN32_DIR)/diff
EKIGA_INSTALLER_DIR := $(BUILDROOT)/nsisinstaller
EKIGA_INSTALLER := ekiga-setup-${EKIGA_VER}.exe
EKIGA_URL1 := http://snapshots.buildserver.net/cvs
EKIGA_URL2 := http://snapshots.voxgratia.org/cvs

OPAL_VER:= 2.3.2
OPAL_DIR:= $(BUILDROOT)/opal
OPAL_ARCHIVE := opal-cvs.tar.gz

PWLIB_VER:= 1.11.3
PWLIB_DIR:= $(BUILDROOT)/pwlib
PWLIB_ARCHIVE := pwlib-cvs.tar.gz

OPENLDAP_VER := 2.3.28
OPENLDAP_DIR := $(BUILDROOT)/openldap-$(OPENLDAP_VER)
OPENLDAP_ARCHIVE := openldap-$(OPENLDAP_VER).tgz
OPENLDAP_URL := http://www.openldap.org/software/download/OpenLDAP/openldap-release

OGG_VER := 1.1.3
OGG_DIR := $(BUILDROOT)/libogg-$(OGG_VER)
OGG_ARCHIVE := libogg-$(OGG_VER).tar.gz
OGG_URL := http://downloads.xiph.org/releases/ogg

SPEEX_VER := 1.1.12
SPEEX_DIR := $(BUILDROOT)/speex-$(SPEEX_VER)
SPEEX_ARCHIVE := speex-$(SPEEX_VER).tar.gz
SPEEX_URL := http://downloads.us.xiph.org/releases/speex

GTK_VER := 2.10.11
GTK_URL := http://ftp.gtk.org/pub/gtk/v2.10/win32
GTK_ARCHIVE := gtk+-dev-$(GTK_VER).zip
GTK_BIN_VER := 2.10.11
GTK_BIN_ARCHIVE := gtk+-$(GTK_BIN_VER)-setup.zip 	 
GTK_INSTALLER_URL := http://www.mirrorservice.org/sites/download.sourceforge.net/pub/sourceforge/g/gi/gimp-win
GTK_BIN_EXE := gtk+-$(GTK_BIN_VER)-setup.exe

GLIB_VER := 2.12.4
GLIB_URL := http://ftp.gtk.org/pub/glib/2.12/win32
GLIB_ARCHIVE := glib-dev-$(GLIB_VER).zip

ATK_VER := 1.12.2
ATK_ARCHIVE := atk-dev-$(ATK_VER).zip
ATK_URL := $(GTK_URL)/dependencies

CAIRO_VER := 1.4.6
CAIRO_ARCHIVE := cairo-dev-$(CAIRO_VER).zip
CAIRO_URL := $(GTK_URL)

XML2_VER := 2.6.26
XML2_DIR := libxml2-$(XML2_VER)
XML2_URL := ftp://xmlsoft.org/libxml2
XML2_ARCHIVE := libxml2-$(XML2_VER).tar.gz

PANGO_VER := 1.10.3
PANGO_URL := http://ftp.gtk.org/pub/gtk/v2.8/win32
PANGO_ARCHIVE := pango-dev-$(PANGO_VER).zip

LIBPNG_VER := 1.2.8
GNUWIN32_URL := http://mesh.dl.sourceforge.net/gnuwin32

EXPAT_VER = 2.0.0
EXPAT_DIR = $(BUILDROOT)/expat-$(EXPAT_VER)
EXPAT_ARCHIVE := expat-$(EXPAT_VER).tar.gz
EXPAT_URL := http://belnet.dl.sourceforge.net/sourceforge/expat

LIBINTL_VER := 0.14.4

REGEXDIR := $(BUILDROOT)/regex
REGEX_ARCHIVE := regex.tar.gz

GIMP_URL := http://www.gimp.org/~tml/gimp/win32
GETTEXT_VER := 0.14.5
GETTEXT_ARCHIVE := gettext-dev-$(GETTEXT_VER).zip
ICONV_VER := 1.9.1
ICONV_ARCHIVE := libiconv-$(ICONV_VER).bin.woe32.zip

DIRECTX_URL := http://www.colombus.org/ekiga
DIRECTX_DIR := $(BUILDROOT)/directx-dev
DIRECTX_ARCHIVE := directx-dev.tar.gz

LN:=ln
RM:=rm
CP:=cp

DOC_LINGUAS=bg de es fr pt_BR sv uk

ifneq (,$(CONCURRENCY))
        MAKEOPTS+=-j$(CONCURRENCY)
endif

confflags:=--prefix=$(BUILDROOT) --build=$(DEB_BUILD_GNU_TYPE) --host=$(DEB_HOST_GNU_TYPE)
confpwlib:=--prefix=$(BUILDROOT) --build=$(DEB_BUILD_GNU_TYPE) --host=$(DEB_HOST_GNU_TYPE) \
	   --with-directx-dir=$(DIRECTX_DIR)
confekiga:= $(confflags) --with-pwlib-dir=$(PWLIB_DIR) --with-opal-dir=$(OPAL_DIR) \
	    --with-directx-dir=$(DIRECTX_DIR) --prefix=$(EKIGA_PREFIX) \
	    --bindir=$(EKIGA_PREFIX) --datadir=$(EKIGA_PREFIX) \
	    --sysconfdir=$(EKIGA_PREFIX) --libdir=$(EKIGA_PREFIX) --disable-gnome \
	    --disable-dbus --disable-avahi --disable-esd 


all: binaries $(TARGETDIR)/$(EKIGA_INSTALLER)
update:
	$(WGET) -P $(SRCDIR) $(EKIGA_URL1)/$(PWLIB_ARCHIVE) || \
	$(WGET) -P $(SRCDIR) $(EKIGA_URL2)/$(PWLIB_ARCHIVE)
	$(WGET) -P $(SRCDIR) $(EKIGA_URL1)/$(OPAL_ARCHIVE) || \
	$(WGET) -P $(SRCDIR) $(EKIGA_URL2)/$(OPAL_ARCHIVE)
	$(WGET) -P $(SRCDIR) $(EKIGA_URL1)/$(EKIGA_ARCHIVE) || \
	$(WGET) -P $(SRCDIR) $(EKIGA_URL2)/$(EKIGA_ARCHIVE)
        
binaries:
	echo Checking prerequisites...
	hash $(REALCC) $(REALCXX) $(LD) $(NM) $(AR) $(RANLIB) $(DLLTOOL) $(DLLWRAP) $(OBJDUMP) $(WINDRES) $(AS)||(echo You need to install mingw32;exit 1)
	hash tar unzip zip wget aclocal-1.9 autoheader libtoolize automake-1.9 autoconf pkg-config gnome-autogen.sh bison gdk-pixbuf-csource||(echo we need more binaries ;exit 1)
	[ -f /usr/share/gnome-doc-utils/gnome-doc-utils.make ]||(echo gnome-doc-tools not installed;exit 1)
	[ -f /usr/lib/pkgconfig/gconf-2.0.pc ]||(echo libgconf2-dev not installed;exit 1)
	[ -f /usr/lib/pkgconfig/avahi-glib.pc ]||(echo libavahi-glib-dev not installed;exit 1)
	[ -f /usr/lib/pkgconfig/avahi-client.pc ]||(echo libavahi-client-dev not installed;exit 1)
	hash makensis||(echo nsis not installed;exit1) 	 
	hash xmlto||(echo xmlto not installed;exit1)
#       [ $(BUILDROOT) = $(PWD) ]||(echo You are supposed to run this as Makefile in $(BUILDROOT);exit 1)
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

### expat
update-sources:: $(SRCDIR)/$(EXPAT_ARCHIVE)
$(SRCDIR)/$(EXPAT_ARCHIVE): binaries
	echo --- Getting EXPAT ...
	$(WGET) -P src $(EXPAT_URL)/$(EXPAT_ARCHIVE)
	touch $@

$(EXPAT_DIR)/config.status: $(SRCDIR)/$(EXPAT_ARCHIVE) 
	tar xfz $(SRCDIR)/$(EXPAT_ARCHIVE)
	(cd $(EXPAT_DIR); ./configure $(confflags))
	touch $@

$(INCLUDEDIR)/expat.h:
	install -m 644 $(EXPAT_DIR)/lib/expat.h $(INCLUDEDIR)/
	touch $@

$(LIBDIR)/libexpat.a:  $(EXPAT_DIR)/config.status $(INCLUDEDIR)/expat.h
	$(MAKE) $(MAKEOPTS) -C $(EXPAT_DIR)
	$(MAKE) $(MAKEOPTS) -C $(EXPAT_DIR) install
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
	$(MAKE) $(MAKE_LIB_OPTS) -C $(OPENLDAP_DIR)/libraries/liblutil
	$(CP) -f $(OPENLDAP_DIR)/libraries/liblutil/liblutil.a $(LIBDIR)
	$(MAKE) $(MAKE_LIB_OPTS) -C $(OPENLDAP_DIR)/libraries/liblber all install 
	$(MAKE) $(MAKE_LIB_OPTS) -C $(OPENLDAP_DIR)/include install
	$(MAKE) $(MAKE_LIB_OPTS) -C $(OPENLDAP_DIR)/libraries/libldap MOD_LIBS="-llutil" all install
	$(MAKE) $(MAKE_LIB_OPTS) -C $(OPENLDAP_DIR)/libraries/libldap_r MOD_LIBS="-llutil" all install
	touch $@

### PWLib
update-sources:: $(SRCDIR)/$(PWLIB_ARCHIVE)
$(SRCDIR)/$(PWLIB_ARCHIVE): binaries 
	echo --- Getting PWLib $(PWLIB_VER) ...
	$(WGET) -P src $(EKIGA_URL1)/$(PWLIB_ARCHIVE) || \
	$(WGET) -P src $(EKIGA_URL2)/$(PWLIB_ARCHIVE)
	touch $@

$(PWLIB_DIR)/configure: binaries $(LIBDIR)/libldap_r.dll $(LIBDIR)/libexpat.a $(LIBDIR)/libogg.a $(LIBDIR)/libspeex.a $(INCLUDEDIR)/directx/dsound.h $(SRCDIR)/$(PWLIB_ARCHIVE) $(EKIGA_WIN32_DIFF_DIR)/ptlib_Makefile.am $(EKIGA_WIN32_DIFF_DIR)/ptlib_configure.ac $(EKIGA_WIN32_DIFF_DIR)/ptlib_pwlib.pc.in $(EKIGA_WIN32_DIFF_DIR)/ptlib_vfw.patch 
	rm -f $(LIBDIR)/libpt.a
	rm -rf $(PWLIB_DIR)
	tar xfz $(SRCDIR)/$(PWLIB_ARCHIVE) -C $(BUILDROOT)
	$(RM) -f $(PWLIB_DIR)/configure.exe $(PWLIB_DIR)/configure $(PWLIB_DIR)/configure.ac $(PWLIB_DIR)/Makefile.in
	$(CP) -fl $(EKIGA_WIN32_DIFF_DIR)/ptlib_configure.ac $(PWLIB_DIR)/configure.ac
	$(CP) -fl $(EKIGA_WIN32_DIFF_DIR)/ptlib_Makefile.am $(PWLIB_DIR)/Makefile.am
	$(CP) -fl $(EKIGA_WIN32_DIFF_DIR)/ptlib_pwlib.pc.in $(PWLIB_DIR)/pwlib.pc.in    
	touch $(PWLIB_DIR)/make/ptlib-config
	chmod 777 $(PWLIB_DIR)/make/ptlib-config
	(cd $(PWLIB_DIR); \
	sed -i "s#\$${buildroot}#${BUILDROOT}#" pwlib.pc.in; \
	sed -i "s/\$${PWLIB_VER}/${PWLIB_VER}/" configure.ac; \
	patch -p1 < $(EKIGA_WIN32_DIFF_DIR)/ptlib_vfw.patch ;\
	aclocal-1.9; autoheader; libtoolize --force; \
	touch NEWS README AUTHORS ChangeLog unused.h.in; \
	automake-1.9 -a -c; autoconf; \
	)
	

$(PWLIB_DIR)/config.status: binaries $(PWLIB_DIR)/configure
	rm -f $(LIBDIR)/libpt.a
	(cd $(PWLIB_DIR)/;./configure $(confpwlib) )

$(LIBDIR)/libpt.a: binaries $(PWLIB_DIR)/config.status
	$(MAKE) $(MAKEOPTS) -C $(PWLIB_DIR)
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
	$(MAKE) $(MAKEOPTS) -C $(OGG_DIR)
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
	$(MAKE) $(MAKE_LIB_OPTS) -C $(SPEEX_DIR)
	$(MAKE) -C $(SPEEX_DIR) install

### OPAL
update-sources:: $(SRCDIR)/$(OPAL_ARCHIVE)
$(SRCDIR)/$(OPAL_ARCHIVE): binaries
	echo --- Getting OPAL $(OPAL_VER)...
	$(WGET) -P src $(EKIGA_URL1)/$(OPAL_ARCHIVE) || \
	$(WGET) -P src $(EKIGA_URL2)/$(OPAL_ARCHIVE)
	touch $@

$(OPAL_DIR)/configure: binaries $(LIBDIR)/libogg.a $(LIBDIR)/libspeex.a $(LIBDIR)/libpt.a $(SRCDIR)/$(OPAL_ARCHIVE) $(EKIGA_WIN32_DIFF_DIR)/opal_configure.ac $(EKIGA_WIN32_DIFF_DIR)/opal_Makefile.am $(EKIGA_WIN32_DIFF_DIR)/opal_opal.pc.in 
	rm -rf $(OPAL_DIR) $(LIBDIR)/libopal.a
	tar xfz $(SRCDIR)/$(OPAL_ARCHIVE) -C $(BUILDROOT)
	$(RM) -f $(OPAL_DIR)/configure.exe $(OPAL_DIR)/configure $(OPAL_DIR)/configure.ac $(OPAL_DIR)/Makefile.in
	$(CP) -fl $(EKIGA_WIN32_DIFF_DIR)/opal_configure.ac $(OPAL_DIR)/configure.ac
	$(CP) -fl $(EKIGA_WIN32_DIFF_DIR)/opal_Makefile.am $(OPAL_DIR)/Makefile.am
	$(CP) -fl $(EKIGA_WIN32_DIFF_DIR)/opal_opal.pc.in $(OPAL_DIR)/opal.pc.in
	(cd opal; \
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
	$(MAKE) $(MAKEOPTS) -C $(OPAL_DIR) 
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
$(SRCDIR)/$(GTK_BIN_EXE) : 	 
	echo --- Getting GTK2+ Runtime Library ... 	 
	$(WGET) -P $(SRCDIR) $(GTK_INSTALLER_URL)/$(GTK_BIN_EXE) 	 
	touch $@ 	 

$(LIBDIR)/$(GTK_BIN_EXE): $(SRCDIR)/$(GTK_BIN_EXE) 	
	cp $(SRCDIR)/$(GTK_BIN_EXE)  $(LIBDIR)
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
	$(MAKE) $(MAKEOPTS) -C $(XML2_DIR)
	$(MAKE) $(MAKEOPTS) -C $(XML2_DIR) install

### DirectX
update-sources:: $(SRCDIR)/$(DIRECTX_ARCHIVE)
$(SRCDIR)/$(DIRECTX_ARCHIVE): binaries
	echo --- Getting DirectX-dev ---
	$(WGET) -P $(SRCDIR) $(DIRECTX_URL)/$(DIRECTX_ARCHIVE)
	touch $@

$(DIRECTX_DIR)/config.status: $(SRCDIR)/$(DIRECTX_ARCHIVE)
	tar xzvf $(SRCDIR)/$(DIRECTX_ARCHIVE) -C $(BUILDROOT)
	touch $@

$(INCLUDEDIR)/directx/dsound.h: $(DIRECTX_DIR)/config.status
	mkdir -p $(INCLUDEDIR)/directx
	(for header in $(DIRECTX_DIR)/include/*h; do \
		cp $$header $(INCLUDEDIR)/directx/; \
	done)
	touch $@

### GnomeMeeting / Ekiga
update-sources:: $(SRCDIR)/$(EKIGA_ARCHIVE)
$(SRCDIR)/$(EKIGA_ARCHIVE): binaries
	echo --- Getting Ekiga-CVS ...
	$(WGET) -P src $(EKIGA_URL1)/$(EKIGA_ARCHIVE)||\
	$(WGET) -P src $(EKIGA_URL2)/$(EKIGA_ARCHIVE)
	touch $@

$(EKIGA_DIR)/autogen.sh: binaries $(SRCDIR)/$(EKIGA_ARCHIVE)
	svn co http://svn.gnome.org/svn/ekiga/trunk ekiga 
#	rm -rf $(EKIGA_DIR)
#	tar xfz $(SRCDIR)/$(EKIGA_ARCHIVE)
#	(cd $(EKIGA_DIR); \
#	)
	touch $@

$(EKIGA_DIR)/config.status: binaries $(EKIGA_DIR)/autogen.sh $(LIBDIR)/libpt.a $(LIBDIR)/libopal.a \
	$(LIBDIR)/pkgconfig/gtk+-2.0.pc $(LIBDIR)/pkgconfig/atk-2.0.pc $(LIBDIR)/pkgconfig/glib-2.0.pc \
	$(LIBDIR)/libpng.a $(LIBDIR)/pkgconfig/pango32.pc $(LIBDIR)/pkgconfig/cairo.pc $(LIBDIR)/libxml2.a \
	$(LIBDIR)/libintl.a $(LIBDIR)/libiconv.a
	rm -f $(EKIGA_DIR)/src/ekiga.exe
	(cd $(EKIGA_DIR)/; ACLOCAL_FLAGS="-I $(BUILDROOT)/share/aclocal" \
	./autogen.sh $(confekiga) )

$(EKIGA_DIR)/src/ekiga.exe: binaries $(EKIGA_DIR)/config.status
	$(WINDRES) $(EKIGA_DIR)/ekiga.rc $(EKIGA_DIR)/ekiga_rc.o
	$(MAKE) $(MAKEOPTS) -C $(EKIGA_DIR)
	$(MAKE) $(MAKEOPTS) -C $(EKIGA_DIR)/help

$(TARGETDIR)/zips: binaries $(EKIGA_DIR)/src/ekiga.exe
	-$(RM) -rf $(TARGETDIR)
	
	mkdir $(TARGETDIR)
	make -C ekiga install
	mkdir -p $(EKIGA_PREFIX)/plugins
	for plugin in $(OPAL_DIR)/.libs/*.dll; do \
	        install -m 755 $$plugin $(EKIGA_PREFIX)/plugins ;\
	done
	for lib in libexpat-1.dll liblber.dll libldap_r.dll libxml2-2.dll libspeex-1.dll; do \
	        install -m 755 $(BINDIR)/$$lib $(EKIGA_PREFIX) ;\
	done
ifeq ($(DEBUG),0) 	 
	$(STRIP) $(EKIGA_PREFIX)/*.dll
	$(STRIP) $(EKIGA_PREFIX)/*.exe; 
endif
	for lc in C $(DOC_LINGUAS); do \
		mkdir -p $(EKIGA_PREFIX)/help/$$lc; \
		if test -f $(EKIGA_DIR)/help/$$lc/ekiga.xml; then \
			xmlto -o $(EKIGA_PREFIX)/help/$$lc/ --skip-validation html $(EKIGA_DIR)/help/$$lc/ekiga.xml; \
		fi ;\
		install -m 644 $(EKIGA_DIR)/help/$$lc/ekiga.xml $(EKIGA_PREFIX)/help/$$lc/;\
		if test -d $(EKIGA_DIR)/help/$$lc/figures; then \
			mkdir -p $(EKIGA_PREFIX)/help/$$lc/figures; \
			install -m 644 $(EKIGA_DIR)/help/$$lc/figures/*.png $(EKIGA_PREFIX)/help/$$lc/figures; \
		fi \
	done
	(cd $(TARGETDIR)/;zip -9r ekiga.zip Ekiga )
	zip -9r $(TARGETDIR)/ekiga_build.zip Makefile $(EKIGA_WIN32_DIFF_DIR)/* ico/*
	tar cfvz $(TARGETDIR)/ekiga_build.tgz Makefile $(EKIGA_WIN32_DIFF_DIR)/* ico/*
	touch $@

#### README about NSIS installer ####
#
# NSIS package shipped with linux distributions does not includes some of the plugins needed.
# - System.dll : You need to get it from the windows zip archive (http://sourceforge.net/projects/nsis) and extract 
#               /Plugins/System.dll TO /usr/share/nsis/Plugins/
#		/Contrib/Modern UI/System.nsh TO /usr/share/nsis/Contrib/System/
# - Process.dll : Is a plugin that can be found here http://nsis.sourceforge.net/Processes_plug-in
#   		extract /bin/Processes.dll TO /usr/share/nsis/Plugins/
#   		
###########################

$(TARGETDIR)/$(EKIGA_INSTALLER): $(TARGETDIR)/zips $(LIBDIR)/$(GTK_BIN_EXE)
	(makensis -DEKIGA_VERSION=$(EKIGA_VER) -DEKIGA_DIR=$(EKIGA_DIR) -DBUILDROOT=$(BUILDROOT) \
	 -DINSTALLER_DIR=$(EKIGA_INSTALLER_DIR) -DLIB_DIR=$(LIBDIR) -DTARGET_DIR=$(TARGETDIR) \
	 -DCROSS_COMPILING=true -DWITH_GTK=true -DGTK_VERSION=$(GTK_BIN_VER) \
	 $(EKIGA_INSTALLER_DIR)/ekiga.nsi )
	touch $@


clean:
	-$(RM) -rf $(TARGETDIR)

distclean:
	-$(RM) -rf $(OPENLDAP_DIR)
	-$(RM) -rf $(BUILDROOT)/share/
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
	-$(RM) -rf $(EKIGA_DIR)
	-$(RM) -rf $(PWLIB_DIR)
	-$(RM) -rf $(REGEXDIR)
	-$(RM) -rf $(SPEEX_DIR)
	-$(RM) -rf $(EXPAT_DIR)
	-$(RM) -rf $(DIRECTX_DIR)
	-$(RM) -rf make
	-$(RM) -f binaries
	-$(RM) -rf $(SRCDIR)/*

.PHONY: clean distclean update update-sources
