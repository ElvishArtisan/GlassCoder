// fileconveyor.cpp
//
// Serialized service for uploading files
//
//   (C) Copyright 2015 Fred Gleason <fredg@paravelsystems.com>
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
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>

#include "fileconveyor.h"

ConveyorEvent::ConveyorEvent(void *orig,const QString &filename,const QString &url,
			     HttpMethod meth)
{
  evt_originator=orig;
  evt_filename=filename;
  evt_url=url;
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


QString ConveyorEvent::url() const
{
  return evt_url;
}


ConveyorEvent::HttpMethod ConveyorEvent::method() const
{
  return evt_method;
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


void FileConveyor::push(const ConveyorEvent &evt)
{
  if(!evt.filename().isEmpty()) {
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


void FileConveyor::push(void *orig,const QString &filename,const QString &url,
			ConveyorEvent::HttpMethod meth)
{
  ConveyorEvent evt(orig,filename,url,meth);
  push(evt);
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
  if(exit_status==QProcess::CrashExit) {
    emit eventFinished(conv_events.front(),256,0,conv_arguments);
  }
  else {
    if(exit_code!=0) {
      emit eventFinished(conv_events.front(),exit_code,0,conv_arguments);
    }
    else {
      QString response=conv_process->readAllStandardOutput();
      emit eventFinished(conv_events.front(),0,response.toInt(),conv_arguments);
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


void FileConveyor::Dispatch()
{
  ConveyorEvent evt=conv_events.front();

  conv_arguments.clear();
  AddCurlAuthArgs(&conv_arguments,evt);
  conv_arguments.push_back("--write-out");
  conv_arguments.push_back("%{http_code}");
  conv_arguments.push_back("--silent");
  conv_arguments.push_back("--output");
  conv_arguments.push_back("/dev/null");

  switch(evt.method()) {
  case ConveyorEvent::NoMethod:  // Shutting down
    emit stopped();
    return;

  case ConveyorEvent::GetMethod:
    conv_arguments.push_back(evt.url());
    break;

  case ConveyorEvent::PutMethod:
    conv_arguments.push_back("-T");
    conv_arguments.push_back(Repath(evt.filename()));
    conv_arguments.push_back(evt.url());
    AddPuttedFile(evt.url()+evt.filename().split("/").back());
    break;

  case ConveyorEvent::DeleteMethod:
    conv_arguments.push_back("-X");
    conv_arguments.push_back("DELETE");
    conv_arguments.push_back(evt.url());
    RemovePuttedFile(evt.url());
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
