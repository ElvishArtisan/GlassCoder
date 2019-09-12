// httpconnection.h
//
// HTTP connection state for HttpServer
//
// (C) Copyright 2016-2019 Fred Gleason <fredg@paravelsystems.com>
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

#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

#include <stdint.h>

#include <QByteArray>
#include <QObject>
#include <QProcess>
#include <QString>
#include <QTcpSocket>
#include <QTimer>

#include "socketmessage.h"

class HttpConnection : public QObject
{
  Q_OBJECT;
 public:
  enum Method {None=0,Get=1,Post=2,Head=3,Put=4,Delete=5};
  enum AuthType {AuthNone=0,AuthBasic=1,AuthDigest=2};
  HttpConnection(int id,QTcpSocket *sock,bool dump_trans,QObject *parent=0);
  ~HttpConnection();
  int id() const;
  unsigned majorProtocolVersion() const;
  unsigned minorProtocolVersion() const;
  bool protocolAtLeast(int major,int minor) const;
  bool setProtocolVersion(const QString &str);
  Method method() const;
  void setMethod(Method meth);
  bool isWebsocket() const;
  void setWebsocket(bool state);
  QString uri() const;
  void setUri(const QString &uri);
  QString hostName() const;
  uint16_t hostPort() const;
  bool setHost(const QString &str);
  AuthType authType() const;
  QString authName() const;
  QString authPassword() const;
  bool setAuthorization(const QString &str);
  int64_t contentLength() const;
  void setContentLength(int64_t len);
  QString contentType() const;
  void setContentType(const QString &mimetype);
  QString referrer() const;
  void setReferrer(const QString &str);
  QString subProtocol() const;
  void setSubProtocol(const QString &str);
  QString upgrade() const;
  void setUpgrade(const QString &str);
  QString userAgent() const;
  void setUserAgent(const QString &str);
  uint16_t socketCloseStatus() const;
  void setSocketCloseStatus(uint16_t);
  QByteArray socketCloseBody() const;
  void setSocketCloseBody(const QByteArray &data);
  QStringList headerNames() const;
  QStringList headerValues() const;
  QString headerValue(const QString &name) const;
  void addHeader(const QString &name,const QString &value);
  QByteArray body() const;
  void appendBody(const QByteArray &data);
  QString dump() const;
  QTcpSocket *socket();
  SocketMessage *appSocketMessage();
  SocketMessage *cntlSocketMessage();
  void startCgiScript(const QString &filename);
  void sendResponseHeader(int stat_code,const QString &mimetype="");
  void sendResponse(int stat_code,
		    const QStringList &hdr_names,const QStringList &hdr_values,
		    const QByteArray &body=QByteArray(),
		    const QString &mimetype="");
  void sendResponse(int stat_code,
		    const QByteArray &body=QByteArray(),
		    const QString &mimetype="");
  void sendError(int stat_code,const QString &msg="",
		 const QStringList &hdr_names=QStringList(),
		 const QStringList &hdr_values=QStringList());
  void sendHeader(const QString &name="",const QString &value="");
  int parseState() const;
  void setParseState(int state);
  static QString statusText(int stat_code);
  static int timezoneOffset();
  static QString datetimeStamp(const QDateTime &dt);

 signals:
  void cgiFinished();

 private slots:
  void cgiStartedData();
  void cgiReadyReadData();
  void cgiFinishedData(int exit_code,QProcess::ExitStatus status);
  void cgiErrorData(QProcess::ProcessError err);

 private:
  int conn_id;
  Method conn_method;
  bool conn_websocket;
  unsigned conn_major_protocol_version;
  unsigned conn_minor_protocol_version;
  QString conn_uri;
  QString conn_host_name;
  uint16_t conn_host_port;
  AuthType conn_auth_type;
  QString conn_auth_name;
  QString conn_auth_password;
  int64_t conn_content_length;
  QString conn_content_type;
  QString conn_referrer;
  QString conn_sub_protocol;
  QString conn_upgrade;
  QString conn_user_agent;
  uint16_t conn_socket_close_status;
  QByteArray conn_socket_close_body;
  QStringList conn_header_names;
  QStringList conn_header_values;
  QByteArray conn_body;
  QTcpSocket *conn_socket;
  SocketMessage *conn_app_socket_message;
  SocketMessage *conn_cntl_socket_message;
  QProcess *conn_cgi_process;
  QStringList conn_cgi_headers;
  bool conn_cgi_headers_active;
  int conn_parse_state;
  bool conn_dump_transactions;
};


#endif  // HTTPCONNECTION_H
