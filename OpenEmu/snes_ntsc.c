/* snes_ntsc 0.2.2. http://www.slack.net/~ant/ */

#include "snes_ntsc.h"

/* Copyright (C) 2006-2007 Shay Green. This module is free software; you
can redistribute it and/or modify it under the terms of the GNU Lesser
General Public License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version. This
module is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
details. You should have received a copy of the GNU Lesser General Public
License along with this module; if not, write to the Free Software Foundation,
Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA */

snes_ntsc_setup_t const snes_ntsc_monochrome = { 0,-1, 0, 0,.2,  0,.2,-.2,-.2,-1,  1, 0, 0 };
snes_ntsc_setup_t const snes_ntsc_composite  = { 0, 0, 0, 0, 0,  0, 0,  0,  0, 0,  1, 0, 0 };
snes_ntsc_setup_t const snes_ntsc_svideo     = { 0, 0, 0, 0,.2,  0,.2, -1, -1, 0,  1, 0, 0 };
snes_ntsc_setup_t const snes_ntsc_rgb        = { 0, 0, 0, 0,.2,  0,.7, -1, -1,-1,  1, 0, 0 };

#define alignment_count 3
#define burst_count     3
#define rescale_in      8
#define rescale_out     7

#define artifacts_mid   1.0f
#define fringing_mid    1.0f
#define std_decoder_hue 0

#define rgb_bits        7 /* half normal range to allow for doubled hires pixels */
#define gamma_size      32

#include "snes_ntsc_impl.h"

/* 3 input pixels -> 8 composite samples */
pixel_info_t const snes_ntsc_pixels [alignment_count] = {
	{ PIXEL_OFFSET( -4, -9 ), { 1, 1, .6667f, 0 } },
	{ PIXEL_OFFSET( -2, -7 ), {       .3333f, 1, 1, .3333f } },
	{ PIXEL_OFFSET(  0, -5 ), {                  0, .6667f, 1, 1 } },
};

static void merge_kernel_fields( snes_ntsc_rgb_t* io )
{
	int n;
	for ( n = burst_size; n; --n )
	{
		snes_ntsc_rgb_t p0 = io [burst_size * 0] + rgb_bias;
		snes_ntsc_rgb_t p1 = io [burst_size * 1] + rgb_bias;
		snes_ntsc_rgb_t p2 = io [burst_size * 2] + rgb_bias;
		/* merge colors without losing precision */
		io [burst_size * 0] =
				((p0 + p1 - ((p0 ^ p1) & snes_ntsc_rgb_builder)) >> 1) - rgb_bias;
		io [burst_size * 1] =
				((p1 + p2 - ((p1 ^ p2) & snes_ntsc_rgb_builder)) >> 1) - rgb_bias;
		io [burst_size * 2] =
				((p2 + p0 - ((p2 ^ p0) & snes_ntsc_rgb_builder)) >> 1) - rgb_bias;
		++io;
	}
}

static void correct_errors( snes_ntsc_rgb_t color, snes_ntsc_rgb_t* out )
{
	int n;
	for ( n = burst_count; n; --n )
	{
		unsigned i;
		for ( i = 0; i < rgb_kernel_size / 2; i++ )
		{
			snes_ntsc_rgb_t error = color -
					out [i    ] - out [(i+12)%14+14] - out [(i+10)%14+28] -
					out [i + 7] - out [i + 5    +14] - out [i + 3    +28];
			DISTRIBUTE_ERROR( i+3+28, i+5+14, i+7 );
		}
		out += alignment_count * rgb_kernel_size;
	}
}

