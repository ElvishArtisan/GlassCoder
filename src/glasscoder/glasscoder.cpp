// glasscoder.cpp
//
// glasscoder(1) Audio Encoder
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

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

#include <QCoreApplication>

#include "audiodevicefactory.h"
#include "cmdswitch.h"
#include "codecfactory.h"
#include "connectorfactory.h"
#include "glasscoder.h"

//
// Globals
//
bool glasscoder_exiting=false;

void SignalHandler(int signo)
{
  switch(signo) {
  case SIGTERM:
  case SIGINT:
    glasscoder_exiting=true;
    break;
  }
}


MainObject::MainObject(QObject *parent)
  : QObject(parent)
{
  bool ok=false;
  sir_exit_count=0;
  audio_device=DEFAULT_AUDIO_DEVICE;
  audio_channels=MAX_AUDIO_CHANNELS;
  audio_format=Codec::TypeVorbis;
  audio_quality=-1.0;
  audio_samplerate=DEFAULT_AUDIO_SAMPLERATE;
  server_type=Connector::Icecast2Server;
  server_hostname="";
  server_mountpoint="";
  server_password="";
  server_port=DEFAULT_SERVER_PORT;
  server_username=DEFAULT_SERVER_USERNAME;
  stream_genre="";
  stream_name="";
  stream_url="";
  stream_irc="";
  stream_icq="";
  stream_aim="";
  list_codecs=false;
  list_devices=false;
  unsigned num;

  CmdSwitch *cmd=
    new CmdSwitch(qApp->argc(),qApp->argv(),"glasscoder",GLASSCODER_USAGE);
  openlog("glasscoder",LOG_PERROR,LOG_DAEMON);
  for(unsigned i=0;i<cmd->keys();i++) {
    if(cmd->key(i)=="--audio-bitrate") {
      num=cmd->value(i).toUInt(&ok);
      if(ok) {
	audio_bitrate.push_back(num);
      }
      else {
	syslog(LOG_ERR,"invalid --audio-bitrate value");
	exit(256);
      }
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="--audio-channels") {
      audio_channels=cmd->value(i).toUInt(&ok);
      if((!ok)||(audio_channels==0)||(audio_channels>MAX_AUDIO_CHANNELS)) {
	syslog(LOG_ERR,"invalid --audio-channels value");
	exit(256);
      }
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="--audio-device") {
      for(unsigned j=0;j<AudioDevice::LastType;j++) {
	if(cmd->value(i).toLower()==
	   AudioDevice::optionKeyword((AudioDevice::DeviceType)j)) {
	  audio_device=(AudioDevice::DeviceType)j;
	  cmd->setProcessed(i,true);
	}
      }
    }
    if(cmd->key(i)=="--audio-format") {
      if(cmd->value(i).toLower()=="mp2") {
	audio_format=Codec::TypeMpegL2;
	cmd->setProcessed(i,true);
      }
      else {
	if(cmd->value(i).toLower()=="mp3") {
	  audio_format=Codec::TypeMpegL3;
	  cmd->setProcessed(i,true);
	}
	else {
	  if(cmd->value(i).toLower()=="aac") {
	    audio_format=Codec::TypeAac;
	    cmd->setProcessed(i,true);
	  }
	  else {
	    if(cmd->value(i).toLower()=="aacp") {
	      audio_format=Codec::TypeHeAac;
	      cmd->setProcessed(i,true);
	    }
	    else {
	      if(cmd->value(i).toLower()=="vorbis") {
		audio_format=Codec::TypeVorbis;
		cmd->setProcessed(i,true);
	      }
	      else {
		if(cmd->value(i).toLower()=="opus") {
		  audio_format=Codec::TypeOpus;
		  cmd->setProcessed(i,true);
		}
		else {
		  syslog(LOG_ERR,"unknown --audio-format value \"%s\"",
			 (const char *)cmd->value(i).toAscii());
		  exit(256);
		}
	      }
	    }
	  }
	}
      }
    }
    if(cmd->key(i)=="--audio-quality") {
      audio_quality=cmd->value(i).toDouble(&ok);
      if((!ok)||(audio_quality<0.0)||(audio_quality>1.0)) {
	syslog(LOG_ERR,"invalid --audio-quality value");
	exit(256);
      }
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="--audio-samplerate") {
      audio_samplerate=cmd->value(i).toUInt(&ok);
      if(!ok) {
	syslog(LOG_ERR,"invalid --audio-samplerate value");
	exit(256);
      }
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="--list-codecs") {
      list_codecs=true;
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="--list-devices") {
      list_devices=true;
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="--server-hostname") {
      server_hostname=cmd->value(i);
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="--server-mountpoint") {
      server_mountpoint=cmd->value(i);
      if(server_mountpoint.left(1)!="/") {
	server_mountpoint="/"+server_mountpoint;
      }
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="--server-password") {
      server_password=cmd->value(i);
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="--server-port") {
      server_port=cmd->value(i).toUInt(&ok);
      if((!ok)||(server_port==0)) {
	syslog(LOG_ERR,"invalid --shout-server-port value");
	exit(256);
      }
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="--server-type") {
      if(cmd->value(i).toLower()=="hls") {
	server_type=Connector::HlsServer;
	cmd->setProcessed(i,true);
      }
      else {
	if(cmd->value(i).toLower()=="icecast2") {
	  server_type=Connector::Icecast2Server;
	  cmd->setProcessed(i,true);
	}
	else {
	  if(cmd->value(i).toLower()=="shout1") {
	    server_type=Connector::Shoutcast1Server;
	    cmd->setProcessed(i,true);
	  }
	  else {
	    if(cmd->value(i).toLower()=="shout2") {
	      server_type=Connector::Shoutcast2Server;
	      cmd->setProcessed(i,true);
	    }
	    else {
	      syslog(LOG_ERR,"unknown server type \"%s\"",
		     (const char *)cmd->value(i).toAscii());
	      exit(256);
	    }
	  }
	}
      }
    }
    if(cmd->key(i)=="--server-username") {
      server_username=cmd->value(i);
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="--stream-description") {
      stream_description=cmd->value(i);
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="--stream-genre") {
      stream_genre=cmd->value(i);
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="--stream-name") {
      stream_name=cmd->value(i);
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="--stream-url") {
      stream_url=cmd->value(i);
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="--stream-irc") {
      stream_irc=cmd->value(i);
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="--stream-icq") {
      stream_icq=cmd->value(i);
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="--stream-aim") {
      stream_aim=cmd->value(i);
      cmd->setProcessed(i,true);
    }
    if(!cmd->processed(i)) {
      device_keys.push_back(cmd->key(i));
      device_values.push_back(cmd->value(i));
    }
  }

  //
  // Resource Enumerations
  //
  if(list_codecs) {
    ListCodecs();
    exit(0);
  }
  if(list_devices) {
    ListDevices();
    exit(0);
  }

  //
  // Sanity Checks
  //
  for(int i=0;i<device_keys.size();i++) {
    if(device_keys[i].split("-",QString::SkipEmptyParts)[0]!=
       AudioDevice::optionKeyword(audio_device)) {
      syslog(LOG_ERR,"glasscoder: unknown/inappropriate option \"%s\"",
	      (const char *)device_keys[i].toAscii());
      exit(256);
    }
  }

  if(server_hostname.isEmpty()) {
    syslog(LOG_ERR,"missing --server-hostname parameter");
    exit(256);
  }
  if((audio_quality>=0.0)&&(audio_bitrate.size()>0)) {
    syslog(LOG_ERR,
	   "--audio-quality and --audio-bitrate are mutually exclusive");
    exit(256);
  }
  if((server_type==Connector::Icecast2Server)&&(server_mountpoint.isEmpty())) {
    syslog(LOG_ERR,"mountpoint not specified");
    exit(256);
  }
  if((audio_bitrate.size()>1)&&(server_type!=Connector::HlsServer)) {
    syslog(LOG_ERR,"only HLS streams can have multiple bitrates");
    exit(256);
  }

  if((audio_quality<0.0)&&(audio_bitrate.size()==0)) {
    audio_bitrate.push_back(DEFAULT_AUDIO_BITRATE);
  }

  if(!StartAudioDevice()) {
    exit(256);
  }

  if(audio_bitrate.size()>1) {
    if(!StartMultiStream()) {
      exit(256);
    }
  }
  else {
    if(!StartSingleStream()) {
      exit(256);
    }
  }

  //
  // Set Signals
  //
  sir_exit_timer=new QTimer(this);
  connect(sir_exit_timer,SIGNAL(timeout()),this,SLOT(exitTimerData()));
  sir_exit_timer->start(250);
  ::signal(SIGINT,SignalHandler);
  ::signal(SIGTERM,SignalHandler);
}


