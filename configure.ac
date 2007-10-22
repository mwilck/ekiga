AC_INIT(ekiga, 2.1.0, http://bugzilla.gnome.org/enter_bug.cgi?product=ekiga)
AC_CANONICAL_TARGET
AC_PREREQ(2.53)
AC_CONFIG_SRCDIR(src/gui/main.cpp)
AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)

AM_MAINTAINER_MODE
AM_CONFIG_HEADER(config.h)



dnl ###########################################################################
dnl   Definitions
dnl ###########################################################################

MAJOR_VERSION=2
MINOR_VERSION=1
BUILD_TYPE=ReleaseCode
BUILD_NUMBER=0

SCHEMA_AGE=61

PTLIB_REC_VERSION="2.1.0"
OPAL_REC_VERSION="3.1.0"

AC_DEFINE_UNQUOTED(MAJOR_VERSION, $MAJOR_VERSION,[fix])
AC_DEFINE_UNQUOTED(MINOR_VERSION, $MINOR_VERSION,[fix])
AC_DEFINE_UNQUOTED(BUILD_TYPE, $BUILD_TYPE,[fix])
AC_DEFINE_UNQUOTED(BUILD_NUMBER, $BUILD_NUMBER,[fix])

AC_SUBST(SCHEMA_AGE)


dnl ###########################################################################
dnl   Disable / Enable Main Ekiga Components
dnl ###########################################################################

PKG_PROG_PKG_CONFIG

AM_PROG_LIBTOOL

AC_PROG_CC
AC_PROG_CXX
AC_ISC_POSIX
AC_HEADER_STDC

AC_CHECK_DECLS(strcasecmp)

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
    ARCH_PTLIB_CFLAGS="-DPTRACING -DNDEBUG -DSOLARIS -g "
    ARCH_OPAL_LIBS="-lpt -lopal -ldl -lpthread"
    AC_MSG_RESULT([yes])
    gm_platform="solaris"
    ;;

  linux-gnu | linux-gnueabi | linux | Linux)
    ARCH_PTLIB_CFLAGS="-DPTRACING -DNDEBUG -Wall -Os -g -fno-exceptions -felide-constructors"
    ARCH_OPAL_LIBS="-lpt -lopal -ldl -lpthread"
    AC_MSG_RESULT([yes])
    gm_platform="linux"

dnl if debug add this and link to -lh323_linux_x86_d -lpt_linux_x86_d
dnl -D_DEBUG -DPMEMORY_CHECK=1 -DPTRACING -g
    ;;

  FreeBSD* | freebsd* )
    ARCH_PTLIB_CFLAGS="-DPTRACING -DNDEBUG -Wall -Os -g"
    ARCH_OPAL_LIBS="-lpt -lopal -lpthread"
    AC_MSG_RESULT([yes])
    gm_platform="freebsd"
    ;;

  darwin* )
    ARCH_PTLIB_CFLAGS="-Os -Wall -DPTRACING -DNDEBUG -DNO_LONG_DOUBLE -DSTATIC_LIBS_USED"
    ARCH_OPAL_LIBS="-multiply_defined suppress -lh323_Darwin_ppc_r_s -lpt_Darwin_ppc_r_s -lssl -lcrypto -lX11 -framework AudioToolbox -framework CoreAudio -framework CoreServices"
    AC_MSG_RESULT([yes])
    gm_platform="macosx"
    ;;

  mingw* )
    ARCH_PTLIB_CFLAGS="-Wall -mms-bitfields -DPTRACING -DSTATIC_LIBS_USED"
    ARCH_OPAL_LIBS="-mwindows -enable-auto-import -enable-runtime-pseudo-reloc -enable-stdcall-fixup -lpt -lopal -lwinmm -lwsock32 -lsnmpapi -lmpr -lcomdlg32"
    AC_MSG_RESULT([yes])
    gm_platform="mingw"
    win32=1
    ;;

  *)
    AC_MSG_ERROR([Only Linux, FreeBSD, Solaris and Mac OS X are supported. Please contact the author to know how you can add more OS types.])
    ;;
esac

AM_CONDITIONAL(WIN32, test "x${win32}" = "x1")


