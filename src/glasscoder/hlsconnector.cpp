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
#include <time.h>
#include <unistd.h>

#include "hlsconnector.h"
#include "logging.h"

HlsConnector::HlsConnector(bool is_top,FileConveyor *conv,QObject *parent)
  : Connector(parent)
{
  hls_is_top=is_top;
  hls_conveyor=conv;
  hls_sequence_head=0;
  hls_sequence_back=0;
  hls_media_frames=0;
  hls_total_media_frames=0;

  //
  // Create working directory
  //
  char tempdir[PATH_MAX];

  strncpy(tempdir,"/tmp",PATH_MAX);
  if(getenv("TEMP")!=NULL) {
    strncpy(tempdir,getenv("TEMP"),PATH_MAX);
  }
  strncat(tempdir,"/glasscoder-XXXXXX",PATH_MAX-strlen(tempdir));
  if(mkdtemp(tempdir)==NULL) {
    Log(LOG_ERR,QString().sprintf("unable to create temporary directory [%s]",
				  strerror(errno)));
    exit(256);
  }
  hls_temp_dir=new QDir(tempdir);
  Log(LOG_INFO,QString().sprintf("using working directory \"%s\"",
				 (const char *)hls_temp_dir->path().toUtf8()));

  //
  // File Conveyor Error Logging
  //
  connect(hls_conveyor,SIGNAL(eventFinished(const ConveyorEvent &,int,int,
					    const QStringList &)),
	  this,SLOT(conveyorEventFinished(const ConveyorEvent &,int,int,
					  const QStringList &)));
  connect(hls_conveyor,
	  SIGNAL(error(const ConveyorEvent &,QProcess::ProcessError,
		       const QStringList &)),
	  this,
	  SLOT(conveyorError(const ConveyorEvent &,QProcess::ProcessError,
			     const QStringList &)));
}


HlsConnector::~HlsConnector()
{
  QStringList files=hls_temp_dir->entryList(QDir::Files);
  for(int i=0;i<files.size();i++) {
    unlink((hls_temp_dir->path()+"/"+files[i]).toUtf8());
  }
  rmdir(hls_temp_dir->path().toAscii());
  delete hls_temp_dir;
}


Connector::ServerType HlsConnector::serverType() const
{
  return Connector::HlsServer;
}


void HlsConnector::connectToHostConnector(const QString &hostname,uint16_t port)
{
  //
  // Calculate publish point info
  //
  QStringList f0=serverMountpoint().split("/",QString::SkipEmptyParts);
  hls_put_basename=f0[f0.size()-1];
  hls_put_basestamp=QString().sprintf("%ld",time(NULL));
  QStringList f1=hls_put_basename.split(".");
  if((f1[f1.size()-1]!="m3u8")&&(f1[f1.size()-1]!="m3u")) {
    hls_put_basename+=".m3u8";
  }
  hls_put_directory="";
  for(int i=0;i<f0.size()-1;i++) {
    hls_put_directory+="/";
    hls_put_directory+=f0[i];
  }

  //
  // Create playlist file
  //
  hls_playlist_filename=hls_temp_dir->path()+"/"+hls_put_basename;

  if(hls_is_top) {
    WriteTopPlaylistFile();
    hls_conveyor->push(this,hls_playlist_filename,
		       "http://"+hostHostname()+hls_put_directory+"/",
		       ConveyorEvent::PutMethod);
    unlink(hls_playlist_filename.toUtf8());
  }
  else {
    //
    // Create initial media file
    //
    hls_media_filename=GetMediaFilename(hls_sequence_back);
    if((hls_media_handle=
	fopen((hls_temp_dir->path()+"/"+hls_media_filename).toUtf8(),"w"))==
       NULL) {
      Log(LOG_WARNING,
	  QString().sprintf("unable to write media data to \"%s\" [%s]",
	     (const char *)(hls_temp_dir->path()+"/"+
			    hls_media_filename).toUtf8(),strerror(errno)));
    }

    //
    // Write ID3 tag
    //
#ifndef HLS_OMIT_ID3_TIMESTAMPS
    uint8_t id3_header[HLS_ID3_HEADER_SIZE];
    hls_total_media_frames=HLS_SEGMENT_SIZE*audioSamplerate();
    GetStreamTimestamp(id3_header,hls_total_media_frames);
    fwrite(id3_header,1,HLS_ID3_HEADER_SIZE,hls_media_handle);
#endif  // HLS_OMIT_ID3_TIMESTAMPS
  }

  setConnected(true);
  emit unmuteRequested();
}


void HlsConnector::disconnectFromHostConnector()
{
}


int64_t HlsConnector::writeDataConnector(int frames,const unsigned char *data,
					 int64_t len)
{
  if((hls_media_frames+(uint64_t)frames)>(HLS_SEGMENT_SIZE*audioSamplerate())) {
    RotateMediaFile();
  }
  hls_media_frames+=(uint64_t)frames;
  hls_total_media_frames+=(uint64_t)frames;
  return fwrite(data,len,1,hls_media_handle);
}