void snes_ntsc_init( snes_ntsc_t* ntsc, snes_ntsc_setup_t const* setup )
{
	int merge_fields;
	int entry;
	init_t impl;
	if ( !setup )
		setup = &snes_ntsc_composite;
	init( &impl, setup );
	
	merge_fields = setup->merge_fields;
	if ( setup->artifacts <= -1 && setup->fringing <= -1 )
		merge_fields = 1;
	
	for ( entry = 0; entry < snes_ntsc_palette_size; entry++ )
	{
		/* Reduce number of significant bits of source color. Clearing the
		low bits of R and B were least notictable. Modifying green was too
		noticeable. */
		int ir = entry >> 8 & 0x1E;
		int ig = entry >> 4 & 0x1F;
		int ib = entry << 1 & 0x1E;
		
		#if SNES_NTSC_BSNES_COLORTBL
			if ( setup->bsnes_colortbl )
			{
				int bgr15 = (ib << 10) | (ig << 5) | ir;
				unsigned long rgb16 = setup->bsnes_colortbl [bgr15];
				ir = rgb16 >> 11 & 0x1E;
				ig = rgb16 >>  6 & 0x1F;
				ib = rgb16       & 0x1E;
			}
		#endif
		
		{
			float rr = impl.to_float [ir];
			float gg = impl.to_float [ig];
			float bb = impl.to_float [ib];
			
			float y, i, q = RGB_TO_YIQ( rr, gg, bb, y, i );
			
			int r, g, b = YIQ_TO_RGB( y, i, q, impl.to_rgb, int, r, g );
			snes_ntsc_rgb_t rgb = PACK_RGB( r, g, b );
			
			snes_ntsc_rgb_t* out = ntsc->table [entry];
			gen_kernel( &impl, y, i, q, out );
			if ( merge_fields )
				merge_kernel_fields( out );
			correct_errors( rgb, out );
		}
	}
}

#ifndef SNES_NTSC_NO_BLITTERS

void snes_ntsc_blit( snes_ntsc_t const* ntsc, SNES_NTSC_IN_T const* input, long in_row_width,
		int burst_phase, int in_width, int in_height, void* rgb_out, long out_pitch )
{
	int chunk_count = (in_width - 1) / snes_ntsc_in_chunk;
	for ( ; in_height; --in_height )
	{
		SNES_NTSC_IN_T const* line_in = input;
		SNES_NTSC_BEGIN_ROW( ntsc, burst_phase,
				snes_ntsc_black, snes_ntsc_black, SNES_NTSC_ADJ_IN( *line_in ) );
		snes_ntsc_out_t* restrict line_out = (snes_ntsc_out_t*) rgb_out;
		int n;
		++line_in;
		
		for ( n = chunk_count; n; --n )
		{
			/* order of input and output pixels must not be altered */
			SNES_NTSC_COLOR_IN( 0, SNES_NTSC_ADJ_IN( line_in [0] ) );
			SNES_NTSC_RGB_OUT( 0, line_out [0], SNES_NTSC_OUT_DEPTH );
			SNES_NTSC_RGB_OUT( 1, line_out [1], SNES_NTSC_OUT_DEPTH );
			
			SNES_NTSC_COLOR_IN( 1, SNES_NTSC_ADJ_IN( line_in [1] ) );
			SNES_NTSC_RGB_OUT( 2, line_out [2], SNES_NTSC_OUT_DEPTH );
			SNES_NTSC_RGB_OUT( 3, line_out [3], SNES_NTSC_OUT_DEPTH );
			
			SNES_NTSC_COLOR_IN( 2, SNES_NTSC_ADJ_IN( line_in [2] ) );
			SNES_NTSC_RGB_OUT( 4, line_out [4], SNES_NTSC_OUT_DEPTH );
			SNES_NTSC_RGB_OUT( 5, line_out [5], SNES_NTSC_OUT_DEPTH );
			SNES_NTSC_RGB_OUT( 6, line_out [6], SNES_NTSC_OUT_DEPTH );
			
			line_in  += 3;
			line_out += 7;
		}
		
		/* finish final pixels */
		SNES_NTSC_COLOR_IN( 0, snes_ntsc_black );
		SNES_NTSC_RGB_OUT( 0, line_out [0], SNES_NTSC_OUT_DEPTH );
		SNES_NTSC_RGB_OUT( 1, line_out [1], SNES_NTSC_OUT_DEPTH );
		
		SNES_NTSC_COLOR_IN( 1, snes_ntsc_black );
		SNES_NTSC_RGB_OUT( 2, line_out [2], SNES_NTSC_OUT_DEPTH );
		SNES_NTSC_RGB_OUT( 3, line_out [3], SNES_NTSC_OUT_DEPTH );
		
		SNES_NTSC_COLOR_IN( 2, snes_ntsc_black );
		SNES_NTSC_RGB_OUT( 4, line_out [4], SNES_NTSC_OUT_DEPTH );
		SNES_NTSC_RGB_OUT( 5, line_out [5], SNES_NTSC_OUT_DEPTH );
		SNES_NTSC_RGB_OUT( 6, line_out [6], SNES_NTSC_OUT_DEPTH );
		
		burst_phase = (burst_phase + 1) % snes_ntsc_burst_count;
		input += in_row_width;
		rgb_out = (char*) rgb_out + out_pitch;
	}
}

