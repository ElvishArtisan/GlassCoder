// aaccodec.cpp
//
// Codec class for Advanced Audio Coding (AAC)
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

#include "aaccodec.h"
#include "logging.h"

AacCodec::AacCodec(Ringbuffer *ring,QObject *parent)
  : Codec(Codec::TypeAac,ring,parent)
{
  aac_buffer=NULL;
}


AacCodec::~AacCodec()
{
  if(aac_buffer!=NULL) {
    delete aac_buffer;
  }
}


bool AacCodec::isAvailable() const
{
#ifdef HAVE_FAAC
  return dlopen("libfaac.so",RTLD_LAZY)!=NULL;
#else
  return false;
#endif  // HAVE_FAAC
}


QString AacCodec::contentType() const
{
  return "audio/aac";
}


unsigned AacCodec::pcmFrames() const
{
  return aac_input_samples/channels();
}


QString AacCodec::defaultExtension() const
{
  return QString("aac");
}


QString AacCodec::formatIdentifier() const
{
  //
  // From ISO/IEC 14496-3
  // (see the summary at https://en.wikipedia.org/wiki/MPEG-4_Part_3)
  //
  return QString("mp4a.40.1");
}


bool AacCodec::startCodec()
{
#ifdef HAVE_FAAC
  faacEncConfiguration *config;

  //
  // Load Library
  //
  aac_handle=dlopen("libfaac.so",RTLD_LAZY);

  if(aac_handle==NULL) {
    Log(LOG_ERR,"unsupported audio format (library not found)");
    return false;
  }
  *(void **)(&faacEncOpen)=dlsym(aac_handle,"faacEncOpen");
  *(void **)(&faacEncClose)=dlsym(aac_handle,"faacEncClose");
  *(void **)(&faacEncGetCurrentConfiguration)=
    dlsym(aac_handle,"faacEncGetCurrentConfiguration");
  *(void **)(&faacEncSetConfiguration)=
    dlsym(aac_handle,"faacEncSetConfiguration");
  *(void **)(&faacEncEncode)=dlsym(aac_handle,"faacEncEncode");

  //
  // Initialize Encoder Instance
  //
  aac_encoder=faacEncOpen(streamSamplerate(),channels(),&aac_input_samples,
			  &aac_buffer_size);
  aac_buffer=new unsigned char[aac_buffer_size];
  if((config=faacEncGetCurrentConfiguration(aac_encoder))==NULL) {
    faacEncClose(aac_encoder);
    return false;
  }
  config->aacObjectType=MAIN;
  config->mpegVersion=MPEG2;
  config->useTns=1;
  config->shortctl=SHORTCTL_NORMAL;
  config->useLfe=0;
  config->allowMidside=1;
  config->bandWidth=0;  // Use FAAC Defaults
  if(bitrate()==0) {
    config->bitRate=0;
    config->quantqual=(int)(400.0*quality()+100.0);
  }
  else {
    config->bitRate=1000*bitrate()/channels();
    config->quantqual=0;
  }
  config->outputFormat=1;  // ADTS
  config->inputFormat=FAAC_INPUT_16BIT;
  if(faacEncSetConfiguration(aac_encoder,config)<0) {
    faacEncClose(aac_encoder);
    return false;
  }

  return true;
#else
  Log(LOG_ERR,"unsupported audio format (no build support)");
  return false;
#endif  // HAVE_FAAC
}


void AacCodec::encodeData(Connector *conn,const float *pcm,int frames)
{
#ifdef HAVE_FAAC
  int s;
  int16_t pcms[frames*channels()];

  src_float_to_short_array(pcm,pcms,frames*channels());
  if((s=faacEncEncode(aac_encoder,(int32_t *)pcms,frames*channels(),aac_buffer,aac_buffer_size))>0) {
    conn->writeData(frames,aac_buffer,s);
  }
  else {
    Log(LOG_WARNING,QString().sprintf("aac encoding error %d",s));
  }
#endif  // HAVE_FAAC
}
