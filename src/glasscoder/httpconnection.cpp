// httpconnection.cpp
//
// HTTP connection state for HttpServer
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

#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include <QDateTime>
#include <QProcessEnvironment>

#include "httpserver.h"

HttpConnection::HttpConnection(int id,QTcpSocket *sock,bool dump_trans,
				   QObject *parent)
  : QObject(parent)
{
  conn_id=id;
  conn_dump_transactions=dump_trans;
  conn_method=HttpConnection::None;
  conn_websocket=false;
  conn_major_protocol_version=0;
  conn_minor_protocol_version=0;
  conn_host_port=0;
  conn_auth_type=HttpConnection::AuthNone;
  conn_content_length=0;
  conn_content_type="application/octet-stream";

  conn_socket=sock;
  conn_cgi_process=NULL;
  conn_cgi_headers_active=true;
  conn_parse_state=0;

  conn_app_socket_message=new SocketMessage();
  conn_cntl_socket_message=new SocketMessage();
}


HttpConnection::~HttpConnection()
{
  delete conn_cntl_socket_message;
  delete conn_app_socket_message;
  if(conn_cgi_process!=NULL) {
    delete conn_cgi_process;
  }
  delete conn_socket;
}


int HttpConnection::id() const
{
  return conn_id;
}


unsigned HttpConnection::majorProtocolVersion() const
{
  return conn_major_protocol_version;
}


unsigned HttpConnection::minorProtocolVersion() const
{
  return conn_minor_protocol_version;
}


bool HttpConnection::protocolAtLeast(int major,int minor) const
{
  return
    QString().sprintf("%u.%u",
		      conn_major_protocol_version,conn_minor_protocol_version).
    toFloat()<=QString().sprintf("%u.%u",major,minor).toFloat();
}


bool HttpConnection::setProtocolVersion(const QString &str)
{
  QStringList f0=str.split(".");
  bool ok=false;

  if(f0.size()!=2) {
    return false;
  }
  conn_major_protocol_version=f0[0].toInt(&ok);
  if(!ok) {
    return false;
  }
  conn_minor_protocol_version=f0[1].toInt(&ok);
  if(!ok) {
    return false;
  }
  return true;
}


HttpConnection::Method HttpConnection::method() const
{
  return conn_method;
}


void HttpConnection::setMethod(Method meth)
{
  conn_method=meth;
}


bool HttpConnection::isWebsocket() const
{
  return conn_websocket;
}


void HttpConnection::setWebsocket(bool state)
{
  conn_websocket=state;
}


QString HttpConnection::uri() const
{
  return conn_uri;
}


void HttpConnection::setUri(const QString &uri)
{
  conn_uri=uri;
}


QString HttpConnection::hostName() const
{
  return conn_host_name;
}


uint16_t HttpConnection::hostPort() const
{
  return conn_host_port;
}


bool HttpConnection::setHost(const QString &str)
{
  QStringList f0=str.split(":");
  bool ok=false;

  conn_socket_close_status=1005;

  if(f0.size()>2) {
    return false;
  }
  conn_host_name=f0[0];
  if(f0.size()==2) {
    conn_host_port=f0[1].toUInt(&ok);
    if((!ok)||(conn_host_port==0)) {
      return false;
    }
  }
  else {
    conn_host_port=80;
  }
  return true;
}


HttpConnection::AuthType HttpConnection::authType() const
{
  return conn_auth_type;
}


QString HttpConnection::authName() const
{
  return conn_auth_name;
}


QString HttpConnection::authPassword() const
{
  return conn_auth_password;
}


bool HttpConnection::setAuthorization(const QString &str)
{
  bool ret=false;

  QStringList f0=str.trimmed().split(" ");
  if((f0[0].toLower()=="basic")&&(f0.size()==2)) {
    QStringList f1=QString(QByteArray::fromBase64(f0[1].toUtf8())).
      split(":",QString::KeepEmptyParts);
    if(f1.size()>=2) {
      conn_auth_name=f1[0];
      f1.erase(f1.begin());
      conn_auth_password=f1.join(":");
      ret=true;
    }
  }

  return ret;
}


int64_t HttpConnection::contentLength() const
{
  return conn_content_length;
}


void HttpConnection::setContentLength(int64_t len)
{
  conn_content_length=len;
}


QString HttpConnection::contentType() const
{
  return conn_content_type;
}


void HttpConnection::setContentType(const QString &mimetype)
{
  conn_content_type=mimetype;
}


QString HttpConnection::referrer() const
{
  return conn_referrer;
}


