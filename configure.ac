AC_INIT([ekiga],[4.0.1],[http://bugzilla.gnome.org/enter_bug.cgi?product=ekiga])

AC_CANONICAL_TARGET
AC_PREREQ([2.53])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR(src/ekiga.cpp)
AM_INIT_AUTOMAKE([1.11 no-dist-gzip dist-xz tar-ustar])

AM_MAINTAINER_MODE([enable])
AC_CONFIG_HEADERS([config.h])

dnl use "make V=1" if you want to see the long awful lines
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

AC_LANG(C++)

dnl ###########################################################################
dnl   Definitions
dnl ###########################################################################

MAJOR_VERSION=4
MINOR_VERSION=0
BUILD_TYPE=ReleaseCode
BUILD_NUMBER=1

PTLIB_REC_VERSION="2.10.10"
OPAL_REC_VERSION="3.10.10"

AC_DEFINE_UNQUOTED(MAJOR_VERSION, $MAJOR_VERSION,[fix])
AC_DEFINE_UNQUOTED(MINOR_VERSION, $MINOR_VERSION,[fix])
AC_DEFINE_UNQUOTED(BUILD_TYPE, $BUILD_TYPE,[fix])
AC_DEFINE_UNQUOTED(BUILD_NUMBER, $BUILD_NUMBER,[fix])

dnl ###########################################################################
dnl   Disable / Enable Main Ekiga Components
dnl ###########################################################################

PKG_PROG_PKG_CONFIG

LT_INIT([dlopen disable-static])

AC_PROG_CC_C99
AC_PROG_CXX
AC_SEARCH_LIBS([strerror],[cposix])
AC_HEADER_STDC

dnl Enable compiler warnings
if test ".$ac_cv_c_compiler_gnu" = .yes; then
   CFLAGS="$CFLAGS -Wall -Wextra -Winit-self -Wswitch-default -Wswitch-enum -Wstrict-aliasing=2 -Wfloat-equal -Wshadow"
   CXXFLAGS="$CXXFLAGS -Wall -Wextra -Winit-self -Wswitch-default -Wswitch-enum -Wstrict-aliasing=2 -Wfloat-equal -Wshadow"
fi

dnl Check the library path
case $host in

  *-*-linux*)
    # Test if the compiler is 64bit
    echo 'int i;' > conftest.$ac_ext
    ekiga_cv_cc_64bit_output=no

    if AC_TRY_EVAL(ac_compile); then
      case `/usr/bin/file conftest.$ac_objext` in
        *"ELF 64"*)
          ekiga_cv_cc_64bit_output=yes
          ;;
      esac
    fi
    rm -rf conftest*
  ;;
esac

case $host_cpu:$ekiga_cv_cc_64bit_output in

  powerpc64:yes | s390x:yes | sparc64:yes | x86_64:yes)
    libname="lib64"
    ;;

  *:*)
    libname="lib"
    ;;
esac

dnl Check the OS type (more types can be added)
AC_MSG_CHECKING(whether OS type is supported)
win32=0

case $target_os in

  solaris*)
    gm_platform="solaris"
    ;;

  linux-gnulp | linux-gnu | linux-gnueabi* | linux | Linux)
    gm_platform="linux"
    ;;

  gnu* )
    gm_platform="gnu"
    ;;

  FreeBSD* | freebsd* | kfreebsd* | NetBSD* | netbsd* )
    gm_platform="freebsd"
    ;;

  darwin* )
    gm_platform="macosx"
    ;;

  mingw* )
    gm_platform="mingw"
    CFLAGS="$CFLAGS -Wl,--enable-auto-import"
    CXXFLAGS="$CXXFLAGS -Wl,--enable-auto-import"
    win32=1
    ;;

  *)
    AC_MSG_ERROR([Only Linux, Hurd, FreeBSD, NetBSD, Solaris and Mac OS X are supported. Please contact the author to know how you can add more OS types.])
    ;;
esac
AC_MSG_RESULT([yes])

AM_CONDITIONAL(WIN32, test "x${win32}" = "x1")

dnl Check the byte order
AC_MSG_CHECKING(Byte order)
byte_order="unknown"
AC_ARG_WITH([endianess],
            [AS_HELP_STRING([--with-endianess=big|little],[endianess of target system])],
	    [byte_order="$withval endian"],
            [AC_C_BIGENDIAN([byte_order="big endian"], [byte_order="little endian"], [])])

if test "x$byte_order" = "xbig endian"; then
    AC_DEFINE(WORDS_BIGENDIAN,1,[Big Endian Architecture])
