/* Finite impulse response (FIR) resampler with adjustable FIR size */

/* Game_Music_Emu 0.5.2. http://www.slack.net/~ant/ */

/* Copyright (C) 2004-2006 Shay Green. This module is free software; you
can redistribute it and/or modify it under the terms of the GNU Lesser
General Public License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version. This
module is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
details. You should have received a copy of the GNU Lesser General Public
License along with this module; if not, write to the Free Software Foundation,
Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA */

/* C Conversion by Eke-Eke for use in Genesis Plus GX (2009). */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "Fir_Resampler.h"
#include "macros.h"

/* sound buffer */
static sample_t *buffer = NULL;
static int buffer_size = 0;

static sample_t impulses[MAX_RES][WIDTH];
static sample_t* write_pos = NULL;
static int res = 1;
static int imp_phase = 0;
static unsigned long skip_bits = 0;
static int step = STEREO;
static int input_per_cycle;
static double ratio = 1.0;

static void gen_sinc(double rolloff, int width, double offset, double spacing, double scale, int count, sample_t *out )
{
  double w, rolloff_cos_a, num, den, sinc;
  double const maxh = 256;
  double const fstep = M_PI / maxh * spacing;
  double const to_w = maxh * 2 / width;
  double const pow_a_n = pow( rolloff, maxh );
  double angle = (count / 2 - 1 + offset) * -fstep;
  scale /= maxh * 2;

  do
  {
    *out++ = 0;
    w = angle * to_w;
    if ( fabs( w ) < M_PI )
    {
      rolloff_cos_a = rolloff * cos( angle );
      num = 1 - rolloff_cos_a -
          pow_a_n * cos( maxh * angle ) +
          pow_a_n * rolloff * cos( (maxh - 1) * angle );
      den = 1 - rolloff_cos_a - rolloff_cos_a + rolloff * rolloff;
      sinc = scale * num / den - scale;

      out [-1] = (short) (cos( w ) * sinc + sinc);
    }
    angle += fstep;
  }
  while(--count);
}

/*static int available( long input_count )
{
  int cycle_count = input_count / input_per_cycle;
  int output_count = cycle_count * res * STEREO;
  input_count -= cycle_count * input_per_cycle;

  unsigned long skip = skip_bits >> imp_phase;
  int remain = res - imp_phase;
  while ( input_count >= 0 )
  {
    input_count -= step + (skip & 1) * STEREO;
    skip >>= 1;
    if ( !--remain )
    {
      skip = skip_bits;
      remain = res;
    }
    output_count += 2;
  }
  return output_count;
}
*/
int Fir_Resampler_avail()
{
  long count = 0;
  sample_t* in = buffer;
  sample_t* end_pos = write_pos;
  unsigned long skip = skip_bits >> imp_phase;
  int remain = res - imp_phase;
  if ( end_pos - in >= WIDTH * STEREO )
  {
    end_pos -= WIDTH * STEREO;
    do
    {
      count++;
      remain--;
      in += (skip * STEREO) & STEREO;
      skip >>= 1;
      in += step;

      if ( !remain )
      {
        skip = skip_bits;
        remain = res;
      }
    }
    while ( in <= end_pos );
  }

  return count;
}

int Fir_Resampler_initialize( int new_size )
{
  res       = 1;
  skip_bits = 0;
  imp_phase = 0;
  step      = STEREO;
  ratio     = 1.0;
  buffer = (sample_t *) realloc( buffer, (new_size + WRITE_OFFSET) * sizeof (sample_t) );
  write_pos = 0;
  if ( !buffer ) return 0;
  buffer_size = new_size + WRITE_OFFSET;
  Fir_Resampler_clear();
  return 1;
}

void Fir_Resampler_shutdown( void )
{
  if (buffer) free(buffer);
  buffer = 0;
  buffer_size = 0;
  write_pos = 0;
}

void Fir_Resampler_clear()
{
  imp_phase = 0;
  if ( buffer_size )
  {
    write_pos = &buffer [WRITE_OFFSET];
    memset( buffer, 0, buffer_size * sizeof (sample_t) );
  }
}

