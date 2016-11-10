// socketserver.h
//
// Receive TCP socket connections via a UNIX socket pipe
//
//   (C) Copyright 2016 Fred Gleason <fredg@paravelsystems.com>
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

#ifndef SOCKETSERVER_H
#define SOCKETSERVER_H

#include <queue>

#include <QObject>
#include <QSocketNotifier>
#include <QTcpSocket>

#define SOCKETSERVER_MAX_PENDING_CONNECTIONS 5

class SocketServer : public QObject
{
  Q_OBJECT;
 public:
  SocketServer(QObject *parent=0);
  ~SocketServer();
  QString path() const;
  int maxPendingConnections() const;
  QTcpSocket *nextPendingConnection();
  bool listen(const QString &path);

 signals:
  void newConnection();

 private slots:
  void notifiedData(int sock);

 private:
  int GetDescriptor(int unix_sock);
  QString server_path;
  QSocketNotifier *server_notifier;
  int server_socket;
  std::queue<QTcpSocket *> server_connections;
};


#endif  // SOCKETSERVER_H
