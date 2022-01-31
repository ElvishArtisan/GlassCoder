// getconveyor.cpp
//
// Serialized service for processing http GET transactions
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

#include "getconveyor.h"

GetConveyor::GetConveyor(QObject *parent)
  : QObject(parent)
{
  conv_process=NULL;

  //
  // Garbage Timer
  //
  conv_garbage_timer=new QTimer(this);
  conv_garbage_timer->setSingleShot(true);
  connect(conv_garbage_timer,SIGNAL(timeout()),
	  this,SLOT(processCollectGarbageData()));
}


GetConveyor::~GetConveyor()
{
  delete conv_garbage_timer;
  if(conv_process!=NULL) {
    conv_process->kill();
    delete conv_process;
  }
}


void GetConveyor::setUsername(const QString &str)
{
  conv_username=str;
}


void GetConveyor::setPassword(const QString &str)
{
  conv_password=str;
}


void GetConveyor::setUserAgent(const QString &str)
{
  conv_user_agent=str;
}


void GetConveyor::setAddedHeaders(const QStringList &hdrs)
{
  conv_added_headers=hdrs;
}


void GetConveyor::push(const QUrl &url)
{
  conv_events.push(url);
  if(conv_events.size()==1) {
    Dispatch();
  }
}


void GetConveyor::processErrorData(QProcess::ProcessError err)
{
  emit error(conv_events.front(),err,conv_arguments);
  conv_garbage_timer->start(0);
}


void GetConveyor::processFinishedData(int exit_code,
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
      if(conv_process==NULL) {
	emit eventFinished(conv_events.front(),0,200,QStringList());
      }
      else {
	QString response=conv_process->readAllStandardOutput();
	if((conv_events.front().scheme().toLower()=="sftp")&&
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


void GetConveyor::processCollectGarbageData()
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


void GetConveyor::Dispatch()
{
  QUrl url=conv_events.front();
  /*
  printf("Dispatch (via \"%s\"): %s => %s\n",
	 (const char *)evt.url().scheme().toUtf8(),
	 (const char *)evt.filename().toUtf8(),
	 (const char *)evt.url().toString().toUtf8());
  */
  if((url.scheme().toLower()=="http")||
     (url.scheme().toLower()=="https")) {
    conv_arguments.clear();
    AddCurlAuthArgs(&conv_arguments,url);
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
    conv_arguments.push_back(url.toEncoded());
    conv_process=new QProcess(this);
    connect(conv_process,SIGNAL(error(QProcess::ProcessError)),
	    this,SLOT(processErrorData(QProcess::ProcessError)));
    connect(conv_process,SIGNAL(finished(int,QProcess::ExitStatus)),
	    this,SLOT(processFinishedData(int,QProcess::ExitStatus)));
    conv_process->start("curl",conv_arguments);
  }
}


void GetConveyor::AddHeaders(QStringList *arglist,const QStringList &hdrs)
{
  for(int i=0;i<hdrs.size();i++) {
    arglist->push_back("-H");
    arglist->push_back(hdrs[i]);
  }
}


void GetConveyor::AddCurlAuthArgs(QStringList *arglist,const QUrl &url)
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
