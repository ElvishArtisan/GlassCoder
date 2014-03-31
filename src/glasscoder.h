// glasscoder.h
//
// glasscoder(1) Audio Encoder
//
//   (C) Copyright 2014 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: glasscoder.h,v 1.5 2014/02/24 21:02:45 cvs Exp $
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

#include <QtCore/QObject>
#include <QtCore/QTimer>

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
#define GLASSCODER_USAGE "[options]\n\n--audio-bitrate=<kbps>\n     The stream data rate in kilobits per second.  Default value is 128.\n\n--audio-channels=<chans>\n     The number of audio channels to use.  Valid values are 1 or 2.  Default\n     value is 2.\n\n--audio-format=<fmt>\n     The audio encoding format to use.  Value values are:\n          MP3 - MPEG-1 Layer 3\n     Default value is MP3.\n\n--audio-samplerate=<rate>\n     The audio sample rate to use for streaming.  Default value is 44100.\n\n--jack-server-name=<name>\n     The name of the JACK server instance to use.\n\n--jack-client-name=<name>\n     The client name to use.  Default value is 'glasscoder'.\n\n--server-hostname=<hostname>\n     The host name or IP address of the streaming server.  This parameter\n     has no default.\n\n--server-mountpoint=<mount>\n     The mountpoint to use on the server.  Default value is to use no\n     mountpoint.\n\n--server-password=<passwd>\n     The password to use when authenticating to the server.  This parameter\n     has no default.\n\n--server-port=<port>\n     The port number to connect to the server.  Default value is 8000.\n\n--server-type=<type>\n     Set the type of server to use.  Options are:\n          IceCast1 - IceCast v1\n          IceCast2 - IceCast v2\n          Shout - ShoutCast\n     Default value is 'IceCast2'.\n\n--server-username=<name>\n     The user name to use when authenticating to the server.  Default\n     value is 'source'.\n\n--stream-description=<str>\n     The description of the stream.\n\n--stream-genre=<str>\n     The genre of the stream.\n\n--stream-name=<str>\n     The name of the stream.\n\n--stream-url=<url>\n     The URL of the stream.\n"

class MainObject : public QObject
{
 Q_OBJECT;
 public:
  MainObject(QObject *parent=0);

 private slots:
  void icyConnectedData(int result,const QString &txt);
  void icyDisconnectedData();
  void encodeData();

 private:
  //
  // Arguments
  //
  unsigned audio_bitrate;
  unsigned audio_channels;
  unsigned audio_samplerate;
  QString stream_description;
  QString stream_genre;
  QString stream_name;
  QString stream_url;
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
  Ringbuffer *sir_ringbuffer;
  friend int JackProcess(jack_nframes_t nframes, void *arg);

  //
  // Server Connection
  //
  void StartServerConnection();
  Connector *sir_connector;

  //
  // Codec
  //
  bool StartCodec();
  Codec *sir_codec;
};


#endif  // GLASSCODER_H
