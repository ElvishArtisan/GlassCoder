// mpegl3codec.h
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

#ifndef MPEGL3CODEC_H
#define MPEGL3CODEC_H

#ifdef HAVE_LAME
#include <lame/lame.h>
#endif  // HAVE_LAME

#include "codec.h"

class MpegL3Codec : public Codec
{
  Q_OBJECT;
 public:
  MpegL3Codec(Ringbuffer *ring,QObject *parent=0);
  bool isAvailable() const;
  QString contentType() const;
  unsigned pcmFrames() const;
  QString defaultExtension() const;
  QString formatIdentifier() const;
  bool startCodec();

 protected:
  void encodeData(Connector *conn,const float *pcm,int frames);

 private:
#ifdef HAVE_LAME
  lame_global_flags *l3_lameopts;
  void *l3_lame_handle;
  lame_global_flags *(*lame_init)(void);
  void (*lame_set_mode)(lame_global_flags *,int);
  void (*lame_set_num_channels)(lame_global_flags *,int);
  void (*lame_set_in_samplerate)(lame_global_flags *,int);
  void (*lame_set_out_samplerate)(lame_global_flags *,int);
  void (*lame_set_brate)(lame_global_flags *,int);
  int (*lame_init_params)(lame_global_flags *);
  void (*lame_close)(lame_global_flags *);
  int (*lame_encode_buffer_interleaved)
    (lame_global_flags *,short int[],int,unsigned char *,int);
  int (*lame_encode_buffer)
    (lame_global_flags *,short int[],short int[],int,unsigned char *,int);
  int (*lame_encode_buffer_float)
    (lame_global_flags *,const float,const float,const int,unsigned char *,
     const int);
  int (*lame_encode_buffer_ieee_float)
    (lame_t,const float[],const float[],const int,unsigned char *,const int);
  int (*lame_encode_buffer_interleaved_ieee_float)
    (lame_t,const float[],const int,unsigned char *,const int);
  int (*lame_encode_buffer_ieee_double)
    (lame_t,const double,const double,const int,unsigned char *,const int);
  int (*lame_encode_buffer_long)
    (lame_global_flags *,const long,const long,const int,unsigned char *,
     const int);
  int (*lame_encode_buffer_long2)
    (lame_global_flags *,const long,const long,const int,unsigned char *,
     const int);
  int (*lame_encode_buffer_int)
    (lame_global_flags *,const int,const int,const int,unsigned char *,
     const int);
  int(*lame_encode_flush_nogap)(lame_global_flags *,unsigned char *,int);
  int (*lame_init_bitstream)(lame_global_flags *);
  int (*lame_encode_flush)(lame_global_flags *,unsigned char *,int);
  int (*lame_set_bWriteVbrTag)(lame_global_flags *, int);
  int (*lame_set_VBR)(lame_global_flags *, vbr_mode);
  int (*lame_set_VBR_quality)(lame_global_flags *, float);
  int (*lame_set_disable_reservoir)(lame_global_flags *,int);
  int (*lame_get_disable_reservoir)(const lame_global_flags *);
#endif  // HAVE_LAME
};


#endif  // MPEGL3CODEC_H
