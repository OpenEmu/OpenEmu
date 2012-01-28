/* http://www.slack.net/~ant/ */

#include "blip.h"

#include <string.h>
#include <stdlib.h>
#include <stddef.h>

/* Copyright (C) 2003-2008 Shay Green. This module is free software; you
can redistribute it and/or modify it under the terms of the GNU Lesser
General Public License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version. This
module is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
details. You should have received a copy of the GNU Lesser General Public
License along with this module; if not, write to the Free Software Foundation,
Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA */

enum { buf_extra   = 2 };  /* extra samples to save past end */
enum { time_bits   = 16 }; /* bits in fraction of fixed-point sample counts */
enum { time_unit   = 1 << time_bits };
enum { phase_bits  = 15 }; /* bits in fraction of deltas in buffer */
enum { phase_count = 1 << phase_bits };
enum { phase_shift = time_bits - phase_bits };

typedef int buf_t; /* type of element in delta buffer */

struct blip_buffer_t
{
  int factor; /* clocks to samples conversion factor */
  int offset; /* fractional position of clock 0 in delta buffer */
  int amp;    /* current output amplitude (sum of all deltas up to now) */
  int size;   /* size of delta buffer */
  buf_t buf [65536]; /* delta buffer, only size elements actually allocated */
};

blip_buffer_t* blip_alloc( double clock_rate, double sample_rate, int size )
{
  /* Allocate space for structure and delta buffer */
  blip_buffer_t* s = (blip_buffer_t*) malloc(
      offsetof (blip_buffer_t, buf) + (size + buf_extra) * sizeof (buf_t) );
  if ( s != NULL )
  {
    /* Calculate output:input ratio and convert to fixed-point */
    double ratio = sample_rate / clock_rate;
    s->factor = (int) (ratio * time_unit + 0.5);
    
    s->size = size;
    blip_clear( s );
  }
  return s;
}

void blip_free( blip_buffer_t* s )
{
  free( s );
}

void blip_clear( blip_buffer_t* s )
{
  s->offset = 0;
  s->amp    = 0;
  memset( s->buf, 0, (s->size + buf_extra) * sizeof (buf_t) );
}

void blip_add( blip_buffer_t* s, int clocks, int delta )
{
  /* Convert to fixed-point time in terms of output samples */
  int fixed_time = clocks * s->factor + s->offset;
  
  /* Extract whole and fractional parts */
  int index = fixed_time >> time_bits; /* whole */
  int phase = fixed_time >> phase_shift & (phase_count - 1); /* fraction */
  
  /* Split delta between first and second samples */
  int second = delta * phase;
  int first  = delta * phase_count - second;
  
  /* Add deltas to buffer */
  s->buf [index  ] += first;
  s->buf [index+1] += second;
}

int blip_clocks_needed( const blip_buffer_t* s, int samples )
{
  int fixed_needed;
  if ( samples > s->size )
    samples = s->size;
  
  /* Fixed-point number of samples needed in addition to those in buffer */
  fixed_needed = samples * time_unit - s->offset;
  
  /* If more are needed, convert to clocks and round up */
  return (fixed_needed <= 0) ? 0 : (fixed_needed - 1) / s->factor + 1;
}

void blip_end_frame( blip_buffer_t* s, int clocks )
{
  s->offset += clocks * s->factor;
}

int blip_samples_avail( const blip_buffer_t* s )
{
  return s->offset >> time_bits;
}

/* Removes n samples from buffer */
static void remove_samples( blip_buffer_t* s, int n )
{
  int remain = blip_samples_avail( s ) + buf_extra - n;
  
  s->offset -= n * time_unit;
  
  /* Copy remaining samples to beginning of buffer and clear the rest */
  memmove( s->buf, &s->buf [n], remain * sizeof (buf_t) );
  memset( &s->buf [remain], 0, n * sizeof (buf_t) );
}

int blip_read_samples( blip_buffer_t* s, short out [], int count, int stereo )
{
  /* can't read more than available */
  int avail = blip_samples_avail( s );
  if ( count > avail )
    count = avail;
  
  if ( count )
  {
    /* Sum deltas and write out */
    int i;
    for ( i = 0; i < count; ++i )
    {
      int sample;
      
      /* Apply slight high-pass filter */
      s->amp -= s->amp >> 9;
      
      /* Add next delta */
      s->amp += s->buf [i];
      
      /* Calculate output sample */
      sample = s->amp >> phase_bits;
      
      /* Keep within 16-bit sample range */
      if ( sample < -32768 ) sample = -32768;
      if ( sample > +32767 ) sample = +32767;
      
      out [i << stereo] = sample;
    }
    
    remove_samples( s, count );
  }
  
  return count;
}
