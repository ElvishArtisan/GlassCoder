// heaaccodec.cpp
//
// Codec class for MPEG-4 Advanced Audio Coding High Efficiency Profile
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

#include <samplerate.h>

#include "heaaccodec.h"

HeAacCodec::HeAacCodec(Ringbuffer *ring,QObject *parent)
  : Codec(Codec::TypeHeAac,ring,parent)
{
  heaac_buffer=NULL;
}


HeAacCodec::~HeAacCodec()
{
  if(heaac_buffer!=NULL) {
    delete heaac_buffer;
  }
}


bool HeAacCodec::isAvailable() const
{
#ifdef HAVE_AACPLUS
  return dlopen("libaacplus.so",RTLD_LAZY)!=NULL;
#else
  return false;
#endif  // HAVE_AACPLUS
}


QString HeAacCodec::contentType() const
{
  return "audio/aacp";
}


unsigned HeAacCodec::pcmFrames() const
{
  return heaac_input_samples/channels();
}


QString HeAacCodec::defaultExtension() const
{
  return QString("aac");
}


QString HeAacCodec::formatIdentifier() const
{
  //
  // See RFC 6381
  //
  return QString("mp4a");
}


bool HeAacCodec::startCodec()
{
#ifdef HAVE_AACPLUS
  aacplusEncConfiguration *config;

  //
  // Sanity Checks
  //
  if(bitrate()==0) {
    syslog(LOG_ERR,"VBR encoding not supported");
    return false;
  }
  if((streamSamplerate()!=32000)&&(streamSamplerate()!=44100)&
     (streamSamplerate()!=48000)) {
    syslog(LOG_ERR,"unsupported stream sample rate");
    return false;
  }
  if(channels()!=2) {
    syslog(LOG_ERR,"unsupported channel count");
    return false;
  }
  if(((streamSamplerate()==32000)&&(bitrate()!=32))|| 
     ((bitrate()!=32)&&(bitrate()!=48)&&(bitrate()!=56)&&
      (bitrate()!=64)&&(bitrate()!=96)&&(bitrate()!=128))) {
    syslog(LOG_ERR,"unsupported stream bit rate");
    return false;
  }

  //
  // Load Library
  //
  heaac_handle=dlopen("libaacplus.so",RTLD_LAZY);

  if(heaac_handle==NULL) {
    syslog(LOG_ERR,"unsupported audio format (library not found)");
    return false;
  }
  *(void **)(&aacplusEncGetCurrentConfiguration)=
    dlsym(heaac_handle,"aacplusEncGetCurrentConfiguration");
  *(void **)(&aacplusEncSetConfiguration)=
    dlsym(heaac_handle,"aacplusEncSetConfiguration");
  *(void **)(&aacplusEncOpen)=dlsym(heaac_handle,"aacplusEncOpen");
  *(void **)(&aacplusEncGetDecoderSpecificInfo)=
    dlsym(heaac_handle,"aacplusEncGetDecoderSpecificInfo");
  *(void **)(&aacplusEncEncode)=dlsym(heaac_handle,"aacplusEncEncode");
  *(void **)(&aacplusEncClose)=dlsym(heaac_handle,"aacplusEncClose");

  //
  // Initialize Encoder Instance
  //
  heaac_encoder=aacplusEncOpen(streamSamplerate(),channels(),
			       &heaac_input_samples,&heaac_buffer_size);
  heaac_buffer=new unsigned char[heaac_buffer_size];
  if((config=aacplusEncGetCurrentConfiguration(heaac_encoder))==NULL) {
    aacplusEncClose(heaac_encoder);
    return false;
  }
  config->nChannelsIn=channels();
  config->nChannelsOut=channels();
  config->bandWidth=0;  // Use AACPlus Defaults
  config->bitRate=1000*bitrate()/channels();
  config->outputFormat=1;  // ADTS
  config->inputFormat=AACPLUS_INPUT_16BIT;
  if(aacplusEncSetConfiguration(heaac_encoder,config)<0) {
    aacplusEncClose(heaac_encoder);
    return false;
  }

  return true;
#else
  syslog(LOG_ERR,"unsupported audio format (no build support)");
  return false;
#endif  // HAVE_AACPLUS
}


void HeAacCodec::encodeData(Connector *conn,const float *pcm,int frames)
{
#ifdef HAVE_AACPLUS
  int s;
  int16_t pcms[frames*channels()];

  src_float_to_short_array(pcm,pcms,frames*channels());
  if((s=aacplusEncEncode(heaac_encoder,(int32_t *)pcms,frames*channels(),heaac_buffer,heaac_buffer_size))>0) {
    conn->writeData(frames,heaac_buffer,s);
  }
  else {
    syslog(LOG_WARNING,"aacplus encoding error %d",s);
  }
#endif  // HAVE_AACPLUS
}
