## automake.am
##
## Makefile for the glasscoder(1) Audio Encoder.
##
## (C) Copyright 2014-2015 Fred Gleason <fredg@paravelsystems.com>
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

AM_CPPFLAGS = -Wall -DPREFIX=\"$(prefix)\" -Wno-strict-aliasing @QT4_CFLAGS@ @SNDFILE_CFLAGS@
MOC = @QT4_MOC@

# The dependency for qt's Meta Object Compiler (moc)
moc_%.cpp:	%.h
	@MOC@ $< -o $@


bin_PROGRAMS = glasscoder

dist_glasscoder_SOURCES = aaccodec.cpp aaccodec.h\
                          audiodevice.cpp audiodevice.h\
                          audiodevicefactory.cpp audiodevicefactory.h\
                          cmdswitch.cpp cmdswitch.h\
                          codec.cpp codec.h\
                          codecfactory.cpp codecfactory.h\
                          connector.cpp connector.h\
                          connectorfactory.cpp connectorfactory.h\
                          filedevice.cpp filedevice.h\
                          glasscoder.cpp glasscoder.h\
                          glasslimits.h\
                          heaaccodec.cpp heaaccodec.h\
                          hlsconnector.cpp hlsconnector.h\
                          iceconnector.cpp iceconnector.h\
                          icyconnector.cpp icyconnector.h\
                          jackdevice.cpp jackdevice.h\
                          mpegl2codec.cpp mpegl2codec.h\
                          mpegl3codec.cpp mpegl3codec.h\
                          opuscodec.cpp opuscodec.h\
                          profile.cpp profile.h\
                          ringbuffer.cpp ringbuffer.h\
                          vorbiscodec.cpp vorbiscodec.h

nodist_glasscoder_SOURCES = moc_aaccodec.cpp\
                            moc_audiodevice.cpp\
                            moc_codec.cpp\
                            moc_connector.cpp\
                            moc_filedevice.cpp\
                            moc_glasscoder.cpp\
                            moc_heaaccodec.cpp\
                            moc_hlsconnector.cpp\
                            moc_iceconnector.cpp\
                            moc_icyconnector.cpp\
                            moc_jackdevice.cpp\
                            moc_mpegl2codec.cpp\
                            moc_mpegl3codec.cpp\
                            moc_opuscodec.cpp\
                            moc_vorbiscodec.cpp

glasscoder_LDADD = @SIRLIBS@ @LIBJACK@ @SNDFILE_LIBS@
glasscoder_LDFLAGS = @SIRFLAGS@

CLEANFILES = *~\
             moc_*\
             *.obj\
             *.idb\
             *.pdb\
             *ilk

MAINTAINERCLEANFILES = *~\
                       Makefile.in
