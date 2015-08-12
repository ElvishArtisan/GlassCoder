// jack.cpp
//
// Jack Processing Routines for glasscoder(1)
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

#include <syslog.h>

#include "glasscoder.h"

//
// JACK Callback
//
jack_default_audio_sample_t *cb_buffers[MAX_AUDIO_CHANNELS];
float cb_interleave_buffer[MAX_AUDIO_CHANNELS*RINGBUFFER_SIZE];

int JackProcess(jack_nframes_t nframes, void *arg)
{
  static unsigned i;
  static jack_nframes_t j;
  MainObject *obj=(MainObject *)arg;

  //
  // Get Buffers
  //
  for(i=0;i<obj->audio_channels;i++) {
    cb_buffers[i]=(jack_default_audio_sample_t *)
      jack_port_get_buffer(obj->sir_jack_ports[i],nframes);
  }

  //
  // Interleave Channels
  //
  for(i=0;i<obj->audio_channels;i++) {
    if(cb_buffers[i]!=NULL) {
      for(j=0;j<nframes;j++) {
	cb_interleave_buffer[obj->audio_channels*j+i]=(float)cb_buffers[i][j];
      }
    }
  }

  //
  // Write It
  //
  for(i=0;i<obj->sir_ringbuffers.size();i++) {
    obj->sir_ringbuffers[i]->write(cb_interleave_buffer,nframes);
  }

  return 0;
}


bool MainObject::StartJack()
{
  jack_options_t jackopts=JackNullOption;
  jack_status_t jackstat=JackFailure;

  //
  // Create Ringbuffers
  //
  if(audio_bitrate.size()==0) {   // For VBR modes
    sir_ringbuffers.push_back(new Ringbuffer(RINGBUFFER_SIZE,audio_channels));
  }
  else {
    for(unsigned i=0;i<audio_bitrate.size();i++) {
      sir_ringbuffers.push_back(new Ringbuffer(RINGBUFFER_SIZE,audio_channels));
    }
  }

  //
  // Connect to JACK Instance
  //
  if(jack_server_name.isEmpty()) {
    sir_jack_client=
      jack_client_open(jack_client_name.toAscii(),jackopts,&jackstat);
  }
  else {
    sir_jack_client=
      jack_client_open(jack_client_name.toAscii(),jackopts,&jackstat,
		       (const char *)jack_server_name.toAscii());
  }
  if(sir_jack_client==NULL) {
    if((jackstat&JackInvalidOption)!=0) {
      syslog(LOG_ERR,"invalid or unsupported JACK option");
    }
    if((jackstat&JackServerError)!=0) {
      syslog(LOG_ERR,"communication error with the JACK server");
    }
    if((jackstat&JackNoSuchClient)!=0) {
      syslog(LOG_ERR,"requested JACK client does not exist");
    }
    if((jackstat&JackLoadFailure)!=0) {
      syslog(LOG_ERR,"unable to load internal JACK client");
    }
    if((jackstat&JackInitFailure)!=0) {
      syslog(LOG_ERR,"unable to initialize JACK client");
    }
    if((jackstat&JackShmFailure)!=0) {
      syslog(LOG_ERR,"unable to access JACK shared memory");
    }
    if((jackstat&JackVersionError)!=0) {
      syslog(LOG_ERR,"JACK protocol version mismatch");
    }
    if((jackstat&JackServerStarted)!=0) {
      syslog(LOG_ERR,"JACK server started");
    }
    if((jackstat&JackServerFailed)!=0) {
      fprintf (stderr, "unable to communication with JACK server\n");
      syslog(LOG_ERR,"unable to communicate with JACK server");
    }
    if((jackstat&JackNameNotUnique)!=0) {
      syslog(LOG_ERR,"JACK client name not unique");
    }
    if((jackstat&JackFailure)!=0) {
      syslog(LOG_ERR,"JACK general failure");
    }
    syslog(LOG_ERR,"no connection to JACK server");
    return false;
  }
  jack_set_process_callback(sir_jack_client,JackProcess,this);

  //
  // Join the Graph
  //
  if(jack_activate(sir_jack_client)) {
    syslog(LOG_ERR,"unable to join JACK graph");
    return false;
  }
  sir_jack_sample_rate=jack_get_sample_rate(sir_jack_client);

  //
  // Register Ports
  //
  for(unsigned i=0;i<audio_channels;i++) {
    QString name=QString().sprintf("input_%d",i+1);
    sir_jack_ports[i]=
      jack_port_register(sir_jack_client,name.toAscii(),JACK_DEFAULT_AUDIO_TYPE,
			 JackPortIsInput|JackPortIsTerminal,0);
  }
  syslog(LOG_DEBUG,"connected to JACK, sample rate = %u",sir_jack_sample_rate);

  return true;
}
