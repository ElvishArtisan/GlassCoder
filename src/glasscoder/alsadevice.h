// alsadevice.h
//
// Audio source for the Advance Linux Sound Architecture
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

#ifndef ALSADEVICE_H
#define ALSADEVICE_H

#ifdef ALSA
#include <alsa/asoundlib.h>
#include <pthread.h>
#endif  // ALSA

#include <QTimer>

#include "audiodevice.h"
#include "meteraverage.h"

#define ALSA_DEFAULT_DEVICE "hw:0"
#define ALSA_PERIOD_QUANTITY 4
class AlsaDevice : public AudioDevice
{
  Q_OBJECT;
 public:
  AlsaDevice(unsigned chans,unsigned samprate,
	     Ringbuffer *ring,QObject *parent=0);
  ~AlsaDevice();
  bool processOptions(QString *err,const QStringList &keys,
		      const QStringList &values);
  bool start(QString *err);
  unsigned deviceSamplerate() const;

 private slots:
  void meterData();

 private:
#ifdef ALSA
  QString alsa_device;
  snd_pcm_t *alsa_pcm;
  AudioDevice::Format alsa_format;
  unsigned alsa_samplerate;
  unsigned alsa_channels;
  unsigned alsa_period_quantity;
  snd_pcm_uframes_t alsa_buffer_size; 
  float *alsa_pcm_buffer;
  pthread_t alsa_pthread;
  MeterAverage *alsa_meter_avg[MAX_AUDIO_CHANNELS];
  QTimer *alsa_meter_timer;
  friend void *AlsaCallback(void *ptr);
#endif  // ALSA
};


#endif  // ALSADEVICE_H
