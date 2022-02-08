// netconveyor.cpp
//
// Serialized service for uploading files
//
//   (C) Copyright 2015-2022 Fred Gleason <fredg@paravelsystems.com>
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
#include <fcntl.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "connector.h"
#include "logging.h"
#include "netconveyor.h"
#include "paths.h"

NetConveyorEvent::NetConveyorEvent(void *orig,const QString &pathname,
				   HttpMethod meth)
{
  evt_originator=orig;
  evt_pathname=pathname;
  evt_method=meth;
}


void *NetConveyorEvent::originator() const
{
  return evt_originator;
}


QString NetConveyorEvent::pathname() const
{
  return evt_pathname;
}


NetConveyorEvent::HttpMethod NetConveyorEvent::method() const
{
  return evt_method;
}


QString NetConveyorEvent::dump() const
{
  return QString::asprintf("NetConveyorEvent %p\n",this)+
    "pathname: "+pathname()+"\n"+
    "method: "+NetConveyorEvent::httpMethodString(method());
}


QString NetConveyorEvent::httpMethodString(HttpMethod method)
{
  QString ret="UNKNOWN";

  switch(method) {
  case NetConveyorEvent::PutMethod:
    ret="PUT";
    break;

  case NetConveyorEvent::DeleteMethod:
    ret="DELETE";
    break;

  case NetConveyorEvent::StopMethod:
    ret="STOP";
    break;
  }

  return ret;
}




NetConveyor::NetConveyor(Config *conf,QObject *parent)
  : QObject(parent)
{
  conv_config=conf;
  conv_process=NULL;

  //
  // Create temp directory
  //
  char tempdir[PATH_MAX];

  strncpy(tempdir,"/tmp",PATH_MAX);
  if(getenv("TEMP")!=NULL) {
    strncpy(tempdir,getenv("TEMP"),PATH_MAX-1);
  }
  strncat(tempdir,"/glasscoder-XXXXXX",PATH_MAX-strlen(tempdir));
  if(mkdtemp(tempdir)==NULL) {
    syslog(LOG_ERR,"unable to create temporary directory [%s]",
	   strerror(errno));
    exit(256);
  }
  conv_temp_dir=new QDir(tempdir);

  //
  // Create Timers
  //
  conv_restart_timer=new QTimer(this);
  conv_restart_timer->setSingleShot(true);
  connect(conv_restart_timer,SIGNAL(timeout()),
	  this,SLOT(startConveyorProcess()));

  //
  // Start glassconv(1) Instance
  //
  conv_restart_timer->start(0);
}


NetConveyor::~NetConveyor()
{
  if(conv_process!=NULL) {
    conv_process->kill();
    delete conv_process;
  }
}


void NetConveyor::push(void *orig,const QString &pathname,
		       NetConveyorEvent::HttpMethod meth)
{
  NetConveyorEvent evt(orig,pathname,meth);
  push(evt);
}


void NetConveyor::push(const NetConveyorEvent &evt)
{
  //  printf("============================================\n");
  //  printf("pushing: %s\n",evt.dump().toUtf8().constData());
  int fd=-1;
  QString timestamp=Connector::timeStampString();
  QString temp_pathname=conv_temp_dir->path()+"/"+
    timestamp+"-"+
    NetConveyorEvent::httpMethodString(evt.method())+"-"+
    evt.pathname().split("/",QString::SkipEmptyParts).last();
  //  printf("sourcePathname: %s\n",evt.pathname().toUtf8().constData());
  //  printf("temp_pathname: %s\n",temp_pathname.toUtf8().constData());

  switch(evt.method()) {
  case NetConveyorEvent::DeleteMethod:
    if((fd=open(temp_pathname.toUtf8(),O_CREAT|O_WRONLY,S_IRUSR|S_IWUSR))>=0) {
      close(fd);
      conv_putted_files.removeAll(evt.pathname());
    }
    break;

  case NetConveyorEvent::PutMethod:
    if(link(evt.pathname().toUtf8(),temp_pathname.toUtf8())==0) {
      if((!conv_putted_files.contains(evt.pathname()))&&
	 (!conv_config->serverNoDeletes())) {
	conv_putted_files.push_back(evt.pathname());
      }
    }
    else {
      Log(LOG_WARNING,
	  QString::asprintf("link() call failed for \"%s\": %s",
			    evt.pathname().toUtf8().constData(),
			    strerror(errno)));
    }
    break;

  case NetConveyorEvent::StopMethod:  // Tell glassconv(1) to exit
    if((fd=open(temp_pathname.toUtf8(),O_CREAT|O_WRONLY,S_IRUSR|S_IWUSR))>=0) {
      close(fd);
    }
    break;
  }
}


