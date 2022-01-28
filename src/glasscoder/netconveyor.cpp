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

#include "netconveyor.h"

NetConveyorEvent::NetConveyorEvent(void *orig,const QString &filename,const QString &url,
			     HttpMethod meth)
{
  evt_originator=orig;
  evt_filename=filename;
  evt_url=QUrl(url,QUrl::StrictMode);
  evt_method=meth;
}


void *NetConveyorEvent::originator() const
{
  return evt_originator;
}


QString NetConveyorEvent::filename() const
{
  return evt_filename;
}


QUrl NetConveyorEvent::url() const
{
  return evt_url;
}


NetConveyorEvent::HttpMethod NetConveyorEvent::method() const
{
  return evt_method;
}


QString NetConveyorEvent::httpMethodString(HttpMethod method)
{
  QString ret="UNKNOWN";

  switch(method) {
  case NetConveyorEvent::NoMethod:
    ret="NONE";
    break;

  case NetConveyorEvent::GetMethod:
    ret="GET";
    break;

  case NetConveyorEvent::PostMethod:
    ret="POST";
    break;

  case NetConveyorEvent::PutMethod:
    ret="PUT";
    break;

  case NetConveyorEvent::DeleteMethod:
    ret="DELETE";
    break;

  case NetConveyorEvent::HeadMethod:
    ret="HEAD";
    break;
  }

  return ret;
}




NetConveyor::NetConveyor(QObject *parent)
  : QObject(parent)
{
  conv_process=NULL;

  //
  // Timers
  //
  conv_nomethod_timer=new QTimer(this);
  conv_nomethod_timer->setSingleShot(true);
  connect(conv_nomethod_timer,SIGNAL(timeout()),this,SLOT(nomethodData()));

  conv_garbage_timer=new QTimer(this);
  conv_garbage_timer->setSingleShot(true);
  connect(conv_garbage_timer,SIGNAL(timeout()),
	  this,SLOT(processCollectGarbageData()));

  conv_dummy_process_timer=new QTimer(this);
  conv_dummy_process_timer->setSingleShot(true);
  connect(conv_dummy_process_timer,SIGNAL(timeout()),
	  this,SLOT(dummyProcessData()));

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
}


NetConveyor::~NetConveyor()
{
  //
  // Clean up temp directory
  //
  QStringList files=conv_temp_dir->entryList(QDir::Files);
  for(int i=0;i<files.size();i++) {
    unlink((conv_temp_dir->path()+"/"+files[i]).toUtf8());
  }
  rmdir(conv_temp_dir->path().toUtf8());
  delete conv_temp_dir;

  delete conv_garbage_timer;
  if(conv_process!=NULL) {
    conv_process->kill();
    delete conv_process;
  }
}


void NetConveyor::setUsername(const QString &str)
{
  conv_username=str;
}


void NetConveyor::setPassword(const QString &str)
{
  conv_password=str;
}


void NetConveyor::setUserAgent(const QString &str)
{
  conv_user_agent=str;
}


void NetConveyor::setAddedHeaders(const QStringList &hdrs)
{
  conv_added_headers=hdrs;
}


void NetConveyor::push(void *orig,const QString &filename,const QString &url,
			NetConveyorEvent::HttpMethod meth)
{
  NetConveyorEvent evt(orig,filename,url,meth);
  push(evt);
}


void NetConveyor::push(void *orig,const QString &url,
			NetConveyorEvent::HttpMethod meth)
{
  push(orig,"",url,meth);
}


void NetConveyor::push(const NetConveyorEvent &evt)
{
  if(!evt.filename().isEmpty()) {
    if(unlink(Repath(evt.filename()).toUtf8())==0) {
      syslog(LOG_DEBUG,"NetConveyor::push: had to move \"%s\" out of the way",
	     (const char *)Repath(evt.filename()).toUtf8());
    }
    if(link(evt.filename().toUtf8(),Repath(evt.filename()).toUtf8())!=0) {
      syslog(LOG_WARNING,"NetConveyor::push: unable to make hard link %s [%s]",
	     (const char *)Repath(evt.filename()).toUtf8(),strerror(errno));
      return;
    }
  }
  conv_events.push(evt);
  if(conv_events.size()==1) {
    Dispatch();
  }
}


void NetConveyor::stop()
{
  QStringList urls=conv_putted_files;
  for(int i=0;i<urls.size();i++) {
    push(this,"",urls[i],NetConveyorEvent::DeleteMethod);
  }
  push(this,"","",NetConveyorEvent::NoMethod);
}


