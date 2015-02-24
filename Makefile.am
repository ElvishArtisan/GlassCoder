## automake.am
##
## Top level automake.am
##
## Use automake to process this into a Makefile.in
##
##   (C) Copyright 2014 Fred Gleason <fredg@paravelsystems.com>
##
##      $Id: Makefile.am,v 1.1.1.1 2014/02/17 13:26:17 cvs Exp $
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

SUBDIRS = docs\
          src

rpm:	dist
	cp glasscoder-$(VERSION).tar.gz $(HOME)/rpmbuild/SOURCES/
	$(RPMBUILD) -ba --target $(ARCH)-$(VENDOR)-linux glasscoder.spec
	mv $(HOME)/rpmbuild/RPMS/$(ARCH)/glasscoder-*.rpm $(top_srcdir)/
	mv $(HOME)/rpmbuild/SRPMS/glasscoder-*.src.rpm $(top_srcdir)/
	rm $(HOME)/rpmbuild/SOURCES/glasscoder-$(VERSION).tar.gz
	rm -rf $(HOME)/rpmbuild/BUILD/glasscoder-$(VERSION)

EXTRA_DIST = glasscoder.spec.in\
             autogen.sh\
             get_distro.sh\
             get_target.sh\
             PACKAGE_VERSION

CLEANFILES = *~\
             *.deb\
             *.mo\
             *.rpm\
             *.tar.gz\
             *.tgz\
             *.zip

DISTCLEANFILES = glasscoder.spec\
                 config.guess\
                 config.sub\
                 ltmain.sh

MAINTAINERCLEANFILES = *~\
                       *.tar.gz\
                       aclocal.m4\
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
