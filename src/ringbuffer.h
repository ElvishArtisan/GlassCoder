// ringbuffer.h
//
// A ringbuffer class for PCM audio
//
// (C) Copyright 2011 Fred Gleason <fredg@paravelsystems.com>
//
//    $Id: ringbuffer.h,v 1.1 2014/02/18 20:16:46 cvs Exp $
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


#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <sys/types.h>

#include <jack/ringbuffer.h>

class Ringbuffer
{
 public:
  Ringbuffer(size_t bytes,unsigned channels);
  ~Ringbuffer();
  unsigned size() const;
  unsigned read(float *data,unsigned frames);
  unsigned readSpace() const;
  unsigned write(float *data,unsigned frames);
  unsigned writeSpace() const;
  unsigned dump(unsigned frames);

 private:
  jack_ringbuffer_t *ring_ring;
  unsigned ring_channels;
};


#endif  // RINGBUFFER_H
