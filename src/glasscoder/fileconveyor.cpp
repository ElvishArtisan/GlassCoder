// fileconveyor.cpp
//
// Serialized service for uploading files
//
//   (C) Copyright 2015-2019 Fred Gleason <fredg@paravelsystems.com>
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

#include "fileconveyor.h"

ConveyorEvent::ConveyorEvent(void *orig,const QString &filename,const QString &url,
			     HttpMethod meth)
{
  evt_originator=orig;
  evt_filename=filename;
  evt_url=QUrl(url);
  evt_method=meth;
}


void *ConveyorEvent::originator() const
{
  return evt_originator;
}


QString ConveyorEvent::filename() const
{
  return evt_filename;
}


QUrl ConveyorEvent::url() const
{
  return evt_url;
}


ConveyorEvent::HttpMethod ConveyorEvent::method() const
{
  return evt_method;
}


QString ConveyorEvent::httpMethodString(HttpMethod method)
{
  QString ret="UNKNOWN";

  switch(method) {
  case ConveyorEvent::NoMethod:
    ret="NONE";
    break;

  case ConveyorEvent::GetMethod:
    ret="GET";
    break;

  case ConveyorEvent::PostMethod:
    ret="POST";
    break;

  case ConveyorEvent::PutMethod:
    ret="PUT";
    break;

  case ConveyorEvent::DeleteMethod:
    ret="DELETE";
    break;

  case ConveyorEvent::HeadMethod:
    ret="HEAD";
    break;
  }

  return ret;
}




FileConveyor::FileConveyor(QObject *parent)
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
    strncpy(tempdir,getenv("TEMP"),PATH_MAX);
  }
  strncat(tempdir,"/glasscoder-XXXXXX",PATH_MAX-strlen(tempdir));
  if(mkdtemp(tempdir)==NULL) {
    syslog(LOG_ERR,"unable to create temporary directory [%s]",
	   strerror(errno));
    exit(256);
  }
  conv_temp_dir=new QDir(tempdir);
}


FileConveyor::~FileConveyor()
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


void FileConveyor::setUsername(const QString &str)
{
  conv_username=str;
}


void FileConveyor::setPassword(const QString &str)
{
  conv_password=str;
}


void FileConveyor::setUserAgent(const QString &str)
{
  conv_user_agent=str;
}


void FileConveyor::setAddedHeaders(const QStringList &hdrs)
{
  conv_added_headers=hdrs;
}


void FileConveyor::push(void *orig,const QString &filename,const QString &url,
			ConveyorEvent::HttpMethod meth)
{
  ConveyorEvent evt(orig,filename,url,meth);
  push(evt);
}


void FileConveyor::push(void *orig,const QString &url,
			ConveyorEvent::HttpMethod meth)
{
  push(orig,"",url,meth);
}


void FileConveyor::push(const ConveyorEvent &evt)
{
  if(!evt.filename().isEmpty()) {
    if(unlink(Repath(evt.filename()).toUtf8())==0) {
      syslog(LOG_DEBUG,"FileConveyor::push: had to move \"%s\" out of the way",
	     (const char *)Repath(evt.filename()).toUtf8());
    }
    if(link(evt.filename().toUtf8(),Repath(evt.filename()).toUtf8())!=0) {
      syslog(LOG_WARNING,"FileConveyor::push: unable to make hard link %s [%s]",
	     (const char *)Repath(evt.filename()).toUtf8(),strerror(errno));
      return;
    }
  }
  conv_events.push(evt);
  if(conv_events.size()==1) {
    Dispatch();
  }
}


void FileConveyor::stop()
{
  QStringList urls=conv_putted_files;
  for(int i=0;i<urls.size();i++) {
    push(this,"",urls[i],ConveyorEvent::DeleteMethod);
  }
  push(this,"","",ConveyorEvent::NoMethod);
}


void FileConveyor::processErrorData(QProcess::ProcessError err)
{
  emit error(conv_events.front(),err,conv_arguments);
  conv_garbage_timer->start(0);
}


