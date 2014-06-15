// mpegl2codec.cpp
//
// Codec class for MPEG-1 Layer 2
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

#include "mpegl2codec.h"

MpegL2Codec::MpegL2Codec(Ringbuffer *ring,QObject *parent)
  : Codec(Codec::TypeMpegL3,ring,parent)
{
#ifdef HAVE_TWOLAME
  /*
  l3_lameopts=NULL;
  l3_lame_handle=NULL;
  */
#endif  // HAVE_TWOLAME
}


QString MpegL2Codec::contentType() const
{
  return "audio/mpeg";
}


unsigned MpegL2Codec::pcmFrames() const
{
  return 1152;
}


bool MpegL2Codec::startCodec()
{
#ifdef HAVE_TWOLAME

  //
  // Load Library
  //
  if((twolame_handle=dlopen("libtwolame.so",RTLD_LAZY))==NULL) {
    syslog(LOG_ERR,"unsupported audio format (library not found)");
    return false;
  }
  *(void **)(&twolame_init)=dlsym(twolame_handle,"twolame_init");
  *(void **)(&twolame_set_mode)=dlsym(twolame_handle,"twolame_set_mode");
  *(void **)(&twolame_set_num_channels)=
    dlsym(twolame_handle,"twolame_set_num_channels");
  *(void **)(&twolame_set_in_samplerate)=
    dlsym(twolame_handle,"twolame_set_in_samplerate");
  *(void **)(&twolame_set_out_samplerate)=
    dlsym(twolame_handle,"twolame_set_out_samplerate");
  *(void **)(&twolame_set_bitrate)=
    dlsym(twolame_handle,"twolame_set_bitrate");
  *(void **)(&twolame_init_params)=
    dlsym(twolame_handle,"twolame_init_params");
  *(void **)(&twolame_close)=dlsym(twolame_handle,"twolame_close");
  *(void **)(&twolame_encode_buffer_interleaved)=
    dlsym(twolame_handle,"twolame_encode_buffer_interleaved");
  *(void **)(&twolame_encode_buffer_float32_interleaved)=
    dlsym(twolame_handle,"twolame_encode_buffer_float32_interleaved");
  *(void **)(&twolame_encode_flush)=
    dlsym(twolame_handle,"twolame_encode_flush");
  *(void **)(&twolame_set_energy_levels)=
    dlsym(twolame_handle,"twolame_set_energy_levels");

  //
  // Initialize Encoder Instance
  //
  TWOLAME_MPEG_mode mpeg_mode=TWOLAME_STEREO;

  switch(channels()) {
  case 1:
    mpeg_mode=TWOLAME_MONO;
    break;

  case 2:
    mpeg_mode=TWOLAME_STEREO;    
    break;
  }
  if((twolame_lameopts=twolame_init())==NULL) {
    syslog(LOG_ERR,"unable to initialize MP2 encoder");
    return false;
  }
  twolame_set_mode(twolame_lameopts,mpeg_mode);
  twolame_set_num_channels(twolame_lameopts,channels());
  twolame_set_in_samplerate(twolame_lameopts,streamSamplerate());
  twolame_set_out_samplerate(twolame_lameopts,streamSamplerate());
  twolame_set_bitrate(twolame_lameopts,bitrate());
  twolame_set_energy_levels(twolame_lameopts,1);
  if(twolame_init_params(twolame_lameopts)!=0) {
    syslog(LOG_ERR,"unable to start MP2 encoder");
    return false;
  }
  return true;
#else
  syslog(LOG_ERR,"unsupported audio format (no build support)");
  return false;
#endif  // HAVE_TWOLAME
}


void MpegL2Codec::encodeData(Connector *conn,const float *pcm,int frames)
{
#ifdef HAVE_TWOLAME
  int s;
  unsigned char mpeg[8640];

  if((s=twolame_encode_buffer_float32_interleaved(twolame_lameopts,pcm,frames,
						  mpeg,8640))>=0) {
    conn->writeData((const char *)mpeg,s);
  }



  /*
  int s;
  unsigned char mpeg[8640];

  if(channels()==2) {
    if((s=lame_encode_buffer_interleaved_ieee_float(l3_lameopts,pcm,
						    frames,mpeg,8640))>=0) {
      conn->writeData((const char *)mpeg,s);
    }
  }
  else {
    if((s=lame_encode_buffer_ieee_float(l3_lameopts,pcm,NULL,frames,
					mpeg,8640))>=0) {
      conn->writeData((const char *)mpeg,s);
    }
  }
  */
#endif  // HAVE_TWOLAME
}
