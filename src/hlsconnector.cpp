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
    syslog(LOG_ERR,"unable to create temporary directory [%s]",
	   strerror(errno));
    exit(256);
  }
  hls_temp_dir=new QDir(tempdir);
  syslog(LOG_DEBUG,"HlsConnector: using temporary directory: %s",
	 (const char *)hls_temp_dir->path().toUtf8());

  //
  // Garbage Collector Timers
  //
  hls_put_garbage_timer=new QTimer(this);
  hls_put_garbage_timer->setSingleShot(true);
  connect(hls_put_garbage_timer,SIGNAL(timeout()),
	  this,SLOT(putCollectGarbageData()));

  hls_delete_garbage_timer=new QTimer(this);
  hls_delete_garbage_timer->setSingleShot(true);
  connect(hls_delete_garbage_timer,SIGNAL(timeout()),
	  this,SLOT(deleteCollectGarbageData()));
}


HlsConnector::~HlsConnector()
{
  delete hls_delete_garbage_timer;
  delete hls_put_garbage_timer;
  rmdir(hls_temp_dir->path().toAscii());
  delete hls_temp_dir;
}


Connector::ServerType HlsConnector::serverType() const
{
  return Connector::HlsServer;
}


void HlsConnector::stop()
{
  //
  // Clean up the publish point
  //
  // The Playlist File
  //
  hls_stop_args.push_back("-X");
  hls_stop_args.push_back("DELETE");
  hls_stop_args.push_back("http://"+
			  hostHostname()+QString().sprintf(":%u",hostPort())+
			  hls_put_directory+"/"+hls_put_basename);

  //
  // Current Media Segments
  //
  for(int i=hls_sequence_head;i<=hls_sequence_back;i++) {
    hls_stop_args.push_back("-X");
    hls_stop_args.push_back("DELETE");
    hls_stop_args.push_back("http://"+hostHostname()+hls_put_directory+"/"+
			    GetMediaFilename(i));
  }

  //
  // Expired Media Segments
  //
  std::map<int,uint64_t>::iterator ci=hls_media_killtimes.begin();
  while(ci!=hls_media_killtimes.end()) {
    hls_stop_args.push_back("-X");
    hls_stop_args.push_back("DELETE");
    hls_stop_args.
      push_back("http://"+hostHostname()+hls_put_directory+"/"+
		GetMediaFilename(ci->first));
    hls_media_killtimes.erase(ci++);
  }

  //
  // Run 'em
  //
  hls_stop_process=new QProcess(this);
  connect(hls_stop_process,SIGNAL(error(QProcess::ProcessError)),
	  this,SLOT(stopErrorData(QProcess::ProcessError)));
  connect(hls_stop_process,SIGNAL(finished(int,QProcess::ExitStatus)),
	  this,SLOT(stopFinishedData(int,QProcess::ExitStatus)));
  hls_stop_process->start("curl",hls_stop_args);
}


