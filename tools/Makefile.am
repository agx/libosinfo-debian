AM_CFLAGS = $(GOBJECT_CFLAGS) \
	    $(GIO_CFLAGS)     \
	    $(GLIB_CFLAGS)    \
	    $(LIBXML_CFLAGS)  \
	    -DPKGDATADIR="\"$(pkgdatadir)\"" \
	    -DLOCALEDIR="\"$(datadir)/locale\"" \
	    $(WARN_CFLAGS) \
	    -I$(top_srcdir) \
            $(NULL)

bin_PROGRAMS = osinfo-detect osinfo-query osinfo-install-script

man1_MANS = osinfo-detect.1 osinfo-query.1 osinfo-install-script.1

CLEANFILES = $(man1_MANS)

POD2MAN = pod2man -c "Virtualization Support" -r "$(PACKAGE)-$(VERSION)"

%.1: %.c Makefile
	$(AM_V_GEN)$(POD2MAN) $< $@

osinfo_detect_SOURCES = osinfo-detect.c
osinfo_detect_LDADD = $(GOBJECT_LIBS)		\
		      $(GIO_LIBS)		\
		      $(GLIB_LIBS)		\
		      $(LIBXML_LIBS)		\
		      $(top_builddir)/osinfo/libosinfo-1.0.la

osinfo_query_SOURCES = osinfo-query.c
osinfo_query_LDADD = $(GOBJECT_LIBS)		\
		      $(GIO_LIBS)		\
		      $(GLIB_LIBS)		\
		      $(top_builddir)/osinfo/libosinfo-1.0.la

osinfo_install_script_SOURCES = osinfo-install-script.c
osinfo_install_script_LDADD = $(GOBJECT_LIBS)	\
		      $(GIO_LIBS)		\
		      $(GLIB_LIBS)		\
		      $(LIBXML_LIBS)		\
		      $(top_builddir)/osinfo/libosinfo-1.0.la
