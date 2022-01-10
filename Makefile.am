## automake.am
##
## Top level automake.am
##
## Use automake to process this into a Makefile.in
##
##   (C) Copyright 2014-2022 Fred Gleason <fredg@paravelsystems.com>
##
##   This program is free software; you can redistribute it and/or modify
##   it under the terms of the GNU General Public License version 2 as
##   published by the Free Software Foundation.
##
##   This program is distributed in the hope that it will be useful,
##   but WITHOUT ANY WARRANTY; without even the implied warranty of
##   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##   GNU General Public License for more details.
##
##   You should have received a copy of the GNU General Public
##   License along with this program; if not, write to the Free Software
##   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
##

if DOCBOOK_AM
  DOCBOOK_AM_OPT = docs
endif

if PYTHON_AM
  PYTHON_AM_OPT = plugins
endif

SUBDIRS = debian $(DOCBOOK_AM_OPT) conf\
          icons $(PYTHON_AM_OPT) \
          src\
          xdg

deb:	dist
	./build_debs.sh

rpm:	dist
	cp glasscoder-$(VERSION).tar.gz $(HOME)/rpmbuild/SOURCES/
	$(RPMBUILD) -ba --target $(ARCH)-$(VENDOR)-linux glasscoder.spec
	mv $(HOME)/rpmbuild/RPMS/$(ARCH)/glasscoder-*.rpm $(top_srcdir)/
	mv $(HOME)/rpmbuild/SRPMS/glasscoder-*.src.rpm $(top_srcdir)/
	rm $(HOME)/rpmbuild/SOURCES/glasscoder-$(VERSION).tar.gz
	rm -rf $(HOME)/rpmbuild/BUILD/glasscoder-$(VERSION)

EXTRA_DIST = autogen.sh\
             CODINGSTYLE\
             get_distro.pl\
             get_target.sh\
             glasscoder.spec.in\
             link_common.sh\
             PACKAGE_VERSION

CLEANFILES = *~\
             *.buildinfo\
             *.changes\
             *.ddeb\
             *.deb\
             *.dsc\
             *.mo\
             *.rpm\
             *.tar.gz\
             *.tar.xz\
             *.tgz\
             *.zip

DISTCLEANFILES = build_debs.sh\
                 glasscoder.spec\
                 config.guess\
                 config.sub\
                 ltmain.sh

MAINTAINERCLEANFILES = *~\
                       *.tar.gz\
                       aclocal.m4\
                       compile\
                       config.guess\
                       config.status\
                       config.sub\
                       configure\
                       depcomp\
                       install-sh\
                       libtool\
                       ltmain.sh\
                       Makefile.in\
                       missing\
                       mkinstalldirs