double Fir_Resampler_time_ratio( double new_factor, double rolloff )
{
  int i, r;
  double nearest, error, filter;
  double fstep = 0.0;
  double least_error = 2;
  double pos = 0.0;
  res = -1;

  for ( r = 1; r <= MAX_RES; r++ )
  {
    pos += new_factor;
    nearest = floor( pos + 0.5 );
    error = fabs( pos - nearest );
    if ( error < least_error )
    {
      res = r;
      fstep = nearest / res;
      least_error = error;
    }
  }

  skip_bits = 0;

  step = STEREO * (int) floor( fstep );

  ratio = fstep;
  fstep = fmod( fstep, 1.0 );

  filter = (ratio < 1.0) ? 1.0 : 1.0 / ratio;
  pos = 0.0;
  input_per_cycle = 0;

  memset(impulses, 0, MAX_RES*WIDTH*sizeof(sample_t));

  for ( i = 0; i < res; i++ )
  {
    gen_sinc( rolloff, (int) (WIDTH * filter + 1) & ~1, pos, filter,
              (double) (0x7FFF * GAIN * filter),
              (int) WIDTH, impulses[i] );

    pos += fstep;
    input_per_cycle += step;
    if ( pos >= 0.9999999 )
    {
      pos -= 1.0;
      skip_bits |= 1 << i;
      input_per_cycle++;
    }
  }

  Fir_Resampler_clear();

  return ratio;
}

/* Current ratio */
double Fir_Resampler_ratio( void )
{
  return ratio;
}

/* Number of input samples that can be written */
int Fir_Resampler_max_write( void )
{
  return buffer + buffer_size - write_pos;
}

/* Pointer to place to write input samples */
sample_t* Fir_Resampler_buffer( void )
{
  return write_pos;
}

/* Number of input samples in buffer */
int Fir_Resampler_written( void )
{
  return write_pos - &buffer [WRITE_OFFSET];
}

/* Number of output samples available */
/*int Fir_Resampler_avail( void )
{
  return available( write_pos - &buffer [WIDTH * STEREO] );
}*/

void Fir_Resampler_write( long count )
{
  write_pos += count;
}

int Fir_Resampler_read( sample_t* out, long count )
{
  sample_t* out_ = out;
  sample_t* in = buffer;
  sample_t* end_pos = write_pos;
  unsigned long skip = skip_bits >> imp_phase;
  sample_t const* imp = impulses [imp_phase];
  int remain = res - imp_phase;
  int n;
  int pt0,pt1;
  sample_t* i;
  long l,r;

  if ( end_pos - in >= WIDTH * STEREO )
  {
    end_pos -= WIDTH * STEREO;
    do
    {
      count--;
      
      if ( count < 0 )
        break;

      /* accumulate in extended precision */
      l = 0;
      r = 0;

      i = in;

      for ( n = WIDTH / 2; n; --n )
      {
        pt0 = imp [0];
        l += pt0 * i [0];
        r += pt0 * i [1];
        pt1 = imp [1];
        imp += 2;
        l += pt1 * i [2];
        r += pt1 * i [3];
        i += 4;
      }

      remain--;

      l >>= 15;
      r >>= 15;

      in += (skip * STEREO) & STEREO;
      skip >>= 1;
      in += step;

      if ( !remain )
      {
        imp = impulses [0];
        skip = skip_bits;
        remain = res;
      }

      *out++ = (sample_t) l;
      *out++ = (sample_t) r;
    }
    while ( in <= end_pos );
  }

  imp_phase = res - remain;

  n = write_pos - in;
  write_pos = &buffer [n];
  memmove( buffer, in, n * sizeof *in );

  return out - out_;
}

 /* fixed (Eke_Eke) */
int Fir_Resampler_input_needed( long output_count )
{
  long input_count = 0;
  unsigned long skip = skip_bits >> imp_phase;
  int remain = res - imp_phase;

  while ( (output_count) > 0 )
  {
    input_count += step + (skip & 1) * STEREO;
    skip >>= 1;
    if ( !--remain )
    {
      skip = skip_bits;
      remain = res;
    }
    output_count --;
  }

  input_count -= (write_pos - &buffer [WRITE_OFFSET]);
  if ( input_count < 0 )
    input_count = 0;
  return (input_count >> 1);
}

int Fir_Resampler_skip_input( long count )
{
  int remain = write_pos - buffer;
  int max_count = remain - WIDTH * STEREO;
  if ( count > max_count )
    count = max_count;

  remain -= count;
  write_pos = &buffer [remain];
  memmove( buffer, &buffer [count], remain * sizeof buffer [0] );

  return count;
}
