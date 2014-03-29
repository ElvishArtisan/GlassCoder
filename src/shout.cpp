// shout.cpp
//
// Shout Routines for glasscoder(1)
//
//   (C) Copyright 2014 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: shout.cpp,v 1.2 2014/02/24 21:02:45 cvs Exp $
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

#include <syslog.h>

#include "connectorfactory.h"
#include "glasscoder.h"

void MainObject::StartShout()
{
  int err;

  //
  // Create Connector Instance
  //
  sir_connector=ConnectorFactory(Connector::Icecast2Server,this);
  connect(sir_connector,SIGNAL(connected(int,const QString &)),
	  this,SLOT(icyConnectedData(int,const QString &)));
  connect(sir_connector,SIGNAL(disconnected()),this,SLOT(icyDisconnectedData()));

  //
  // Set Configuration
  //
  sir_connector->setServerMountpoint(shout_server_mountpoint);
  sir_connector->setServerUsername(shout_server_username);
  sir_connector->setServerPassword(shout_server_password);
  sir_connector->setContentType("audio/mpeg");
  sir_connector->setAudioBitrate(audio_bitrate);
  sir_connector->setAudioChannels(audio_channels);
  sir_connector->setAudioSamplerate(audio_samplerate);
  sir_connector->setStreamDescription(stream_description);
  sir_connector->setStreamGenre(stream_genre);
  sir_connector->setStreamName(stream_name);
  sir_connector->setStreamUrl(stream_url);

  //
  // Open the server connection
  //
  sir_connector->connectToServer(shout_server_hostname,shout_server_port);
}
