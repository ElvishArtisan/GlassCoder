// metaserver.h
//
// HTTP Server for Metadata Processing
//
// (C) Copyright 2016 Fred Gleason <fredg@paravelsystems.com>
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

#ifndef METASERVER_H
#define METASERVER_H

#include <wh/whhttpserver.h>

#include "config.h"
#include "metaevent.h"

class MetaServer : public WHHttpServer
{
  Q_OBJECT;
 public:
  MetaServer(Config *config,QObject *parent=0);

 signals:
  void metadataReceived(MetaEvent *e);

 protected:
  void getRequestReceived(WHHttpConnection *conn);
  bool authenticateUser(const QString &realm,const QString &name,
			const QString &passwd);

 private:
  Config *meta_config;
};


#endif  // METASERVER_H
