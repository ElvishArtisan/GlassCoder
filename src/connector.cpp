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
  icy_server_username="source";
  icy_server_password="";
  icy_server_mountpoint="";
  icy_content_type="";
  icy_audio_channels=2;
  icy_audio_samplerate=44100;
  icy_audio_bitrate=128;
  icy_stream_name="no name";
  icy_stream_description="unknown";
  icy_stream_url="";
  icy_stream_genre="unknown";
  icy_stream_public=true;
}


Connector::~Connector()
{
}


QString Connector::serverUsername() const
{
  return icy_server_username;
}


void Connector::setServerUsername(const QString &str)
{
  icy_server_username=str;
}


QString Connector::serverPassword() const
{
  return icy_server_password;
}


void Connector::setServerPassword(const QString &str)
{
  icy_server_password=str;
}


QString Connector::serverMountpoint() const
{
  return icy_server_mountpoint;
}


void Connector::setServerMountpoint(const QString &str)
{
  icy_server_mountpoint=str;
}


QString Connector::contentType() const
{
  return icy_content_type;
}


void Connector::setContentType(const QString &str)
{
  icy_content_type=str;
}


unsigned Connector::audioChannels() const
{
  return icy_audio_channels;
}


void Connector::setAudioChannels(unsigned chans)
{
  icy_audio_channels=chans;
}


unsigned Connector::audioSamplerate() const
{
  return icy_audio_samplerate;
}


void Connector::setAudioSamplerate(unsigned rate)
{
  icy_audio_samplerate=rate;
}


unsigned Connector::audioBitrate() const
{
  return icy_audio_bitrate;
}


void Connector::setAudioBitrate(unsigned rate)
{
  icy_audio_bitrate=rate;
}


QString Connector::streamName() const
{
  return icy_stream_name;
}


void Connector::setStreamName(const QString &str)
{
  icy_stream_name=str;
}


QString Connector::streamDescription() const
{
  return icy_stream_description;
}


void Connector::setStreamDescription(const QString &str)
{
  icy_stream_description=str;
}


QString Connector::streamUrl() const
{
  return icy_stream_url;
}


void Connector::setStreamUrl(const QString &str)
{
  icy_stream_url=str;
}


QString Connector::streamGenre() const
{
  return icy_stream_genre;
}


void Connector::setStreamGenre(const QString &str)
{
  icy_stream_genre=str;
}


bool Connector::streamPublic() const
{
  return icy_stream_public;
}


void Connector::setStreamPublic(bool state)
{
  icy_stream_public=state;
}


QString Connector::serverTypeText(Connector::ServerType)
{
  QString ret=tr("Unknown");

  return ret;
}