void HlsConnector::connectToHostConnector(const QString &hostname,uint16_t port)
{
  //
  // Calculate publish point info
  //
  QStringList f0=serverMountpoint().split("/",QString::SkipEmptyParts);
  hls_put_basename=f0[f0.size()-1];
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

  //
  // Create initial media file
  //
  hls_media_filename=GetMediaFilename(hls_sequence_back);
  if((hls_media_handle=
      fopen((hls_temp_dir->path()+"/"+hls_media_filename).toUtf8(),"w"))==
     NULL) {
    syslog(LOG_WARNING,"unable to write media data to \"%s\" [%s]",
	   (const char *)(hls_temp_dir->path()+"/"+hls_media_filename).toUtf8(),
	   strerror(errno));
  }

  //
  // Write ID3 tag
  //
  uint8_t id3_header[HLS_ID3_HEADER_SIZE];
  hls_total_media_frames=HLS_SEGMENT_SIZE*audioSamplerate();
  GetStreamTimestamp(id3_header,hls_total_media_frames);
  fwrite(id3_header,1,HLS_ID3_HEADER_SIZE,hls_media_handle);

  setConnected(true);
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


void HlsConnector::putErrorData(QProcess::ProcessError err)
{
  syslog(LOG_ERR,"curl(1) process error: %d, cmd: \"curl %s\"",err,
	 (const char *)hls_put_args.join(" ").toUtf8());

  exit(256);
}


void HlsConnector::putFinishedData(int exit_code,
				   QProcess::ExitStatus exit_status)
{
  if(exit_status==QProcess::CrashExit) {
    syslog(LOG_ERR,"curl(1) process crashed, cmd: \"curl %s\"",
	   (const char *)hls_put_args.join(" ").toUtf8());
    exit(256);
  }
  if(exit_code!=0) {
    syslog(LOG_WARNING,"curl(1) returned exit code: %d, cmd: \"curl %s\"",
	   exit_code,(const char *)hls_put_args.join(" ").toUtf8());
  }
  hls_temp_dir->remove(hls_media_killname);
  hls_put_garbage_timer->start(0);
}


void HlsConnector::putCollectGarbageData()
{
  delete hls_put_process;
  hls_put_process=NULL;
}


void HlsConnector::deleteErrorData(QProcess::ProcessError err)
{
  syslog(LOG_ERR,"curl(1) process error: %d, cmd: \"curl %s\"",err,
	 (const char *)hls_delete_args.join(" ").toUtf8());

  exit(256);
}


void HlsConnector::deleteFinishedData(int exit_code,
				      QProcess::ExitStatus exit_status)
{
  if(exit_status==QProcess::CrashExit) {
    syslog(LOG_ERR,"curl(1) process crashed, cmd: \"curl %s\"",
	   (const char *)hls_delete_args.join(" ").toUtf8());
    exit(256);
  }
  if(exit_code!=0) {
    syslog(LOG_WARNING,"curl(1) returned exit code: %d, cmd: \"curl %s\"",
	   exit_code,(const char *)hls_delete_args.join(" ").toUtf8());
  }
  hls_delete_garbage_timer->start(0);
}


void HlsConnector::deleteCollectGarbageData()
{
  delete hls_delete_process;
  hls_delete_process=NULL;
}


void HlsConnector::stopErrorData(QProcess::ProcessError err)
{
  syslog(LOG_ERR,"curl(1) process error: %d, cmd: \"curl %s\"",err,
	 (const char *)hls_stop_args.join(" ").toUtf8());

  emit stopped();
}


void HlsConnector::stopFinishedData(int exit_code,QProcess::ExitStatus exit_status)
{
  if(exit_status==QProcess::CrashExit) {
    syslog(LOG_ERR,"curl(1) process crashed, cmd: \"curl %s\"",
	   (const char *)hls_stop_args.join(" ").toUtf8());
    exit(256);
  }
  if(exit_code!=0) {
    syslog(LOG_WARNING,"curl(1) returned exit code: %d, cmd: \"curl %s\"",
	   exit_code,(const char *)hls_stop_args.join(" ").toUtf8());
  }

  //
  // Remove temporary directory
  //
  QStringList files=hls_temp_dir->entryList(QDir::Files|QDir::NoDotAndDotDot);
  for(int i=0;i<files.size();i++) {
    hls_temp_dir->remove(files[i]);
  }
  rmdir(hls_temp_dir->path().toUtf8());

  emit stopped();
}


void HlsConnector::RotateMediaFile()
{
  //
  // Update working files
  //
  fclose(hls_media_handle);
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
  if(hls_put_process==NULL) {
    hls_put_args.clear();
    hls_put_args.push_back("-T");
    hls_put_args.push_back(hls_temp_dir->path()+"/"+hls_media_filename);
    hls_put_args.
      push_back("http://"+hostHostname()+QString().sprintf(":%u",hostPort())+
		hls_put_directory+"/");
    hls_put_args.push_back("-T");
    hls_put_args.push_back(hls_playlist_filename);
    hls_put_args.push_back("http://"+hostHostname()+hls_put_directory+"/");
    hls_put_process=new QProcess(this);
    connect(hls_put_process,SIGNAL(error(QProcess::ProcessError)),
	    this,SLOT(putErrorData(QProcess::ProcessError)));
    connect(hls_put_process,SIGNAL(finished(int,QProcess::ExitStatus)),
	    this,SLOT(putFinishedData(int,QProcess::ExitStatus)));
    hls_put_process->start("curl",hls_put_args);
  }
  else {
    syslog(LOG_WARNING,"curl(1) command overrun");
  }

  //
  // Take out the trash
  //
  if(hls_delete_process==NULL) {
    hls_delete_args.clear();
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
	hls_delete_args.push_back("-X");
	hls_delete_args.push_back("DELETE");
	hls_delete_args.
	  push_back("http://"+hostHostname()+hls_put_directory+"/"+
		    GetMediaFilename(ci->first));
	hls_media_killtimes.erase(ci++);
      }
      else {
	++ci;
      }
    }
    if(hls_delete_args.size()>0) {
      hls_delete_process=new QProcess(this);
      connect(hls_delete_process,SIGNAL(error(QProcess::ProcessError)),
	      this,SLOT(deleteErrorData(QProcess::ProcessError)));
      connect(hls_delete_process,SIGNAL(finished(int,QProcess::ExitStatus)),
	      this,SLOT(deleteFinishedData(int,QProcess::ExitStatus)));
      hls_delete_process->start("curl",hls_delete_args);
    }
  }

  //
  // Initialize for next segment
  //
  hls_sequence_back++;
  hls_media_frames=0;
  hls_media_killname=hls_media_filename;
  hls_media_filename=GetMediaFilename(hls_sequence_back);
  if((hls_media_handle=
      fopen((hls_temp_dir->path()+"/"+hls_media_filename).toUtf8(),"w"))==
     NULL) {
    syslog(LOG_WARNING,"unable to write media data to \"%s\" [%s]",
	   (const char *)(hls_temp_dir->path()+"/"+hls_media_filename).toUtf8(),
	   strerror(errno));
  }
  uint8_t id3_header[HLS_ID3_HEADER_SIZE];
  GetStreamTimestamp(id3_header,hls_total_media_frames);
  fwrite(id3_header,1,HLS_ID3_HEADER_SIZE,hls_media_handle);
}


void HlsConnector::WritePlaylistFile()
{
  FILE *f=NULL;

  if((f=fopen(hls_playlist_filename.toUtf8(),"w"))==NULL) {
    syslog(LOG_ERR,"unable to write playlist data to \"%s\" [%s]",
	   (const char *)(hls_temp_dir->path()+"/"+
			  hls_playlist_filename).toUtf8(),strerror(errno));
    exit(256);
  }
  fprintf(f,"#EXTM3U\n");
  fprintf(f,"#EXT-X-TARGETDURATION:%d\n",HLS_SEGMENT_SIZE);
  fprintf(f,"#EXT-X-VERSION:%d\n",HLS_VERSION);
  fprintf(f,"#EXT-X-MEDIA-SEQUENCE:%d\n",hls_sequence_head);
  for(int i=hls_sequence_head;i<=hls_sequence_back;i++) {
    fprintf(f,"#EXTINF:%7.5lf,\n%s\n",
	    hls_media_durations[i],(const char *)GetMediaFilename(i).toUtf8());
  }
  fclose(f);
}


QString HlsConnector::GetMediaFilename(int seqno)
{
  return hls_put_basename+QString().sprintf("%d.",seqno)+extension();
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
