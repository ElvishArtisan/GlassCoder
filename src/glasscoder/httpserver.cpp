// httpserver.cpp
//
// HTTP Server
//
// (C) Copyright 2016 Fred Gleason <fredg@paravelsystems.com>
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

#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include <QDateTime>
#include <QFile>

#include <openssl/evp.h>

#include "httpserver.h"
#include "profile.h"

HttpServer::HttpServer(QObject *parent)
  : QObject(parent)
{
  http_dump_transactions=getenv("Webhost_ShowHttpTransaction")!=NULL;

  http_server=new QTcpServer(this);
  connect(http_server,SIGNAL(newConnection()),this,SLOT(newConnectionData()));

  http_read_mapper=new QSignalMapper(this);
  connect(http_read_mapper,SIGNAL(mapped(int)),this,SLOT(readyReadData(int)));

  http_disconnect_mapper=new QSignalMapper(this);
  connect(http_disconnect_mapper,SIGNAL(mapped(int)),
	  this,SLOT(disconnectedData(int)));

  http_cgi_finished_mapper=new QSignalMapper(this);
  connect(http_cgi_finished_mapper,SIGNAL(mapped(int)),
	  this,SLOT(cgiFinishedData(int)));

  http_garbage_timer=new QTimer(this);
  http_garbage_timer->setSingleShot(true);
  connect(http_garbage_timer,SIGNAL(timeout()),this,SLOT(garbageData()));
}


HttpServer::~HttpServer()
{
  for(unsigned i=0;i<http_connections.size();i++) {
    delete http_connections[i];
  }
  delete http_garbage_timer;
  delete http_disconnect_mapper;
  delete http_read_mapper;
  delete http_server;
}


bool HttpServer::listen(uint16_t port)
{
  return http_server->listen(QHostAddress::Any,port);
}


bool HttpServer::listen(const QHostAddress &iface,uint16_t port)
{
  return http_server->listen(iface,port);
}


void HttpServer::sendSocketMessage(int conn_id,SocketMessage::OpCode opcode,
				     const QByteArray &data)
{
  QByteArray packet;
  HttpConnection *conn=http_connections[conn_id];

  //
  // OpCode
  //
  packet.append(0x80|(0x0F&opcode));

  //
  // Payload Length
  //
  // WARNING: This breaks with messages longer than 4,294,967,295 bytes!
  //          See RFC 6455 Section 5.2
  //
  // DO NOT MASK return messages, as per RFC 6455 Sect. 5.1
  //
  if(data.length()<126) {
    packet.append(data.length());
  }
  else {
    if(data.length()<65536) {
      packet.append(126);
      packet.append(data.length()>>8);
      packet.append(0xFF&data.length());
    }
    else {
      packet.append(127);
      packet.append((char)0);
      packet.append((char)0);
      packet.append((char)0);
      packet.append((char)0);
      packet.append(data.length()>>24);
      packet.append(0xFF&(data.length()>>16));
      packet.append(0xFF&(data.length()>>8));
      packet.append(0xFF&data.length());
    }
  }

  //
  // Payload
  //
  packet.append(data);

  conn->socket()->write(packet);
}


void HttpServer::sendSocketMessage(int conn_id,const QByteArray &data)
{
  sendSocketMessage(conn_id,SocketMessage::Binary,data);
}


void HttpServer::sendSocketMessage(int conn_id,const QString &str)
{
  sendSocketMessage(conn_id,SocketMessage::Text,str.toUtf8());
}


void HttpServer::closeSocketConnection(int conn_id,uint16_t status,
					 const QByteArray &body)
{
  QByteArray payload;

  payload.append(0xFF&(status>>8));
  payload.append(0xFF&status);
  payload.append(body);
  sendSocketMessage(conn_id,SocketMessage::Close,payload);
}


QStringList HttpServer::userRealms() const
{
  QStringList ret;

  for(std::map<QString,std::vector<HttpUser *> >::const_iterator it=http_users.begin();
      it!=http_users.end();it++) {
    ret.push_back(it->first);
  }

  return ret;
}


