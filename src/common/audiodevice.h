// audiodevice.h
//
// Abstract base class for audio input sources.
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

#ifndef AUDIODEVICE_H
#define AUDIODEVICE_H

#include <vector>

#include <QObject>
#include <QStringList>

#include "glasslimits.h"
#include "ringbuffer.h"

#define AUDIO_METER_INTERVAL 50

class AudioDevice : public QObject
{
  Q_OBJECT;
 public:
  enum DeviceType {Alsa=0,AsiHpi=1,File=2,Jack=3,LastType=4};
  enum Format {FLOAT=0,S16_LE=1,S32_LE=2,LastFormat=3};
  AudioDevice(unsigned chans,unsigned samprate,
	      Ringbuffer *ring,QObject *parent=0);
  ~AudioDevice();
  virtual bool isAvailable() const;
  virtual bool processOptions(QString *err,const QStringList &keys,
			      const QStringList &values)=0;
  virtual bool start(QString *err)=0;
  virtual unsigned deviceSamplerate() const;
  void meterLevels(int *lvls) const;
  static QString deviceTypeText(AudioDevice::DeviceType type);
  static QString optionKeyword(AudioDevice::DeviceType type);
  static AudioDevice::DeviceType deviceType(const QString &key);
  static QString formatString(AudioDevice::Format fmt);

 signals:
  void hasStopped();

 public slots:
  void unmute();

 protected:
  void setMeterLevels(float *lvls);
  void setMeterLevels(int *lvls);
  void updateMeterLevels(int *lvls);
  Ringbuffer *ringBuffer();
  unsigned channels() const;
  unsigned samplerate() const;
  void remixChannels(float *pcm_out,unsigned chans_out,
		     float *pcm_in,unsigned chans_in,unsigned nframes); 
  void convertToFloat(float *pcm_out,const void *pcm_in,Format fmt_in,
		      unsigned nframes,unsigned chans);
  void peakLevels(float *lvls,const float *pcm,unsigned nframes,unsigned chans);
  void peakLevels(int *lvls,const float *pcm,unsigned nframes,unsigned chans);

 private:
  Ringbuffer *audio_ring;
  unsigned audio_channels;
  unsigned audio_samplerate;
  int audio_meter_levels[MAX_AUDIO_CHANNELS];
};


#endif  // AUDIODEVICE_H