else
    if test "x$byte_order" = "xlittle endian"; then
	AC_DEFINE(WORDS_LITTLEENDIAN,1,[Little Endian Architecture])
    else
      AC_MSG_ERROR([Unable to detect endianess of target system - please specify])
    fi
fi

AC_MSG_RESULT([$byte_order])


dnl ###############################
dnl   Mandatory intltool support
dnl ###############################
IT_PROG_INTLTOOL([0.35.0])


dnl ###############################
dnl   Mandatory GTK+ support
dnl ###############################
# on Windows the icons are manually copied in the installer
if test "x${gm_platform}" = "xmingw" ; then
   PKG_CHECK_MODULES([GTK], [gtk+-2.0 >= 2.20.0])
else
   PKG_CHECK_MODULES([GTK], [gtk+-2.0 >= 2.20.0 gnome-icon-theme >= 3.0.0])
fi
PKG_CHECK_MODULES([GLIB], [glib-2.0 >= 2.24.0 gmodule-2.0 gobject-2.0 gthread-2.0])
AC_ARG_ENABLE([gtk-debug],
              [AS_HELP_STRING([--enable-gtk-debug],[enable GTK+ debug flags (default is disabled)])],
              [if test "x$enableval" = "xyes"; then
                GTK_CFLAGS="$GTK_CFLAGS -DGDK_PIXBUF_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED -DGDK_PIXBUF_DISABLE_SINGLE_INCLUDES -DGTK_DISABLE_SINGLE_INCLUDES"
                GLIB_CFLAGS="$GLIB_CFLAGS -DG_DISABLE_SINGLE_INCLUDES"
              fi])
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)


dnl ###############################
dnl   Mandatory BOOST support
dnl ###############################
AX_BOOST_BASE([1.34])
AX_BOOST_SIGNALS

if test "x${ax_cv_boost_signals}" == "xno"; then
   AC_MSG_ERROR([You need the boost signals library to compile Ekiga])
fi

BOOST_LIBS="${BOOST_SIGNALS_LIB}"
AC_SUBST(BOOST_LIBS)


dnl ###############################
dnl   Mandatory libXML support
dnl ###############################
PKG_CHECK_MODULES([XML], [libxml-2.0])
AC_SUBST(XML_CFLAGS)
AC_SUBST(XML_LIBS)


dnl ###############################
dnl   GConf support
dnl ###############################
GCONF="disabled"

if test "x${gm_platform}" != "xmingw" ; then
  AC_ARG_ENABLE(gconf, AS_HELP_STRING([--enable-gconf],[enable GConf support (default is enabled)]),
[if test "x$enableval" = "xyes"; then
  enable_gconf = yes
fi],enable_gconf=yes)

  if test "x$enable_gconf" = "xyes"; then
    PKG_CHECK_MODULES([GCONF], [gconf-2.0 >= 2.6.0], [found_gconf=yes])
    AM_GCONF_SOURCE_2
    AC_SUBST(GCONF_CFLAGS)
    AC_SUBST(GCONF_LIBS)
    GCONF="enabled"
    AC_DEFINE(HAVE_GCONF,1,[GConf support])
  else
    AM_CONDITIONAL(GCONF_SCHEMAS_INSTALL, false)
  fi
else
  AM_CONDITIONAL(GCONF_SCHEMAS_INSTALL, false)
fi

AM_CONDITIONAL(HAVE_GCONF, test "x$found_gconf" = "xyes")


dnl ###############################
dnl   Evolution-data-server support
dnl ###############################
EDS="disabled"

if test "x${gm_platform}" != "xmingw" ; then
  AC_ARG_ENABLE(eds, AS_HELP_STRING([--enable-eds],[enable evolution-data-server addressbook support (default is enabled)]),
[if test "x$enableval" = "xyes"; then
  enable_eds=yes
fi],enable_eds=yes)

  if test "x$enable_eds" = "xyes"; then
    PKG_CHECK_MODULES([EDS], [libebook-1.2], [found_eds=yes])
    AC_SUBST(EDS_CFLAGS)
    AC_SUBST(EDS_LIBS)
    EDS="enabled"
    AC_DEFINE(HAVE_EDS,1,[EDS support])
  fi
fi

AM_CONDITIONAL(HAVE_EDS, test "x$found_eds" = "xyes")


dnl ###############################
dnl   NOTIFY support
dnl ###############################
NOTIFY="disabled"

