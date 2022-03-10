// filedevice.cpp
//
// Audio source for streaming direct from a file.
//
//   (C) Copyright 2014-2022 Fred Gleason <fredg@paravelsystems.com>
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

#include "filedevice.h"
#include "glasslimits.h"

FileDevice::FileDevice(unsigned chans,unsigned samprate,
		       Ringbuffer *ring,QObject *parent)
  : AudioDevice(chans,samprate,ring,parent)
{
#ifdef SNDFILE
  file_sndfile=NULL;
  file_muted=true;
  memset(&file_sfinfo,0,sizeof(file_sfinfo));

  for(int i=0;i<MAX_AUDIO_CHANNELS;i++) {
    file_meter_avg[i]=new MeterAverage(8);
  }

  file_read_timer=new QTimer(this);
  connect(file_read_timer,SIGNAL(timeout()),this,SLOT(readTimerData()));
#endif  // SNDFILE
}


FileDevice::~FileDevice()
{
#ifdef SNDFILE
  delete file_read_timer;
  for(int i=0;i<MAX_AUDIO_CHANNELS;i++) {
    delete file_meter_avg[i];
  }
  if(file_sndfile!=NULL) {
    sf_close(file_sndfile);
  }
#endif  // SNDFILE
}


bool FileDevice::processOptions(QString *err,const QStringList &keys,
				const QStringList &values)
{
#ifdef SNDFILE
  for(int i=0;i<keys.size();i++) {
    bool processed=false;
    if(keys[i]=="--file-name") {
      file_name=values[i];
      processed=true;
    }
    if(!processed) {
      *err=tr("unrecognized option")+" "+keys[i]+"\"";
      return false;
    }
  }
  if(file_name.isEmpty()) {
    char filename[256];

    if(scanf("%255s",filename)!=1) {
      *err=tr("error reading keyboard");
      return false;
    }
    file_name=filename;
  }
  return true;
#else
  *err=tr("device not supported");
  return false;
#endif
}


bool FileDevice::start(QString *err)
{
#ifdef SNDFILE
  if((file_sndfile=sf_open(file_name.toUtf8(),SFM_READ,&file_sfinfo))==NULL) {
    *err=sf_strerror(file_sndfile);
    return false;
  }

  if(file_sfinfo.channels>MAX_AUDIO_CHANNELS) {
    *err=tr("unsupported channel count");
    sf_close(file_sndfile);
    return false;
  }
  file_read_timer->start(1000*SNDFILE_BUFFER_SIZE/file_sfinfo.samplerate);

  return true;
#else
  return false;
#endif  // SNDFILE
}


unsigned FileDevice::deviceSamplerate() const
{
#ifdef SNDFILE
  return file_sfinfo.samplerate;
#else
  return DEFAULT_AUDIO_SAMPLERATE;
#endif  // SNDFILE
}


void FileDevice::unmute()
{
  file_muted=false;
}


void FileDevice::readTimerData()
{
#ifdef SNDFILE
  float pcm1[SNDFILE_BUFFER_SIZE*MAX_AUDIO_CHANNELS];
  float pcm2[SNDFILE_BUFFER_SIZE*MAX_AUDIO_CHANNELS];
  float *pcm=pcm1;
  sf_count_t nframes;
  float levels[MAX_AUDIO_CHANNELS];

  if(file_muted) {
    memset(pcm,0,sizeof(SNDFILE_BUFFER_SIZE*sizeof(float)*channels()));
    ringBuffer()->write(pcm,SNDFILE_BUFFER_SIZE);
  }
  else {
    if((nframes=sf_readf_float(file_sndfile,pcm1,SNDFILE_BUFFER_SIZE))>0) {
      if(file_sfinfo.channels!=(int)channels()) {
	remixChannels(pcm2,channels(),pcm1,file_sfinfo.channels,nframes);
	pcm=pcm2;
      }
      ringBuffer()->write(pcm,nframes);
      peakLevels(levels,pcm,nframes,channels());
      for(unsigned i=0;i<channels();i++) {
	file_meter_avg[i]->addValue(levels[i]);
	levels[i]=file_meter_avg[i]->average();
      }
      setMeterLevels(levels);
    }
    else {
      sf_close(file_sndfile);
      file_sndfile=NULL;
      emit hasStopped();
    }
  }
#endif  // SNDFILE
}
