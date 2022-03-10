// audiodevicefactory.h
//
// Instantiate AudioDevice classes
//
//   (C) Copyright 2014-2022 Fred Gleason <fredg@paravelsystems.com>
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License version 2 as
//   published by the Free Software Foundation.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public
//   License along with this program; if not, write to the Free Software
//   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//

#ifndef AUDIODEVICEFACTORY_H
#define AUDIODEVICEFACTORY_H

#include "audiodevice.h"

AudioDevice *AudioDeviceFactory(AudioDevice::DeviceType type,
				unsigned chans,unsigned samprate,
				Ringbuffer *ring,QObject *parent=0);


#endif  // AUDIODEVICEFACTORY_H
