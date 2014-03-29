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
#include "connectorfactory.h"
#include "glasscoder.h"

MainObject::MainObject(QObject *parent)
  : QObject(parent)
{
  bool ok=false;
  bool debug=false;
  audio_bitrate=DEFAULT_AUDIO_BITRATE;
  audio_channels=MAX_AUDIO_CHANNELS;
  audio_samplerate=DEFAULT_AUDIO_SAMPLERATE;
  jack_server_name="";
  jack_client_name=DEFAULT_JACK_CLIENT_NAME;
  shout_server_hostname="";
  shout_server_mountpoint="";
  shout_server_password="";
  shout_server_port=DEFAULT_SHOUT_SERVER_PORT;
  shout_server_username=DEFAULT_SHOUT_SERVER_USERNAME;

  CmdSwitch *cmd=new CmdSwitch(qApp->argc(),qApp->argv(),"glasscoder",GLASSCODER_USAGE);
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
	// audio_format=SHOUT_FORMAT_MP3;
	cmd->setProcessed(i,true);
      }
      else {
	syslog(LOG_ERR,"unknown --audio-format value \"%s\"",
	       (const char *)cmd->value(i).toAscii());
	exit(256);
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
      shout_server_hostname=cmd->value(i);
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="--server-mountpoint") {
      shout_server_mountpoint=cmd->value(i);
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="--server-password") {
      shout_server_password=cmd->value(i);
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="--server-port") {
      shout_server_port=cmd->value(i).toUInt(&ok);
      if((!ok)||(shout_server_port==0)) {
	syslog(LOG_ERR,"invalid --shout-server-port value");
	exit(256);
      }
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="--server-type") {
      if(cmd->value(i).toLower()=="icecast1") {
	cmd->setProcessed(i,true);
      }
      else {
	if(cmd->value(i).toLower()=="icecast2") {
	  cmd->setProcessed(i,true);
	}
	else {
	  if(cmd->value(i).toLower()=="shout") {
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
    if(cmd->key(i)=="--server-username") {
      shout_server_username=cmd->value(i);
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
    if(!cmd->processed(i)) {
      syslog(LOG_ERR,"glasscoder: unknown option \"%s\"\n",
	      (const char *)cmd->key(i).toAscii());
      exit(256);
    }
  }
  if(shout_server_hostname.isEmpty()) {
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

  //
  // Initialize Encoder
  //
  if(!StartLame()) {
    exit(256);
  }

  //
  // Start Shout Instance
  //
  StartServerConnection();

  //
  // Encoder Timer
  //
  sir_encoder_timer=new QTimer(this);
  sir_encoder_timer->setSingleShot(true);
  connect(sir_encoder_timer,SIGNAL(timeout()),this,SLOT(layer3EncodeData()));
}


void MainObject::icyConnectedData(int result,const QString &txt)
{
  switch(result) {
  case 200:  // Success
    if(!StartJack()) {
      exit(256);
    }
    sir_encoder_timer->start(RINGBUFFER_SERVICE_INTERVAL);
    break;

  default:
    syslog(LOG_ERR,"server returned \"%d %s\"",
	   result,(const char *)txt.toUtf8());
    exit(256);
    break;
  }
}


void MainObject::icyDisconnectedData()
{
  syslog(LOG_ERR,"unable to connect to server");
  exit(256);
}


void MainObject::StartServerConnection()
{
  int err;

  //
  // Create Connector Instance
  //
  sir_connector=ConnectorFactory(Connector::Icecast2Server,this);
  connect(sir_connector,SIGNAL(connected(int,const QString &)),
	  this,SLOT(icyConnectedData(int,const QString &)));
  connect(sir_connector,SIGNAL(disconnected()),this,SLOT(icyDisconnectedData()));

  //
  // Set Configuration
  //
  sir_connector->setServerMountpoint(shout_server_mountpoint);
  sir_connector->setServerUsername(shout_server_username);
  sir_connector->setServerPassword(shout_server_password);
  sir_connector->setContentType("audio/mpeg");
  sir_connector->setAudioBitrate(audio_bitrate);
  sir_connector->setAudioChannels(audio_channels);
  sir_connector->setAudioSamplerate(audio_samplerate);
  sir_connector->setStreamDescription(stream_description);
  sir_connector->setStreamGenre(stream_genre);
  sir_connector->setStreamName(stream_name);
  sir_connector->setStreamUrl(stream_url);

  //
  // Open the server connection
  //
  sir_connector->connectToServer(shout_server_hostname,shout_server_port);
}


int main(int argc,char *argv[])
{
  QCoreApplication a(argc,argv);
  new MainObject();
  return a.exec();
}
