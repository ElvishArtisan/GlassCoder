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

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <iostream>
#include <fstream>

#include <QCoreApplication>

#ifdef HAVE_AWS_S3
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/s3/model/DeleteObjectRequest.h>
#include <aws/s3/model/ListObjectsV2Request.h>
#endif  // HAVE_AWS_S3

#include "cmdswitch.h"
#include "glassconv.h"
#include "hlsconnector.h"
#include "profile.h"

MainObject::MainObject(QObject *parent)
  : QObject(parent)
{
  d_source_dir=NULL;
  d_dest_url=NULL;

  int syslog_option=0;

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
    if(cmd->key(i)=="--debug") {
      syslog_option+=LOG_PERROR;
      cmd->setProcessed(i,true);
    }
    if(!cmd->processed(i)) {
      fprintf(stderr,"unrecognized option \"%s\"",
	      cmd->key(i).toUtf8().constData());
      CleanExit(Config::ExitFatal);
    }
  }
  openlog("glassconv",syslog_option,LOG_DAEMON);
  Log(LOG_DEBUG,"starting up");
  if(d_source_dir==NULL) {
    Log(LOG_ERR,"\"--source-dir\" argument required");
    CleanExit(Config::ExitFatal);
  }
  if(d_dest_url==NULL) {
    Log(LOG_ERR,"\"--dest-url\" argument required");
    CleanExit(Config::ExitFatal);
  }
  if(!d_source_dir->exists()) {
    Log(LOG_ERR,"source directory does not exist");
    CleanExit(Config::ExitFatal);
  }
  if((!d_dest_url->isValid())||d_dest_url->isRelative()) {
    Log(LOG_ERR,"destination url is invalid");
    CleanExit(Config::ExitFatal);
  }
#ifdef HAVE_AWS_S3
  if((d_dest_url->scheme().toLower()!="http")&&
     (d_dest_url->scheme().toLower()!="https")&&
     (d_dest_url->scheme().toLower()!="sftp")&&
     (d_dest_url->scheme().toLower()!="file")&&
     (d_dest_url->scheme().toLower()!="s3")) {
    Log(LOG_ERR,"destination url has unsupported scheme");
    CleanExit(Config::ExitFatal);
  }
#else
  if((d_dest_url->scheme().toLower()!="http")&&
     (d_dest_url->scheme().toLower()!="https")&&
     (d_dest_url->scheme().toLower()!="sftp")&&
     (d_dest_url->scheme().toLower()!="file")) {
    Log(LOG_ERR,"destination url has unsupported scheme");
    CleanExit(Config::ExitFatal);
  }
#endif  // HAVE_AWS_S3

  //
  // Initialize CURL
  //
  if(curl_global_init(CURL_GLOBAL_ALL)!=0) {
    Log(LOG_ERR,"curl global initialization failed");
    CleanExit(Config::ExitRetry);
  }
  if((d_curl_handle=curl_easy_init())==NULL) {
    Log(LOG_ERR,"curl initialization failed");
    CleanExit(Config::ExitRetry);
  }

  //
  // Initialize AWS S3 API
  //
#ifdef HAVE_AWS_S3
  if(d_dest_url->scheme().toLower()=="s3") {
    Aws::InitAPI(d_aws_options);
  }
