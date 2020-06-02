// socketserver.cpp
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

#include <stdio.h>

#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "socketserver.h"

SocketServer::SocketServer(QObject *parent)
  : QObject(parent)
{
  server_socket=-1;
  server_notifier=NULL;
}


SocketServer::~SocketServer()
{
  while(server_connections.size()>0) {
    delete server_connections.front();
    server_connections.pop();
  }
  if(server_socket>=0) {
    delete server_notifier;
    close(server_socket);
  }
  if(!server_path.isEmpty()) {
    unlink(server_path.toUtf8());
  }
}


QString SocketServer::path() const
{
  return server_path;
}


int SocketServer::maxPendingConnections() const
{
  return SOCKETSERVER_MAX_PENDING_CONNECTIONS;
}


QTcpSocket *SocketServer::nextPendingConnection()
{
  if(server_connections.size()==0) {
    return NULL;
  }
  QTcpSocket *sock=server_connections.front();
  server_connections.pop();
  return sock;
}


bool SocketServer::listen(const QString &path)
{
  struct sockaddr_un sa;

  if(server_socket>=0) {
    return false;
  }

  if((server_socket=socket(AF_UNIX,SOCK_STREAM,0))<0) {
    return false;
  }
  server_notifier=new QSocketNotifier(server_socket,QSocketNotifier::Read,this);
  connect(server_notifier,SIGNAL(activated(int)),this,SLOT(notifiedData(int)));
  memset(&sa,0,sizeof(sa));
  sa.sun_family=AF_UNIX;
  strncpy(sa.sun_path,path.toUtf8(),107);
  if(bind(server_socket,(const struct sockaddr *)(&sa),sizeof(sa))<0) {
    close(server_socket);
    server_socket=-1;
    delete server_notifier;
    server_notifier=NULL;
    return false;
  }
  server_path=path;
  if(::listen(server_socket,SOCKETSERVER_MAX_PENDING_CONNECTIONS)<0) {
    close(server_socket);
    server_socket=-1;
    delete server_notifier;
    server_notifier=NULL;
    return false;
  }

  return true;
}


void SocketServer::notifiedData(int sock)
{
  int unixsock=-1;
  int newsock=-1;

  if((unixsock=accept(sock,NULL,NULL))>=0) {
    if((newsock=GetDescriptor(unixsock))>=0) {
      server_connections.push(new QTcpSocket());
      server_connections.back()->setSocketDescriptor(newsock);
      emit newConnection();
    }
  }
}


int SocketServer::GetDescriptor(int unix_sock)
{
  char buf[1];
  struct msghdr msg;
  struct iovec iov[1];
  ssize_t n;
  char data[256];
  union {
    struct cmsghdr cm;
    char control[CMSG_SPACE(sizeof(int))];
  } control_un;
  struct cmsghdr *cmptr;

  //
  // Did The Operation Succeed?
  //
  if(recv(unix_sock,buf,1,MSG_NOSIGNAL)<=0) {
    return -1;
  }
  if(buf[0]!=1) {
    return -1;
  }

  //
  // Get Descriptor
  //
  memset(&msg,0,sizeof(msg));
  memset(&iov,0,sizeof(struct iovec));

  msg.msg_control=control_un.control;
  msg.msg_controllen=sizeof(control_un.control);
  iov[0].iov_base=data;
  iov[0].iov_len=256;
  msg.msg_iov=iov;
  msg.msg_iovlen=1;

  if((n=recvmsg(unix_sock,&msg,0))<=0) {
    close(unix_sock);
    return -1;
  }
  close(unix_sock);
  if((cmptr=CMSG_FIRSTHDR(&msg))!=NULL &&
     cmptr->cmsg_len==CMSG_LEN(sizeof(int))) {
    if(cmptr->cmsg_level!=SOL_SOCKET) {
      return -1;
    }
    if(cmptr->cmsg_type!=SCM_RIGHTS) {
      return -1;
    }
    return *((int *)CMSG_DATA(cmptr));
  }
  return -1;
}
