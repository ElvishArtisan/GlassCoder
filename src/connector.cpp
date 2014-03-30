// connector.cpp
//
// Abstract base class for streaming server source connections.
//
//   (C) Copyright 2014 Fred Gleason <fredg@paravelsystems.com>
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

#include "connector.h"

Connector::Connector(QObject *parent)
  : QObject(parent)
{
  conn_server_username="source";
  conn_server_password="";
  conn_server_mountpoint="";
  conn_content_type="";
  conn_audio_channels=2;
  conn_audio_samplerate=44100;
  conn_audio_bitrate=128;
  conn_stream_name="no name";
  conn_stream_description="unknown";
  conn_stream_url="";
  conn_stream_genre="unknown";
  conn_stream_public=true;

  conn_data_timer=new QTimer(this);
  connect(conn_data_timer,SIGNAL(timeout()),this,SLOT(dataTimeoutData()));
}


Connector::~Connector()
{
  delete conn_data_timer;
}


QString Connector::serverUsername() const
{
  return conn_server_username;
}


void Connector::setServerUsername(const QString &str)
{
  conn_server_username=str;
}


QString Connector::serverPassword() const
{
  return conn_server_password;
}


void Connector::setServerPassword(const QString &str)
{
  conn_server_password=str;
}


QString Connector::serverMountpoint() const
{
  return conn_server_mountpoint;
}


void Connector::setServerMountpoint(const QString &str)
{
  conn_server_mountpoint=str;
}


QString Connector::contentType() const
{
  return conn_content_type;
}


void Connector::setContentType(const QString &str)
{
  conn_content_type=str;
}


unsigned Connector::audioChannels() const
{
  return conn_audio_channels;
}


void Connector::setAudioChannels(unsigned chans)
{
  conn_audio_channels=chans;
}


unsigned Connector::audioSamplerate() const
{
  return conn_audio_samplerate;
}


void Connector::setAudioSamplerate(unsigned rate)
{
  conn_audio_samplerate=rate;
}


unsigned Connector::audioBitrate() const
{
  return conn_audio_bitrate;
}


void Connector::setAudioBitrate(unsigned rate)
{
  conn_audio_bitrate=rate;
}


QString Connector::streamName() const
{
  return conn_stream_name;
}


void Connector::setStreamName(const QString &str)
{
  conn_stream_name=str;
}


QString Connector::streamDescription() const
{
  return conn_stream_description;
}


void Connector::setStreamDescription(const QString &str)
{
  conn_stream_description=str;
}


QString Connector::streamUrl() const
{
  return conn_stream_url;
}


void Connector::setStreamUrl(const QString &str)
{
  conn_stream_url=str;
}


QString Connector::streamGenre() const
{
  return conn_stream_genre;
}


void Connector::setStreamGenre(const QString &str)
{
  conn_stream_genre=str;
}


bool Connector::streamPublic() const
{
  return conn_stream_public;
}


void Connector::setStreamPublic(bool state)
{
  conn_stream_public=state;
}


QString Connector::serverTypeText(Connector::ServerType type)
{
  QString ret=tr("Unknown");

  switch(type) {
  case Connector::Shoutcast1Server:
    ret=tr("Shoutcast v1");
    break;

  case Connector::Shoutcast2Server:
    ret=tr("Shoutcast v2");
    break;

  case Connector::Icecast2Server:
    ret=tr("Icecast v2");
    break;
  }

  return ret;
}


void Connector::dataTimeoutData()
{
  emit dataRequested(this);
}


void Connector::setConnected(bool state)
{
  if(state) {
    conn_data_timer->start(RINGBUFFER_SERVICE_INTERVAL);
  }
  else {
    conn_data_timer->stop();
  }
}
