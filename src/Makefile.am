## automake.am
##
## Makefile for the glasscoder(1) Audio Encoder.
##
## (C) Copyright 2014 Fred Gleason <fredg@paravelsystems.com>
##
##      $Id: Makefile.am,v 1.3 2014/02/18 20:35:55 cvs Exp $
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

AM_CPPFLAGS = -Wall -DPREFIX=\"$(prefix)\"

# The dependency for qt's Meta Object Compiler (moc)
moc_%.cpp:	%.h
	@MOC@ $< -o $@


bin_PROGRAMS = glasscoder

dist_glasscoder_SOURCES = cmdswitch.cpp cmdswitch.h\
                          connector.cpp connector.h\
                          connectorfactory.cpp connectorfactory.h\
                          icyconnector.cpp icyconnector.h\
                          jack.cpp\
                          layer3.cpp\
                          profile.cpp profile.h\
                          ringbuffer.cpp ringbuffer.h\
                          shout.cpp\
                          glasscoder.cpp glasscoder.h

nodist_glasscoder_SOURCES = moc_connector.cpp\
                            moc_glasscoder.cpp\
                            moc_icyconnector.cpp

glasscoder_LDADD = @SIRLIBS@
glasscoder_LDFLAGS = @SIRFLAGS@

CLEANFILES = *~\
             moc_*\
             *.obj\
             *.idb\
             *.pdb\
             *ilk

MAINTAINERCLEANFILES = *~\
                       Makefile.in
