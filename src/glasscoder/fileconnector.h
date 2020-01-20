// fileconnector.h
//
// Source connector class for local files
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

#ifndef FILECONNECTOR_H
#define FILECONNECTOR_H

#include <sndfile.h>

#include "connector.h"

class FileConnector : public Connector
{
  Q_OBJECT;
 public:
  FileConnector(QObject *parent=0);
  ~FileConnector();
  FileConnector::ServerType serverType() const;

 protected:
  void connectToHostConnector(const QUrl &url);
  void disconnectFromHostConnector();
  int64_t writeDataConnector(int frames,const unsigned char *data,int64_t len);

 private:
  int file_fd;
  SNDFILE *file_snd;
};


#endif  // FILECONNECTOR_H
