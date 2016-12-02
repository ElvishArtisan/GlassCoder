// vorbiscodec.h
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

#ifndef VORBISCODEC_H
#define VORBISCODEC_H

#ifdef HAVE_VORBIS
#include <ogg/ogg.h>
#include <vorbis/vorbisenc.h>
#endif  // HAVE_VORBIS

#include "codec.h"

class VorbisCodec : public Codec
{
  Q_OBJECT;
 public:
  VorbisCodec(Ringbuffer *ring,QObject *parent=0);
  ~VorbisCodec();
  bool isAvailable() const;
  QString contentType() const;
  unsigned pcmFrames() const;
  QString defaultExtension() const;
  QString formatIdentifier() const;
  QByteArray streamPrologue() const;
  bool startCodec();

 protected:
  void encodeData(Connector *conn,const float *pcm,int frames);

 private:
#ifdef HAVE_VORBIS
  void *vorbis_vorbisenc_handle;
  void *vorbis_vorbis_handle;
  void *vorbis_ogg_handle;

  int (*vorbis_encode_init)(vorbis_info *,long,long,long,long,long);
  int (*vorbis_encode_setup_managed)(vorbis_info *,long,long,long,long,long);
  int (*vorbis_encode_setup_vbr)(vorbis_info *,long,long,float);

  int (*vorbis_encode_init_vbr)(vorbis_info *,long,long,float);
  int (*vorbis_encode_setup_init)(vorbis_info *);
  int (*vorbis_encode_ctl)(vorbis_info *,int,void *);

  void (*vorbis_info_init)(vorbis_info *);
  void (*vorbis_info_clear)(vorbis_info *);
  int (*vorbis_info_blocksize)(vorbis_info,int);
  void (*vorbis_comment_init)(vorbis_comment *);
  void (*vorbis_comment_add)(vorbis_comment *, const char *);
  void (*vorbis_comment_add_tag)(vorbis_comment *,const char *,const char *);
  char (*vorbis_comment_query)(vorbis_comment *, const char *, int);
  int (*vorbis_comment_query_count)(vorbis_comment *, const char *);
  void (*vorbis_comment_clear)(vorbis_comment *);
  int (*vorbis_block_init)(vorbis_dsp_state *, vorbis_block *);
  int (*vorbis_block_clear)(vorbis_block *);
  void (*vorbis_dsp_clear)(vorbis_dsp_state *);
  double (*vorbis_granule_time)(vorbis_dsp_state *,ogg_int64_t);
  const char *(*vorbis_version_string)(void);
  int (*vorbis_analysis_init)(vorbis_dsp_state *,vorbis_info *);
  int (*vorbis_commentheader_out)(vorbis_comment *, ogg_packet *);
  int (*vorbis_analysis_headerout)(vorbis_dsp_state *,vorbis_comment *,
				   ogg_packet *,ogg_packet *,ogg_packet *);
  float **(*vorbis_analysis_buffer)(vorbis_dsp_state *,int);
  int (*vorbis_analysis_wrote)(vorbis_dsp_state *,int);
  int (*vorbis_analysis_blockout)(vorbis_dsp_state *,vorbis_block *);
  int (*vorbis_analysis)(vorbis_block *,ogg_packet *);
  int (*vorbis_bitrate_addblock)(vorbis_block *);
  int (*vorbis_bitrate_flushpacket)(vorbis_dsp_state *,ogg_packet *);
  int (*vorbis_synthesis_idheader)(ogg_packet *);
  int (*vorbis_synthesis_headerin)(vorbis_info *vi,vorbis_comment *,
				   ogg_packet *);
  int (*vorbis_synthesis_init)(vorbis_dsp_state *,vorbis_info *);
  int (*vorbis_synthesis_restart)(vorbis_dsp_state *);
  int (*vorbis_synthesis)(vorbis_block *,ogg_packet *);
  int (*vorbis_synthesis_trackonly)(vorbis_block *,ogg_packet *);
  int (*vorbis_synthesis_blockin)(vorbis_dsp_state *,vorbis_block *);
  int (*vorbis_synthesis_pcmout)(vorbis_dsp_state *,float ***);
  int (*vorbis_synthesis_lapout)(vorbis_dsp_state *,float ***);
  int (*vorbis_synthesis_read)(vorbis_dsp_state *,int);
  long (*vorbis_packet_blocksize)(vorbis_info *,ogg_packet *);
  int (*vorbis_synthesis_halfrate)(vorbis_info *,int);
  int (*vorbis_synthesis_halfrate_p)(vorbis_info *);

