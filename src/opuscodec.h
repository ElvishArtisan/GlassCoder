// opuscodec.h
//
// Codec class for Opus [RFC 6716]
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

#ifndef OPUSCODEC_H
#define OPUSCODEC_H

#ifdef HAVE_OPUS
#include <opus/opus.h>
#endif  // HAVE_OPUS

#include "codec.h"

class OpusCodec : public Codec
{
  Q_OBJECT;
 public:
  OpusCodec(Ringbuffer *ring,QObject *parent=0);
  QString contentType() const;
  unsigned pcmFrames() const;
  QString defaultExtension() const;
  bool startCodec();

 protected:
  void encodeData(Connector *conn,const float *pcm,int frames);

 private:
#ifdef HAVE_OPUS
  void *opus_handle;
  OpusEncoder *opus_encoder;
  int (*opus_encoder_get_size)(int);
  OpusEncoder *(*opus_encoder_create)(opus_int32, int, int, int *);
  int (*opus_encoder_init)(OpusEncoder *, opus_int32, int, int);
  opus_int32 (*opus_encode)(OpusEncoder *, const opus_int16 *, int,
			    unsigned char *, opus_int32);
  opus_int32 (*opus_encode_float)(OpusEncoder *, const float *, int,
				  unsigned char *, opus_int32);
  void (*opus_encoder_destroy)(OpusEncoder *);
  int (*opus_encoder_ctl)(OpusEncoder *, int request,...);
  const char *(*opus_strerror)(int error);
#endif  // HAVE_OPUS
};


#endif  // OPUSCODEC_H