void FileConveyor::processFinishedData(int exit_code,
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
      if(conv_events.front().method()==ConveyorEvent::DeleteMethod) {
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


void FileConveyor::processCollectGarbageData()
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


void FileConveyor::nomethodData()
{
  emit eventFinished(conv_events.front(),0,200,QStringList());
  processCollectGarbageData();
}


void FileConveyor::dummyProcessData()
{
  processFinishedData(0,QProcess::NormalExit);
}


void FileConveyor::Dispatch()
{
  ConveyorEvent evt=conv_events.front();
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


void FileConveyor::DispatchFile(const ConveyorEvent &evt)
{
  QString destname;

  switch(evt.method()) {
  case ConveyorEvent::PutMethod:
    destname=evt.url().path()+evt.filename().split("/").back();
    rename(Repath(evt.filename()).toUtf8(),destname.toUtf8());
    AddPuttedFile(evt.url().toString()+evt.filename().split("/").back());
    break;

  case ConveyorEvent::DeleteMethod:
    unlink(evt.url().path().toUtf8());
    RemovePuttedFile(evt.url().toString());
    break;

  case ConveyorEvent::PostMethod:  // Should never happen!
  case ConveyorEvent::GetMethod:
  case ConveyorEvent::HeadMethod:
  case ConveyorEvent::NoMethod:
    break;
  }

  conv_dummy_process_timer->start(0);
}


void FileConveyor::DispatchHttp(const ConveyorEvent &evt)
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
  case ConveyorEvent::GetMethod:
    conv_arguments.push_back(evt.url().toString());
    break;

  case ConveyorEvent::PutMethod:
    conv_arguments.push_back("-T");
    conv_arguments.push_back(Repath(evt.filename()));
    conv_arguments.push_back(evt.url().toString());
    AddPuttedFile(evt.url().toString()+evt.filename().split("/").back());
    break;

  case ConveyorEvent::DeleteMethod:
    conv_arguments.push_back("-X");
    conv_arguments.push_back("DELETE");
    conv_arguments.push_back(evt.url().toString());
    RemovePuttedFile(evt.url().toString());
    break;

  case ConveyorEvent::PostMethod:  // Should never happen!
  case ConveyorEvent::HeadMethod:
  case ConveyorEvent::NoMethod:
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


void FileConveyor::DispatchSftp(const ConveyorEvent &evt)
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
  case ConveyorEvent::PutMethod:
    conv_arguments.push_back("-T");
    conv_arguments.push_back(Repath(evt.filename()));
    conv_arguments.push_back(evt.url().toString());
    AddPuttedFile(evt.url().toString()+evt.filename().split("/").back());
    break;

  case ConveyorEvent::DeleteMethod:
    conv_arguments.push_back("-Q");
    conv_arguments.push_back("rm "+evt.url().path());
    conv_arguments.push_back(evt.url().toString(QUrl::RemovePath));
    RemovePuttedFile(evt.url().toString());
    break;

  case ConveyorEvent::PostMethod:  // Should never happen!
  case ConveyorEvent::GetMethod:
  case ConveyorEvent::HeadMethod:
  case ConveyorEvent::NoMethod:
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


void FileConveyor::AddHeaders(QStringList *arglist,const QStringList &hdrs)
{
  for(int i=0;i<hdrs.size();i++) {
    arglist->push_back("-H");
    arglist->push_back(hdrs[i]);
  }
}


void FileConveyor::AddCurlAuthArgs(QStringList *arglist,
				   const ConveyorEvent &evt)
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


QString FileConveyor::Repath(const QString &filename) const
{
  return conv_temp_dir->path()+"/"+filename.split("/").back();
}


void FileConveyor::AddPuttedFile(const QString &url)
{
  for(int i=0;i<conv_putted_files.size();i++) {
    if(url==conv_putted_files[i]) {
      return;
    }
  }
  conv_putted_files.push_back(url);
}


void FileConveyor::RemovePuttedFile(const QString &url)
{
  for(int i=0;i<conv_putted_files.size();i++) {
    if(url==conv_putted_files[i]) {
      conv_putted_files.erase(conv_putted_files.begin()+i);
      return;
    }
  }
}
