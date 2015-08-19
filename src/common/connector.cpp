// connector.cpp
//
// Abstract base class for streaming server source connections.
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

#include <syslog.h>

#include <QStringList>

#include "connector.h"

Connector::Connector(QObject *parent)
  : QObject(parent)
{
  conn_server_username="source";
  conn_server_password="";
  conn_server_mountpoint="";
  conn_content_type="";
  conn_audio_channels=2;
  conn_audio_samplerate=44100;
  conn_audio_bitrates.push_back(128);
  conn_stream_name="no name";
  conn_stream_description="unknown";
  conn_stream_url="";
  conn_stream_irc="";
  conn_stream_icq="";
  conn_stream_aim="";
  conn_stream_genre="unknown";
  conn_stream_public=true;
  conn_host_hostname="";
  conn_host_port=0;
  conn_connected=false;
  conn_watchdog_active=false;

  conn_data_timer=new QTimer(this);
  connect(conn_data_timer,SIGNAL(timeout()),this,SLOT(dataTimeoutData()));
  conn_data_timer->start(RINGBUFFER_SERVICE_INTERVAL);

  conn_watchdog_timer=new QTimer(this);
  conn_watchdog_timer->setSingleShot(true);
  connect(conn_watchdog_timer,SIGNAL(timeout()),
	  this,SLOT(watchdogTimeoutData()));

  conn_stop_timer=new QTimer(this);
  conn_stop_timer->setSingleShot(true);
  connect(conn_stop_timer,SIGNAL(timeout()),this,SLOT(stopTimeoutData()));
}


Connector::~Connector()
{
  delete conn_stop_timer;
  delete conn_data_timer;
}


QString Connector::serverUsername() const
{
  return conn_server_username;
}


void Connector::setServerUsername(const QString &str)
{
  conn_server_username=str;
}


QString Connector::serverPassword() const
{
  return conn_server_password;
}


void Connector::setServerPassword(const QString &str)
{
  conn_server_password=str;
}


QString Connector::serverMountpoint() const
{
  return conn_server_mountpoint;
}


void Connector::setServerMountpoint(const QString &str)
{
  conn_server_mountpoint=str;
}


QString Connector::contentType() const
{
  return conn_content_type;
}


void Connector::setContentType(const QString &str)
{
  conn_content_type=str;
}


unsigned Connector::audioChannels() const
{
  return conn_audio_channels;
}


void Connector::setAudioChannels(unsigned chans)
{
  conn_audio_channels=chans;
}


unsigned Connector::audioSamplerate() const
{
  return conn_audio_samplerate;
}


void Connector::setAudioSamplerate(unsigned rate)
{
  conn_audio_samplerate=rate;
}


unsigned Connector::audioBitrate() const
{
  return conn_audio_bitrates[0];
}


void Connector::setAudioBitrate(unsigned rate)
{
  conn_audio_bitrates.clear();
  conn_audio_bitrates.push_back(rate);
}


std::vector<unsigned> *Connector::audioBitrates()
{
  return &conn_audio_bitrates;
}


void Connector::setAudioBitrates(std::vector<unsigned> *rates)
{
  conn_audio_bitrates.clear();
  for(unsigned i=0;i<rates->size();i++) {
    conn_audio_bitrates.push_back(rates->at(i));
  }
}


QString Connector::streamName() const
{
  return conn_stream_name;
}


void Connector::setStreamName(const QString &str)
{
  conn_stream_name=str;
}


QString Connector::streamDescription() const
{
  return conn_stream_description;
}


void Connector::setStreamDescription(const QString &str)
{
  conn_stream_description=str;
}


QString Connector::streamUrl() const
{
  return conn_stream_url;
}


void Connector::setStreamUrl(const QString &str)
{
  conn_stream_url=str;
}


QString Connector::streamIrc() const
{
  return conn_stream_irc;
}


void Connector::setStreamIrc(const QString &str)
{
  conn_stream_irc=str;
}


QString Connector::streamIcq() const
{
  return conn_stream_icq;
}


void Connector::setStreamIcq(const QString &str)
{
  conn_stream_icq=str;
}


QString Connector::streamAim() const
{
  return conn_stream_aim;
}


void Connector::setStreamAim(const QString &str)
{
  conn_stream_aim=str;
}


QString Connector::streamGenre() const
{
  return conn_stream_genre;
}


void Connector::setStreamGenre(const QString &str)
{
  conn_stream_genre=str;
}


