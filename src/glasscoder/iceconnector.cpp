// iceconnector.cpp
//
// Source connector class for IceCast2 servers
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

#include <QStringList>

#include "iceconnector.h"
#include "logging.h"

IceConnector::IceConnector(QObject *parent)
  : Connector(parent)
{
  ice_recv_buffer="";

  ice_socket=NULL;

  //
  // Metadata File Conveyor
  //
  ice_conveyor=new GetConveyor(this);
  connect(ice_conveyor,SIGNAL(eventFinished(const QUrl &,int,int,
					    const QStringList &)),
	  this,SLOT(conveyorEventFinished(const QUrl &,int,int,
					  const QStringList &)));
  connect(ice_conveyor,
	  SIGNAL(error(const QUrl &,QProcess::ProcessError,
		       const QStringList &)),
	  this,
	  SLOT(conveyorError(const QUrl &,QProcess::ProcessError,
			     const QStringList &)));
}


IceConnector::~IceConnector()
{
  delete ice_socket;
}


IceConnector::ServerType IceConnector::serverType() const
{
  return Connector::Icecast2Server;
}


void IceConnector::sendMetadata(MetaEvent *e)
{
  if(e->fieldKeys().contains("StreamTitle")) {
    QString url=QString("http://")+
      serverUrl().host()+
      QString().sprintf(":%u",serverUrl().port())+
      "/admin/metadata?"+
      "mount="+serverMountpoint()+"&"+
      "mode=updinfo&"+
      "song="+Connector::urlEncode(e->field("StreamTitle"));
    ice_conveyor->push(url);
  }
}


void IceConnector::connectToHostConnector(const QUrl &url)
{
  //
  // Create Socket
  //
  if(ice_socket!=NULL) {
    ice_socket->
      disconnect(SIGNAL(connected()),this,SLOT(socketConnectedData()));
    ice_socket->
      disconnect(SIGNAL(disconnected()),this,SLOT(socketDisconnectedData()));
    ice_socket->
      disconnect(SIGNAL(readyRead()),this,SLOT(socketReadyReadData()));
    ice_socket->
      disconnect(SIGNAL(error(QAbstractSocket::SocketError)),
		 this,SLOT(socketErrorData(QAbstractSocket::SocketError)));
    ice_socket->deleteLater();
  }
  ice_socket=new QTcpSocket(this);
  connect(ice_socket,SIGNAL(connected()),this,SLOT(socketConnectedData()));
  connect(ice_socket,SIGNAL(disconnected()),
	  this,SLOT(socketDisconnectedData()));
  connect(ice_socket,SIGNAL(readyRead()),this,SLOT(socketReadyReadData()));
  connect(ice_socket,SIGNAL(error(QAbstractSocket::SocketError)),
	  this,SLOT(socketErrorData(QAbstractSocket::SocketError)));

  //
  // Initiate the Connection
  //
  ice_conveyor->setUsername(serverUsername());
  ice_conveyor->setPassword(serverPassword());
  ice_socket->connectToHost(url.host(),url.port());
  emit unmuteRequested();
}


void IceConnector::disconnectFromHostConnector()
{
  ice_socket->disconnectFromHost();
}


int64_t IceConnector::writeDataConnector(int frames,const unsigned char *data,
					 int64_t len)
{
  if(ice_socket->state()==QAbstractSocket::ConnectedState) {
    return ice_socket->write((const char *)data,len);
  }
  return len;
}


