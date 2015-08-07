// hlsconnector.cpp
//
// HLS/HTTP streaming connector for GlassCoder
//
//   (C) Copyright 2014-2015 Fred Gleason <fredg@paravelsystems.com>
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
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#include "hlsconnector.h"

HlsConnector::HlsConnector(QObject *parent)
  : Connector(parent)
{
  hls_sequence_number=0;
  hls_media_frames=0;

  char tempdir[PATH_MAX];

  strncpy(tempdir,"/tmp",PATH_MAX);
  if(getenv("TEMP")!=NULL) {
    strncpy(tempdir,getenv("TEMP"),PATH_MAX);
  }
  strncat(tempdir,"/glasscoder-XXXXXX",PATH_MAX-strlen(tempdir));
  if(mkdtemp(tempdir)==NULL) {
    syslog(LOG_ERR,"unable to create temporary directory [%s]",
	   strerror(errno));
    exit(256);
  }
  hls_temp_dir=new QDir(tempdir);
}


HlsConnector::~HlsConnector()
{
  rmdir(hls_temp_dir->path().toAscii());
  delete hls_temp_dir;
}


Connector::ServerType HlsConnector::serverType() const
{
  return Connector::HlsServer;
}


void HlsConnector::connectToServer(const QString &hostname,uint16_t port)
{
  //
  // Create playlist file
  //
  hls_playlist_filename=hls_temp_dir->path()+"/"+serverMountpoint()+".m3u8";
  printf("playlist: %s\n",(const char *)hls_playlist_filename.toAscii());
  if((hls_playlist_handle=fopen(hls_playlist_filename.toUtf8(),"w"))!=NULL) {
    fprintf(hls_playlist_handle,"#EXTM3U\n");
    fprintf(hls_playlist_handle,"#EXT-X-TARGETDURATION:%d\n",HLS_SEGMENT_SIZE);
    fprintf(hls_playlist_handle,"#EXT-X-VERSION:%d\n",HLS_VERSION);
    fprintf(hls_playlist_handle,"#EXT-X-MEDIA-SEQUENCE:0\n");
    fprintf(hls_playlist_handle,"#EXT-X-PLAYLIST-TYPE:VOD\n");
    fflush(hls_playlist_handle);
  }

  //
  // Create initial media file
  //
  hls_media_filename=
    QString().sprintf("fileSequence%d.mp3",hls_sequence_number);
  if((hls_media_handle=
      fopen((hls_temp_dir->path()+"/"+hls_media_filename).toUtf8(),"w"))==
     NULL) {
    syslog(LOG_ERR,"unable to write media data to \"%s\" [%s]",
	   (const char *)(hls_temp_dir->path()+"/"+hls_media_filename).toUtf8(),
	   strerror(errno));
    exit(0);
  }
  setConnected(true);
}


void HlsConnector::connectToHostConnector(const QString &hostname,uint16_t port)
{
}


void HlsConnector::disconnectFromHostConnector()
{
}


int64_t HlsConnector::writeDataConnector(int frames,const unsigned char *data,
					 int64_t len)
{
  if((hls_media_frames+frames)>(int)(HLS_SEGMENT_SIZE*audioSamplerate())) {
    RotateMediaFile();
  }
  hls_media_frames+=frames;
  return fwrite(data,len,1,hls_media_handle);
}


void HlsConnector::RotateMediaFile()
{
  fclose(hls_media_handle);
  fprintf(hls_playlist_handle,"EXTINF:%7.5lf,\n",
	  (double)hls_media_frames/(double)audioSamplerate());
  fprintf(hls_playlist_handle,"fileSequence%d.mp3\n",hls_sequence_number);
  fflush(hls_playlist_handle);
  //
  // FIXME: Signal upload layer here!!!
  //
  hls_sequence_number++;
  hls_media_frames=0;
  hls_media_filename=
    QString().sprintf("fileSequence%d.mp3",hls_sequence_number);
  if((hls_media_handle=
      fopen((hls_temp_dir->path()+"/"+hls_media_filename).toUtf8(),"w"))==
     NULL) {
    syslog(LOG_WARNING,"unable to write media data to \"%s\" [%s]",
	   (const char *)(hls_temp_dir->path()+"/"+hls_media_filename).toUtf8(),
	   strerror(errno));
  }
}
