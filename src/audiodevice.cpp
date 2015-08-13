// audiodevice.cpp
//
// Abstract base class for audio input sources.
//
//   (C) Copyright 2014-2015 Fred Gleason <fredg@paravelsystems.com>
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

#include "audiodevice.h"

AudioDevice::AudioDevice(unsigned chans,unsigned samprate,
			 std::vector<Ringbuffer *> *rings,QObject *parent)
  : QObject(parent)
{
  audio_rings=rings;
  audio_channels=chans;
  audio_samplerate=samprate;
}


AudioDevice::~AudioDevice()
{
}


unsigned AudioDevice::deviceSamplerate() const
{
  return audio_samplerate;
}


QString AudioDevice::deviceTypeText(AudioDevice::DeviceType type)
{
  QString ret=tr("Unknown Device");

  switch(type) {
  case AudioDevice::File:
    ret=tr("File Streaming");
    break;

  case AudioDevice::Jack:
    ret=tr("JACK Audio Connection Kit");
    break;
  }

  return ret;
}


QString AudioDevice::optionKeyword(AudioDevice::DeviceType type)
{
  QString ret;

  switch(type) {
  case AudioDevice::File:
    ret="file";
    break;

  case AudioDevice::Jack:
    ret="jack";
    break;
  }

  return ret;
}


unsigned AudioDevice::ringBufferQuantity() const
{
  return audio_rings->size();
}


Ringbuffer *AudioDevice::ringBuffer(unsigned n)
{
  return audio_rings->at(n);
}


unsigned AudioDevice::channels() const
{
  return audio_channels;
}


unsigned AudioDevice::samplerate() const
{
  return audio_samplerate;
}
