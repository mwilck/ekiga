#!/bin/sh
# Run this to generate all the initial makefiles, etc.
srcdir=`dirname $0`
PROJECT="gnomemeeting"
TEST_TYPE=-d



DIE=0

(autoconf --version) < /dev/null > /dev/null 2>&1 || {
	echo
	echo "You must have autoconf installed to compile $PROJECT."
	echo "Download the appropriate package for your distribution,"
	echo "or get the source tarball at ftp://ftp.gnu.org/pub/gnu/"
	DIE=1
}

(automake --version) < /dev/null > /dev/null 2>&1 || {
	echo
	echo "You must have automake installed to compile $PROJECT."
	echo "Get ftp://ftp.cygnus.com/pub/home/tromey/automake-1.2d.tar.gz"
	echo "(or a newer version if it is available)"
	DIE=1
}

if test "$DIE" -eq 1; then
	exit 1
fi

case $CC in
*lcc | *lcc\ *) am_opt=--include-deps;;
esac

echo "Running autoconf"
aclocal  -I ./macros $ACLOCAL_FLAGS

echo "Running automake"
automake -a --gnu $am_opt
autoconf

echo "Running configure"
$srcdir/configure "$@"

