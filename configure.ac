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
AM_INIT_AUTOMAKE([1.14 dist-xz foreign subdir-objects -Wall -Werror])
AM_MAINTAINER_MODE([enable])
AM_SILENT_RULES([yes])

AC_PROG_CC

AC_OUTPUT([
  Makefile
])
