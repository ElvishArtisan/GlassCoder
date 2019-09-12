## automake.am
##
## Makefile for the glasscoder(1) Audio Encoder.
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

AM_CPPFLAGS = -Wall -DPREFIX=\"$(prefix)\" -Wno-strict-aliasing -std=c++11 -fPIC @QT5_CLI_CFLAGS@ @SNDFILE_CFLAGS@ @OPENSSL_CFLAGS@ @ALSA_CFLAGS@
MOC = @QT5_MOC@

# The dependency for qt's Meta Object Compiler (moc)
moc_%.cpp:	%.h
	@MOC@ $< -o $@


bin_PROGRAMS = glasscoder

dist_glasscoder_SOURCES = alsadevice.cpp alsadevice.h\
                          asihpidevice.cpp asihpidevice.h\
                          audiodevice.cpp audiodevice.h\
                          audiodevicefactory.cpp audiodevicefactory.h\
                          codec.cpp codec.h\
                          codecfactory.cpp codecfactory.h\
                          config.cpp config.h\
                          connector.cpp connector.h\
                          connectorfactory.cpp connectorfactory.h\
                          fdkcodec.cpp fdkcodec.h\
                          fileconnector.cpp fileconnector.h\
                          filearchiveconnector.cpp filearchiveconnector.h\
                          fileconveyor.cpp fileconveyor.h\
                          filedevice.cpp filedevice.h\
                          glasscoder.cpp glasscoder.h\
                          hlsconnector.cpp hlsconnector.h\
                          httpconnection.cpp httpconnection.h\
                          httpserver.cpp httpserver.h\
                          httpuser.cpp httpuser.h\
                          iceconnector.cpp iceconnector.h\
                          iceoutconnector.cpp iceoutconnector.h\
                          icestreamconnector.cpp icestreamconnector.h\
                          icyconnector.cpp icyconnector.h\
                          jackdevice.cpp jackdevice.h\
                          metaserver.cpp metaserver.h\
                          meteraverage.cpp meteraverage.h\
                          mpegl2codec.cpp mpegl2codec.h\
                          mpegl3codec.cpp mpegl3codec.h\
                          pcm16codec.cpp pcm16codec.h\
                          opuscodec.cpp opuscodec.h\
                          profile.cpp profile.h\
                          socketmessage.cpp socketmessage.h\
                          socketserver.cpp socketserver.h\
                          vorbiscodec.cpp vorbiscodec.h

nodist_glasscoder_SOURCES = asihpi.cpp asihpi.h\
                            cmdswitch.cpp cmdswitch.h\
                            glasslimits.h\
                            logging.cpp logging.h\
                            metaevent.cpp metaevent.h\
                            moc_alsadevice.cpp\
                            moc_asihpidevice.cpp\
                            moc_audiodevice.cpp\
                            moc_codec.cpp\
                            moc_connector.cpp\
                            moc_fdkcodec.cpp\
                            moc_fileconnector.cpp\
                            moc_filearchiveconnector.cpp\
                            moc_fileconveyor.cpp\
                            moc_filedevice.cpp\
                            moc_glasscoder.cpp\
                            moc_hlsconnector.cpp\
                            moc_httpconnection.cpp\
                            moc_httpserver.cpp\
                            moc_iceconnector.cpp\
                            moc_iceoutconnector.cpp\
                            moc_icestreamconnector.cpp\
                            moc_icyconnector.cpp\
                            moc_jackdevice.cpp\
                            moc_metaserver.cpp\
                            moc_mpegl2codec.cpp\
                            moc_mpegl3codec.cpp\
                            moc_opuscodec.cpp\
                            moc_pcm16codec.cpp\
                            moc_socketserver.cpp\
                            moc_vorbiscodec.cpp\
                            ringbuffer.cpp ringbuffer.h

glasscoder_LDADD = @LIBJACK@ @SNDFILE_LIBS@ @OPENSSL_LIBS@ @ALSA_LIBS@ @ASIHPI_LIBS@ @QT5_CLI_LIBS@ -lsamplerate -ldl -lpthread
#glasscoder_LDFLAGS = 

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
