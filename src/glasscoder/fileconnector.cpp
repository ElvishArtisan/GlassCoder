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
#include <unistd.h>
#include <arpa/inet.h>

#include <QStringList>

#include "fileconnector.h"
#include "logging.h"

FileConnector::FileConnector(QObject *parent)
  : Connector(parent)
{
  file_fd=-1;
  file_snd=NULL;
}


FileConnector::~FileConnector()
{
  if(file_snd!=NULL) {
    sf_close(file_snd);
  }
  if(file_fd>=0) {
    close(file_fd);
  }
}


FileConnector::ServerType FileConnector::serverType() const
{
  return Connector::FileServer;
}


void FileConnector::connectToHostConnector(const QUrl &url)
{
  SF_INFO sf;
  if(contentType()=="audio/x-wav") {
    memset(&sf,0,sizeof(sf));
    sf.samplerate=audioSamplerate();
    sf.channels=audioChannels();
    sf.format=SF_FORMAT_WAV|SF_FORMAT_PCM_16;
    if((file_snd=sf_open(serverMountpoint().toUtf8(),SFM_WRITE,&sf))!=NULL) {
      setConnected(true);
    }
    else {
      Log(LOG_ERR,("unable to open destination file \""+serverMountpoint()+
		   "\" ["+sf_strerror(file_snd)+"]").toUtf8());
      setConnected(false);
    }
  }
  else {
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
  emit unmuteRequested();
}


void FileConnector::disconnectFromHostConnector()
{
  if(file_snd==NULL) {
    close(file_fd);
    file_fd=-1;
  }
  else {
    sf_close(file_snd);
    file_snd=NULL;
  }
}


int64_t FileConnector::writeDataConnector(int frames,const unsigned char *data,
					 int64_t len)
{
  if(file_snd==NULL) {
    return write(file_fd,data,len);
  }
  short pcm[frames*audioChannels()];
  for(int i=0;i<frames*(int)audioChannels();i++) {
    pcm[i]=ntohs(((short *)data)[i]);
  }
  return sf_writef_short(file_snd,pcm,frames)*2*audioChannels();
}
