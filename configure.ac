m4_define([libgaril_major_version], [0])
m4_define([libgaril_minor_version], [0])
m4_define([libgaril_micro_version], [1])
m4_define([libgaril_interface_age], [1])
m4_define([libgaril_binary_age],
          [m4_eval(100 * libgaril_minor_version + libgaril_micro_version)])
m4_define([libgaril_version],
          [libgaril_major_version.libgaril_minor_version.libgaril_micro_version])
m4_define([libgaril_api_version], [0.0])

AC_PREREQ([2.62])
AC_INIT([libgaril], [libgaril_version],
        [https://github.com/vicamo/libgaril/issues],
        [libgaril],
        [https://github.com/vicamo/libgaril])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])

# Depends on automake 1.14 or newer so that we don't need AM_PROG_CC_C_O
AM_INIT_AUTOMAKE([1.14 dist-xz foreign subdir-objects no-define -Wall -Werror])
AM_MAINTAINER_MODE([enable])
AM_SILENT_RULES([yes])

GARIL_MAJOR_VERSION=libgaril_major_version
GARIL_MINOR_VERSION=libgaril_minor_version
GARIL_MICRO_VERSION=libgaril_micro_version
GARIL_INTERFACE_AGE=libgaril_interface_age
GARIL_BINARY_AGE=libgaril_binary_age
GARIL_VERSION=libgaril_version
GARIL_API_VERSION=libgaril_api_version
AC_SUBST(GARIL_MAJOR_VERSION)
AC_SUBST(GARIL_MINOR_VERSION)
AC_SUBST(GARIL_MICRO_VERSION)
AC_SUBST(GARIL_INTERFACE_AGE)
AC_SUBST(GARIL_BINARY_AGE)
AC_SUBST(GARIL_VERSION)
AC_SUBST(GARIL_API_VERSION)

m4_define([lt_current], [m4_eval(100 * libgaril_minor_version + libgaril_micro_version - libgaril_interface_age)])
m4_define([lt_revision], [libgaril_interface_age])
m4_define([lt_age], [m4_eval(libgaril_binary_age - libgaril_interface_age)])
LT_VERSION_INFO="lt_current:lt_revision:lt_age"
LT_CURRENT_MINUS_AGE=m4_eval(lt_current - lt_age)
AC_SUBST(LT_VERSION_INFO)
AC_SUBST(LT_CURRENT_MINUS_AGE)

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

AC_CONFIG_FILES([
  garil/garilversion.h
  garil.pc
  Makefile
])

AC_OUTPUT