QStringList HttpServer::userNames(const QString &realm)
{
  QStringList ret;

  for(unsigned i=0;i<http_users[realm].size();i++) {
    ret.push_back(http_users[realm][i]->name());
  }

  return ret;
}


void HttpServer::addUser(const QString &realm,const QString &name,
			   const QString &passwd)
{
  for(unsigned i=0;i<http_users[realm].size();i++) {
    if(http_users[realm][i]->name()==name) {
      http_users[realm][i]->setPassword(passwd);
      return;
    }
  }
  http_users[realm].push_back(new HttpUser(name,passwd));
}


void HttpServer::removeUser(const QString &realm,const QString &name)
{
  for(unsigned i=0;i<http_users[realm].size();i++) {
    if(http_users[realm][i]->name()==name) {
      delete http_users[realm][i];
      http_users[realm].erase(http_users[realm].begin()+i);
      return;
    }
  }
}


bool HttpServer::loadUsers(const QString &filename)
{
  Profile *p=new Profile();
  if(!p->setSource(filename)) {
    return false;
  }
  for(std::map<QString,std::vector<HttpUser *> >::const_iterator it=http_users.begin();
      it!=http_users.end();it++) {
    for(unsigned i=0;i<it->second.size();i++) {
      delete it->second[i];
    }
  }
  http_users.clear();

  int count=0;
  QString realm;
  QString name;
  QString passwd;
  bool ok=false;
  QString section=QString().sprintf("WebHostUser%d",count+1);

  realm=p->stringValue(section,"Realm","",&ok);
  while(ok) {
    name=p->stringValue(section,"Name");
    passwd=p->stringValue(section,"Password");
    addUser(realm,name,passwd);
    count++;
    section=QString().sprintf("WebHostUser%d",count+1);
    realm=p->stringValue(section,"Realm","",&ok);
  }

  return true;
}


bool HttpServer::saveUsers(const QString &filename)
{
  FILE *f=NULL;
  int count=0;

  if((f=fopen((filename+".back").toUtf8(),"w"))==NULL) {
    return false;
  }
  for(std::map<QString,std::vector<HttpUser *> >::const_iterator it=http_users.begin();
      it!=http_users.end();it++) {
    for(unsigned i=0;i<it->second.size();i++) {
      fprintf(f,"[WebHostUser%d]\n",count+1);
      fprintf(f,"Realm=%s\n",(const char *)it->first.toUtf8());
      fprintf(f,"Name=%s\n",(const char *)it->second[i]->name().toUtf8());
      fprintf(f,"Password=%s\n",
	      (const char *)it->second[i]->password().toUtf8());
      fprintf(f,"\n");
      count++;
    }
  }
  fclose(f);
  rename((filename+".back").toUtf8(),filename.toUtf8());

  return true;
}


void HttpServer::addStaticSource(const QString &uri,const QString &mimetype,
				   const QString &filename,const QString &realm)
{
  http_static_uris.push_back(uri);
  http_static_mimetypes.push_back(mimetype);
  http_static_filenames.push_back(filename);
  http_static_realms.push_back(realm);
}


void HttpServer::addCgiSource(const QString &uri,const QString &filename,
				const QString &realm)
{
  http_cgi_uris.push_back(uri);
  http_cgi_filenames.push_back(filename);
  http_cgi_realms.push_back(realm);
}


void HttpServer::addSocketSource(const QString &uri,const QString &proto,
				   const QString &realm)
{
  http_socket_uris.push_back(uri);
  http_socket_protocols.push_back(proto);
  http_socket_realms.push_back(realm);
}


void HttpServer::requestReceived(HttpConnection *conn)
{
  switch(conn->method()) {
  case HttpConnection::Get:
    getRequestReceived(conn);
    return;

  case HttpConnection::Post:
    postRequestReceived(conn);
    return;

  case HttpConnection::Head:
    headRequestReceived(conn);
    return;

  case HttpConnection::Put:
    putRequestReceived(conn);
    return;

  case HttpConnection::Delete:
    deleteRequestReceived(conn);
    return;

  case HttpConnection::None:
    break;
  }
  fprintf(stderr,"URI \"%s\" not found\n",
	  (const char *)conn->uri().toUtf8());
  conn->sendError(404,"404 Not found");
}


