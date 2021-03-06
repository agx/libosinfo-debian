
SUBDIRS = osinfo test tools docs po examples

INTLTOOL_FILES = \
	intltool-extract.in \
	intltool-merge.in \
	intltool-update.in \
	$(NULL)

DISTCLEANFILES =				\
	intltool-extract			\
	intltool-merge				\
	intltool-update				\
	po/.intltool-merge-cache		\
	$(NULL)

EXTRA_DIST = \
  COPYING.LIB \
  libosinfo.spec \
  libosinfo.spec.in \
  mingw-libosinfo.spec.in \
  build-aux/mktempd \
  cfg.mk \
  GNUmakefile \
  maint.mk \
  AUTHORS.in \
  $(INTLTOOL_FILES) \
  $(NULL)

MAINTAINERCLEANFILES =				\
	po/Makefile.in.in			\
	po/Makevars.template			\
	po/Rules-quot				\
	po/boldquot.sed				\
	po/en@boldquot.header			\
	po/en@quot.header			\
	po/insert-header.sin			\
	po/quot.sed				\
	po/remove-potcdate.sin			\
	$(NULL)

DISTCHECK_CONFIGURE_FLAGS=--enable-gtk-doc

ACLOCAL_AMFLAGS = -I m4


CLEANFILES =

if LOCAL_USB_IDS
usb_database_DATA = usb.ids
usb_databasedir = $(pkgdatadir)/
usb.ids:
	-wget -q -O $@ http://www.linux-usb.org/usb.ids
CLEANFILES += usb.ids
endif

if LOCAL_PCI_IDS
pci_database_DATA = pci.ids
pci_databasedir = $(pkgdatadir)/
pci.ids:
	-wget -q -O $@ http://pciids.sourceforge.net/v2.2/pci.ids
CLEANFILES += pci.ids
endif

LCOV = lcov
GENHTML = genhtml

cov: clean-cov
	mkdir $(top_builddir)/coverage
	$(LCOV) -c -o $(top_builddir)/coverage/libosinfo.info.tmp \
	  -d $(top_builddir)/osinfo
	$(LCOV) -r $(top_builddir)/coverage/libosinfo.info.tmp \
	  -o $(top_builddir)/coverage/libosinfo.info
	rm $(top_builddir)/coverage/libosinfo.info.tmp
	$(GENHTML) --show-details -t "libosinfo" -o $(top_builddir)/coverage \
	  --legend $(top_builddir)/coverage/libosinfo.info


clean-cov:
	rm -rf $(top_builddir)/coverage

dist-hook: gen-ChangeLog gen-AUTHORS

# Generate the ChangeLog file (with all entries since the switch to git)
# and insert it into the directory we're about to use to create a tarball.
.PHONY: gen-ChangeLog gen-AUTHORS
gen-ChangeLog:
	if test -d $(srcdir)/.git; then				\
	  $(top_srcdir)/build-aux/gitlog-to-changelog		\
	    > $(distdir)/cl-t;					\
	  rm -f $(distdir)/ChangeLog;				\
	  mv $(distdir)/cl-t $(distdir)/ChangeLog;		\
	fi

gen-AUTHORS:
	$(AM_V_GEN)if test -d $(srcdir)/.git; then                      \
	   out="`cd $(srcdir) && git log --pretty=format:'%aN <%aE>' | sort -u`" && \
	  perl -p -e "s/#authorslist#// and print '$$out'"            \
	     < $(srcdir)/AUTHORS.in > $(distdir)/AUTHORS-tmp &&        \
	   mv -f $(distdir)/AUTHORS-tmp $(distdir)/AUTHORS ;           \
	fi
