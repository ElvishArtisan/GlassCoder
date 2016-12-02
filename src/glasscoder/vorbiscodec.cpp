// vorbiscodec.cpp
//
// Codec class for OggVorbis
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

#include <samplerate.h>

#include "logging.h"
#include "vorbiscodec.h"

VorbisCodec::VorbisCodec(Ringbuffer *ring,QObject *parent)
  : Codec(Codec::TypeVorbis,ring,parent)
{
  vorbis_prologue_sent=false;
  vorbis_buffer=NULL;
}


VorbisCodec::~VorbisCodec()
{
  if(vorbis_buffer!=NULL) {
    delete vorbis_buffer;
  }
}


bool VorbisCodec::isAvailable() const
{
#ifdef HAVE_VORBIS
  return (dlopen("libvorbisenc.so.2",RTLD_LAZY)!=NULL)&&
    (dlopen("libvorbis.so.0",RTLD_LAZY)!=NULL)&&
    (dlopen("libogg.so.0",RTLD_LAZY)!=NULL);
#else
  return false;
#endif  // HAVE_VORBIS
}


QString VorbisCodec::contentType() const
{
  return "audio/ogg";
}


unsigned VorbisCodec::pcmFrames() const
{
  return 2048;
}


QString VorbisCodec::defaultExtension() const
{
  return QString("ogg");
}


QString VorbisCodec::formatIdentifier() const
{
  return QString();
}


QByteArray VorbisCodec::streamPrologue() const
{
  return vorbis_stream_prologue;
}


