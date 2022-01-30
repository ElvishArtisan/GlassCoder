## automake.am
##
## Makefile for the glasscommander(1) Audio Encoder front-end.
##
## (C) Copyright 2016-2022 Fred Gleason <fredg@paravelsystems.com>
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

AM_CPPFLAGS = -Wall -DPREFIX=\"$(prefix)\" -Wno-strict-aliasing -std=c++11 -fPIC @QT5_GUI_CFLAGS@ @SNDFILE_CFLAGS@ @ALSA_CFLAGS@
MOC = @QT5_MOC@

# The dependency for qt's Meta Object Compiler (moc)
moc_%.cpp:	%.h
	@MOC@ $< -o $@


bin_PROGRAMS = glasscommander

dist_glasscommander_SOURCES = configdialog.cpp configdialog.h\
                              deletedialog.cpp deletedialog.h\
                              filenamevalidator.cpp filenamevalidator.h\
                              glasscommander.cpp glasscommander.h\
                              glasswidget.cpp glasswidget.h\
                              instancedialog.cpp instancedialog.h\
                              playmeter.cpp playmeter.h

nodist_glasscommander_SOURCES = asihpi.cpp asihpi.h\
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
                                moc_audiodevice.cpp\
                                moc_codec.cpp\
                                moc_codecdialog.cpp\
                                moc_codeviewer.cpp\
                                moc_combobox.cpp\
                                moc_configdialog.cpp\
                                moc_connector.cpp\
                                moc_deletedialog.cpp\
                                moc_filenamevalidator.cpp\
                                moc_glasscommander.cpp\
                                moc_glasswidget.cpp\
                                moc_guiapplication.cpp\
                                moc_hpiinputlistview.cpp\
                                moc_hpiwidget.cpp\
                                moc_instancedialog.cpp\
                                moc_messagewidget.cpp\
                                moc_playmeter.cpp\
                                moc_segmeter.cpp\
                                moc_serverdialog.cpp\
                                moc_sourcedialog.cpp\
                                moc_spinbox.cpp\
                                moc_statuswidget.cpp\
                                moc_stereometer.cpp\
                                moc_streamdialog.cpp\
                                profile.cpp profile.h\
                                ringbuffer.cpp ringbuffer.h\
                                segmeter.cpp segmeter.h\
                                serverdialog.cpp serverdialog.h\
                                sourcedialog.cpp sourcedialog.h\
                                spinbox.cpp spinbox.h\
                                statuswidget.cpp statuswidget.h\
                                stereometer.cpp stereometer.h\
                                streamdialog.cpp streamdialog.h

glasscommander_LDADD = @GUILIBS@ @LIBJACK@ @SNDFILE_LIBS@ @ALSA_LIBS@ @ASIHPI_LIBS@ @QT5_GUI_LIBS@ -lpthread
glasscommander_LDFLAGS = @GUIFLAGS@

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