void NetConveyor::processErrorData(QProcess::ProcessError err)
{
  emit error(conv_events.front(),err,conv_arguments);
  conv_garbage_timer->start(0);
}


void NetConveyor::processFinishedData(int exit_code,
				       QProcess::ExitStatus exit_status)
{
  bool ok=false;

  if(exit_status==QProcess::CrashExit) {
    emit eventFinished(conv_events.front(),256,0,conv_arguments);
  }
  else {
    if(exit_code!=0) {
      emit eventFinished(conv_events.front(),exit_code,0,conv_arguments);
    }
    else {
      if(conv_events.front().method()==NetConveyorEvent::PutMethod) {
	AddPuttedFile(conv_events.front().url().toEncoded()+
		      conv_events.front().filename().split("/").back());
      }
      if(conv_events.front().method()==NetConveyorEvent::DeleteMethod) {
	conv_putted_files.removeAll(conv_events.front().url().toString());
      }
      if(conv_process==NULL) {
	emit eventFinished(conv_events.front(),0,200,QStringList());
      }
      else {
	QString response=conv_process->readAllStandardOutput();
	if((conv_events.front().url().scheme().toLower()=="sftp")&&
	   (response.toInt(&ok)==0)) {
	  if(ok) {
	    response="200";
	  }
	}
	emit eventFinished(conv_events.front(),0,response.toInt(),
			   conv_arguments);
      }
    }
  }
  conv_garbage_timer->start(0);
}


void NetConveyor::processCollectGarbageData()
{
  if(conv_process!=NULL) {
    delete conv_process;
    conv_process=NULL;
  }
  if(!conv_events.front().filename().isEmpty()) {
    unlink(Repath(conv_events.front().filename()).toUtf8());
  }
  conv_events.pop();
  if(conv_events.size()>0) {
    Dispatch();
  }
}


void NetConveyor::nomethodData()
{
  emit eventFinished(conv_events.front(),0,200,QStringList());
  processCollectGarbageData();
}


void NetConveyor::dummyProcessData()
{
  processFinishedData(0,QProcess::NormalExit);
}


void NetConveyor::Dispatch()
{
  NetConveyorEvent evt=conv_events.front();
  /*
  printf("Dispatch (via \"%s\"): %s => %s\n",
	 (const char *)evt.url().scheme().toUtf8(),
	 (const char *)evt.filename().toUtf8(),
	 (const char *)evt.url().toString().toUtf8());
  */
  if(evt.url().toString().isEmpty()) {
    emit stopped();
  }
  if(evt.url().scheme().toLower()=="file") {
    DispatchFile(evt);
  }
  if((evt.url().scheme().toLower()=="http")||
     (evt.url().scheme().toLower()=="https")) {
    DispatchHttp(evt);
  }
  if(evt.url().scheme().toLower()=="sftp") {
    DispatchSftp(evt);
  }
}


void NetConveyor::DispatchFile(const NetConveyorEvent &evt)
{
  QString destname;

  switch(evt.method()) {
  case NetConveyorEvent::PutMethod:
    destname=evt.url().path()+evt.filename().split("/").back();
    rename(Repath(evt.filename()).toUtf8(),destname.toUtf8());
    AddPuttedFile(evt.url().toString()+evt.filename().split("/").back());
    break;

  case NetConveyorEvent::DeleteMethod:
    unlink(evt.url().path().toUtf8());
    RemovePuttedFile(evt.url().toString());
    break;

  case NetConveyorEvent::PostMethod:  // Should never happen!
  case NetConveyorEvent::GetMethod:
  case NetConveyorEvent::HeadMethod:
  case NetConveyorEvent::NoMethod:
    break;
  }

  conv_dummy_process_timer->start(0);
}


