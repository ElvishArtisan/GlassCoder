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

#include <QDateTime>
#include <QHostAddress>

#include "icestreamconnector.h"

IceStream::IceStream(QTcpSocket *sock)
{
  ice_socket=sock;
  ice_is_negotiated=false;
  ice_metadata_enabled=false;
  ice_metadata_bytes=0;
}


IceStream::~IceStream()
{
  delete ice_socket;
}


QTcpSocket *IceStream::socket() const
{
  return ice_socket;
}


bool IceStream::isNegotiated() const
{
  return ice_is_negotiated;
}


void IceStream::setNegotiated()
{
  ice_is_negotiated=true;
}


QString IceStream::mountpoint() const
{
  return ice_mountpoint;
}


void IceStream::setMountpoint(const QString &str)
{
  ice_mountpoint=str;
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

  iceserv_server=new QTcpServer(this);
  connect(iceserv_server,SIGNAL(newConnection()),
	  this,SLOT(newConnectionData()));

  iceserv_readyread_mapper=new QSignalMapper(this);
  connect(iceserv_readyread_mapper,SIGNAL(mapped(int)),
	  this,SLOT(readyReadData(int)));

  iceserv_garbage_timer=new QTimer(this);
  iceserv_garbage_timer->setSingleShot(true);
  connect(iceserv_garbage_timer,SIGNAL(timeout()),this,SLOT(garbageData()));
}


IceStreamConnector::~IceStreamConnector()
{
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


void IceStreamConnector::sendMetadata(MetaEvent *e)
{
  //
  // This has to be the most retarded metadata protocol in existence.
  // Codecs have ancillary channels for this sort of thing!  BAD LLAMA!!
  //
  iceserv_metadata=("StreamTitle='"+
		    e->field(MetaEvent::StreamTitle).toString()+"';").toUtf8();
  while((iceserv_metadata.length()%16)!=0) {
    iceserv_metadata.append((char)0);
  }
  iceserv_metadata.prepend((char)(iceserv_metadata.length()/16));
  //  printf("Metadata: %s\n",(const char *)e->field(MetaEvent::StreamTitle).toString().toUtf8());
}


void IceStreamConnector::newConnectionData()
{
  //
  // Accept Connection
  //
  int id=-1;
  QTcpSocket *sock=iceserv_server->nextPendingConnection();
  connect(sock,SIGNAL(disconnected()),this,SLOT(disconnectedData()));
  for(unsigned i=0;i<iceserv_streams.size();i++) {
    if(iceserv_streams.at(i)==NULL) {
      iceserv_streams[i]=new IceStream(sock);
      id=i;
      break;
    }
  }
  if(id<0) {
    iceserv_streams.push_back(new IceStream(sock));
    id=iceserv_streams.size()-1;
  }
  iceserv_readyread_mapper->setMapping(sock,id);
  connect(sock,SIGNAL(readyRead()),iceserv_readyread_mapper,SLOT(map()));
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
}


void IceStreamConnector::connectToHostConnector(const QString &hostname,
						uint16_t port)
{
  QHostAddress addr;
  if(!addr.setAddress(hostname)) {
    setConnected(false);
    return;
  }
  setConnected(iceserv_server->listen(addr,port));
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


void IceStreamConnector::SendHeader(IceStream *strm,const QString &hdr) const
{
  strm->socket()->write((hdr+"\r\n").toUtf8());
}


void IceStreamConnector::ProcessHeader(IceStream *strm)
{
  QStringList f0;
  QStringList f1;
  bool ok=false;

  if(strm->mountpoint().isEmpty()) {
    f0=strm->accum.split(" ",QString::SkipEmptyParts);
    if(f0.size()==3) {
      if(f0.at(0)=="GET") {
	f1=f0.at(2).split("/");
	if((f1.size()==2)&&(f1.at(0)=="HTTP")) {
	  strm->setMountpoint(f0.at(1));
	  ok=true;
	}
      }
    }
    if(!ok) {
      DenyConnection(strm,400,"Malformed request");
    }
  }
  else {
    if(strm->accum.isEmpty()) {
      if(strm->mountpoint()==serverMountpoint()) {
	//
	// Send Headers
	//
	SendHeader(strm,"HTTP/1.0 200 OK");
	SendHeader(strm,"Server: Icecast 2.4.0");
	SendHeader(strm,"Date: "+
		   QDateTime::currentDateTime().addSecs(4*3600).
		   toString("ddd, dd MM yyyy hh:mm:ss GMT").toUtf8());
	SendHeader(strm,"Content-Type: "+contentType());
	SendHeader(strm,"Cache-Control: no-cache");
	SendHeader(strm,"Pragma: no-cache");
	SendHeader(strm,"icy-br: "+QString().sprintf("%u",audioBitrate()));
	SendHeader(strm,"ice-audio-info: "+
		   QString().sprintf("bitrate=%u",audioBitrate()));
	SendHeader(strm,"icy-description: "+streamDescription());
	SendHeader(strm,"icy-genre: "+streamGenre());
	SendHeader(strm,"icy-name: "+streamName());
	SendHeader(strm,"icy-pub: "+QString().sprintf("%u",streamPublic()));
	SendHeader(strm,"icy-url: "+streamUrl());
	if(strm->metadataEnabled()) {
	  SendHeader(strm,"icy-metaint: "+
		     QString().sprintf("%u",ICESTREAM_METADATA_INTERVAL));
	}
	SendHeader(strm);

	strm->setNegotiated();
      }
      else {
	DenyConnection(strm,404,"No such stream");
      }
    }
    else {
      f0=strm->accum.split(":",QString::SkipEmptyParts);
      if((f0.size()==2)&&(f0.at(0).trimmed()=="icy-metadata")) {
	bool state=f0.at(1).trimmed().toUInt(&ok);
	if(ok) {
	  strm->setMetadataEnabled(state);
	}
      }
    }
  }
}


void IceStreamConnector::DenyConnection(IceStream *strm,int code,
					const QString &str)
{
  SendHeader(strm,QString().sprintf("HTTP/1.0 %d ",code)+str);
  SendHeader(strm,"Server: Icecast 2.4.0");
  SendHeader(strm,"Date: "+
	     QDateTime::currentDateTime().addSecs(4*3600).
	     toString("ddd, dd MM yyyy hh:mm:ss GMT").toUtf8());
  SendHeader(strm,
	     QString().sprintf("Content-Length: %d",str.toUtf8().length()));
  SendHeader(strm);
  strm->socket()->write(str.toUtf8());
  strm->socket()->disconnect();
  iceserv_garbage_timer->start(1);
}
