## automake.am
##
## xdg/automake.am for GlassCoder
##
##   (C) Copyright 2015 Fred Gleason <fredg@paravelsystems.com>
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
## Use automake to process this into a Makefile.in

install-exec-am:	
	mkdir -p $(DESTDIR)@prefix@/share/applications
	cp glasscommander.desktop $(DESTDIR)@prefix@/share/applications/
	cp glassgui.desktop $(DESTDIR)@prefix@/share/applications/

uninstall-local:	
	rm -f $(DESTDIR)@prefix@/share/applications/glasscommander.desktop
	rm -f $(DESTDIR)@prefix@/share/applications/glassgui.desktop

EXTRA_DIST = glasscommander.desktop\
             glassgui.desktop

CLEANFILES = *~

MAINTAINERCLEANFILES = *~\
                       Makefile.in
