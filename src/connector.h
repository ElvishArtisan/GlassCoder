// connector.h
//
// Abstract base class for streaming server source connections.
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

#ifndef CONNECTOR_H
#define CONNECTOR_H

#include <stdint.h>

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QTimer>
#include <QtNetwork/QTcpSocket>

#define RINGBUFFER_SERVICE_INTERVAL 50

class Connector : public QObject
{
  Q_OBJECT;
 public:
  enum ServerType {Shoutcast1Server=0,Icecast2Server=1,Shoutcast2Server=2,
		   HlsServer=3};
  Connector(QObject *parent=0);
  ~Connector();
  virtual Connector::ServerType serverType() const=0;
  QString serverUsername() const;
  void setServerUsername(const QString &str);
  QString serverPassword() const;
  void setServerPassword(const QString &str);
  QString serverMountpoint() const;
  void setServerMountpoint(const QString &str);
  QString contentType() const;
  void setContentType(const QString &str);
  unsigned audioChannels() const;
  void setAudioChannels(unsigned chans);
  unsigned audioSamplerate() const;
  void setAudioSamplerate(unsigned rate);
  unsigned audioBitrate() const;
  void setAudioBitrate(unsigned rate);
  QString streamName() const;
  void setStreamName(const QString &str);
  QString streamDescription() const;
  void setStreamDescription(const QString &str);
  QString streamUrl() const;
  void setStreamUrl(const QString &str);
  QString streamIrc() const;
  void setStreamIrc(const QString &str);
  QString streamIcq() const;
  void setStreamIcq(const QString &str);
  QString streamAim() const;
  void setStreamAim(const QString &str);
  QString streamGenre() const;
  void setStreamGenre(const QString &str);
  bool streamPublic() const;
  void setStreamPublic(bool state);
  virtual void connectToServer(const QString &hostname,uint16_t port);
  virtual int64_t writeData(int frames,const unsigned char *data,int64_t len);
  static QString serverTypeText(Connector::ServerType);
  static QString urlEncode(const QString &str);
  static QString urlDecode(const QString &str);
  static QString base64Encode(const QString &str);
  static QString base64Decode(const QString &str,bool *ok=NULL);

 signals:
  void dataRequested(Connector *conn);
  void error(QAbstractSocket::SocketError err);

 private slots:
  void dataTimeoutData();
  void watchdogTimeoutData();

 protected:
  void setConnected(bool state);
  void setError(QAbstractSocket::SocketError err);
  virtual void connectToHostConnector(const QString &hostname,uint16_t port)=0;
  virtual void disconnectFromHostConnector()=0;
  virtual int64_t writeDataConnector(int frames,const unsigned char *data,
				     int64_t len)=0;
  QString hostHostname() const;
  uint16_t hostPort() const;

 private:
  QString conn_server_username;
  QString conn_server_password;
  QString conn_server_mountpoint;
  QString conn_content_type;
  unsigned conn_audio_channels;
  unsigned conn_audio_samplerate;
  unsigned conn_audio_bitrate;
  QString conn_stream_name;
  QString conn_stream_description;
  QString conn_stream_url;
  QString conn_stream_irc;
  QString conn_stream_icq;
  QString conn_stream_aim;
  QString conn_stream_genre;
  bool conn_stream_public;
  QTimer *conn_data_timer;
  QTimer *conn_watchdog_timer;
  bool conn_watchdog_active;
  bool conn_connected;
  QString conn_host_hostname;
  uint16_t conn_host_port;
};


#endif  // CONNECTOR_H
