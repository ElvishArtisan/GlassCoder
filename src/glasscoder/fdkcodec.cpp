// fdkcodec.cpp
//
// Codec class for MPEG-4 Advanced Audio Coding High Efficiency Profile
//
//   (C) Copyright 2014-2020 Fred Gleason <fredg@paravelsystems.com>
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

#include "fdkcodec.h"
#include "logging.h"

FdkCodec::FdkCodec(Ringbuffer *ring,QObject *parent)
  : Codec(Codec::TypeFdk,ring,parent)
{
#ifdef HAVE_FDKAAC
  fdk_input_buffer=NULL;
  fdk_output_buffer=NULL;
#endif  // HAVE_FDKAAC
}


FdkCodec::~FdkCodec()
{
#ifdef HAVE_FDKAAC
  if(fdk_input_buffer!=NULL) {
    delete fdk_input_buffer;
  }
  if(fdk_output_buffer!=NULL) {
    delete fdk_output_buffer;
  }
#endif  // HAVE_FDKAAC
}


bool FdkCodec::isAvailable() const
{
#ifdef HAVE_FDKAAC
  return (dlopen("libfdk-aac.so.2",RTLD_LAZY)!=NULL)||
    (dlopen("libfdk-aac.so.1",RTLD_LAZY)!=NULL);
#else
  return false;
#endif  // HAVE_FDKAAC
}


QString FdkCodec::contentType() const
{
  return "audio/aacp";
}


unsigned FdkCodec::pcmFrames() const
{
#ifdef HAVE_FDKAAC
  return fdk_info.frameLength;
#else
  return 0;
#endif  // HAVE_FDKAAC
}


QString FdkCodec::defaultExtension() const
{
  return QString("aac");
}


QString FdkCodec::formatIdentifier() const
{
  //
  // From ISO/IEC 14496-3
  // (see the summary at https://en.wikipedia.org/wiki/MPEG-4_Part_3)
  //
  return QString("mp4a.40.29");
}


bool FdkCodec::startCodec()
{
#ifdef HAVE_FDKAAC
  AACENC_ERROR err;

  //
  // Sanity Checks
  //
  if(bitrate()==0) {
    Log(LOG_ERR,"VBR encoding not supported");
    return false;
  }

  //
  // Load Library
  //
  if((fdk_handle=dlopen("libfdk-aac.so.2",RTLD_LAZY))==NULL) {
    fdk_handle=dlopen("libfdk-aac.so.1",RTLD_LAZY);
  }

  if(fdk_handle==NULL) {
    Log(LOG_ERR,"unsupported audio format (library not found)");
    return false;
  }
  *(void **)(&aacEncOpen)=dlsym(fdk_handle,"aacEncOpen");
  *(void **)(&aacEncClose)=dlsym(fdk_handle,"aacEncClose");
  *(void **)(&aacEncoder_GetParam)=dlsym(fdk_handle,"aacEncoder_GetParam");
  *(void **)(&aacEncoder_SetParam)=dlsym(fdk_handle,"aacEncoder_SetParam");
  *(void **)(&aacEncEncode)=dlsym(fdk_handle,"aacEncEncode");
  *(void **)(&aacEncInfo)=dlsym(fdk_handle,"aacEncInfo");
  *(void **)(&aacEncGetLibInfo)=dlsym(fdk_handle,"aacEncGetLibInfo");

  //
  // Initialize Encoder Instance
  //
  fdk_encoder=NULL;
  if((err=aacEncOpen(&fdk_encoder,0,0))!=AACENC_OK) {
    Log(LOG_ERR,
	QString().sprintf("unable to open encoder instance [0x%04X]",err));
    return false;
  }
  fdk_input_buffer=new INT_PCM[2048*channels()];
  fdk_input_ids[0]=IN_AUDIO_DATA;
  fdk_input_sizes[0]=2048*channels();
  fdk_inputel_sizes[0]=sizeof(INT_PCM);
  memset(&fdk_input_desc,0,sizeof(fdk_input_desc));
  fdk_input_desc.numBufs=1;
  fdk_input_desc.bufs=(void **)&fdk_input_buffer;
  fdk_input_desc.bufferIdentifiers=fdk_input_ids;
  fdk_input_desc.bufSizes=fdk_input_sizes;
  fdk_input_desc.bufElSizes=fdk_inputel_sizes;

  fdk_output_buffer=new UCHAR[6144*channels()];
  fdk_output_ids[0]=OUT_BITSTREAM_DATA;
  fdk_output_sizes[0]=6144*channels();
  fdk_outputel_sizes[0]=sizeof(UCHAR);
  memset(&fdk_output_desc,0,sizeof(fdk_output_desc));
  fdk_output_desc.numBufs=1;
  fdk_output_desc.bufs=(void **)&fdk_output_buffer;
  fdk_output_desc.bufferIdentifiers=fdk_output_ids;
  fdk_output_desc.bufSizes=fdk_output_sizes;
  fdk_output_desc.bufElSizes=fdk_outputel_sizes;

  switch(channels()) {
  case 1:
    aacEncoder_SetParam(fdk_encoder,AACENC_AOT,5);   // MPEG-4 HE-AAC
    aacEncoder_SetParam(fdk_encoder,AACENC_CHANNELMODE,MODE_1);
    break;

  case 2:
    if (bitrate() <= 48) {
      aacEncoder_SetParam(fdk_encoder,AACENC_AOT,29);  // MPEG-4 HE-AAC/PS
    }
    else {
      aacEncoder_SetParam(fdk_encoder,AACENC_AOT,2);   // AAC-LC
    }
    aacEncoder_SetParam(fdk_encoder,AACENC_CHANNELMODE,MODE_2);
    break;

  default:
    Log(LOG_ERR,"unsupported channel count");
    return false;
  }
  aacEncoder_SetParam(fdk_encoder,AACENC_BITRATE,1000*bitrate());
  aacEncoder_SetParam(fdk_encoder,AACENC_SAMPLERATE,streamSamplerate());
  aacEncoder_SetParam(fdk_encoder,AACENC_AFTERBURNER,1);
  aacEncoder_SetParam(fdk_encoder,AACENC_TRANSMUX,2);  // ADTS bitstream

  if((err=aacEncEncode(fdk_encoder,NULL,NULL,NULL,NULL))!=AACENC_OK) {
    Log(LOG_ERR,
	QString().sprintf("unable to initialize encoder instance [0x%04X]",err));
    return false;
  }
  aacEncInfo(fdk_encoder,&fdk_info);
  return true;
#else
  Log(LOG_ERR,"unsupported audio format (no build support)");
  return false;
#endif  // HAVE_FDKAAC
}


void FdkCodec::encodeData(Connector *conn,const float *pcm,int frames)
{
#ifdef HAVE_FDKAAC

  AACENC_InArgs inargs;
  AACENC_OutArgs outargs;
  AACENC_ERROR err;

  inargs.numInSamples=frames*channels();
  inargs.numAncBytes=0;

  src_float_to_short_array(pcm,fdk_input_buffer,frames*channels());
  if((err=aacEncEncode(fdk_encoder,&fdk_input_desc,&fdk_output_desc,&inargs,&outargs))== AACENC_OK) {
    conn->writeData(frames,fdk_output_buffer,outargs.numOutBytes);
  }
  else {
    Log(LOG_WARNING,QString().sprintf("fdk_aac encoding error %d",err));
  }
#endif  // HAVE_FDKAAC
}