void HttpConnection::setReferrer(const QString &str)
{
  conn_referrer=str;
}


QString HttpConnection::subProtocol() const
{
  return conn_sub_protocol;
}


void HttpConnection::setSubProtocol(const QString &str)
{
  conn_sub_protocol=str;
}


QString HttpConnection::upgrade() const
{
  return conn_upgrade;
}


void HttpConnection::setUpgrade(const QString &str)
{
  conn_upgrade=str;
}


QString HttpConnection::userAgent() const
{
  return conn_user_agent;
}


void HttpConnection::setUserAgent(const QString &str)
{
  conn_user_agent=str;
}


uint16_t HttpConnection::socketCloseStatus() const
{
  return conn_socket_close_status;
}


void HttpConnection::setSocketCloseStatus(uint16_t status)
{
  conn_socket_close_status=status;
}


QByteArray HttpConnection::socketCloseBody() const
{
  return conn_socket_close_body;
}


void HttpConnection::setSocketCloseBody(const QByteArray &data)
{
  conn_socket_close_body=data;
}


QStringList HttpConnection::headerNames() const
{
  return conn_header_names;
}


QStringList HttpConnection::headerValues() const
{
  return conn_header_values;
}


QString HttpConnection::headerValue(const QString &name) const
{
  for(int i=0;i<conn_header_names.size();i++) {
    if(conn_header_names[i]==name) {
      return conn_header_values[i];
    }
  }

  return QString();
}


void HttpConnection::addHeader(const QString &name,const QString &value)
{
  conn_header_names.push_back(name);
  conn_header_values.push_back(value);
}


QByteArray HttpConnection::body() const
{
  return conn_body;
}


void HttpConnection::appendBody(const QByteArray &data)
{
  conn_body.append(data);
}


QString HttpConnection::dump() const
{
  QString ret="";

  switch(method()) {
  case HttpConnection::Get:
    ret+="Method: GET\n";
    break;

  case HttpConnection::None:
    ret+="Method: [none]\n";
    break;

  default:
    ret+="Method: Unknown\n";
    break;
  }
  ret+=QString().sprintf("HTTP Protocol: %u.%u\n",majorProtocolVersion(),
			 minorProtocolVersion());
  ret+="URI: "+uri()+"\n";
  ret+="Host: "+hostName()+":"+QString().sprintf("%u",hostPort())+"\n";
  ret+="User-Agent: "+userAgent()+"\n";

  ret+="HEADERS\n";
  for(int i=0;i<conn_header_names.size();i++) {
    ret+="  "+conn_header_names[i]+": "+conn_header_values[i]+"\n";
  }
  return ret;
}


void HttpConnection::startCgiScript(const QString &filename)
{
  conn_cgi_process=new QProcess(this);
  connect(conn_cgi_process,SIGNAL(started()),this,SLOT(cgiStartedData()));
  connect(conn_cgi_process,SIGNAL(readyReadStandardOutput()),
	  this,SLOT(cgiReadyReadData()));
  connect(conn_cgi_process,SIGNAL(finished(int,QProcess::ExitStatus)),
	  this,SLOT(cgiFinishedData(int,QProcess::ExitStatus)));
  connect(conn_cgi_process,SIGNAL(error(QProcess::ProcessError)),
	  this,SLOT(cgiErrorData(QProcess::ProcessError)));
  QProcessEnvironment env;
  if(contentLength()>=0) {
    env.insert("CONTENT_LENGTH",
	       QString().sprintf("%ld",contentLength()));
    env.insert("CONTENT_TYPE",contentType());
  }
  env.insert("GATEWAY_INTERFACE","1.1");
  env.insert("HTTP_HOST",socket()->localAddress().toString());
  if(!referrer().isEmpty()) {
    env.insert("HTTP_REFERER",referrer());
  }
  if(!userAgent().isEmpty()) {
    env.insert("HTTP_USER_AGENT",userAgent());
  }
  env.insert("REMOTE_ADDR",socket()->peerAddress().toString());
  env.insert("REMOTE_HOST",socket()->peerAddress().toString());
  env.insert("REMOTE_PORT",QString().sprintf("%u",0xFFFF&socket()->peerPort()));
  if(method()==HttpConnection::Post) {
    env.insert("REQUEST_METHOD","POST");
  }
  else {
    env.insert("REQUEST_METHOD","GET");
  }
  env.insert("REQUEST_URI",uri());
  env.insert("SCRIPT_FILENAME",filename);
  env.insert("SCRIPT_NAME",filename);
  env.insert("SERVER_ADMIN","noone@nowhere.com");
  env.insert("SERVER_NAME",socket()->localAddress().toString());
  env.insert("SERVER_PORT",
	     QString().sprintf("%u",0xFFFF&socket()->localPort()));
  env.insert("SERVER_SOFTWARE",QString("Webhost-")+VERSION);
  conn_cgi_process->setProcessEnvironment(env);
  conn_cgi_process->start(filename);
}


