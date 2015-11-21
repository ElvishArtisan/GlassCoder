// fileconnector.cpp
//
// Source connector class for local files
//
//   (C) Copyright 2015 Fred Gleason <fredg@paravelsystems.com>
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
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <QStringList>

#include "fileconnector.h"
#include "logging.h"

FileConnector::FileConnector(QObject *parent)
  : Connector(parent)
{
  file_fd=-1;
}


FileConnector::~FileConnector()
{
  if(file_fd>=0) {
    close(file_fd);
  }
}


FileConnector::ServerType FileConnector::serverType() const
{
  return Connector::FileServer;
}


void FileConnector::connectToHostConnector(const QString &hostname,uint16_t port)
{
  if((file_fd=open(serverMountpoint().toUtf8(),O_WRONLY|O_TRUNC|O_CREAT,
		   S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH))>=0) {
    setConnected(true);
  }
  else {
    Log(LOG_ERR,("unable to open destination file \""+serverMountpoint()+
		 "\" ["+strerror(errno)+"]").toUtf8());
    setConnected(false);
  }
}


void FileConnector::disconnectFromHostConnector()
{
  close(file_fd);
  file_fd=-1;
}


int64_t FileConnector::writeDataConnector(int frames,const unsigned char *data,
					 int64_t len)
{
  return write(file_fd,data,len);
}
