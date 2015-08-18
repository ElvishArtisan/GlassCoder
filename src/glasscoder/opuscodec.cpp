// opuscodec.cpp
//
// Codec class for Advanced Audio Coding (AAC)
//
//   (C) Copyright 2014 Fred Gleason <fredg@paravelsystems.com>
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

#include "opuscodec.h"

OpusCodec::OpusCodec(Ringbuffer *ring,QObject *parent)
  : Codec(Codec::TypeAac,ring,parent)
{
}


bool OpusCodec::isAvailable() const
{
#ifdef HAVE_OPUS
  return dlopen("libopus.so",RTLD_LAZY)!=NULL;
#else
  return false;
#endif  // HAVE_OPUS
}


QString OpusCodec::contentType() const
{
  return "audio/ogg";
}


unsigned OpusCodec::pcmFrames() const
{
  return 960;
}


QString OpusCodec::defaultExtension() const
{
  return QString("ogg");
}


QString OpusCodec::formatIdentifier() const
{
  return QString();
}


bool OpusCodec::startCodec()
{
#ifdef HAVE_OPUS
  int err;

  //
  // Load Library
  //
  opus_handle=dlopen("libopus.so",RTLD_LAZY);

  if(opus_handle==NULL) {
    syslog(LOG_ERR,"unsupported audio format (library not found)");
    return false;
  }
  *(void **)(&opus_encoder_create)=dlsym(opus_handle,"opus_encoder_create");
  *(void **)(&opus_encoder_init)=dlsym(opus_handle,"opus_encoder_init");
  *(void **)(&opus_encode)=dlsym(opus_handle,"opus_encode");
  *(void **)(&opus_encode_float)=dlsym(opus_handle,"opus_encode_float");
  *(void **)(&opus_encoder_destroy)=dlsym(opus_handle,"opus_encoder_destroy");
  *(void **)(&opus_encoder_ctl)=dlsym(opus_handle,"opus_encoder_ctl");
  *(void **)(&opus_strerror)=dlsym(opus_handle,"opus_strerror");

  //
  // Initialize Encoder Instance
  //
  if((opus_encoder=opus_encoder_create(streamSamplerate(),channels(),
				       OPUS_APPLICATION_AUDIO,&err))==NULL) {
    syslog(LOG_ERR,"unable to create codec [%s]",opus_strerror(err));
    return false;
  }
  if(bitrate()==0) {
    opus_encoder_ctl(opus_encoder,OPUS_SET_VBR(1));
    opus_encoder_ctl(opus_encoder,OPUS_SET_COMPLEXITY(10.0*quality()));
  }
  else {
    opus_encoder_ctl(opus_encoder,OPUS_SET_VBR(0));
    opus_encoder_ctl(opus_encoder,OPUS_SET_BITRATE(1000*bitrate()));
  }
  return true;
#else
  syslog(LOG_ERR,"unsupported audio format (no build support)");
  return false;
#endif  // HAVE_OPUS
}


void OpusCodec::encodeData(Connector *conn,const float *pcm,int frames)
{
#ifdef HAVE_OPUS
  int s;
  unsigned char data[8192];

  if((s=opus_encode_float(opus_encoder,pcm,frames,data,8192))>1) {
    conn->writeData(frames,data,s);
  }
  else {
    syslog(LOG_WARNING,"opus encoding error %d",s);
  }
#endif  // HAVE_OPUS
}
