// audiodevice.h
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

#ifndef AUDIODEVICE_H
#define AUDIODEVICE_H

#include <vector>

#include <QObject>
#include <QStringList>

#include "ringbuffer.h"

class AudioDevice : public QObject
{
  Q_OBJECT;
 public:
  enum DeviceType {Jack=0};
  AudioDevice(unsigned chans,unsigned samprate,
	      std::vector<Ringbuffer *> *rings,QObject *parent=0);
  ~AudioDevice();
  virtual bool processOptions(QString *err,const QStringList &keys,
			      const QStringList &values)=0;
  virtual bool start()=0;
  virtual unsigned deviceSamplerate() const;
  static QString deviceTypeText(AudioDevice::DeviceType type);
  static QString optionKeyword(AudioDevice::DeviceType type);

 protected:
  unsigned ringBufferQuantity() const;
  Ringbuffer *ringBuffer(unsigned n);
  unsigned channels() const;
  unsigned samplerate() const;

 private:
  std::vector<Ringbuffer *> *audio_rings;
  unsigned audio_channels;
  unsigned audio_samplerate;
};


#endif  // AUDIODEVICE_H
