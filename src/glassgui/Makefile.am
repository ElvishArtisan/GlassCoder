## automake.am
##
## Makefile for the glassgui(1) Audio Encoder front-end.
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

AM_CPPFLAGS = -Wall -DPREFIX=\"$(prefix)\" -Wno-strict-aliasing @QT4_CFLAGS@ @SNDFILE_CFLAGS@ @ALSA_CFLAGS@
MOC = @QT4_MOC@

# The dependency for qt's Meta Object Compiler (moc)
moc_%.cpp:	%.h
	@MOC@ $< -o $@


bin_PROGRAMS = glassgui

dist_glassgui_SOURCES = codecdialog.cpp codecdialog.h\
                        codeviewer.cpp codeviewer.h\
                        combobox.cpp combobox.h\
                        glassgui.cpp glassgui.h\
                        hpiinputlistview.cpp hpiinputlistview.h\
                        hpiwidget.cpp hpiwidget.h\
                        profile.cpp profile.h\
                        segmeter.cpp segmeter.h\
                        serverdialog.cpp serverdialog.h\
                        sourcedialog.cpp sourcedialog.h\
                        statuswidget.cpp statuswidget.h\
                        stereometer.cpp stereometer.h\
                        streamdialog.cpp streamdialog.h

nodist_glassgui_SOURCES = asihpi.cpp asihpi.h\
                          audiodevice.cpp audiodevice.h\
                          cmdswitch.cpp cmdswitch.h\
                          codec.cpp codec.h\
                          connector.cpp connector.h\
                          glasslimits.h\
                          logging.cpp logging.h\
                          moc_audiodevice.cpp\
                          moc_codec.cpp\
                          moc_codecdialog.cpp\
                          moc_codeviewer.cpp\
                          moc_combobox.cpp\
                          moc_connector.cpp\
                          moc_glassgui.cpp\
                          moc_hpiinputlistview.cpp\
                          moc_hpiwidget.cpp\
                          moc_segmeter.cpp\
                          moc_serverdialog.cpp\
                          moc_sourcedialog.cpp\
                          moc_statuswidget.cpp\
                          moc_stereometer.cpp\
                          moc_streamdialog.cpp\
                          ringbuffer.cpp ringbuffer.h

glassgui_LDADD = @GUILIBS@ @LIBJACK@ @SNDFILE_LIBS@ @ALSA_LIBS@ @ASIHPI_LIBS@ -lpthread
glassgui_LDFLAGS = @GUIFLAGS@

CLEANFILES = *~\
             moc_*\
             *.obj\
             *.idb\
             *.pdb\
             *ilk

DISTCLEANFILES = asihpi.cpp asihpi.h\
                 audiodevice.cpp audiodevice.h\
                 cmdswitch.cpp cmdswitch.h\
                 codec.cpp codec.h\
                 connector.cpp connector.h\
                 glasslimits.h\
                 logging.cpp logging.h\
                 ringbuffer.cpp ringbuffer.h

MAINTAINERCLEANFILES = *~\
                       Makefile.in
