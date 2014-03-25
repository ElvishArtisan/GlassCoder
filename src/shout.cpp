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

#include "glasscoder.h"

bool MainObject::StartShout()
{
  int err;

  //
  // Create Shout Instance
  //
  shout_init();
  if((sir_shout=shout_new())==NULL) {
    syslog(LOG_ERR,"unable to create new shout instance");
    return false;
  }

  //
  // Set Configuration
  //
  if((err=shout_set_host(sir_shout,shout_server_hostname.toAscii()))!=
     SHOUTERR_SUCCESS) {
    syslog(LOG_ERR,"unable to set server hostname [%s]",
	   (const char *)shoutErrorText(err).toAscii());
    return false;
  }
  if((err=shout_set_mount(sir_shout,shout_server_mountpoint.toAscii()))!=
     SHOUTERR_SUCCESS) {
    syslog(LOG_ERR,"unable to set server mountpoint [%s]",
	   (const char *)shoutErrorText(err).toAscii());
    return false;
  }
  if((err=shout_set_port(sir_shout,shout_server_port))!=SHOUTERR_SUCCESS) {
    syslog(LOG_ERR,"unable to set server port [%s]",
	   (const char *)shoutErrorText(err).toAscii());
    return false;
  }
  if((err=shout_set_user(sir_shout,shout_server_username.toAscii()))!=
     SHOUTERR_SUCCESS) {
    syslog(LOG_ERR,"unable to set server username [%s]",
	   (const char *)shoutErrorText(err).toAscii());
    return false;
  }
  if((err=shout_set_password(sir_shout,shout_server_password.toAscii()))!=
     SHOUTERR_SUCCESS) {
    syslog(LOG_ERR,"unable to set server password [%s]",
	   (const char *)shoutErrorText(err).toAscii());
    return false;
  }
  if((err=shout_set_protocol(sir_shout,shout_server_type))!=SHOUTERR_SUCCESS) {
    syslog(LOG_ERR,"unable to set server type/protocol [%s]",
	   (const char *)shoutErrorText(err).toAscii());
    return false;
  }
  if((err=shout_set_format(sir_shout,audio_format))!=SHOUTERR_SUCCESS) {
    syslog(LOG_ERR,"unable to set server audio format [%s]",
	   (const char *)shoutErrorText(err).toAscii());
    return false;
  }
  if((err=shout_set_audio_info(sir_shout,SHOUT_AI_BITRATE,
			 QString().sprintf("%u",audio_bitrate).toAscii()))!=
     SHOUTERR_SUCCESS) {
    syslog(LOG_ERR,"unable to set server audio bitrate [%s]",
	   (const char *)shoutErrorText(err).toAscii());
    return false;
  }
  if((err=shout_set_audio_info(sir_shout,SHOUT_AI_CHANNELS,
			 QString().sprintf("%u",audio_channels).toAscii()))!=
     SHOUTERR_SUCCESS) {
    syslog(LOG_ERR,"unable to set server audio channels [%s]",
	   (const char *)shoutErrorText(err).toAscii());
    return false;
  }
  if((err=shout_set_audio_info(sir_shout,SHOUT_AI_SAMPLERATE,
			 QString().sprintf("%u",audio_samplerate).toAscii()))!=
     SHOUTERR_SUCCESS) {
    syslog(LOG_ERR,"unable to set server audio samplerate [%s]",
	   (const char *)shoutErrorText(err).toAscii());
    return false;
  }
  if(!stream_description.isEmpty()) {
    if((err=shout_set_description(sir_shout,stream_description.toAscii()))!=
       SHOUTERR_SUCCESS) {
      syslog(LOG_ERR,"unable to set server stream description [%s]",
	     (const char *)shoutErrorText(err).toAscii());
      return false;
    }
  }
  if(!stream_genre.isEmpty()) {
    if((err=shout_set_genre(sir_shout,stream_genre.toAscii()))!=
       SHOUTERR_SUCCESS) {
      syslog(LOG_ERR,"unable to set server stream genre [%s]",
	     (const char *)shoutErrorText(err).toAscii());
      return false;
    }
  }
  if(!stream_name.isEmpty()) {
    if((err=shout_set_name(sir_shout,stream_name.toAscii()))!=
       SHOUTERR_SUCCESS) {
      syslog(LOG_ERR,"unable to set server stream name [%s]",
	     (const char *)shoutErrorText(err).toAscii());
      return false;
    }
  }
  if(!stream_url.isEmpty()) {
    if((err=shout_set_url(sir_shout,stream_url.toAscii()))!=SHOUTERR_SUCCESS) {
      syslog(LOG_ERR,"unable to set server stream url [%s]",
	     (const char *)shoutErrorText(err).toAscii());
      return false;
    }
  }

  //
  // Open the server connection
  //
  if((err=shout_open(sir_shout))!=SHOUTERR_SUCCESS) {
    syslog(LOG_ERR,"unable to connect to server [%s]",
	   (const char *)shoutErrorText(err).toAscii());
    return false;
  }

  return true;
}


QString MainObject::shoutErrorText(int err) const
{
  QString ret=tr("Unknown error");

  switch(err) {
  case SHOUTERR_SUCCESS:
    ret=tr("Success");
    break;

  case SHOUTERR_INSANE:
    ret=tr("Internal Fault");
    break;

  case SHOUTERR_CONNECTED:
    ret=tr("Connection Already Open");
    break;

  case SHOUTERR_UNSUPPORTED:
    ret=tr("Unsupported Connection");
    break;

  case SHOUTERR_NOCONNECT:
    ret=tr("Unable to Connect");
    break;

  case SHOUTERR_SOCKET:
    ret=tr("Server Socket Error");
    break;

  case SHOUTERR_NOLOGIN:
    ret=tr("Unable to Authenticate to Server");
    break;

  case SHOUTERR_MALLOC:
    ret=tr("Insufficient Memory");
    break;
  }

  return ret;
}
