// opuscodec.cpp
//
// Codec class for Advanced Audio Coding (AAC)
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

#include <samplerate.h>

#include "logging.h"
#include "opuscodec.h"

OpusCodec::OpusCodec(Ringbuffer *ring,QObject *parent)
  : Codec(Codec::TypeOpus,ring,parent)
{
  opus_packet_number=0;
  opus_packet_granulepos=0;
}


QByteArray OpusCodec::streamPrologue() const
{
  return opus_stream_prologue;
}


bool OpusCodec::isAvailable() const
{
#ifdef HAVE_OPUS
  return (dlopen("libopus.so.0",RTLD_LAZY)!=NULL)&&
    (dlopen("libogg.so.0",RTLD_LAZY)!=NULL);
#else
  return false;
#endif  // HAVE_OPUS
}


QString OpusCodec::contentType() const
{
  return "audio/ogg";
}


unsigned OpusCodec::pcmFrames() const
{
  return 2880;
}


QString OpusCodec::defaultExtension() const
{
  return QString("ogg");
}


QString OpusCodec::formatIdentifier() const
{
  return QString();
}


bool OpusCodec::startCodec()
{
#ifdef HAVE_OPUS
  int err;

  //
  // Load Libraries
  //
  opus_handle=dlopen("libopus.so.0",RTLD_LAZY);

  if(opus_handle==NULL) {
    Log(LOG_ERR,"unsupported audio format (library not found)");
    return false;
  }
  *(void **)(&opus_encoder_create)=dlsym(opus_handle,"opus_encoder_create");
  *(void **)(&opus_encoder_init)=dlsym(opus_handle,"opus_encoder_init");
  *(void **)(&opus_encode)=dlsym(opus_handle,"opus_encode");
  *(void **)(&opus_encode_float)=dlsym(opus_handle,"opus_encode_float");
  *(void **)(&opus_encoder_destroy)=dlsym(opus_handle,"opus_encoder_destroy");
  *(void **)(&opus_encoder_ctl)=dlsym(opus_handle,"opus_encoder_ctl");
  *(void **)(&opus_strerror)=dlsym(opus_handle,"opus_strerror");

  opus_ogg_handle=dlopen("libogg.so.0",RTLD_LAZY);
  if(opus_ogg_handle==NULL) {
    Log(LOG_ERR,"unsupported audio format (library not found)");
    return false;
  }
  *(void **)(&oggpack_writeinit)=dlsym(opus_ogg_handle,"oggpack_writeint");
  *(void **)(&oggpack_writecheck)=dlsym(opus_ogg_handle,"oggpack_writecheck");
  *(void **)(&oggpack_writetrunc)=dlsym(opus_ogg_handle,"oggpack_writetrunc");
  *(void **)(&oggpack_writealign)=dlsym(opus_ogg_handle,"oggpack_writealign");
  *(void **)(&oggpack_writecopy)=dlsym(opus_ogg_handle,"oggpack_writecopy");
  *(void **)(&oggpack_reset)=dlsym(opus_ogg_handle,"oggpack_reset");
  *(void **)(&oggpack_writeclear)=dlsym(opus_ogg_handle,"oggpack_writeclear");
  *(void **)(&oggpack_readinit)=dlsym(opus_ogg_handle,"oggpack_readinit");
  *(void **)(&oggpack_write)=dlsym(opus_ogg_handle,"oggpack_write");
  *(void **)(&oggpack_look)=dlsym(opus_ogg_handle,"oggpack_look");
  *(void **)(&oggpack_look1)=dlsym(opus_ogg_handle,"oggpack_look1");
  *(void **)(&oggpack_adv)=dlsym(opus_ogg_handle,"oggpack_adv");
  *(void **)(&oggpack_adv1)=dlsym(opus_ogg_handle,"oggpack_adv1");
  *(void **)(&oggpack_read)=dlsym(opus_ogg_handle,"oggpack_read");
  *(void **)(&oggpack_read1)=dlsym(opus_ogg_handle,"oggpack_read1");
  *(void **)(&oggpack_bytes)=dlsym(opus_ogg_handle,"oggpack_bytes");
  *(void **)(&oggpack_bits)=dlsym(opus_ogg_handle,"oggpack_bits");
    *(void **)(&oggpack_get_buffer)=
      dlsym(opus_ogg_handle,"oggpack_get_buffer");
  *(void **)(&oggpackB_writeinit)=dlsym(opus_ogg_handle,"oggpackB_writeinit");
  *(void **)(&oggpackB_writecheck)=
    dlsym(opus_ogg_handle,"oggpackB_writecheck");
  *(void **)(&oggpackB_writetrunc)=
    dlsym(opus_ogg_handle,"oggpackB_writetrunc");
  *(void **)(&oggpackB_writealign)=
    dlsym(opus_ogg_handle,"oggpackB_writealign");
  *(void **)(&oggpackB_writecopy)=dlsym(opus_ogg_handle,"oggpackB_writecopy");
  *(void **)(&oggpackB_reset)=dlsym(opus_ogg_handle,"oggpackB_reset");
  *(void **)(&oggpackB_writeclear)=
    dlsym(opus_ogg_handle,"oggpackB_writeclear");
  *(void **)(&oggpackB_readinit)=dlsym(opus_ogg_handle,"oggpackB_readini");
  *(void **)(&oggpackB_write)=dlsym(opus_ogg_handle,"oggpackB_write");
  *(void **)(&oggpackB_look)=dlsym(opus_ogg_handle,"oggpackB_look");
  *(void **)(&oggpackB_look1)=dlsym(opus_ogg_handle,"oggpackB_look1");
  *(void **)(&oggpackB_adv)=dlsym(opus_ogg_handle,"oggpackB_adv");
  *(void **)(&oggpackB_adv1)=dlsym(opus_ogg_handle,"oggpackB_adv1");
  *(void **)(&oggpackB_read)=dlsym(opus_ogg_handle,"oggpackB_read");
  *(void **)(&oggpackB_read1)=dlsym(opus_ogg_handle,"oggpackB_read1");
  *(void **)(&oggpackB_bytes)=dlsym(opus_ogg_handle,"oggpackB_bytes");
  *(void **)(&oggpackB_bits)=dlsym(opus_ogg_handle,"oggpackB_bits");
  *(void **)(&oggpackB_get_buffer)=
    dlsym(opus_ogg_handle,"oggpackB_get_buffer");
  *(void **)(&ogg_stream_packetin)=
    dlsym(opus_ogg_handle,"ogg_stream_packetin");
  *(void **)(&ogg_stream_iovecin)=dlsym(opus_ogg_handle,"ogg_stream_iovecin");
  *(void **)(&ogg_stream_pageout)=dlsym(opus_ogg_handle,"ogg_stream_pageout");
  *(void **)(&ogg_stream_flush)=dlsym(opus_ogg_handle,"ogg_stream_flush");
  *(void **)(&ogg_sync_init)=dlsym(opus_ogg_handle,"ogg_sync_init");
  *(void **)(&ogg_sync_clear)=dlsym(opus_ogg_handle,"ogg_sync_clear");
  *(void **)(&ogg_sync_reset)=dlsym(opus_ogg_handle,"ogg_sync_reset");
  *(void **)(&ogg_sync_destroy)=dlsym(opus_ogg_handle,"ogg_sync_destroy");
  *(void **)(&ogg_sync_check)=dlsym(opus_ogg_handle,"ogg_sync_check");
  *(void **)(&ogg_sync_buffer)=dlsym(opus_ogg_handle,"ogg_sync_buffer");
  *(void **)(&ogg_sync_wrote)=dlsym(opus_ogg_handle,"ogg_sync_wrote");
  *(void **)(&ogg_sync_pageseek)=dlsym(opus_ogg_handle,"ogg_sync_pageseek");
  *(void **)(&ogg_sync_pageout)=dlsym(opus_ogg_handle,"ogg_sync_pageout");
  *(void **)(&ogg_stream_pagein)=dlsym(opus_ogg_handle,"ogg_stream_pagein");
  *(void **)(&ogg_stream_packetout)=
    dlsym(opus_ogg_handle,"ogg_stream_packetout");
  *(void **)(&ogg_stream_packetpeek)=
    dlsym(opus_ogg_handle,"ogg_stream_packetpeek");
  *(void **)(&ogg_stream_init)=dlsym(opus_ogg_handle,"ogg_stream_init");
  *(void **)(&ogg_stream_clear)=dlsym(opus_ogg_handle,"ogg_stream_clear");
  *(void **)(&ogg_stream_reset)=dlsym(opus_ogg_handle,"ogg_stream_reset");
  *(void **)(&ogg_stream_reset_serialno)=
    dlsym(opus_ogg_handle,"ogg_stream_reset_serialn");
  *(void **)(&ogg_stream_destroy)=dlsym(opus_ogg_handle,"ogg_stream_destroy");
  *(void **)(&ogg_stream_check)=dlsym(opus_ogg_handle,"ogg_stream_check");
  *(void **)(&ogg_stream_eos)=dlsym(opus_ogg_handle,"ogg_stream_eos");
  *(void **)(&ogg_page_checksum_set)=
    dlsym(opus_ogg_handle,"ogg_page_checksum_set");
  *(void **)(&ogg_page_version)=dlsym(opus_ogg_handle,"ogg_page_version");
  *(void **)(&ogg_page_continued)=dlsym(opus_ogg_handle,"ogg_page_continued");
  *(void **)(&ogg_page_bos)=dlsym(opus_ogg_handle,"ogg_page_bos");
  *(void **)(&ogg_page_eos)=dlsym(opus_ogg_handle,"ogg_page_eos");
  *(void **)(&ogg_page_granulepos)=
    dlsym(opus_ogg_handle,"ogg_page_granulepos");
  *(void **)(&ogg_page_serialno)=dlsym(opus_ogg_handle,"ogg_page_serialno");
  *(void **)(&ogg_page_pageno)=dlsym(opus_ogg_handle,"ogg_page_pageno");
  *(void **)(&ogg_page_packets)=dlsym(opus_ogg_handle,"ogg_page_packets");
  *(void **)(&ogg_packet_clear)=dlsym(opus_ogg_handle,"ogg_packet_clear");

  //
  // Initialize Encoder Instance
  //
  if((opus_encoder=opus_encoder_create(streamSamplerate(),channels(),
				       OPUS_APPLICATION_AUDIO,&err))==NULL) {
    Log(LOG_ERR,QString().sprintf("unable to create codec [%s]",
				  opus_strerror(err)));
    return false;
  }
  if(bitrate()==0) {
    opus_encoder_ctl(opus_encoder,OPUS_SET_VBR(1));
    opus_encoder_ctl(opus_encoder,OPUS_SET_COMPLEXITY(10.0*quality()));
  }
  else {
    opus_encoder_ctl(opus_encoder,OPUS_SET_VBR(0));
    opus_encoder_ctl(opus_encoder,OPUS_SET_BITRATE(1000*bitrate()));
  }
  opus_encoder_ctl(opus_encoder,
		   OPUS_SET_COMPLEXITY(OPUSCODEC_ENCODER_COMPLEXITY));

  //
  // Initialize the stream
  //
  ogg_stream_init(&opus_ogg_stream,rand());

  //
  // Header Packet
  //
  QByteArray info=MakeInfoHeader(channels(),streamSamplerate());
  opus_ogg_packet.packet=(unsigned char *)info.constData();
  opus_ogg_packet.bytes=info.length();
  opus_ogg_packet.b_o_s=1;
  opus_ogg_packet.e_o_s=0;
  opus_ogg_packet.granulepos=0;
  opus_ogg_packet.packetno=opus_packet_number++;
  ogg_stream_packetin(&opus_ogg_stream,&opus_ogg_packet);
  while(ogg_stream_flush(&opus_ogg_stream,&opus_ogg_page)) {
    opus_stream_prologue.append((const char *)opus_ogg_page.header,
			     opus_ogg_page.header_len);
    opus_stream_prologue.append((const char *)opus_ogg_page.body,
			     opus_ogg_page.body_len);
  }

  //
  // Comment Packet
  //
  QByteArray comment=MakeCommentHeader();
  opus_ogg_packet.packet=(unsigned char *)comment.constData();
  opus_ogg_packet.bytes=comment.length();
  opus_ogg_packet.b_o_s=0;
  opus_ogg_packet.e_o_s=0;
  opus_ogg_packet.granulepos=0;
  opus_ogg_packet.packetno=opus_packet_number++;
  ogg_stream_packetin(&opus_ogg_stream,&opus_ogg_packet);
  while(ogg_stream_flush(&opus_ogg_stream,&opus_ogg_page)!=0) {
    opus_stream_prologue.append((const char *)opus_ogg_page.header,
			     opus_ogg_page.header_len);
    opus_stream_prologue.append((const char *)opus_ogg_page.body,
			     opus_ogg_page.body_len);
  }
  return true;
#else
  Log(LOG_ERR,"unsupported audio format (no build support)");
  return false;
#endif  // HAVE_OPUS
}


