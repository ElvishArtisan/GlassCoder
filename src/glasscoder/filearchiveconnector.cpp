// filearchiveconnector.cpp
//
// Source connector class for local file archives
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
#include <unistd.h>

#include <QStringList>

#include "filearchiveconnector.h"
#include "logging.h"

FileArchiveConnector::FileArchiveConnector(QObject *parent)
  : Connector(parent)
{
  archive_fd=-1;
  archive_hour=-1;

  archive_rotate_timer=new QTimer(this);
  archive_rotate_timer->setSingleShot(true);
  connect(archive_rotate_timer,SIGNAL(timeout()),this,SLOT(rotateFile()));
}


FileArchiveConnector::~FileArchiveConnector()
{
  delete archive_rotate_timer;
  if(archive_fd>=0) {
    close(archive_fd);
  }
}


FileArchiveConnector::ServerType FileArchiveConnector::serverType() const
{
  return Connector::FileServer;
}


void FileArchiveConnector::rotateFile()
{
  QDateTime now(QDate::currentDate(),QTime::currentTime());
  QString filename=GetFilename(now);

  if(!DidHourAdvance(now)) {  // We haven't crossed into new hour, holdoff
    archive_rotate_timer->start(100);
    return;
  }

  if(archive_fd>=0) {
    close(archive_fd);
    archive_fd=-1;
  }
  if((archive_fd=open(filename.toUtf8(),O_WRONLY|O_TRUNC|O_CREAT,
		   S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH))>=0) {
    setConnected(true);
    Log(LOG_DEBUG,"now writing to \""+filename+"\"");
  }
  else {
    Log(LOG_WARNING,("unable to open destination file \""+filename+
		     "\" ["+strerror(errno)+"]").toUtf8());
    setConnected(false);
  }
  archive_rotate_timer->
    start(now.time().msecsTo(QTime(now.time().hour(),59,59,999)));
}


void FileArchiveConnector::connectToHostConnector(const QString &hostname,
						  uint16_t port)
{
  //
  // Validate Mountpoint
  //
  if(serverMountpoint().right(1)=="/") {
    Log(LOG_ERR,"invalid --server-url");
    exit(256);
  }
  if((archive_fd=open(serverMountpoint().toUtf8(),O_WRONLY|O_TRUNC|O_CREAT,
		   S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH))<0) {
    Log(LOG_ERR,("unable to write to archive location \""+serverMountpoint()+
		 "\" ["+strerror(errno)+"]").toUtf8());
    exit(256);
  }
  close(archive_fd);
  archive_fd=-1;
  unlink(serverMountpoint().toUtf8());
  rotateFile();
}


void FileArchiveConnector::disconnectFromHostConnector()
{
  archive_rotate_timer->stop();
  close(archive_fd);
  archive_fd=-1;
}


int64_t FileArchiveConnector::writeDataConnector(int frames,
						 const unsigned char *data,
						 int64_t len)
{
  return write(archive_fd,data,len);
}


bool FileArchiveConnector::DidHourAdvance(const QDateTime &dt)
{
  if(dt.time().hour()==archive_hour) {
    return false;
  }
  archive_hour=dt.time().hour();
  return true;
}


QString FileArchiveConnector::GetFilename(const QDateTime &dt) const
{
  return serverMountpoint()+"-"+dt.toString(FILEARCHIVE_DATETIME_PATTERN)+"."+
    extension();
}