void NetConveyor::stop()
{
  //
  // Clean up files on the publishing point
  //
  QStringList files=conv_putted_files;  // Get an invariant copy
  for(int i=0;i<files.size();i++) {
    push(this,files.at(i),NetConveyorEvent::DeleteMethod);
  }

  //
  // Tell glassconv(1) to exit
  //
  push(this,"glassconv",NetConveyorEvent::StopMethod);
}


void NetConveyor::startConveyorProcess()
{
  //
  // Generate Credentials
  //
  FILE *f=NULL;
  if((f=fopen((conv_temp_dir->path()+"/creds").toUtf8(),"w"))==NULL) {
    syslog(LOG_ERR,"unable to write to temp directory \"%s\": %s",
	   (conv_temp_dir->path()+"/creds").toUtf8().constData(),
	   strerror(errno));
    exit(1);
  }
  fprintf(f,"[Credentials]\n");
  if(!conv_config->serverUsername().isEmpty()) {
    fprintf(f,"Username=%s\n",
	    conv_config->serverUsername().toUtf8().constData());
    fprintf(f,"Password=%s\n",
	    conv_config->serverPassword().toUtf8().constData());
  }
  if(!conv_config->sshIdentity().isEmpty()) {
    fprintf(f,"SshIdentity=%s\n",
	    conv_config->sshIdentity().toUtf8().constData());
  }
  fprintf(f,"UserAgent=%s\n",
	  conv_config->serverUserAgent().toUtf8().constData());
  fclose(f);

  //
  // Start glassconv(1)
  //
  QStringList args;

  args.push_back("--dest-url="+conv_config->serverBaseUrl());
  args.push_back("--source-dir="+conv_temp_dir->path());

  conv_process=new QProcess(this);
  connect(conv_process,SIGNAL(readyReadStandardOutput()),
	  this,SLOT(processReadyReadData()));
  connect(conv_process,SIGNAL(finished(int,QProcess::ExitStatus)),
	  this,SLOT(processFinishedData(int,QProcess::ExitStatus)));
  conv_process->start(GLASSCODER_PREFIX+"/bin/glassconv",args);
}


void NetConveyor::processFinishedData(int exit_code,
				       QProcess::ExitStatus exit_status)
{
  if(exit_status!=QProcess::NormalExit) {
    Log(LOG_WARNING,
	"process \""+conv_process->program()+" "+
	conv_process->arguments().join(" ")+"\" crashed, restarting");
    conv_process->deleteLater();
    conv_restart_timer->start(1000);
  }
  else {
    switch((Config::ExitCode)conv_process->exitCode()) {
    case Config::ExitOk:
      emit stopped();

    case Config::ExitRetry:
      Log(LOG_WARNING,"program \""+conv_process->program()+"\""+
	  " exited unexpectedly, restarting");
      conv_process->deleteLater();
      conv_restart_timer->start(1000);
      break;

    case Config::ExitFatal:
      Log(LOG_ERR,"fatal conveyor error, exiting");
      exit(1);
      break;
    }
  }
}


void NetConveyor::processReadyReadData()
{
  QString msg=QString::fromUtf8(conv_process->readAllStandardOutput());
  QStringList f0=msg.split(" ",QString::KeepEmptyParts);
  bool ok=false;

  if((f0.size()>=3)&&(f0.at(0)=="ER")) {
    int prio=f0.at(1).toInt(&ok);
    if(ok) {
      f0.removeFirst();
      f0.removeFirst();
      Log(prio,f0.join(" "));
    }
  }
}
