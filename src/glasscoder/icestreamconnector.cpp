// icestreamconnector.cpp
//
// Glasscoder connector for an integrated IceCast server
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
#include <QUrlQuery>

#include "icestreamconnector.h"

IceStream::IceStream(QTcpSocket *sock,IceStream::Type type)
{
  ice_socket=sock;
  ice_type=type;
  ice_is_negotiated=false;
  ice_is_authenticated=false;
  ice_type=IceStream::New;
  ice_metadata_enabled=false;
  ice_metadata_bytes=0;
  ice_timeout_timer=new QTimer();
  ice_timeout_timer->setSingleShot(true);
  ice_timeout_timer->start(ICESTREAM_CONNECTION_TIMEOUT);
}


IceStream::~IceStream()
{
  delete ice_socket;
  delete ice_timeout_timer;
}


QTcpSocket *IceStream::socket() const
{
  return ice_socket;
}


QTimer *IceStream::timeoutTimer() const
{
  return ice_timeout_timer;
}


IceStream::Type IceStream::type() const
{
  return ice_type;
}


void IceStream::setType(IceStream::Type type)
{
  ice_type=type;
}


bool IceStream::isNegotiated() const
{
  return ice_is_negotiated;
}


void IceStream::setNegotiated()
{
  ice_timeout_timer->stop();
  ice_is_negotiated=true;
}


bool IceStream::isAuthenticated() const
{
  return ice_is_authenticated;
}


void IceStream::setAuthenticated(bool state)
{
  ice_is_authenticated=state;
}


QString IceStream::streamTitle() const
{
  return ice_stream_title;
}


void IceStream::setStreamTitle(const QString &str)
{
  ice_stream_title=str;
}


bool IceStream::metadataEnabled() const
{
  return ice_metadata_enabled;
}


void IceStream::setMetadataEnabled(bool state)
{
  ice_metadata_enabled=state;
}


int IceStream::addMetadataBytes(int bytes)
{
  ice_metadata_bytes+=bytes;
  if((!metadataEnabled())||(ice_metadata_bytes<ICESTREAM_METADATA_INTERVAL)) {
    return -1;
  }
  ice_metadata_bytes-=ICESTREAM_METADATA_INTERVAL;

  return bytes-ice_metadata_bytes;
}




IceStreamConnector::IceStreamConnector(QObject *parent)
  : Connector(parent)
{
  iceserv_metadata=QString().sprintf("%cStreamTitle=''; ",1).toUtf8();
  iceserv_socket_server=NULL;

  iceserv_server=new QTcpServer(this);
  connect(iceserv_server,SIGNAL(newConnection()),
	  this,SLOT(newConnectionData()));

  iceserv_readyread_mapper=new QSignalMapper(this);
  connect(iceserv_readyread_mapper,SIGNAL(mapped(int)),
	  this,SLOT(readyReadData(int)));

  iceserv_timeout_mapper=new QSignalMapper(this);
  connect(iceserv_timeout_mapper,SIGNAL(mapped(int)),
	  this,SLOT(timeoutData(int)));

  iceserv_garbage_timer=new QTimer(this);
  iceserv_garbage_timer->setSingleShot(true);
  connect(iceserv_garbage_timer,SIGNAL(timeout()),this,SLOT(garbageData()));
}


IceStreamConnector::~IceStreamConnector()
{
  if(iceserv_socket_server!=NULL) {
    delete iceserv_socket_server;
  }
  delete iceserv_garbage_timer;
  delete iceserv_readyread_mapper;
  for(unsigned i=0;i<iceserv_streams.size();i++) {
    if(iceserv_streams.at(i)!=NULL) {
      delete iceserv_streams.at(i);
    }
  }
  delete iceserv_server;
}


Connector::ServerType IceStreamConnector::serverType() const
{
  return Connector::IcecastStreamerServer;
}


void IceStreamConnector::setStreamPrologue(const QByteArray &data)
{
  iceserv_stream_prologue=data;
}