void OpusCodec::encodeData(Connector *conn,const float *pcm,int frames)
{
#ifdef HAVE_OPUS
  int s;
  unsigned char data[4096];

  if(!opus_prologue_sent) {
    conn->writeData(0,(const unsigned char *)opus_stream_prologue.constData(),
		    opus_stream_prologue.size());
    opus_prologue_sent=true;
  }

  if((s=opus_encode_float(opus_encoder,pcm,frames,data,4096))>1) {
    opus_packet_granulepos+=frames;
    opus_ogg_packet.packet=data;
    opus_ogg_packet.bytes=s;
    opus_ogg_packet.b_o_s=0;
    opus_ogg_packet.e_o_s=0;
    opus_ogg_packet.granulepos=opus_packet_granulepos;
    opus_ogg_packet.packetno=opus_packet_number++;
    ogg_stream_packetin(&opus_ogg_stream,&opus_ogg_packet);
    while(ogg_stream_pageout(&opus_ogg_stream,&opus_ogg_page)!=0) {
      conn->
	writeData(frames,opus_ogg_page.header,opus_ogg_page.header_len);
      conn->writeData(0,opus_ogg_page.body,opus_ogg_page.body_len);
    }
  }
  else {
    Log(LOG_WARNING,QString().sprintf("opus encoding error %d",s));
  }

#endif  // HAVE_OPUS
}