void IceConnector::socketConnectedData()
{
  QString username=serverUsername();
  if(username.isEmpty()) {
    username="source";
  }
  WriteHeader("SOURCE "+serverMountpoint()+" HTTP/1.0");
  WriteHeader(QString("Authorization: Basic ")+
	      Connector::base64Encode(username+":"+serverPassword()));
  WriteHeader("User-Agent: "+serverUserAgent());
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


void IceConnector::socketDisconnectedData()
{
  setConnected(false);
}


void IceConnector::socketReadyReadData()
{
  char data[1501];
  int64_t n;

  while((n=ice_socket->read(data,1500))>0) {
    data[n]=0;
    //printf("recvd %ld bytes: |%s|\n",n,data);
    for(int64_t i=0;i<n;i++) {
      switch(0xFF&data[i]) {
      case 10:
	if((ice_recv_buffer.length()>=2)&&
	   (ice_recv_buffer.toUtf8().at(ice_recv_buffer.length()-3)==13)) {
	  ice_recv_buffer=ice_recv_buffer.left(ice_recv_buffer.length()-1);
	  ProcessHeaders(ice_recv_buffer);
	  ice_recv_buffer="";
	}
	else {
	  ice_recv_buffer+=data[i];
	}
	break;

      default:
	ice_recv_buffer+=data[i];
	break;
      }
    }
  }
}


void IceConnector::socketErrorData(QAbstractSocket::SocketError err)
{
  setError(err);
}


void IceConnector::conveyorEventFinished(const QUrl &url,int exit_code,
					 int resp_code,const QStringList &args)
{
  //
  // Exit code handler
  //
  if(exit_code!=0) {
    setConnected(false);
    if(global_log_verbose) {
      Log(LOG_WARNING,
	  QString().sprintf("curl(1) error: %s, cmd: \"curl %s\"",
			    (const char *)Connector::curlStrError(exit_code).toUtf8(),
			    (const char *)args.join(" ").toUtf8()));
    }
    else {
      Log(LOG_WARNING,QString().sprintf("CURL error: %s",
					(const char *)Connector::curlStrError(exit_code).toUtf8()));
    }
  }
  else {
    //
    // Reponse code handler
    //
    if((resp_code<200)||(resp_code>299)) {
      setConnected(false);
      if(global_log_verbose) {
	Log(LOG_WARNING,"curl(1) response error: "+
	    Connector::httpStrError(resp_code)+
	    ", cmd: \"curl "+args.join(" ")+"\"");
      }
      else {
	Log(LOG_WARNING,"curl(1) response error: "+
	    Connector::httpStrError(resp_code));
      }
    }
    else {
      setConnected(true);
    }
  }
}


void IceConnector::conveyorError(const QUrl &url,
				 QProcess::ProcessError err,
				 const QStringList &args)
{
  Log(LOG_ERR,
      QString().sprintf("curl(1) process error: %d, cmd: \"curl %s\"",err,
			(const char *)args.join(" ").toUtf8()));
  setConnected(false);
  exit(256);
}


void IceConnector::ProcessHeaders(const QString &hdrs)
{
  QStringList f0;
  QStringList f1;
  QString txt;

  f0=hdrs.split("\r\n");
  for(int i=0;i<f0.size();i++) {
    if(dumpHeaders()) {
      fprintf(stderr,"==> %s\n",(const char *)f0.at(i).toUtf8());
    }
    f1=f0[i].split(" ");
    if(f1[0].left(7)=="HTTP/1.") {
      for(int i=2;i<f1.size();i++) {
	txt+=f1[i]+" ";
      }
      txt=txt.left(txt.length()-1);
      if(f1[1].toInt()==200) {
	setConnected(true);
      }
      else {
	Log(LOG_ERR,
	    QString().sprintf("server \"%s:%u/%s\" returned \"%d %s\"",
			      (const char *)serverUrl().host().toUtf8(),
			      0xFFFF&serverUrl().port(),
			      (const char *)serverMountpoint().toUtf8(),
			      f1[1].toInt(),(const char *)txt.toUtf8()));
	    setError(QAbstractSocket::UnknownSocketError);
      }
      return;
    }
  }
  Log(LOG_ERR,
      QString().sprintf("server \"%s:%u/%s\" returned unrecognized response",
			(const char *)serverUrl().host().toUtf8(),
			0xFFFF&serverUrl().port(),
			(const char *)serverMountpoint().toUtf8()));
  setError(QAbstractSocket::UnknownSocketError);
}


void IceConnector::WriteHeader(const QString &str)
{
  if(dumpHeaders()) {
    fprintf(stderr,"<== %s\n",(const char *)str.toUtf8());
  }
  ice_socket->write((str+"\r\n").toUtf8());
}
