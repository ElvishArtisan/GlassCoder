// glassconv.cpp
//
// glassconv(1) File Conveyor Service
//
//   (C) Copyright 2022 Fred Gleason <fredg@paravelsystems.com>
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

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <QCoreApplication>

#include "cmdswitch.h"
#include "config.h"
#include "glassconv.h"
#include "logging.h"
#include "profile.h"

MainObject::MainObject(QObject *parent)
  : QObject(parent)
{
  d_source_dir=NULL;
  d_dest_url=NULL;

  openlog("glassconv",LOG_PERROR,LOG_DAEMON);

  //
  // Process Command Arguments
  //
  CmdSwitch *cmd=new CmdSwitch("glassconv",GLASSCONV_USAGE);
  for(unsigned i=0;i<cmd->keys();i++) {
    if(cmd->key(i)=="--source-dir") {
      d_source_dir=new QDir(cmd->value(i));
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="--dest-url") {
      d_dest_url=new QUrl(cmd->value(i));
      cmd->setProcessed(i,true);
    }
    if(!cmd->processed(i)) {
      Log(LOG_ERR,"unrecognized option \""+cmd->key(i)+"\"");
      exit(1);
    }
  }
  if(d_source_dir==NULL) {
    Log(LOG_ERR,"\"--source-dir\" argument required");
    exit(1);
  }
  if(d_dest_url==NULL) {
    Log(LOG_ERR,"\"--dest-url\" argument required");
    exit(1);
  }
  if(!d_source_dir->exists()) {
    Log(LOG_ERR,"source directory does not exist");
    exit(1);
  }
  if((!d_dest_url->isValid())||d_dest_url->isRelative()) {
    Log(LOG_ERR,"destination url is invalid");
    exit(1);
  }
  if((d_dest_url->scheme().toLower()!="http")&&
     (d_dest_url->scheme().toLower()!="https")) {
    Log(LOG_ERR,"destination url has unsupported scheme");
    exit(1);
  }

  //
  // Initialize CURL
  //
  if(curl_global_init(CURL_GLOBAL_ALL)!=0) {
    Log(LOG_ERR,"curl global initialization failed");
    exit(1);
  }
  if((d_curl_handle=curl_easy_init())==NULL) {
    Log(LOG_ERR,"curl initialization failed");
    exit(1);
  }

  //
  // Load Credentials
  //
  Profile *p=new Profile();
  if(p->setSource(d_source_dir->path()+"/"+GLASSCODER_CREDENTIALS)) {
    Log(LOG_DEBUG,"reading transfer credentials from \""+p->source()+"\"");
    d_username=p->stringValue("Credentials","Username");
    d_password=p->stringValue("Credentials","Password");
    d_user_agent=p->stringValue("Credentials","UserAgent",
				QString("GlassCoder/")+VERSION);
    UnlinkLocalFile(p->source());
  }
  delete p;

  //
  // Set Directory Filters
  //
  d_source_dir->setFilter(QDir::Files|QDir::Readable|QDir::NoDotAndDotDot);
  d_source_dir->setSorting(QDir::Name);

  //
  // Scan Timer
  //
  d_scan_timer=new QTimer(this);
  d_scan_timer->setSingleShot(true);
  connect(d_scan_timer,SIGNAL(timeout()),this,SLOT(scanData()));

  d_scan_timer->start(0);
}


void MainObject::scanData()
{
  QStringList files=d_source_dir->entryList(QStringList());

  for(int i=0;i<files.size();i++) {
    ProcessFile(d_source_dir->path()+"/"+files.at(i));
  }

  d_scan_timer->start(1000);
}


void MainObject::ProcessFile(const QString &filename)
{
  bool ok=false;
  QStringList f0=filename.split("/",QString::SkipEmptyParts);
  QStringList f1=f0.last().split("-",QString::KeepEmptyParts);

  if(f1.size()<3) {
    Log(LOG_WARNING,
	"unrecognized file naming scheme \""+filename+"\", skipping");
    UnlinkLocalFile(filename);
    return;
  }
  for(int i=3;i<f1.size();i++) {
    f1[2]+="-"+f1.at(i);
  }
  while(f1.size()>3) {
    f1.removeLast();
  }
  f1.first().toUInt(&ok);
  if(ok) {
    QString method=f1.at(1).trimmed();
    if((method!="DELETE")&&(method!="PUT")) {
      Log(LOG_WARNING,
	  "unsupported transfer method in \""+filename+"\", skipping");
      UnlinkLocalFile(filename);
      return;
    }
    QString destname=f1.at(2);
    if(method=="DELETE") {
      DeleteFile(destname,filename);
      UnlinkLocalFile(filename);
    }
    if(method=="PUT") {
      PutFile(destname,filename);
      UnlinkLocalFile(filename);
    }
  }
  else {
    Log(LOG_WARNING,
	"unrecognized file naming scheme \""+filename+"\", skipping");
  }
}


