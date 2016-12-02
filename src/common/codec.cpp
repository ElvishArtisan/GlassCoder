// codec.cpp
//
// Abstract base class for audio codecs.
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

#include "codec.h"
#include "logging.h"

Codec::Codec(Codec::Type type,Ringbuffer *ring,QObject *parent)
{
  codec_ring1=ring;
  codec_bitrate=128;
  codec_channels=2;
  codec_quality=0.5;
  codec_source_samplerate=48000;
  codec_stream_samplerate=48000;
  codec_complete_frames=false;
  codec_ring2=NULL;
}


Codec::~Codec()
{
  if(codec_src_state!=NULL) {
    src_delete(codec_src_state);
  }
  if(codec_src_data!=NULL) {
    delete codec_src_data;
  }
  if((codec_ring2!=codec_ring1)&&(codec_ring2!=NULL)) {
    delete codec_ring2;
  }
}


unsigned Codec::bitrate() const
{
  return codec_bitrate;
}


void Codec::setBitrate(unsigned rate)
{
  codec_bitrate=rate;
}


unsigned Codec::channels() const
{
  return codec_channels;
}


void Codec::setChannels(unsigned chans)
{
  codec_channels=chans;
}


double Codec::quality() const
{
  return codec_quality;
}


void Codec::setQuality(double qual)
{
  codec_quality=qual;
}


unsigned Codec::sourceSamplerate() const
{
  return codec_source_samplerate;
}


void Codec::setSourceSamplerate(unsigned rate)
{
  codec_source_samplerate=rate;
}


unsigned Codec::streamSamplerate() const
{
  return codec_stream_samplerate;
}


void Codec::setStreamSamplerate(unsigned rate)
{
  codec_stream_samplerate=rate;
}


bool Codec::completeFrames() const
{
  return codec_complete_frames;
}


void Codec::setCompleteFrames(bool state)
{
  codec_complete_frames=state;
}


QByteArray Codec::streamPrologue() const
{
  return QByteArray();
}


bool Codec::start()
{
  int err;

  if(codec_source_samplerate==codec_stream_samplerate) {
    codec_pcm_buffer[0]=new float[MAX_AUDIO_CHANNELS*MAX_AUDIO_BUFFER];
    codec_pcm_buffer[1]=NULL;
    codec_pcm_in=codec_pcm_buffer[0];
    codec_pcm_out=codec_pcm_buffer[0];
    codec_src_state=NULL;
    codec_src_data=NULL;
    codec_ring2=codec_ring1;
  }
  else {
    codec_pcm_buffer[0]=new float[MAX_AUDIO_CHANNELS*MAX_AUDIO_BUFFER];
    codec_pcm_buffer[1]=new float[MAX_AUDIO_CHANNELS*MAX_AUDIO_BUFFER*6];
    codec_pcm_in=codec_pcm_buffer[0];
    codec_pcm_out=codec_pcm_buffer[1];
    if((codec_src_state=src_new(SRC_SINC_FASTEST,codec_channels,&err))==NULL) {
      Log(LOG_ERR,"unable to create sample rate converter");
      return false;
    }
    codec_src_data=new SRC_DATA;
    memset(codec_src_data,0,sizeof(SRC_DATA));
    codec_src_data->data_in=codec_pcm_buffer[0];
    codec_src_data->data_out=codec_pcm_buffer[1];
    codec_src_data->output_frames=MAX_AUDIO_BUFFER*6;
    codec_src_data->src_ratio=
      (double)codec_stream_samplerate/(double)codec_source_samplerate;
    codec_ring2=new Ringbuffer(262144,codec_channels);
  }
  return startCodec();
}


QString Codec::codecTypeText(Codec::Type type)
{
  QString ret=tr("Unknown");

  switch(type) {
  case Codec::TypeFdk:
    ret=tr("MPEG-4 AAC High Efficiency");
    break;

  case Codec::TypeMpegL2:
    ret=tr("MPEG-1 Layer 2");
    break;
 
  case Codec::TypeMpegL3:
    ret=tr("MPEG-1 Layer 3");
    break;
 
  case Codec::TypeOpus:
    ret=tr("Ogg Opus");
    break;

  case Codec::TypePcm16:
    ret=tr("PCM16 Uncompressed");
    break;

  case Codec::TypeVorbis:
    ret=tr("Ogg Vorbis");
    break;

  case Codec::TypeLast:
    break;
  }

  return ret;
}


QString Codec::optionKeyword(Codec::Type type)
{
  QString ret;

  switch(type) {
  case Codec::TypeFdk:
    ret="aacp";
    break;

  case Codec::TypeMpegL2:
    ret="mp2";
    break;
 
  case Codec::TypeMpegL3:
    ret="mp3";
    break;
 
  case Codec::TypeOpus:
    ret="opus";
    break;

  case Codec::TypePcm16:
    ret="pcm16";
    break;

  case Codec::TypeVorbis:
    ret="vorbis";
    break;

  case Codec::TypeLast:
    break;
  }

  return ret;
}


Codec::Type Codec::codecType(const QString &key)
{
  Codec::Type ret=Codec::TypeLast;

  for(int i=0;i<Codec::TypeLast;i++) {
    if(optionKeyword((Codec::Type)i)==key.toLower()) {
      ret=(Codec::Type)i;
    }
  }

  return ret;
}


void Codec::encode(Connector *conn)
{
  int n;
  int err=0;

  if(codec_src_state!=NULL) {
    while(codec_ring1->readSpace()>=pcmFrames()) {
      n=codec_ring1->read(codec_pcm_in,pcmFrames());
      codec_src_data->input_frames=n;
      if((err=src_process(codec_src_state,codec_src_data))!=0) {
	Log(LOG_WARNING,QString().sprintf("SRC error [%s]",src_strerror(err)));
	continue;
      }
      n=codec_src_data->output_frames_gen;
      codec_ring2->write(codec_pcm_out,n);
    }
  }
  while(codec_ring2->readSpace()>=pcmFrames()) {
    n=codec_ring2->read(codec_pcm_in,pcmFrames());
    encodeData(conn,codec_pcm_in,n);
  }
}


Ringbuffer *Codec::ring()
{
  return codec_ring1;
}