if test "x${gm_platform}" != "xmingw" ; then
  AC_ARG_ENABLE(notify, AS_HELP_STRING([--enable-notify],[enable libnotify support (default is enabled)]),
[if test "x$enableval" = "xyes"; then
  enable_notify=yes
fi],enable_notify=yes)

  if test "x$enable_notify" = "xyes"; then
    PKG_CHECK_MODULES([NOTIFY], [libnotify], [found_notify=yes], foo=bar)
    if test "x$found_notify" = "xyes"; then
      AC_SUBST(NOTIFY_CFLAGS)
      AC_SUBST(NOTIFY_LIBS)
      NOTIFY="enabled"
      AC_DEFINE(HAVE_NOTIFY,1,[NOTIFY support])
    fi
  fi
fi

AM_CONDITIONAL(HAVE_NOTIFY, test "x$found_notify" = "xyes")


dnl ###############################
dnl   LDAP support
dnl ###############################
LDAP="disabled"

AC_ARG_ENABLE(ldap, AS_HELP_STRING([--enable-ldap],[enable LDAP addressbook support (default is enabled)]),
[if test "x$enableval" = "xyes"; then
  enable_ldap=yes
fi],enable_ldap=yes)

if test "x$enable_ldap" = "xyes"; then

  AC_ARG_WITH(ldap-dir, AS_HELP_STRING([--with-ldap-dir=PFX],[location of LDAP]), with_ldap_dir="$withval", with_ldap_dir="/usr")

  dnl Check for the includes presence
  AC_MSG_CHECKING(for LDAP includes in ${with_ldap_dir}/include/)
  AC_MSG_RESULT()


  if test -f ${with_ldap_dir}/include/ldap.h; then
  	LDAP_CFLAGS="-I${with_ldap_dir}/include"
  else
  	AC_MSG_ERROR([You need the LDAP headers to compile Ekiga with LDAP support])
  fi

  dnl Checking for the library presence
  LIBS_save="$LIBS"
  LIBS="${LIBS} -L${with_ldap_dir}/${libname}/ -llber"
  AC_CHECK_LIB(ldap, main, ldap_libs="yes", ldap_libs="no")
  LIBS="${LIBS_save}"

  if test "x${ldap_libs}" != "xno"; then
  	LDAP_LIBS="-llber -lldap"
  	if test "x${with_ldap_dir}" != "x/usr"; then
  		LDAP_LIBS="-L${with_ldap_dir}/${libname} $LDAP_LIBS"
  	fi
  else
  	AC_MSG_ERROR([You need the LDAP library to compile Ekiga with LDAP support])
  fi

  dnl Checking for libsasl2
  AC_ARG_WITH(libsasl2-dir, AS_HELP_STRING([--with-libsasl2-dir=PFX],[location of LIBSASL2]), with_libsasl2_dir="$withval", with_libsasl2_dir="/usr")

  dnl Check for the libsasl2 includes presence
  AC_MSG_CHECKING(for LIBSASL2 includes in ${with_libsasl2_dir}/include/)
  AC_MSG_RESULT()

  if test -f ${with_libsasl2_dir}/include/sasl/sasl.h; then
  	LDAP_CFLAGS="${LDAP_CFLAGS} -I${with_libsasl2_dir}/include"
  else
  	AC_MSG_ERROR([You need the SASL2 library to compile Ekiga])
  fi
  LDAP_LIBS="$LDAP_LIBS -lsasl2"

  dnl Checking for libresolv
  if test ${gm_platform} = "linux" ; then
  	AC_CHECK_LIB(resolv, res_gethostbyaddr, [LIBS="-lresolv $LIBS"], AC_MSG_ERROR([You need the libresolv library to compile Ekiga]), -lresolv)
  	LDAP_LIBS="${LDAP_LIBS} -lresolv"
  fi
  found_ldap="yes"
  AC_SUBST(LDAP_CFLAGS)
  AC_SUBST(LDAP_LIBS)
  LDAP="enabled"
  AC_DEFINE(HAVE_LDAP,1,[LDAP support])
fi

AM_CONDITIONAL(HAVE_LDAP, test "x$found_ldap" = "xyes")


AM_PROG_AS

dnl ###############################
dnl Checks for MMX support
dnl ###############################
AC_MSG_CHECKING(for x86 platform)
case $host_cpu in
  i386|i486|i586|i686|i786|k6|k7)
        AC_ARG_ENABLE(mmx, AS_HELP_STRING([--enable-mmx],[enable MMX acceleration (default is enabled)]), [if test "x$enableval" = "xyes"; then
                             use_x86_asm=yes
			   else
			     use_x86_asm=no
                           fi], use_x86_asm=yes)
        ;;
   *)
  	use_x86_asm=no
esac
AC_MSG_RESULT($use_x86_asm)

