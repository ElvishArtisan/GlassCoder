// fdkcodec.h
//
// Codec class for MPEG-4 Advanced Audio Coding High Efficiency Profile v2
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

#ifndef FDKCODEC_H
#define FDKCODEC_H

#ifdef HAVE_FDKAAC
#include <fdk-aac/aacenc_lib.h>
#endif  // HAVE_FDKAAC

#include "codec.h"

class FdkCodec : public Codec
{
  Q_OBJECT;
 public:
  FdkCodec(Ringbuffer *ring,QObject *parent=0);
  ~FdkCodec();
  bool isAvailable() const;
  QString contentType() const;
  unsigned pcmFrames() const;
  QString defaultExtension() const;
  QString formatIdentifier() const;
  bool startCodec();

 protected:
  void encodeData(Connector *conn,const float *pcm,int frames);

 private:
#ifdef HAVE_FDKAAC
  void *fdk_handle;
  HANDLE_AACENCODER fdk_encoder;
  AACENC_ERROR (*aacEncOpen)(HANDLE_AACENCODER *,const UINT,const UINT);
  AACENC_ERROR (*aacEncClose)(HANDLE_AACENCODER *);
  UINT (*aacEncoder_GetParam)(const HANDLE_AACENCODER,const AACENC_PARAM);
  AACENC_ERROR (*aacEncoder_SetParam)(const HANDLE_AACENCODER,
				      const AACENC_PARAM,const UINT);
  AACENC_ERROR (*aacEncEncode)(const HANDLE_AACENCODER,
			       const AACENC_BufDesc *,const AACENC_BufDesc *,
			       const AACENC_InArgs *,AACENC_OutArgs *);
  AACENC_ERROR (*aacEncInfo)(const HANDLE_AACENCODER,AACENC_InfoStruct *);
  AACENC_ERROR (*aacEncGetLibInfo)(LIB_INFO *);
  unsigned long fdk_input_samples;
  AACENC_BufDesc fdk_input_desc;
  INT_PCM *fdk_input_buffer;
  INT fdk_input_ids[1];
  INT fdk_input_sizes[1];
  INT fdk_inputel_sizes[1];
  AACENC_BufDesc fdk_output_desc;
  UCHAR *fdk_output_buffer;
  INT fdk_output_ids[1];
  INT fdk_output_sizes[1];
  INT fdk_outputel_sizes[1];
  AACENC_InfoStruct fdk_info;
#endif  // HAVE_FDKAAC
};


#endif  // FDKCODEC_H