dnl ###############################
dnl   Mandatory intltool support
dnl ###############################
IT_PROG_INTLTOOL([0.35.0])


dnl ###############################
dnl   Mandatory GTK support
dnl ###############################
PKG_CHECK_MODULES(GTK, gtk+-2.0 >= 2.10.0)
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)


dnl ###############################
dnl   Mandatory GLIB support
dnl ###############################
PKG_CHECK_MODULES(GLIB, glib-2.0 >= 2.6.0 gmodule-2.0 gobject-2.0 gthread-2.0)
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)


dnl ###############################
dnl   Mandatory SIGC++ support
dnl ###############################
PKG_CHECK_MODULES(SIGC, sigc++-2.0)
AC_SUBST(SIGC_CFLAGS)
AC_SUBST(SIGC_LIBS)


dnl ###############################
dnl   Mandatory libXML support
dnl ###############################
PKG_CHECK_MODULES(XML, libxml-2.0)
AC_SUBST(XML_CFLAGS)
AC_SUBST(XML_LIBS)


dnl ###############################
dnl   GNOME support
dnl ###############################
GNOME="disabled"

if test "x${gm_platform}" != "xmingw" ; then
  AC_ARG_ENABLE(gnome, [  --disable-gnome		Disable GNOME support],,enable_gnome=yes)
  
  if test "x$enable_gnome" = "xyes"; then
    PKG_CHECK_MODULES(GNOME, libgnome-2.0 >= 2.14.0 libgnomeui-2.0 >= 2.14.0, found_gnome=yes)
    AC_SUBST(GNOME_CFLAGS)
    AC_SUBST(GNOME_LIBS)
    GNOME="enabled"
    AC_DEFINE(HAVE_GNOME,1,[GNOME support])
  fi
fi
AM_CONDITIONAL(HAVE_GNOME, test "x$found_gnome" = "xyes")


dnl ###############################
dnl   GConf support
dnl ###############################
GCONF="disabled"

if test "x${gm_platform}" != "xmingw" ; then
  AC_ARG_ENABLE(gconf, [  --disable-gconf		Disable GConf support],,enable_gconf=yes)
  
  if test "x$enable_gconf" = "xyes"; then
    PKG_CHECK_MODULES(GCONF, gconf-2.0 >= 2.6.0, found_gconf=yes)
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
dnl   BONOBO support
dnl ###############################
BONOBO="disabled"

if test "x${gm_platform}" != "xmingw" ; then
  AC_ARG_ENABLE(bonobo, [  --disable-bonobo		Disable Bonobo support],,enable_bonobo=yes)
  
  if test "x$enable_bonobo" = "xyes"; then
    PKG_CHECK_MODULES(BONOBO, libbonobo-2.0, found_bonobo=yes)
    AC_SUBST(BONOBO_CFLAGS)
    AC_SUBST(BONOBO_LIBS)
    BONOBO="enabled"
    AC_DEFINE(HAVE_BONOBO,1,[Bonobo support])
  fi
fi

AM_CONDITIONAL(HAVE_BONOBO, test "x$found_bonobo" = "xyes")


dnl ###############################
dnl   Evolution-data-server support
dnl ###############################
EDS="disabled"

if test "x${gm_platform}" != "xmingw" ; then
  AC_ARG_ENABLE(eds, [  --disable-eds			Disable evolution-data-server addressbook support],,enable_eds=yes)
  
  if test "x$enable_eds" = "xyes"; then
    PKG_CHECK_MODULES(EDS, libebook-1.2, found_eds=yes)
    AC_SUBST(EDS_CFLAGS)
    AC_SUBST(EDS_LIBS)
    EDS="enabled"
    AC_DEFINE(HAVE_EDS,1,[EDS support])
  fi
fi

AM_CONDITIONAL(HAVE_EDS, test "x$found_eds" = "xyes")


dnl ###############################
dnl   LDAP support
dnl ###############################
LDAP="disabled"

AC_ARG_ENABLE(ldap, [  --disable-ldap			Disable LDAP addressbook support],,enable_ldap=yes)

