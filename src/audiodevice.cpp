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

#include <string.h>
#include <syslog.h>

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


void AudioDevice::remixChannels(float *pcm_out,unsigned chans_out,
				float *pcm_in,unsigned chans_in,unsigned frames)
{
  if(chans_out==chans_in) {
    memcpy(pcm_out,pcm_in,frames*chans_in*sizeof(float));
    return;
  }
  if((chans_in==1)&&(chans_out==2)) {
    for(unsigned i=0;i<frames;i++) {
      pcm_out[2*i]=pcm_in[i];
      pcm_out[2*i+1]=pcm_in[i];
    }
    return;
  }
  if((chans_in==2)&&(chans_out==1)) {
    for(unsigned i=0;i<frames;i++) {
      pcm_out[i]=(pcm_in[2*i]+pcm_in[2*i+1])/2.0;
    }
    return;
  }
  syslog(LOG_ERR,"invalid channel remix: chans_in: %d  chans_out: %d",
	 chans_in,chans_out);
  exit(256);
}
