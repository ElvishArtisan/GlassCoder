// ringbuffer.h
//
// A ringbuffer class for PCM audio
//
// (C) Copyright 2011-2015 Fred Gleason <fredg@paravelsystems.com>
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
//  ISO/POSIX C version of Paul Davis's lock free ringbuffer C++ code.
//  This is safe for the case of one read thread and one write thread.
//
//   Copyright (C) 2000 Paul Davis
//   Copyright (C) 2003 Rohan Drape
//

#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct {
    char *buf;
    size_t len;
}
glass_ringbuffer_data_t ;

typedef struct {
    char	*buf;
    volatile size_t write_ptr;
    volatile size_t read_ptr;
    size_t	size;
    size_t	size_mask;
    int	mlocked;
}
glass_ringbuffer_t ;
glass_ringbuffer_t *glass_ringbuffer_create(int sz);
void glass_ringbuffer_free(glass_ringbuffer_t *rb);
void glass_ringbuffer_get_read_vector(const glass_ringbuffer_t *rb,
                                     glass_ringbuffer_data_t *vec);
void glass_ringbuffer_get_write_vector(const glass_ringbuffer_t *rb,
                                      glass_ringbuffer_data_t *vec);
size_t glass_ringbuffer_read(glass_ringbuffer_t *rb, char *dest, size_t cnt);
size_t glass_ringbuffer_peek(glass_ringbuffer_t *rb, char *dest, size_t cnt);
void glass_ringbuffer_read_advance(glass_ringbuffer_t *rb, size_t cnt);
size_t glass_ringbuffer_read_space(const glass_ringbuffer_t *rb);
int glass_ringbuffer_mlock(glass_ringbuffer_t *rb);
void glass_ringbuffer_reset(glass_ringbuffer_t *rb);
void glass_ringbuffer_reset_size (glass_ringbuffer_t * rb, size_t sz);
size_t glass_ringbuffer_write(glass_ringbuffer_t *rb, const char *src,
                             size_t cnt);
void glass_ringbuffer_write_advance(glass_ringbuffer_t *rb, size_t cnt);
size_t glass_ringbuffer_write_space(const glass_ringbuffer_t *rb);

#ifdef __cplusplus
}
#endif

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
  glass_ringbuffer_t *ring_ring;
  unsigned ring_channels;
};


#endif  // RINGBUFFER_H
