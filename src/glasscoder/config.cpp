// config.cpp
//
// Configuration Class for glasscoder(1)
//
// (C) Copyright 2016-2022 Fred Gleason <fredg@paravelsystems.com>
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

#include <unistd.h>

#include <QCoreApplication>

#include "audiodevicefactory.h"
#include "codecfactory.h"
#include "config.h"
#include "logging.h"
#include "profile.h"

Config::Config()
{
  unsigned num;
  bool ok=false;
  audio_atomic_frames=false;
  audio_bitrate=0;
  audio_channels=MAX_AUDIO_CHANNELS;
  audio_device=DEFAULT_AUDIO_DEVICE;
  audio_format=Codec::TypeVorbis;
  audio_quality=-1.0;
  audio_samplerate=DEFAULT_AUDIO_SAMPLERATE;
  server_exit_on_last=false;
  server_max_connections=-1;
  server_password="";
  credentials_file="";
  delete_credentials=false;
  ssh_identity="";
  server_type=Connector::Icecast2Server;
  server_script_down="";
  server_script_up="";
  server_start_connections=0;
  server_pipe="";
  server_start_connections=0;
  server_no_deletes=false;
  stream_aim="";
  stream_genre="";
  stream_icq="";
  stream_irc="";
  stream_name="";
  stream_timestamp_offset=0;
  stream_url="";
  list_codecs=false;
  list_devices=false;
  metadata_port=0;
  global_log_string="";
  meter_data=false;
  dump_headers=false;
  show_verbose=false;
  server_user_agent=QString("GlassCoder/")+VERSION;

  CmdSwitch *cmd=new CmdSwitch("glasscoder",GLASSCODER_USAGE);
  for(unsigned i=0;i<cmd->keys();i++) {
    if(cmd->key(i)=="--audio-atomic-frames") {
      audio_atomic_frames=true;
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="--audio-bitrate") {
      num=cmd->value(i).toUInt(&ok);
      if(ok) {
	audio_bitrate=num;
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
			      (const char *)cmd->value(i).toUtf8()));
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
    if(cmd->key(i)=="--credentials-file") {
      credentials_file=cmd->value(i);
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="--delete-credentials") {
      delete_credentials=true;
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="--ssh-identity") {
      ssh_identity=cmd->value(i);
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="--dump-headers") {
      dump_headers=true;
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="--errors-string") {
      global_log_string=cmd->value(i);
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
    if(cmd->key(i)=="--metadata-port") {
      metadata_port=cmd->value(i).toUInt(&ok);
      if((!ok)||(metadata_port>0xFFFF)) {
	Log(LOG_ERR,"invalid --metadata-port argument");
	exit(256);
      }
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
    if(cmd->key(i)=="--server-exit-on-last") {
      server_exit_on_last=true;
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="--server-max-connections") {
      server_max_connections=cmd->value(i).toInt(&ok);
      if((!ok)||(server_max_connections<0)) {
	Log(LOG_ERR,"invalid argument for --server-max-connections");
	exit(256);
      }
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="--server-url") {
      server_url=QUrl(cmd->value(i));
      if(server_url.port()<0) {
	if(server_url.scheme().toLower()=="file") {
	  server_url.setPort(0);
	}
	if(server_url.scheme().toLower()=="http") {
	  server_url.setPort(80);
	}
	if(server_url.scheme().toLower()=="https") {
	  server_url.setPort(443);
	}
	if(server_url.scheme().toLower()=="sftp") {
	  server_url.setPort(22);
	}
	if(server_url.port()<0) {
	  Log(LOG_ERR,
	      "unknown/unsupported URL scheme \""+server_url.scheme()+"\"");
	  exit(256);
	}
      }
      if(cmd->value(i).right(1)=="/") {
	server_base_url=cmd->value(i).left(cmd->value(i).length()-1);
      }
      else {
	QStringList f0=cmd->value(i).split("/",QString::KeepEmptyParts);
	f0.removeLast();
	server_base_url=f0.join("/");
      }
      cmd->setProcessed(i,true);
      if(!server_url.isValid()) {
	Log(LOG_ERR,"invalid argument for --server-url");
	exit(256);
      }
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="--server-user-agent") {
      server_user_agent=cmd->value(i);
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
    if(cmd->key(i)=="--server-start-connections") {
      server_start_connections=cmd->value(i).toInt(&ok);
      if((!ok)||(server_start_connections<0)) {
	Log(LOG_ERR,"invalid --server-start-connections value \""+
	    cmd->value(i)+"\"");
	exit(256);
      }
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
			      (const char *)cmd->value(i).toUtf8()));
	exit(256);
      }
    }
    if(cmd->key(i)=="--server-pipe") {
      server_pipe=cmd->value(i);
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="--server-no-deletes") {
      server_no_deletes=true;
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
    if(cmd->key(i)=="--stream-timestamp-offset") {
      stream_timestamp_offset=cmd->value(i).toInt(&ok);
      if(!ok) {
	fprintf(stderr,"invalid --stream-timestamp-offset\n");
	exit(256);
      }
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="--verbose") {
      global_log_verbose=true;
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="--verbose") {
      show_verbose=true;
      cmd->setProcessed(i,true);
    }
    if(!cmd->processed(i)) {
      device_keys.push_back(cmd->key(i));
      device_values.push_back(cmd->value(i));
    }
  }

  //
  // Read Credentials
  //
  if(!credentials_file.isEmpty()) {
    Profile *p=new Profile();
    if(!p->setSource(credentials_file)) {
      Log(LOG_ERR,"credentials file not found");
      exit(256);
    }
    server_username=p->stringValue("Credentials","Username");
    server_password=p->stringValue("Credentials","Password");
    delete p;
    if(delete_credentials) {
      unlink(credentials_file.toUtf8());
    }
  }

  //
  // Sanity Checks
  //
  for(int i=0;i<device_keys.size();i++) {
    if(device_keys[i].split("-",QString::SkipEmptyParts)[0]!=
       AudioDevice::optionKeyword(audio_device)) {
      Log(LOG_ERR,
	  QString().sprintf("glasscoder: unknown/inappropriate option \"%s\"",
			    (const char *)device_keys[i].toUtf8()));
      exit(256);
    }
  }

  //
  // Resource Enumerations
  //
  if(listCodecs()) {
    ListCodecs();
    exit(0);
  }
  if(listDevices()) {
    ListDevices();
    exit(0);
  }

  //
  // Sanity Checks
  //
  if(Connector::requiresServerUrl(server_type)&&server_url.isEmpty()) {
    Log(LOG_ERR,"missing --server-url parameter");
    exit(256);
  }
  if((audio_quality>=0.0)&&(audio_bitrate>0)) {
    Log(LOG_ERR,"--audio-quality and --audio-bitrate are mutually exclusive");
    exit(256);
  }
  if((audio_quality<0.0)&&(audio_bitrate==0)) {
    audio_bitrate=DEFAULT_AUDIO_BITRATE;
  }
}


bool Config::audioAtomicFrames() const
{
  return audio_atomic_frames;
}


unsigned Config::audioBitrate() const
{
  return audio_bitrate;
}


unsigned Config::audioChannels() const
{
  return audio_channels;
}


AudioDevice::DeviceType Config::audioDevice() const
{
  return audio_device;
}


Codec::Type Config::audioFormat() const
{
  return audio_format;
}


unsigned Config::audioQuality() const
{
  return audio_quality;
}


unsigned Config::audioSamplerate() const
{
  return audio_samplerate;
}


bool Config::serverExitOnLast() const
{
  return server_exit_on_last;
}


int Config::serverMaxConnections() const
{
  return server_max_connections;
}


QString Config::serverPassword() const
{
  return server_password;
}


QString Config::credentialsFile() const
{
  return credentials_file;
}


bool Config::deleteCredentials() const
{
  return delete_credentials;
}


QString Config::sshIdentity() const
{
  return ssh_identity;
}


QString Config::serverScriptDown() const
{
  return server_script_down;
}


QString Config::serverScriptUp() const
{
  return server_script_up;
}


int Config::serverStartConnections() const
{
  return server_start_connections;
}


Connector::ServerType Config::serverType() const
{
  return server_type;
}


QUrl Config::serverUrl() const
{
  return server_url;
}


QString Config::serverBaseUrl() const
{
  return server_base_url;
}


QString Config::serverUserAgent() const
{
  return server_user_agent;
}


QString Config::serverUsername() const
{
  return server_username;
}


QString Config::serverPipe() const
{
  return server_pipe;
}

bool Config::serverNoDeletes() const
{
  return server_no_deletes;
}


QString Config::streamAim() const
{
  return stream_aim;
}


QString Config::streamDescription() const
{
  return stream_description;
}


QString Config::streamGenre() const
{
  return stream_genre;
}


QString Config::streamIcq() const
{
  return stream_icq;
}


QString Config::streamIrc() const
{
  return stream_irc;
}


QString Config::streamName() const
{
  return stream_name;
}


int Config::streamTimestampOffset() const
{
  return stream_timestamp_offset;
}


QString Config::streamUrl() const
{
  return stream_url;
}


QStringList Config::deviceKeys() const
{
  return device_keys;
}


QStringList Config::deviceValues() const
{
  return device_values;
}


bool Config::listCodecs() const
{
  return list_codecs;
}


bool Config::listDevices() const
{
  return list_devices;
}


unsigned Config::metadataPort() const
{
  return metadata_port;
}


bool Config::dumpHeaders() const
{
  return dump_headers;
}


bool Config::verbose() const
{
  return show_verbose;
}


bool Config::meterData() const
{
  return meter_data;
}


void Config::ListCodecs() const
{
  for(int i=0;i<Codec::TypeLast;i++) {
    if(CodecFactory((Codec::Type)i,NULL)->isAvailable()) {
      printf("%s\n",
	     (const char *)Codec::optionKeyword((Codec::Type)i).toUtf8());
    }
  }
}


void Config::ListDevices() const
{
  for(int i=0;i<AudioDevice::LastType;i++) {
    if(AudioDeviceFactory((AudioDevice::DeviceType)i,2,48000,NULL)!=NULL) {
      printf("%s\n",(const char *)AudioDevice::optionKeyword((AudioDevice::DeviceType)i).toUtf8());
    }
  }
}