void IceStreamConnector::sendMetadata(MetaEvent *e)
{
  SetMetadata(e->field("StreamTitle"));
}


void IceStreamConnector::newConnectionData()
{
  //
  // Accept Connection
  //
  QTcpSocket *sock=iceserv_server->nextPendingConnection();
  if((int)iceserv_streams.size()==serverMaxConnections()) {
    sock->disconnectFromHost();
    return;
  }
  int id=GetFreeStreamId();
  iceserv_streams[id]=new IceStream(sock);
  connect(sock,SIGNAL(disconnected()),this,SLOT(disconnectedData()));
  iceserv_readyread_mapper->setMapping(sock,id);
  connect(sock,SIGNAL(readyRead()),iceserv_readyread_mapper,SLOT(map()));

  iceserv_timeout_mapper->setMapping(iceserv_streams.at(id)->timeoutTimer(),id);
  connect(iceserv_streams.at(id)->timeoutTimer(),SIGNAL(timeout()),
	  iceserv_timeout_mapper,SLOT(map()));
}


void IceStreamConnector::newPipeConnectionData()
{
  //
  // Accept Connection
  //
  QTcpSocket *sock=iceserv_socket_server->nextPendingConnection();
  if((int)iceserv_streams.size()==serverMaxConnections()) {
    sock->disconnectFromHost();
    return;
  }
  int id=GetFreeStreamId();
  iceserv_streams[id]=new IceStream(sock,IceStream::Player);
  connect(sock,SIGNAL(disconnected()),this,SLOT(disconnectedData()));
  StartStream(iceserv_streams[id]);
}


void IceStreamConnector::readyReadData(int id)
{
  IceStream *strm=iceserv_streams.at(id);
  QByteArray data=iceserv_streams.at(id)->socket()->readAll();
  if(!strm->isNegotiated()) {
    for(int i=0;i<data.length();i++) {
      switch(0xFF&data[i]) {
      case 13:
	break;

      case 10:
	ProcessHeader(strm);
	strm->accum="";
	break;

      default:
	strm->accum+=data[i];
	break;
      }
    }
  }
}


void IceStreamConnector::timeoutData(int id)
{
  iceserv_streams.at(id)->socket()->disconnectFromHost();
  iceserv_garbage_timer->start(1);
}


void IceStreamConnector::disconnectedData()
{
  iceserv_garbage_timer->start(1);
}


void IceStreamConnector::garbageData()
{
  for(unsigned i=0;i<iceserv_streams.size();i++) {
    if(iceserv_streams.at(i)!=NULL) {
      if(iceserv_streams.at(i)->socket()->state()!=
	 QAbstractSocket::ConnectedState) {
	delete iceserv_streams.at(i);
	iceserv_streams[i]=NULL;
      }
    }
  }
  if(serverExitOnLast()) {
    for(unsigned i=0;i<iceserv_streams.size();i++) {
      if(iceserv_streams.at(i)!=NULL) {
	return;
      }
    }
    kill(getpid(),SIGTERM);
  }
}


void IceStreamConnector::startStopping()
{
  if(iceserv_socket_server!=NULL) {
    delete iceserv_socket_server;
    iceserv_socket_server=NULL;
  }
  for(unsigned i=0;i<iceserv_streams.size();i++) {
    delete iceserv_streams.at(i);
    iceserv_streams[i]=NULL;
  }
  iceserv_streams.clear();

  emit stopped();
}