void NetConveyor::DispatchHttp(const NetConveyorEvent &evt)
{
  conv_arguments.clear();
  AddCurlAuthArgs(&conv_arguments,evt);
  if((!conv_user_agent.isEmpty())&&(!conv_password.isEmpty())) {
    conv_arguments.push_back("--user-agent");
    conv_arguments.push_back(conv_user_agent);
  }
  AddHeaders(&conv_arguments,conv_added_headers);
  conv_arguments.push_back("--write-out");
  conv_arguments.push_back("%{response_code}");
  conv_arguments.push_back("--silent");
  conv_arguments.push_back("--output");
  conv_arguments.push_back("/dev/null");

  switch(evt.method()) {
  case NetConveyorEvent::GetMethod:
    conv_arguments.push_back(evt.url().toEncoded());
    break;

  case NetConveyorEvent::PutMethod:
    conv_arguments.push_back("-T");
    conv_arguments.push_back(Repath(evt.filename()));
    conv_arguments.push_back(evt.url().toEncoded());
    break;

  case NetConveyorEvent::DeleteMethod:
    conv_arguments.push_back("-X");
    conv_arguments.push_back("DELETE");
    conv_arguments.push_back(evt.url().toEncoded());
    break;

  case NetConveyorEvent::PostMethod:  // Should never happen!
  case NetConveyorEvent::HeadMethod:
  case NetConveyorEvent::NoMethod:
    break;
  }

  conv_process=new QProcess(this);
  connect(conv_process,SIGNAL(error(QProcess::ProcessError)),
	  this,SLOT(processErrorData(QProcess::ProcessError)));
  connect(conv_process,SIGNAL(finished(int,QProcess::ExitStatus)),
	  this,SLOT(processFinishedData(int,QProcess::ExitStatus)));
  conv_process->start("curl",conv_arguments);
}


void NetConveyor::DispatchSftp(const NetConveyorEvent &evt)
{
  conv_arguments.clear();
  AddCurlAuthArgs(&conv_arguments,evt);
  if((!conv_user_agent.isEmpty())&&(!conv_password.isEmpty())) {
    conv_arguments.push_back("--user-agent");
    conv_arguments.push_back(conv_user_agent);
  }
  conv_arguments.push_back("--write-out");
  conv_arguments.push_back("%{response_code}");
  conv_arguments.push_back("--silent");
  conv_arguments.push_back("--output");
  conv_arguments.push_back("/dev/null");
  conv_arguments.push_back("-k");

  switch(evt.method()) {
  case NetConveyorEvent::PutMethod:
    conv_arguments.push_back("-T");
    conv_arguments.push_back(Repath(evt.filename()));
    conv_arguments.push_back(evt.url().toString());
    break;

  case NetConveyorEvent::DeleteMethod:
    conv_arguments.push_back("-Q");
    conv_arguments.push_back("rm "+evt.url().path());
    conv_arguments.push_back(evt.url().toString(QUrl::RemovePath));
    break;

  case NetConveyorEvent::PostMethod:  // Should never happen!
  case NetConveyorEvent::GetMethod:
  case NetConveyorEvent::HeadMethod:
  case NetConveyorEvent::NoMethod:
    break;
  }

  //printf("curl %s\n",(const char *)conv_arguments.join(" ").toUtf8());

  conv_process=new QProcess(this);
  connect(conv_process,SIGNAL(error(QProcess::ProcessError)),
	  this,SLOT(processErrorData(QProcess::ProcessError)));
  connect(conv_process,SIGNAL(finished(int,QProcess::ExitStatus)),
	  this,SLOT(processFinishedData(int,QProcess::ExitStatus)));
  conv_process->start("curl",conv_arguments);
}


void NetConveyor::AddHeaders(QStringList *arglist,const QStringList &hdrs)
{
  for(int i=0;i<hdrs.size();i++) {
    arglist->push_back("-H");
    arglist->push_back(hdrs[i]);
  }
}


void NetConveyor::AddCurlAuthArgs(QStringList *arglist,
				   const NetConveyorEvent &evt)
{
  if(!conv_username.isEmpty()) {
    arglist->push_back("-u");
    if(conv_password.isEmpty()) {
      arglist->push_back(conv_username);
    }
    else {
      arglist->push_back(conv_username+":"+conv_password);
    }
  }
}


QString NetConveyor::Repath(const QString &filename) const
{
  return conv_temp_dir->path()+"/"+filename.split("/").back();
}


void NetConveyor::AddPuttedFile(const QString &url)
{
  for(int i=0;i<conv_putted_files.size();i++) {
    if(url==conv_putted_files[i]) {
      return;
    }
  }
  conv_putted_files.push_back(url);
}


void NetConveyor::RemovePuttedFile(const QString &url)
{
  for(int i=0;i<conv_putted_files.size();i++) {
    if(url==conv_putted_files[i]) {
      conv_putted_files.erase(conv_putted_files.begin()+i);
      return;
    }
  }
}