use_mmx_asm=no
if test "x$use_x86_asm" = "xyes"; then
    save_ac_ext=$ac_ext
    ac_ext=S

    AC_MSG_CHECKING(compiler support for MMX)
    cp $srcdir/lib/pixops/scale_line_22_33_mmx.S conftest.S
    if AC_TRY_EVAL(ac_compile); then
        use_mmx_asm=yes
    fi

    rm -rf conftest*

    ac_ext=$save_ac_ext
    if test "x$use_mmx_asm" = "xyes"; then
      AC_DEFINE([USE_MMX], 1, [MMX Support])
      AC_MSG_RESULT(yes)
    else
      AC_MSG_RESULT(no)
    fi
fi

AM_CONDITIONAL(USE_MMX, test "x$use_mmx_asm" = "xyes")



dnl ###############################
dnl Checks for Shared Memory extension
dnl ###############################
SHM="disabled"

if test "x${gm_platform}" = "xlinux" ; then
  AC_ARG_ENABLE(shm, AS_HELP_STRING([--enable-shm],[enable SHM support (default is enabled)]),
[if test "x$enableval" = "xyes"; then
  enable_shm=yes
fi],enable_shm=yes)
  if test "x$enable_shm" = "xyes"; then
    AC_CHECK_LIB(Xext, XShmAttach, with_shm="yes", with_shm="no")
    if test "x${with_shm}" != "xno"; then
      SHM="enabled"
      AC_DEFINE(HAVE_SHM,1,[SHM Support])
      SHM_LIBS="-lXext"
      AC_SUBST(SHM_LIBS)
    fi
  fi
fi
AM_CONDITIONAL(HAVE_SHM, test "x$SHM" = "xenabled")

dnl ###############################
dnl Checks for Xv extension
dnl ###############################
XV="disabled"
AC_ARG_ENABLE(xv, AS_HELP_STRING([--disable-xv],[enable XVideo hardware acceleration (default is enabled)]),
[if test "x$enableval" = "xyes"; then
  enable_xv=yes
fi],enable_xv=yes)
if test "x${gm_platform}" != "xmingw" -a "x$enable_xv" = "xyes"; then
  PKG_CHECK_MODULES([XV], [xv],
                    [AC_DEFINE(HAVE_XV,1,[XVideo support])
                     XV="enabled"
                    ])
  AC_SUBST(XV_CFLAGS)
  AC_SUBST(XV_LIBS)
fi

AM_CONDITIONAL(HAVE_XV, test "x$XV" = "xenabled")


dnl ###########################################################################
dnl Checks for DX extension
dnl ###########################################################################
DX="disabled"

if test "x${gm_platform}" = "xmingw" ; then
  AC_ARG_WITH(directx-dir, AS_HELP_STRING([--with-directx-dir=PFX],[location of DirectDraw]), with_directx_dir="$withval", with_directx_dir=".")

  dnl Check for the includes presence
  AC_MSG_CHECKING(for DirectX includes in ${with_directx_dir})
  AC_MSG_RESULT()
  if test -d ${with_directx_dir}; then
    DX_CFLAGS="-I${with_directx_dir}"
    DX_LIBS="$EKIGA_LIBS -lddraw"
    AC_SUBST(DX_LIBS)
    AC_SUBST(DX_CFLAGS)
    DX="enabled"
    AC_DEFINE(HAVE_DX,1,[DirectDraw support])
  else
    AC_MSG_ERROR([Unable to locate DirectX header files; --with-directx-dir is obligatory])
  fi
fi

AM_CONDITIONAL(HAVE_DX, test "x$DX" = "xenabled")


dnl ###############################
dnl   DBUS Support
dnl ###############################
DBUS="disabled"
DBUS_SERVICE="disabled"

if test "x${gm_platform}" != "xmingw" ; then
  AC_ARG_ENABLE(dbus, AS_HELP_STRING([--enable-dbus],[enable DBUS support (default is enabled)]),
[if test "x$enableval" = "xyes"; then
  enable_dbus=yes
fi],enable_dbus=yes)
  AC_ARG_ENABLE(dbus-service, AS_HELP_STRING([--enable-dbus-service],[enable DBUS service installation (default is enabled)]),
[if test "x$enableval" = "xyes"; then
  enable_dbus_service=yes
fi],enable_dbus_service=yes)

  if test "x$enable_dbus" = "xyes"; then
    PKG_CHECK_MODULES([DBUS], [dbus-1 >= 0.36 dbus-glib-1 >= 0.36], [found_dbus=yes])
    AC_SUBST(DBUS_CFLAGS)
    AC_SUBST(DBUS_LIBS)
    DBUS="enabled"
    AC_DEFINE(HAVE_DBUS,1,[DBUS support])

    if test "x$found_dbus" = "xyes" -a "x$enable_dbus_service" = "xyes"; then
      DBUS_SERVICE="enabled"
    fi
  fi