void HttpConnection::sendResponseHeader(int stat_code,const QString &mimetype)
{
  QString statline=QString().sprintf("HTTP/1.1 %d ",stat_code)+
    HttpConnection::statusText(stat_code)+"\r\n";
  if(conn_dump_transactions) {
    fprintf(stderr,"STATUS-LINE: %s",(const char *)statline.toUtf8());
  }
  socket()->write(statline.toUtf8());
  sendHeader("Date",HttpConnection::
	 datetimeStamp(QDateTime(QDate::currentDate(),QTime::currentTime())));
  sendHeader("Server",QString("Webhost/")+VERSION);
  sendHeader("Connection","close");// FIXME: Implement persistent connections
  if(!mimetype.isEmpty()) {
    sendHeader("Content-Type",mimetype);
  }
}


void HttpConnection::sendResponse(int stat_code,
				    const QStringList &hdr_names,
				    const QStringList &hdr_values,
				    const QByteArray &body,
				    const QString &mimetype)
{
  sendResponseHeader(stat_code,mimetype);
  if(body.length()>0) {
    sendHeader("Content-Length",QString().sprintf("%d",body.length()));
  }
  for(int i=0;i<hdr_names.size();i++) {
    sendHeader(hdr_names[i],hdr_values[i]);
  }
  sendHeader();
  socket()->write(body);
}


void HttpConnection::sendResponse(int stat_code,
				      const QByteArray &body,
				      const QString &mimetype)
{
  sendResponse(stat_code,QStringList(),QStringList(),body,mimetype);
}


void HttpConnection::sendError(int stat_code,const QString &msg,
				   const QStringList &hdr_names,
				   const QStringList &hdr_values)
{
  QString err_text=msg;
  if(err_text.isEmpty()) {
    err_text=
      QString().sprintf("%d ",stat_code)+HttpConnection::statusText(stat_code);
  }
  sendResponse(stat_code,hdr_names,hdr_values,err_text.toUtf8());
}


QTcpSocket *HttpConnection::socket()
{
  return conn_socket;
}


SocketMessage *HttpConnection::appSocketMessage()
{
  return conn_app_socket_message;
}

SocketMessage *HttpConnection::cntlSocketMessage()
{
  return conn_cntl_socket_message;
}

void HttpConnection::cgiStartedData()
{
  if(method()==HttpConnection::Post) {
    conn_cgi_process->write(body());
    conn_cgi_process->write("\r\n");
  }
}


void HttpConnection::cgiReadyReadData()
{
  QByteArray data;

  if(conn_cgi_headers_active) {
    data=conn_cgi_process->readLine().trimmed();
    if(data.length()==0) {
      sendResponseHeader(200);
      for(int i=0;i<conn_cgi_headers.size();i++) {
	socket()->write((conn_cgi_headers[i]+"\r\n").toUtf8());
      }
      socket()->write("\r\n",2);
      conn_cgi_headers_active=false;
      while(conn_cgi_process->bytesAvailable()>0) {
	data=conn_cgi_process->read(1024);
	socket()->write(data);
      }
    }
    else {
      conn_cgi_headers.push_back(data);
    }
  }
  else {
    while(conn_cgi_process->bytesAvailable()>0) {
      data=conn_cgi_process->read(1024);
      socket()->write(data);
    }
  }
}


void HttpConnection::cgiFinishedData(int exit_code,
					 QProcess::ExitStatus status)
{
  if(status!=QProcess::NormalExit) {
    sendError(500,"CGI process crashed");
  }
  else {
    if(exit_code!=0) {
      sendError(500,QString().
	  sprintf("CGI process returned non-zero exit code [%d]",exit_code));
    }
    else {
      cgiReadyReadData();
    }
  }
  socket()->close();
  emit cgiFinished();
}


void HttpConnection::cgiErrorData(QProcess::ProcessError err)
{
  QString err_text=QString().sprintf("unknown process error %d",err);
  switch(err) {
  case QProcess::FailedToStart:
    err_text="failed to start";
    break;

  case QProcess::Crashed:
    err_text="crashed";
    break;

  case QProcess::Timedout:
    err_text="Timed out";
    break;

  case QProcess::WriteError:
    err_text="write error";
    break;

  case QProcess::ReadError:
    err_text="read error";
    break;

  case QProcess::UnknownError:
    err_text="unknown error";
    break;
  }
  sendError(500,"500 CGI process error ["+err_text+"]");
  socket()->close();
  emit cgiFinished();
}




