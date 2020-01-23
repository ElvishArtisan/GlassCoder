// metaserver.cpp
//
// HTTP Server for Metadata Processing
//
// (C) Copyright 2016-2019 Fred Gleason <fredg@paravelsystems.com>
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

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <QUrlQuery>

#include "metaserver.h"

MetaServer::MetaServer(Config *config,QObject *parent)
  : HttpServer(parent)
{
  meta_config=config;
}


void MetaServer::getRequestReceived(HttpConnection *conn)
{
  //printf("HTTP: %s\n",(const char *)conn->dump().toUtf8());
  int resp_code=404;
  QString resp_str="Not Found";
  QUrl url(conn->uri());
  QUrlQuery query(url);

  if(url.path()=="/admin.cgi") {   // Shoutcast Style
    if(query.queryItemValue("pass",QUrl::FullyDecoded)==
       meta_config->serverPassword()) {
      if(query.queryItemValue("mode")=="updinfo") {
	MetaEvent *e=new MetaEvent();
	e->setField("StreamTitle",
		    query.queryItemValue("song",QUrl::FullyDecoded));
	e->setField("StreamUrl",
		    query.queryItemValue("url",QUrl::FullyDecoded));
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
    if(query.queryItemValue("mode")=="updinfo") {
      MetaEvent *e=new MetaEvent();
      e->setField("StreamTitle",
		  query.queryItemValue("song",QUrl::FullyDecoded));
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


void MetaServer::postRequestReceived(HttpConnection *conn)
{
  //  printf("HTTP: %s\n",(const char *)conn->dump().toUtf8());
  int resp_code=404;
  QString resp_str="Not Found";
  QUrl url(conn->uri());

  if(url.path()=="/json_pad") {  // JSON Articulated Metadata
    resp_code=400;
    resp_str="Invalid Data";
    QJsonDocument doc=QJsonDocument::fromJson(conn->body());
    if(!doc.isNull()) {
      if(doc.isObject()) {
	QJsonObject obj=doc.object();
	if(obj.keys().at(0)=="Metadata") {
	  if(ProcessJsonMetadataUpdates(obj.value("Metadata").toObject())) {
	    resp_code=200;
	    resp_str="OK";
	  }
	}
      }
    }
  }

  conn->sendError(resp_code,resp_str);
}


bool MetaServer::authenticateUser(const QString &realm,const QString &name,
				  const QString &passwd)
{
  return true;
}


bool MetaServer::ProcessJsonMetadataUpdates(const QJsonObject &obj)
{
  QStringList keys=obj.keys();
  MetaEvent *e=new MetaEvent();

  for(int i=0;i<keys.size();i++) {
    if(obj.value(keys.at(i)).isString()) {
      e->setField(keys.at(i),obj.value(keys.at(i)).toString());
    }
    else {
      e->setField(keys.at(i),
		  QString().sprintf("%d",obj.value(keys.at(i)).toInt()));
    }
  }
  emit metadataReceived(e);
  delete e;

  return true;
}