bool VorbisCodec::startCodec()
{
#ifdef HAVE_VORBIS
  ogg_packet header;
  ogg_packet comment;
  ogg_packet codebook;
  vorbis_comment vorbis_comment;

  //
  // Load Library
  //
  vorbis_vorbisenc_handle=dlopen("libvorbisenc.so.2",RTLD_LAZY);
  if(vorbis_vorbisenc_handle==NULL) {
    Log(LOG_ERR,"unsupported audio format (library not found)");
    return false;
  }
  vorbis_vorbis_handle=dlopen("libvorbis.so.0",RTLD_LAZY);
  if(vorbis_vorbis_handle==NULL) {
    Log(LOG_ERR,"unsupported audio format (library not found)");
    return false;
  }
  vorbis_ogg_handle=dlopen("libogg.so.0",RTLD_LAZY);
  if(vorbis_ogg_handle==NULL) {
    Log(LOG_ERR,"unsupported audio format (library not found)");
    return false;
  }

  *(void **)(&vorbis_encode_init)=
    dlsym(vorbis_vorbisenc_handle,"vorbis_encode_init");
  *(void **)(&vorbis_encode_setup_managed)=
    dlsym(vorbis_vorbisenc_handle,"vorbis_encode_setup_managed");
  *(void **)(&vorbis_encode_setup_vbr)=
    dlsym(vorbis_vorbisenc_handle,"vorbis_encode_setup_vbr");

  *(void **)(&vorbis_encode_init_vbr)=
    dlsym(vorbis_vorbisenc_handle,"vorbis_encode_init_vbr");
  *(void **)(&vorbis_encode_setup_init)=
    dlsym(vorbis_vorbisenc_handle,"vorbis_encode_setup_init");
  *(void **)(&vorbis_encode_ctl)=
    dlsym(vorbis_vorbisenc_handle,"vorbis_encode_ctl");

  *(void **)(&vorbis_info_init)=
    dlsym(vorbis_vorbis_handle,"vorbis_info_init");
  *(void **)(&vorbis_info_clear)=
    dlsym(vorbis_vorbis_handle,"vorbis_info_clear");
  *(void **)(&vorbis_info_blocksize)=
    dlsym(vorbis_vorbis_handle,"vorbis_info_blocksize");
  *(void **)(&vorbis_comment_init)=
    dlsym(vorbis_vorbis_handle,"vorbis_comment_init");
  *(void **)(&vorbis_comment_add)=
    dlsym(vorbis_vorbis_handle,"vorbis_comment_add");
  *(void **)(&vorbis_comment_add_tag)=
    dlsym(vorbis_vorbis_handle,"vorbis_comment_add_tag");
  *(void **)(&vorbis_comment_query)=
    dlsym(vorbis_vorbis_handle,"vorbis_comment_query");
  *(void **)(&vorbis_comment_query_count)=
    dlsym(vorbis_vorbis_handle,"vorbis_comment_query_count");
  *(void **)(&vorbis_comment_clear)=
    dlsym(vorbis_vorbis_handle,"vorbis_comment_clear");
  *(void **)(&vorbis_block_init)=
    dlsym(vorbis_vorbis_handle,"vorbis_block_init");
  *(void **)(&vorbis_block_clear)=
    dlsym(vorbis_vorbis_handle,"vorbis_block_clear");
  *(void **)(&vorbis_dsp_clear)=
    dlsym(vorbis_vorbis_handle,"vorbis_dsp_clear");
  *(void **)(&vorbis_granule_time)=
    dlsym(vorbis_vorbis_handle,"vorbis_granule_time");
  *(void **)(&vorbis_version_string)=
    dlsym(vorbis_vorbis_handle,"vorbis_version_string");
  *(void **)(&vorbis_analysis_init)=
    dlsym(vorbis_vorbis_handle,"vorbis_analysis_init");
  *(void **)(&vorbis_commentheader_out)=
    dlsym(vorbis_vorbis_handle,"vorbis_commentheader_out");
  *(void **)(&vorbis_analysis_headerout)=
    dlsym(vorbis_vorbis_handle,"vorbis_analysis_headerout");
  *(void **)(&vorbis_analysis_buffer)=
    dlsym(vorbis_vorbis_handle,"vorbis_analysis_buffer");
  *(void **)(&vorbis_analysis_wrote)=
    dlsym(vorbis_vorbis_handle,"vorbis_analysis_wrote");
  *(void **)(&vorbis_analysis_blockout)=
    dlsym(vorbis_vorbis_handle,"vorbis_analysis_blockout");
  *(void **)(&vorbis_analysis)=
    dlsym(vorbis_vorbis_handle,"vorbis_analysis");
  *(void **)(&vorbis_bitrate_addblock)=
    dlsym(vorbis_vorbis_handle,"vorbis_bitrate_addblock");
  *(void **)(&vorbis_bitrate_flushpacket)=
    dlsym(vorbis_vorbis_handle,"vorbis_bitrate_flushpacket");
  *(void **)(&vorbis_synthesis_idheader)=
    dlsym(vorbis_vorbis_handle,"vorbis_synthesis_idheader");
  *(void **)(&vorbis_synthesis_headerin)=
    dlsym(vorbis_vorbis_handle,"vorbis_synthesis_headerin");
  *(void **)(&vorbis_synthesis_init)=
    dlsym(vorbis_vorbis_handle,"vorbis_synthesis_init");
  *(void **)(&vorbis_synthesis_restart)=
    dlsym(vorbis_vorbis_handle,"vorbis_synthesis_restart");
  *(void **)(&vorbis_synthesis)=
    dlsym(vorbis_vorbis_handle,"vorbis_synthesis");
  *(void **)(&vorbis_synthesis_trackonly)=
    dlsym(vorbis_vorbis_handle,"vorbis_synthesis_trackonly");
  *(void **)(&vorbis_synthesis_blockin)=
    dlsym(vorbis_vorbis_handle,"vorbis_synthesis_blockin");
  *(void **)(&vorbis_synthesis_pcmout)=
    dlsym(vorbis_vorbis_handle,"vorbis_synthesis_pcmout");
  *(void **)(&vorbis_synthesis_lapout)=
    dlsym(vorbis_vorbis_handle,"vorbis_synthesis_lapout");
  *(void **)(&vorbis_synthesis_read)=
    dlsym(vorbis_vorbis_handle,"vorbis_synthesis_read");
  *(void **)(&vorbis_packet_blocksize)=
    dlsym(vorbis_vorbis_handle,"vorbis_packets_blocksize");
  *(void **)(&vorbis_synthesis_halfrate)=
    dlsym(vorbis_vorbis_handle,"vorbis_synthesis_halfrate");
  *(void **)(&vorbis_synthesis_halfrate_p)=
    dlsym(vorbis_vorbis_handle,"vorbis_synthesis_halfrate");


  *(void **)(&oggpack_writeinit)=dlsym(vorbis_ogg_handle,"oggpack_writeint");
  *(void **)(&oggpack_writecheck)=dlsym(vorbis_ogg_handle,"oggpack_writecheck");
  *(void **)(&oggpack_writetrunc)=dlsym(vorbis_ogg_handle,"oggpack_writetrunc");
  *(void **)(&oggpack_writealign)=dlsym(vorbis_ogg_handle,"oggpack_writealign");
  *(void **)(&oggpack_writecopy)=dlsym(vorbis_ogg_handle,"oggpack_writecopy");
  *(void **)(&oggpack_reset)=dlsym(vorbis_ogg_handle,"oggpack_reset");
  *(void **)(&oggpack_writeclear)=dlsym(vorbis_ogg_handle,"oggpack_writeclear");
  *(void **)(&oggpack_readinit)=dlsym(vorbis_ogg_handle,"oggpack_readinit");
  *(void **)(&oggpack_write)=dlsym(vorbis_ogg_handle,"oggpack_write");
  *(void **)(&oggpack_look)=dlsym(vorbis_ogg_handle,"oggpack_look");
  *(void **)(&oggpack_look1)=dlsym(vorbis_ogg_handle,"oggpack_look1");
  *(void **)(&oggpack_adv)=dlsym(vorbis_ogg_handle,"oggpack_adv");
  *(void **)(&oggpack_adv1)=dlsym(vorbis_ogg_handle,"oggpack_adv1");
  *(void **)(&oggpack_read)=dlsym(vorbis_ogg_handle,"oggpack_read");
  *(void **)(&oggpack_read1)=dlsym(vorbis_ogg_handle,"oggpack_read1");
  *(void **)(&oggpack_bytes)=dlsym(vorbis_ogg_handle,"oggpack_bytes");
  *(void **)(&oggpack_bits)=dlsym(vorbis_ogg_handle,"oggpack_bits");
    *(void **)(&oggpack_get_buffer)=
      dlsym(vorbis_ogg_handle,"oggpack_get_buffer");
  *(void **)(&oggpackB_writeinit)=dlsym(vorbis_ogg_handle,"oggpackB_writeinit");
  *(void **)(&oggpackB_writecheck)=
    dlsym(vorbis_ogg_handle,"oggpackB_writecheck");
  *(void **)(&oggpackB_writetrunc)=
    dlsym(vorbis_ogg_handle,"oggpackB_writetrunc");
  *(void **)(&oggpackB_writealign)=
    dlsym(vorbis_ogg_handle,"oggpackB_writealign");
  *(void **)(&oggpackB_writecopy)=dlsym(vorbis_ogg_handle,"oggpackB_writecopy");
  *(void **)(&oggpackB_reset)=dlsym(vorbis_ogg_handle,"oggpackB_reset");
  *(void **)(&oggpackB_writeclear)=
    dlsym(vorbis_ogg_handle,"oggpackB_writeclear");
  *(void **)(&oggpackB_readinit)=dlsym(vorbis_ogg_handle,"oggpackB_readini");
  *(void **)(&oggpackB_write)=dlsym(vorbis_ogg_handle,"oggpackB_write");
  *(void **)(&oggpackB_look)=dlsym(vorbis_ogg_handle,"oggpackB_look");
  *(void **)(&oggpackB_look1)=dlsym(vorbis_ogg_handle,"oggpackB_look1");
  *(void **)(&oggpackB_adv)=dlsym(vorbis_ogg_handle,"oggpackB_adv");
  *(void **)(&oggpackB_adv1)=dlsym(vorbis_ogg_handle,"oggpackB_adv1");
  *(void **)(&oggpackB_read)=dlsym(vorbis_ogg_handle,"oggpackB_read");
  *(void **)(&oggpackB_read1)=dlsym(vorbis_ogg_handle,"oggpackB_read1");
  *(void **)(&oggpackB_bytes)=dlsym(vorbis_ogg_handle,"oggpackB_bytes");
  *(void **)(&oggpackB_bits)=dlsym(vorbis_ogg_handle,"oggpackB_bits");
  *(void **)(&oggpackB_get_buffer)=
    dlsym(vorbis_ogg_handle,"oggpackB_get_buffer");
  *(void **)(&ogg_stream_packetin)=
    dlsym(vorbis_ogg_handle,"ogg_stream_packetin");
  *(void **)(&ogg_stream_iovecin)=dlsym(vorbis_ogg_handle,"ogg_stream_iovecin");
  *(void **)(&ogg_stream_pageout)=dlsym(vorbis_ogg_handle,"ogg_stream_pageout");
  *(void **)(&ogg_stream_flush)=dlsym(vorbis_ogg_handle,"ogg_stream_flush");
  *(void **)(&ogg_sync_init)=dlsym(vorbis_ogg_handle,"ogg_sync_init");
  *(void **)(&ogg_sync_clear)=dlsym(vorbis_ogg_handle,"ogg_sync_clear");
  *(void **)(&ogg_sync_reset)=dlsym(vorbis_ogg_handle,"ogg_sync_reset");
  *(void **)(&ogg_sync_destroy)=dlsym(vorbis_ogg_handle,"ogg_sync_destroy");
  *(void **)(&ogg_sync_check)=dlsym(vorbis_ogg_handle,"ogg_sync_check");
  *(void **)(&ogg_sync_buffer)=dlsym(vorbis_ogg_handle,"ogg_sync_buffer");
  *(void **)(&ogg_sync_wrote)=dlsym(vorbis_ogg_handle,"ogg_sync_wrote");
  *(void **)(&ogg_sync_pageseek)=dlsym(vorbis_ogg_handle,"ogg_sync_pageseek");
  *(void **)(&ogg_sync_pageout)=dlsym(vorbis_ogg_handle,"ogg_sync_pageout");
  *(void **)(&ogg_stream_pagein)=dlsym(vorbis_ogg_handle,"ogg_stream_pagein");
  *(void **)(&ogg_stream_packetout)=
    dlsym(vorbis_ogg_handle,"ogg_stream_packetout");
  *(void **)(&ogg_stream_packetpeek)=
    dlsym(vorbis_ogg_handle,"ogg_stream_packetpeek");
  *(void **)(&ogg_stream_init)=dlsym(vorbis_ogg_handle,"ogg_stream_init");
  *(void **)(&ogg_stream_clear)=dlsym(vorbis_ogg_handle,"ogg_stream_clear");
  *(void **)(&ogg_stream_reset)=dlsym(vorbis_ogg_handle,"ogg_stream_reset");
  *(void **)(&ogg_stream_reset_serialno)=
    dlsym(vorbis_ogg_handle,"ogg_stream_reset_serialn");
  *(void **)(&ogg_stream_destroy)=dlsym(vorbis_ogg_handle,"ogg_stream_destroy");
  *(void **)(&ogg_stream_check)=dlsym(vorbis_ogg_handle,"ogg_stream_check");
  *(void **)(&ogg_stream_eos)=dlsym(vorbis_ogg_handle,"ogg_stream_eos");
  *(void **)(&ogg_page_checksum_set)=
    dlsym(vorbis_ogg_handle,"ogg_page_checksum_set");
  *(void **)(&ogg_page_version)=dlsym(vorbis_ogg_handle,"ogg_page_version");
  *(void **)(&ogg_page_continued)=dlsym(vorbis_ogg_handle,"ogg_page_continued");
  *(void **)(&ogg_page_bos)=dlsym(vorbis_ogg_handle,"ogg_page_bos");
  *(void **)(&ogg_page_eos)=dlsym(vorbis_ogg_handle,"ogg_page_eos");
  *(void **)(&ogg_page_granulepos)=
    dlsym(vorbis_ogg_handle,"ogg_page_granulepos");
  *(void **)(&ogg_page_serialno)=dlsym(vorbis_ogg_handle,"ogg_page_serialno");
  *(void **)(&ogg_page_pageno)=dlsym(vorbis_ogg_handle,"ogg_page_pageno");
  *(void **)(&ogg_page_packets)=dlsym(vorbis_ogg_handle,"ogg_page_packets");
  *(void **)(&ogg_packet_clear)=dlsym(vorbis_ogg_handle,"ogg_packet_clear");

  //
  // Initialize Encoder Instance
  //
  vorbis_info_init(&vorbis_vorbis_info);
  if(bitrate()==0) {
    if(vorbis_encode_init_vbr(&vorbis_vorbis_info,channels(),streamSamplerate(),
			      quality())!=0) {
      Log(LOG_ERR,"unable to initialize encoder");
      return false;
    }
  }
  else {
    if(vorbis_encode_init(&vorbis_vorbis_info,channels(),
			  streamSamplerate(),1000*bitrate(),
			  1000*bitrate(),1000*bitrate())!=0) {
      Log(LOG_ERR,"unable to initialize encoder");
      return false;
    }
  }
  vorbis_comment_init(&vorbis_comment);
  // Metadata stuff goes here...
  vorbis_analysis_init(&vorbis_vorbis_dsp,&vorbis_vorbis_info);
  vorbis_block_init(&vorbis_vorbis_dsp,&vorbis_vorbis_block);
  vorbis_analysis_headerout(&vorbis_vorbis_dsp,&vorbis_comment,
			    &header,&comment,&codebook);
  ogg_stream_init(&vorbis_ogg_stream,rand());
  ogg_stream_packetin(&vorbis_ogg_stream,&header);
  ogg_stream_packetin(&vorbis_ogg_stream,&comment);
  ogg_stream_packetin(&vorbis_ogg_stream,&codebook);
  while(ogg_stream_flush(&vorbis_ogg_stream,&vorbis_ogg_page)!=0) {
    vorbis_stream_prologue.append((const char *)vorbis_ogg_page.header,
			     vorbis_ogg_page.header_len);
    vorbis_stream_prologue.append((const char *)vorbis_ogg_page.body,
			     vorbis_ogg_page.body_len);
  }

  return true;
#else
  Log(LOG_ERR,"unsupported audio format (no build support)");
  return false;
#endif  // HAVE_VORBIS
}