void IceStreamConnector::connectToHostConnector(const QUrl &url)
{
  QHostAddress addr;

  if(!serverPipe().isEmpty()) {
    iceserv_socket_server=new SocketServer(this);
    connect(iceserv_socket_server,SIGNAL(newConnection()),
	    this,SLOT(newPipeConnectionData()));
    if(!iceserv_socket_server->listen(serverPipe())) {
      fprintf(stderr,"glasscoder: unable to create server pipe [%s]\n",
	      strerror(errno));
      exit(256);
    }
  }
  if(!url.host().isEmpty()) {
    if(!addr.setAddress(url.host())) {
      fprintf(stderr,"glasscoder: invalid interface address in URL\n");
      exit(256);
    }
    if(!iceserv_server->listen(addr,url.port())) {
      fprintf(stderr,"glasscoder: unable to bind TCP port %u\n",
	      0xFFFF&url.port());
      exit(256);
    }
  }
  setConnected(true);
  if(serverStartConnections()==0) {
    emit unmuteRequested();
  }
}


void IceStreamConnector::disconnectFromHostConnector()
{
  for(unsigned i=0;i<iceserv_streams.size();i++) {
    if(iceserv_streams.at(i)!=NULL) {
      delete iceserv_streams.at(i);
    }
  }
  iceserv_streams.clear();
}


int64_t IceStreamConnector::writeDataConnector(int frames,
					       const unsigned char *data,
					       int64_t len)
{
  IceStream *strm=NULL;
  int offset=0;

  for(unsigned i=0;i<iceserv_streams.size();i++) {
    strm=iceserv_streams.at(i);
    if((strm!=NULL)&&(strm->isNegotiated())) {
      if((offset=strm->addMetadataBytes(len))<0) {
	strm->socket()->write((const char *)data,len);
      }
      else {
	strm->socket()->write((const char *)data,offset);
	strm->socket()->write(iceserv_metadata);
	strm->socket()->write((const char *)data+offset,len-offset);
      }
    }
  }
  return len;
}


void IceStreamConnector::SetMetadata(const QString &title)
{
  //
  // This has to be the lamest metadata protocol in existence.
  // Codecs have ancillary channels for this sort of thing!  ** BAD LLAMA!! **
  //
  iceserv_metadata=("StreamTitle='"+title+"';").toUtf8();
  while((iceserv_metadata.length()%16)!=0) {
    iceserv_metadata.append((char)0);
  }
  iceserv_metadata.prepend((char)(iceserv_metadata.length()/16));
}


void IceStreamConnector::SendHeader(IceStream *strm,const QString &hdr) const
{
  strm->socket()->write((hdr+"\r\n").toUtf8());
}


void IceStreamConnector::ProcessHeader(IceStream *strm)
{
  QStringList f0;
  QStringList f1;
  QStringList hdrs;
  bool ok=false;

  if(strm->type()==IceStream::New) {
    f0=strm->accum.split(" ",QString::SkipEmptyParts);
    if(f0.size()==3) {
      if(f0.at(0)=="GET") {
	f1=f0.at(2).split("/");
	if((f1.size()==2)&&(f1.at(0)=="HTTP")) {
	  QUrl url(f0.at(1));
	  if(url.path()==serverMountpoint()) {
	    strm->setType(IceStream::Player);
	    ok=true;
	  }
	  if(url.path()=="/admin/metadata") {
	    QUrlQuery query(url.query());
	    if(("/"+query.queryItemValue("mount")==serverMountpoint())&&
	       (query.queryItemValue("mode")=="updinfo")) {
	      strm->setType(IceStream::Updinfo);
	      strm->setStreamTitle(query.queryItemValue("song"));
	      //	      printf("SONG: %s\n",(const char *)url.queryItemValue("song").toUtf8());
	      ok=true;
	    }
	  }
	}
      }
    }
    if(!ok) {
      CloseConnection(strm,400,"Malformed request");
    }
  }
  else {
    if(strm->accum.isEmpty()) {
      switch(strm->type()) {
      case IceStream::Player:
	StartStream(strm);
	break;

      case IceStream::Updinfo:
	if(strm->isAuthenticated()) {
	  SetMetadata(strm->streamTitle());
	  CloseConnection(strm,200,"OK");
	}
	else {
	  hdrs.push_back("WWW-Authenticate: Basic realm="+
			 serverMountpoint().right(serverMountpoint().length()-1));
	  CloseConnection(strm,401,"Unauthorized",hdrs);
	}
	break;
	
      default:
	CloseConnection(strm,404,"Not Found");
      }
    }
    else {
      f0=strm->accum.split(":",QString::SkipEmptyParts);
      if((f0.size()==2)&&(f0.at(0).trimmed()=="icy-metadata")) {
	bool state=f0.at(1).trimmed().toUInt(&ok);
	if(ok&&((contentType()=="audio/mpeg")||(contentType()=="audio/aacp"))) {
	  strm->setMetadataEnabled(state);
	}
      }
      if((f0.size()==2)&&(f0.at(0).trimmed()=="Authorization")) {
	f1=f0.at(1).trimmed().split(" ");
	if((f1.size()==2)&&(f1.at(0).toLower()=="basic")) {
	  strm->setAuthenticated(f1.at(1)==serverBasicAuthString());
	}
      }
    }
  }
}


