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

#ifndef FIR_RESAMPLER_H
#define FIR_RESAMPLER_H

#define STEREO        2
#define MAX_RES       32
#define WIDTH         16
#define WRITE_OFFSET  (WIDTH * STEREO) - STEREO
#define GAIN          1.0

typedef signed int sample_t;

extern int Fir_Resampler_initialize( int new_size );
extern void Fir_Resampler_shutdown( void );
extern void Fir_Resampler_clear( void );
extern double Fir_Resampler_time_ratio( double new_factor, double rolloff );
extern double Fir_Resampler_ratio( void );
extern int Fir_Resampler_max_write( void );
extern sample_t* Fir_Resampler_buffer( void );
extern int Fir_Resampler_written( void );
extern int Fir_Resampler_avail( void );
extern void Fir_Resampler_write( long count );
extern int Fir_Resampler_read( sample_t* out, long count );
extern int Fir_Resampler_input_needed( long output_count );
extern int Fir_Resampler_skip_input( long count );

#endif
