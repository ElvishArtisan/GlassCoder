#!/bin/sh

# link_common.sh
#
#  Link common sources for rivendell-browser
#
#   (C) Copyright 2016 Fred Gleason <fredg@paravelsystems.com>
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License version 2 as
#   published by the Free Software Foundation.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public
#   License along with this program; if not, write to the Free Software
#   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#

DESTDIR=$1

rm -f src/$DESTDIR/asihpi.cpp
ln -s ../../src/common/asihpi.cpp src/$DESTDIR/asihpi.cpp
rm -f src/$DESTDIR/asihpi.h
ln -s ../../src/common/asihpi.h src/$DESTDIR/asihpi.h

rm -f src/$DESTDIR/audiodevice.cpp
ln -s ../../src/common/audiodevice.cpp src/$DESTDIR/audiodevice.cpp
rm -f src/$DESTDIR/audiodevice.h
ln -s ../../src/common/audiodevice.h src/$DESTDIR/audiodevice.h

rm -f src/$DESTDIR/cmdswitch.cpp
ln -s ../../src/common/cmdswitch.cpp src/$DESTDIR/cmdswitch.cpp
rm -f src/$DESTDIR/cmdswitch.h
ln -s ../../src/common/cmdswitch.h src/$DESTDIR/cmdswitch.h

rm -f src/$DESTDIR/codec.cpp
ln -s ../../src/common/codec.cpp src/$DESTDIR/codec.cpp
rm -f src/$DESTDIR/codec.h
ln -s ../../src/common/codec.h src/$DESTDIR/codec.h

rm -f src/$DESTDIR/codecdialog.cpp
ln -s ../../src/common/codecdialog.cpp src/$DESTDIR/codecdialog.cpp
rm -f src/$DESTDIR/codecdialog.h
ln -s ../../src/common/codecdialog.h src/$DESTDIR/codecdialog.h

rm -f src/$DESTDIR/codeviewer.cpp
ln -s ../../src/common/codeviewer.cpp src/$DESTDIR/codeviewer.cpp
rm -f src/$DESTDIR/codeviewer.h
ln -s ../../src/common/codeviewer.h src/$DESTDIR/codeviewer.h

rm -f src/$DESTDIR/combobox.cpp
ln -s ../../src/common/combobox.cpp src/$DESTDIR/combobox.cpp
rm -f src/$DESTDIR/combobox.h
ln -s ../../src/common/combobox.h src/$DESTDIR/combobox.h

rm -f src/$DESTDIR/connector.cpp
ln -s ../../src/common/connector.cpp src/$DESTDIR/connector.cpp
rm -f src/$DESTDIR/connector.h
ln -s ../../src/common/connector.h src/$DESTDIR/connector.h

rm -f src/$DESTDIR/guiapplication.cpp
ln -s ../../src/common/guiapplication.cpp src/$DESTDIR/guiapplication.cpp
rm -f src/$DESTDIR/guiapplication.h
ln -s ../../src/common/guiapplication.h src/$DESTDIR/guiapplication.h

rm -f src/$DESTDIR/hpiinputlistview.cpp
ln -s ../../src/common/hpiinputlistview.cpp src/$DESTDIR/hpiinputlistview.cpp
rm -f src/$DESTDIR/hpiinputlistview.h
ln -s ../../src/common/hpiinputlistview.h src/$DESTDIR/hpiinputlistview.h

rm -f src/$DESTDIR/hpiwidget.cpp
ln -s ../../src/common/hpiwidget.cpp src/$DESTDIR/hpiwidget.cpp
rm -f src/$DESTDIR/hpiwidget.h
ln -s ../../src/common/hpiwidget.h src/$DESTDIR/hpiwidget.h

rm -f src/$DESTDIR/logging.cpp
ln -s ../../src/common/logging.cpp src/$DESTDIR/logging.cpp
rm -f src/$DESTDIR/logging.h
ln -s ../../src/common/logging.h src/$DESTDIR/logging.h

rm -f src/$DESTDIR/messagewidget.cpp
ln -s ../../src/common/messagewidget.cpp src/$DESTDIR/messagewidget.cpp
rm -f src/$DESTDIR/messagewidget.h
ln -s ../../src/common/messagewidget.h src/$DESTDIR/messagewidget.h

rm -f src/$DESTDIR/metaevent.cpp
ln -s ../../src/common/metaevent.cpp src/$DESTDIR/metaevent.cpp
rm -f src/$DESTDIR/metaevent.h
ln -s ../../src/common/metaevent.h src/$DESTDIR/metaevent.h

rm -f src/$DESTDIR/profile.cpp
ln -s ../../src/common/profile.cpp src/$DESTDIR/profile.cpp
rm -f src/$DESTDIR/profile.h
ln -s ../../src/common/profile.h src/$DESTDIR/profile.h

rm -f src/$DESTDIR/ringbuffer.cpp
ln -s ../../src/common/ringbuffer.cpp src/$DESTDIR/ringbuffer.cpp
rm -f src/$DESTDIR/ringbuffer.h
ln -s ../../src/common/ringbuffer.h src/$DESTDIR/ringbuffer.h

rm -f src/$DESTDIR/segmeter.cpp
ln -s ../../src/common/segmeter.cpp src/$DESTDIR/segmeter.cpp
rm -f src/$DESTDIR/segmeter.h
ln -s ../../src/common/segmeter.h src/$DESTDIR/segmeter.h

rm -f src/$DESTDIR/serverdialog.cpp
ln -s ../../src/common/serverdialog.cpp src/$DESTDIR/serverdialog.cpp
rm -f src/$DESTDIR/serverdialog.h
ln -s ../../src/common/serverdialog.h src/$DESTDIR/serverdialog.h

rm -f src/$DESTDIR/sourcedialog.cpp
ln -s ../../src/common/sourcedialog.cpp src/$DESTDIR/sourcedialog.cpp
rm -f src/$DESTDIR/sourcedialog.h
ln -s ../../src/common/sourcedialog.h src/$DESTDIR/sourcedialog.h

rm -f src/$DESTDIR/spinbox.cpp
ln -s ../../src/common/spinbox.cpp src/$DESTDIR/spinbox.cpp
rm -f src/$DESTDIR/spinbox.h
ln -s ../../src/common/spinbox.h src/$DESTDIR/spinbox.h

rm -f src/$DESTDIR/statuswidget.cpp
ln -s ../../src/common/statuswidget.cpp src/$DESTDIR/statuswidget.cpp
rm -f src/$DESTDIR/statuswidget.h
ln -s ../../src/common/statuswidget.h src/$DESTDIR/statuswidget.h

rm -f src/$DESTDIR/stereometer.cpp
ln -s ../../src/common/stereometer.cpp src/$DESTDIR/stereometer.cpp
rm -f src/$DESTDIR/stereometer.h
ln -s ../../src/common/stereometer.h src/$DESTDIR/stereometer.h

rm -f src/$DESTDIR/streamdialog.cpp
ln -s ../../src/common/streamdialog.cpp src/$DESTDIR/streamdialog.cpp
rm -f src/$DESTDIR/streamdialog.h
ln -s ../../src/common/streamdialog.h src/$DESTDIR/streamdialog.h

rm -f src/$DESTDIR/glasslimits.h
ln -s ../../src/common/glasslimits.h src/$DESTDIR/glasslimits.h

rm -f src/$DESTDIR/paths.h
ln -s ../../src/common/paths.h src/$DESTDIR/paths.h