  void (*oggpack_writeinit)(oggpack_buffer *);
  int (*oggpack_writecheck)(oggpack_buffer *);
  void (*oggpack_writetrunc)(oggpack_buffer *,long);
  void (*oggpack_writealign)(oggpack_buffer *);
  void (*oggpack_writecopy)(oggpack_buffer *,void *,long);
  void (*oggpack_reset)(oggpack_buffer *);
  void (*oggpack_writeclear)(oggpack_buffer *);
  void (*oggpack_readinit)(oggpack_buffer *,unsigned char *,int);
  void (*oggpack_write)(oggpack_buffer *,unsigned long,int);
  long (*oggpack_look)(oggpack_buffer *,int);
  long (*oggpack_look1)(oggpack_buffer *);
  void (*oggpack_adv)(oggpack_buffer *,int);
  void (*oggpack_adv1)(oggpack_buffer *);
  long (*oggpack_read)(oggpack_buffer *,int);
  long (*oggpack_read1)(oggpack_buffer *);
  long (*oggpack_bytes)(oggpack_buffer *);
  long (*oggpack_bits)(oggpack_buffer *);
  unsigned char *(*oggpack_get_buffer)(oggpack_buffer *);
  void (*oggpackB_writeinit)(oggpack_buffer *);
  int (*oggpackB_writecheck)(oggpack_buffer *);
  void (*oggpackB_writetrunc)(oggpack_buffer *,long);
  void (*oggpackB_writealign)(oggpack_buffer *);
  void (*oggpackB_writecopy)(oggpack_buffer *,void *,long);
  void (*oggpackB_reset)(oggpack_buffer *);
  void (*oggpackB_writeclear)(oggpack_buffer *);
  void (*oggpackB_readinit)(oggpack_buffer *,unsigned char *,int);
  void (*oggpackB_write)(oggpack_buffer *,unsigned long,int);
  long (*oggpackB_look)(oggpack_buffer *,int);
  long (*oggpackB_look1)(oggpack_buffer *);
  void (*oggpackB_adv)(oggpack_buffer *,int);
  void (*oggpackB_adv1)(oggpack_buffer *);
  long (*oggpackB_read)(oggpack_buffer *,int);
  long (*oggpackB_read1)(oggpack_buffer *);
  long (*oggpackB_bytes)(oggpack_buffer *);
  long (*oggpackB_bits)(oggpack_buffer *);
  unsigned char *(*oggpackB_get_buffer)(oggpack_buffer *);
  int (*ogg_stream_packetin)(ogg_stream_state *, ogg_packet *);
  int (*ogg_stream_iovecin)(ogg_stream_state *, ogg_iovec_t *,
			    int,long, ogg_int64_t);
  int (*ogg_stream_pageout)(ogg_stream_state *, ogg_page *);
  int (*ogg_stream_flush)(ogg_stream_state *, ogg_page *);
  int (*ogg_sync_init)(ogg_sync_state *);
  int (*ogg_sync_clear)(ogg_sync_state *);
  int (*ogg_sync_reset)(ogg_sync_state *);
  int (*ogg_sync_destroy)(ogg_sync_state *);
  int (*ogg_sync_check)(ogg_sync_state *);
  char (*ogg_sync_buffer)(ogg_sync_state *, long);
  int (*ogg_sync_wrote)(ogg_sync_state *, long);
  long (*ogg_sync_pageseek)(ogg_sync_state *,ogg_page *);
  int (*ogg_sync_pageout)(ogg_sync_state *, ogg_page *);
  int (*ogg_stream_pagein)(ogg_stream_state *, ogg_page *);
  int (*ogg_stream_packetout)(ogg_stream_state *,ogg_packet *);
  int (*ogg_stream_packetpeek)(ogg_stream_state *,ogg_packet *);
  int (*ogg_stream_init)(ogg_stream_state *,int);
  int (*ogg_stream_clear)(ogg_stream_state *);
  int (*ogg_stream_reset)(ogg_stream_state *);
  int (*ogg_stream_reset_serialno)(ogg_stream_state *,int);
  int (*ogg_stream_destroy)(ogg_stream_state *);
  int (*ogg_stream_check)(ogg_stream_state *);
  int (*ogg_stream_eos)(ogg_stream_state *);
  void (*ogg_page_checksum_set)(ogg_page *);
  int (*ogg_page_version)(const ogg_page *);
  int (*ogg_page_continued)(const ogg_page *);
  int (*ogg_page_bos)(const ogg_page *);
  int (*ogg_page_eos)(const ogg_page *);
  ogg_int64_t (*ogg_page_granulepos)(const ogg_page *);
  int (*ogg_page_serialno)(const ogg_page *);
  long (*ogg_page_pageno)(const ogg_page *);
  int (*ogg_page_packets)(const ogg_page *);
  void (*ogg_packet_clear)(ogg_packet *);

  vorbis_info vorbis_vorbis_info;
  vorbis_dsp_state vorbis_vorbis_dsp;
  vorbis_block vorbis_vorbis_block;
  ogg_stream_state vorbis_ogg_stream;
  ogg_page vorbis_ogg_page;
  ogg_packet vorbis_ogg_packet;
#endif  // HAVE_VORBIS
  unsigned long vorbis_input_samples;
  unsigned long vorbis_buffer_size;
  unsigned char *vorbis_buffer;
  QByteArray vorbis_stream_prologue;
  bool vorbis_prologue_sent;
};


#endif  // VORBISCODEC_H
