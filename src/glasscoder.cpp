// glasscoder.cpp
//
// glasscoder(1) Audio Encoder
//
//   (C) Copyright 2014 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: glasscoder.cpp,v 1.4 2014/02/24 21:02:45 cvs Exp $
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
#include <stdlib.h>
#include <syslog.h>

#include <QtCore/QCoreApplication>

#include "cmdswitch.h"
#include "codecfactory.h"
#include "connectorfactory.h"
#include "glasscoder.h"

MainObject::MainObject(QObject *parent)
  : QObject(parent)
{
  bool ok=false;
  bool debug=false;
  audio_bitrate=DEFAULT_AUDIO_BITRATE;
  audio_channels=MAX_AUDIO_CHANNELS;
  audio_format=Codec::TypeMpegL3;
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


  CmdSwitch *cmd=
    new CmdSwitch(qApp->argc(),qApp->argv(),"glasscoder",GLASSCODER_USAGE);
  for(unsigned i=0;i<cmd->keys();i++) {
    if(cmd->key(i)=="-d") {
      debug=true;
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="--audio-bitrate") {
      audio_bitrate=cmd->value(i).toUInt(&ok);
      if(!ok) {
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
	  syslog(LOG_ERR,"unknown --audio-format value \"%s\"",
		 (const char *)cmd->value(i).toAscii());
	  exit(256);
	}
      }
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
	  syslog(LOG_ERR,"unknown server type \"%s\"",
		 (const char *)cmd->value(i).toAscii());
	  exit(256);
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
      syslog(LOG_ERR,"glasscoder: unknown option \"%s\"\n",
	      (const char *)cmd->key(i).toAscii());
      exit(256);
    }
  }
  if(server_hostname.isEmpty()) {
    syslog(LOG_ERR,"missing --server-hostname parameter");
    exit(256);
  }

  //
  // Open Syslog
  //
  if(debug) {
    openlog("glasscoder",LOG_PERROR,LOG_DAEMON);
  }
  else {
    openlog("glasscoder",0,LOG_DAEMON);
  }

  if(!StartJack()) {
    exit(256);
  }

  //
  // Initialize Encoder
  //
  if(!StartCodec()) {
    exit(256);
  }

  //
  // Start Server Connection
  //
  StartServerConnection();
}


void MainObject::encodeData()
{
  sir_codec->encode(sir_connector);
}


bool MainObject::StartCodec()
{
  if((sir_codec=CodecFactory(audio_format,sir_ringbuffer,this))==NULL) {
    syslog(LOG_ERR,"unsupported codec type \"%s\"",
	   (const char *)Codec::codecTypeText(Codec::TypeMpegL3).toUtf8());
    return false;
  }
  sir_codec->setBitrate(audio_bitrate);
  sir_codec->setChannels(audio_channels);
  sir_codec->setSourceSamplerate(jack_get_sample_rate(sir_jack_client));
  sir_codec->setStreamSamplerate(audio_samplerate);

  return sir_codec->start();
}


void MainObject::StartServerConnection()
{
  //
  // Create Connector Instance
  //
  sir_connector=ConnectorFactory(server_type,this);
  connect(sir_connector,SIGNAL(dataRequested(Connector *)),
	  sir_codec,SLOT(encode(Connector *)));

  //
  // Set Configuration
  //
  sir_connector->setServerMountpoint(server_mountpoint);
  sir_connector->setServerUsername(server_username);
  sir_connector->setServerPassword(server_password);
  sir_connector->setContentType(sir_codec->contentType());
  sir_connector->setAudioBitrate(audio_bitrate);
  sir_connector->setAudioChannels(audio_channels);
  sir_connector->setAudioSamplerate(audio_samplerate);
  sir_connector->setStreamDescription(stream_description);
  sir_connector->setStreamGenre(stream_genre);
  sir_connector->setStreamName(stream_name);
  sir_connector->setStreamUrl(stream_url);
  sir_connector->setStreamIrc(stream_irc);
  sir_connector->setStreamIcq(stream_icq);
  sir_connector->setStreamAim(stream_aim);

  //
  // Open the server connection
  //
  sir_connector->connectToServer(server_hostname,server_port);
}


int main(int argc,char *argv[])
{
  QCoreApplication a(argc,argv);
  new MainObject();
  return a.exec();
}
