## automake.am
##
## Makefile for GlassCoder Audio Encoder tests.
##
## (C) Copyright 2014-2019 Fred Gleason <fredg@paravelsystems.com>
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

AM_CPPFLAGS = -Wall -DPREFIX=\"$(prefix)\" -Wno-strict-aliasing -std=c++11 -fPIC @QT5_CLI_CFLAGS@ @SNDFILE_CFLAGS@ @ALSA_CFLAGS@
MOC = @QT5_MOC@

# The dependency for qt's Meta Object Compiler (moc)
moc_%.cpp:	%.h
	@MOC@ $< -o $@


noinst_PROGRAMS = pipe_connect\
                  urldecode\
                  urlencode


dist_pipe_connect_SOURCES = pipe_connect.cpp pipe_connect.h
nodist_pipe_connect_SOURCES = cmdswitch.cpp cmdswitch.h\
                              moc_pipe_connect.cpp
pipe_connect_LDADD = @SIRLIBS@ @LIBJACK@ @SNDFILE_LIBS@ @ALSA_LIBS@ @ASIHPI_LIBS@ @QT5_CLI_LIBS@ -lpthread
pipe_connect_LDFLAGS = @SIRFLAGS@

dist_urldecode_SOURCES = urldecode.cpp urldecode.h
nodist_urldecode_SOURCES = cmdswitch.cpp cmdswitch.h\
                           connector.cpp connector.h\
                           logging.cpp logging.h\
                           metaevent.cpp metaevent.h\
                           moc_connector.cpp\
                           moc_urldecode.cpp
urldecode_LDADD = @SIRLIBS@ @LIBJACK@ @SNDFILE_LIBS@ @ALSA_LIBS@ @ASIHPI_LIBS@ @QT5_CLI_LIBS@ -lpthread
urldecode_LDFLAGS = @SIRFLAGS@

dist_urlencode_SOURCES = urlencode.cpp urlencode.h
nodist_urlencode_SOURCES = cmdswitch.cpp cmdswitch.h\
                           connector.cpp connector.h\
                           logging.cpp logging.h\
                           metaevent.cpp metaevent.h\
                           moc_connector.cpp\
                           moc_urlencode.cpp
urlencode_LDADD = @SIRLIBS@ @LIBJACK@ @SNDFILE_LIBS@ @ALSA_LIBS@ @ASIHPI_LIBS@ @QT5_CLI_LIBS@ -lpthread
urlencode_LDFLAGS = @SIRFLAGS@

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
                 codecdialog.cpp codecdialog.h\
                 codeviewer.cpp codeviewer.h\
                 combobox.cpp combobox.h\
                 connector.cpp connector.h\
                 glasslimits.h\
                 guiapplication.cpp guiapplication.h\
                 hpiinputlistview.cpp hpiinputlistview.h\
                 hpiwidget.cpp hpiwidget.h\
                 logging.cpp logging.h\
                 messagewidget.cpp messagewidget.h\
                 metaevent.cpp metaevent.h\
                 paths.h\
                 profile.cpp profile.h\
                 ringbuffer.cpp ringbuffer.h\
                 segmeter.cpp segmeter.h\
                 serverdialog.cpp serverdialog.h\
                 sourcedialog.cpp sourcedialog.h\
                 spinbox.cpp spinbox.h\
                 statuswidget.cpp statuswidget.h\
                 stereometer.cpp stereometer.h\
                 streamdialog.cpp streamdialog.h

MAINTAINERCLEANFILES = *~\
                       Makefile.in