void MainObject::audioDeviceStoppedData()
{
  glasscoder_exiting=true;
}


void MainObject::connectorStoppedData()
{
  if(++sir_exit_count==sir_connectors.size()) {
    exit(0);
  }
}


void MainObject::exitTimerData()
{
  if(glasscoder_exiting) {
    for(unsigned i=0;i<sir_connectors.size();i++) {
      sir_connectors[i]->stop();
    }
  }
}


bool MainObject::StartAudioDevice()
{
  //
  // Create Ringbuffers
  //
  if(audio_bitrate.size()==0) {   // For VBR modes
    sir_ringbuffers.push_back(new Ringbuffer(RINGBUFFER_SIZE,audio_channels));
  }
  else {
    for(unsigned i=0;i<audio_bitrate.size();i++) {
      sir_ringbuffers.push_back(new Ringbuffer(RINGBUFFER_SIZE,audio_channels));
    }
  }

  //
  // Start Audio Device
  //
  QString err;
  if((sir_audio_device=
      AudioDeviceFactory(audio_device,audio_channels,audio_samplerate,
			 &sir_ringbuffers,this))==NULL) {
    syslog(LOG_ERR,"%s devices not supported",(const char *)AudioDevice::deviceTypeText(audio_device).toUtf8());
    exit(256);
  }
  connect(sir_audio_device,SIGNAL(hasStopped()),
	  this,SLOT(audioDeviceStoppedData()));
  if(!sir_audio_device->processOptions(&err,device_keys,device_values)) {
    syslog(LOG_ERR,"%s",(const char *)err.toUtf8());
    exit(256);
  }
  if(!sir_audio_device->start(&err)) {
    syslog(LOG_ERR,"%s",(const char *)err.toUtf8());
    exit(256);
  }

  return true;
}


