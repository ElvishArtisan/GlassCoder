// fileconveyor.h
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

#ifndef FILECONVEYOR_H
#define FILECONVEYOR_H

#include <queue>
#include <vector>

#include <curl/curl.h>

#include <QDir>
#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QTimer>

class ConveyorEvent
{
 public:
  enum HttpMethod {NoMethod=0,GetMethod=1,PutMethod=2,DeleteMethod=3};
  ConveyorEvent(void *orig,const QString &filename,const QString &url,
		HttpMethod meth=NoMethod);
  void *originator() const;
  QString filename() const;
  QString url() const;
  ConveyorEvent::HttpMethod method() const;

 private:
  void *evt_originator;
  QString evt_filename;
  QString evt_url;
  ConveyorEvent::HttpMethod evt_method;
};


class FileConveyor : public QObject
{
  Q_OBJECT;
 public:
  FileConveyor(QObject *parent=0);
  ~FileConveyor();
  void setUsername(const QString &str);
  void setPassword(const QString &str);
  void setUserAgent(const QString &str);
  void setAddedHeaders(const QStringList &hdrs);
  void push(const ConveyorEvent &evt);
  void push(void *orig,const QString &url,ConveyorEvent::HttpMethod meth);
  void push(void *orig,const QString &filename,const QString &url,
	    ConveyorEvent::HttpMethod meth);
  void stop();

 signals:
  void eventFinished(const ConveyorEvent &evt,int exit_code,int resp_code,
		     const QStringList &args);
  void error(const ConveyorEvent &evt,QProcess::ProcessError err,
	     const QStringList &args);
  void stopped();

 private slots:
  void processErrorData(QProcess::ProcessError err);
  void processFinishedData(int exit_code,QProcess::ExitStatus exit_status);
  void processCollectGarbageData();
  void nomethodData();

 private:
  void Dispatch();
  void AddHeaders(QStringList *arglist,const QStringList &hdrs);
  void AddCurlAuthArgs(QStringList *arglist,const ConveyorEvent &evt);
  QString Repath(const QString &filename) const;
  void AddPuttedFile(const QString &url);
  void RemovePuttedFile(const QString &url);
  std::queue<ConveyorEvent> conv_events;
  QProcess *conv_process;
  QStringList conv_arguments;
  QTimer *conv_nomethod_timer;
  QTimer *conv_garbage_timer;
  QDir *conv_temp_dir;
  QStringList conv_putted_files;
  QString conv_username;
  QString conv_password;
  QString conv_user_agent;
  QStringList conv_added_headers;
};


#endif  // FILECONVEYOR_H
