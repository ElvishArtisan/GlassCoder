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
#define HLS_VERSION 6
#define HLS_MINIMUM_SEGMENT_QUAN 4
#define HLS_ID3_HEADER_SIZE 73

#include <stdio.h>

#include <map>

#include <QDir>
#include <QProcess>
#include <QStringList>
#include <QTimer>

#include "connector.h"

class HlsConnector : public Connector
{
  Q_OBJECT;
 public:
  HlsConnector(bool is_top,QObject *parent=0);
  ~HlsConnector();
  Connector::ServerType serverType() const;

 protected:
  void startStopping();
  void connectToHostConnector(const QString &hostname,uint16_t port);
  void disconnectFromHostConnector();
  int64_t writeDataConnector(int frames,const unsigned char *data,int64_t len);

 private slots:
  void putErrorData(QProcess::ProcessError err);
  void putFinishedData(int exit_code,QProcess::ExitStatus exit_status);
  void putCollectGarbageData();
  void deleteErrorData(QProcess::ProcessError err);
  void deleteFinishedData(int exit_code,QProcess::ExitStatus exit_status);
  void deleteCollectGarbageData();
  void stopErrorData(QProcess::ProcessError err);
  void stopFinishedData(int exit_code,QProcess::ExitStatus exit_status);

 private:
  void RotateMediaFile();
  void WritePlaylistFile();
  void WriteTopPlaylistFile();
  QString GetMediaFilename(int seqno);
  void GetStreamTimestamp(uint8_t *bytes,uint64_t frames);
  void AddCurlAuthArgs(QStringList *arglist);
  bool hls_is_top;
  QDir *hls_temp_dir;
  QString hls_playlist_filename;
  int hls_sequence_head;
  int hls_sequence_back;
  std::map<int,double> hls_media_durations;
  std::map<int,uint64_t> hls_media_killtimes;
  QString hls_media_filename;
  QString hls_media_killname;
  FILE *hls_media_handle;
  uint64_t hls_media_frames;
  uint64_t hls_total_media_frames;
  QString hls_put_directory;
  QString hls_put_basename;
  QProcess *hls_put_process;
  QStringList hls_put_args;
  QTimer *hls_put_garbage_timer;
  QProcess *hls_delete_process;
  QStringList hls_delete_args;
  QTimer *hls_delete_garbage_timer;
  QProcess *hls_stop_process;
  QStringList hls_stop_args;
};


#endif  // HLSCONNECTOR_H