fi

AM_CONDITIONAL(HAVE_DBUS, test "x$found_dbus" = "xyes")
AM_CONDITIONAL(INSTALL_DBUS_SERVICE, test "x$DBUS_SERVICE" = "xenabled")


dnl ###############################
dnl   V4L headers
dnl   These are only necessary until HAL has a decent V4L support...
dnl ###############################

OLDV4L="disabled"
V4L="disabled"
V4L2="disabled"
VIDEO4LINUX="disabled"
if test "x${win32}" != "x1"; then
  AC_CHECK_HEADER(libv4l1-videodev.h, V4L="enabled")
  AC_CHECK_HEADER(linux/videodev2.h, V4L2="enabled")
  AC_CHECK_HEADER(linux/videodev.h, OLDV4L="enabled")

  if test "x${V4L}" != "xdisabled" -a "x${V4L2}" != "xdisabled"; then
    AC_DEFINE(HAVE_V4L,1,[V4L support])
    VIDEO4LINUX="enabled"
  else
    if test "x${OLDV4L}" != "xdisabled"; then
      AC_DEFINE(HAVE_V4L,1,[V4L support])
      AC_DEFINE(HAVE_OLD_V4L,1,[V4L is old style])
      VIDEO4LINUX="enabled"
    fi
  fi
fi
AM_CONDITIONAL(HAVE_V4L, test "x${VIDEO4LINUX}" = "xenabled")

dnl ###############################
dnl   Avahi / mDNS Support
dnl ###############################
AVAHI="disabled"

if test "x${gm_platform}" != "xmingw" ; then
  AC_ARG_ENABLE(avahi, AS_HELP_STRING([--enable-avahi],[enable mDNS support (default is disabled)]),
[if test "x$enableval" = "xyes"; then
  enable_avahi=yes
fi],)

  if test "x$enable_avahi" = "xyes"; then
    PKG_CHECK_MODULES([AVAHI], [avahi-client >= 0.6 avahi-glib >= 0.6], [found_avahi=yes])
    AC_SUBST(AVAHI_CFLAGS)
    AC_SUBST(AVAHI_LIBS)
    AVAHI="enabled"
    AC_DEFINE(HAVE_AVAHI,1,[Avahi support])
  fi
fi

AM_CONDITIONAL(HAVE_AVAHI, test "x$found_avahi" = "xyes")


dnl ###############################
dnl    GStreamer Support
dnl ###############################
GSTREAMER="disabled"

AC_ARG_ENABLE(gstreamer, AS_HELP_STRING([--enable-gstreamer],[enable experimental GStreamer support (default is disabled)]),
[if test "x$enableval" = "xyes"; then
  enable_gstreamer=yes
fi],)

if test "x$enable_gstreamer" = "xyes"; then
  found_gstreamer=yes
  PKG_CHECK_MODULES([GSTREAMER], [gstreamer-plugins-base-0.10 >= 0.10.21.3 gstreamer-interfaces-0.10])

  if test "x$found_gstreamer" = "xno"; then
    AC_MSG_ERROR([Could not find GStreamer libraries])
  fi
  GSTREAMER_LIBS="$GSTREAMER_LIBS -lgstapp-0.10"
  AC_SUBST(GSTREAMER_CFLAGS)
  AC_SUBST(GSTREAMER_LIBS)
  GSTREAMER="enabled"
  AC_DEFINE(HAVE_GSTREAMER,1,[GStreamer support])
fi

AM_CONDITIONAL(HAVE_GSTREAMER, test "x$found_gstreamer" = "xyes")


dnl ###############################
dnl  KDE support
dnl ###############################
KDE="disabled"

AC_ARG_ENABLE(kde, AS_HELP_STRING([--enable-kde],[enable experimental KDE support (default is disabled)]),
[if test "x$enableval" = "xyes"; then
  enable_kde=yes
fi],)

