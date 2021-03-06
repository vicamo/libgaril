m4_define([garil_major_version], [0])
m4_define([garil_minor_version], [0])
m4_define([garil_micro_version], [1])
m4_define([garil_interface_age], [1])
m4_define([garil_binary_age],
          [m4_eval(100 * garil_minor_version + garil_micro_version)])
m4_define([garil_version],
          [garil_major_version.garil_minor_version.garil_micro_version])
m4_define([garil_api_version], [0.0])

AC_PREREQ([2.62])
AC_INIT([garil], [garil_version],
        [https://github.com/vicamo/libgaril/issues],
        [garil],
        [https://github.com/vicamo/libgaril])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])

# Depends on automake 1.14 or newer so that we don't need AM_PROG_CC_C_O
AM_INIT_AUTOMAKE([1.14 dist-xz foreign subdir-objects no-define -Wall])
AM_MAINTAINER_MODE([enable])
AM_SILENT_RULES([yes])

GARIL_MAJOR_VERSION=garil_major_version
GARIL_MINOR_VERSION=garil_minor_version
GARIL_MICRO_VERSION=garil_micro_version
GARIL_INTERFACE_AGE=garil_interface_age
GARIL_BINARY_AGE=garil_binary_age
GARIL_VERSION=garil_version
GARIL_API_VERSION=garil_api_version
AC_SUBST(GARIL_MAJOR_VERSION)
AC_SUBST(GARIL_MINOR_VERSION)
AC_SUBST(GARIL_MICRO_VERSION)
AC_SUBST(GARIL_INTERFACE_AGE)
AC_SUBST(GARIL_BINARY_AGE)
AC_SUBST(GARIL_VERSION)
AC_SUBST(GARIL_API_VERSION)

m4_define([lt_current], [m4_eval(100 * garil_minor_version + garil_micro_version - garil_interface_age)])
m4_define([lt_revision], [garil_interface_age])
m4_define([lt_age], [m4_eval(garil_binary_age - garil_interface_age)])
LT_VERSION_INFO="lt_current:lt_revision:lt_age"
LT_CURRENT_MINUS_AGE=m4_eval(lt_current - lt_age)
AC_SUBST(LT_VERSION_INFO)
AC_SUBST(LT_CURRENT_MINUS_AGE)

AC_CANONICAL_HOST

AC_MSG_CHECKING([for Unix])
case "$host" in
  *-*-mingw*)
    garil_os_unix=no
    ;;
  *)
    garil_os_unix=yes
    ;;
esac
AC_MSG_RESULT([$garil_os_unix])

AM_CONDITIONAL(OS_UNIX, [test "$garil_os_unix" = "yes"])

AC_PROG_CC

# Define _GNU_SOURCE etc. where appropriate, e.g. for strptime() on glibc
AC_USE_SYSTEM_EXTENSIONS
# Make sure we use 64-bit versions of various file stuff.
AC_SYS_LARGEFILE

AM_PROG_AR

dnl Initialize libtool
LT_PREREQ([2.2.6])
LT_INIT([disable-static])
# Create libtool early, because it's used in configure
LT_OUTPUT

PKG_PROG_PKG_CONFIG

m4_define([gio_required_version], [2.22])

PKG_CHECK_MODULES(BASE_DEPENDENCIES,
                  [gio-2.0 >= gio_required_version])

GARIL_PACKAGES="gio-2.0 >= gio_required_version"
AC_SUBST(GARIL_PACKAGES)

if test "$garil_os_unix" = "yes"; then
  PKG_CHECK_MODULES(GIO_UNIX, [gio-unix-2.0 >= gio_required_version])
fi

GLIB_MKENUMS=`$PKG_CONFIG --variable=glib_mkenums glib-2.0`
AC_SUBST(GLIB_MKENUMS)

# GTK-DOC generation
GTK_DOC_CHECK([1.20],[--flavour no-tmpl])

GLIB_TESTS

AC_CONFIG_FILES([
  docs/reference/Makefile
  docs/reference/version.xml
  garil/garilversion.h
  garil.pc
  Makefile
])

AC_OUTPUT
