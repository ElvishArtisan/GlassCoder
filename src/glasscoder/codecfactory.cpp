// codecfactory.cpp
//
// Instantiate Codec classes
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

#include "codecfactory.h"
#include "fdkcodec.h"
#include "mpegl2codec.h"
#include "mpegl3codec.h"
#include "opuscodec.h"
#include "pcm16codec.h"
#include "vorbiscodec.h"

Codec *CodecFactory(Codec::Type type,Ringbuffer *ring,QObject *parent)
{
  Codec *cdc=NULL;

  switch(type) {
  case Codec::TypeFdk:
    cdc=new FdkCodec(ring,parent);
    break;

  case Codec::TypeMpegL2:
    cdc=new MpegL2Codec(ring,parent);
    break;

  case Codec::TypeMpegL3:
    cdc=new MpegL3Codec(ring,parent);
    break;

  case Codec::TypeOpus:
    cdc=new OpusCodec(ring,parent);
    break;

  case Codec::TypePcm16:
    cdc=new Pcm16Codec(ring,parent);
    break;

  case Codec::TypeVorbis:
    cdc=new VorbisCodec(ring,parent);
    break;

  case Codec::TypeLast:
    break;
  }

  return cdc;
}