bool MainObject::StartCodec()
{
  Codec *codec;

  if((codec=
    CodecFactory(audio_format,sir_ringbuffers[sir_codecs.size()],this))==NULL) {
    syslog(LOG_ERR,"unsupported codec type \"%s\"",
	   (const char *)Codec::codecTypeText(Codec::TypeMpegL3).toUtf8());
    return false;
  }
  if(audio_bitrate.size()>0) {
    codec->setBitrate(audio_bitrate[sir_codecs.size()]);
  }
  else {
    codec->setBitrate(0);
  }
  codec->setChannels(audio_channels);
  codec->setQuality(audio_quality);
  codec->setSourceSamplerate(sir_audio_device->deviceSamplerate());
  codec->setStreamSamplerate(audio_samplerate);

  sir_codecs.push_back(codec);

  return sir_codecs.back()->start();
}


void MainObject::StartServerConnection(const QString &mntpt,bool is_top)
{
  Connector *conn;

  //
  // Create Connector Instance
  //
  conn=ConnectorFactory(server_type,is_top,this);
  if(!is_top) {
    connect(conn,SIGNAL(dataRequested(Connector *)),
	    sir_codecs[sir_connectors.size()],SLOT(encode(Connector *)));
    connect(conn,SIGNAL(stopped()),this,SLOT(connectorStoppedData()));
    sir_codecs[sir_connectors.size()]->
      setCompleteFrames(server_type==Connector::HlsServer);
  }

  //
  // Set Configuration
  //
  if(mntpt.isEmpty()) {
    conn->setServerMountpoint(server_mountpoint);
  }
  else {
    conn->setServerMountpoint(mntpt);
  }
  conn->setServerUsername(server_username);
  conn->setServerPassword(server_password);
  if(is_top) {
    conn->setAudioBitrates(&audio_bitrate);
    conn->setFormatIdentifier(sir_codecs[0]->formatIdentifier());
  }
  else {
    conn->setContentType(sir_codecs[sir_connectors.size()]->contentType());
    conn->setExtension(sir_codecs[sir_connectors.size()]->defaultExtension());
    conn->setFormatIdentifier(sir_codecs[sir_connectors.size()]->
			      formatIdentifier());
    if(audio_bitrate.size()>0) {
      conn->setAudioBitrate(audio_bitrate[sir_connectors.size()]);
    }
    else {
      conn->setAudioBitrate(0);
    }
  }
  conn->setAudioChannels(audio_channels);
  conn->setAudioSamplerate(audio_samplerate);
  conn->setStreamDescription(stream_description);
  conn->setStreamGenre(stream_genre);
  conn->setStreamName(stream_name);
  conn->setStreamUrl(stream_url);
  conn->setStreamIrc(stream_irc);
  conn->setStreamIcq(stream_icq);
  conn->setStreamAim(stream_aim);

  //
  // Open the server connection
  //
  sir_connectors.push_back(conn);
  sir_connectors.back()->connectToServer(server_hostname,server_port);
}


bool MainObject::StartSingleStream()
{
  if(!StartCodec()) {
    return false;
  }
  StartServerConnection();

  return true;
}


bool MainObject::StartMultiStream()
{
  //
  // Media streams
  //
  for(unsigned i=0;i<audio_bitrate.size();i++) {
    if(!StartCodec()) {
      return false;
    }
  }
  for(unsigned i=0;i<audio_bitrate.size();i++) {
    StartServerConnection(Connector::subMountpointName(server_mountpoint,
						       audio_bitrate[i]));
  }

  //
  // Top-level playlist
  //
  StartServerConnection(server_mountpoint,true);

  return true;
}


void MainObject::ListCodecs()
{
  for(int i=0;i<Codec::TypeLast;i++) {
    if(CodecFactory((Codec::Type)i,NULL,this)->isAvailable()) {
      printf("%s\n",
	     (const char *)Codec::optionKeyword((Codec::Type)i).toUtf8());
    }
  }
}


void MainObject::ListDevices()
{
  for(int i=0;i<AudioDevice::LastType;i++) {
    if(AudioDeviceFactory((AudioDevice::DeviceType)i,2,48000,NULL,this)!=NULL) {
      printf("%s\n",(const char *)AudioDevice::optionKeyword((AudioDevice::DeviceType)i).toUtf8());
    }
  }
}


int main(int argc,char *argv[])
{
  QCoreApplication a(argc,argv);
  new MainObject();
  return a.exec();
}
