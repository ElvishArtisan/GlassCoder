// audiodevicefactory.cpp
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

#include "alsadevice.h"
#include "asihpidevice.h"
#include "filedevice.h"
#include "jackdevice.h"
#include "audiodevicefactory.h"
AudioDevice *AudioDeviceFactory(AudioDevice::DeviceType type,
				unsigned chans,unsigned samprate,
				Ringbuffer *ring,
				QObject *parent)
{
  AudioDevice *dev=NULL;

  switch(type) {
  case AudioDevice::Alsa:
#ifdef ALSA
    dev=new AlsaDevice(chans,samprate,ring,parent);
#endif  // ALSA
    break;

  case AudioDevice::AsiHpi:
#ifdef ASIHPI
    dev=new AsiHpiDevice(chans,samprate,ring,parent);
#endif  // ASIHPI
    break;

  case AudioDevice::File:
#ifdef SNDFILE
    dev=new FileDevice(chans,samprate,ring,parent);
#endif  // SNDFILE
    break;

  case AudioDevice::Jack:
#ifdef JACK
    dev=new JackDevice(chans,samprate,ring,parent);
#endif  // JACK
    break;

  case AudioDevice::LastType:
    break;
  }

  if(dev!=NULL) {
    if(!dev->isAvailable()) {
      delete dev;
      dev=NULL;
    }
  }

  return dev;
}
