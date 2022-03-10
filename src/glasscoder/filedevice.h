// filedevice.h
//
// Audio source for streaming direct from a file.
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

#ifndef FILEDEVICE_H
#define FILEDEVICE_H

#include <stdio.h>

#include <QObject>
#include <QString>
#include <QStringList>
#include <QTimer>

#ifdef SNDFILE
#include <sndfile.h>
#endif  // SNDFILE

#include "audiodevice.h"
#include "meteraverage.h"
#include "ringbuffer.h"

#define SNDFILE_BUFFER_SIZE 1024

class FileDevice : public AudioDevice
{
  Q_OBJECT;
 public:
  FileDevice(unsigned chans,unsigned samprate,
	     Ringbuffer *ring,QObject *parent=0);
  ~FileDevice();
  bool processOptions(QString *err,const QStringList &keys,
		      const QStringList &values);
  bool start(QString *err);
  unsigned deviceSamplerate() const;

 public slots:
  void unmute();

 private slots:
  void readTimerData();

 private:
#ifdef SNDFILE
  QString file_name;
  SNDFILE *file_sndfile;
  SF_INFO file_sfinfo;
  QTimer *file_read_timer;
  MeterAverage *file_meter_avg[MAX_AUDIO_CHANNELS];
  bool file_muted;
#endif  // SNDFILE
};


#endif  // FILEDEVICE_H
