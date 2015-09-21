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

#include "fileconveyor.h"

ConveyorEvent::ConveyorEvent(const QString &filename,const QString &url,
			     const QString &username,const QString &passwd,
			     HttpMethod meth)
{
  evt_filename=filename;
  evt_url=url;
  evt_username=username;
  evt_password=passwd;
  evt_method=meth;
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


QString ConveyorEvent::username() const
{
  return evt_username;
}


QString ConveyorEvent::password() const
{
  return evt_password;
}




FileConveyor::FileConveyor(QObject *parent)
  : QObject(parent)
{
  conv_process=NULL;

  conv_nomethod_timer=new QTimer(this);
  conv_nomethod_timer->setSingleShot(true);
  connect(conv_nomethod_timer,SIGNAL(timeout()),this,SLOT(nomethodData()));

  conv_garbage_timer=new QTimer(this);
  conv_garbage_timer->setSingleShot(true);
  connect(conv_garbage_timer,SIGNAL(timeout()),
	  this,SLOT(processCollectGarbageData()));
}


FileConveyor::~FileConveyor()
{
  delete conv_garbage_timer;
  if(conv_process!=NULL) {
    conv_process->kill();
    delete conv_process;
  }
}


void FileConveyor::push(const ConveyorEvent &evt)
{
  conv_events.push(evt);
  if(conv_events.size()==1) {
    Dispatch();
  }
}


void FileConveyor::push(const QString &filename,const QString &url,
			const QString &username,const QString &passwd,
			ConveyorEvent::HttpMethod meth)
{
  ConveyorEvent evt(filename,url,username,passwd,meth);
  push(evt);
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
  case ConveyorEvent::NoMethod:  // Simply pass this through
    conv_nomethod_timer->start(0);
    break;

  case ConveyorEvent::GetMethod:
    conv_arguments.push_back(evt.url());
    break;

  case ConveyorEvent::PutMethod:
    conv_arguments.push_back("-T");
    conv_arguments.push_back(evt.filename());
    conv_arguments.push_back(evt.url());
    break;

  case ConveyorEvent::DeleteMethod:
    conv_arguments.push_back("-X");
    conv_arguments.push_back("DELETE");
    conv_arguments.push_back(evt.url());
    break;
  }

  printf("curl: %s\n",(const char *)conv_arguments.join(" ").toUtf8());

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
  if(!evt.username().isEmpty()) {
    arglist->push_back("-u");
    if(evt.password().isEmpty()) {
      arglist->push_back(evt.username());
    }
    else {
      arglist->push_back(evt.username()+":"+evt.password());
    }
  }
}
