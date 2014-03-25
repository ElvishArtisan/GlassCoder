// layer3.cpp
//
// MPEG Layer3 Processing Routines for glasscoder(1).
//
//   (C) Copyright 2014 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: layer3.cpp,v 1.2 2014/02/18 21:40:57 cvs Exp $
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

#include <dlfcn.h>
#include <syslog.h>

#include "glasscoder.h"

void MainObject::layer3EncodeData()
{
  unsigned char mpeg[8640];
  int n;
  int s;
  int err=0;

  while(sir_ringbuffer->readSpace()>=1152) {
    n=sir_ringbuffer->read(sir_pcm_in,1152);
    if(sir_src_state!=NULL) {
      sir_src_data->input_frames=n;
      if((err=src_process(sir_src_state,sir_src_data))!=0) {
	syslog(LOG_WARNING,"SRC error [%s]",src_strerror(err));
	continue;
      }
      n=sir_src_data->output_frames_gen;
    }
    if(audio_channels==2) {
      if((s=lame_encode_buffer_interleaved_ieee_float(sir_lameopts,sir_pcm_out,
						      n,mpeg,8640))>=0) {
	shout_send(sir_shout,mpeg,s);
      }
    }
    else {
      if((s=lame_encode_buffer_ieee_float(sir_lameopts,sir_pcm_out,NULL,n,
					  mpeg,8640))>=0) {
	shout_send(sir_shout,mpeg,s);
      }
    }
  }
  sir_encoder_timer->start(RINGBUFFER_SERVICE_INTERVAL);
}


bool MainObject::StartLame()
{
#ifdef HAVE_LAME
  MPEG_mode mpeg_mode=STEREO;

  //
  // Load Library
  //
  sir_lame_handle=dlopen("libmp3lame.so",RTLD_LAZY);

  if(sir_lame_handle==NULL) {
    syslog(LOG_ERR,"unsupported audio format (library not found)");
    return false;
  }
  *(void **)(&lame_init)=dlsym(sir_lame_handle,"lame_init");
  *(void **)(&lame_set_mode)=
    dlsym(sir_lame_handle,"lame_set_mode");
  *(void **)(&lame_set_num_channels)=
    dlsym(sir_lame_handle,"lame_set_num_channels");
  *(void **)(&lame_set_in_samplerate)=
    dlsym(sir_lame_handle,"lame_set_in_samplerate");
  *(void **)(&lame_set_out_samplerate)=
    dlsym(sir_lame_handle,"lame_set_out_samplerate");
  *(void **)(&lame_set_brate)=dlsym(sir_lame_handle,"lame_set_brate");
  *(void **)(&lame_init_params)=dlsym(sir_lame_handle,"lame_init_params");
  *(void **)(&lame_close)=dlsym(sir_lame_handle,"lame_close");
  *(void **)(&lame_encode_buffer_interleaved)=
    dlsym(sir_lame_handle,"lame_encode_buffer_interleaved");
  *(void **)(&lame_encode_buffer)=
    dlsym(sir_lame_handle,"lame_encode_buffer");
  *(void **)(&lame_encode_flush)=dlsym(sir_lame_handle,"lame_encode_flush");
  *(void **)(&lame_set_bWriteVbrTag)=
    dlsym(sir_lame_handle,"lame_set_bWriteVbrTag");
  *(void **)(&lame_encode_buffer_float)=
    dlsym(sir_lame_handle,"lame_encoder_buffer_float");
  *(void **)(&lame_encode_buffer_ieee_float)=
    dlsym(sir_lame_handle,"lame_encode_buffer_ieee_float");
  *(void **)(&lame_encode_buffer_interleaved_ieee_float)=
    dlsym(sir_lame_handle,"lame_encode_buffer_interleaved_ieee_float");
  *(void **)(&lame_encode_buffer_ieee_double)=
    dlsym(sir_lame_handle,"lame_encode_buffer_ieee_double");
  *(void **)(&lame_encode_buffer_long)=
    dlsym(sir_lame_handle,"lame_encode_buffer_long");
  *(void **)(&lame_encode_buffer_long2)=
    dlsym(sir_lame_handle,"lame_encode_buffer_long2");
  *(void **)(&lame_encode_buffer_int)=
    dlsym(sir_lame_handle,"lame_encode_buffer_int");
  *(void **)(&lame_encode_flush_nogap)=
    dlsym(sir_lame_handle,"lame_encode_flush_nogap");
  *(void **)(&lame_init_bitstream)=
    dlsym(sir_lame_handle,"lame_init_bitstream");

  //
  // Initialize Encoder Instance
  //
  switch(audio_channels) {
  case 1:
    mpeg_mode=MONO;
    break;

  case 2:
    mpeg_mode=STEREO;    
    break;

  default:
    syslog(LOG_ERR,"invalid audio channels");
    return false;
  }
  if((sir_lameopts=lame_init())==NULL) {
    syslog(LOG_ERR,"unable to initialize MP3 encoder");
    return false;
  }
  lame_set_mode(sir_lameopts,mpeg_mode);
  lame_set_num_channels(sir_lameopts,audio_channels);
  lame_set_in_samplerate(sir_lameopts,audio_samplerate);
  lame_set_out_samplerate(sir_lameopts,audio_samplerate);
  lame_set_brate(sir_lameopts,audio_bitrate);
  lame_set_bWriteVbrTag(sir_lameopts,0);
  if(lame_init_params(sir_lameopts)!=0) {
    lame_close(sir_lameopts);
    syslog(LOG_ERR,"unable to start MP3 encoder");
    return false;
  }
  return true;
#else
  syslog(LOG_ERR,"unsupported audio format (no build support)");
  return false;
#endif  // HAVE_LAME
}