if test "x$enable_kde" = "xyes"; then

  PKG_CHECK_MODULES(KDE, [QtCore], [found_kde=yes], [found_kde=no])

  CPPFLAGS_save="$CPPFLAGS"
  LIBS_save="$LIBS"
  CPPFLAGS="$CPPFLAGS $KDE_CFLAGS"
  LIBS="$LIBS $KDE_LIBS"
  AC_CHECK_HEADER(kapplication.h,,found_kde=no)
  CPPFLAGS="$CPPFLAGS_save"
  LIBS="$LIBS_save"
  if test "x$found_kde" = "xno"; then
    AC_MSG_ERROR([Could not find KDE headers])
  fi

  LIBS_save="$LIBS"
  CPPFLAGS="$CPPFLAGS $KDE_CFLAGS"
  LIBS="$LIBS $KDE_LIBS"
  AC_CHECK_LIB(kdeui,ki18n,found_kde=no)
  CPPFLAGS="$CPPFLAGS_save"
  LIBS="$LIBS_save"
  if test "x$found_kde" = "xno"; then
    AC_MSG_ERROR([Could not find KDE libraries])
  fi

  KDE_LIBS="$KDE_LIBS -lkdeui"

  AC_SUBST(KDE_CFLAGS)
  AC_SUBST(KDE_LIBS)
  KDE="enabled"
  AC_DEFINE(HAVE_KDE,1,[KDE support])
fi

AM_CONDITIONAL(HAVE_KDE, test "x$found_kde" = "xyes")

dnl ###############################
dnl  KAddressBook support
dnl ###############################
KAB="disabled"

AC_ARG_ENABLE(kab, AS_HELP_STRING([--enable-kab],[enable experimental KAddressBook support (default is disabled)]),
[if test "x$enableval" = "xyes"; then
  enable_kab=yes
fi],)

if test "x$enable_kab" = "xyes"; then

  if test "x$found_kde" != "xyes"; then
    AC_MSG_ERROR([No kab without kde: --enable-kde])
  fi
  found_kab=yes
  KAB_CFLAGS="-I/usr/include/qt4"
  KAB_LIBS="-lQtCore -lkabc"
  CPPFLAGS_save="$CPPFLAGS"
  LIBS_save="$LIBS"
  CPPFLAGS="$CPPFLAGS $KAB_CFLAGS"
  LIBS="$LIBS $KAB_LIBS"
  AC_CHECK_HEADER(kabc/addressee.h,,found_kab=no)
  CPPFLAGS="$CPPFLAGS_save"
  LIBS="$LIBS_save"
  if test "x$found_kab" = "xno"; then
    AC_MSG_ERROR([We need this header!])
  fi
  AC_SUBST(KAB_CFLAGS)
  AC_SUBST(KAB_LIBS)
  KAB="enabled"
  AC_DEFINE(HAVE_KAB,1,[KAddressBook support])
fi

AM_CONDITIONAL(HAVE_KAB, test "x$found_kab" = "xyes")

dnl ###############################
dnl   XCAP support
dnl ###############################
XCAP="disabled"

AC_ARG_ENABLE(xcap, AS_HELP_STRING([--enable-xcap],[enable experimental XCAP support (default is disabled)]),
[if test "x$enableval" = "xyes"; then
  enable_xcap=yes
fi],)

if test "x$enable_xcap" = "xyes"; then
   PKG_CHECK_MODULES([SOUP], [libsoup-2.4])
   found_xcap=yes
   XCAP="enabled"
   AC_SUBST(SOUP_CFLAGS)
   AC_SUBST(SOUP_LIBS)
  AC_DEFINE(HAVE_XCAP,1,[XCAP support])
fi

AM_CONDITIONAL(HAVE_XCAP, test "x$found_xcap" = "xyes")

dnl ###############################
dnl   LOUDMOUTH support
dnl ###############################
LOUDMOUTH="disabled"

AC_ARG_ENABLE(loudmouth, AS_HELP_STRING([--enable-loudmouth],[enable experimental Loudmouth support (default is disabled)]),
[if test "x$enableval" = "xyes"; then
  enable_loudmouth=yes
fi],)

if test "x$enable_loudmouth" = "xyes"; then
   PKG_CHECK_MODULES([LOUDMOUTH], [loudmouth-1.0])
   found_loudmouth=yes
   LOUDMOUTH="enabled"
   AC_SUBST(LOUDMOUTH_CFLAGS)
   AC_SUBST(LOUDMOUTH_LIBS)
  AC_DEFINE(HAVE_LOUDMOUTH,1,[Loudmouth support])
fi

AM_CONDITIONAL(HAVE_LOUDMOUTH, test "x$found_loudmouth" = "xyes")


