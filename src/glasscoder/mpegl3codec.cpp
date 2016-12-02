// mpegl3codec.cpp
//
// Codec class for MPEG-1 Layer 3
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

#include "logging.h"
#include "mpegl3codec.h"

MpegL3Codec::MpegL3Codec(Ringbuffer *ring,QObject *parent)
  : Codec(Codec::TypeMpegL3,ring,parent)
{
#ifdef HAVE_LAME
  l3_lameopts=NULL;
  l3_lame_handle=NULL;
#endif  // HAVE_LAME
}


bool MpegL3Codec::isAvailable() const
{
#ifdef HAVE_LAME
  return dlopen("libmp3lame.so.0",RTLD_LAZY)!=NULL;
#else
  return false;
#endif  // HAVE_LAME
}


QString MpegL3Codec::contentType() const
{
  return "audio/mpeg";
}


unsigned MpegL3Codec::pcmFrames() const
{
  return 1152;
}


QString MpegL3Codec::defaultExtension() const
{
  return QString("mp3");
}


QString MpegL3Codec::formatIdentifier() const
{
  //
  // From ISO/IEC 14496-3
  // (see the summary at https://en.wikipedia.org/wiki/MPEG-4_Part_3)
  //
  return QString("mp4a.40.34");
}


bool MpegL3Codec::startCodec()
{
#ifdef HAVE_LAME
  MPEG_mode mpeg_mode=STEREO;

  //
  // Load Library
  //
  l3_lame_handle=dlopen("libmp3lame.so.0",RTLD_LAZY);

  if(l3_lame_handle==NULL) {
    Log(LOG_ERR,"unsupported audio format (library not found)");
    return false;
  }
  *(void **)(&lame_init)=dlsym(l3_lame_handle,"lame_init");
  *(void **)(&lame_set_mode)=
    dlsym(l3_lame_handle,"lame_set_mode");
  *(void **)(&lame_set_num_channels)=
    dlsym(l3_lame_handle,"lame_set_num_channels");
  *(void **)(&lame_set_in_samplerate)=
    dlsym(l3_lame_handle,"lame_set_in_samplerate");
  *(void **)(&lame_set_out_samplerate)=
    dlsym(l3_lame_handle,"lame_set_out_samplerate");
  *(void **)(&lame_set_brate)=dlsym(l3_lame_handle,"lame_set_brate");
  *(void **)(&lame_init_params)=dlsym(l3_lame_handle,"lame_init_params");
  *(void **)(&lame_close)=dlsym(l3_lame_handle,"lame_close");
  *(void **)(&lame_encode_buffer_interleaved)=
    dlsym(l3_lame_handle,"lame_encode_buffer_interleaved");
  *(void **)(&lame_encode_buffer)=
    dlsym(l3_lame_handle,"lame_encode_buffer");
  *(void **)(&lame_encode_flush)=dlsym(l3_lame_handle,"lame_encode_flush");
  *(void **)(&lame_set_bWriteVbrTag)=
    dlsym(l3_lame_handle,"lame_set_bWriteVbrTag");
  *(void **)(&lame_encode_buffer_float)=
    dlsym(l3_lame_handle,"lame_encoder_buffer_float");
  *(void **)(&lame_encode_buffer_ieee_float)=
    dlsym(l3_lame_handle,"lame_encode_buffer_ieee_float");
  *(void **)(&lame_encode_buffer_interleaved_ieee_float)=
    dlsym(l3_lame_handle,"lame_encode_buffer_interleaved_ieee_float");
  *(void **)(&lame_encode_buffer_ieee_double)=
    dlsym(l3_lame_handle,"lame_encode_buffer_ieee_double");
  *(void **)(&lame_encode_buffer_long)=
    dlsym(l3_lame_handle,"lame_encode_buffer_long");
  *(void **)(&lame_encode_buffer_long2)=
    dlsym(l3_lame_handle,"lame_encode_buffer_long2");
  *(void **)(&lame_encode_buffer_int)=
    dlsym(l3_lame_handle,"lame_encode_buffer_int");
  *(void **)(&lame_encode_flush_nogap)=
    dlsym(l3_lame_handle,"lame_encode_flush_nogap");
  *(void **)(&lame_init_bitstream)=
    dlsym(l3_lame_handle,"lame_init_bitstream");
  *(void **)(&lame_set_VBR)=dlsym(l3_lame_handle,"lame_set_VBR");
  *(void **)(&lame_set_VBR_quality)=
    dlsym(l3_lame_handle,"lame_set_VBR_quality");
  *(void **)(&lame_set_disable_reservoir)=
    dlsym(l3_lame_handle,"lame_set_disable_reservoir");
  *(void **)(&lame_get_disable_reservoir)=
    dlsym(l3_lame_handle,"lame_get_disable_reservoir");
  if(lame_encode_buffer_ieee_float==NULL) {  // Earlier versions of LAME didn't include this!
    return false;
  }

  //
  // Initialize Encoder Instance
  //
  switch(channels()) {
  case 1:
    mpeg_mode=MONO;
    break;

  case 2:
    mpeg_mode=STEREO;    
    break;

  default:
    Log(LOG_ERR,"invalid audio channels");
    return false;
  }
  if((l3_lameopts=lame_init())==NULL) {
    Log(LOG_ERR,"unable to initialize MP3 encoder");
    return false;
  }
  if(completeFrames()) {
    lame_set_disable_reservoir(l3_lameopts,1);
  }
  lame_set_mode(l3_lameopts,mpeg_mode);
  lame_set_num_channels(l3_lameopts,channels());
  lame_set_in_samplerate(l3_lameopts,streamSamplerate());
  lame_set_out_samplerate(l3_lameopts,streamSamplerate());
  if(bitrate()==0) {
    lame_set_VBR(l3_lameopts,vbr_default);
    lame_set_VBR_quality(l3_lameopts,(int)(9.0*(1.0-quality())));
  }
  else {
    lame_set_brate(l3_lameopts,bitrate());
  }
  lame_set_bWriteVbrTag(l3_lameopts,0);
  if(lame_init_params(l3_lameopts)!=0) {
    lame_close(l3_lameopts);
    Log(LOG_ERR,"unable to start MP3 encoder");
    return false;
  }
  return true;
#else
  Log(LOG_ERR,"unsupported audio format (no build support)");
  return false;
#endif  // HAVE_LAME
}


void MpegL3Codec::encodeData(Connector *conn,const float *pcm,int frames)
{
#ifdef HAVE_LAME
  int s;
  unsigned char mpeg[8640];

  if(channels()==2) {
    if((s=lame_encode_buffer_interleaved_ieee_float(l3_lameopts,pcm,
						    frames,mpeg,8640))>=0) {
      conn->writeData(frames,mpeg,s);
    }
  }
  else {
    if((s=lame_encode_buffer_ieee_float(l3_lameopts,pcm,NULL,frames,
					mpeg,8640))>=0) {
      conn->writeData(frames,mpeg,s);
    }
  }
#endif  // HAVE_LAME
}
