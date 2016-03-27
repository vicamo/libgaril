#!/bin/sh

test -n "$srcdir" || srcdir=`dirname "$0"`
test -n "$srcdir" || srcdir=.

olddir=`pwd`
cd "$srcdir"

mkdir -p m4

GTKDOCIZE=`which gtkdocize`
if test -z $GTKDOCIZE; then
  echo "*** No GTK-Doc found, please install it ***"
  exit 1
else
  gtkdocize || exit $?
fi

# README and INSTALL are required by automake, but may be deleted by clean
# up rules. to get automake to work, simply touch these here, they will be
# regenerated from their corresponding *.in files by ./configure anyway.
touch README INSTALL

AUTORECONF=`which autoreconf`
if test -z $AUTORECONF; then
  echo "*** No autoreconf found, please install it ***"
  exit 1
fi

autoreconf --force --install --verbose || exit $?

cd "$olddir"
test -n "$NOCONFIGURE" || "$srcdir/configure" "$@"
