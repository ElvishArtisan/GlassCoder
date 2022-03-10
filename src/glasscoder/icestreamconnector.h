// icestreamconnector.h
//
// Glasscoder connector for an integrated IceCast server
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

#ifndef ICESTREAMCONNECTOR_H
#define ICESTREAMCONNECTOR_H

#include <QSignalMapper>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>

#include "connector.h"
#include "socketserver.h"

#define ICESTREAM_METADATA_INTERVAL 16000
#define ICESTREAM_CONNECTION_TIMEOUT 10000

class IceStream
{
 public:
  enum Type {New=0,Player=1,Updinfo=2};
  IceStream(QTcpSocket *sock,Type type=New);
  ~IceStream();
  QTcpSocket *socket() const;
  QTimer *timeoutTimer() const;
  Type type() const;
  void setType(Type type);
  bool isNegotiated() const;
  void setNegotiated();
  bool isAuthenticated() const;
  void setAuthenticated(bool state);
  QString streamTitle() const;
  void setStreamTitle(const QString &str);
  bool metadataEnabled() const;
  void setMetadataEnabled(bool state);
  int addMetadataBytes(int bytes);
  QString accum;

 private:
  unsigned ice_id;
  QTcpSocket *ice_socket;
  QTimer *ice_timeout_timer;
  Type ice_type;
  bool ice_is_negotiated;
  bool ice_is_authenticated;
  QString ice_stream_title;
  bool ice_metadata_enabled;
  int ice_metadata_bytes;
};




class IceStreamConnector : public Connector
{
  Q_OBJECT;
 public:
  IceStreamConnector(QObject *parent=0);
  ~IceStreamConnector();
  Connector::ServerType serverType() const;

 public slots:
  void setStreamPrologue(const QByteArray &data);
  void sendMetadata(MetaEvent *e);

 private slots:
  void newConnectionData();
  void newPipeConnectionData();
  void readyReadData(int id);
  void timeoutData(int id);
  void disconnectedData();
  void garbageData();

 protected:
  void startStopping();
  void connectToHostConnector(const QUrl &url);
  void disconnectFromHostConnector();
  int64_t writeDataConnector(int frames,const unsigned char *data,int64_t len);

 private:
  void SetMetadata(const QString &title);
  void SendHeader(IceStream *strm,const QString &hdr="") const;
  void ProcessHeader(IceStream *strm);
  void CloseConnection(IceStream *strm,int code,const QString &str,
		       const QStringList &hdrs=QStringList());
  void StartStream(IceStream *strm);
  int GetFreeStreamId();
  QTcpServer *iceserv_server;
  std::vector<IceStream *> iceserv_streams;
  QSignalMapper *iceserv_readyread_mapper;
  QSignalMapper *iceserv_timeout_mapper;
  QTimer *iceserv_garbage_timer;
  QByteArray iceserv_metadata;
  SocketServer *iceserv_socket_server;
  QByteArray iceserv_stream_prologue;
};


#endif  // ICESTREAMCONNECTOR_H
