// connector.cpp
//
// Abstract base class for streaming server source connections.
//
//   (C) Copyright 2014-2020 Fred Gleason <fredg@paravelsystems.com>
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

#include <ctype.h>
#include <time.h>

#include <QStringList>

#include "connector.h"
#include "logging.h"

Connector::Connector(QObject *parent)
  : QObject(parent)
{
  conn_server_exit_on_last=false;
  conn_server_max_connections=-1;
  conn_server_username="source";
  conn_server_password="";
  conn_server_mountpoint="";
  conn_server_start_connections=0;
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
  conn_stream_timestamp_offset=0;
  conn_connected=false;
  conn_watchdog_active=false;
  conn_script_up_process=NULL;
  conn_script_down_process=NULL;
  conn_dump_headers=false;
  conn_is_stopping=false;

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

  conn_script_down_garbage_timer=new QTimer(this);
  conn_script_down_garbage_timer->setSingleShot(true);
  connect(conn_script_down_garbage_timer,SIGNAL(timeout()),
	  this,SLOT(scriptDownCollectGarbageData()));

  conn_script_up_garbage_timer=new QTimer(this);
  conn_script_up_garbage_timer->setSingleShot(true);
  connect(conn_script_up_garbage_timer,SIGNAL(timeout()),
	  this,SLOT(scriptUpCollectGarbageData()));
}


Connector::~Connector()
{
  delete conn_stop_timer;
  delete conn_data_timer;
}


bool Connector::serverExitOnLast() const
{
  return conn_server_exit_on_last;
}


bool Connector::isConnected() const
{
  return conn_connected;
}


void Connector::setServerExitOnLast(bool state)
{
  conn_server_exit_on_last=state;
}


int Connector::serverMaxConnections() const
{
  return conn_server_max_connections;
}


void Connector::setServerMaxConnections(int max)
{
  conn_server_max_connections=max;
}


QString Connector::serverUserAgent() const
{
  return conn_server_user_agent;
}