QByteArray OpusCodec::MakeInfoHeader(unsigned chans,unsigned samprate)
{
  QByteArray hdr(19,0);

  hdr[0]='O';
  hdr[1]='p';
  hdr[2]='u';
  hdr[3]='s';
  hdr[4]='H';
  hdr[5]='e';
  hdr[6]='a';
  hdr[7]='d';
  hdr[8]=1;
  hdr[9]=0xFF&chans;
  hdr[12]=0xFF&samprate;
  hdr[13]=0xFF&(samprate>>8);
  hdr[14]=0xFF&(samprate>>16);
  hdr[15]=0xFF&(samprate>>24);

  return hdr;
}


QByteArray OpusCodec::MakeCommentHeader()
{
  QString version=QString("GlassCoder ")+VERSION;
  QByteArray hdr(12,0);
  hdr[0]='O';
  hdr[1]='p';
  hdr[2]='u';
  hdr[3]='s';
  hdr[4]='T';
  hdr[5]='a';
  hdr[6]='g';
  hdr[7]='s';
  hdr[8]=version.toUtf8().length();
  hdr[9]=0;
  hdr[10]=0;
  hdr[11]=0;
  hdr.append(version.toUtf8());
  hdr.append((char)0);
  hdr.append((char)0);
  hdr.append((char)0);
  hdr.append((char)0);

  return hdr;
}
