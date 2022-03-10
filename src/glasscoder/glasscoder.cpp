// glasscoder.cpp
//
// glasscoder(1) Audio Encoder
//
//   (C) Copyright 2014-2022 Fred Gleason <fredg@paravelsystems.com>
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

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <curl/curl.h>

#include <QCoreApplication>

#include "audiodevicefactory.h"
#include "cmdswitch.h"
#include "codecfactory.h"
#include "connectorfactory.h"
#include "glasscoder.h"
#include "logging.h"

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
  sir_exit_count=0;
  sir_meta_server=NULL;

  sir_config=new Config();

  if(!StartAudioDevice()) {
    exit(256);
  }

  //
  // Initialize CURL
  //
  curl_global_init(CURL_GLOBAL_ALL);

  //
  // Stdin Interface
  //
  int flags=fcntl(0,F_GETFL,NULL);
  flags=flags|O_NONBLOCK;
  fcntl(0,F_SETFL,flags);
  sir_stdin_notify=new QSocketNotifier(0,QSocketNotifier::Read,this);
  connect(sir_stdin_notify,SIGNAL(activated(int)),
	  this,SLOT(stdinActivatedData(int)));

  //
  // Metadata Processor
  //
  if(sir_config->metadataPort()>0) {
    sir_meta_server=new MetaServer(sir_config,this);
    if(!sir_meta_server->listen(sir_config->metadataPort())) {
      Log(LOG_ERR,QString().sprintf("unable to bind port %u",
				    sir_config->metadataPort()));
      exit(256);
    }
  }

  //
  // Start Server Connection
  //
  if(!StartSingleStream()) {
    exit(256);
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


void MainObject::stdinActivatedData(int sock)
{
  char data[1501];
  int n;

  while((n=read(sock,data,1500))>0) {
    for(int i=0;i<n;i++) {
      switch(0xFF&data[i]) {
      case 10:
	ProcessCommand(sir_stdin_accum);
	sir_stdin_accum="";
	break;

      case 13:
	break;

      default:
	sir_stdin_accum+=data[i];
	break;
      }
    }
  }
}


void MainObject::audioDeviceStoppedData()
{
  glasscoder_exiting=true;
}


void MainObject::connectorStoppedData()
{
  if(++sir_exit_count==(sir_connectors.size()+1)) {
    for(unsigned i=0;i<sir_connectors.size();i++) {
      delete sir_connectors[i];
    }
    exit(0);
  }
}


void MainObject::meterData()
{
  int lvls[MAX_AUDIO_CHANNELS];

  sir_audio_device->meterLevels(lvls);
  switch(sir_codec->channels()) {
  case 1:
    printf("ME %04X%04X\n",lvls[0],lvls[0]);
    break;

  case 2:
    printf("ME %04X%04X\n",lvls[0],lvls[1]);
    break;
  }
  fflush(stdout);
}


void MainObject::connectedData(bool state)
{
  if(global_log_to==LOG_TO_STDOUT) {
    if(state) {
      printf("CS %d\n",CONNECTION_OK);
    }
    else {
      printf("CS %d\n",CONNECTION_FAILED);
    }
  }
}


void MainObject::exitTimerData()
{
  if(glasscoder_exiting) {
    bool connected=sir_connectors.at(0)->isConnected();
    for(unsigned i=0;i<sir_connectors.size();i++) {
      sir_connectors[i]->stop();
    }
    sir_exit_timer->stop();
    if((!sir_config->serverScriptDown().isEmpty())&&connected) {
      QString cmd=sir_config->serverScriptDown();
      if(fork()==0) {
	if(system(cmd.toUtf8())==0);  // FIXME: Replace this with QProcess
	_exit(0);  // _exit(2) NOT exit(3), to avoid racing with the parent
      }
    }
  }
}


bool MainObject::StartAudioDevice()
{
  //
  // Create Ringbuffer
  //
  if(sir_config->audioBitrate()==0) {   // For VBR modes
    sir_ringbuffer=new Ringbuffer(RINGBUFFER_SIZE,sir_config->audioChannels());
  }
  else {
    sir_ringbuffer=new Ringbuffer(RINGBUFFER_SIZE,sir_config->audioChannels());
  }

  //
  // Start Audio Device
  //
  QString err;
  if((sir_audio_device=
      AudioDeviceFactory(sir_config->audioDevice(),sir_config->audioChannels(),
			 sir_config->audioSamplerate(),
			 //			 &sir_ringbuffers,this))==NULL) {
			 sir_ringbuffer,this))==NULL) {
    Log(LOG_ERR,
	QString().sprintf("%s devices not supported",
	    (const char *)AudioDevice::deviceTypeText(sir_config->audioDevice()).toUtf8()));
    exit(256);
  }
  connect(sir_audio_device,SIGNAL(hasStopped()),
	  this,SLOT(audioDeviceStoppedData()));
  if(!sir_audio_device->processOptions(&err,sir_config->deviceKeys(),
				       sir_config->deviceValues())) {
    Log(LOG_ERR,err);
    exit(256);
  }
  if(!sir_audio_device->start(&err)) {
    Log(LOG_ERR,err);
    exit(256);
  }
  sir_meter_timer=new QTimer(this);
  connect(sir_meter_timer,SIGNAL(timeout()),this,SLOT(meterData()));
  if(sir_config->meterData()) {
    sir_meter_timer->start(AUDIO_METER_INTERVAL);
  }

  return true;
}


bool MainObject::StartCodec()
{
  if((sir_codec=
    CodecFactory(sir_config->audioFormat(),sir_ringbuffer,this))==NULL) {
    Log(LOG_ERR,
	QString().sprintf("unsupported codec type \"%s\"",
	      (const char *)Codec::codecTypeText(Codec::TypeMpegL3).toUtf8()));
    return false;
  }
  if(sir_config->audioBitrate()>0) {
    sir_codec->setBitrate(sir_config->audioBitrate());
  }
  else {
    sir_codec->setBitrate(0);
  }
  sir_codec->setChannels(sir_config->audioChannels());
  sir_codec->setQuality(sir_config->audioQuality());
  sir_codec->setSourceSamplerate(sir_audio_device->deviceSamplerate());
  sir_codec->setStreamSamplerate(sir_config->audioSamplerate());
  sir_codec->setCompleteFrames(sir_config->audioAtomicFrames());

  return sir_codec->start();
}


void MainObject::StartServerConnection(const QString &mntpt)
{
  Connector *conn;

  //
  // Create Connector Instance
  //
  conn=ConnectorFactory(sir_config->serverType(),sir_config,this);
  connect(conn,SIGNAL(stopped()),this,SLOT(connectorStoppedData()));
  connect(conn,SIGNAL(unmuteRequested()),sir_audio_device,SLOT(unmute()));
  connect(conn,SIGNAL(dataRequested(Connector *)),
	  sir_codec,SLOT(encode(Connector *)));
  connect(conn,SIGNAL(connected(bool)),this,SLOT(connectedData(bool)));
  connect(conn,SIGNAL(stopped()),this,SLOT(connectorStoppedData()));
  conn->setStreamPrologue(sir_codec->streamPrologue());
  sir_codec->setCompleteFrames(sir_config->serverType()==Connector::HlsServer);
  if(sir_meta_server!=NULL) {
    connect(sir_meta_server,SIGNAL(metadataReceived(MetaEvent *)),
	    conn,SLOT(sendMetadata(MetaEvent *)));
  }

  //
  // Set Configuration
  //
  if(mntpt.isEmpty()) {
    conn->setServerMountpoint(sir_config->serverUrl().path());
  }
  else {
    conn->setServerMountpoint(mntpt);
  }
  conn->setServerExitOnLast(sir_config->serverExitOnLast());
  conn->setServerUsername(sir_config->serverUsername());
  conn->setServerPassword(sir_config->serverPassword());
  conn->setServerMaxConnections(sir_config->serverMaxConnections());
  conn->setServerPipe(sir_config->serverPipe());
  conn->setServerStartConnections(sir_config->serverStartConnections());
  conn->setServerUserAgent(sir_config->serverUserAgent());
  conn->setDumpHeaders(sir_config->dumpHeaders());
  conn->setContentType(sir_codec->contentType());
  conn->setExtension(sir_codec->defaultExtension());
  conn->setFormatIdentifier(sir_codec->formatIdentifier());
  conn->setAudioBitrate(sir_config->audioBitrate());
  conn->setAudioChannels(sir_config->audioChannels());
  conn->setAudioSamplerate(sir_config->audioSamplerate());
  conn->setScriptUp(sir_config->serverScriptUp());
  conn->setScriptDown(sir_config->serverScriptDown());
  conn->setStreamDescription(sir_config->streamDescription());
  conn->setStreamGenre(sir_config->streamGenre());
  conn->setStreamName(sir_config->streamName());
  conn->setStreamUrl(sir_config->streamUrl());
  conn->setStreamIrc(sir_config->streamIrc());
  conn->setStreamIcq(sir_config->streamIcq());
  conn->setStreamAim(sir_config->streamAim());
  conn->setStreamTimestampOffset(sir_config->streamTimestampOffset());

  //
  // Open the server connection
  //
  sir_connectors.push_back(conn);
  sir_connectors.back()->connectToServer(sir_config->serverUrl());
}


bool MainObject::StartSingleStream()
{
  if(!StartCodec()) {
    return false;
  }
  StartServerConnection();

  return true;
}


void MainObject::ProcessCommand(const QString &cmd)
{
  QStringList cmds=cmd.split(" ");

  if(cmds[0]=="MD") {  // Metadata update
    MetaEvent *e=new MetaEvent();
    cmds.erase(cmds.begin());
    e->setField("StreamTitle",cmds.join(" "));
    for(unsigned i=0;i<sir_connectors.size();i++) {
      sir_connectors[i]->sendMetadata(e);
    }
    delete e;
  }
}


int main(int argc,char *argv[])
{
  QCoreApplication a(argc,argv);
  new MainObject();
  return a.exec();
}
