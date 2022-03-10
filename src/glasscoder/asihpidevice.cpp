// asihpidevice.cpp
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

#include <string.h>

#include <stdio.h>

#include "asihpi.h"
#include "asihpidevice.h"
#include "logging.h"

AsiHpiDevice::AsiHpiDevice(unsigned chans,unsigned samprate,
			   Ringbuffer *ring,QObject *parent)
  : AudioDevice(chans,samprate,ring,parent)
{
#ifdef ASIHPI
  struct hpi_format fmt;

  asihpi_adapter_index=ASIHPI_DEFAULT_INDEX;
  asihpi_input_index=ASIHPI_DEFAULT_INPUT_INDEX;
  asihpi_input_gain=0;
  asihpi_channel_mode=HPI_CHANNEL_MODE_NORMAL;
  asihpi_input_source=HPI_SOURCENODE_LINEIN;
  asihpi_input_type=HPI_SOURCENODE_LINEIN;
  asihpi_pcm_buffer=NULL;
  asihpi_dma_buffer_size=0;

  //
  // Calculate DMA Buffer Size
  //
  memset(&fmt,0,sizeof(fmt));  // Worst case situation
  fmt.dwSampleRate=48000;
  fmt.wChannels=2;
  fmt.wFormat=HPI_FORMAT_PCM32_FLOAT;
  if(HPI_StreamEstimateBufferSize(&fmt,ASIHPI_READ_INTERVAL,&asihpi_dma_buffer_size)!=0) {
    asihpi_dma_buffer_size=0;
  }

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
#ifdef ASIHPI
  uint32_t index;
  uint16_t type;
  bool ret=(HPI_SubSysGetAdapter(NULL,0,&index,&type)==0);

  return ret;
#else
  return false;
#endif  // ASIHPI
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
    if(keys[i]=="--asihpi-input-gain") {
      asihpi_input_gain=values[i].toInt(&ok);
      if((!ok)||((asihpi_input_gain<-100)&&(asihpi_input_gain>20))) {
	*err=tr("invalid")+" --asihpi-input-gain "+tr("argument");
	return false;
      }
      processed=true;
    }
    if(keys[i]=="--asihpi-channel-mode") {
      if(values[i].toLower()=="normal") {
	asihpi_channel_mode=HPI_CHANNEL_MODE_NORMAL;
	processed=true;
      }
      if(values[i].toLower()=="swap") {
	asihpi_channel_mode=HPI_CHANNEL_MODE_SWAP;
	processed=true;
      }
      if(values[i].toLower()=="left") {
	asihpi_channel_mode=HPI_CHANNEL_MODE_LEFT_TO_STEREO;
	processed=true;
      }
      if(values[i].toLower()=="right") {
	asihpi_channel_mode=HPI_CHANNEL_MODE_RIGHT_TO_STEREO;
	processed=true;
      }
      if(!processed) {
	*err=tr("invalid value for")+" --asihpi-channel-mode";
	return false;
      }
    }
    if(keys[i]=="--asihpi-input-source") {
      if((asihpi_input_source=AsihpiSourceNode(values[i]))==0) {
	*err=tr("invalid value for --asihpi-input-source");
	return false;
      }
      processed=true;
    }
    if(keys[i]=="--asihpi-input-type") {
      if((asihpi_input_type=AsihpiSourceNode(values[i]))==0) {
	*err=tr("invalid value for --asihpi-input-type");
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

  hpi_handle_t handle;
  short lvls[HPI_MAX_CHANNELS];

  //
  // Open Mixer
  //
  if(HpiLog(HPI_MixerOpen(NULL,asihpi_adapter_index,&asihpi_mixer))==0) {

    //
    // Input Gain
    //
    if(HPI_MixerGetControl(NULL,asihpi_mixer,
			     HPI_SOURCENODE_LINEIN,asihpi_input_index,
			     HPI_DESTNODE_NONE,0,HPI_CONTROL_VOLUME,
			     &handle)==0) {
      for(unsigned i=0;i<HPI_MAX_CHANNELS;i++) {
	lvls[i]=asihpi_input_gain*100;
      }
      HpiLog(HPI_VolumeSetGain(NULL,handle,lvls));
    }

    //
    // Input Source
    //
    if(HpiLog(HPI_MixerGetControl(NULL,asihpi_mixer,0,0,
				  HPI_DESTNODE_ISTREAM,asihpi_input_index,
				  HPI_CONTROL_MULTIPLEXER,&handle))==0) {
      HpiLog(HPI_Multiplexer_SetSource(NULL,handle,asihpi_input_source,
				       asihpi_input_index));
    }

    //
    // Input Type
    //
    if((HpiLog(HPI_MixerGetControl(NULL,asihpi_mixer,
				   HPI_SOURCENODE_LINEIN,asihpi_input_index,
				   HPI_DESTNODE_NONE,0,
				   HPI_CONTROL_MULTIPLEXER,&handle)))==0) {
      HpiLog(HPI_Multiplexer_SetSource(NULL,handle,asihpi_input_type,
				       asihpi_input_index));
    }

    //
    // Channel Mode
    //
    if((HpiLog(HPI_MixerGetControl(NULL,asihpi_mixer,0,0,
				     HPI_DESTNODE_ISTREAM,asihpi_input_index,
				     HPI_CONTROL_CHANNEL_MODE,
				     &handle)))==0) {
      HpiLog(HPI_ChannelModeSet(NULL,handle,asihpi_channel_mode));
    }

    //
    // Input Meter
    //
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
  if(asihpi_dma_buffer_size>0) {
    if(HpiLog(HPI_InStreamHostBufferAllocate(NULL,asihpi_input_stream,asihpi_dma_buffer_size))!=0) {
      return false;
    }
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
      ringBuffer()->write((float *)asihpi_pcm_buffer,
			  data_recorded/(sizeof(float)*channels()));
    }
  }
  else {
    Log(LOG_WARNING,"not in recording state"+
	QString().sprintf(" [state: %u]",state));
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
      lvls[i]=-levels[i];
    }
    setMeterLevels(lvls);
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
    Log(priority,
	QString().sprintf("HPI error %d: \"%s\"",err,hpi_strerror(err)));
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
