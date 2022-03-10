// httpserver.h
//
// HTTP Server
//
// (C) Copyright 2016-2022 Fred Gleason <fredg@paravelsystems.com>
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

#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <stdint.h>

#include <map>

#include <QByteArray>
#include <QObject>
#include <QSignalMapper>
#include <QString>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>

#include "httpconnection.h"
#include "httpuser.h"
#include "socketmessage.h"

#define WEBSOCKET_VERSION 13
#define WEBSOCKET_MAGIC_STRING "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

class HttpServer : public QObject
{
  Q_OBJECT;
 public:
  HttpServer(QObject *parent=0);
  ~HttpServer();
  bool listen(uint16_t port);
  bool listen(const QHostAddress &iface,uint16_t port);
  void sendSocketMessage(int conn_id,SocketMessage::OpCode opcode,
			 const QByteArray &data);
  void sendSocketMessage(int conn_id,const QByteArray &data);
  void sendSocketMessage(int conn_id,const QString &str);
  void closeSocketConnection(int conn_id,uint16_t status,
			     const QByteArray &body=QByteArray());
  QStringList userRealms() const;
  QStringList userNames(const QString &realm);
  void addUser(const QString &realm,const QString &name,const QString &passwd);
  void removeUser(const QString &realm,const QString &name);
  bool loadUsers(const QString &filename);
  bool saveUsers(const QString &filename);
  void addStaticSource(const QString &uri,const QString &mimetype,
		       const QString &filename,const QString &realm="");
  void addCgiSource(const QString &uri,const QString &filename,
		    const QString &realm="");
  void addSocketSource(const QString &uri,const QString &proto,
		       const QString &realm="");

 signals:
  void newSocketConnection(int conn_id,const QString &uri,const QString &proto);
  void socketMessageReceived(int conn_id,SocketMessage *msg);
  void socketConnectionClosed(int conn_id,uint16_t stat_code,
			      const QByteArray &body);

 protected:
  virtual void requestReceived(HttpConnection *conn);
  virtual void getRequestReceived(HttpConnection *conn);
  virtual void postRequestReceived(HttpConnection *conn);
  virtual void headRequestReceived(HttpConnection *conn);
  virtual void putRequestReceived(HttpConnection *conn);
  virtual void deleteRequestReceived(HttpConnection *conn);
  virtual bool authenticateUser(const QString &realm,const QString &name,
				const QString &passwd);

 private slots:
  void newConnectionData();
  void readyReadData(int id);
  void disconnectedData(int id);
  void cgiFinishedData(int id);
  void garbageData();

 private:
  void ReadMethodLine(HttpConnection *conn);
  void ReadHeaders(HttpConnection *conn);
  void ReadBody(HttpConnection *conn);
  void ReadWebsocket(HttpConnection *conn);
  void ProcessRequest(HttpConnection *conn);
  void StartWebsocket(HttpConnection *conn,int n);
  void SendStaticSource(HttpConnection *conn,int n);
  void SendCgiSource(HttpConnection *conn,int n);
  bool IsCgiScript(const QString &uri) const;
  bool AuthenticateRealm(HttpConnection *conn,const QString &realm,
			 const QString &name,const QString &passwd);
  QByteArray GetWebsocketHandshake(const QString &key) const;
  QStringList http_static_filenames;
  QStringList http_static_uris;
  QStringList http_static_mimetypes;
  QStringList http_static_realms;
  QStringList http_cgi_filenames;
  QStringList http_cgi_uris;
  QStringList http_cgi_realms;
  QStringList http_socket_uris;
  QStringList http_socket_protocols;
  QStringList http_socket_realms;
  QTcpServer *http_server;
  QSignalMapper *http_read_mapper;
  QSignalMapper *http_disconnect_mapper;
  QSignalMapper *http_cgi_finished_mapper;
  std::vector<HttpConnection *> http_connections;
  QTimer *http_garbage_timer;
  std::map<QString,std::vector<HttpUser *> > http_users;
  bool http_dump_transactions;
};


#endif  // HTTPSERVER_H
