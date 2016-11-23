## automake.am
##
## docs/Automake.am for GlassCoder
##
##   (C) Copyright 2014-2015 Fred Gleason <fredg@paravelsystems.com>
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
## by Fred Gleason <fredg@paravelsystems.com>
##
## Use automake to process this into a Makefile.in

# The dependency for man pages
%.1:	%.xml
	xsltproc $(DOCBOOK_STYLESHEETS)/manpages/docbook.xsl $<
%.7:	%.xml
	xsltproc $(DOCBOOK_STYLESHEETS)/manpages/docbook.xsl $<
%.html:	%.xml
	xsltproc -o $@ $(DOCBOOK_STYLESHEETS)/xhtml/docbook.xsl $<

all-local:	glasscoder.1 glasscoder.html\
                glasscoder-ipc.7 glasscoder-ipc.xml\
                glasscommander.1 glasscommander.html\
                glassgui.1 glassgui.html

man_MANS = glasscoder.1\
           glasscoder-ipc.7\
           glasscommander.1\
           glassgui.1

EXTRA_DIST = glasscoder.1\
             glasscoder.html\
             glasscoder.xml.in\
             glasscoder-ipc.7\
             glasscoder-ipc.html\
             glasscoder-ipc.xml\
             glasscommander.1\
             glasscommander.html\
             glasscommander.xml\
             glassgui.1\
             glassgui.html\
             glassgui.xml

CLEANFILES = *~
DISTCLEANFILES = *.1\
                 *.7\
                 *.html\
                 glasscoder.xml\
                 moc_*
MAINTAINERCLEANFILES = *~\
                       *.fo\
                       *.pdf\
                       Makefile.in
