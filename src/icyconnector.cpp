// icyconnector.cpp
//
// Source connector class for IceCast2 servers
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

#include <syslog.h>

#include <QtCore/QStringList>

#include "icyconnector.h"

IcyConnector::IcyConnector(QObject *parent)
  : Connector(parent)
{
  icy_recv_buffer="";

  icy_socket=new QTcpSocket(this);
  connect(icy_socket,SIGNAL(connected()),this,SLOT(socketConnectedData()));
  connect(icy_socket,SIGNAL(disconnected()),
	  this,SLOT(socketDisconnectedData()));
  connect(icy_socket,SIGNAL(readyRead()),this,SLOT(socketReadyReadData()));
  connect(icy_socket,SIGNAL(error(QAbstractSocket::SocketError)),
	  this,SLOT(socketErrorData(QAbstractSocket::SocketError)));
}


IcyConnector::~IcyConnector()
{
  delete icy_socket;
}


IcyConnector::ServerType IcyConnector::serverType() const
{
  return Connector::Shoutcast1Server;
}


void IcyConnector::connectToHostConnector(const QString &hostname,uint16_t port)
{
  icy_socket->connectToHost(hostname,port+1);
}


void IcyConnector::disconnectFromHostConnector()
{
  icy_socket->disconnectFromHost();
}


int64_t IcyConnector::writeDataConnector(const unsigned char *data,int64_t len)
{
  return icy_socket->write((const char *)data,len);
}


void IcyConnector::socketConnectedData()
{
  icy_socket->write(serverPassword().toAscii()+"\r\n",
		    serverPassword().length()+2);
}


void IcyConnector::socketDisconnectedData()
{
  setConnected(false);
}


void IcyConnector::socketReadyReadData()
{
  char data[1501];
  int64_t n;

  while((n=icy_socket->read(data,1500))>0) {
    data[n]=0;
    //printf("recvd %ld bytes: |%s|\n",n,data);
    for(int64_t i=0;i<n;i++) {
      switch(0xFF&data[i]) {
      case 10:
	if((icy_recv_buffer.length()>=2)&&
	   (icy_recv_buffer.toUtf8().at(icy_recv_buffer.length()-3)==13)) {
	  icy_recv_buffer=icy_recv_buffer.left(icy_recv_buffer.length()-1);
	  ProcessHeaders(icy_recv_buffer);
	  icy_recv_buffer="";
	}
	else {
	  icy_recv_buffer+=data[i];
	}
	break;

      case 13:
	if(QString(icy_recv_buffer)=="invalid password") {
	  syslog(LOG_WARNING,"login to \"%s:%d\" rejected: invalid password",
		 (const char *)hostHostname().toUtf8(),0xFFFF&hostPort());
	}
	else {
	  icy_recv_buffer+=data[i];
	}
	break;

      default:
	icy_recv_buffer+=data[i];
	break;
      }
    }
  }
}


void IcyConnector::socketErrorData(QAbstractSocket::SocketError err)
{
  setError(err);
}


void IcyConnector::ProcessHeaders(const QString &hdrs)
{
  QStringList f0;
  QStringList f1;
  QString txt;

  f0=hdrs.split("\r\n");
  if(f0[0]!="OK2") {
    syslog(LOG_WARNING,"login to \"%s:%d\" rejected: %s",
	   (const char *)hostHostname().toUtf8(),0xFFFF&hostPort(),
	   (const char *)f0[0].toUtf8());
    return;
  }
  WriteHeader("icy-name: "+streamName());
  WriteHeader("icy-genre: "+streamGenre());
  WriteHeader("icy-pub: "+QString().sprintf("%d",streamPublic()));
  WriteHeader("icy-br: "+QString().sprintf("%u",audioBitrate()));
  WriteHeader("icy-url: "+streamUrl());
  WriteHeader("icy-irc: "+streamIrc());
  WriteHeader("icy-icq: "+streamIcq());
  WriteHeader("icy-aim: "+streamAim());
  WriteHeader("Content-Type: "+contentType());
  WriteHeader("");

  setConnected(true);
}


void IcyConnector::WriteHeader(const QString &str)
{
  icy_socket->write((str+"\r\n").toUtf8());
}