void IceStreamConnector::CloseConnection(IceStream *strm,int code,
					 const QString &str,
					 const QStringList &hdrs)
{
  QString msg=QString().sprintf("%d ",code)+str+"\r\n";
  SendHeader(strm,QString().sprintf("HTTP/1.0 %d ",code)+str);
  SendHeader(strm,"Server: GlassCoder "+QString(VERSION));
  SendHeader(strm,"Date: "+
	     QDateTime::currentDateTime().addSecs(4*3600).
	     toString("ddd, dd MM yyyy hh:mm:ss GMT").toUtf8());
  SendHeader(strm,
	     QString().sprintf("Content-Length: %d",msg.toUtf8().length()));
  for(int i=0;i<hdrs.size();i++) {
    SendHeader(strm,hdrs.at(i));
  }
  SendHeader(strm);
  strm->socket()->write(msg.toUtf8());
  strm->socket()->disconnectFromHost();
  iceserv_garbage_timer->start(1);
}


void IceStreamConnector::StartStream(IceStream *strm)
{
  //
  // Send Headers
  //
  SendHeader(strm,"HTTP/1.0 200 OK");
  SendHeader(strm,"Server: Icecast 2.4.0");
  SendHeader(strm,"Date: "+
	     QDateTime::currentDateTime().toUTC().
	     toString("ddd, dd MM yyyy hh:mm:ss GMT").toUtf8());
  SendHeader(strm,"Content-Type: "+contentType());
  SendHeader(strm,"Cache-Control: no-cache");
  SendHeader(strm,"Pragma: no-cache");
  SendHeader(strm,"icy-br: "+QString().sprintf("%u",audioBitrate()));
  SendHeader(strm,"ice-audio-info: "+
	     QString().sprintf("bitrate=%u",audioBitrate())+
	     QString().sprintf(";channels=%u",audioChannels())+
	     QString().sprintf(";samplerate=%u",audioSamplerate()));
  SendHeader(strm,"icy-description: "+streamDescription());
  SendHeader(strm,"icy-genre: "+streamGenre());
  SendHeader(strm,"icy-name: "+streamName());
  SendHeader(strm,"icy-pub: "+QString().sprintf("%u",streamPublic()));
  SendHeader(strm,"icy-url: "+streamUrl().toString());
  if(strm->metadataEnabled()) {
    SendHeader(strm,"icy-metaint: "+
	       QString().sprintf("%u",ICESTREAM_METADATA_INTERVAL));
  }
  SendHeader(strm);

  strm->setNegotiated();
  strm->socket()->write(iceserv_stream_prologue);
  if((int)iceserv_streams.size()==serverStartConnections()) {
    emit unmuteRequested();
  }
}


int IceStreamConnector::GetFreeStreamId()
{
  for(unsigned i=0;i<iceserv_streams.size();i++) {
    if(iceserv_streams.at(i)==NULL) {
      return i;
    }
  }
  iceserv_streams.push_back(NULL);
  return iceserv_streams.size()-1;
}
