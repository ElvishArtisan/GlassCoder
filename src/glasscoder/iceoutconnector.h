// iceoutconnector.h
//
// Glasscoder connector for a single IceCast stream
//
//   (C) Copyright 2014-2016 Fred Gleason <fredg@paravelsystems.com>
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

#ifndef ICEOUTCONNECTOR_H
#define ICEOUTCONNECTOR_H

#include "connector.h"

class IceOutConnector : public Connector
{
  Q_OBJECT;
 public:
  IceOutConnector(QObject *parent=0);
  ~IceOutConnector();
  Connector::ServerType serverType() const;

 protected:
  void startStopping();
  void connectToHostConnector(const QUrl &url);
  void disconnectFromHostConnector();
  int64_t writeDataConnector(int frames,const unsigned char *data,int64_t len);

 private:
  void SendHeader(const QString &hdr="") const;
  void StartStream();
};


#endif  // ICEOUTCONNECTOR_H
