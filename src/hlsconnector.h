// hlsconnector.h
//
// HLS/HTTP streaming connector for GlassCoder
//
//   (C) Copyright 2014-2015 Fred Gleason <fredg@paravelsystems.com>
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

#ifndef HLSCONNECTOR_H
#define HLSCONNECTOR_H

#define HLS_SEGMENT_SIZE 10
#define HLS_VERSION 3

#include <stdio.h>

#include <QtCore/QDir>

#include "connector.h"

class HlsConnector : public Connector
{
  Q_OBJECT;
 public:
  HlsConnector(QObject *parent=0);
  ~HlsConnector();
  Connector::ServerType serverType() const;
  void connectToServer(const QString &hostname,uint16_t port);

 protected:
  void connectToHostConnector(const QString &hostname,uint16_t port);
  void disconnectFromHostConnector();
  int64_t writeDataConnector(int frames,const unsigned char *data,int64_t len);

 private:
  void RotateMediaFile();
  QDir *hls_temp_dir;
  QString hls_playlist_filename;
  FILE *hls_playlist_handle;
  int hls_sequence_number;
  QString hls_media_filename;
  FILE *hls_media_handle;
  int hls_media_frames;
};


#endif  // HLSCONNECTOR_H