void HttpServer::getRequestReceived(HttpConnection *conn)
{
  fprintf(stderr,"URI \"%s\" not found\n",
	  (const char *)conn->uri().toUtf8());
  conn->sendError(404,"404 Not found");
}


void HttpServer::postRequestReceived(HttpConnection *conn)
{
  fprintf(stderr,"URI \"%s\" not found\n",
	  (const char *)conn->uri().toUtf8());
  conn->sendError(404,"404 Not found");
}


void HttpServer::headRequestReceived(HttpConnection *conn)
{
  fprintf(stderr,"URI \"%s\" not found\n",
	  (const char *)conn->uri().toUtf8());
  conn->sendError(404,"404 Not found");
}


void HttpServer::putRequestReceived(HttpConnection *conn)
{
  fprintf(stderr,"URI \"%s\" not found\n",
	  (const char *)conn->uri().toUtf8());
  conn->sendError(404,"404 Not found");
}


void HttpServer::deleteRequestReceived(HttpConnection *conn)
{
  fprintf(stderr,"URI \"%s\" not found\n",
	  (const char *)conn->uri().toUtf8());
  conn->sendError(404,"404 Not found");
}


bool HttpServer::authenticateUser(const QString &realm,const QString &name,
				    const QString &passwd)
{
  for(unsigned i=0;i<http_users[realm].size();i++) {
    if(http_users[realm][i]->isValid(name,passwd)) {
      return true;
    }
  }
  return false;
}


void HttpServer::newConnectionData()
{
  int id=-1;

  for(unsigned i=0;i<http_connections.size();i++) {
    if(http_connections[i]==NULL) {
      http_connections[i]=
	new HttpConnection(i,http_server->nextPendingConnection(),
			     http_dump_transactions,this);
      id=i;
      break;
    }
  }
  if(id<0) {
    id=http_connections.size();
    http_connections.
      push_back(new HttpConnection(id,http_server->nextPendingConnection(),
				     http_dump_transactions,this));
  }
  connect(http_connections[id]->socket(),SIGNAL(readyRead()),
	  http_read_mapper,SLOT(map()));
  http_read_mapper->setMapping(http_connections[id]->socket(),id);

  connect(http_connections[id]->socket(),SIGNAL(disconnected()),
	  http_disconnect_mapper,SLOT(map()));
  http_disconnect_mapper->setMapping(http_connections[id]->socket(),id);

  connect(http_connections[id],SIGNAL(cgiFinished()),
	  http_cgi_finished_mapper,SLOT(map()));
  http_cgi_finished_mapper->setMapping(http_connections[id]->socket(),id);
}


void HttpServer::readyReadData(int id)
{
  HttpConnection *conn=http_connections[id];

  switch(conn->parseState()) {
  case 0:
    ReadMethodLine(conn);
    break;

  case 1:
    ReadHeaders(conn);
    break;

  case 2:
    ReadBody(conn);
    break;

  case 10:
    ReadWebsocket(conn);
    break;
  }
}


void HttpServer::disconnectedData(int id)
{
  HttpConnection *conn=http_connections[id];

  if(http_connections[id]->isWebsocket()) {
    emit socketConnectionClosed(id,conn->socketCloseStatus(),
				conn->socketCloseBody());
  }
  http_garbage_timer->start(0);
}


void HttpServer::cgiFinishedData(int id)
{
  http_garbage_timer->start(0);
}


void HttpServer::garbageData()
{
  for(unsigned i=0;i<http_connections.size();i++) {
    if(http_connections[i]!=NULL) {
      if(http_connections[i]->socket()->state()!=
	 QAbstractSocket::ConnectedState) {
	delete http_connections[i];
	http_connections[i]=NULL;
      }
    }
  }
}