void MainObject::PutFile(const QString &destname,const QString &srcname)
{
  Log(LOG_DEBUG,
      "uploading \""+srcname+"\" to \""+
      d_dest_url->toDisplayString()+"/"+destname+"\"");

  long resp_code=0;
  FILE *f=fopen(srcname.toUtf8(),"r");
  if(f==NULL) {
    Log(LOG_WARNING,
	"upload of \""+srcname+"\" failed: "+strerror(errno));
    return;
  }
  struct stat st;
  memset(&st,0,sizeof(st));
  stat(srcname.toUtf8(),&st);

  QUrl url(d_dest_url->toDisplayString()+"/"+destname);

  curl_easy_reset(d_curl_handle);
  curl_easy_setopt(d_curl_handle,CURLOPT_ERRORBUFFER,d_curl_errorbuffer);
  curl_easy_setopt(d_curl_handle,CURLOPT_READDATA,(void *)f);
  if(st.st_size>0) {
    curl_easy_setopt(d_curl_handle,CURLOPT_INFILESIZE,st.st_size);
  }
  if(!d_username.isEmpty()) {
    curl_easy_setopt(d_curl_handle,CURLOPT_USERPWD,
		     (d_username+":"+d_password).toUtf8().constData());
  }
  curl_easy_setopt(d_curl_handle,CURLOPT_URL,url.toEncoded().constData());
  curl_easy_setopt(d_curl_handle,CURLOPT_UPLOAD,1);
  curl_easy_setopt(d_curl_handle,CURLOPT_FOLLOWLOCATION,1);
  curl_easy_setopt(d_curl_handle,CURLOPT_USERAGENT,
		   d_user_agent.toUtf8().constData());
  CURLcode code=curl_easy_perform(d_curl_handle);
  if(code==CURLE_OK) {
    curl_easy_getinfo(d_curl_handle,CURLINFO_RESPONSE_CODE,&resp_code);
    if((resp_code<200)||(resp_code>=300)) {
      Log(LOG_WARNING,
	  QString::asprintf("upload of \"%s\" returned code %lu",
			    srcname.toUtf8().constData(),resp_code));
    }
  }
  else {
    Log(LOG_WARNING,
	"upload of \""+srcname+"\" failed: "+d_curl_errorbuffer);
  }
}


void MainObject::DeleteFile(const QString &destname,const QString &srcname)
{
  Log(LOG_DEBUG,
      "removing \""+srcname+"\" from \""+
      d_dest_url->toDisplayString()+"/"+destname+"\"");

  long resp_code=0;
  QUrl url(d_dest_url->toDisplayString()+"/"+destname);

  curl_easy_reset(d_curl_handle);
  curl_easy_setopt(d_curl_handle,CURLOPT_ERRORBUFFER,d_curl_errorbuffer);
  if(!d_username.isEmpty()) {
    curl_easy_setopt(d_curl_handle,CURLOPT_USERPWD,
		     (d_username+":"+d_password).toUtf8().constData());
  }
  curl_easy_setopt(d_curl_handle,CURLOPT_URL,url.toEncoded().constData());
  curl_easy_setopt(d_curl_handle,CURLOPT_CUSTOMREQUEST,"DELETE");
  curl_easy_setopt(d_curl_handle,CURLOPT_FOLLOWLOCATION,1);
  curl_easy_setopt(d_curl_handle,CURLOPT_USERAGENT,
		   d_user_agent.toUtf8().constData());
  CURLcode code=curl_easy_perform(d_curl_handle);
  if(code==CURLE_OK) {
    curl_easy_getinfo(d_curl_handle,CURLINFO_RESPONSE_CODE,&resp_code);
    if((resp_code<200)||(resp_code>=300)) {
      Log(LOG_WARNING,
	  QString::asprintf("removal of \"%s\" returned code %lu",
			    srcname.toUtf8().constData(),resp_code));
    }
  }
  else {
    Log(LOG_WARNING,
	"removal of \""+url.toDisplayString()+"\" failed: "+d_curl_errorbuffer);
  }
}


void MainObject::UnlinkLocalFile(const QString &pathname) const
{
  Log(LOG_DEBUG,"unlinking \""+pathname+"\"");
  unlink(pathname.toUtf8());
}


int main(int argv,char *argc[])
{
  QCoreApplication a(argv,argc);

  new MainObject();

  return a.exec();
}
