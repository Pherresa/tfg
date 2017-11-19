AUTOMAKE_OPTIONS = 1.4

EXTRA_DIST=CONTRIBUTORS acconfig.h
if BUILD_DOC
DOC = doc
endif
SUBDIRS = src docs intl po debian $(DOC)
DIST_SUBDIRS = src docs intl po debian doc

dist-hook:
#	-cvs2cl -w --stdout >$(distdir)/ChangeLog
	-rcs2log -c "" >$(distdir)/ChangeLog

if MAINTAINER_MODE
include $(GNOME_ACLOCAL_DIR)/gnome-macros.dep
endif