void snes_ntsc_blit_hires( snes_ntsc_t const* ntsc, SNES_NTSC_IN_T const* input, long in_row_width,
		int burst_phase, int in_width, int in_height, void* rgb_out, long out_pitch )
{
	int chunk_count = (in_width - 2) / (snes_ntsc_in_chunk * 2);
	for ( ; in_height; --in_height )
	{
		SNES_NTSC_IN_T const* line_in = input;
		SNES_NTSC_HIRES_ROW( ntsc, burst_phase,
				snes_ntsc_black, snes_ntsc_black, snes_ntsc_black,
				SNES_NTSC_ADJ_IN( line_in [0] ),
				SNES_NTSC_ADJ_IN( line_in [1] ) );
		snes_ntsc_out_t* restrict line_out = (snes_ntsc_out_t*) rgb_out;
		int n;
		line_in += 2;
		
		for ( n = chunk_count; n; --n )
		{
			/* twice as many input pixels per chunk */
			SNES_NTSC_COLOR_IN( 0, SNES_NTSC_ADJ_IN( line_in [0] ) );
			SNES_NTSC_HIRES_OUT( 0, line_out [0], SNES_NTSC_OUT_DEPTH );
			
			SNES_NTSC_COLOR_IN( 1, SNES_NTSC_ADJ_IN( line_in [1] ) );
			SNES_NTSC_HIRES_OUT( 1, line_out [1], SNES_NTSC_OUT_DEPTH );
			
			SNES_NTSC_COLOR_IN( 2, SNES_NTSC_ADJ_IN( line_in [2] ) );
			SNES_NTSC_HIRES_OUT( 2, line_out [2], SNES_NTSC_OUT_DEPTH );
			
			SNES_NTSC_COLOR_IN( 3, SNES_NTSC_ADJ_IN( line_in [3] ) );
			SNES_NTSC_HIRES_OUT( 3, line_out [3], SNES_NTSC_OUT_DEPTH );
			
			SNES_NTSC_COLOR_IN( 4, SNES_NTSC_ADJ_IN( line_in [4] ) );
			SNES_NTSC_HIRES_OUT( 4, line_out [4], SNES_NTSC_OUT_DEPTH );
			
			SNES_NTSC_COLOR_IN( 5, SNES_NTSC_ADJ_IN( line_in [5] ) );
			SNES_NTSC_HIRES_OUT( 5, line_out [5], SNES_NTSC_OUT_DEPTH );
			SNES_NTSC_HIRES_OUT( 6, line_out [6], SNES_NTSC_OUT_DEPTH );
			
			line_in  += 6;
			line_out += 7;
		}
		
		SNES_NTSC_COLOR_IN( 0, snes_ntsc_black );
		SNES_NTSC_HIRES_OUT( 0, line_out [0], SNES_NTSC_OUT_DEPTH );
		
		SNES_NTSC_COLOR_IN( 1, snes_ntsc_black );
		SNES_NTSC_HIRES_OUT( 1, line_out [1], SNES_NTSC_OUT_DEPTH );
		
		SNES_NTSC_COLOR_IN( 2, snes_ntsc_black );
		SNES_NTSC_HIRES_OUT( 2, line_out [2], SNES_NTSC_OUT_DEPTH );
		
		SNES_NTSC_COLOR_IN( 3, snes_ntsc_black );
		SNES_NTSC_HIRES_OUT( 3, line_out [3], SNES_NTSC_OUT_DEPTH );
		
		SNES_NTSC_COLOR_IN( 4, snes_ntsc_black );
		SNES_NTSC_HIRES_OUT( 4, line_out [4], SNES_NTSC_OUT_DEPTH );
		
		SNES_NTSC_COLOR_IN( 5, snes_ntsc_black );
		SNES_NTSC_HIRES_OUT( 5, line_out [5], SNES_NTSC_OUT_DEPTH );
		SNES_NTSC_HIRES_OUT( 6, line_out [6], SNES_NTSC_OUT_DEPTH );
		
		burst_phase = (burst_phase + 1) % snes_ntsc_burst_count;
		input += in_row_width;
		rgb_out = (char*) rgb_out + out_pitch;
	}
}

#endif
