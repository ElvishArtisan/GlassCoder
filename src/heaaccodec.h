// heaaccodec.h
//
// Codec class for MPEG-4 Advanced Audio Coding High Efficiency Profile
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

#ifndef HEAACCODEC_H
#define HEAACCODEC_H

#ifdef HAVE_AACPLUS
#include <aacplus.h>
#endif  // HAVE_AACPLUS

#include "codec.h"

class HeAacCodec : public Codec
{
  Q_OBJECT;
 public:
  HeAacCodec(Ringbuffer *ring,QObject *parent=0);
  ~HeAacCodec();
  QString contentType() const;
  unsigned pcmFrames() const;
  bool startCodec();

 protected:
  void encodeData(Connector *conn,const float *pcm,int frames);

 private:
#ifdef HAVE_AACPLUS
  void *heaac_handle;
  aacplusEncHandle heaac_encoder;
  aacplusEncConfiguration *(*aacplusEncGetCurrentConfiguration)
    (aacplusEncHandle);
  int (*aacplusEncSetConfiguration)
    (aacplusEncHandle,aacplusEncConfiguration *);
  aacplusEncHandle (*aacplusEncOpen)(unsigned long,unsigned int,
				     unsigned long *,unsigned long *);
  int (*aacplusEncGetDecoderSpecificInfo)(aacplusEncHandle, unsigned char **,
                                          unsigned long *);
  int (*aacplusEncEncode)(aacplusEncHandle, int32_t *, unsigned int,
                         unsigned char *,unsigned int);
  int (*aacplusEncClose)(aacplusEncHandle);
#endif  // HAVE_AACPLUS
  unsigned long heaac_input_samples;
  unsigned long heaac_buffer_size;
  unsigned char *heaac_buffer;
};


#endif  // HEAACCODEC_H