void HttpConnection::sendHeader(const QString &name,const QString &value)
{
  if(name.isEmpty()&&value.isEmpty()) {
    if(conn_dump_transactions) {
      fprintf(stderr,"HEADER:\r\n");
    }
    socket()->write("\r\n");
  }
  else {
    QString line=name+": "+value+"\r\n";
    if(conn_dump_transactions) {
      fprintf(stderr,"HEADER: %s",(const char *)line.toUtf8());
    }
    socket()->write(line.toUtf8());
  }
}


int HttpConnection::parseState() const
{
  return conn_parse_state;
}


void HttpConnection::setParseState(int state)
{
  conn_parse_state=state;
}


QString HttpConnection::statusText(int stat_code)
{
  //
  // From RFC 2616 Section 10
  //
  QString ret="Unknown";

  if((stat_code>=100)&&(stat_code<200)) {
    ret="Continue";
  }
  if((stat_code>=200)&&(stat_code<300)) {
    ret="OK";
  }
  if((stat_code>=300)&&(stat_code<400)) {
    ret="Multiple Choices";
  }
  if((stat_code>=400)&&(stat_code<500)) {
    ret="Bad Request";
  }
  if((stat_code>=500)&&(stat_code<600)) {
    ret="Internal Server Error";
  }

  switch(stat_code) {
  case 100:
    ret="Continue";
    break;

  case 101:
    ret="Switching Protocols";
    break;

  case 200:
    ret="OK";
    break;

  case 201:
    ret="Created";
    break;

  case 202:
    ret="Accepted";
    break;

  case 203:
    ret="Non-Authoritative Information";
    break;

  case 204:
    ret="No Content";
    break;

  case 205:
    ret="Reset Content";
    break;

  case 206:
    ret="Partial Content";
    break;

  case 300:
    ret="Multiple Choices";
    break;

  case 301:
    ret="Moved Permanently";
    break;

  case 302:
    ret="Found";
    break;

  case 303:
    ret="See Other";
    break;

  case 304:
    ret="Not Modified";
    break;

  case 305:
    ret="Use Proxy";
    break;

  case 306:
    ret="(Unused)";
    break;

  case 307:
    ret="Temporary Redirect";
    break;

  case 400:
    ret="Bad Request";
    break;

  case 401:
    ret="Unauthorized";
    break;

  case 402:
    ret="Payment Required";
    break;

  case 403:
    ret="Forbidden";
    break;

  case 404:
    ret="Not Found";
    break;

  case 405:
    ret="Method Not Allowed";
    break;

  case 406:
    ret="Not Acceptable";
    break;

  case 407:
    ret="Proxy Authentication Required";
    break;

  case 408:
    ret="Request Timeout";
    break;

  case 409:
    ret="Conflict";
    break;

  case 410:
    ret="Gone";
    break;

  case 411:
    ret="Length Required";
    break;

  case 412:
    ret="Precondition Failed";
    break;

  case 413:
    ret="Request Entity Too Large";
    break;

  case 414:
    ret="Request-URI Too Long";
    break;

  case 415:
    ret="Unsupported Media Type";
    break;

  case 416:
    ret="Requested Range Not Satisfiable";
    break;

  case 417:
    ret="Expectation Failed";
    break;

  case 500:
    ret="Internal Server Error";
    break;

  case 501:
    ret="Not Implemented";
    break;

  case 502:
    ret="Bad Gateway";
    break;

  case 503:
    ret="Service Unavailable";
    break;

  case 504:
    ret="Gateway Timeout";
    break;

  case 505:
    ret="HTTP Version Not Supported";
    break;
  }
  return ret;
}


int HttpConnection::timezoneOffset()
{
  time_t t=time(&t);
  struct tm *tm=localtime(&t);
  time_t local_time=3600*tm->tm_hour+60*tm->tm_min+tm->tm_sec;
  tm=gmtime(&t);
  time_t gmt_time=3600*tm->tm_hour+60*tm->tm_min+tm->tm_sec;

  return gmt_time-local_time;
}


QString HttpConnection::datetimeStamp(const QDateTime &dt)
{
  return dt.addSecs(HttpConnection::timezoneOffset()).
    toString("ddd, dd MMM yyyy hh:mm:ss")+" GMT";
}
