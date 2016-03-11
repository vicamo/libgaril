ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}

DISTCHECK_CONFIGURE_FLAGS = \
  --disable-maintainer-mode

GITIGNOREFILES = \
  $(DIST_ARCHIVES)

EXTRA_DIST = \
  autogen.sh \
  INSTALL \
  README

MAINTAINERCLEANFILES = \
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

garilincdir = $(includedir)/garil-$(GARIL_API_VERSION)
garilinc_HEADERS = \
  garil/garil.h \
  garil/version.h

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
