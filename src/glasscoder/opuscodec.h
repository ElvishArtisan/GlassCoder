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
#include <ogg/ogg.h>
#include <opus/opus.h>
#endif  // HAVE_OPUS

#include "codec.h"

#define OPUSCODEC_ENCODER_COMPLEXITY 0

class OpusCodec : public Codec
{
  Q_OBJECT;
 public:
  OpusCodec(Ringbuffer *ring,QObject *parent=0);
  QByteArray streamPrologue() const;
  bool isAvailable() const;
  QString contentType() const;
  unsigned pcmFrames() const;
  QString defaultExtension() const;
  QString formatIdentifier() const;
  bool startCodec();

 protected:
  void encodeData(Connector *conn,const float *pcm,int frames);

 private:
  QByteArray MakeInfoHeader(unsigned chans,unsigned samprate);
  QByteArray MakeCommentHeader();
#ifdef HAVE_OPUS
  void *opus_handle;
  void *opus_ogg_handle;
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

  ogg_stream_state opus_ogg_stream;
  ogg_page opus_ogg_page;
  ogg_packet opus_ogg_packet;
#endif  // HAVE_OPUS
  uint64_t opus_packet_number;
  uint64_t opus_packet_granulepos;
  QByteArray opus_stream_prologue;
  bool opus_prologue_sent;
};


#endif  // OPUSCODEC_H