dnl ###############################
dnl   Gnome-Doc-Utils
dnl ###############################
# GDU is disabled for Windows; help is generated separately manually in Makefile
GNOME_DOC_INIT(,,[GDI=no])
if test "x${gm_platform}" != "xmingw" ; then
  GDU="disabled"
  AC_ARG_ENABLE(gdu, AS_HELP_STRING([--enable-gdu],[enable GNOME-Doc-Utils support, i.e. help file generation (default is enabled)]),
[if test "x$enableval" = "xyes"; then
  enable_gdu=yes
fi],enable_gdu=yes)
  if test "x$enable_gdu" = "xyes"; then
    AC_PATH_PROG(SK_CONFIG,scrollkeeper-config,no)
    if test "x$SK_CONFIG" != "xno" -a "x$GDI" != "xno"; then
      GDU="enabled"
      AC_DEFINE(HAVE_GNOME_DOC_UTILS,1,[GNOME-Doc-Utils support])
    fi
  fi
fi

AM_CONDITIONAL(HAVE_GNOME_DOC_UTILS, test "x${enable_gdu}" = "xyes")


dnl ###########################################################################
dnl PTLib + OPAL Headers and Libraries
dnl ###########################################################################
PKG_CHECK_MODULES([PTLIB], [ptlib >= ${PTLIB_REC_VERSION}])
PKG_CHECK_MODULES([OPAL], [opal >= ${OPAL_REC_VERSION}])

# expat is mandatory for presence (at least)
AC_MSG_CHECKING(for PTLIB support for expat)
PTLIB_EXPAT=`$PKG_CONFIG ptlib --variable=PTLIB_EXPAT`
AC_MSG_RESULT($PTLIB_EXPAT)
if test "x$PTLIB_EXPAT" != "xyes"; then
  AC_MSG_ERROR([You need ptlib expat support to compile ekiga])
fi

SUFFIX=
AC_ARG_ENABLE([opal-debug],
              [AS_HELP_STRING([--enable-opal-debug],[link to debug versions of opal and ptlib (opal_d and ptlib_d) (default is disabled)])],
              [if test "x$enableval" = "xyes"; then
                SUFFIX="_d"
              fi])

AC_ARG_ENABLE([static-libs],
              [AS_HELP_STRING([--enable-static-libs],[link to opal and ptlib static libraries (default is disabled)])],
              [if test "x$enableval" = "xyes"; then
                SUFFIX="${SUFFIX}_s"
               fi
              ])

if test "x$SUFFIX" != x; then
    OPAL_LIBS=`$PKG_CONFIG opal --define-variable=suffix=$SUFFIX --libs`
    PTLIB_LIBS=`$PKG_CONFIG ptlib --define-variable=suffix=$SUFFIX --libs`
fi

AC_SUBST(PTLIB_CFLAGS)
AC_SUBST(PTLIB_LIBS)
AC_SUBST(OPAL_CFLAGS)
AC_SUBST(OPAL_LIBS)

# check for H.323 support
AC_ARG_ENABLE(h323, AS_HELP_STRING([--enable-h323],[enable H.323 support (default same as opal)]),
              enable_h323=$enableval)
if test x$enable_h323 != xno; then
  dnl The =1 checks are here only for backward compatibility
  H323=`$PKG_CONFIG --variable=h323 opal`
  if test "x$H323" = "x1"; then
    H323="yes"
  else
    H323=`$PKG_CONFIG --variable=OPAL_H323 opal`
  fi

  if test "x$H323" = "xyes"; then
    AC_DEFINE(HAVE_H323,1,[H323 support])
  else
    if test x$enable_h323 = xyes; then
      AC_MSG_ERROR([To enable H.323 support you need to re-install opal with H.323 support])
    fi
  fi
else
  H323="no"
fi

AM_CONDITIONAL(HAVE_H323, test "x${H323}" = "xyes")

# check for mandatory SIP support in opal
SIP=`$PKG_CONFIG --variable=sip opal`
if test "x$SIP" = "x1"; then
  SIP="yes"
else
  SIP=`$PKG_CONFIG --variable=OPAL_SIP opal`
fi

if test "x$SIP" != "xyes"; then
  AC_MSG_ERROR([You have to compile opal with SIP support. SIP support is mandatory.])
fi


dnl ###############################
dnl DEBUG support
dnl ###############################
has_debug=
AC_ARG_ENABLE([debug],
              [AS_HELP_STRING([--enable-debug],[compile with debug support (default is disabled)])],
              [if test "x$enableval" = "xyes"; then
                CPPFLAGS="-DDEBUG $CPPFLAGS"
                has_debug="yes"
              fi])
AM_CONDITIONAL(DEBUG, test "x$has_debug" = "xyes")


dnl #########################################################################
dnl  Support for internationalization
dnl ########################################################################
GETTEXT_PACKAGE=AC_PACKAGE_NAME
AC_SUBST(GETTEXT_PACKAGE)  # needed by gettext in po directory
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE",[fix])
AM_GLIB_GNU_GETTEXT


