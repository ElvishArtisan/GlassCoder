## automake.am
##
## icons/automake.am for GlassCoder
##
## (C) Copyright 2015 Fred Gleason <fredg@paravelsystems.com>
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
##
## Use automake to process this into a Makefile.in

install-exec-am:	
	mkdir -p $(DESTDIR)@prefix@/share/icons/hicolor/16x16/apps
	cp glasscoder-16x16.png $(DESTDIR)@prefix@/share/icons/hicolor/16x16/apps/glasscoder.png
	mkdir -p $(DESTDIR)@prefix@/share/icons/hicolor/22x22/apps
	cp glasscoder-22x22.png $(DESTDIR)@prefix@/share/icons/hicolor/22x22/apps/glasscoder.png
	mkdir -p $(DESTDIR)@prefix@/share/icons/hicolor/48x48/apps
	cp glasscoder-48x48.png $(DESTDIR)@prefix@/share/icons/hicolor/48x48/apps/glasscoder.png


uninstall-local:	
	rm -f $(DESTDIR)@prefix@/share/icons/hicolor/16x16/apps/glasscoder.png
	rm -f $(DESTDIR)@prefix@/share/icons/hicolor/22x22/apps/glasscoder.png
	rm -f $(DESTDIR)@prefix@/share/icons/hicolor/48x48/apps/glasscoder.png

EXTRA_DIST = back.xpm\
             glasscoder-16x16.png\
             glasscoder-16x16.xpm\
             glasscoder-22x22.png\
             glasscoder-48x48.png\
             minussign.xpm\
             plussign.xpm

CLEANFILES = *~

MAINTAINERCLEANFILES = *~\
                       aclocal.m4\
                       configure\
                       Makefile.in
