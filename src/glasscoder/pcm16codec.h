// pcm16codec.h
//
// Codec class for 16 bit PCM (little endian)
//
//   (C) Copyright 2015 Fred Gleason <fredg@paravelsystems.com>
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

#ifndef PCM16CODEC_H
#define PCM16CODEC_H

#include "codec.h"

#define PCM16_MAX_FRAMES 1024

class Pcm16Codec : public Codec
{
  Q_OBJECT;
 public:
  Pcm16Codec(Ringbuffer *ring,QObject *parent=0);
  bool isAvailable() const;
  QString contentType() const;
  unsigned pcmFrames() const;
  QString defaultExtension() const;
  QString formatIdentifier() const;
  bool startCodec();

 protected:
  void encodeData(Connector *conn,const float *pcm,int frames);

 private:
  short pcm16_buffer[PCM16_MAX_FRAMES*MAX_AUDIO_CHANNELS];
};


#endif  // PCM16CODEC_H
