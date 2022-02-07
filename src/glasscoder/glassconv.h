// glassconv.h
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

#ifndef GLASSCONV_H
#define GLASSCONV_H

#include <stdint.h>

#include <curl/curl.h>

#include <QDir>
#include <QObject>
#include <QTimer>
#include <QUrl>

#define GLASSCONV_USAGE "--source-dir=<dir> --dest-url=<url> [--debug]"

class MainObject : public QObject
{
  Q_OBJECT;
 public:
  MainObject(QObject *parent=0);

 private slots:
  void scanData();

 private:
  void ProcessFile(const QString &filename);
  void Put(const QString &destname,const QString &srcname);
  void Delete(const QString &destname,const QString &srcname);
  void DeleteHttp(const QString &destname,const QString &srcname);
  void DeleteFile(const QString &destname,const QString &srcname);
  void DeleteSftp(const QString &destname,const QString &srcname);
  void SetCurlAuthentication(CURL *handle) const;
  void UnlinkLocalFile(const QString &pathname) const;
  void Log(int prio,const char *fmt,...) const;
  QDir *d_source_dir;
  QUrl *d_dest_url;
  QString d_username;
  QString d_password;
  QString d_ssh_identity;
  QTimer *d_scan_timer;
  CURL *d_curl_handle;
  char d_curl_errorbuffer[CURL_ERROR_SIZE];
  QString d_user_agent;
};


#endif  // GLASSCONV_H
