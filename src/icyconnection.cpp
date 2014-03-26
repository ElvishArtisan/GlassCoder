// icyconnection.cpp
//
// Abstract an ICY source connection.
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

#include "icyconnection.h"

IcyConnection::IcyConnection(QObject *parent)
  : QObject(parent)
{
  icy_server_type=IcyConnection::Icecast2Server;
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


IcyConnection::~IcyConnection()
{
  delete icy_socket;
}


IcyConnection::ServerType IcyConnection::serverType() const
{
  return icy_server_type;
}


void IcyConnection::setServerType(IcyConnection::ServerType type)
{
  icy_server_type=type;
}


QString IcyConnection::serverUsername() const
{
  return icy_server_username;
}


void IcyConnection::setServerUsername(const QString &str)
{
  icy_server_username=str;
}


QString IcyConnection::serverPassword() const
{
  return icy_server_password;
}


void IcyConnection::setServerPassword(const QString &str)
{
  icy_server_password=str;
}


QString IcyConnection::serverMountpoint() const
{
  return icy_server_mountpoint;
}


void IcyConnection::setServerMountpoint(const QString &str)
{
  icy_server_mountpoint=str;
}


QString IcyConnection::contentType() const
{
  return icy_content_type;
}


void IcyConnection::setContentType(const QString &str)
{
  icy_content_type=str;
}


unsigned IcyConnection::audioChannels() const
{
  return icy_audio_channels;
}


void IcyConnection::setAudioChannels(unsigned chans)
{
  icy_audio_channels=chans;
}


unsigned IcyConnection::audioSamplerate() const
{
  return icy_audio_samplerate;
}


void IcyConnection::setAudioSamplerate(unsigned rate)
{
  icy_audio_samplerate=rate;
}


unsigned IcyConnection::audioBitrate() const
{
  return icy_audio_bitrate;
}


void IcyConnection::setAudioBitrate(unsigned rate)
{
  icy_audio_bitrate=rate;
}


QString IcyConnection::streamName() const
{
  return icy_stream_name;
}


void IcyConnection::setStreamName(const QString &str)
{
  icy_stream_name=str;
}


QString IcyConnection::streamDescription() const
{
  return icy_stream_description;
}


void IcyConnection::setStreamDescription(const QString &str)
{
  icy_stream_description=str;
}


QString IcyConnection::streamUrl() const
{
  return icy_stream_url;
}


void IcyConnection::setStreamUrl(const QString &str)
{
  icy_stream_url=str;
}


QString IcyConnection::streamGenre() const
{
  return icy_stream_genre;
}


void IcyConnection::setStreamGenre(const QString &str)
{
  icy_stream_genre=str;
}


bool IcyConnection::streamPublic() const
{
  return icy_stream_public;
}


void IcyConnection::setStreamPublic(bool state)
{
  icy_stream_public=state;
}


void IcyConnection::connectToServer(const QString &hostname,uint16_t port)
{
  icy_hostname=hostname;
  icy_port=port;
  icy_socket->connectToHost(hostname,port);
}


int64_t IcyConnection::writeData(const char *data,int64_t len)
{
  return icy_socket->write(data,len);
}


void IcyConnection::socketConnectedData()
{
  WriteHeader("SOURCE /"+icy_server_mountpoint+" HTTP/1.0");
  WriteHeader(QString("Authorization: Basic ")+
	      "c291cmNlOmt1am8kYXRvbWlj");  // FIXME
  //WriteHeader(QString("Authorization: Basic ")+
  //"c291cmNlOmt1am8kYXRvbWll");  // FIXME
  WriteHeader(QString("User-Agent: GlassCoder/")+VERSION);
  WriteHeader("Content-Type: "+icy_content_type);
  WriteHeader("ice-name: "+icy_stream_name);
  WriteHeader("ice-description: "+icy_stream_description);
  WriteHeader("ice-genre: "+icy_stream_genre);
  WriteHeader("ice-public: "+QString().sprintf("%d",icy_stream_public));
  WriteHeader(QString("ice-audio-info: ")+
	      QString().sprintf("bitrate=%u;",icy_audio_bitrate)+
	      QString().sprintf("channels=%u;",icy_audio_channels)+
	      QString().sprintf("samplerate=%u",icy_audio_samplerate));
  WriteHeader("");
}


void IcyConnection::socketDisconnectedData()
{
  printf("socketDisconnectedData()\n");
}


void IcyConnection::socketReadyReadData()
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


void IcyConnection::socketErrorData(QAbstractSocket::SocketError err)
{
  printf("socketErrorData(): %d\n",err);
}


void IcyConnection::ProcessHeaders(const QString &hdrs)
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


void IcyConnection::WriteHeader(const QString &str)
{
  icy_socket->write((str+"\r\n").toUtf8());
}