void HttpServer::ReadMethodLine(HttpConnection *conn)
{
  QStringList hdr_names;
  QStringList hdr_values;
  QString line;

  if(conn->socket()->canReadLine()) {
    line=QString(conn->socket()->readLine()).trimmed();
    if(http_dump_transactions) {
      fprintf(stderr,"REQUEST-LINE: %s\n",(const char *)line.toUtf8());
    }
    QStringList f0=line.split(" ");
    if(f0.size()!=3) {
      conn->sendError(400,"400 Bad Request<br>Malformed HTTP request");
      return;
    }

    //
    // The HTTP Method
    //
    if(f0[0].trimmed()=="GET") {
      conn->setMethod(HttpConnection::Get);
    }
    if(f0[0].trimmed()=="POST") {
      conn->setMethod(HttpConnection::Post);
    }
    if(f0[0].trimmed()=="HEAD") {
      conn->setMethod(HttpConnection::Head);
    }
    if(conn->method()==HttpConnection::None) {
      hdr_names.push_back("Allow");
      hdr_values.push_back("GET");
      if(IsCgiScript(f0[1].trimmed())) {
	hdr_values.back()+=",POST";
      }
      conn->sendError(501,"501 Not implemented",hdr_names,hdr_values);
      return;
    }

    conn->setUri(f0[1].trimmed());

    QStringList f1=f0[2].trimmed().split("/");
    if(f1.size()!=2) {
      conn->sendError(400,"400 Bad Request<br>Malformed HTTP request");
      return;
    }
    if(!conn->setProtocolVersion(f1[1])) {
      conn->sendError(400,"400 Bad Request<br>Malformed HTTP request");
      return;
    }
    if((conn->majorProtocolVersion()!=1)||(conn->minorProtocolVersion()>1)) {
      conn->sendError(505,"505 HTTP Version Not Supported<br>This server only supports HTTP v1.x");
      return;
    }
    conn->setParseState(1);
    ReadHeaders(conn);
  }
}


void HttpServer::ReadHeaders(HttpConnection *conn)
{
  QStringList hdr_names;
  QStringList hdr_values;
  QString line;
  bool ok=false;

  while(conn->socket()->canReadLine()) {
    line=QString(conn->socket()->readLine()).trimmed();
    if(http_dump_transactions) {
      fprintf(stderr,"HEADER: %s\n",(const char *)line.toUtf8());
    }
    bool processed=false;
    QStringList f0=line.split(": ",QString::KeepEmptyParts);
    if(f0.size()>=2) {
      QString hdr=f0[0].trimmed().toLower();
      f0.erase(f0.begin());
      QString value=f0.join(": ");
      if(hdr=="authorization") {
	conn->setAuthorization(value);
	processed=true;
      }
      if(hdr=="content-length") {
	int64_t len=value.toLongLong(&ok);
	if(!ok) {
	  conn->
	    sendError(400,"400 Bad Request Malformed Content-Length: header");
	  return;
	}
	conn->setContentLength(len);
	processed=true;
      }
      if(hdr=="content-type") {
	QStringList f1=value.split(";");
	conn->setContentType(f1[0]);
	processed=true;
      }
      if(hdr=="host") {
	if(!conn->setHost(value)) {
	  conn->sendError(400,"400 Bad Request Malformed Host: header");
	  return;
	}
	processed=true;
      }
      if(hdr=="referer") {
	conn->setReferrer(value);
	processed=true;
      }
      if(hdr=="sec-websocket-protocol") {
	conn->setSubProtocol(value);
	processed=true;
      }
      if(hdr=="upgrade") {
	conn->setUpgrade(value);
	processed=true;
      }
      if(hdr=="user-agent") {
	conn->setUserAgent(value);
	processed=true;
      }
      if(!processed) {
	conn->addHeader(hdr,value);
      }
    }
    else {
      if(line.isEmpty()) {  // End of headers
	if((conn->minorProtocolVersion()==1)&&(conn->hostPort()==0)) {
	  conn->
	    sendError(400,"400 Bad Request -- Missing/malformed Host: header");
	  return;
	}
	if(conn->method()==HttpConnection::Get) {
	  ProcessRequest(conn);
	}
	else {
	  conn->setParseState(2);
	  ReadBody(conn);
	}
	return;
      }
    }
  }
}