if test "x$enable_ldap" = "xyes"; then

  AC_ARG_WITH(ldap-dir, [  --with-ldap-dir=PFX   Location of LDAP], with_ldap_dir="$withval", with_ldap_dir="/usr")

  dnl Check for the includes presence
  AC_MSG_CHECKING(for LDAP includes in ${with_ldap_dir}/include/)
  AC_MSG_RESULT()


  if test -f ${with_ldap_dir}/include/ldap.h; then
  	LDAP_CFLAGS="-I${with_ldap_dir}/include"
  else
  	AC_MSG_ERROR(You need the LDAP headers to compile Ekiga with LDAP support)
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
  	AC_MSG_ERROR(You need the LDAP library to compile Ekiga with LDAP support)
  fi

  dnl Checking for libresolv
  if test ${gm_platform} = "linux" ; then
  	AC_CHECK_LIB(resolv, res_gethostbyaddr, [LIBS="-lresolv $LIBS"], AC_MSG_ERROR([*** libresolv not found.]), -lresolv)
  	LDAP_LIBS="${LDAP_LIBS} -lresolv"
  fi
  found_ldap="yes"
  AC_SUBST(LDAP_CFLAGS)
  AC_SUBST(LDAP_LIBS)
  LDAP="enabled"
  AC_DEFINE(HAVE_LDAP,1,[LDAP support])
fi

AM_CONDITIONAL(HAVE_LDAP, test "x$found_ldap" = "xyes")


dnl ###############################
dnl   ESound support
dnl ###############################
ESD="disabled"

if test "x${gm_platform}" != "xmingw" ; then
  AC_ARG_ENABLE(esd, [  --disable-esd			Disable ESound support],,enable_esd=yes)

  if test "x$enable_esd" = "xyes"; then
    PKG_CHECK_MODULES(ESD, esound >= 0.2.28, found_esd=yes)
    AC_SUBST(ESD_CFLAGS)
    AC_SUBST(ESD_LIBS)
    ESD="enabled"
    AC_DEFINE(HAVE_ESD,1,[ESound support])
  fi
fi

AM_CONDITIONAL(HAVE_ESD, test "x$found_esd" = "xyes")


dnl ###############################
dnl Checks for Shared Memory extension
dnl ###############################
SHM="disabled"

if test "x${gm_platform}" = "xlinux" ; then
  AC_CHECK_LIB(Xext, XShmAttach, with_shm="yes", with_shm="no")
  if test "x${with_shm}" != "xno"; then
  SHM="enabled"
  AC_DEFINE(HAVE_SHM,1,[SHM Support])
  fi
fi
AM_CONDITIONAL(HAVE_SHM, test "x$SHM" = "xenabled")

dnl ###############################
dnl Checks for Xv extension
dnl ###############################
XV="disabled"

if test "x${gm_platform}" = "xlinux" ; then
  AC_ARG_ENABLE(xv, [  --disable-xv			Disable XVideo hardware acceleration],,enable_xv=yes)

  if test "x${enable_xv}" = "xyes" ; then
    AC_CHECK_LIB(Xv, XvPutImage, xv_libs="yes", xv_libs="no")
    if test "x${xv_libs}" != "xno"; then
      XV_LIBS="-lXv"
      AC_SUBST(XV_LIBS)
      XV="enabled"
      AC_DEFINE(HAVE_XV,1,[XVideo support])
    fi
  fi
fi

AM_CONDITIONAL(HAVE_XV, test "x$XV" = "xenabled")


dnl ###########################################################################
dnl Checks for DX extension
dnl ###########################################################################
DX="disabled"

if test "x${gm_platform}" = "xmingw" ; then
  AC_ARG_ENABLE(directx, [  --disable-directx		Disable DirectDraw hardware acceleration],,enable_directx=yes)

  if test "x${enable_directx}" = "xyes" ; then
    AC_ARG_WITH(directx-dir, [  --with-directx-dir=PFX		Location of DirectDraw], with_directx_dir="$withval", with_directx_dir=".")

    dnl Check for the includes presence
    AC_MSG_CHECKING(for DirectX includes in ${with_directx_dir}/include)
    AC_MSG_RESULT()
    if test -d ${with_directx_dir}/include; then
      DX_CFLAGS="-I${with_directx_dir}/include"
      DX_LIBS="$EKIGA_LIBS -lddraw"
      AC_SUBST(DX_LIBS)
      AC_SUBST(DX_CFLAGS)
      DX="enabled"
      AC_DEFINE(HAVE_DX,1,[DirectDraw support])
    fi
  fi
