// icyconnector.cpp
//
// Source connector class for Shoutcast servers
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

#include "icyconnector.h"
#include "logging.h"

IcyConnector::IcyConnector(int version,QObject *parent)
  : Connector(parent)
{
  icy_protocol_version=version;
  icy_recv_buffer="";
  icy_authenticated=false;

  icy_socket=new QTcpSocket(this);
  connect(icy_socket,SIGNAL(connected()),this,SLOT(socketConnectedData()));
  connect(icy_socket,SIGNAL(disconnected()),
	  this,SLOT(socketDisconnectedData()));
  connect(icy_socket,SIGNAL(readyRead()),this,SLOT(socketReadyReadData()));
  connect(icy_socket,SIGNAL(error(QAbstractSocket::SocketError)),
	  this,SLOT(socketErrorData(QAbstractSocket::SocketError)));

  //
  // Metadata File Conveyor
  //
  icy_conveyor=new GetConveyor(this);
  QStringList hdrs;
  //
  // D.N.A.S v1.9.8 refuses to process updates with the default CURL
  // user-agent value, hence we lie to it.
  //
  hdrs.push_back("User-agent: Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; rv:1.8.1.2) Gecko/20070219 Firefox/2.0.0.2");
  icy_conveyor->setAddedHeaders(hdrs);
  connect(icy_conveyor,SIGNAL(eventFinished(const QUrl &,int,int,
					    const QStringList &)),
	  this,SLOT(conveyorEventFinished(const QUrl &,int,int,
					  const QStringList &)));
  connect(icy_conveyor,
	  SIGNAL(error(const QUrl &,QProcess::ProcessError,
		       const QStringList &)),
	  this,
	  SLOT(conveyorError(const QUrl &,QProcess::ProcessError,
			     const QStringList &)));
}


IcyConnector::~IcyConnector()
{
  delete icy_socket;
}


IcyConnector::ServerType IcyConnector::serverType() const
{
  return Connector::Shoutcast1Server;
}


void IcyConnector::sendMetadata(MetaEvent *e)
{
  if(e->fieldKeys().contains("StreamTitle")||
     e->fieldKeys().contains("StreamUrl")) {
    QString url=QString("http://")+
      serverUrl().host()+
      QString().sprintf(":%u",serverUrl().port())+
      "/admin.cgi?"+
      "pass="+Connector::urlEncode(serverPassword())+"&"+
      "mode=updinfo";
    if(e->fieldKeys().contains("StreamTitle")) {
      url+="&song="+Connector::urlEncode(e->field("StreamTitle"));
    }
    if(e->fieldKeys().contains("StreamUrl")) {
      url+="&url="+Connector::urlEncode(e->field("StreamUrl"));
    }
    icy_conveyor->push(url);
  }
}


void IcyConnector::connectToHostConnector(const QUrl &url)
{
  icy_socket->connectToHost(url.host(),url.port()+1);
  emit unmuteRequested();
}


void IcyConnector::disconnectFromHostConnector()
{
  icy_socket->disconnectFromHost();
}


int64_t IcyConnector::writeDataConnector(int frames,const unsigned char *data,
					 int64_t len)
{
  if(icy_socket->state()==QAbstractSocket::ConnectedState) {
    return icy_socket->write((const char *)data,len);
  }
  return len;
}


void IcyConnector::socketConnectedData()
{
  QString auth=serverPassword()+"\r\n";
  if(!serverUsername().isEmpty()) {
    auth=serverUsername()+":"+serverPassword()+"\r\n";
  }
  icy_socket->write(auth.toUtf8());
}


void IcyConnector::socketDisconnectedData()
{
  if(!icy_authenticated) {
    Log(LOG_WARNING,
	QString().sprintf("login to \"%s:%d\" rejected: bad password",
			  (const char *)serverUrl().host().toUtf8(),
			  0xFFFF&serverUrl().port()));
  }
  icy_authenticated=false;
  setConnected(false);
}


void IcyConnector::socketReadyReadData()
{
  char data[1501];
  int64_t n;

  while((n=icy_socket->read(data,1500))>0) {
    data[n]=0;
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
	  Log(LOG_WARNING,
	      QString().sprintf("login to \"%s:%d\" rejected: invalid password",
				(const char *)serverUrl().host().toUtf8(),
				0xFFFF&serverUrl().port()));
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


void IcyConnector::conveyorEventFinished(const QUrl &url,int exit_code,
					 int resp_code,const QStringList &args)
{
  //
  // Exit code handler
  //
  if((exit_code!=0)&&(exit_code!=CURLE_GOT_NOTHING)) {
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
    // Response code handler
    //
    if(((resp_code<200)||(resp_code>299))&&(resp_code!=0)) {
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


void IcyConnector::conveyorError(const QUrl &url,
				 QProcess::ProcessError err,
				 const QStringList &args)
{
  Log(LOG_ERR,
      QString().sprintf("curl(1) process error: %d, cmd: \"curl %s\"",err,
			(const char *)args.join(" ").toUtf8()));
  setConnected(false);
  exit(256);
}


void IcyConnector::ProcessHeaders(const QString &hdrs)
{
  QStringList f0;
  QStringList f1;
  QString txt;

  f0=hdrs.split("\r\n");
  if((!icy_authenticated)&&(f0[0]!="OK2")) {
    Log(LOG_WARNING,
	QString().sprintf("login to \"%s:%d\" rejected: %s",
			  (const char *)serverUrl().host().toUtf8(),
			  0xFFFF&serverUrl().port(),
			  (const char *)f0[0].toUtf8()));
    return;
  }
  icy_authenticated=true;
  WriteHeader("icy-name: "+streamName());
  WriteHeader("icy-genre: "+streamGenre());
  WriteHeader("icy-pub: "+QString().sprintf("%d",streamPublic()));
  WriteHeader("icy-br: "+QString().sprintf("%u",audioBitrate()));
  if(icy_protocol_version==1) {
    WriteHeader("icy-url: "+streamUrl().toString());
  }
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