void HttpServer::ReadWebsocket(HttpConnection *conn)
{
  QByteArray data=conn->socket()->readAll();
  uint32_t plen=0;
  int offset=0;
  SocketMessage *msg=NULL;

  if((0x70&data[0])!=0) {  // Extension bits set
    conn->socket()->close();
  }
  if((0x80&data[1])==0) {  // Mask not set
    conn->socket()->close();
  }
  SocketMessage::OpCode opcode=(SocketMessage::OpCode)(0x0F&data[0]);
  if(SocketMessage::isControlMessage(opcode)) {
    msg=conn->cntlSocketMessage();
  }
  else {
    msg=conn->appSocketMessage();
  }

  bool finished=(0x80&data[0])!=0;

  if(opcode!=SocketMessage::Continuation) {
    msg->setOpCode(opcode);
    msg->clearPayload();
  }
  
  //
  // Payload Length
  //
  // WARNING: This breaks with messages longer than 4,294,967,295 bytes!
  //          See RFC 6455 Section 5.2
  //
  plen=0x7F&data[1];
  if(plen==126) {
    plen=((0xFF&data[2])<<8)+(0xFF&data[3]);
    offset=2;
  }
  else {
    if(plen==127) {
      plen=((0xFF&data[6])<<24)+((0xFF&data[7])<<16)+((0xFF&data[8])<<8)+
	(0xFF&data[9]);
      offset=8;
    }
  }

  //
  // Extract Payload
  //
  for(uint32_t i=0;i<plen;i++) {
    msg->appendPayload(data[offset+6+i]^data[offset+2+(i%4)]);
  }

  //
  // Disposition
  //
  switch(msg->opCode()) {
  case SocketMessage::Text:
  case SocketMessage::Binary:
  case SocketMessage::AppReserv3:
  case SocketMessage::AppReserv4:
  case SocketMessage::AppReserv5:
  case SocketMessage::AppReserv6:
  case SocketMessage::AppReserv7:
    if(finished) {
      emit socketMessageReceived(conn->id(),msg);
    }
    break;

  case SocketMessage::Close:
    if(msg->payload().length()>=2) {
      uint16_t status=((0xFF&msg->payload()[0])<<8)+(0xFF&msg->payload()[1]);
      conn->setSocketCloseStatus(status);
      conn->setSocketCloseBody(msg->payload().
			       mid(2,msg->payload().length()-2).constData());
    }
    sendSocketMessage(conn->id(),SocketMessage::Close,msg->payload());
    conn->socket()->close();
    break;

  case SocketMessage::Ping:
    sendSocketMessage(conn->id(),SocketMessage::Pong,msg->payload());
    break;

  case SocketMessage::Continuation:
  case SocketMessage::Pong:
  case SocketMessage::CntlReserv11:
  case SocketMessage::CntlReserv12:
  case SocketMessage::CntlReserv13:
  case SocketMessage::CntlReserv14:
  case SocketMessage::CntlReserv15:
    break;
  }
}


void HttpServer::ReadBody(HttpConnection *conn)
{
  conn->appendBody(conn->socket()->
		  read(conn->contentLength()-conn->body().length()));
  if(conn->body().length()==conn->contentLength()) {
    ProcessRequest(conn);
  }
}


void HttpServer::ProcessRequest(HttpConnection *conn)
{
  if(conn->upgrade().toLower()=="websocket") {
    for(int i=0;i<http_socket_uris.size();i++) {
      if((conn->uri()==http_socket_uris[i])||
	 (conn->subProtocol()==http_socket_protocols[i])) {
	StartWebsocket(conn,i);
	return;
      }
    }
    requestReceived(conn);
    return;
  }
  for(int i=0;i<http_static_uris.size();i++) {
    if(conn->uri()==http_static_uris[i]) {
      SendStaticSource(conn,i);
      return;
    }
  }
  for(int i=0;i<http_cgi_uris.size();i++) {
    if(conn->uri()==http_cgi_uris[i]) {
      SendCgiSource(conn,i);
      return;
    }
  }
  requestReceived(conn);
}