void Connector::setServerUserAgent(const QString &str)
{
  conn_server_user_agent=str;
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


QString Connector::serverBasicAuthString() const
{
  //
  // As per RFC 2617 (2)
  //
  return Connector::base64Encode(serverUsername()+":"+serverPassword());
}


QString Connector::serverMountpoint() const
{
  return conn_server_mountpoint;
}


void Connector::setServerMountpoint(const QString &str)
{
  conn_server_mountpoint=str;
}


QString Connector::serverPipe() const
{
  return conn_server_pipe;
}


void Connector::setServerPipe(const QString &str)
{
  conn_server_pipe=str;
}


int Connector::serverStartConnections() const
{
  return conn_server_start_connections;
}


void Connector::setServerStartConnections(int conns)
{
  conn_server_start_connections=conns;
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


QUrl Connector::streamUrl() const
{
  return conn_stream_url;
}


void Connector::setStreamUrl(const QString &str)
{
  conn_stream_url=QUrl(str);
}


void Connector::setStreamUrl(const QUrl &url)
{
  conn_stream_url=url;
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


int Connector::streamTimestampOffset() const
{
  return conn_stream_timestamp_offset;
}


void Connector::setStreamTimestampOffset(int msec)
{
  conn_stream_timestamp_offset=msec;
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


QUrl Connector::serverUrl() const
{
  return conn_server_url;
}


void Connector::connectToServer(const QUrl &url)
{
  conn_server_url=url;
  connectToHostConnector(url);
}


int64_t Connector::writeData(int frames,const unsigned char *data,int64_t len)
{
  return writeDataConnector(frames,data,len);
}


void Connector::stop()
{
  conn_is_stopping=true;
  startStopping();
  setConnected(false);

  //
  // We have to run this synchronously since we're shutting down
  //
  if(conn_connected&&(!conn_script_down.isEmpty())) {
    QStringList args;
    args=conn_script_down.split(" ");
    QString cmd=args[0];
    args.erase(args.begin());
    conn_script_down_process=new QProcess(this);
    conn_script_down_process->start(cmd,args);
    conn_script_down_process->waitForFinished(3000);
  }
}


QString Connector::scriptUp() const
{
  return conn_script_up;
}


void Connector::setScriptUp(const QString &cmd)
{
  conn_script_up=cmd;
}


QString Connector::scriptDown() const
{
  return conn_script_down;
}


void Connector::setScriptDown(const QString &cmd)
{
  conn_script_down=cmd;
}


bool Connector::dumpHeaders() const
{
  return conn_dump_headers;
}


void Connector::setDumpHeaders(bool state)
{
  conn_dump_headers=state;
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

  case Connector::FileServer:
    ret=tr("Local File");
    break;

  case Connector::FileArchiveServer:
    ret=tr("Local File Archive");
    break;

  case Connector::IcecastStreamerServer:
    ret=tr("Integrated Icecast Server");
    break;

  case Connector::IcecastOutServer:
    ret=tr("Icecast Direct Stream");
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

  case Connector::FileServer:
    ret="file";
    break;

  case Connector::FileArchiveServer:
    ret="filearchive";
    break;

  case Connector::IcecastStreamerServer:
    ret="icestreamer";
    break;

  case Connector::IcecastOutServer:
    ret="iceout";
    break;

  case Connector::LastServer:
    break;
  }

  return ret;
}


bool Connector::requiresServerUrl(Connector::ServerType type)
{
  bool ret=true;

  switch(type) {
  case Connector::HlsServer:
  case Connector::Shoutcast1Server:
  case Connector::Shoutcast2Server:
  case Connector::Icecast2Server:
  case Connector::FileServer:
  case Connector::FileArchiveServer:
  case Connector::LastServer:
    ret=true;
    break;

  case Connector::IcecastStreamerServer:
  case Connector::IcecastOutServer:
    ret=false;
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
  connectToHostConnector(conn_server_url);
}


void Connector::stopTimeoutData()
{
  emit stopped();
}


void Connector::scriptErrorData(QProcess::ProcessError err)
{
  Log(LOG_ERR,
      QString().sprintf("curl(1) process error: %d, cmd: \"curl %s\"",err,
			(const char *)conn_script_up_args.join(" ").toUtf8()));
}


void Connector::scriptUpFinishedData(int exit_code,
				     QProcess::ExitStatus exit_status)
{
  conn_script_up_garbage_timer->start(0);
}


void Connector::scriptUpCollectGarbageData()
{
  delete conn_script_up_process;
  conn_script_up_process=NULL;
}


void Connector::scriptDownFinishedData(int exit_code,
				       QProcess::ExitStatus exit_status)
{
  conn_script_down_garbage_timer->start(0);
}


void Connector::scriptDownCollectGarbageData()
{
  delete conn_script_down_process;
  conn_script_down_process=NULL;
}


void Connector::startStopping()
{
  conn_stop_timer->start(0);
}


void Connector::setConnected(bool state)
{
  QStringList args;
  QString cmd;

  if(conn_connected!=state) {
    if(!conn_is_stopping) {
      if(state) {
	if(conn_script_up_process==NULL) {
	  if(!conn_script_up.isEmpty()) {
	    args=conn_script_up.split(" ");
	    cmd=args[0];
	    args.erase(args.begin());
	    conn_script_up_process=new QProcess(this);
	    connect(conn_script_up_process,
		    SIGNAL(error(QProcess::ProcessError)),
		    this,SLOT(scriptErrorData(QProcess::ProcessError)));
	    connect(conn_script_up_process,
		    SIGNAL(finished(int,QProcess::ExitStatus)),
		    this,SLOT(scriptUpFinishedData(int,QProcess::ExitStatus)));
	    conn_script_up_process->start(cmd,args);
	  }
	}
	else {
	  if(global_log_verbose) {
	    Log(LOG_WARNING,"curl(1) script-up command overrun, cmd: \""+
		args.join(" ")+"\"");
	  }
	  else {
	    Log(LOG_WARNING,"curl(1) command overrun");
	  }
	}
      }
      else {
	if(conn_script_down_process==NULL) {
	  if(!conn_script_down.isEmpty()) {
	    args=conn_script_down.split(" ");
	    cmd=args[0];
	    args.erase(args.begin());
	    conn_script_down_process=new QProcess(this);
	    connect(conn_script_down_process,
		    SIGNAL(error(QProcess::ProcessError)),
		    this,SLOT(scriptErrorData(QProcess::ProcessError)));
	    connect(conn_script_down_process,
		    SIGNAL(finished(int,QProcess::ExitStatus)),
		    this,
		    SLOT(scriptDownFinishedData(int,QProcess::ExitStatus)));
	    conn_script_down_process->start(cmd,args);
	  }
	}
	else {
	  if(global_log_verbose) {
	    Log(LOG_WARNING,"curl(1) script-down command overrun, cmd: \""+
		args.join(" ")+"\"");
	  }
	  else {
	    Log(LOG_WARNING,"curl(1) command overrun");
	  }
	}
      }
    }
  }
  if(state&&conn_watchdog_active) {
    if(conn_server_mountpoint.isEmpty()) {
      Log(LOG_WARNING,
	  "connection to \""+conn_server_url.toString()+"\" restored");
    }
    else {
      Log(LOG_WARNING,
	  "connection to \""+conn_server_url.toString()+"/"+
	  conn_server_mountpoint+"\" restored");
    }
    conn_watchdog_active=false;
  }
  conn_connected=state;
  emit connected(state);
}


void Connector::setError(QAbstractSocket::SocketError err)
{
  syslog(LOG_DEBUG,"received socket error \"%s\"",
	 Connector::socketErrorText(err).toUtf8().constData());
  if(!conn_watchdog_active) {
    if(conn_server_mountpoint.isEmpty()) {
      Log(LOG_WARNING,
	  "connection to \""+conn_server_url.toString()+"\" lost");
    }
    else {
      Log(LOG_WARNING,
	  "connection to \""+conn_server_url.toString()+"/"+
	  conn_server_mountpoint+"\" lost");
    }
    conn_watchdog_active=true;
  }
  disconnectFromHostConnector();
  conn_watchdog_timer->start(5000);
}


QString Connector::urlEncode(const QString &str)
{
  QByteArray ret;

  QByteArray bytes=str.toUtf8();
  for(int i=0;i<bytes.length();i++) {
    int ch=0xFF&bytes[i];
    if(isalnum(ch)||  // Unreserved characters (as per RFC 3986 2.3)
       (ch=='-')||(ch=='_')||(ch=='.')||(ch=='~')) {
      ret+=ch;
    }
    else {
      ret+=QString().sprintf("%%%02X",ch).toUtf8();
    }
  }

  return QString(ret);
}


QString Connector::urlDecode(const QString &str)
{
  int istate=0;
  unsigned n;
  QString code;
  QByteArray ret;
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
	  ret+=str.at(i).toLatin1();
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
      ret+=code.toInt(&ok,16);
      istate=0;
      break;
    }
  }

  return QString::fromUtf8(ret);
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


QString Connector::curlStrError(int exit_code)
{
  //
  // From the curl(1) man page (v7.29.0)
  //
  QString ret=tr("Unknown CURL error");

  switch(exit_code) {
  case 1:
    ret=tr("Unsupported protocol");
    break;

  case 2:
    ret=tr("Failed to initialize");
    break;

  case 3:
    ret=tr("Malformed URL");
    break;

  case 4:
    ret=tr("Feature not supported");
    break;

  case 5:
    ret=tr("Cannot resolve proxy");
    break;

  case 6:
    ret=tr("Cannot resolve host");
    break;

  case 7:
    ret=tr("Connection to host failed");
    break;

  case 8:
    ret=tr("Unrecognized FTP server response");
    break;

  case 9:
    ret=tr("FTP access denied");
    break;

  case 11:
    ret=tr("Unrecognized FTP PASS reply");
    break;

  case 13:
    ret=tr("Unrecognized FTP PASV reply");
    break;

  case 14:
    ret=tr("Unrecognized FTP 227 format");
    break;

  case 15:
    ret=tr("FTP can't get host");
    break;

  case 17:
    ret=tr("FTP couldn't set binary mode");
    break;

  case 18:
    ret=tr("Partial file transfer");
    break;

  case 19:
    ret=tr("FTP download failed");
    break;

  case 21:
    ret=tr("FTP quote error");
    break;

  case 22:
    ret=tr("HTTP page not retrieved");
    break;

  case 23:
    ret=tr("Write error");
    break;

  case 25:
    ret=tr("FTP STOR error");
    break;

  case 26:
    ret=tr("Read error");
    break;

  case 27:
    ret=tr("Out of memory");
    break;

  case 28:
    ret=tr("Write error");
    break;

  case 30:
    ret=tr("FTP PORT failed");
    break;

  case 31:
    ret=tr("FTP REST failed");
    break;

  case 33:
    ret=tr("HTTP range error");
    break;

  case 34:
    ret=tr("HTTP POST error");
    break;

  case 35:
    ret=tr("SSL connect error");
    break;

  case 36:
    ret=tr("FTP bad download resume");
    break;

  case 37:
    ret=tr("FILE read failure");
    break;

  case 38:
    ret=tr("LDAP bind error");
    break;

  case 39:
    ret=tr("LDAP search failed");
    break;

  case 41:
    ret=tr("LDAP function not found");
    break;

  case 42:
    ret=tr("Aborted");
    break;

  case 43:
    ret=tr("Internal error");
    break;

  case 45:
    ret=tr("Interface error");
    break;

  case 47:
    ret=tr("Too many redirects");
    break;

  case 48:
    ret=tr("Unknown option");
    break;

  case 49:
    ret=tr("Malformed telnet option");
    break;

  case 51:
    ret=tr("Bad certificate");
    break;

  case 52:
    ret=tr("No data returned");
    break;

  case 53:
    ret=tr("No SSL crypto engine");
    break;

  case 54:
    ret=tr("Cannot set SSL crypto engine as default");
    break;

  case 55:
    ret=tr("Send failure");
    break;

  case 56:
    ret=tr("Receive failure");
    break;

  case 58:
    ret=tr("Local certificate problem");
    break;

  case 59:
    ret=tr("Cannot use requested SSL cipher");
    break;

  case 60:
    ret=tr("Peer certificate cannot be authenticated");
    break;

  case 61:
    ret=tr("Unrecognized transfer encoding");
    break;

  case 62:
    ret=tr("Invalid LDAP URL");
    break;

  case 63:
    ret=tr("Maximum file size exceeded");
    break;

  case 64:
    ret=tr("Requested FTP SSL level failed");
    break;

  case 65:
    ret=tr("Rewind failed");
    break;

  case 66:
    ret=tr("SSL engine initialization failed");
    break;

  case 67:
    ret=tr("Authentication failure");
    break;

  case 68:
    ret=tr("TFTP file not found");
    break;

  case 69:
    ret=tr("TFTP permmission problem");
    break;

  case 70:
    ret=tr("TFTP out of disc space");
    break;

  case 71:
    ret=tr("TFTP illegal operation");
    break;

  case 72:
    ret=tr("TFTP unkown transfer ID");
    break;

  case 73:
    ret=tr("TFTP file already exists");
    break;

  case 74:
    ret=tr("TFTP no such user");
    break;

  case 75:
    ret=tr("Character conversion failed");
    break;

  case 76:
    ret=tr("Character conversion functions required");
    break;

  case 77:
    ret=tr("SSL CA cert read problems");
    break;

  case 78:
    ret=tr("Reference resources does not exist");
    break;

  case 79:
    ret=tr("SSH unspecified error");
    break;

  case 80:
    ret=tr("SSL failed to shut down connection");
    break;

  case 82:
    ret=tr("Cannot load CRL file");
    break;

  case 83:
    ret=tr("Issuer check failed");
    break;

  case 84:
    ret=tr("FTP PRET command failed");
    break;

  case 85:
    ret=tr("RTSP CSeq mismatch");
    break;

  case 86:
    ret=tr("RTSP SSID mismatch");
    break;

  case 87:
    ret=tr("FTP unable to parse file list");
    break;

  case 88:
    ret=tr("FTP chunk callback error");
    break;

  default:
    ret=tr("Unknown CURL error")+QString().sprintf(" [%d]",exit_code);
  }

  return ret;
}


QString Connector::httpStrError(int status_code)
{
  //
  // From RFC-2616 Section 6.1.1
  //
  QString ret=tr("Unknown status code");

  switch(status_code) {
  case 100:
    ret=tr("Continue");
    break;
    
  case 101:
    ret=tr("Switching Protocols");
    break;
    
  case 200:
    ret=tr("OK");
    break;
    
  case 201:
    ret=tr("Created");
    break;
    
  case 202:
    ret=tr("Accepted");
    break;
    
  case 203:
    ret=tr("Non-Authoritative Information");
    break;
    
  case 204:
    ret=tr("No Content");
    break;
    
  case 205:
    ret=tr("Reset Content");
    break;
    
  case 206:
    ret=tr("Partial Content");
    break;
    
  case 300:
    ret=tr("Multiple Choices");
    break;
    
  case 301:
    ret=tr("Moved Permanently");
    break;
    
  case 302:
    ret=tr("Found");
    break;
    
  case 303:
    ret=tr("See Other");
    break;
    
  case 304:
    ret=tr("Not Modified");
    break;
    
  case 305:
    ret=tr("Use Proxy");
    break;
    
  case 307:
    ret=tr("Temporary Redirect");
    break;
    
  case 400:
    ret=tr("Bad Request");
    break;
    
  case 401:
    ret=tr("Unauthorized");
    break;
    
  case 402:
    ret=tr("Payment Required");
    break;
    
  case 403:
    ret=tr("Forbidden");
    break;
    
  case 404:
    ret=tr("Not Found");
    break;
    
  case 405:
    ret=tr("Method Not Allowed");
    break;
    
  case 406:
    ret=tr("Not Acceptable");
    break;
    
  case 407:
    ret=tr("Proxy Authentication Required");
    break;
    
  case 408:
    ret=tr("Request Time-out");
    break;
    
  case 409:
    ret=tr("Conflict");
    break;
    
  case 410:
    ret=tr("Gone");
    break;
    
  case 411:
    ret=tr("Length Required");
    break;
    
  case 412:
    ret=tr("Precondition Failed");
    break;
    
  case 413:
    ret=tr("Request Entity Too Large");
    break;
    
  case 414:
    ret=tr("Request-URI Too Large");
    break;
    
  case 415:
    ret=tr("Unsupported Media Type");
    break;
    
  case 416:
    ret=tr("Requested range not satisfiable");
    break;
    
  case 417:
    ret=tr("Expectation Failed");
    break;
    
  case 500:
    ret=tr("Internal Server Error");
    break;
    
  case 501:
    ret=tr("Not Implemented");
    break;
    
  case 502:
    ret=tr("Bad Gateway");
    break;
    
  case 503:
    ret=tr("Service Unavailable");
    break;
    
  case 504:
    ret=tr("Gateway Time-out");
    break;
    
  case 505:
    ret=tr("HTTP Version not supported");
    break;
  }

  return QString().sprintf("%d - ",status_code)+ret;
}


QString Connector::timezoneOffset()
{
  QString ret="Z";
  time_t t=time(NULL);
  time_t gmt;
  time_t lt;
  
  gmt=mktime(gmtime(&t));
  lt=mktime(localtime(&t));

  if(gmt<lt) {
    ret=QString().sprintf("-%02ld:%02ld",(lt-gmt)/3600,((lt-gmt)%3600)/60);
  }
  if(gmt>lt) {
    ret=QString().sprintf("+%02ld:%02ld",(gmt-lt)/3600,((gmt-lt)%3600)/60);
  }

  return ret;
}


int Connector::id3TagSize(const QByteArray &data)
{
  //
  // See id3v2.4.0-structure, section 3
  //
  return ((0xFF&data[6])*2048383)+  // Synchsafe integer
    ((0xFF&data[7])*16129)+
    ((0xFF&data[8])*127)+
    (0xFF&data[9])+
    10;
}


QString Connector::socketErrorText(QAbstractSocket::SocketError err)
{
  //
  // This *really* ought to be part of Qt!
  //
  QString ret=QString().sprintf("unknown socket error [%d]",err);

  switch(err) {
  case QAbstractSocket::ConnectionRefusedError:
    ret="connection refused";
    break;

  case QAbstractSocket::RemoteHostClosedError:
    ret="remote host closed connection";
    break;

  case QAbstractSocket::HostNotFoundError:
    ret="remote host address not found";
    break;

  case QAbstractSocket::SocketAccessError:
    ret="socket access error";
    break;

  case QAbstractSocket::SocketResourceError:
    ret="socket resource error";
    break;

  case QAbstractSocket::SocketTimeoutError:
    ret="remote host connection timed out";
    break;

  case QAbstractSocket::DatagramTooLargeError:
    ret="datagram too large error";
    break;

  case QAbstractSocket::NetworkError:
    ret="network error";
    break;

  case QAbstractSocket::AddressInUseError:
    ret="unable to bind to specified port (port in use)";
    break;

  case QAbstractSocket::SocketAddressNotAvailableError:
    ret="unable to bind to specfied port (insuffcient privileges)";
    break;

  case QAbstractSocket::UnsupportedSocketOperationError:
    ret="unsupported socket operation";
    break;

  case QAbstractSocket::ProxyAuthenticationRequiredError:
    ret="proxy authentication failed";
    break;

  case QAbstractSocket::SslHandshakeFailedError:
    ret="SSL handshake failed";
    break;

  case QAbstractSocket::UnfinishedSocketOperationError:
    ret="unfinished socket operations error";
    break;

  case QAbstractSocket::ProxyConnectionRefusedError:
    ret="proxy connection refused";
    break;

  case QAbstractSocket::ProxyConnectionClosedError:
    ret="proxy connection closed unexpectedly";
    break;

  case QAbstractSocket::ProxyConnectionTimeoutError:
    ret="proxy connection timed out";
    break;

  case QAbstractSocket::ProxyNotFoundError:
    ret="proxy not found";
    break;

  case QAbstractSocket::ProxyProtocolError:
    ret="proxy protocol error";
    break;

  case QAbstractSocket::OperationError:
    ret="socket operation error";
    break;

  case QAbstractSocket::SslInternalError:
    ret="SSL internal error";
    break;

  case QAbstractSocket::SslInvalidUserDataError:
    ret="SLL authentication error";
    break;

  case QAbstractSocket::TemporaryError:
    ret="temporary socket error";
    break;

  case QAbstractSocket::UnknownSocketError:
    break;
  }

  return ret;
}


QString Connector::timeStampString()
{
  struct timespec ts;

  memset(&ts,0,sizeof(ts));
  if(clock_gettime(CLOCK_REALTIME,&ts)!=0) {
    Log(LOG_WARNING,
	QString::asprintf("unable to get system time: %s",strerror(errno)));
  }
  return QString::asprintf("%020lu%09lu",ts.tv_sec,ts.tv_nsec);
}


void Connector::sendMetadata(MetaEvent *e)
{
  QStringList keys=e->fieldKeys();

  for(int i=0;i<keys.size();i++) {
    printf("%s: %s\n",(const char *)keys.at(i).toUtf8(),
	   (const char *)e->field(keys.at(i)).toUtf8());
  }
  //  printf("StreamTitle: %s\n",(const char *)e->field(MetaEvent::StreamTitle).toString().toUtf8());
}


void Connector::setStreamPrologue(const QByteArray &data)
{
}
