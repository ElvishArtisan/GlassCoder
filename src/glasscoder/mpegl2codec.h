// mpegl2codec.h
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

#ifndef MPEGL2CODEC_H
#define MPEGL2CODEC_H

#ifdef HAVE_TWOLAME
#include <twolame.h>
#endif  // HAVE_TWOLAME

#include "codec.h"

class MpegL2Codec : public Codec
{
  Q_OBJECT;
 public:
  MpegL2Codec(Ringbuffer *ring,QObject *parent=0);
  bool isAvailable() const;
  QString contentType() const;
  unsigned pcmFrames() const;
  QString defaultExtension() const;
  QString formatIdentifier() const;
  bool startCodec();

 protected:
  void encodeData(Connector *conn,const float *pcm,int frames);

 private:
#ifdef HAVE_TWOLAME
  void *twolame_handle;
  twolame_options *twolame_lameopts;
  twolame_options *(*twolame_init)(void);
  void (*twolame_set_mode)(twolame_options *,TWOLAME_MPEG_mode);
  void (*twolame_set_num_channels)(twolame_options *,int);
  void (*twolame_set_in_samplerate)(twolame_options *,int);
  void (*twolame_set_out_samplerate)(twolame_options *,int);
  void (*twolame_set_bitrate)(twolame_options *,int);
  int (*twolame_init_params)(twolame_options *);
  void (*twolame_close)(twolame_options **);
  int (*twolame_encode_buffer_interleaved)(twolame_options *,const short int[],
					   int,unsigned char *,int);
  int (*twolame_encode_buffer_float32_interleaved)
    (twolame_options *,const float[],int,unsigned char *,int);
  int (*twolame_encode_flush)(twolame_options *,unsigned char *,int);
  int (*twolame_set_energy_levels)(twolame_options *,int);
  int (*twolame_set_VBR)(twolame_options *, int);
  int (*twolame_set_VBR_level)(twolame_options *, float);
#endif  // HAVE_TWOLAME
};


#endif  // MPEGL2CODEC_H
