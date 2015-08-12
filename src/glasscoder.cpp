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
  audio_channels=MAX_AUDIO_CHANNELS;
  audio_format=Codec::TypeVorbis;
  audio_quality=-1.0;
  audio_samplerate=DEFAULT_AUDIO_SAMPLERATE;
  jack_server_name="";
  jack_client_name=DEFAULT_JACK_CLIENT_NAME;
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
    if(cmd->key(i)=="--jack-server-name") {
      jack_server_name=cmd->value(i);
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="--jack-client-name") {
      jack_client_name=cmd->value(i);
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="--server-hostname") {
      server_hostname=cmd->value(i);
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="--server-mountpoint") {
      server_mountpoint=cmd->value(i);
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
      syslog(LOG_ERR,"glasscoder: unknown option \"%s\"",
	      (const char *)cmd->key(i).toAscii());
      exit(256);
    }
  }

  //
  // Sanity Checks
  //
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
  printf("size: %lu  type: %u\n",audio_bitrate.size(),server_type);
  if((audio_bitrate.size()>1)&&(server_type!=Connector::HlsServer)) {
    syslog(LOG_ERR,"only HLS streams can have multiple bitrates");
    exit(256);
  }

  if((audio_quality<0.0)&&(audio_bitrate.size()==0)) {
    audio_bitrate.push_back(DEFAULT_AUDIO_BITRATE);
  }

  if(!StartJack()) {
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
  codec->setSourceSamplerate(jack_get_sample_rate(sir_jack_client));
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


int main(int argc,char *argv[])
{
  QCoreApplication a(argc,argv);
  new MainObject();
  return a.exec();
}
