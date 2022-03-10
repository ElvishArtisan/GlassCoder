// glasscoder.h
//
// glasscoder(1) Audio Encoder
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

#ifndef GLASSCODER_H
#define GLASSCODER_H

#include <stdint.h>

#include <QObject>
#include <QSocketNotifier>
#include <QStringList>
#include <QTimer>
#include <QUrl>

#include "audiodevice.h"
#include "codec.h"
#include "config.h"
#include "connector.h"
#include "glasslimits.h"
#include "metaserver.h"
#include "ringbuffer.h"

class MainObject : public QObject
{
  Q_OBJECT;
 public:
  MainObject(QObject *parent=0);

 private slots:
  void stdinActivatedData(int sock);
  void audioDeviceStoppedData();
  void connectorStoppedData();
  void meterData();
  void connectedData(bool state);
  void exitTimerData();

 private:
  Config *sir_config;
  //
  // Audio Device
  //
  bool StartAudioDevice();
  Ringbuffer *sir_ringbuffer;
  AudioDevice *sir_audio_device;

  //
  // Server Connection
  //
  void StartServerConnection(const QString &mntpt="");
  std::vector<Connector *> sir_connectors;

  //
  // Codec
  //
  bool StartCodec();
  Codec * sir_codec;

  //
  // Metadata Processor
  //
  MetaServer *sir_meta_server;

  //
  // Miscelaneous
  //
  bool StartSingleStream();
  QTimer *sir_meter_timer;
  QTimer *sir_exit_timer;
  unsigned sir_exit_count;

  //
  // Stdin Processor
  //
  void ProcessCommand(const QString &cmd);
  QSocketNotifier *sir_stdin_notify;
  QString sir_stdin_accum;
};


#endif  // GLASSCODER_H