#endif  // HAVE_AWS_S3

  //
  // Load Credentials
  //
  Profile *p=new Profile();
  if(p->setSource(d_source_dir->path()+"/"+GLASSCODER_CREDENTIALS)) {
    Log(LOG_DEBUG,"reading transfer credentials from \"%s\"",
	   p->source().toUtf8().constData());
    d_username=p->stringValue("Credentials","Username");
    d_password=p->stringValue("Credentials","Password");
    d_ssh_identity=p->stringValue("Credentials","SshIdentity");
    d_user_agent=p->stringValue("Credentials","UserAgent",
				QString("GlassCoder/")+VERSION);
    if(!p->stringValue("Credentials","PrecleanUrl").isEmpty()) {
      d_preclean_url=QUrl(p->stringValue("Credentials","PrecleanUrl"));
    }
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

  //
  // Preclean PublishPoint
  //
#ifdef HAVE_AWS_S3
  if(d_preclean_url.scheme().toLower()=="s3") {
    CleanS3Bucket(d_preclean_url);
  }
#endif  // HAVE_AWS_S3

  //
  // Signal Disposition
  //
  ::signal(SIGTERM,SIG_IGN);
  ::signal(SIGINT,SIG_IGN);

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
  QStringList f0=filename.split("/",QString::SkipEmptyParts);
  QStringList f1=f0.last().split("-",QString::KeepEmptyParts);

  if(f1.size()<3) {
    Log(LOG_WARNING,
	   "unrecognized file naming scheme \"%s\", skipping",
	   filename.toUtf8().constData());
    UnlinkLocalFile(filename);
    return;
  }
  for(int i=3;i<f1.size();i++) {
    f1[2]+="-"+f1.at(i);
  }
  while(f1.size()>3) {
    f1.removeLast();
  }
  QString method=f1.at(1).trimmed();
  QString destname=f1.at(2);
  Log(LOG_NOTICE,"method: %s  destname: %s",method.toUtf8().constData(),
      destname.toUtf8().constData());
  if(method=="DELETE") {
    Delete(destname);
    UnlinkLocalFile(filename);
    return;
  }
  if(method=="PUT") {
    Put(destname,filename);
    UnlinkLocalFile(filename);
    return;
  }
  if(method=="STOP") {
    //
    // Clean up temp directory
    //
    QStringList files=d_source_dir->entryList(QDir::Files);
    for(int i=0;i<files.size();i++) {
      unlink((d_source_dir->path()+"/"+files.at(i)).toUtf8());
    }
    rmdir(d_source_dir->path().toUtf8());

    Log(LOG_DEBUG,"exiting normally");
    CleanExit(Config::ExitOk);
  }
  Log(LOG_WARNING,
	 "unsupported transfer method in \"%s\", skipping",
	 filename.toUtf8().constData());
  UnlinkLocalFile(filename);
}


void MainObject::Put(const QString &destname,const QString &srcname)
{
  Log(LOG_DEBUG,"uploading \"%s\" to \"%s/%s\"",
	 srcname.toUtf8().constData(),
	 d_dest_url->toDisplayString().toUtf8().constData(),
	 destname.toUtf8().constData());

  QString scheme=d_dest_url->scheme().toLower();

  if((scheme=="http")||(scheme=="https")||(scheme=="file")||(scheme=="sftp")) {
    PutCurl(destname,srcname);
  }
  if(scheme=="s3") {
    PutAwsS3(destname,srcname);
  }
}


void MainObject::PutCurl(const QString &destname,const QString &srcname)
{
  long resp_code=0;
  FILE *f=fopen(srcname.toUtf8(),"r");
  if(f==NULL) {
    Log(LOG_WARNING,"upload of \"%s\" failed: %s",
	   srcname.toUtf8().constData(),strerror(errno));
    return;
  }
  struct stat st;
  memset(&st,0,sizeof(st));
  stat(srcname.toUtf8(),&st);

  QUrl url(d_dest_url->toDisplayString()+"/"+destname);

  curl_easy_reset(d_curl_handle);

  //
  // Authentication
  //
  SetCurlAuthentication(d_curl_handle);

  //
  // Transaction
  //
  curl_easy_setopt(d_curl_handle,CURLOPT_ERRORBUFFER,d_curl_errorbuffer);
  curl_easy_setopt(d_curl_handle,CURLOPT_READDATA,(void *)f);
  if(st.st_size>0) {
    curl_easy_setopt(d_curl_handle,CURLOPT_INFILESIZE,st.st_size);
  }
  curl_easy_setopt(d_curl_handle,CURLOPT_URL,url.toEncoded().constData());
  curl_easy_setopt(d_curl_handle,CURLOPT_UPLOAD,1);
  curl_easy_setopt(d_curl_handle,CURLOPT_FOLLOWLOCATION,1);
  curl_easy_setopt(d_curl_handle,CURLOPT_USERAGENT,
		   d_user_agent.toUtf8().constData());
  CURLcode code=curl_easy_perform(d_curl_handle);
  if(code==CURLE_OK) {
    curl_easy_getinfo(d_curl_handle,CURLINFO_RESPONSE_CODE,&resp_code);
    if(((resp_code<200)||(resp_code>=300))&&(resp_code!=0)) {
      Log(LOG_WARNING,"upload of \"%s\" returned code %lu",
	     srcname.toUtf8().constData(),resp_code);
    }
  }
  else {
    Log(LOG_WARNING,"upload of \"%s\" failed: %s",
	   srcname.toUtf8().constData(),d_curl_errorbuffer);
  }
  fclose(f);
}