fi

AM_CONDITIONAL(HAVE_DX, test "x$DX" = "xenabled")


dnl ###############################
dnl   DBUS Support
dnl ###############################
DBUS="disabled"

if test "x${gm_platform}" != "xmingw" ; then
  AC_ARG_ENABLE(dbus, [  --disable-dbus		Disable DBUS support],,enable_dbus=yes)

  if test "x$enable_dbus" = "xyes"; then
    PKG_CHECK_MODULES(DBUS, dbus-1 >= 0.36 dbus-glib-1 >= 0.36, found_dbus=yes)
    AC_SUBST(DBUS_CFLAGS)
    AC_SUBST(DBUS_LIBS)
    DBUS="enabled"
    AC_DEFINE(HAVE_DBUS,1,[DBUS support])
  fi
fi

AM_CONDITIONAL(HAVE_DBUS, test "x$found_dbus" = "xyes")
AM_CONDITIONAL(DBUS_SERVICES_INSTALL, test "x$found_dbus" = "xyes")


dnl ###############################
dnl   Avahi / mDNS Support
dnl ###############################
AVAHI="disabled"

if test "x${gm_platform}" != "xmingw" ; then
  AC_ARG_ENABLE(avahi, [  --disable-avahi		Disable mDNS support],,enable_avahi=yes)

  if test "x$enable_avahi" = "xyes"; then
    PKG_CHECK_MODULES(AVAHI, avahi-client >= 0.6 avahi-glib >= 0.6, found_avahi=yes)
    AC_SUBST(AVAHI_CFLAGS)
    AC_SUBST(AVAHI_LIBS)
    AVAHI="enabled"
    AC_DEFINE(HAVE_AVAHI,1,[Avahi support])
  fi
fi

AM_CONDITIONAL(HAVE_AVAHI, test "x$found_avahi" = "xyes")


dnl ###############################
dnl   Gnome-Doc-Utils
dnl ###############################
GDU="disabled"
if test "x${gm_platform}" != "xmingw" ; then
  AC_ARG_ENABLE(gdu, [  --disable-gdu			Disable GNOME-Doc-Utils support],,enable_gdu=yes)

  if test "x$enable_gdu" = "xyes"; then
