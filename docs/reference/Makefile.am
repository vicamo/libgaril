ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}

MAINTAINERCLEANFILES =

AM_CPPFLAGS = \
  -I$(top_builddir) \
  -I$(top_srcdir)

# gtk-doc.make defines EXTRA_DIST and CLEANFILES
include $(top_srcdir)/gtk-doc.make

# The name of the module.
DOC_MODULE = garil

# The top-level SGML file.
DOC_MAIN_SGML_FILE = $(DOC_MODULE)-docs.xml

# The directory containing the source code. Relative to $(srcdir).
# gtk-doc will search all .c & .h files beneath here for inline comments
# documenting functions and macros.
DOC_SOURCE_DIR = \
  $(top_srcdir)/garil \
  $(top_builddir)/garil

SCAN_OPTIONS = \
  --rebuild-types

# Extra options to supply to gtkdoc-scangobj.
SCANGOBJ_OPTIONS =

# Extra options to supply to gtkdoc-mkdb
MKDB_OPTIONS = \
  --output-format=xml \
  --name-space=$(DOC_MODULE)

# Extra options to supply to gtkdoc-fixref.
FIXXREF_OPTIONS =

# Used for dependencies.
HFILE_GLOB = \
  $(top_srcdir)/garil/*.h
CFILE_GLOB = \
  $(top_srcdir)/garil/*.c

# Header files to ignore when scanning.
IGNORE_HFILES =

# Extra XML files that are included by $(DOC_MAIN_SGML_FILE).
content_files = \
  version.xml

# Other files to distribute.
extra_files =

# CFLAGS and LDFLAGS for compiling scan program. Only needed
# if $(DOC_MODULE).types is non-empty.
GTKDOC_CFLAGS = \
  $(BASE_DEPENDENCIES_CFLAGS) \
  -I$(top_builddir) \
  -I$(top_srcdir) \
  -DHAVE_CONFIG_H \
  -DLIBGARIL_COMPILATION

GTKDOC_LIBS = \
  $(BASE_DEPENDENCIES_LIBS) \
  $(top_builddir)/garil/libgaril.la

CLEANFILES += \
  html/* \
  garil-decl-list.txt \
  garil-decl.txt \
  garil-overrides.txt \
  garil-sections.txt \
  garil.types \
  xml/*

EXTRA_DIST += \
  version.xml.in

TESTS = $(GTKDOC_CHECK)

all-local: check-doc-coverage
.PHONY: check-doc-coverage
check-doc-coverage: $(DOC_MODULE)-undocumented.txt
	$(AM_V_at) O=$$(cat $< | $(GREP) "^0 not documented.$$"); \
	if test -z "$$O"; then echo "# undocumented:"; cat $<; exit 1; fi

-include $(top_srcdir)/git.mk
