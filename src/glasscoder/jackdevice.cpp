// jackdevice.cpp
//
// Audio source for the Jack Audio Connection Kit
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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "glasslimits.h"
#include "jackdevice.h"
#include "logging.h"

//
// JACK Callback
//
#ifdef JACK
jack_default_audio_sample_t *jack_cb_buffers[MAX_AUDIO_CHANNELS];
float jack_cb_interleave_buffer[MAX_AUDIO_CHANNELS*RINGBUFFER_SIZE];

int JackProcess(jack_nframes_t nframes, void *arg)
{
  static unsigned i;
  static jack_nframes_t j;
  static JackDevice *obj=(JackDevice *)arg;
  static float lvls[MAX_AUDIO_CHANNELS];

  //
  // Get Buffers
  //
  for(i=0;i<obj->channels();i++) {
    jack_cb_buffers[i]=(jack_default_audio_sample_t *)
      jack_port_get_buffer(obj->jack_jack_ports[i],nframes);
  }

  //
  // Interleave Channels
  //
  for(i=0;i<obj->channels();i++) {
    if(jack_cb_buffers[i]!=NULL) {
      for(j=0;j<nframes;j++) {
	jack_cb_interleave_buffer[obj->channels()*j+i]=
	  (float)jack_cb_buffers[i][j]*obj->jack_gain;
      }
    }
  }

  //
  // Write It
  //
  obj->ringBuffer()->write(jack_cb_interleave_buffer,nframes);
  obj->peakLevels(lvls,jack_cb_interleave_buffer,nframes,obj->channels());
  for(i=0;i<obj->channels();i++) {
    obj->jack_meter_avg[i]->addValue(lvls[i]);
  }

  return 0;
}
#endif  // JACK


JackDevice::JackDevice(unsigned chans,unsigned samprate,
		       Ringbuffer *ring,QObject *parent)
  : AudioDevice(chans,samprate,ring,parent)
{
#ifdef JACK
  jack_server_name="";
  jack_client_name=DEFAULT_JACK_CLIENT_NAME;
  jack_gain=1.0;

  for(int i=0;i<MAX_AUDIO_CHANNELS;i++) {
    jack_meter_avg[i]=new MeterAverage(8);
  }
  jack_meter_timer=new QTimer(this);
  connect(jack_meter_timer,SIGNAL(timeout()),this,SLOT(meterData()));
#endif  // JACK
}


JackDevice::~JackDevice()
{
#ifdef JACK
  delete jack_meter_timer;
  for(int i=0;i<MAX_AUDIO_CHANNELS;i++) {
    delete jack_meter_avg[i];
  }
#endif  // JACK
}


bool JackDevice::processOptions(QString *err,const QStringList &keys,
				const QStringList &values)
{
#ifdef JACK
  bool ok=false;

  for(int i=0;i<keys.size();i++) {
    bool processed=false;
    if(keys.at(i)=="--jack-server-name") {
      jack_server_name=values[i];
      processed=true;
    }
    if(keys.at(i)=="--jack-client-name") {
      jack_client_name=values[i];
      processed=true;
    }
    if(keys.at(i)=="--jack-gain") {
      float db=values[i].toFloat(&ok);
      if(!ok) {
	*err=tr("invalid value for \"--jack-gain\"");
	return false;
      }
      if(db<=-100.0) {
	jack_gain=0.0;
      }
      else {
	jack_gain=powf(10.0,db/20.0);
      }
      processed=true;
    }
    if(!processed) {
      *err=tr("unrecognized option")+" "+keys[i]+"\"";
      return false;
    }
  }
  return true;
#else
  *err=tr("device not supported");
  return false;
#endif  // JACK
}



bool JackDevice::start(QString *err)
{
#ifdef JACK
  jack_options_t jackopts=JackNullOption;
  jack_status_t jackstat=JackFailure;

  //
  // Connect to JACK Instance
  //
  if(jack_server_name.isEmpty()) {
    jack_jack_client=
      jack_client_open(jack_client_name.toUtf8(),jackopts,&jackstat);
  }
  else {
    jack_jack_client=
      jack_client_open(jack_client_name.toUtf8(),jackopts,&jackstat,
		       (const char *)jack_server_name.toUtf8());
  }
  if(jack_jack_client==NULL) {
    if((jackstat&JackInvalidOption)!=0) {
      *err=tr("invalid or unsupported JACK option");
    }
    if((jackstat&JackServerError)!=0) {
      *err=tr("communication error with the JACK server");
    }
    if((jackstat&JackNoSuchClient)!=0) {
      *err=tr("requested JACK client does not exist");
    }
    if((jackstat&JackLoadFailure)!=0) {
      *err=tr("unable to load internal JACK client");
    }
    if((jackstat&JackInitFailure)!=0) {
      *err=tr("unable to initialize JACK client");
    }
    if((jackstat&JackShmFailure)!=0) {
      *err=tr("unable to access JACK shared memory");
    }
    if((jackstat&JackVersionError)!=0) {
      *err=tr("JACK protocol version mismatch");
    }
    if((jackstat&JackServerStarted)!=0) {
      *err=tr("JACK server started");
    }
    if((jackstat&JackServerFailed)!=0) {
      fprintf (stderr, "unable to communication with JACK server\n");
      *err=tr("unable to communicate with JACK server");
    }
    if((jackstat&JackNameNotUnique)!=0) {
      *err=tr("JACK client name not unique");
    }
    if((jackstat&JackFailure)!=0) {
      *err=tr("JACK general failure");
    }
    *err=tr("no connection to JACK server");
    return false;
  }
  jack_set_process_callback(jack_jack_client,JackProcess,this);

  //
  // Join the Graph
  //
  if(jack_activate(jack_jack_client)) {
    *err=tr("unable to join JACK graph");
    return false;
  }
  jack_jack_sample_rate=jack_get_sample_rate(jack_jack_client);

  //
  // Register Ports
  //
  for(unsigned i=0;i<channels();i++) {
    QString name=QString().sprintf("input_%d",i+1);
    jack_jack_ports[i]=
      jack_port_register(jack_jack_client,name.toUtf8(),JACK_DEFAULT_AUDIO_TYPE,
			 JackPortIsInput|JackPortIsTerminal,0);
  }
  Log(LOG_INFO,QString().sprintf("connected to JACK graph at %u samples/sec.",
				 jack_jack_sample_rate));

  jack_meter_timer->start(AUDIO_METER_INTERVAL);

  return true;

#endif  // JACK
  return false;
}


unsigned JackDevice::deviceSamplerate() const
{
#ifdef JACK
  return jack_jack_sample_rate;
#else
  return DEFAULT_AUDIO_SAMPLERATE;
#endif  // JACK
}


void JackDevice::meterData()
{
#ifdef JACK
  float lvls[MAX_AUDIO_CHANNELS];

  for(unsigned i=0;i<channels();i++) {
    lvls[i]=jack_meter_avg[i]->average();
  }
  setMeterLevels(lvls);

#endif  // JACK
}
