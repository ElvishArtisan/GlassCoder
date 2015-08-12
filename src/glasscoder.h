// glasscoder.h
//
// glasscoder(1) Audio Encoder
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

#ifndef GLASSCODER_H
#define GLASSCODER_H

#include <stdint.h>

#include <vector>

#include <QObject>
#include <QTimer>

#include <jack/jack.h>

#include "codec.h"
#include "connector.h"
#include "ringbuffer.h"

#define DEFAULT_JACK_CLIENT_NAME "glasscoder"
#define DEFAULT_SERVER_PORT 8000
#define DEFAULT_SERVER_USERNAME "source"
#define DEFAULT_AUDIO_BITRATE 128
#define DEFAULT_AUDIO_SAMPLERATE 44100
#define RINGBUFFER_SIZE 262144
#define GLASSCODER_USAGE "[options]\n"

class MainObject : public QObject
{
 Q_OBJECT;
 public:
  MainObject(QObject *parent=0);

 private slots:
  void connectorStoppedData();
  void exitTimerData();

 private:
  //
  // Arguments
  //
  std::vector<unsigned> audio_bitrate;
  unsigned audio_channels;
  Codec::Type audio_format;
  double audio_quality;
  unsigned audio_samplerate;
  QString stream_description;
  QString stream_genre;
  QString stream_name;
  QString stream_url;
  QString stream_irc;
  QString stream_icq;
  QString stream_aim;
  Connector::ServerType server_type;
  QString server_hostname;
  QString server_mountpoint;
  QString server_password;
  uint16_t server_port;
  QString server_username;

  //
  // Jack
  //
  bool StartJack();
  QString jack_server_name;
  QString jack_client_name;
  jack_client_t *sir_jack_client;
  jack_nframes_t sir_jack_sample_rate;
  jack_port_t *sir_jack_ports[MAX_AUDIO_CHANNELS];
  std::vector<Ringbuffer *> sir_ringbuffers;
  friend int JackProcess(jack_nframes_t nframes, void *arg);

  //
  // Server Connection
  //
  void StartServerConnection(const QString &mntpt="",bool is_top=false);
  std::vector<Connector *> sir_connectors;

  //
  // Codec
  //
  bool StartCodec();
  std::vector<Codec *> sir_codecs;

  //
  // Miscelaneous
  //
  bool StartSingleStream();
  bool StartMultiStream();
  QTimer *sir_exit_timer;
  unsigned sir_exit_count;
};


#endif  // GLASSCODER_H
