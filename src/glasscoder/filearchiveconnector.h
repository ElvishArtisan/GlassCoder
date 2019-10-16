// filearchiveconnector.h
//
// Source connector class for local file archives
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

#ifndef FILEARCHIVECONNECTOR_H
#define FILEARCHIVECONNECTOR_H

#include <QDateTime>
#include <QTimer>

#include <sndfile.h>

#include "connector.h"

#define FILEARCHIVE_DATETIME_PATTERN "yyyy-MM-dd-hh"

class FileArchiveConnector : public Connector
{
  Q_OBJECT;
 public:
  FileArchiveConnector(QObject *parent=0);
  ~FileArchiveConnector();
  FileArchiveConnector::ServerType serverType() const;

 private slots:
  void rotateFile();

 protected:
  void connectToHostConnector(const QUrl &url);
  void disconnectFromHostConnector();
  int64_t writeDataConnector(int frames,const unsigned char *data,int64_t len);

 private:
  bool DidHourAdvance(const QDateTime &dt);
  QString GetFilename(const QDateTime &dt) const;
  QTimer *archive_rotate_timer;
  int archive_fd;
  SNDFILE *archive_snd;
  SF_INFO archive_sf;
  int archive_hour;
};


#endif  // FILEARCHIVECONNECTOR_H
