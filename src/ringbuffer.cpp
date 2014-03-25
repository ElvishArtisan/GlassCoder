// ringbuffer.cpp
//
// A ringbuffer class for PCM audio
//
// (C) Copyright 2011 Fred Gleason <fredg@paravelsystems.com>
//
//    $Id: ringbuffer.cpp,v 1.1 2014/02/18 20:16:46 cvs Exp $
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Library General Public License 
//   version 2 as published by the Free Software Foundation.
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
//

#include <ringbuffer.h>

Ringbuffer::Ringbuffer(size_t bytes,unsigned channels)
{
  ring_channels=channels;
  ring_ring=jack_ringbuffer_create(bytes);
}


Ringbuffer::~Ringbuffer()
{
  jack_ringbuffer_free(ring_ring);
}


unsigned Ringbuffer::size() const
{
  return 0;
}


unsigned Ringbuffer::read(float *data,unsigned frames)
{
  return jack_ringbuffer_read(ring_ring,(char *)data,
			      frames*sizeof(float)*ring_channels)/
    (sizeof(float)*ring_channels);
}


unsigned Ringbuffer::readSpace() const
{
  return jack_ringbuffer_read_space(ring_ring)/(sizeof(float)*ring_channels);
}


unsigned Ringbuffer::write(float *data,unsigned frames)
{
  return jack_ringbuffer_write(ring_ring,(const char *)data,
			      frames*sizeof(float)*ring_channels)/
    (sizeof(float)*ring_channels);
}


unsigned Ringbuffer::writeSpace() const
{
  return jack_ringbuffer_write_space(ring_ring)/(sizeof(float)*ring_channels);
}


unsigned Ringbuffer::dump(unsigned frames)
{
  size_t bytes=frames*ring_channels*sizeof(float);
  unsigned ret=frames;


  if(jack_ringbuffer_read_space(ring_ring)<bytes) {
    bytes=jack_ringbuffer_read_space(ring_ring);
    ret=bytes/(ring_channels*sizeof(float));
  }
  jack_ringbuffer_read_advance(ring_ring,bytes);

  return ret;
}
