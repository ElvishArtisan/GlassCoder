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
  bool ok=false;
  sir_exit_count=0;
  audio_device=DEFAULT_AUDIO_DEVICE;
  audio_channels=MAX_AUDIO_CHANNELS;
  audio_format=Codec::TypeVorbis;
  audio_quality=-1.0;
  audio_samplerate=DEFAULT_AUDIO_SAMPLERATE;
  server_type=Connector::Icecast2Server;
  server_password="";
  server_script_up="";
  server_script_down="";
  stream_genre="";
  stream_name="";
  stream_url="";
  stream_irc="";
  stream_icq="";
  stream_aim="";
  list_codecs=false;
  list_devices=false;
  meter_data=false;

  unsigned num;

  CmdSwitch *cmd=
    new CmdSwitch(qApp->argc(),qApp->argv(),"glasscoder",GLASSCODER_USAGE);
  for(unsigned i=0;i<cmd->keys();i++) {
    if(cmd->key(i)=="--audio-bitrate") {
      num=cmd->value(i).toUInt(&ok);
      if(ok) {
	audio_bitrate.push_back(num);
      }
      else {
	Log(LOG_ERR,"invalid --audio-bitrate value");
	exit(256);
      }
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="--audio-channels") {
      audio_channels=cmd->value(i).toUInt(&ok);
      if((!ok)||(audio_channels==0)||(audio_channels>MAX_AUDIO_CHANNELS)) {
	Log(LOG_ERR,"invalid --audio-channels value");
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
      for(int j=0;j<Codec::TypeLast;j++) {
	if(Codec::optionKeyword((Codec::Type)j)==
	   cmd->value(i).toLower()) {
	  audio_format=(Codec::Type)j;
	  cmd->setProcessed(i,true);
	}
      }
      if(!cmd->processed(i)) {
	Log(LOG_ERR,
	    QString().sprintf("unknown --audio-format value \"%s\"",
			      (const char *)cmd->value(i).toAscii()));
	exit(256);
      }
    }
    if(cmd->key(i)=="--audio-quality") {
      audio_quality=cmd->value(i).toDouble(&ok);
      if((!ok)||(audio_quality<0.0)||(audio_quality>1.0)) {
	Log(LOG_ERR,"invalid --audio-quality value");
	exit(256);
      }
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="--audio-samplerate") {
      audio_samplerate=cmd->value(i).toUInt(&ok);
      if(!ok) {
	Log(LOG_ERR,"invalid --audio-samplerate value");
	exit(256);
      }
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="--errors-to") {
      if(cmd->value(i).toLower()=="stderr") {
	global_log_to=LOG_TO_STDERR;
	cmd->setProcessed(i,true);
      }
      if(cmd->value(i).toLower()=="syslog") {
	global_log_to=LOG_TO_SYSLOG;
	openlog("glasscoder",0,LOG_DAEMON);
	cmd->setProcessed(i,true);
      }
      if(cmd->value(i).toLower()=="stdout") {
	global_log_to=LOG_TO_STDOUT;
	cmd->setProcessed(i,true);
      }
    }
    if(cmd->key(i)=="--list-codecs") {
      list_codecs=true;
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="--list-devices") {
      list_devices=true;
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="--meter-data") {
      meter_data=true;
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="--server-auth") {
      QStringList f0=cmd->value(i).split(":");
      if(f0.size()==2) {
	server_username=f0[0];
	server_password=f0[1];
      }
      else {
	server_username=f0[0];
      }
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="--server-url") {
      server_url.setUrl(cmd->value(i));
      if(!server_url.isValid()) {
	Log(LOG_ERR,"invalid argument for --server-url");
	exit(256);
      }
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="--server-script-down") {
      server_script_down=cmd->value(i);
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="--server-script-up") {
      server_script_up=cmd->value(i);
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="--server-type") {
      for(int j=0;j<Connector::LastServer;j++) {
	if(Connector::optionKeyword((Connector::ServerType)j)==
	   cmd->value(i).toLower()) {
	  server_type=(Connector::ServerType)j;
	  cmd->setProcessed(i,true);
	}
      }
      if(!cmd->processed(i)) {
	Log(LOG_ERR,
	    QString().sprintf("unknown --server-type value \"%s\"",
			      (const char *)cmd->value(i).toAscii()));
	exit(256);
      }
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
    if(cmd->key(i)=="--verbose") {
      global_log_verbose=true;
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
      Log(LOG_ERR,
	  QString().sprintf("glasscoder: unknown/inappropriate option \"%s\"",
			    (const char *)device_keys[i].toAscii()));
      exit(256);
    }
  }

  if(server_url.isEmpty()) {
    Log(LOG_ERR,"missing --server-url parameter");
    exit(256);
  }
  if((audio_quality>=0.0)&&(audio_bitrate.size()>0)) {
    Log(LOG_ERR,"--audio-quality and --audio-bitrate are mutually exclusive");
    exit(256);
  }
  if((audio_bitrate.size()>1)&&(server_type!=Connector::HlsServer)) {
    Log(LOG_ERR,"only HLS streams can have multiple bitrates");
    exit(256);
  }

  if((audio_quality<0.0)&&(audio_bitrate.size()==0)) {
    audio_bitrate.push_back(DEFAULT_AUDIO_BITRATE);
  }

  if(!StartAudioDevice()) {
    exit(256);
  }

  //
  // Start Server Connections
  //
  sir_conveyor=new FileConveyor(this);
  sir_conveyor->setUsername(server_username);
  sir_conveyor->setPassword(server_password);
  connect(sir_conveyor,SIGNAL(stopped()),this,SLOT(connectorStoppedData()));
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
  if(++sir_exit_count==(sir_connectors.size()+1)) {
    for(unsigned i=0;i<sir_connectors.size();i++) {
      delete sir_connectors[i];
    }
    delete sir_conveyor;
    exit(0);
  }
}


void MainObject::meterData()
{
  int lvls[MAX_AUDIO_CHANNELS];

  sir_audio_device->meterLevels(lvls);
  switch(sir_codecs[0]->channels()) {
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
    for(unsigned i=0;i<sir_connectors.size();i++) {
      sir_connectors[i]->stop();
    }
    sir_conveyor->stop();
    sir_exit_timer->stop();
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
    Log(LOG_ERR,
	QString().sprintf("%s devices not supported",
	    (const char *)AudioDevice::deviceTypeText(audio_device).toUtf8()));
    exit(256);
  }
  connect(sir_audio_device,SIGNAL(hasStopped()),
	  this,SLOT(audioDeviceStoppedData()));
  if(!sir_audio_device->processOptions(&err,device_keys,device_values)) {
    Log(LOG_ERR,err);
    exit(256);
  }
  if(!sir_audio_device->start(&err)) {
    Log(LOG_ERR,err);
    exit(256);
  }
  sir_meter_timer=new QTimer(this);
  connect(sir_meter_timer,SIGNAL(timeout()),this,SLOT(meterData()));
  if(meter_data) {
    sir_meter_timer->start(AUDIO_METER_INTERVAL);
  }

  return true;
}


bool MainObject::StartCodec()
{
  Codec *codec;

  if((codec=
    CodecFactory(audio_format,sir_ringbuffers[sir_codecs.size()],this))==NULL) {
    Log(LOG_ERR,
	QString().sprintf("unsupported codec type \"%s\"",
	      (const char *)Codec::codecTypeText(Codec::TypeMpegL3).toUtf8()));
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
  conn=ConnectorFactory(server_type,is_top,sir_conveyor,this);
  connect(conn,SIGNAL(stopped()),this,SLOT(connectorStoppedData()));
  if(!is_top) {
    connect(conn,SIGNAL(dataRequested(Connector *)),
	    sir_codecs[sir_connectors.size()],SLOT(encode(Connector *)));
    connect(conn,SIGNAL(connected(bool)),this,SLOT(connectedData(bool)));
    sir_codecs[sir_connectors.size()]->
      setCompleteFrames(server_type==Connector::HlsServer);
  }

  //
  // Set Configuration
  //
  if(mntpt.isEmpty()) {
    conn->setServerMountpoint(server_url.path());
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
  conn->setScriptUp(server_script_up);
  conn->setScriptDown(server_script_down);
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
  uint16_t port=DEFAULT_SERVER_PORT;
  if(server_url.port()>0) {
    port=server_url.port();
  }
  sir_connectors.back()->connectToServer(server_url.host(),port);
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
    StartServerConnection(Connector::subMountpointName(server_url.path(),
						       audio_bitrate[i]));
  }

  //
  // Top-level playlist
  //
  StartServerConnection(server_url.path(),true);

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
