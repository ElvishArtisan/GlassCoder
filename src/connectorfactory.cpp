// connectorfactory.cpp
//
// Instantiate Connector classes.
//
//   (C) Copyright 2014 Fred Gleason <fredg@paravelsystems.com>
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

#include "icyconnector.h"
#include "connectorfactory.h"

Connector *ConnectorFactory(Connector::ServerType type,QObject *parent)
{
  Connector *conn=NULL;

  switch(type) {
  case Connector::Shoutcast1Server:
  case Connector::Shoutcast2Server:
  case Connector::Icecast1Server:
    break;

  case Connector::Icecast2Server:
    conn=new IcyConnector(parent);
    break;
  }

  return conn;
}
