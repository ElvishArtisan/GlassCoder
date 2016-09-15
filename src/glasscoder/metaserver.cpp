// metaserver.cpp
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

#include <QUrl>

#include "metaserver.h"

MetaServer::MetaServer(Config *config,QObject *parent)
  : WHHttpServer(parent)
{
  meta_config=config;
}


void MetaServer::getRequestReceived(WHHttpConnection *conn)
{
  //printf("HTTP: %s\n",(const char *)conn->dump().toUtf8());
  int resp_code=404;
  QString resp_str="Not Found";
  QUrl url(conn->uri());

  if(url.path()=="/admin.cgi") {   // Shoutcast Style
    if(url.queryItemValue("pass")==meta_config->serverPassword()) {
      if(url.queryItemValue("mode")=="updinfo") {
	MetaEvent *e=new MetaEvent();
	e->setField(MetaEvent::StreamTitle,url.queryItemValue("song"));
	emit metadataReceived(e);
	delete e;
	conn->sendError(200,"OK");
	return;
      }
      else {
	resp_code=403;
	resp_str="Unrecognized Mode";
      }
    }
    else {
      resp_code=403;
      resp_str="Bad Password";
    }
  }

  if(url.path()=="/admin/metadata") {   // Icecast Style
    if(url.queryItemValue("mode")=="updinfo") {
      MetaEvent *e=new MetaEvent();
      e->setField(MetaEvent::StreamTitle,url.queryItemValue("song"));
      emit metadataReceived(e);
      delete e;
      conn->sendError(200,"OK");
      return;
    }
    else {
      resp_code=403;
      resp_str="Unrecognized Mode";
    }
  }

  conn->sendError(resp_code,resp_str);
}


bool MetaServer::authenticateUser(const QString &realm,const QString &name,
				  const QString &passwd)
{
  return true;
}