GNOME_DOC_INIT
    AC_PATH_PROG(SK_CONFIG,scrollkeeper-config,no)
    if test "x$SK_CONFIG" = "xno"; then
      AC_MSG_ERROR(Couldn't find scrollkeeper-config. Please install the scrollkeeper package: http://scrollkeeper.sourceforge.net)
    fi
    GDU="enabled"
  else
    ENABLE_SK_TRUE="#"
    ENABLE_SK_FALSE=""
  fi
else
  ENABLE_SK_TRUE="#"
  ENABLE_SK_FALSE=""
fi

AM_CONDITIONAL(HAVE_GNOME_DOC_UTILS, test "x${enable_gdu}" = "xyes")


dnl ###########################################################################
dnl PWLib Headers and Libraries
dnl ###########################################################################
AC_ARG_WITH(ptlib-dir, [  --with-ptlib-dir=PFX			Location of PWLib], with_ptlib_dir="$withval", with_ptlib_dir="/usr")

dnl Check for the includes presence
AC_MSG_CHECKING(for PWLib includes in ${with_ptlib_dir}/include/)
AC_MSG_RESULT()

if test -f ${with_ptlib_dir}/include/ptlib/pprocess.h; then
	PTLIB_CFLAGS="-I${with_ptlib_dir}/include/ptlib -I${with_ptlib_dir}/include/ptclib"
	if test "x${with_ptlib_dir}" != "x/usr"; then
		PTLIB_CFLAGS="${PTLIB_CFLAGS} -I${with_ptlib_dir}/include"
	fi	
else
	AC_MSG_ERROR(You need the PWLib headers to compile Ekiga)
fi

dnl Checking for the library presence
LDFLAGS_save="$LDFLAGS"
AC_LANG(C++)
LDFLAGS="$LDFLAGS -L${with_ptlib_dir}/${libname}/"
AC_CHECK_LIB(pt, read, ptlib_libs="yes", ptlib_libs="no")
LDFLAGS="${LDFLAGS_save}"

if test "x${ptlib_libs}" != "xno"; then
	if test "x${with_ptlib_dir}" != "x/usr"; then
		PTLIB_LIBS="-L${with_ptlib_dir}/${libname}"
	fi
else
	AC_MSG_ERROR(You need the PWLib library to compile Ekiga)
fi

dnl Checking for PWLib version
CPPFLAGS_save="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $PTLIB_CFLAGS"
AC_MSG_CHECKING(for PWLib version);
AC_EGREP_CPP(${PTLIB_REC_VERSION},
	     [#include <ptbuildopts.h>
	      PTLIB_VERSION 
	     ],AC_MSG_RESULT(${PTLIB_REC_VERSION}), AC_MSG_ERROR(Sorry but the recommended PWLib version is ${PTLIB_REC_VERSION}))
CPPFLAGS="$CPPFLAGS_save"

AC_LANG(C)
if test -f ${with_ptlib_dir}/make/ptlib-config; then
    AC_PATH_PROG(PTLIB_CONFIG, ptlib-config, , ${with_ptlib_dir}/make/)
else
    if test -f ${with_ptlib_dir}/bin/ptlib-config; then
        AC_PATH_PROG(PTLIB_CONFIG, ptlib-config, , ${with_ptlib_dir}/bin/)
    else
        AC_MSG_ERROR([Sorry but ptlib-config can not be found, please check your installation]);
    fi
fi

dnl Substitute
PTLIB_CFLAGS="$ARCH_PTLIB_CFLAGS $PTLIB_CFLAGS `$PTLIB_CONFIG --ccflags`"
AC_SUBST(PTLIB_CFLAGS)
AC_SUBST(PTLIB_LIBS)
                  

dnl ###########################################################################
dnl Opal Headers and Libraries
dnl ###########################################################################
AC_ARG_WITH(opal-dir, [  --with-opal-dir=PFX			Location of Opal], with_opal_dir="$withval", with_opal_dir="/usr")

dnl Check for the includes presence
AC_MSG_CHECKING(for Opal includes in ${with_opal_dir}/include)
AC_MSG_RESULT()
if test -f ${with_opal_dir}/include/opal/opal/call.h; then
	OPAL_CFLAGS="-I${with_opal_dir}/include/opal"
else
	if test -f ${with_opal_dir}/include/opal/call.h; then
	    OPAL_CFLAGS="-I${with_opal_dir}/include"
	else
    	    AC_MSG_ERROR(You need the Opal headers to compile Ekiga)
	fi
fi
dnl Checking for the library presence
LIBS_save="$LIBS"
LIBS="${LIBS} -L${with_opal_dir}/${libname}/ -L${with_ptlib_dir}/${libname}/ -lpt"
AC_CHECK_LIB(opal, main, opal_libs="yes", opal_libs="yes")
opal_libs="yes"
LIBS="${LIBS_save}"

if test "x${opal_libs}" != "xno"; then
	if test "x${with_opal_dir}" != "x/usr"; then
		OPAL_LIBS="-L${with_opal_dir}/lib"
	fi
else
	AC_MSG_ERROR(You need the Opal library to compile Ekiga)
fi

dnl Checking for Opal version
CPPFLAGS="$CPPFLAGS $OPAL_CFLAGS"
AC_MSG_CHECKING(for Opal version);
AC_EGREP_CPP(${OPAL_REC_VERSION},
	     [#include <opal/buildopts.h>
	      OPAL_VERSION 
	     ],AC_MSG_RESULT(${OPAL_REC_VERSION}), AC_MSG_ERROR(Sorry but the recommended OPAL version is ${OPAL_REC_VERSION}))
CPPFLAGS="$CPPFLAGS_save"

dnl Substitute
OPAL_LIBS="$ARCH_OPAL_LIBS $OPAL_LIBS"
AC_SUBST(OPAL_CFLAGS)
AC_SUBST(OPAL_LIBS)


dnl #########################################################################
dnl  Support for internationalization
dnl ########################################################################
GETTEXT_PACKAGE=AC_PACKAGE_NAME
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE",[fix])
AM_GLIB_GNU_GETTEXT


dnl #########################################################################
dnl Set PACKAGE_LOCALE_DIR in config.h.
dnl #########################################################################
if test "x${prefix}" = "xNONE"; then
  AC_DEFINE_UNQUOTED(PACKAGE_LOCALE_DIR, "${ac_default_prefix}/${DATADIRNAME}/locale",[fix])
else
  AC_DEFINE_UNQUOTED(PACKAGE_LOCALE_DIR, "${prefix}/${DATADIRNAME}/locale",[fix])
fi

AH_BOTTOM(
#ifndef _
#include <libintl.h>
#define _(x) gettext(x)
#ifdef gettext_noop
#define N_(String) gettext_noop (String)
#else
#define N_(String) (String)
#endif
#endif
)


dnl ###########################################################################
dnl  Data and configuration directories for the system
dnl ###########################################################################
gnomedatadir=`eval "echo ${datadir}"`
gnomeconfdir=`eval "echo ${sysconfdir}"`
AC_SUBST(gnomedatadir)
AC_SUBST(gnomeconfdir)

 
dnl ###########################################################################
dnl  Output the different Makefiles
dnl ###########################################################################
AC_OUTPUT(
Makefile
src/ekiga-config-tool
AC_PACKAGE_NAME.schemas.in
AC_PACKAGE_NAME.desktop.in
net.ekiga.instance.service
net.ekiga.helper.service
lib/Makefile
lib/druid/Makefile
lib/gmconf/Makefile
lib/gui/Makefile
lib/toolbox/Makefile
lib/platform/Makefile
po/Makefile.in
man/Makefile
src/Makefile
pixmaps/Makefile
sounds/Makefile
help/Makefile
lib/engine/Makefile
lib/engine/addressbook/Makefile
lib/engine/addressbook/evolution/Makefile
lib/engine/addressbook/ldap/Makefile
lib/engine/addressbook/skel/Makefile
lib/engine/framework/Makefile
lib/engine/presence/Makefile
lib/engine/presence/skel/Makefile
lib/engine/presence/local-roster/Makefile
lib/engine/protocol/Makefile
lib/engine/protocol/sip/Makefile
lib/engine/gui/Makefile
lib/engine/gui/gtk-core/Makefile
lib/engine/gui/gtk-frontend/Makefile
)


dnl ###########################################################################
dnl Summary
dnl ###########################################################################

echo ""
echo "================ Final configuration ==================="
echo "          Installing into prefix  :  $prefix"
echo ""
echo "                 OPAL Version is  :  $OPAL_REC_VERSION"
echo "               OPAL Directory is  :  $with_opal_dir"
echo "                PTLIB Version is  :  $PTLIB_REC_VERSION"
echo "              PTLIB Directory is  :  $with_ptlib_dir"
echo "                 ptlib-config is  :  $with_ptlib_dir/bin/ptlib-config"
echo ""
echo ""
if test "x${gm_platform}" != "xmingw" ; then
echo "                   GNOME support  :  $GNOME"
echo "                   GConf support  :  $GCONF"
echo "   Evolution-Data-Server support  :  $EDS"
echo "                  Esound support  :  $ESD"
fi
echo "                    LDAP support  :  $LDAP"
echo ""
if test "x${gm_platform}" = "xlinux" ; then
echo "                  XVideo support  :  $XV"
fi
if test "x${gm_platform}" = "xmingw" ; then
echo "                 DirectX support  :  $DX"
fi
echo ""
if test "x${gm_platform}" != "xmingw" ; then
echo "                    DBUS support  :  $DBUS"
echo "             mDNS/DNS-SD support  :  $AVAHI"
fi
echo ""
echo "                         OS Type  :  $target_os"
echo "                    Machine Type  :  $target_cpu"
echo ""
echo " If all settings are OK, type make and make install "
echo "========================================================"
