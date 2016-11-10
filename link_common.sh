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

rm -f src/$DESTDIR/connector.cpp
ln -s ../../src/common/connector.cpp src/$DESTDIR/connector.cpp
rm -f src/$DESTDIR/connector.h
ln -s ../../src/common/connector.h src/$DESTDIR/connector.h

rm -f src/$DESTDIR/logging.cpp
ln -s ../../src/common/logging.cpp src/$DESTDIR/logging.cpp
rm -f src/$DESTDIR/logging.h
ln -s ../../src/common/logging.h src/$DESTDIR/logging.h

rm -f src/$DESTDIR/metaevent.cpp
ln -s ../../src/common/metaevent.cpp src/$DESTDIR/metaevent.cpp
rm -f src/$DESTDIR/metaevent.h
ln -s ../../src/common/metaevent.h src/$DESTDIR/metaevent.h

rm -f src/$DESTDIR/ringbuffer.cpp
ln -s ../../src/common/ringbuffer.cpp src/$DESTDIR/ringbuffer.cpp
rm -f src/$DESTDIR/ringbuffer.h
ln -s ../../src/common/ringbuffer.h src/$DESTDIR/ringbuffer.h

rm -f src/$DESTDIR/glasslimits.h
ln -s ../../src/common/glasslimits.h src/$DESTDIR/glasslimits.h