bool Connector::streamPublic() const
{
  return conn_stream_public;
}


void Connector::setStreamPublic(bool state)
{
  conn_stream_public=state;
}


QString Connector::extension() const
{
  return conn_extension;
}


void Connector::setExtension(const QString &str)
{
  conn_extension=str;
}


QString Connector::formatIdentifier() const
{
  return conn_format_identifier;
}


void Connector::setFormatIdentifier(const QString &str)
{
  conn_format_identifier=str;
}


void Connector::connectToServer(const QString &hostname,uint16_t port)
{
  conn_host_hostname=hostname;
  conn_host_port=port;
  connectToHostConnector(hostname,port);
}


int64_t Connector::writeData(int frames,const unsigned char *data,int64_t len)
{
  if(conn_connected) {
    return writeDataConnector(frames,data,len);
  }
  return 0;
}


void Connector::stop()
{
  conn_stop_timer->start(0);
}


QString Connector::serverTypeText(Connector::ServerType type)
{
  QString ret=tr("Unknown");

  switch(type) {
  case Connector::HlsServer:
    ret=tr("HTTP Live Streaming (HLS)");
    break;

  case Connector::Shoutcast1Server:
    ret=tr("Shoutcast v1");
    break;

  case Connector::Shoutcast2Server:
    ret=tr("Shoutcast v2");
    break;

  case Connector::Icecast2Server:
    ret=tr("Icecast v2");
    break;

  case Connector::LastServer:
    break;
  }

  return ret;
}


QString Connector::optionKeyword(Connector::ServerType type)
{
  QString ret;

  switch(type) {
  case Connector::HlsServer:
    ret="hls";
    break;

  case Connector::Shoutcast1Server:
    ret="shout1";
    break;

  case Connector::Shoutcast2Server:
    ret="shout2";
    break;

  case Connector::Icecast2Server:
    ret="icecast2";
    break;

  case Connector::LastServer:
    break;
  }

  return ret;
}


Connector::ServerType Connector::serverType(const QString &key)
{
  Connector::ServerType ret=Connector::LastServer;

  for(int i=0;i<Connector::LastServer;i++) {
    if(optionKeyword((Connector::ServerType)i)==key.toLower()) {
      ret=(Connector::ServerType)i;
    }
  }

  return ret;
}


QString Connector::subMountpointName(const QString &mntpt,unsigned bitrate)
{
  QStringList f0=mntpt.split(".");
  int offset=0;

  if((f0[f0.size()-1]=="m3u")||(f0[f0.size()-1]=="m3u8")) {
    offset=1;
  }
  f0.insert(f0.begin()+f0.size()-offset,QString().sprintf("%u",bitrate));
  return f0.join(".");
}


QString Connector::pathPart(const QString &fullpath)
{
  QStringList f0=fullpath.split("/");
  f0.erase(f0.begin()+f0.size()-1);
  return f0.join("/");
}


QString Connector::basePart(const QString &fullpath)
{
  QStringList f0=fullpath.split("/");
  return f0[f0.size()-1];
}


void Connector::dataTimeoutData()
{
  emit dataRequested(this);
}


void Connector::watchdogTimeoutData()
{
  connectToHostConnector(conn_host_hostname,conn_host_port);
}


void Connector::stopTimeoutData()
{
  emit stopped();
}


void Connector::setConnected(bool state)
{
  if(state&&conn_watchdog_active) {
    if(conn_server_mountpoint.isEmpty()) {
      syslog(LOG_WARNING,"connection to \"%s:%u\" restored",
	     (const char *)conn_host_hostname.toUtf8(),0xFFFF&conn_host_port);
    }
    else {
      syslog(LOG_WARNING,"connection to \"%s:%u/%s\" restored",
	     (const char *)conn_host_hostname.toUtf8(),0xFFFF&conn_host_port,
	     (const char *)conn_server_mountpoint.toUtf8());
    }
    conn_watchdog_active=false;
  }
  conn_connected=state;
}


void Connector::setError(QAbstractSocket::SocketError err)
{
  if(!conn_watchdog_active) {
    if(conn_server_mountpoint.isEmpty()) {
      syslog(LOG_WARNING,"connection to \"%s:%u\" lost",
	     (const char *)conn_host_hostname.toUtf8(),0xFFFF&conn_host_port);
    }
    else {
      syslog(LOG_WARNING,"connection to \"%s:%u/%s\" lost",
	     (const char *)conn_host_hostname.toUtf8(),0xFFFF&conn_host_port,
	     (const char *)conn_server_mountpoint.toUtf8());
    }
    conn_watchdog_active=true;
  }
  disconnectFromHostConnector();
  conn_watchdog_timer->start(5000);
}


