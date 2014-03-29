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

#include <QtCore/QStringList>

#include "icyconnector.h"

IcyConnector::IcyConnector(QObject *parent)
  : Connector(parent)
{
  icy_hostname="";
  icy_port=8000;
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
  return Connector::Icecast2Server;
}


void IcyConnector::connectToServer(const QString &hostname,uint16_t port)
{
  icy_hostname=hostname;
  icy_port=port;
  icy_socket->connectToHost(hostname,port);
}


int64_t IcyConnector::writeData(const char *data,int64_t len)
{
  return icy_socket->write(data,len);
}


void IcyConnector::socketConnectedData()
{
  WriteHeader("SOURCE /"+serverMountpoint()+" HTTP/1.0");
  WriteHeader(QString("Authorization: Basic ")+
	      "c291cmNlOmt1am8kYXRvbWlj");  // FIXME
  //WriteHeader(QString("Authorization: Basic ")+
  //"c291cmNlOmt1am8kYXRvbWll");  // FIXME
  WriteHeader(QString("User-Agent: GlassCoder/")+VERSION);
  WriteHeader("Content-Type: "+contentType());
  WriteHeader("ice-name: "+streamName());
  WriteHeader("ice-description: "+streamDescription());
  WriteHeader("ice-genre: "+streamGenre());
  WriteHeader("ice-public: "+QString().sprintf("%d",streamPublic()));
  WriteHeader(QString("ice-audio-info: ")+
	      QString().sprintf("bitrate=%u;",audioBitrate())+
	      QString().sprintf("channels=%u;",audioChannels())+
	      QString().sprintf("samplerate=%u",audioSamplerate()));
  WriteHeader("");
}


void IcyConnector::socketDisconnectedData()
{
  printf("socketDisconnectedData()\n");
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

      default:
	icy_recv_buffer+=data[i];
	break;
      }
    }
  }
}


void IcyConnector::socketErrorData(QAbstractSocket::SocketError err)
{
  printf("socketErrorData(): %d\n",err);
}


void IcyConnector::ProcessHeaders(const QString &hdrs)
{
  QStringList f0;
  QStringList f1;
  QString txt;

  f0=hdrs.split("\r\n");
  f1=f0[0].split(" ");
  if(f1[0].left(7)!="HTTP/1.") {
    emit disconnected();
  }
  for(int i=2;i<f1.size();i++) {
    txt+=f1[i]+" ";
  }
  txt=txt.left(txt.length()-1);
  emit connected(f1[1].toInt(),txt);
}


void IcyConnector::WriteHeader(const QString &str)
{
  icy_socket->write((str+"\r\n").toUtf8());
}
