// getconveyor.h
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

#ifndef GETCONVEYOR_H
#define GETCONVEYOR_H

#include <queue>

#include <curl/curl.h>

#include <QDir>
#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QUrl>

class GetConveyor : public QObject
{
  Q_OBJECT;
 public:
  GetConveyor(QObject *parent=0);
  ~GetConveyor();
  void setUsername(const QString &str);
  void setPassword(const QString &str);
  void setUserAgent(const QString &str);
  void setAddedHeaders(const QStringList &hdrs);
  void push(const QUrl &url);

 signals:
  void eventFinished(const QUrl &url,int exit_code,int resp_code,
		     const QStringList &args);
  void error(const QUrl &url,QProcess::ProcessError err,
	     const QStringList &args);

 private slots:
  void processErrorData(QProcess::ProcessError err);
  void processFinishedData(int exit_code,QProcess::ExitStatus exit_status);
  void processCollectGarbageData();

 private:
  void Dispatch();
  void AddHeaders(QStringList *arglist,const QStringList &hdrs);
  void AddCurlAuthArgs(QStringList *arglist,const QUrl &url);
  std::queue<QUrl> conv_events;
  QProcess *conv_process;
  QStringList conv_arguments;
  QTimer *conv_garbage_timer;
  QString conv_username;
  QString conv_password;
  QString conv_user_agent;
  QStringList conv_added_headers;
};


#endif  // GETCONVEYOR_H
