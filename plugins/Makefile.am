## Makefile.am
##
## Makefile.am for GlassCoder 'plugins/'
##
##   (C) Copyright 2019 Fred Gleason <fredg@paravelsystems.com>
##
##   This program is free software; you can redistribute it and/or modify
##   it under the terms of the GNU General Public License as
##   published by the Free Software Foundation; either version 2 of
##   the License, or (at your option) any later version.
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
	mkdir -p $(DESTDIR)@libdir@/rivendell/pypad
	cp pypad_glasscoder.exemplar $(DESTDIR)@libdir@/rivendell/pypad/
	./install_pypad.sh pypad_glasscoder.py $(DESTDIR)@libdir@/rivendell/pypad/pypad_glasscoder.py


uninstall-local:	
	rm -f $(DESTDIR)@libdir@/rivendell/pypad/pypad_glasscoder.exemplar
	rm -f $(DESTDIR)@libdir@/rivendell/pypad/pypad_glasscoder.py


EXTRA_DIST = install_pypad.sh.in\
             pypad_glasscoder.exemplar\
             pypad_glasscoder.py


CLEANFILES = *~\
             *.idb\
             *ilk\
             *.obj\
             *.pdb\
             *.qm\
             moc_*

DISTCLEANFILES = install_pypad.sh

MAINTAINERCLEANFILES = *~\
                       *.tar.gz\
                       aclocal.m4\
                       configure\
                       Makefile.in\
                       moc_*