QString Connector::hostHostname() const
{
  return conn_host_hostname;
}


uint16_t Connector::hostPort() const
{
  return conn_host_port;
}


QString Connector::urlEncode(const QString &str)
{
  QString ret;

  for(int i=0;i<str.length();i++) {
    if(str.at(i).isLetterOrNumber()) {
      ret+=str.mid(i,1);
    }
    else {
      ret+=QString().sprintf("%%%02X",str.at(i).toLatin1());
    }
  }

  return ret;
}


QString Connector::urlDecode(const QString &str)
{
  int istate=0;
  unsigned n;
  QString code;
  QString ret;
  bool ok=false;

  for(int i=0;i<str.length();i++) {
    switch(istate) {
    case 0:
      if(str.at(i)==QChar('+')) {
	ret+=" ";
      }
      else {
	if(str.at(i)==QChar('%')) {
	  istate=1;
	}
	else {
	  ret+=str.at(i);
	}
      }
      break;

    case 1:
      n=str.mid(i,1).toUInt(&ok);
      if((!ok)||(n>9)) {
	istate=0;
      }
      code=str.mid(i,1);
      istate=2;
      break;

    case 2:
      n=str.mid(i,1).toUInt(&ok);
      if((!ok)||(n>9)) {
	istate=0;
      }
      code+=str.mid(i,1);
      ret+=QChar(code.toInt(&ok,16));
      istate=0;
      break;
    }
  }

  return ret;
}


char base64_dict[64]={'A','B','C','D','E','F','G','H','I','J','K','L','M','N',
		      'O','P','Q','R','S','T','U','V','W','X','Y','Z','a','b',
		      'c','d','e','f','g','h','i','j','k','l','m','n','o','p',
		      'q','r','s','t','u','v','w','x','y','z','0','1','2','3',
		      '4','5','6','7','8','9','+','/'};


QString Connector::base64Encode(const QString &str)
{
  QString ret;
  uint32_t buf;

  //
  // Build Groups
  //
  for(int i=0;i<str.length();i+=3) {
    buf=(0xff&(str.at(i).toLatin1()))<<16;
    if((i+1)<str.length()) {
      buf|=(0xff&(str.at(i+1).toLatin1()))<<8;
      if((i+2)<str.length()) {
	buf|=0xff&(str.at(i+2).toLatin1());
      }
    }

    //
    // Dictionary Lookup
    //
    for(int i=3;i>=0;i--) {
      ret+=base64_dict[0x3f&(buf>>(6*i))];
    }
  }

  //
  // Apply Padding
  //
  switch(str.length()%3) {
    case 1:
      ret=ret.left(ret.length()-2)+"==";
      break;
      
    case 2:
      ret=ret.left(ret.length()-1)+"=";
      break;
  }


  return ret;
}


QString Connector::base64Decode(const QString &str,bool *ok)
{
  QString ret;
  uint32_t buf=0;
  char c;
  int pad=0;
  bool found=false;

  //
  // Set Status
  //
  if(ok!=NULL) {
    *ok=true;
  }

  //
  // Dictionary Lookup
  //
  for(int i=0;i<str.length();i+=4) {
    buf=0;
    for(int j=0;j<4;j++) {
      if(((i+j)<str.length())&&(c=str.at(i+j).toLatin1())!='=') {
	found=false;
	for(unsigned k=0;k<64;k++) {
	  if(base64_dict[k]==c) {
	    buf|=(k<<(6*(3-j)));
	    found=true;
	  }
	}
	if(!found) {  // Illegal character!
	  if(ok!=NULL) {
	    *ok=false;
	  }
	  return QString();
	}
      }
      else {
	if(str.at(i+j)=='=') {
	  pad++;
	}
      }
    }

    //
    // Extract Groups
    //
    for(int i=2;i>=0;i--) {
      ret+=(0xff&(buf>>(8*i)));
    }
  }

  //
  // Apply Padding
  //
  if(pad>2) {
    if(ok!=NULL) {
      *ok=false;
    }
    return QString();
  }
  ret=ret.left(ret.length()-pad);

  return ret;
}
