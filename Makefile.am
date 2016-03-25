ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}

include $(top_srcdir)/glib-tap.mk

DISTCHECK_CONFIGURE_FLAGS = \
  --disable-maintainer-mode

GITIGNOREFILES = \
  $(DIST_ARCHIVES)

EXTRA_DIST += \
  autogen.sh \
  INSTALL \
  README \
  tap-driver.sh \
  tap-test

MAINTAINERCLEANFILES += \
  $(DIST_ARCHIVES) \
  $(GITIGNORE_MAINTAINERCLEANFILES_TOPLEVEL) \
  $(GITIGNORE_MAINTAINERCLEANFILES_M4_LIBTOOL) \
  $(GITIGNORE_MAINTAINERCLEANFILES_MAKEFILE_IN) \
  $(srcdir)/ChangeLog \
  $(srcdir)/configure \
  $(srcdir)/INSTALL \
  $(srcdir)/README

AM_CPPFLAGS = \
  -DG_LOG_DOMAIN=\"garil\" \
  -I$(top_builddir) \
  -I$(top_srcdir)

AM_CFLAGS = \
  -Wall -Werror

lib_LTLIBRARIES =

garilincdir = $(includedir)/garil-$(GARIL_API_VERSION)
garilinc_HEADERS = \
  garil/garil.h \
  garil/garilversion.h

###############################
## libgaril

lib_LTLIBRARIES += garil/libgaril.la

garil_public_headers = \
  garil/garilclient.h \
  garil/garilconnection.h

garil_libgaril_la_SOURCES = \
  $(garil_public_headers) \
  garil/garilclient.c \
  garil/garilconnection.c

garil_libgaril_la_CFLAGS = \
  $(BASE_DEPENDENCIES_CFLAGS) \
  -DLIBGARIL_COMPILATION

garil_libgaril_la_LIBADD = \
  $(BASE_DEPENDENCIES_LIBS)

garil_libgaril_la_LDFLAGS = \
  -version-info $(LT_VERSION_INFO)

garilinc_HEADERS += $(garil_public_headers)

###############################
## libgaril - enums

garil_libgaril_enum_csources = \
  garil/garilenumtypes.c \
  garil/garilenumtypes.h

garil_libgaril_enum_cheaders = \
  garil/garilconnection.h

$(garil_libgaril_enum_csources): Makefile.am $(garil_libgaril_enum_cheaders) $(garil_libgaril_enum_csources:=.template)
	$(AM_V_GEN) $(GLIB_MKENUMS) \
	  --template $@.template \
	  $(garil_libgaril_enum_cheaders) > $@

BUILT_SOURCES += $(garil_libgaril_enum_csources)
garil_libgaril_la_SOURCES += $(garil_libgaril_enum_csources)
garilinc_HEADERS += garil/garilenumtypes.h

EXTRA_DIST += \
   $(garil_libgaril_enum_csources:=.template)

###############################
## tests

test_cflags = \
  $(BASE_DEPENDENCIES_CFLAGS)
test_ldadd = \
  $(BASE_DEPENDENCIES_LIBS) \
  garil/libgaril.la

if OS_UNIX
test_cflags += $(GIO_UNIX_CFLAGS)
test_ldadd += $(GIO_UNIX_LIBS)
endif

test_programs = \
  tests/test-connection

tests_test_connection_CFLAGS = $(test_cflags)
tests_test_connection_LDADD = $(test_ldadd)

###############################
## pkg-config DATA

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = garil.pc

DISTCLEANFILES += $(pkgconfig_DATA)

distclean-local:
	if test "$(srcdir)" = "."; then :; else \
	  rm -f ChangeLog; \
	fi

.PHONY: ChangeLog
ChangeLog:
	$(AM_V_GEN) if test -d "$(srcdir)/.git"; then \
	  (GIT_DIR=$(top_srcdir)/.git $(top_builddir)/build-aux/missing git log --stat) | fmt --split-only > $@.tmp \
	  && mv -f $@.tmp $@ \
	  || ($(RM) $@.tmp; \
	      echo Failed to generate ChangeLog, your ChangeLog may be outdated >&2; \
	      (test -f $@ || echo git-log is required to generate this file >> $@)); \
	else \
	  test -f $@ || \
	  (echo A git checkout and git-log is required to generate ChangeLog >&2 && \
	  echo A git checkout and git-log is required to generate this file >> $@); \
	fi

-include $(top_srcdir)/git.mk
