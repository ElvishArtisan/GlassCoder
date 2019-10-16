// iceoutconnector.cpp
//
// Glasscoder connector for a single Icecast stream
//
//   (C) Copyright 2014-2016 Fred Gleason <fredg@paravelsystems.com>
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

#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#include <QCoreApplication>
#include <QDateTime>
#include <QHostAddress>
#include <QUrl>

#include "iceoutconnector.h"

IceOutConnector::IceOutConnector(QObject *parent)
  : Connector(parent)
{
}


IceOutConnector::~IceOutConnector()
{
}


Connector::ServerType IceOutConnector::serverType() const
{
  return Connector::IcecastOutServer;
}


void IceOutConnector::startStopping()
{
  emit stopped();
}


void IceOutConnector::connectToHostConnector(const QUrl &url)
{
  StartStream();
  setConnected(true);
  emit unmuteRequested();
}


void IceOutConnector::disconnectFromHostConnector()
{
}


int64_t IceOutConnector::writeDataConnector(int frames,
					    const unsigned char *data,
					    int64_t len)
{
  fwrite(data,len,1,stdout);
  return len;
}


void IceOutConnector::SendHeader(const QString &hdr) const
{
  printf("%s\r\n",(const char *)hdr.toUtf8());
}


void IceOutConnector::StartStream()
{
  //
  // Send Headers
  //
  SendHeader("Content-Type: "+contentType());
  SendHeader("icy-br: "+QString().sprintf("%u",audioBitrate()));
  SendHeader("ice-audio-info: "+
	     QString().sprintf("bitrate=%u",audioBitrate()));
  SendHeader("icy-description: "+streamDescription());
  SendHeader("icy-genre: "+streamGenre());
  SendHeader("icy-name: "+streamName());
  SendHeader("icy-pub: "+QString().sprintf("%u",streamPublic()));
  SendHeader("icy-url: "+streamUrl().toString());
  SendHeader();
}