dnl ###########################################################################
dnl  Data and configuration directories for the system
dnl ###########################################################################
gnomedatadir=`eval "echo ${datadir}"`
gnomeconfdir=`eval "echo ${sysconfdir}"`
AC_SUBST(gnomedatadir)
AC_SUBST(gnomeconfdir)

# on Windows, data_dir is obtained at run-time, since it depends on
#   the installation directory, so do not define it (to avoids bugs)
if test "x${gm_platform}" != "xmingw" ; then
   if test "x${prefix}" = "xNONE"; then
      AC_DEFINE_UNQUOTED(DATA_DIR, "${ac_default_prefix}/${DATADIRNAME}",[fix])
   else
      AC_DEFINE_UNQUOTED(DATA_DIR, "${prefix}/${DATADIRNAME}",[fix])
   fi
fi

dnl ###########################################################################
dnl  Plugin support
dnl ###########################################################################

case "$host" in
*-mingw*)
        # libtool really installs in ../bin, so we have to add the dummy!
        # and we have to set PLUGIN_DIR to that directory
        bindir='${exec_prefix}'
        stacklibdir='${exec_prefix}'
        plugindir='${exec_prefix}/plugins'
        PLUGIN_DIR='plugins'
        ;;
*)
        versioned_libdir='${libdir}/AC_PACKAGE_NAME/AC_PACKAGE_VERSION'
        stacklibdir=${versioned_libdir}
        plugindir=${versioned_libdir}/plugins
        PLUGIN_DIR='${plugindir}'
        ;;
esac

PLUGINS_LIBTOOL_FLAGS="-no-undefined -avoid-version -module"
STACKLIB_LDFLAGS='-shared -module -no-undefined -avoid-version'

AC_SUBST(bindir)
AC_SUBST(stacklibdir)
AC_SUBST(plugindir)
AC_SUBST(PLUGINS_LIBTOOL_FLAGS)
AC_SUBST(PLUGIN_DIR)
AC_SUBST(STACKLIB_LDFLAGS)

dnl ###########################################################################
dnl  Output the different Makefiles
dnl ###########################################################################
AC_CONFIG_FILES([Makefile
Doxyfile
src/ekiga-config-tool
ekiga.schemas.in
ekiga.desktop.in
lib/Makefile
po/Makefile.in
man/Makefile
src/Makefile
pixmaps/Makefile
sounds/Makefile
help/Makefile
plugins/Makefile
plugins/avahi/Makefile
plugins/evolution/Makefile
plugins/gstreamer/Makefile
plugins/kab/Makefile
plugins/kde/Makefile
plugins/ldap/Makefile
plugins/libnotify/Makefile
plugins/loudmouth/Makefile
plugins/resource-list/Makefile
plugins/xcap/Makefile
])
AC_OUTPUT

dnl ###########################################################################
dnl Summary
dnl ###########################################################################

echo ""
echo "================ Final configuration ==================="
echo "          Installing into prefix  :  $prefix"
echo ""
if test "x${gm_platform}" != "xmingw" ; then
echo "                     GDU support  :  $GDU"
echo "                   GConf support  :  $GCONF"
echo "   Evolution-Data-Server support  :  $EDS"
echo "                  NOTIFY support  :  $NOTIFY"
fi
echo "                    LDAP support  :  $LDAP"
if test "x${gm_platform}" != "xmingw" ; then
echo ""
echo "             Video4Linux support  :  $VIDEO4LINUX"
fi
echo ""
echo "                  XVideo support  :  $XV"
echo "           Shared Memory support  :  $SHM"
if test "x${gm_platform}" = "xmingw" ; then
echo "                 DirectX support  :  $DX"
fi
echo ""
echo "                   H.323 support  :  $H323"
echo ""
if test "x${gm_platform}" != "xmingw" ; then
echo "                    DBUS support  :  $DBUS"
echo "            DBUS service support  :  $DBUS_SERVICE"
echo "             mDNS/DNS-SD support  :  $AVAHI"
fi
echo ""
echo "The following features are experimental -- you've been warned:"
echo "               GStreamer support  :  $GSTREAMER"
echo "            KAddressBook support  :  $KAB"
echo "                     KDE support  :  $KDE"
echo "                    XCAP support  :  $XCAP"
echo "               Loudmouth support  :  $LOUDMOUTH"
echo ""
echo "                         OS Type  :  $target_os"
echo "                    Machine Type  :  $target_cpu"
echo "                      Byte Order  :  $byte_order"
echo ""
echo " If all settings are OK, type make and make install "
echo "========================================================"