void HlsConnector::conveyorEventFinished(const ConveyorEvent &evt,int exit_code,
					 int resp_code,const QStringList &args)
{
  if(evt.originator()==this) {
    //
    // Exit code handler
    //
    if(exit_code!=0) {
      setConnected(false);
      if(global_log_verbose) {
	Log(LOG_WARNING,
	    QString().sprintf("curl(1) error: %s, cmd: \"curl %s\"",
		   (const char *)Connector::curlStrError(exit_code).toUtf8(),
		   (const char *)args.join(" ").toUtf8()));
      }
      else {
	Log(LOG_WARNING,QString().sprintf("CURL error: %s",
		   (const char *)Connector::curlStrError(exit_code).toUtf8()));
      }
    }
    else {
      //
      // Reponse code handler
      //
      if((resp_code<200)||(resp_code>299)) {
	setConnected(false);
	if(global_log_verbose) {
	  Log(LOG_WARNING,"curl(1) response error: "+
	      Connector::httpStrError(resp_code)+
	      ", cmd: \"curl "+args.join(" ")+"\"");
	}
	else {
	  Log(LOG_WARNING,"curl(1) response error: "+
	      Connector::httpStrError(resp_code));
	}
      }
      else {
	setConnected(true);
      }
    }
  }
}


void HlsConnector::conveyorError(const ConveyorEvent &evt,
				 QProcess::ProcessError err,
				 const QStringList &args)
{
  Log(LOG_ERR,
      QString().sprintf("curl(1) process error: %d, cmd: \"curl %s\"",err,
			(const char *)args.join(" ").toUtf8()));
  setConnected(false);
  exit(256);
}


void HlsConnector::RotateMediaFile()
{
  //
  // Update working files
  //
  fclose(hls_media_handle);
  hls_media_datetimes[hls_sequence_back]=
    QDateTime(QDate::currentDate(),QTime::currentTime());
  hls_media_durations[hls_sequence_back]=
    (double)hls_media_frames/(double)audioSamplerate();
  if((hls_sequence_back-hls_sequence_head)>=HLS_MINIMUM_SEGMENT_QUAN) {
    // Schedule garbage collection
    hls_media_killtimes[hls_sequence_head++]=hls_total_media_frames+
      (HLS_MINIMUM_SEGMENT_QUAN+1)*HLS_SEGMENT_SIZE*audioSamplerate();
  }
  WritePlaylistFile();

  //
  // HTTP Uploads
  //
  hls_conveyor->push(this,hls_temp_dir->path()+"/"+hls_media_filename,
		     "http://"+hostHostname()+
		     QString().sprintf(":%u",hostPort())+
		     hls_put_directory+"/",ConveyorEvent::PutMethod);
  unlink((hls_temp_dir->path()+"/"+hls_media_filename).toUtf8());
  hls_conveyor->push(this,hls_playlist_filename,
		     "http://"+hostHostname()+hls_put_directory+"/",
		     ConveyorEvent::PutMethod);
  unlink(hls_playlist_filename.toUtf8());

  //
  // Take out the trash
  //
  std::map<int,uint64_t>::iterator ci=hls_media_killtimes.begin();
  while(ci!=hls_media_killtimes.end()) {
    if(ci->second<hls_total_media_frames) {
      std::map<int,double>::iterator cj=hls_media_durations.begin();
      while(cj!=hls_media_durations.end()) {
	if(cj->first==ci->first) {
	  hls_media_durations.erase(cj++);
	}
	else {
	  ++cj;
	}
      }
      std::map<int,QDateTime>::iterator dj=hls_media_datetimes.begin();
      while(dj!=hls_media_datetimes.end()) {
	if(dj->first==ci->first) {
	  hls_media_datetimes.erase(dj++);
	}
	else {
	  ++dj;
	}
      }
      hls_conveyor->push(this,"","http://"+hostHostname()+hls_put_directory+"/"+
			 GetMediaFilename(ci->first),
			 ConveyorEvent::DeleteMethod);
      hls_media_killtimes.erase(ci++);
    }
    else {
      ++ci;
    }
  }

  //
  // Initialize for next segment
  //
  hls_sequence_back++;
  hls_media_frames=0;
  //hls_media_killname=hls_media_filename;
  hls_media_filename=GetMediaFilename(hls_sequence_back);
  if((hls_media_handle=
      fopen((hls_temp_dir->path()+"/"+hls_media_filename).toUtf8(),"w"))==
     NULL) {
    Log(LOG_WARNING,
	QString().sprintf("unable to write media data to \"%s\" [%s]",
	   (const char *)(hls_temp_dir->path()+"/"+hls_media_filename).toUtf8(),
			  strerror(errno)));
  }
#ifndef HLS_OMIT_ID3_TIMESTAMPS
  uint8_t id3_header[HLS_ID3_HEADER_SIZE];
  GetStreamTimestamp(id3_header,hls_total_media_frames);
  fwrite(id3_header,1,HLS_ID3_HEADER_SIZE,hls_media_handle);
#endif  // HLS_OMIT_ID3_TIMESTAMPS
}