void MainObject::PutAwsS3(const QString &destname,const QString &srcname)
{
#ifdef HAVE_AWS_S3
  QStringList f0=d_dest_url->toDisplayString().split("/");
  QString bucket=f0.last();
  QString key=destname;

  Aws::S3::S3ClientConfiguration config;
  config.profileName=d_username.toUtf8().constData();
  Aws::S3::S3Client client(config);

  Aws::S3::Model::PutObjectRequest request;
  request.SetBucket(bucket.toUtf8().constData());
  request.SetKey(key.toUtf8().constData());
  SetS3FileMetadata(request,key);
  std::shared_ptr<Aws::IOStream> in=
    Aws::MakeShared<Aws::FStream>("SomeTag",srcname.toUtf8().constData(),
				  std::ios_base::in|std::ios_base::binary);
  if(!*in) {
    Log(LOG_WARNING,"unable to read file \"%s\"",srcname.toUtf8().constData());
    return;
  }
  request.SetBody(in);
  Aws::S3::Model::PutObjectOutcome out=client.PutObject(request);
  if(!out.IsSuccess()) {
    auto err=out.GetError();
    Log(LOG_WARNING,"S3 PutObject to \"%s\%s\" failed [%s]",
	bucket.toUtf8().constData(),key.toUtf8().constData(),
	err.GetMessage().c_str());

  }
#else  // HAVE_AWS_S3
  Log(LOG_WARNING,"AWS S3 support has not been enabled, ignoring PUT request");
#endif  // HAVE_AWS_S3
}


void MainObject::Delete(const QString &destname)
{
  Log(LOG_DEBUG,"removing \"%s/%s\"",
	 d_dest_url->toDisplayString().toUtf8().constData(),
	 destname.toUtf8().constData());

  QString scheme=d_dest_url->scheme().toLower();

  if((scheme=="http")||(scheme=="https")) {
    DeleteHttp(destname);
  }
  if(scheme=="file") {
    DeleteFile(destname);
  }
  if(scheme=="sftp") {
    DeleteSftp(destname);
  }
  if(scheme=="s3") {
    DeleteAwsS3(destname);
  }
}


void MainObject::DeleteHttp(const QString &destname)
{
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
      Log(LOG_WARNING,"removal of \"%s\" returned code %lu",
	     destname.toUtf8().constData(),resp_code);
    }
  }
  else {
    Log(LOG_WARNING,"removal of \"%s\" failed: %s",
	   url.toDisplayString().toUtf8().constData(),d_curl_errorbuffer);
  }
}


void MainObject::DeleteFile(const QString &destname)
{
  QUrl url(d_dest_url->toDisplayString()+"/"+destname);
  if((unlink(url.path().toUtf8())!=0)&&(errno!=ENOENT)) {
    Log(LOG_WARNING,"removal of \"%s\" failed: %s",
	   url.toDisplayString().toUtf8().constData(),strerror(errno));
  }
}


void MainObject::DeleteSftp(const QString &destname)
{
  struct curl_slist *cmds=NULL;
  QUrl url(d_dest_url->toDisplayString()+"/"+destname);

  //
  // Set Up The Transaction
  //
  curl_easy_reset(d_curl_handle);
  SetCurlAuthentication(d_curl_handle);
  curl_easy_setopt(d_curl_handle,CURLOPT_URL,url.toEncoded().constData());
  curl_easy_setopt(d_curl_handle,CURLOPT_HTTPAUTH,CURLAUTH_ANY);
  curl_easy_setopt(d_curl_handle,CURLOPT_USERAGENT,
		   d_user_agent.toUtf8().constData());
  cmds=curl_slist_append(cmds,(QString("rm ")+url.path()).toUtf8());
  curl_easy_setopt(d_curl_handle,CURLOPT_QUOTE,cmds);

  //
  // Execute It
  //
  CURLcode code=curl_easy_perform(d_curl_handle);
  if((code!=CURLE_OK)&&(code!=CURLE_REMOTE_FILE_NOT_FOUND)) {
    Log(LOG_WARNING,"removal of \"%s\" failed: [%d] %s",
	   url.toDisplayString().toUtf8().constData(),code,d_curl_errorbuffer);
  }
}


void MainObject::DeleteAwsS3(const QString &destname) const
{
#ifdef HAVE_AWS_S3
  QStringList f0=d_dest_url->toDisplayString().split("/");
  QString bucket=f0.last();
  QString key=destname;
  
  Aws::S3::S3ClientConfiguration config;
  Aws::S3::S3Client client(config);
  //  config.region="us-east-1";
  Aws::S3::Model::DeleteObjectRequest request;
  request.SetBucket(bucket.toUtf8().constData());
  request.SetKey(key.toUtf8().constData());
  Aws::S3::Model::DeleteObjectOutcome out=client.DeleteObject(request);
  if(!out.IsSuccess()) {
    auto err=out.GetError();
    Log(LOG_WARNING,"S3 DeleteObject on \"%s\%s\" failed [%s]",
	bucket.toUtf8().constData(),key.toUtf8().constData(),
	err.GetMessage().c_str());

  }
  //  Aws::ShutdownAPI(options);
#else  // HAVE_AWS_S3
  Log(LOG_WARNING,
      "AWS S3 support has not been enabled, ignoring DELETE request");
#endif  // HAVE_AWS_S3
}


