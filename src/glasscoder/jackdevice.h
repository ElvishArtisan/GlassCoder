// jackdevice.h
//
// Audio source for the Jack Audio Connection Kit
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

#ifndef JACKDEVICE_H
#define JACKDEVICE_H

#ifdef JACK
#include <jack/jack.h>
#endif  // JACK

#include <QString>
#include <QTimer>

#include "audiodevice.h"
#include "glasslimits.h"
#include "meteraverage.h"

#define DEFAULT_JACK_CLIENT_NAME "glasscoder"

class JackDevice : public AudioDevice
{
  Q_OBJECT;
 public:
  JackDevice(unsigned chans,unsigned samprate,
	     Ringbuffer *ring,QObject *parent=0);
  ~JackDevice();
  bool processOptions(QString *err,const QStringList &keys,
		      const QStringList &values);
  bool start(QString *err);
  unsigned deviceSamplerate() const;

 private slots:
  void meterData();

 private:
#ifdef JACK
  QString jack_server_name;
  QString jack_client_name;
  float jack_gain;
  jack_client_t *jack_jack_client;
  jack_nframes_t jack_jack_sample_rate;
  jack_port_t *jack_jack_ports[MAX_AUDIO_CHANNELS];
  MeterAverage *jack_meter_avg[MAX_AUDIO_CHANNELS];
  QTimer *jack_meter_timer;
  friend int JackProcess(jack_nframes_t nframes, void *arg);
#endif  // JACK
};


#endif  // JACKDEVICE_H
