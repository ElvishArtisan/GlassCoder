// connectorfactory.h
//
// Instantiate Connector classes
//
//   (C) Copyright 2014-2022 Fred Gleason <fredg@paravelsystems.com>
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

#ifndef CONNECTORFACTORY_H
#define CONNECTORFACTORY_H

#include "config.h"
#include "connector.h"

Connector *ConnectorFactory(Connector::ServerType type,Config *conf,
			    QObject *parent=0);


#endif  // CONNECTORFACTORY_H