void MainObject::SetCurlAuthentication(CURL *handle) const
{
  if(d_ssh_identity.isEmpty()) {
    curl_easy_setopt(d_curl_handle,CURLOPT_USERPWD,
		     (d_username+":"+d_password).toUtf8().constData());
  }
  else {
    curl_easy_setopt(d_curl_handle,
		     CURLOPT_USERNAME,d_username.toUtf8().constData());
    curl_easy_setopt(d_curl_handle,CURLOPT_SSH_PRIVATE_KEYFILE,
		     d_ssh_identity.toUtf8().constData());
    curl_easy_setopt(d_curl_handle,CURLOPT_KEYPASSWD,
		     d_password.toUtf8().constData());
  }
}


void MainObject::UnlinkLocalFile(const QString &pathname) const
{
  Log(LOG_DEBUG,"unlinking \"%s\"",pathname.toUtf8().constData());
  unlink(pathname.toUtf8());
}


void MainObject::Log(int prio,const char *fmt,...) const
{
  char line[1024];
  va_list args;

  //
  // Send to parent controller
  //
  va_start(args,fmt);
  if(vsnprintf(line,1023,fmt,args)>0) {
    printf("ER %d %s",prio,line);
    fflush(stdout);
  }
  va_end(args);

  //
  // Send to syslog
  //
  va_start(args,fmt);
  vsyslog(prio,fmt,args);
  va_end(args);
}


void MainObject::SetS3FileMetadata(Aws::S3::Model::PutObjectRequest &request,
				 const QString &filename) const
{
  QStringList f0=filename.split(".");
  QString ext=f0.last().toLower();

  if(ext=="m3u8") {
    request.SetContentType("application/vnd.apple.mpegurl");
    request.SetCacheControl(QString::asprintf("max-age=%d",HLS_SEGMENT_SIZE).
			    toUtf8().constData());
  }
  if(ext=="aac") {
    request.SetContentType("audio/aac");
    request.SetCacheControl("max-age=3600, stale-if-error=86400");
  }
}


void MainObject::CleanS3Bucket(const QUrl &bucket_prefix) const
{
  QStringList f0=ListS3Objects(bucket_prefix.path().split("/").last());
  for(int i=0;i<f0.size();i++) {
    DeleteAwsS3(f0.at(i));
  }
  if(f0.size()>0) {
    Log(LOG_INFO,"cleaned up %d stale files on publish point");
  }
}


QStringList MainObject::ListS3Objects(const QString &prefix) const
{
  QStringList f0=d_dest_url->toDisplayString().split("/");
  QString bucket=f0.last();
  QStringList objs;
  
  Aws::Vector<Aws::S3::Model::Object> objects;
  Aws::S3::S3ClientConfiguration config;
  Aws::S3::S3Client client(config);
  Aws::S3::Model::ListObjectsV2Request request;
  request.SetBucket(bucket.toUtf8().constData());
  request.SetPrefix(prefix.toUtf8().constData());
  Aws::String ctk;

  do {
    if(!ctk.empty()) {
      request.SetContinuationToken(ctk);
    }
    auto out=client.ListObjectsV2(request);
    if(out.IsSuccess()) {
      auto contents=out.GetResult().GetContents();
      for(int i=0;i<contents.size();i++) {
	objs.push_back(contents.at(i).GetKey().c_str());
      }
    }
    else {
      Log(LOG_WARNING,"failed to enumerate bucket \"%s\" [%s]",
	  bucket.toUtf8().constData(),out.GetError().GetMessage().c_str());
      return objs;
    }
  } while(!ctk.empty());

  return objs;
}


void MainObject::CleanExit(Config::ExitCode exit_code) const
{
#ifdef HAVE_AWS_S3
  if(d_dest_url->scheme().toLower()=="s3") {
    Aws::ShutdownAPI(d_aws_options);
  }
#endif  // HAVE_AWS_S3
  exit(exit_code);
}


int main(int argv,char *argc[])
{
  QCoreApplication a(argv,argc);

  new MainObject();

  return a.exec();
}
