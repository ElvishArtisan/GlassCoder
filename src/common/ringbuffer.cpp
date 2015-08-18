// ringbuffer.cpp
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

#include <ringbuffer.h>

#include <stdlib.h>
#include <string.h>
#ifdef USE_MLOCK
#include <sys/mman.h>
#endif /* USE_MLOCK */

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

/* Create a new ringbuffer to hold at least `sz' bytes of data. The
   actual buffer size is rounded up to the next power of two.  */

glass_ringbuffer_t *
glass_ringbuffer_create (int sz)
{
  	int power_of_two;
	glass_ringbuffer_t *rb;

	if ((rb = (glass_ringbuffer_t *) malloc (sizeof (glass_ringbuffer_t))) == NULL) {
		return NULL;
	}

	for (power_of_two = 1; 1 << power_of_two < sz; power_of_two++);

	rb->size = 1 << power_of_two;
	rb->size_mask = rb->size;
	rb->size_mask -= 1;
	rb->write_ptr = 0;
	rb->read_ptr = 0;
	if ((rb->buf = (char *) malloc (rb->size)) == NULL) {
		free (rb);
		return NULL;
	}
	rb->mlocked = 0;

	return rb;
}

/* Free all data associated with the ringbuffer `rb'. */

void
glass_ringbuffer_free (glass_ringbuffer_t * rb)
{
#ifdef USE_MLOCK
	if (rb->mlocked) {
		munlock (rb->buf, rb->size);
	}
#endif /* USE_MLOCK */
	free (rb->buf);
	free (rb);
}

/* Lock the data block of `rb' using the system call 'mlock'.  */

int
glass_ringbuffer_mlock (glass_ringbuffer_t * rb)
{
#ifdef USE_MLOCK
	if (mlock (rb->buf, rb->size)) {
		return -1;
	}
#endif /* USE_MLOCK */
	rb->mlocked = 1;
	return 0;
}

/* Reset the read and write pointers to zero. This is not thread
   safe. */

void
glass_ringbuffer_reset (glass_ringbuffer_t * rb)
{
	rb->read_ptr = 0;
	rb->write_ptr = 0;
    memset(rb->buf, 0, rb->size);
}

/* Reset the read and write pointers to zero. This is not thread
   safe. */

void
glass_ringbuffer_reset_size (glass_ringbuffer_t * rb, size_t sz)
{
    rb->size = sz;
    rb->size_mask = rb->size;
    rb->size_mask -= 1;
    rb->read_ptr = 0;
    rb->write_ptr = 0;
}

/* Return the number of bytes available for reading.  This is the
   number of bytes in front of the read pointer and behind the write
   pointer.  */

size_t
glass_ringbuffer_read_space (const glass_ringbuffer_t * rb)
{
	size_t w, r;

	w = rb->write_ptr;
	r = rb->read_ptr;

	if (w > r) {
		return w - r;
	} else {
		return (w - r + rb->size) & rb->size_mask;
	}
}

/* Return the number of bytes available for writing.  This is the
   number of bytes in front of the write pointer and behind the read
   pointer.  */

size_t
glass_ringbuffer_write_space (const glass_ringbuffer_t * rb)
{
	size_t w, r;

	w = rb->write_ptr;
	r = rb->read_ptr;

	if (w > r) {
		return ((r - w + rb->size) & rb->size_mask) - 1;
	} else if (w < r) {
		return (r - w) - 1;
	} else {
		return rb->size - 1;
	}
}

/* The copying data reader.  Copy at most `cnt' bytes from `rb' to
   `dest'.  Returns the actual number of bytes copied. */

size_t
glass_ringbuffer_read (glass_ringbuffer_t * rb, char *dest, size_t cnt)
{
	size_t free_cnt;
	size_t cnt2;
	size_t to_read;
	size_t n1, n2;

	if ((free_cnt = glass_ringbuffer_read_space (rb)) == 0) {
		return 0;
	}

	to_read = cnt > free_cnt ? free_cnt : cnt;

	cnt2 = rb->read_ptr + to_read;

	if (cnt2 > rb->size) {
		n1 = rb->size - rb->read_ptr;
		n2 = cnt2 & rb->size_mask;
	} else {
		n1 = to_read;
		n2 = 0;
	}

	memcpy (dest, &(rb->buf[rb->read_ptr]), n1);
	rb->read_ptr = (rb->read_ptr + n1) & rb->size_mask;

	if (n2) {
		memcpy (dest + n1, &(rb->buf[rb->read_ptr]), n2);
		rb->read_ptr = (rb->read_ptr + n2) & rb->size_mask;
	}

	return to_read;
}

/* The copying data reader w/o read pointer advance.  Copy at most
   `cnt' bytes from `rb' to `dest'.  Returns the actual number of bytes
   copied. */

size_t
glass_ringbuffer_peek (glass_ringbuffer_t * rb, char *dest, size_t cnt)
{
	size_t free_cnt;
	size_t cnt2;
	size_t to_read;
	size_t n1, n2;
	size_t tmp_read_ptr;

	tmp_read_ptr = rb->read_ptr;

	if ((free_cnt = glass_ringbuffer_read_space (rb)) == 0) {
		return 0;
	}

	to_read = cnt > free_cnt ? free_cnt : cnt;

	cnt2 = tmp_read_ptr + to_read;

	if (cnt2 > rb->size) {
		n1 = rb->size - tmp_read_ptr;
		n2 = cnt2 & rb->size_mask;
	} else {
		n1 = to_read;
		n2 = 0;
	}

	memcpy (dest, &(rb->buf[tmp_read_ptr]), n1);
	tmp_read_ptr = (tmp_read_ptr + n1) & rb->size_mask;

	if (n2) {
		memcpy (dest + n1, &(rb->buf[tmp_read_ptr]), n2);
	}

	return to_read;
}

