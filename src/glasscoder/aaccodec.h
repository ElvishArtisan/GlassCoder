// aaccodec.h
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

#ifndef AACCODEC_H
#define AACCODEC_H

#ifdef HAVE_FAAC
#include <faac.h>
#endif  // HAVE_FAAC

#include "codec.h"

class AacCodec : public Codec
{
  Q_OBJECT;
 public:
  AacCodec(Ringbuffer *ring,QObject *parent=0);
  ~AacCodec();
  bool isAvailable() const;
  QString contentType() const;
  unsigned pcmFrames() const;
  QString defaultExtension() const;
  QString formatIdentifier() const;
  bool startCodec();

 protected:
  void encodeData(Connector *conn,const float *pcm,int frames);

 private:
#ifdef HAVE_FAAC
  void *aac_handle;
  faacEncHandle aac_encoder;
  faacEncHandle FAACAPI (*faacEncOpen)
    (unsigned long,unsigned int,unsigned long *,unsigned long *);
  void FAACAPI (*faacEncClose)(faacEncHandle);
  faacEncConfigurationPtr FAACAPI (*faacEncGetCurrentConfiguration)
    (faacEncHandle);
  int FAACAPI (*faacEncSetConfiguration)(faacEncHandle,faacEncConfigurationPtr);
  int FAACAPI (*faacEncEncode)
    (faacEncHandle,int32_t *,unsigned int,unsigned char *,unsigned int);
#endif  // HAVE_FAAC
  unsigned long aac_input_samples;
  unsigned long aac_buffer_size;
  unsigned char *aac_buffer;
};


#endif  // AACCODEC_H
