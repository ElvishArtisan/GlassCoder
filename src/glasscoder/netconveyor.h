// netconveyor.h
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

#ifndef NETCONVEYOR_H
#define NETCONVEYOR_H

#include <QDir>
#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QTimer>

#include "config.h"

class NetConveyorEvent
{
 public:
  //  enum HttpMethod {NoMethod=0,GetMethod=1,PutMethod=2,DeleteMethod=3,
  //		   PostMethod=4,HeadMethod=5,StopMethod=6};
  enum HttpMethod {PutMethod=0,DeleteMethod=1,StopMethod=2};
  NetConveyorEvent(void *orig,const QString &pathname,HttpMethod meth);
  void *originator() const;
  QString pathname() const;
  NetConveyorEvent::HttpMethod method() const;
  QString dump() const;
  static QString httpMethodString(HttpMethod method);

 private:
  void *evt_originator;
  QString evt_pathname;
  NetConveyorEvent::HttpMethod evt_method;
};


class NetConveyor : public QObject
{
  Q_OBJECT;
 public:
  NetConveyor(Config *conf,QObject *parent=0);
  ~NetConveyor();
  void push(void *orig,const QString &pathname,
	    NetConveyorEvent::HttpMethod meth);
  void push(const NetConveyorEvent &evt);
  void stop();

 signals:
  void stopped();

 private slots:
  void startConveyorProcess();
  void processFinishedData(int exit_code,QProcess::ExitStatus exit_status);
  void processReadyReadData();

 private:
  QProcess *conv_process;
  QTimer *conv_restart_timer;
  QDir *conv_temp_dir;
  QStringList conv_putted_files;
  Config *conv_config;
};


#endif  // NETCONVEYOR_H
