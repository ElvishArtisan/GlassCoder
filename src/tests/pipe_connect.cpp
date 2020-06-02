// pipe_connect.cpp
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

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include <QCoreApplication>
#include <QTcpSocket>

#include "cmdswitch.h"
#include "pipe_connect.h"

MainObject::MainObject(QObject *parent)
  : QObject(parent)
{
  pipe_server_pipe="";
  unsigned port=0;
  bool ok=false;

  //
  // Get Arguments
  //
  CmdSwitch *cmd=new CmdSwitch("pipe_connect",PIPE_CONNECT_USAGE);
  for(unsigned i=0;i<cmd->keys();i++) {
    if(cmd->key(i)=="--server-pipe") {
      pipe_server_pipe=cmd->value(i);
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="--port") {
      port=cmd->value(i).toUInt(&ok);
      if((!ok)||(port>=65536)) {
	fprintf(stderr,"pipe_connect: invalid value for --port\n");
	exit(256);
      }
      cmd->setProcessed(i,true);
    }
  }
  if(port==0) {
    fprintf(stderr,"pipe_connect: you must specify a --port\n");
    exit(256);
  }
  if(pipe_server_pipe.isEmpty()) {
    fprintf(stderr,"pipe_connect: you must specify a --server-pipe\n");
    exit(256);
  }

  //
  // Initialize Sending UNIX Socket
  //
  memset(&pipe_sa,0,sizeof(pipe_sa));
  pipe_sa.sun_family=AF_UNIX;
  strncpy(pipe_sa.sun_path,pipe_server_pipe.toUtf8(),107);

  //
  // TCP Server
  //
  pipe_server=new QTcpServer(this);
  connect(pipe_server,SIGNAL(newConnection()),this,SLOT(newConnectionData()));
  if(!pipe_server->listen(QHostAddress::Any,port)) {
    fprintf(stderr,"pipe_connect: unable to listen to port %u\n",port);
    exit(256);
  }
}


void MainObject::newConnectionData()
{
  QTcpSocket *sock=pipe_server->nextPendingConnection();
  printf("processing connection from %s:%u...",
	 (const char *)sock->peerAddress().toString().toUtf8(),
	 0xFFFF&sock->peerPort());
  fflush(stdout);
  SendSocket(sock->socketDescriptor());
  printf("done.\n");
  delete sock;
}


void MainObject::SendSocket(int sock) const
{
  char buf[1]={1};
  struct msghdr msg;
  struct iovec iov[1];
  union {
    struct cmsghdr cm;
    char control[CMSG_SPACE(sizeof(int))];
  } control_un;
  struct cmsghdr *cmptr;
  int sendsock=-1;

  //
  // Open UNIX Socket Connection
  //
  if((sendsock=socket(AF_UNIX,SOCK_STREAM,0))<0) {
    fprintf(stderr,
	    "pipe_connect: unable to initialize UNIX socket at \"%s\" [%s]\n",
	    (const char *)pipe_server_pipe.toUtf8(),strerror(errno));
    exit(256);
  }
  if(::connect(sendsock,(const struct sockaddr *)(&pipe_sa),sizeof(pipe_sa))<0) {
    fprintf(stderr,"pipe_connect: unable to connect to \"%s\" [%s]\n",
	    (const char *)pipe_server_pipe.toUtf8(),strerror(errno));
    exit(256);
  }

  //
  // Send Socket Descriptor
  //
  send(sendsock,buf,1,MSG_NOSIGNAL);

  memset(&msg,0,sizeof(msg));
  memset(iov,0,sizeof(struct iovec));

  msg.msg_control=control_un.control;
  msg.msg_controllen=sizeof(control_un.control);
  cmptr=CMSG_FIRSTHDR(&msg);
  cmptr->cmsg_len=CMSG_LEN(sizeof(int));
  cmptr->cmsg_level=SOL_SOCKET;
  cmptr->cmsg_type=SCM_RIGHTS;
  *((int *) CMSG_DATA(cmptr))=sock;
  
  iov[0].iov_base=buf;
  iov[0].iov_len=1;
  msg.msg_iov=iov;
  msg.msg_iovlen=1;
  
  sendmsg(sendsock,&msg,MSG_NOSIGNAL);
  close(sock);

  //
  // Clean Up
  //
  close(sendsock);
}


int main(int argc,char *argv[])
{
  QCoreApplication a(argc,argv);

  new MainObject();
  return a.exec();
}