void HttpServer::StartWebsocket(HttpConnection *conn,int n)
{
  QStringList hdrs=conn->headerNames();
  QStringList values=conn->headerValues();
  QString key;
  QByteArray resp;

  if(!AuthenticateRealm(conn,http_socket_realms[n],
			conn->authName(),conn->authPassword())) {
    return;
  }

  for(int i=0;i<hdrs.size();i++) {
    if(hdrs[i]=="sec-websocket-key") {
      key=values[i].trimmed();
    }
  }
  if((!conn->protocolAtLeast(1,1))||(conn->method()!=HttpConnection::Get)||
     key.isEmpty()) {
    conn->sendError(400);
    return;
  }
  resp=GetWebsocketHandshake(key);
  if(resp.length()==0) {
    conn->sendError(500,"500 Internal processing error");
    return;
  }
  QString statline="HTTP/1.1 101 Switching Protocols\r\n";
  if(http_dump_transactions) {
    fprintf(stderr,"STATUS-LINE: %s",(const char *)statline.toUtf8());
  }
  conn->socket()->write(statline.toUtf8());
  conn->sendHeader("Upgrade","websocket");
  conn->sendHeader("Connection","upgrade");
  conn->sendHeader("Sec-WebSocket-Accept",resp);
  conn->sendHeader("Sec-WebSocket-Protocol",conn->subProtocol());
  conn->sendHeader();
  conn->setParseState(10);
  conn->setWebsocket(true);
  emit newSocketConnection(conn->id(),conn->uri(),conn->subProtocol());
}


void HttpServer::SendStaticSource(HttpConnection *conn,int n)
{
  QFile file(http_static_filenames[n]);
  QByteArray data;

  if(!AuthenticateRealm(conn,http_static_realms[n],
			conn->authName(),conn->authPassword())) {
    return;
  }

  if(!file.exists()) {
    conn->sendError(404);
    return;
  }
  if(!file.open(QIODevice::ReadOnly)) {
    conn->sendError(500);
    return;
  }
  switch(conn->method()) {
  case HttpConnection::Get:
  case HttpConnection::Post:
    data=file.readAll(); 
    conn->sendResponse(200,data,http_static_mimetypes[n]);
    file.close();
    return;

  case HttpConnection::Head:
    conn->sendResponseHeader(200,http_static_mimetypes[n]);
    conn->sendHeader();
    return;

  case HttpConnection::Put:
  case HttpConnection::Delete:
  case HttpConnection::None:
    break;
  }
  conn->sendResponse(405,"Method not allowed");
}


void HttpServer::SendCgiSource(HttpConnection *conn,int n)
{
  if(!AuthenticateRealm(conn,http_cgi_realms[n],
			conn->authName(),conn->authPassword())) {
    return;
  }
  if(conn->method()==HttpConnection::Head) {
    conn->sendResponse(405,"405 Method Not Allowd");
    return;
  }
  conn->startCgiScript(http_cgi_filenames[n]);
}


bool HttpServer::IsCgiScript(const QString &uri) const
{
  for(int i=0;i<http_cgi_uris.size();i++) {
    if(http_cgi_uris[i]==uri) {
      return true;
    }
  }
  return false;
}


bool HttpServer::AuthenticateRealm(HttpConnection *conn,
				     const QString &realm,const QString &name,
				     const QString &passwd)
{
  QStringList hdrs;
  QStringList values;

  if(realm.isEmpty()||authenticateUser(realm,name,passwd)) {
    return true;
  }
  hdrs.push_back("WWW-Authenticate");
  values.push_back("Basic realm=\""+realm+"\"");
  conn->sendResponse(401,hdrs,values,"401 Unauthorized");
  return false;
}


QByteArray HttpServer::GetWebsocketHandshake(const QString &key) const
{
  EVP_MD_CTX *mdctx;
  const EVP_MD *md;
  unsigned char md_value[EVP_MAX_MD_SIZE];
  unsigned md_len;

  OpenSSL_add_all_digests();
  if((md=EVP_get_digestbyname("sha1"))==NULL) {
    return QByteArray();
  }
  mdctx=EVP_MD_CTX_create();
  EVP_DigestInit_ex(mdctx,md,NULL);
  EVP_DigestUpdate(mdctx,(key+WEBSOCKET_MAGIC_STRING).toUtf8(),
		   (key+WEBSOCKET_MAGIC_STRING).length());
  EVP_DigestFinal_ex(mdctx,md_value,&md_len);
  EVP_MD_CTX_destroy(mdctx);

  return QByteArray((const char *)md_value).toBase64();
}