void VorbisCodec::encodeData(Connector *conn,const float *pcm,int frames)
{
#ifdef HAVE_VORBIS
  float **vorbis;

  if(!vorbis_prologue_sent) {
    conn->writeData(0,(const unsigned char *)vorbis_stream_prologue.constData(),
		    vorbis_stream_prologue.size());
    vorbis_prologue_sent=true;
  }

  if((vorbis=vorbis_analysis_buffer(&vorbis_vorbis_dsp,frames))==NULL) {
    Log(LOG_ERR,"unable to allocate stream buffer");
    exit(256);
  }
  for(int i=0;i<frames;i++) {
    for(unsigned j=0;j<channels();j++) {
      vorbis[j][i]=pcm[channels()*i+j];
    }
  }
  vorbis_analysis_wrote(&vorbis_vorbis_dsp,frames);
  while(vorbis_analysis_blockout(&vorbis_vorbis_dsp,&vorbis_vorbis_block)>0) {
    vorbis_analysis(&vorbis_vorbis_block,&vorbis_ogg_packet);
    vorbis_bitrate_addblock(&vorbis_vorbis_block);
    while(vorbis_bitrate_flushpacket(&vorbis_vorbis_dsp,&vorbis_ogg_packet)) {
      ogg_stream_packetin(&vorbis_ogg_stream,&vorbis_ogg_packet);
      while(ogg_stream_pageout(&vorbis_ogg_stream,&vorbis_ogg_page)!=0) {
	conn->
	  writeData(frames,vorbis_ogg_page.header,vorbis_ogg_page.header_len);
	conn->writeData(frames,vorbis_ogg_page.body,vorbis_ogg_page.body_len);
      }
    }
  }
#endif  // HAVE_VORBIS
}
