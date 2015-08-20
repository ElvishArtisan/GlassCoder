// asihpidevice.cpp
//
// Audio source for AudioScience HPI devices
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

#include <string.h>

#include <stdio.h>

#include "asihpidevice.h"

AsiHpiDevice::AsiHpiDevice(unsigned chans,unsigned samprate,
			   std::vector<Ringbuffer *> *rings,QObject *parent)
  : AudioDevice(chans,samprate,rings,parent)
{
#ifdef ASIHPI
  asihpi_adapter_index=ASIHPI_DEFAULT_INDEX;
  asihpi_input_index=ASIHPI_DEFAULT_INPUT_INDEX;
  asihpi_pcm_buffer=NULL;

  asihpi_read_timer=new QTimer(this);
  connect(asihpi_read_timer,SIGNAL(timeout()),this,SLOT(readData()));

  asihpi_meter_timer=new QTimer(this);
  connect(asihpi_meter_timer,SIGNAL(timeout()),this,SLOT(meterData()));
#endif  // ASIHPI
}


AsiHpiDevice::~AsiHpiDevice()
{
#ifdef ASIHPI
  if(asihpi_pcm_buffer!=NULL) {
    delete asihpi_pcm_buffer;
  }
  delete asihpi_meter_timer;
  delete asihpi_read_timer;
#endif  // ASIHPI
}


bool AsiHpiDevice::isAvailable() const
{
  uint32_t index;
  uint16_t type;
  bool ret=(HPI_SubSysGetAdapter(NULL,0,&index,&type)==0);

  return ret;
}


bool AsiHpiDevice::processOptions(QString *err,const QStringList &keys,
				  const QStringList &values)
{
#ifdef ASIHPI
  for(int i=0;i<keys.size();i++) {
    bool processed=false;
    bool ok=false;
    if(keys[i]=="--asihpi-adapter-index") {
      asihpi_adapter_index=values[i].toUInt(&ok)-1;
      if((!ok)||(asihpi_adapter_index>=HPI_MAX_ADAPTERS)) {
	*err=tr("invalid")+" --asihpi-adapter-index "+tr("argument");
	return false;
      }
      processed=true;
    }
    if(keys[i]=="--asihpi-input-index") {
      asihpi_input_index=values[i].toUInt(&ok)-1;
      if((!ok)||(asihpi_input_index>HPI_MAX_STREAMS)) {
	*err=tr("invalid")+" --asihpi-input-index "+tr("argument");
	return false;
      }
      processed=true;
    }
    if(!processed) {
      *err=tr("unrecognized option")+" "+keys[i]+"\"";
      return false;
    }
  }
  return true;
#else
  return false;
#endif  // ASIHPI
}


bool AsiHpiDevice::start(QString *err)
{
#ifdef ASIHPI
  hpi_err_t herr;
  struct hpi_format fmt;
  uint16_t state=0;
  uint32_t buffer_size=0;
  uint32_t data_recorded=0;
  uint32_t samples_recorded=0;
  uint32_t aux_data_recorded=0;

  //
  // Open Mixer
  //
  if(HpiLog(HPI_MixerOpen(NULL,asihpi_adapter_index,&asihpi_mixer))==0) {
    if((HpiLog(HPI_MixerGetControl(NULL,asihpi_mixer,0,0,HPI_DESTNODE_ISTREAM,
      asihpi_input_index,HPI_CONTROL_METER,&asihpi_input_meter)))==0) {
      asihpi_meter_timer->start(100);
    }
  }

  //
  // Open Input Stream
  //
  if((herr=HPI_InStreamOpen(NULL,asihpi_adapter_index,asihpi_input_index,&asihpi_input_stream))!=0) {
    *err=tr("HPI error")+": "+hpi_strerror(herr);
    return false;
  }

  //
  // Find Supported Format
  //
  MakeFormat(&fmt,HPI_FORMAT_PCM32_FLOAT);
  if((herr=HPI_InStreamQueryFormat(NULL,asihpi_input_stream,&fmt))!=0) {
    *err=tr("HPI error")+": "+hpi_strerror(herr);
    return false;
  }

  //
  // Set Format
  //
  if((herr=HPI_InStreamSetFormat(NULL,asihpi_input_stream,&fmt))!=0) {
    *err=tr("HPI error")+": "+hpi_strerror(herr);
    return false;
  }

  //
  // Start input stream
  //
  if((herr=HPI_InStreamStart(NULL,asihpi_input_stream))!=0) {
    *err=tr("HPI error")+": "+hpi_strerror(herr);
    return false;
  }

  //
  // Create PCM buffer
  //
  if((herr=HPI_InStreamGetInfoEx(NULL,asihpi_input_stream,&state,&buffer_size,
				 &data_recorded,&samples_recorded,
				 &aux_data_recorded))!=0) {
    *err=tr("HPI error")+": "+hpi_strerror(herr);
    return false;
  }
  asihpi_pcm_buffer=new uint8_t[buffer_size];

  asihpi_read_timer->start(ASIHPI_READ_INTERVAL);  

  return true;
#else
  return false;
#endif  // ASIHPI
}


void AsiHpiDevice::readData()
{
#ifdef ASIHPI
  uint16_t state=0;
  uint32_t buffer_size=0;
  uint32_t data_recorded=0;
  uint32_t samples_recorded=0;
  uint32_t aux_data_recorded=0;

  HpiLog(HPI_InStreamGetInfoEx(NULL,asihpi_input_stream,&state,&buffer_size,
			       &data_recorded,&samples_recorded,
			       &aux_data_recorded));
  if(state==HPI_STATE_RECORDING) {
    if(HpiLog(HPI_InStreamReadBuf(NULL,asihpi_input_stream,asihpi_pcm_buffer,
				  data_recorded))==0) {
      for(unsigned i=0;i<ringBufferQuantity();i++) {
	ringBuffer(i)->write((float *)asihpi_pcm_buffer,
			     data_recorded/(sizeof(float)*channels()));
      }
    }
  }
  else {
    syslog(LOG_WARNING,"not in recording state");
  }


#endif  // ASIHPI
}


void AsiHpiDevice::meterData()
{
#ifdef ASIHPI
  short levels[HPI_MAX_CHANNELS];
  int lvls[MAX_AUDIO_CHANNELS];

  if(HpiLog(HPI_MeterGetRms(NULL,asihpi_input_meter,levels))==0) {
    for(unsigned i=0;i<MAX_AUDIO_CHANNELS;i++) {
      lvls[i]=-levels[i]/100;
    }
    updateMeterLevels(lvls);
  }
#endif  // ASIHPI
}


#ifdef ASIHPI
void AsiHpiDevice::MakeFormat(struct hpi_format *fmt,uint16_t hfmt)
{
  memset(fmt,0,sizeof(hpi_format));
  fmt->dwSampleRate=samplerate();
  fmt->wChannels=channels();
  fmt->wFormat=hfmt;
}
#endif  // ASIHPI


#ifdef ASIHPI
hpi_err_t AsiHpiDevice::HpiLog(hpi_err_t err,int priority) const
{
  if(err!=0) {
    syslog(priority,"HPI error %d: \"%s\"",err,hpi_strerror(err));
  }
  return err;
}
#endif  // ASIHPI


#ifdef ASIHPI
const char *AsiHpiDevice::hpi_strerror(hpi_err_t err) const
{
  static char err_text[200];

  HPI_GetErrorText(err,err_text);
  return err_text;
}
#endif  // ASIHPI