/* The copying data writer.  Copy at most `cnt' bytes to `rb' from
   `src'.  Returns the actual number of bytes copied. */

size_t
glass_ringbuffer_write (glass_ringbuffer_t * rb, const char *src, size_t cnt)
{
	size_t free_cnt;
	size_t cnt2;
	size_t to_write;
	size_t n1, n2;

	if ((free_cnt = glass_ringbuffer_write_space (rb)) == 0) {
		return 0;
	}

	to_write = cnt > free_cnt ? free_cnt : cnt;

	cnt2 = rb->write_ptr + to_write;

	if (cnt2 > rb->size) {
		n1 = rb->size - rb->write_ptr;
		n2 = cnt2 & rb->size_mask;
	} else {
		n1 = to_write;
		n2 = 0;
	}

	memcpy (&(rb->buf[rb->write_ptr]), src, n1);
	rb->write_ptr = (rb->write_ptr + n1) & rb->size_mask;

	if (n2) {
		memcpy (&(rb->buf[rb->write_ptr]), src + n1, n2);
		rb->write_ptr = (rb->write_ptr + n2) & rb->size_mask;
	}

	return to_write;
}

/* Advance the read pointer `cnt' places. */

void
glass_ringbuffer_read_advance (glass_ringbuffer_t * rb, size_t cnt)
{
	size_t tmp = (rb->read_ptr + cnt) & rb->size_mask;
	rb->read_ptr = tmp;
}

/* Advance the write pointer `cnt' places. */

void
glass_ringbuffer_write_advance (glass_ringbuffer_t * rb, size_t cnt)
{
	size_t tmp = (rb->write_ptr + cnt) & rb->size_mask;
	rb->write_ptr = tmp;
}

/* The non-copying data reader.  `vec' is an array of two places.  Set
   the values at `vec' to hold the current readable data at `rb'.  If
   the readable data is in one segment the second segment has zero
   length.  */

void
glass_ringbuffer_get_read_vector (const glass_ringbuffer_t * rb,
				 glass_ringbuffer_data_t * vec)
{
	size_t free_cnt;
	size_t cnt2;
	size_t w, r;

	w = rb->write_ptr;
	r = rb->read_ptr;

	if (w > r) {
		free_cnt = w - r;
	} else {
		free_cnt = (w - r + rb->size) & rb->size_mask;
	}

	cnt2 = r + free_cnt;

	if (cnt2 > rb->size) {

		/* Two part vector: the rest of the buffer after the current write
		   ptr, plus some from the start of the buffer. */

		vec[0].buf = &(rb->buf[r]);
		vec[0].len = rb->size - r;
		vec[1].buf = rb->buf;
		vec[1].len = cnt2 & rb->size_mask;

	} else {

		/* Single part vector: just the rest of the buffer */

		vec[0].buf = &(rb->buf[r]);
		vec[0].len = free_cnt;
		vec[1].len = 0;
	}
}

/* The non-copying data writer.  `vec' is an array of two places.  Set
   the values at `vec' to hold the current writeable data at `rb'.  If
   the writeable data is in one segment the second segment has zero
   length.  */

void
glass_ringbuffer_get_write_vector (const glass_ringbuffer_t * rb,
				  glass_ringbuffer_data_t * vec)
{
	size_t free_cnt;
	size_t cnt2;
	size_t w, r;

	w = rb->write_ptr;
	r = rb->read_ptr;

	if (w > r) {
		free_cnt = ((r - w + rb->size) & rb->size_mask) - 1;
	} else if (w < r) {
		free_cnt = (r - w) - 1;
	} else {
		free_cnt = rb->size - 1;
	}

	cnt2 = w + free_cnt;

	if (cnt2 > rb->size) {

		/* Two part vector: the rest of the buffer after the current write
		   ptr, plus some from the start of the buffer. */

		vec[0].buf = &(rb->buf[w]);
		vec[0].len = rb->size - w;
		vec[1].buf = rb->buf;
		vec[1].len = cnt2 & rb->size_mask;
	} else {
		vec[0].buf = &(rb->buf[w]);
		vec[0].len = free_cnt;
		vec[1].len = 0;
	}
}


Ringbuffer::Ringbuffer(size_t bytes,unsigned channels)
{
  ring_channels=channels;
  ring_ring=glass_ringbuffer_create(bytes);
}


Ringbuffer::~Ringbuffer()
{
  glass_ringbuffer_free(ring_ring);
}


unsigned Ringbuffer::size() const
{
  return 0;
}


unsigned Ringbuffer::read(float *data,unsigned frames)
{
  return glass_ringbuffer_read(ring_ring,(char *)data,
			      frames*sizeof(float)*ring_channels)/
    (sizeof(float)*ring_channels);
}


unsigned Ringbuffer::readSpace() const
{
  return glass_ringbuffer_read_space(ring_ring)/(sizeof(float)*ring_channels);
}


unsigned Ringbuffer::write(float *data,unsigned frames)
{
  return glass_ringbuffer_write(ring_ring,(const char *)data,
			      frames*sizeof(float)*ring_channels)/
    (sizeof(float)*ring_channels);
}


unsigned Ringbuffer::writeSpace() const
{
  return glass_ringbuffer_write_space(ring_ring)/(sizeof(float)*ring_channels);
}


unsigned Ringbuffer::dump(unsigned frames)
{
  size_t bytes=frames*ring_channels*sizeof(float);
  unsigned ret=frames;


  if(glass_ringbuffer_read_space(ring_ring)<bytes) {
    bytes=glass_ringbuffer_read_space(ring_ring);
    ret=bytes/(ring_channels*sizeof(float));
  }
  glass_ringbuffer_read_advance(ring_ring,bytes);

  return ret;
}
