// asihpidevice.h
//
// Audio source for AudioScience HPI devices
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

#ifndef ASIHPIDEVICE_H
#define ASIHPIDEVICE_H

#include <stdint.h>
#include <syslog.h>

#ifdef ASIHPI
#include <asihpi/hpi.h>
#endif  // ASIHPI

#include <QTimer>

#include "audiodevice.h"

#define ASIHPI_DEFAULT_INDEX 0
#define ASIHPI_DEFAULT_INPUT_INDEX 0
#define ASIHPI_READ_INTERVAL 10

class AsiHpiDevice : public AudioDevice
{
  Q_OBJECT;
 public:
  AsiHpiDevice(unsigned chans,unsigned samprate,
	       Ringbuffer *ring,QObject *parent=0);
  ~AsiHpiDevice();
  bool isAvailable() const;
  bool processOptions(QString *err,const QStringList &keys,
		      const QStringList &values);
  bool start(QString *err);

 private slots:
  void readData();
  void meterData();

 private:
#ifdef ASIHPI
  void MakeFormat(struct hpi_format *fmt,uint16_t hfmt);
  hpi_err_t HpiLog(hpi_err_t err,int priority=LOG_DEBUG) const;
  const char *hpi_strerror(hpi_err_t err) const;

  //
  // Arguments
  //
  uint16_t asihpi_adapter_index;
  uint16_t asihpi_input_index;
  uint16_t asihpi_input_mode;
  uint16_t asihpi_input_gain;
  uint16_t asihpi_channel_mode;
  uint16_t asihpi_input_source;
  uint16_t asihpi_input_type;
  hpi_handle_t asihpi_input_stream;
  hpi_handle_t asihpi_mixer;
  hpi_handle_t asihpi_input_meter;
  uint8_t *asihpi_pcm_buffer;
  QTimer *asihpi_read_timer;
  QTimer *asihpi_meter_timer;
  uint32_t asihpi_dma_buffer_size;
#endif  // ASIHPI
};


#endif  // ASIHPIDEVICE_H
