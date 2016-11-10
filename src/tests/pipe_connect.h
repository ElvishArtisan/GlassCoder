// pipe_connect.h
//
// Test a UNIX pipe connection
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

#ifndef PIPE_CONNECT_H
#define PIPE_CONNECT_H

#include <sys/socket.h>
#include <sys/un.h>

#include <QObject>
#include <QTcpServer>

#define PIPE_CONNECT_USAGE "--server-pipe=<filename> --port-number=<port>\n\n"

class MainObject : public QObject
{
 Q_OBJECT;
 public:
  MainObject(QObject *parent=0);

 private slots:
  void newConnectionData();

 private:
  void SendSocket(int sock) const;
  QString pipe_server_pipe;
  struct sockaddr_un pipe_sa;
  QTcpServer *pipe_server;
};


#endif  // PIPE_CONNECT_H
