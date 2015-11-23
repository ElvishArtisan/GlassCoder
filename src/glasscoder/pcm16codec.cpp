// pcm16codec.cpp
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

#include <arpa/inet.h>

#include <samplerate.h>

#include "pcm16codec.h"

Pcm16Codec::Pcm16Codec(Ringbuffer *ring,QObject *parent)
  : Codec(Codec::TypePcm16,ring,parent)
{
}


bool Pcm16Codec::isAvailable() const
{
  return true;
}


QString Pcm16Codec::contentType() const
{
  return QString("audio/x-wav");
}


unsigned Pcm16Codec::pcmFrames() const
{
  return 1024;
}


QString Pcm16Codec::defaultExtension() const
{
  return QString("wav");
}


QString Pcm16Codec::formatIdentifier() const
{
  return QString("wav");
}


bool Pcm16Codec::startCodec()
{
  return true;
}


void Pcm16Codec::encodeData(Connector *conn,const float *pcm,int frames)
{
  src_float_to_short_array(pcm,pcm16_buffer,frames*channels());
  for(int i=0;i<(frames*(int)channels());i++) {
    pcm16_buffer[i]=htons(pcm16_buffer[i]);
  }
  conn->writeData(frames,(const unsigned char *)pcm16_buffer,
		  frames*channels()*2);
}