void HlsConnector::WritePlaylistFile()
{
  FILE *f=NULL;

  if((f=fopen(hls_playlist_filename.toUtf8(),"w"))==NULL) {
    Log(LOG_ERR,
	QString().sprintf("unable to write playlist data to \"%s\" [%s]",
	   (const char *)(hls_temp_dir->path()+"/"+
			  hls_playlist_filename).toUtf8(),strerror(errno)));
    exit(256);
  }
  fprintf(f,"#EXTM3U\n");
  fprintf(f,"#EXT-X-TARGETDURATION:%d\n",HLS_SEGMENT_SIZE);
  fprintf(f,"#EXT-X-VERSION:%d\n",HLS_VERSION);
  fprintf(f,"#EXT-X-MEDIA-SEQUENCE:%d\n",hls_sequence_head);
  for(int i=hls_sequence_head;i<=hls_sequence_back;i++) {
    fprintf(f,"#EXT-X-PROGRAM-DATE-TIME:%s%s\n",(const char *)
	    hls_media_datetimes[i].addSecs(streamTimestampOffset()).
	    toString("yyyy-MM-ddThh:mm:ss.zzz").toUtf8(),
	    (const char *)Connector::timezoneOffset().toUtf8());
    fprintf(f,"#EXTINF:%7.5lf,\n%s\n",
    	    hls_media_durations[i],(const char *)GetMediaFilename(i).toUtf8());
  }
  fclose(f);
}


void HlsConnector::WriteTopPlaylistFile()
{
  FILE *f=NULL;

  if((f=fopen(hls_playlist_filename.toUtf8(),"w"))==NULL) {
    Log(LOG_ERR,
	QString().sprintf("unable to write playlist data to \"%s\" [%s]",
	   (const char *)(hls_temp_dir->path()+"/"+
			  hls_playlist_filename).toUtf8(),strerror(errno)));
    exit(256);
  }
  fprintf(f,"#EXTM3U\n");
  //fprintf(f,"#EXT-X-VERSION:%d\n",HLS_VERSION);
  for(unsigned i=0;i<audioBitrates()->size();i++) {
    QString str=
      QString().sprintf("#EXT-X-STREAM-INF:PROGRAM-ID=1,BANDWIDTH=%u",
			1000*audioBitrates()->at(i));
    if(!formatIdentifier().isEmpty()) {
      str+=",CODECS=\""+formatIdentifier()+"\"";
    }
    fprintf(f,"%s\n",(const char *)str.toUtf8());
    fprintf(f,"%s\n",(const char *)Connector::basePart(Connector::subMountpointName(serverMountpoint(),audioBitrates()->at(i))).toUtf8());
  }
  fclose(f);
}


QString HlsConnector::GetMediaFilename(int seqno)
{
  return hls_put_basename+"-"+hls_put_basestamp+"-"+
    QString().sprintf("%d.",seqno)+extension();
}


void HlsConnector::GetStreamTimestamp(uint8_t *bytes,uint64_t frames)
{
  //
  // ID3 PRIV tag (see HTTP Live Streaming Draft sec. 4)
  //
  static uint8_t id3_header[HLS_ID3_HEADER_SIZE]= 
    {0x49,0x44,0x33,0x04,0x00,0x00,0x00,0x00,
     0x00,0x3F,0x50,0x52,0x49,0x56,0x00,0x00,
     0x00,0x35,0x00,0x00,0x63,0x6F,0x6D,0x2E,
     0x61,0x70,0x70,0x6C,0x65,0x2E,0x73,0x74,
     0x72,0x65,0x61,0x6D,0x69,0x6E,0x67,0x2E,
     0x74,0x72,0x61,0x6E,0x73,0x70,0x6F,0x72,
     0x74,0x53,0x74,0x72,0x65,0x61,0x6D,0x54,
     0x69,0x6D,0x65,0x73,0x74,0x61,0x6D,0x70,
     0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
     0x00};

  memcpy(bytes,id3_header,HLS_ID3_HEADER_SIZE);
  
  //
  // MPEG-2 timestamps are expressed in 1/90000ths of a second
  //
  uint64_t stamp=(uint64_t)(90000.0*(double)frames/(double)audioSamplerate())&
    (0x1FFFFFFFF);
  for(int i=0;i<8;i++) {  // Use network byte order!
    bytes[(HLS_ID3_HEADER_SIZE-8)+i]=0xFF&(stamp>>(56-8*i));
  }
}
